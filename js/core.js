// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

var curses = require("curses");
var errno = require("errno");
var signal = require("signal");
var sys = require("sys");
var setInterval = require("js/timers.js").setInterval;

var EventListener = require("js/event_listener.js").EventListener;

var core = {
	column: 0,
	line: 0,
	inEscape: false, // true when part of an escape sequence
	viMode: true,
	logContents: false, // when true, log the file contents after each keypress
	curmode: "command",
	parser: require("js/parser.js").parser,
	listeners: {},
};

/**
 * Adds a function to the core object, and adds a displayName attribute.
 *
 * @param {string} name The name of the function
 * @param {function} func The function object
 */
core.addFunction = function (name, func) {
	func.displayName = name;
	core[name] = func;
	return func;
};
core.addFunction.displayName = "addFunction";

/**
 * Gets the current line in the buffer.
 *
 * @param {number} [line] the "current" line
 */
core.addFunction("currentLine", function (line) {
	if (line === undefined) {
		line = core.line;
	}
	return world.buffer.getLine(core.line);
});

/**
 * Converts a value to a boolean, optionally with a default value for undefined
 * values.
 *
 * @param val The current value
 * @param {boolean} [defaultValue] the default value, if val is undefined
 */
core.addFunction("toBool", function (val, defaultValue) {
	if (val === undefined) {
		val = defaultValue;
	}
	return !!val;
});

/**
 * Get the line number of the top line in the window (zero indexed).
 */
core.addFunction("windowTop", function () {
	return core.line - core.windows.buffer.getcury();
});

/**
 * Get the line number of the bottom line in the window (zero indexed).
 */
core.addFunction("windowBottom", function () {
	return core.windowTop() + core.windows.buffer.getmaxy();
});

/**
 * Low-level method to scroll a region of the screen. If lines is positive then
 * the screen is scrolled UP, i.e. line (n + lines) moves to the position on the
 * screen formerly occoupied by line n. And vice versa for negative lines.
 *
 * The value of core.line will *not* change as a result of calling this function.
 *
 * @param {number} lines The number of lines to scroll
 * @param {number} top The top line of the scroll region
 * @param {number} bot The bottom line of the scroll region
 */
core.addFunction("scrollRegion", function (lines, top, bot) {
	var curx = core.windows.buffer.getcurx();
	var cury = core.windows.buffer.getcury();
	var maxy = core.windows.buffer.getmaxy();
	var maxAllowed = world.buffer.length - 1;

	var wt = core.windowTop();
	log("wt = " + wt);
	if (wt + lines <= 0) {
		lines = -wt;
	} else if (wt + lines > maxAllowed) {
		lines -= (wt + lines - maxAllowed);
	}
	if (lines == 0) {
		return lines;
	}
	log("tesT");

	var newLine, newLinePos;
	var lineDelta = core.line - cury;
	for (var i = top; i <= bot; i++) {
		//log("moving line at " + (i + lineDelta + lines) + " to screen position " + i);
		newLinePos = i + lineDelta + lines;
		if (newLinePos > maxAllowed) {
			newLine = "~";
		} else {
			newLine = world.buffer.getLine(i + lineDelta + lines).value();
		}
		core.windows.buffer.mvaddstr(i, 0, newLine);
		core.windows.buffer.clrtoeol();
	}
	core.updateAllWindows();
	//core.move.absolute(cury, curx);
	return lines;
});

// higher-level method to scroll a region
//
// partition -- the line to partition on; always included in scrolling
// tophalf -- if true, top half is scrolled down, if false, bottom half is
//            scrolled up
// lines -- the number of lines to scroll by, always positive
core.addFunction("scroll", function (partition, tophalf, lines) {
	var top, bot;
	if (tophalf) {
		top = 0;
		bot = partition;
	} else {
		top = partition;
		bot = core.windows.buffer.getmaxy();
	}
	core.scrollRegion(top, bot, lines);
});

/**
 * Attempt to log a traceback.
 */
