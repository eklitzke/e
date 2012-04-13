var curses = require("curses");

// This are various methods we want to run when the editor is loaded. This file
// is loaded by core.js just for its side effects.

// Called when the editor is first loaded; this loads any files specified on the
// command line.
world.addEventListener("load", function (event) {
	if (world.args.length) {
		try {
			world.buffer.open(world.args[0]);
		} catch (e) {
			core.setError("failed to open \"" + world.args[0] + "\" due to " + errno.errorcode[e]);
		}
	}
});

// Draw and create the initial windows.
world.addEventListener("load", function (event) {
	core.windows.tab = curses.stdscr.subwin(1, curses.stdscr.getmaxx(), 0, 0);
	core.drawTabBar();

	core.windows.buffer = curses.stdscr.subwin(curses.stdscr.getmaxy() - 3, curses.stdscr.getmaxx(), 1, 0);
	core.windows.buffer.scrollok(true);

	core.windows.status = curses.stdscr.subwin(2, curses.stdscr.getmaxx(), curses.stdscr.getmaxy() - 2, 0);
	// blank out the status line
	var blanks = "";
	for (var i = 0; i < curses.stdscr.getmaxx(); i++) {
		blanks += " ";
	}
	core.windows.status.standout();
	core.windows.status.mvaddstr(0, 0, blanks);
	core.windows.status.standend();
	core.drawStatus();
	core.drawStatusRight();
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

	core.windows.buffer.attron(colors.getColorPair(curses.COLOR_BLUE, -1));
	for (; i < maxy; i++) {
		core.windows.buffer.mvaddstr(i, 0, "~");
	}
	core.windows.buffer.attroff(colors.getColorPair(curses.COLOR_BLUE, -1));
});

// set up the refresh on the clock
world.addEventListener("load", function (event) {

	if (core.clockShowSeconds) {
		var getNext = function () {
			var d = new Date();
			return new Date(d.getYear() + 1900, d.getMonth(), d.getDate(), d.getHours(), d.getMinutes(), d.getSeconds() + 1);
		}
	} else {
		var getNext = function () {
			var d = new Date();
			return new Date(d.getYear() + 1900, d.getMonth(), d.getDate(), d.getHours(), d.getMinutes() + 1, 0);
		}
	}

	var clockRefresh = function () {
		var next = getNext();
		core.drawStatusRight();
		core.updateAllWindows();
		setDeadline(clockRefresh, next);
	}
	clockRefresh();
});

// Move the cursor and update all windows.
world.addEventListener("load", function (event) {
	core.moveAbsolute(0, 0);
	core.updateAllWindows();
});
