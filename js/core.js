core = { column: 0, line: 0 };

log("started script!");
log("buffer name is " + window.buffer.getName());

core.currentLine = function () {
    return window.buffer.getLine(core.line);
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
    restore = restore || true;
    if (restore) {
        var x = curses.getcurx();
        var y = curses.getcury();
        curses.move(0, 1);
        curses.addstr(window.buffer.getName());
        curses.move(y, x);
    } else {
        curses.move(0, 0);
        curses.addstr(window.buffer.getName());
    }
}

window.addEventListener("load", function (event) {
    core.drawTabBar();

    // draw tildes on blank lines
    var maxy = curses.getmaxy();
    for (var i = 2; i < maxy; i++) {
        curses.move(i, 0);
        curses.addstr("~");
    }
    curses.move(1, 0);
});

window.addEventListener("keypress", function (event) {
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
            window.stopLoop();
            break;
        case 5: // Ctrl-E
            curses.move(cury, curses.getmaxx() - 1);
            break;
        case 12: // Ctrl-L
            curses.redrawwin();
            break;
        case 13: // Ctrl-M, carriage return
            window.buffer.addLine(cury + 1);
            curses.move(cury + 1, 0);
            curses.clrtoeol();
            core.line++;
            core.column = 0;
            break;
        case 17: // Ctrl-Q
            window.stopLoop();
            break;
        case 26: // Ctrl-Z
            sys.kill(sys.getpid(), sys.SIGTSTP);
            break;
        default:
            var curline = window.buffer.getLine(cury);
            var val = curline.value();
            curline.insert(curx, name);
            curses.addstr(name);
            log(curline.length);
            if (curx < curline.length) {
                // need to redraw the rest of the string to prevent overwrite
                // FIXME: this is slow
                curses.clrtoeol();
                curses.addstr(val.substr(curx));
                curses.move(cury, curx + 1);
                core.column++;
            }
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
            if (cury < curses.getmaxy() - 1 && (core.line < window.buffer.length - 1)) {
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