core.addFunction("logTrace", function () {
	var currentFunction = arguments.callee.caller;
	log("STACK TRACE");
	log("===========");
	while (currentFunction) {
		var fname = currentFunction.displayName;
		if (!fname) {
			var fn = currentFunction.toString();
			fname = fn.substring(fn.indexOf("function") + 8, fn.indexOf("")) || "anonymous";
		}
		log(fname + "()");
		currentFunction = currentFunction.caller;
	}
});

// move to an absolute window position (specified relative to the
// core.windows.buffer window)
core.addFunction("moveAbsolute", function (y, x) {
	core.windows.buffer.move(y, x);
	curses.stdscr.move(y + 1, x);
});

/**
 * This is the main interface to move the cursor. This takes of not only moving
 * the actual cursor on the screen, but also bounds checking (ensuring you can't
 * move off the end of the buffer), making sure that the screen is scrolled, and
 * that the current column/line is adjust appropriately.
 *
 * It is expected that this will be the main interface that almost all functions
 * that want to do movement commands will do. There should be almost no reason
 * for user-supplied JavaScript to be manually scrolling the screen, moving the
 * curses cursor, or using any of the other movement commands.
 *
 * @param {number} down The number of lines to scroll odwn; positive down means
 *                      moving DOWN, negative down means moving UP.
 * @param {number} [over] The number of columns to scroll horizontally. Positive
 *                        over means moving to the right, negative over means
 *                        moving to the left.
 * @param {bool} [restrictRight] If not explicitly set to false, we'll restrict
 *                               the cursor from scrolling past the right edge
 *                               of the new line.
 */
core.addFunction("move", function (down, over, restrictRight) {
	down = parseInt(down || 0);
	over = parseInt(over || 0);
	restrictRight = core.toBool(restrictRight, true);
	var cury = core.windows.buffer.getcury();
	var newy = cury, newx;

	if (down) {
		var newLine = core.line + down;

		// don't allow moving the cursor past the end of the buffer
		if (newLine >= world.buffer.length) {
			newLine = world.buffer.length - 1;
		} else if (newLine < 0) {
			newLine = 0;
		}

		// Compute the actual delta, after restricting scrolling past the end of
		// the buffer. Don't update core.line until after we've called
		// core.scrollRegion.
		var actualDelta = newLine - core.line;
		if (actualDelta != 0) {
			var maxy = core.windows.buffer.getmaxy();
			var targety = cury + actualDelta;
			if (targety < 0) {
				// we tried to move too far up, need to scroll the screen DOWN
				core.scrollRegion(targety, 0, maxy);
				core.line = newLine;
				newy = 0;
			} else if (targety >= maxy) {
				// we tried to move too far down, need to scroll the screen UP
				core.scrollRegion(targety - maxy + 1, 0, maxy);
				core.line = newLine;
				newy = maxy;
			} else {
				// just move the cursor
				core.line = newLine;
				newy = cury + actualDelta;
			}
		}
	}

	newx = core.restrictX(core.column + over);
	core.column = newx;
	var maxx = core.windows.buffer.getmaxx();
	core.move.absolute(newy, maxx < newx ? maxx : newx);
});
				
core.move.absolute = core.moveAbsolute;

core.move.left = function (updateBuffer) {
	updateBuffer = core.toBool(updateBuffer, true);
	var cury = core.windows.buffer.getcury();
	core.move.absolute(cury, 0);
	if (updateBuffer === true) {
		core.column = 0;
	}
};

core.move.right = function (pastText, updateBuffer) {
	pastText = core.toBool(pastText, false);
	updateBuffer = core.toBool(updateBuffer, true);
	var cury = core.windows.buffer.getcury();
	var newx;
	var maxx = core.windows.buffer.getmaxx();
	if (pastText) {
		newx = maxx;
	} else {
		var curlen = core.currentLine().length;
		newx = curlen < maxx ? curlen : maxx;
	}
	core.move.absolute(cury, newx);
	if (updateBuffer === true) {
		core.column = newx;
	}
};

core.addFunction("restrictX", function (newx, lineNum) {
	var line;
	if (lineNum === undefined) {
		lineNum = core.line;
	}
	line = world.buffer.getLine(lineNum);
	if (newx < 0) {
		newx = 0;
	} else if (newx > line.length) {
		newx = line.length;
	}
	var maxx = core.windows.buffer.getmaxx();
	if (newx > maxx) {
		newx = maxx;
	}
	core.column = newx;
	return newx;
});

