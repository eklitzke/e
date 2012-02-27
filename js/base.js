log("started script!");

window.addEventListener("keypress", function (event) {
    var curx = window.getcurx();
    var cury = window.getcury();
	var code = event.getCode();
	var name = event.getName();
	log("got keypress, code is " + code + ", name is \"" + name + "\", name length is " + name.length);
	if (event.isASCII()) {
        var print = true;
		switch (code) {
		case 13:
			window.move(cury + 1, 0);
			print = false;
			break;
		}
		if (print) {
			if (name === "q")
				window.stopLoop();
			else
				window.addstr(name);
		}
	} else {
		switch (name) {
		case "key_backspace":
			if (curx > 0) {
				window.mvdelch(cury, curx -1);
			}
			break;
		case "key_down":
			if (cury < window.getmaxy()) {
				window.move(cury + 1, curx);
			}
			break;
		case "key_left":
			if (curx > 0) {
				window.move(cury, curx - 1);
			}
			break;
		case "key_right":
			if (curx < window.getmaxx()) {
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
