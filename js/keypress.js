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
			if (!world.buffer.persist(world.buffer.getFile())) {
				core.setError("failed to save file!");
			}
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

	// we're in ex-mode
	if (core.exBuffer.length) {
		log("exBuffer is \"" + core.exBuffer + "\"");
		if (code == 13) {
			switch (core.exBuffer) {
			case ":q":
			case ":qa":
			case ":q!":
			case ":qa!":
				world.stopLoop();
				break;
			}
			core.exBuffer = "";
		} else {
			core.exBuffer += wch;
		}
		return;
	}

	// for 'o' and 'O'
	var insertLine = function (lineDelta) {
		world.buffer.addLine(core.line + lineDelta, "");
		core.scrollRegion(-lineDelta, core.windows.buffer.getcury(), core.windows.buffer.getmaxy());
		core.move(lineDelta);
		core.move.left();
		core.windows.buffer.clrtoeol();
		core.switchMode("insert");
	};

	switch (wch) {
	case 'a':
		core.move(0, 1);
		core.switchMode("insert");
		break;
	case 'A':
		core.move.right();
		core.switchMode("insert");
		break;
	case 'c':
	case 'C':
		core.switchMode("insert");
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
		core.switchMode("insert");
		break;
	case 'I':
		core.move.left();
		core.switchMode("insert");
		break;
	case 'o':
		insertLine(1);
		break;
	case 'O':
		insertLine(0);
		break;
	case 's':
	case 'S':
		core.switchMode("insert");
		break;
	case '^':
	case '0':
		core.move.left();
		break;
	case '$':
		core.move.right(false, true, -1);
		break;
	case ':':
		core.exBuffer = ":";
		break;
	default:
		var wchName;
		if (event.isPrintable()) {
			wchName = "'" + wch + "'";
		} else {
			wchName = "keycode " + event.getCode();
		}
		log("didn't know how to handle " + wchName + " in command mode");
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
	core.clearError();

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
			core.switchMode("command");
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