core.addFunction("drawTabBar", function () {
	core.windows.tab.standend();
	core.windows.tab.attron(curses.A_BOLD);
	core.windows.tab.addstr(" " + world.buffer.getName() + " ");

	var spaces = "";
	var maxx = core.windows.tab.getmaxx();
	var curx = core.windows.tab.getcurx();
	while (spaces.length < maxx - curx - 1) {
		spaces += " ";
	}
	core.windows.tab.standout();
	core.windows.tab.addstr(spaces);

	core.windows.tab.standend();
	core.windows.tab.attron(curses.A_BOLD);
	core.windows.tab.attron(curses.A_UNDERLINE);
	core.windows.tab.addstr("X");
});

core.addFunction("drawStatus", function () {
	core.windows.status.standout();
	core.windows.status.mvaddstr(0, 0, "  ");
	var tabStr = "" + (core.line + 1) + "," + core.column;
	var ratio = core.windowBottom() * 100 / world.buffer.length;
	if (ratio > 100) {
		ratio = 100;
	}
	ratio = parseInt(ratio);
	tabStr += "  (" + ratio + "%)";

	core.windows.status.addstr(tabStr);

	var fmtTime = function (n) {
		if (n < 10) {
			return new String("0" + n);
		} else {
			return new String(n);
		}
	};
	var statusEnd = "mode: " + core.curmode + "     ";
	var d = new Date();
	statusEnd += (fmtTime(d.getHours()) + ":" +
				  fmtTime(d.getMinutes()) + ":" +
				  fmtTime(d.getSeconds()) + " ");

	var spacesNeeded = (core.windows.status.getmaxx() -
						core.windows.status.getcurx() -
						statusEnd.length);
	var spaces = "";
	for (var i = 0; i < spacesNeeded; i++) {
		spaces += " ";
	}
	core.windows.status.addstr(spaces);
	core.windows.status.addstr(statusEnd);
	core.windows.status.standend();
	core.moveAbsolute(core.windows.buffer.getcury(), core.column);
});

// call drawStatus() once a second, and update all of the windows; this ensures
// that the clock in the corner is refreshed
setInterval(function () {
	core.drawStatus();
	core.updateAllWindows();
}, 1000);

core.addFunction("updateAllWindows", function (doupdate) {
	if (doupdate === undefined) {
		doupdate = true;
	}
	var w;
	for (w in core.windows) {
		if (core.windows.hasOwnProperty(w) && core.windows[w].noutrefresh) {
			// update each stdscr subwindow
			core.windows[w].noutrefresh();
		}
	}
	// update stdscr
	curses.stdscr.noutrefresh();

	// do the update
	curses.doupdate();
});

// Called when the editor is first loaded; this loads any files specified on the
// command line.
world.addEventListener("load", function (event) {
	if (world.args.length) {
		world.buffer.open(world.args[0]);
	}
});

// Draw and create the initial windows.
world.addEventListener("load", function (event) {
	core.windows = {};
	core.windows.tab = curses.stdscr.subwin(1, curses.stdscr.getmaxx(), 0, 0);
	core.drawTabBar();

	core.windows.buffer = curses.stdscr.subwin(curses.stdscr.getmaxy() - 3, curses.stdscr.getmaxx(), 1, 0);
	core.windows.buffer.scrollok(true);

	core.windows.status = curses.stdscr.subwin(2, curses.stdscr.getmaxx(), curses.stdscr.getmaxy() - 2, 0);
	core.drawStatus();
});

// Draw the buffer contents to the main window.
world.addEventListener("load", function (event) {
	var i = 0;
	var maxy = core.windows.buffer.getmaxy();
	var buflen = world.buffer.length;
	if (buflen > maxy) {
		buflen = maxy;
	}
	for (i = 0; i < buflen; i++) {
		core.windows.buffer.mvaddstr(i, 0, world.buffer.getLine(i).value());
	}
	for (; i < maxy; i++) {
		core.windows.buffer.mvaddstr(i, 0, "~");
	}
});

// Move the cursor and update all windows.
world.addEventListener("load", function (event) {
	core.moveAbsolute(0, 0);
	core.updateAllWindows();
});

