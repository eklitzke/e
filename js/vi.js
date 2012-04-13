// Implementation of vi-mode.

// The main keypress listener for insert mode.
core.addKeypressListener("insert", function (event) {
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
      // chop the end of the line, so that it can be added to the following line
      chopped = line.value().substring(core.column, line.length);
      line.chop(core.column);
      core.windows.buffer.clrtoeol();
    }
    // scroll the region below the cursor one line down
    core.scrollRegion(-1, core.windows.buffer.getcury() + 1, core.windows.buffer.getmaxy());
    world.buffer.addLine(core.line + 1, chopped);  // add the new line, with the chopped contents

    core.move(1);  // this implicitly updates core.line
    core.move.left();
    if (chopped) {
      core.windows.buffer.addstr(chopped);
      core.windows.buffer.clrtoeol();
      core.move.left();
    } else {
      core.windows.buffer.clrtoeol();
    }
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
