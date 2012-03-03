log("started script!");

window.addEventListener("keypress", function (event) {
	log("first line is " + window.buffer.getLine(0).value());
    var curx = curses.getcurx();
    var cury = curses.getcury();
	var code = event.getCode();
	var name = event.getName();
	log("buffer name is " + window.buffer.getName());
	log("got keypress, code is " + code + ", name is \"" + name + "\", name length is " + name.length);
	if (event.isASCII()) {
		switch (code) {
		case 1: // Ctrl-A
			curses.move(cury, 0);
			break;
		case 5: // Ctrl-E
			curses.move(cury, curses.getmaxx() - 1);
			break;
		case 12: // Ctrl-L
			curses.redrawwin();
			break;
		case 13: // Ctrl-M, carriage return
			curses.move(cury + 1, 0);
			break;
		case 17: // Ctrl-Q
			window.stopLoop();
			break;
		default:
			window.buffer.getLine(0).insert(curx, name);
			curses.addstr(name);
			break;
		}
	} else {
		switch (name) {
		case "key_backspace":
			if (curx > 0) {
				curses.mvdelch(cury, curx -1);
			}
			break;
		case "key_down":
			if (cury < curses.getmaxy() - 1) {
				curses.move(cury + 1, curx);
			}
			break;
		case "key_end":
			curses.move(cury, curses.getmaxx() - 1);
			break;
		case "key_home":
			curses.move(cury, 0);
			break;
		case "key_left":
			if (curx > 0) {
				curses.move(cury, curx - 1);
			}
			break;
		case "key_right":
			if (curx < curses.getmaxx() - 1) {
				curses.move(cury, curx + 1);
			}
			break;
		case "key_up":
			if (cury > 0) {
				curses.move(cury - 1, curx);
			}
			break;
		}
	}
});