// The absolute lowest level interface to keypresses is callbacks registered by
// world.addEventListener("keypress", ...). That's not really a useful
// abstraction though, because it doesn't deal with modes. The key handling for
// the character 'x' is different if you're in vi emulation command mode, if
// you're in insert mode, if you're typing in the minibuffer, etc. If you wanted
// to deal with these details directly in an event handler for raw keypresses,
// there would be a *lot* of complication caused by checking which mode you're
// in, etc. There are also complications with the keycode abstraction provided
// by TTYs (for instance, "meta" characters (e.g. holding down Alt while typing
// another character) are sent as two separate characters.
//
// For this reason, the editor is separated into differnt "modes". An example of
// a mode is "insert" for insert mode in the main buffer, "command" for
// emulating vi's command mode, etc. There can be arbitrarily many modes, and
// the current mode is set by the value of core.curmode.
//
// When a keypress happens, any handlers that have registered themself for the
// mode using core.addKeypressListener will be called.
core.addKeypressListener = function (mode, handler) {
	core.listeners[mode] = core.listeners[mode] || new EventListener();
	core.listeners[mode].addEventListener("keypress", handler);
};

// The main keypress listener for insert mode.
core.addKeypressListener("insert", function (event) {
	var curx = core.windows.buffer.getcurx();
	var cury = core.windows.buffer.getcury();
	var code = event.getCode();
	var msg = "@(" + core.line + ", " + core.column + ")  isKeypad = " + event.isKeypad() + ", code = " + code;
	if (!event.isKeypad()) {
		var wch = event.getChar();
		msg += ", wch = '" + wch + "'";
		log(msg);
		switch (code) {
		case 1: // Ctrl-A
			core.move.left();
			break;
		case 3: // Ctrl-C
			// this should have been handled already
			break;
		case 5: // Ctrl-E
			core.move.right();
			break;
		case 12: // Ctrl-L
			curses.redrawwin();
			break;
		case 13: // Ctrl-M, carriage return
			// chop the line
			var line = core.currentLine();
			var chopped = "";
			if (core.column < line.length) {
				chopped = line.value().substring(core.column, line.length - core.column + 1);
				line.chop(core.column);
			}
			core.scrollRegion(-1, core.windows.buffer.getcury() + 1, core.windows.buffer.getmaxy());
			// add the new line, with the chopped contents
			world.buffer.addLine(core.line + 1, chopped);

			//move the cursor
			core.windows.buffer.clrtoeol();
			core.move(1);
			if (chopped) {
				core.windows.buffer.addstr(chopped);
				core.move.left();
			}
			//core.line++;
			core.column = 0;
			core.move(0);
			break;
		case 19: // Ctrl-S
			world.buffer.persist(world.buffer.getFile())
		case 26: // Ctrl-Z
			sys.kill(sys.getpid(), sys.SIGTSTP);
			break;
		default:
			var curline = world.buffer.getLine(core.line);
			curline.insert(core.column, wch);
			if (core.column < curline.length - 1) {
				core.windows.buffer.insch(wch);
				core.move(0, 1);
			} else {
				core.windows.buffer.addstr(wch);
			}
			core.column++;
			break;
		}
	} else {
		var name = event.getName();
		msg += ", name = " + name;
		//log(msg);
		switch (name) {
		case "KEY_BACKSPACE":
			log("backspace, core.line = " + core.line);
			if (core.column > 0) {
				var curline = world.buffer.getLine(core.line);
				core.windows.buffer.mvdelch(cury, curx - 1);
				curses.stdscr.move(cury + 1, curx - 1);
				core.column--;
				curline.erase(core.column, 1);
			} else if (core.line > 0) {
				// update the buffers
				var curline = world.buffer.getLine(core.line);
				var origSize = curline.length;
				var contents = curline.value();
				world.buffer.deleteLine(core.line--);
				curline = world.buffer.getLine(core.line);
				core.column = curline.length;
				if (contents) {
					curline.append(contents);
				}

				// clear the current line
				core.moveAbsolute(cury, 0);
				core.windows.buffer.clrtoeol();
				// scroll up
				core.scrollRegion(1, core.windows.buffer.getcury(), core.windows.buffer.getmaxy());
				// redraw the previous line
				core.moveAbsolute(cury - 1, core.column);
				if (contents) {
					core.windows.buffer.addstr(contents)
					core.moveAbsolute(cury - 1, core.column);
				}
			}
			break;
		case "KEY_DOWN":
			if (core.line < world.buffer.length - 1) {
				core.move(1);
			}
			break;
		case "KEY_END":
			core.move.right();
			break;
		case "KEY_HOME":
			core.move.left();
			break;
		case "KEY_LEFT":
			core.move(0, -1);
			break;
		case "KEY_NPAGE": // page down
			var maxy = core.windows.buffer.getmaxy();
			var delta = maxy - 1 - cury;
			core.move(maxy - 1);
			break;
		case "KEY_PPAGE": // page up
			var maxy = core.windows.buffer.getmaxy();
			var delta = maxy - 1 - cury;
			core.move(1 - maxy);
			break;
		case "KEY_RIGHT":
			core.move(0, 1);
			break;
		case "KEY_UP":
			if (core.line > 0)
				core.move(-1);
			break;
		}
	}
});

