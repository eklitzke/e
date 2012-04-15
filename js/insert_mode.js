// The main keypress listener for insert mode.
core.addKeypressListener("insert", function (event) {
  var curx = core.windows.buffer.getcurx();
  var cury = core.windows.buffer.getcury();
  var code = event.getCode();
  if (!event.isKeypad()) {
  var wch = event.getChar();
  switch (code) {
  case 1: // Ctrl-A
    core.column = 0;
    break;
  case 3: // Ctrl-C
    // this should have been handled already
    break;
  case 5: // Ctrl-E
    core.column = core.currentLine().length;
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
    core.scrollRegion(-1, core.windows.buffer.getcury() + 2);
    world.buffer.addLine(++core.line, chopped);  // add the new line, with the chopped contents
    core.column = 0;

    // need to move before drawing the new line
    core.move();

    if (chopped) {
      core.windows.buffer.addstr(chopped);
      core.windows.buffer.clrtoeol();
    } else {
      core.windows.buffer.clrtoeol();
    }
    break;
  case 19: // Ctrl-S
    if (!world.buffer.persist(world.buffer.getFile())) {
      core.errorText.set("failed to save file!");
    }
  case 26: // Ctrl-Z
    sys.kill(sys.getpid(), sys.SIGTSTP);
    break;
  default:
    var curline = world.buffer.getLine(core.line);
    curline.insert(core.column, wch);
    if (core.column < curline.length - 1) {
      core.windows.buffer.insch(wch);
    } else {
      core.windows.buffer.addstr(wch);
    }
    core.column += wch.length;
    break;
  }
  } else {
    var name = event.getName();
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
        core.line++;
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
