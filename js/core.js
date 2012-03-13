core = { column: 0, line: 0, windowTop: 0 };

log("entered core.js");

/**
 * Gets the current line in the buffer.
 *
 * @param {number} [line] the "current" line
 */
core.currentLine = function (line) {
	if (line === undefined) {
		line = core.line;
	}
	return world.buffer.getLine(core.line);
};

/**
 * Converts a value to a boolean, optionally with a default value for undefined
 * values.
 *
 * @param val The current value
 * @param {boolean} [defaultValue] the default value, if val is undefined
 */
core.toBool = function (val, defaultValue) {
	if (val === undefined) {
		val = defaultValue;
	}
	return !!val;
};

/**
 * Low-level method to scroll a region of the screen.
 *
 * @param {number} lines The number of lines to scroll
 * @param {number} top The top line of the scroll region
 * @param {number} bot The bottom line of the scroll region
 */
core.scrollRegion = function (lines, top, bot) {
	var cury = core.windows.buffer.getcury();
	var maxy = core.windows.buffer.getmaxy();
	if (lines < 0 && core.line == 0) {
		// if we're already at the top of the screen, we can't scroll up
		return;
	}
	else if (lines > 0) {
		// make sure the last effective line isn't past the last line we can see
		// in the buffer
		var maxEffectiveLine = world.buffer.length + maxy - 1;
		// XXX: off by one error?
		if (core.windowTop + lines + maxy > maxEffectiveLine) {
			return;
		}
	}
	core.line += (lines - cury);
	core.windowTop += lines;
	core.windows.buffer.setscrreg(top, bot);
	core.windows.buffer.scrl(lines);
	log("a " + lines);

	if (lines > 0) {
		for (var i = maxy - lines; i < maxy; i++) {
			var lineNum = core.windowTop + i;
			log("lineNum " + lineNum);
			if (lineNum > world.buffer.length) {
				core.windows.buffer.mvaddstr(i, 0, "~");
			} else {
				var val = world.buffer.getLine(lineNum).value();
				log(val);
				core.windows.buffer.mvaddstr(i, 0, val);
			}
		}
		//core.windows.buffer.moveAbsolute(0, 0);
		core.moveAbsolute(0, 0);
	}
};

// higher-level method to scroll a region
//
// partition -- the line to partition on; always included in scrolling
// tophalf -- if true, top half is scrolled down, if false, bottom half is
//            scrolled up
// lines -- the number of lines to scroll by, always positive
core.scroll = function (partition, tophalf, lines) {
	var top, bot;
	if (tophalf) {
		top = 0;
		bot = partition;
	} else {
		top = partition;
		bot = core.windows.buffer.getmaxy();
	}
	core.scrollRegion(top, bot, lines);
};

// move to an absolute window position (specified relative to the
// core.windows.buffer window)
core.moveAbsolute = function (y, x) {
	core.windows.buffer.move(y, x);
	curses.stdscr.move(y + 1, x);
};

// move to a new position, relative to the current cursor position (delta-y =
// `up` and delta-x = `over`).
core.move = function (up, over, restrictRight) {
	up = parseInt(up || 0);
	over = parseInt(over || 0);
	restrictRight = core.toBool(restrictRight, true);
	var curx = core.windows.buffer.getcurx();
	var cury = core.windows.buffer.getcury();
	var newx = curx;
	var newy = cury;
	if (up) {
		var newy = cury + up;
		var maxy = core.windows.buffer.getmaxy();
		if (newy < 0) {
			newy = 0;
		} else if (newy > maxy - 1) {
			//newy = maxy - 1;
			core.scrollRegion(up, 0, maxy);
			return;
		}
		if (newy != cury) {
			core.line += newy - cury;
		}
	}
	newx = core.restrictX(curx + over);
	if (newx != curx) {
		core.column += newx - curx;
	}
	if (newx != curx || newy != cury) {
		core.moveAbsolute(newy, newx);
	}
};

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

core.restrictX = function (newx, lineNum) {
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
	return newx;
};