// The main keypress listener for command mode.
core.addKeypressListener("command", function (event) {
	var code = event.getCode();
	var wch = event.getChar();

	var enterInsert = function () {
		core.curmode = "insert";
	};
	switch (wch) {
	case 'a':
	case 'A':
		enterInsert();
		break;
	case 'c':
	case 'C':
		enterInsert();
		break;
	case 'h':
		core.move(0, -1);
		break;
	case 'j':
		core.move(1);
		break;
	case 'k':
		core.move(-1);
		break;
	case 'l':
		core.move(0, 1);
		break;
	case 'i':
	case 'I':
		enterInsert();
		break;
	case 'o':
	case 'O':
		enterInsert();
		break;
	case 's':
	case 'S':
		enterInsert();
		break;
	}
});

// This method checks for Ctrl-C. We add it as its own top level handler to
// prevent the editor from getting "stuck" due other JavaScript errors (IOW, no
// matter what else happens you'll be able to exit using Ctrl-C).
world.addEventListener("keypress", function (event) {
	if (event.getCode() == 3) {
		log("Caught Ctrl-C, stopping the main loop");
		world.stopLoop();
	}
});

// Core routine called on each keypress; this essentially just dispatches to
// listeners registered by core.addKeypressListener (see the comments at that
// function for the details).
world.addEventListener("keypress", function (event) {
	var code = event.getCode();

	// Escape sequences are sent as two separate characters. For instance, if
	// you type M-x (the "Alt" key followed by the letter 'x'), the sequence
	// sent to the terminal is the two character sequence ^[ x.
	//
	// When the character ^[ is sent we set core.inEscape to true, and skip any
	// other processing. After the next character is entered and handled
	// core.inEscape will be reset to false.
	var setEscape = false;
	switch (code) {
	case 27: // ^[ a.k.a. Ctrl-[ a.k.a. escape
		if (core.viMode) {
			core.curmode = "command";
			core.move(0, -1);  // like vi
		} else {
			core.inEscape = true;
		}
		setEscape = true;
		break;
	}
	if (!setEscape) {
		var listener = core.listeners[core.curmode] || new EventListener();
		listener.dispatch("keypress", event);
		core.inEscape = false;
	}

	// refresh the status bar
	core.drawStatus();

	if (core.logContents) {
		log("");
		log("<<<<<< START <<<<<<");
		var lines = world.buffer.getContents();
		for (var i = 0; i < lines.length; i++) {
			log('"' + lines[i] + '"');
		}
		log(">>>>>>> END >>>>>>>");
	}
});

// This is the callback that specifically causes curses to flush all of its
// drawing operations. It *must* be called after any functions that may do
// drawing operations, which is why it has its own special event. In general,
// it's not recommended that other functions register to listen to the
// "after_keypress" event, and if they do they must not do any drawing
// operations.
//
// It might be possible to just fold this into the main event listener for
// "keypress" events, which users should not be hooking into. But for extra
// safety, it's split out like this.
world.addEventListener("after_keypress", function (e) {
	core.updateAllWindows();
});

