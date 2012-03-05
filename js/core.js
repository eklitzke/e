core = { column: 0, line: 0, windowTop: 0 };

log("started script!");
log("buffer name is " + world.buffer.getName());

core.currentLine = function () {
    return world.buffer.getLine(core.line);
};

core.checkCall = function (f) {
	log("calling f, " + f());
}

core.rightmost = function () {
    var l = core.currentLine();
    if (l.length < core.column) {
        return l.length;
    } else {
        return core.column;
    }
};

core.drawTabBar = function (restore) {
	core.windows.tab.mvaddstr(0, 1, world.buffer.getName());
};

core.updateAllWindows = function (doupdate) {
	if (doupdate === undefined)
		doupdate = true;
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

world.addEventListener("load", function (event) {
	core.windows = {};
	core.windows.tab = curses.stdscr.subwin(1, curses.stdscr.getmaxx(), 0, 0);
	core.windows.buffer = curses.stdscr.subwin(curses.stdscr.getmaxy() - 1, curses.stdscr.getmaxx(), 1, 0);

	core.drawTabBar();

    // draw tildes on blank lines
    var maxy = core.windows.buffer.getmaxy();
    for (var i = 1; i < maxy; i++) {
        core.windows.buffer.mvaddstr(i, 0, "~");
    }
	core.windows.buffer.move(0, 0);
	curses.stdscr.move(1, 0);
	core.updateAllWindows();
});

world.addEventListener("keypress", function (event) {
	log(core.windows.buffer);
    var curx = core.windows.buffer.getcurx();
    var cury = core.windows.buffer.getcury();
    var code = event.getCode();
    var name = event.getName();
    //log("got keypress, code is " + code + ", name is \"" + name + "\", name length is " + name.length);
    if (event.isASCII()) {
        switch (code) {
        case 1: // Ctrl-A
            curses.move(cury, 0);
            break;
        case 3: // Ctrl-C
            world.stopLoop();
            break;
        case 5: // Ctrl-E
            curses.move(cury, core.windows.buffer.getmaxx() - 1);
            break;
        case 12: // Ctrl-L
            curses.redrawwin();
            break;
        case 13: // Ctrl-M, carriage return
            world.buffer.addLine(core.line + 1);
			core.windows.buffer.move(cury + 1, 0);
            curses.stdscr.move(cury + 2, 0);
            core.windows.buffer.clrtoeol();
            core.line++;
            core.column = 0;
            break;
        case 17: // Ctrl-Q
            world.stopLoop();
            break;
        case 26: // Ctrl-Z
            sys.kill(sys.getpid(), sys.SIGTSTP);
            break;
        default:
            var curline = world.buffer.getLine(core.line);
            var val = curline.value();
            curline.insert(core.column, name);
			core.windows.buffer.addstr(name);
            //curses.addstr(name);
            if (curx < curline.length) {
                // need to redraw the rest of the string to prevent overwrite
				core.windows.buffer.clrtoeol();
				core.windows.buffer.addstr(val.substr(core.column));
                curses.move(cury + 1, curx + 1); // XXX: need to check curxhere
            }
            core.column++;
            break;
        }
    } else {
        switch (name) {
        case "key_backspace":
            if (curx > 0 && core.column > 0) {
                //curses.mvdelch(cury, curx -1);
				//core.windows.buffer.mvdelch(
                if (core.column > 0) {
                    core.column--;
                }
            }
            break;
        case "key_down":
            if (cury < core.windows.buffer.getmaxy() - 1 && (core.line < world.buffer.length - 1)) {
                core.line++;
				core.windows.buffer.move(cury + 1, core.rightmost());
                curses.move(cury + 2, core.rightmost());
            }
            break;
        case "key_end":
            core.column = core.currentLine().length;
            curses.move(cury, core.column);
            break;
        case "key_home":
            core.column = 0;
            curses.move(cury, 0);
            break;
        case "key_left":
            if (curx > 0) {
                if (core.column)
                    core.column--;
                core.windows.buffer.move(cury, core.column);
                curses.stdscr.move(cury + 1, core.column);
            }
            break;
        case "key_right":
            if (curx < core.windows.buffer.getmaxx() - 1) {
				core.column++;
				core.windows.buffer.move(cury, core.column);
				curses.stdscr.move(cury + 1, core.column);
            }
            break;
        case "key_up":
            if (cury > 0 && core.line > 0) {
                core.line--;
				core.windows.buffer.move(cury - 1, core.rightmost());
                curses.stdscr.move(cury, core.rightmost());
            }
            break;
        }
    }
});

world.addEventListener("keypress", function (e) {
	core.updateAllWindows();
});
