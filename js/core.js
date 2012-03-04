core = { column: 0, line: 0 };

log("started script!");
log("buffer name is " + world.buffer.getName());

core.currentLine = function () {
    return world.buffer.getLine(core.line);
}

core.rightmost = function () {
    var l = core.currentLine();
    if (l.length < core.column) {
        return l.length;
    } else {
        return core.column;
    }
}

core.drawTabBar = function (restore) {
	core.windows.tab.mvaddstr(0, 1, world.buffer.getName());
	//core.windows.tab.refresh();
}

world.addEventListener("load", function (event) {
	core.windows = {};
	core.windows.tab = curses.stdscr.subwin(1, curses.getmaxx(), 0, 0);
	core.windows.buffer = curses.stdscr.subwin(curses.getmaxy() - 1, curses.getmaxx(), 1, 0);

	core.drawTabBar();

    // draw tildes on blank lines
    var maxy = curses.getmaxy();
    for (var i = 0; i < maxy; i++) {
        core.windows.buffer.mvaddstr(i, 0, "~");
    }
	//core.windows.buffer.move(1, 0);
	//core.windows.buffer.refresh();
	//curses.doupdate();
});

world.addEventListener("keypress", function (event) {
    var curx = curses.getcurx();
    var cury = curses.getcury();
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
            curses.move(cury, curses.getmaxx() - 1);
            break;
        case 12: // Ctrl-L
            curses.redrawwin();
            break;
        case 13: // Ctrl-M, carriage return
            world.buffer.addLine(cury + 1);
            curses.move(cury + 1, 0);
            curses.clrtoeol();
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
            curses.addstr(name);
            log(curline.length);
            if (curx < curline.length) {
                // need to redraw the rest of the string to prevent overwrite
                // FIXME: this is slow
                curses.clrtoeol();
                curses.addstr(val.substr(curx));
                curses.move(cury, curx + 1);
            }
            core.column++;
            break;
        }
    } else {
        switch (name) {
        case "key_backspace":
            if (curx > 0 && core.column > 0) {
                curses.mvdelch(cury, curx -1);
                if (core.column > 0) {
                    core.column--;
                }
            }
            break;
        case "key_down":
            if (cury < curses.getmaxy() - 1 && (core.line < world.buffer.length - 1)) {
                core.line++;
                curses.move(cury + 1, core.rightmost());
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
                curses.move(cury, core.column);
            }
            break;
        case "key_right":
            if (curx < curses.getmaxx() - 1) {
                core.column++;
                curses.move(cury, core.column);
            }
            break;
        case "key_up":
            if (cury > 0 && core.line > 0) {
                core.line--;
                curses.move(cury - 1, core.rightmost());
            }
            break;
        }
    }
});
