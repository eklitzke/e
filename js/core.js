core = { column: 0, line: 0, windowTop: 0 };

log("started script!");
log("buffer name is " + world.buffer.getName());

// get the current line in the buffer
core.currentLine = function () {
	return world.buffer.getLine(core.line);
};

// convert a value to a boolean
core.toBool = function (val, defaultValue) {
	if (val === undefined) {
		val = defaultValue;
	}
	return !!val;
};

// move to an absolute position (specified relative to the core.windows.buffer
// window)
core.moveAbsolute = function (y, x) {
	core.windows.buffer.move(y, x);
	curses.stdscr.move(y + 1, x);
};

// move to a relative position
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
		} else if (newy > maxy) {
			newy = maxy;
		}
		if (newy != cury) {
			core.line += newy - cury;
		}
	}
	if (over) {
		var newx = curx + over;
		var maxx = core.windows.buffer.getmaxx();
		if (newx < 0) {
			newx = 0;
		} else if (newx > maxx) {
			newx = maxx;
		}
		if (newx != curx) {
			core.column += newx - curx;
		}
	}
	core.moveAbsolute(newy, newx);
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

core.drawTabBar = function (restore) {
	var val = "";
	var maxx = core.windows.tab.getmaxx();
	while (val.length < maxx) {
		val += " ";
	}
	core.windows.tab.standout();
	core.windows.tab.mvaddstr(0, 0, val);
	core.windows.tab.move(0, 1);
	core.windows.tab.standend();
	core.windows.tab.addstr(world.buffer.getName());
	core.windows.tab.refresh();
};

core.updateAllWindows = function (doupdate) {
	if (doupdate === undefined) {
		doupdate = true;
	}
	var w;
	for (w in core.windows) {
		if (core.windows.hasOwnProperty(w) && core.windows[w].noutrefresh) {
			core.windows[w].noutrefresh();
		}
	}
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
	core.windows.buffer = curses.stdscr.subwin(curses.stdscr.getmaxy() - 1, curses.stdscr.getmaxx(), 1, 0);

	// draw tildes on blank lines
	var maxy = core.windows.buffer.getmaxy();
	for (var i = 1; i < maxy; i++) {
		core.windows.buffer.mvaddstr(i, 0, "~");
	}
	core.moveAbsolute(0, 0);
	core.updateAllWindows();
});

// Core routine called on each keypress
world.addEventListener("keypress", function (event) {
	log("COLORS = " + curses.COLORS + ", COLOR_PAIRS = " + curses.COLOR_PAIRS);
	var curx = core.windows.buffer.getcurx();
	var cury = core.windows.buffer.getcury();
	var code = event.getCode();
	var name = event.getName();
	if (event.isASCII()) {
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
			world.buffer.addLine(core.line + 1);
			core.move.left();
			core.move(1);
			core.windows.buffer.clrtoeol();
			// FIXME: scroll lines down
			break;
		//case 17: // Ctrl-Q
		//	world.stopLoop();
		//	break;
		case 26: // Ctrl-Z
			sys.kill(sys.getpid(), sys.SIGTSTP);
			break;
		default:
			var curline = world.buffer.getLine(core.line);
			curline.insert(core.column, name);
			// FIXME: use insch
			if (core.column < curline.length - 1) {
				// we need to redraw the rest of the string to prevent overwrite
				var val = curline.value();
				log("line value is " + val);
				core.windows.buffer.clrtoeol();
				core.windows.buffer.addstr(val.substr(core.column));
				core.move.absolute(cury, curx + 1); // XXX: need to check curxhere
			} else {
				core.windows.buffer.addstr(name);
			}
			core.column++;
			break;
		}
	} else {
		switch (name) {
		case "key_backspace":
			if (core.column > 0) {
				var curline = world.buffer.getLine(core.line);
				core.windows.buffer.mvdelch(cury, curx - 1);
				curses.stdscr.move(cury + 1, curx - 1);
				core.column--;
				curline.erase(core.column, 1);
			}
			break;
		case "key_down":
			if (core.line < world.buffer.length - 1) {
				core.move(1);
			}
			break;
		case "key_end":
			core.move.right();
			break;
		case "key_home":
			core.move.left();
			break;
		case "key_left":
			core.move(0, -1);
			break;
		case "key_right":
			core.move(0, 1);
			break;
		case "key_up":
			if (core.line > 0)
				core.move(-1);
			break;
		}
	}
});

world.addEventListener("keypress", function (e) {
	log("===============");
	var lines = world.buffer.getContents();
	for (var i = 0; i < lines.length; i++) {
		log(lines[i]);
	}
});

world.addEventListener("keypress", function (e) {
	core.updateAllWindows();
});