core.drawTabBar = function () {
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
};

core.drawStatus = function () {
	core.windows.status.standout();
	core.windows.status.mvaddstr(0, 0, "  ");
	core.windows.status.addstr(core.line + 1 + "," + core.column);

	var spaces = "";
	var curx = core.windows.status.getcurx();
	var maxx = core.windows.status.getmaxx();
	while (spaces.length < maxx - curx) {
		spaces += " ";
	}
	core.windows.status.addstr(spaces);
	core.windows.status.standend();
	core.moveAbsolute(core.windows.buffer.getcury(), core.windows.buffer.getcurx());
};

core.updateAllWindows = function (doupdate) {
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
};

// This function is called when the terminal needs to be scrolled (e.g. after
// using an arrow key, after hitting a newline, etc.). The screen contents may
// or may not be scolled, but the cursor will generally move.
core.scrollTo = function (lineNumber) {
	if (lineNumber === undefined) {
		lineNumber = core.line;
	}
	curses.move(1 + lineNumber, core.rightmost());
};

// Called when the editor is loaded
world.addEventListener("load", function (event) {
	core.windows = {};
	core.windows.tab = curses.stdscr.subwin(1, curses.stdscr.getmaxx(), 0, 0);
	core.drawTabBar();

	core.windows.buffer = curses.stdscr.subwin(curses.stdscr.getmaxy() - 3, curses.stdscr.getmaxx(), 1, 0);
	core.windows.buffer.scrollok(true);

	core.windows.status = curses.stdscr.subwin(2, curses.stdscr.getmaxx(), curses.stdscr.getmaxy() - 2, 0);
	core.drawStatus();
});

world.addEventListener("load", function (event) {
	if (world.args.length) {
		world.buffer.open(world.args[0]);
	}
});

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

world.addEventListener("load", function (event) {
	core.moveAbsolute(0, 0);
	core.updateAllWindows();
});

// Core routine called on each keypress
world.addEventListener("keypress", function (event) {
	var curx = core.windows.buffer.getcurx();
	var cury = core.windows.buffer.getcury();
	var code = event.getCode();
	if (!event.isKeypad()) {
		var wch = event.getChar();
		switch (code) {
		case 1: // Ctrl-A
			core.move.left();
			break;
		case 3: // Ctrl-C
			world.stopLoop();
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
				chopped = line.chop(core.column);
			}
			// add the new line, with the chopped contents
			world.buffer.addLine(core.line + 1, chopped);

			//move the cursor
			core.windows.buffer.clrtoeol();
			core.move.absolute(cury + 1, 0);
			core.windows.buffer.setscrreg(cury + 1, core.windows.buffer.getmaxy() - 1);
			core.windows.buffer.scrl(-1);
			if (chopped) {
				core.windows.buffer.addstr(chopped);
				core.move.left();
			}
			core.line++;
			core.column = 0;
			break;
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
		switch (name) {
		case "KEY_BACKSPACE":
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
				if (contents) {
					curline.append(contents);
				}

				// clear the current line
				core.moveAbsolute(cury, 0);
				core.windows.buffer.clrtoeol();
				// scroll up
				// redraw the previous line
				core.column = origSize;
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
			core.scrollRegion(1);
			break;
		case "KEY_PPAGE": // page up
			core.scrollRegion(-1);
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

world.addEventListener("keypress", function (e) {
	core.drawStatus();
});

world.addEventListener("keypress", function (e) {
	//log("===============");
	//var lines = world.buffer.getContents();
	/*for (var i = 0; i < lines.length; i++) {
		log(lines[i]);
	}*/
});

// This is the callback that specifically causes curses to flush all of its
// drawing operations. It *must* be called after any functions that may do
// drawing operations, which is why it has its own special event. In general,
// it's not recommended that other functions register to listen to the
// "after_keypress" event, and if they do they must not do any drawing
// operations.
world.addEventListener("after_keypress", function (e) {
	core.updateAllWindows();
});
