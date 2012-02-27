log("started script!");

window.addEventListener("keypress", function (event) {
    var curx = window.getcurx();
    var cury = window.getcury();
	var code = event.getCode();
	var name = event.getName();
	log("got keypress, code is " + code + ", name is \"" + name + "\", name length is " + name.length);
	if (event.isASCII()) {
		switch (code) {
		case 1: // Ctrl-A
			window.move(cury, 0);
			break;
		case 5: // Ctrl-E
			window.move(cury, window.getmaxx() - 1);
			break;
		case 12: // Ctrl-L
			window.redrawwin();
			break;
		case 13: // Ctrl-M, carriage return
			window.move(cury + 1, 0);
			break;
		case 17: // Ctrl-Q
			window.stopLoop();
			break;
		default:
			window.addstr(name);
			break;
		}
	} else {
		switch (name) {
		case "key_backspace":
			if (curx > 0) {
				window.mvdelch(cury, curx -1);
			}
			break;
		case "key_down":
			if (cury < window.getmaxy() - 1) {
				window.move(cury + 1, curx);
			}
			break;
		case "key_end":
			window.move(cury, window.getmaxx() - 1);
			break;
		case "key_home":
			window.move(cury, 0);
			break;
		case "key_left":
			if (curx > 0) {
				window.move(cury, curx - 1);
			}
			break;
		case "key_right":
			if (curx < window.getmaxx() - 1) {
				window.move(cury, curx + 1);
			}
			break;
		case "key_up":
			if (cury > 0) {
				window.move(cury - 1, curx);
			}
			break;
		}
	}
});
