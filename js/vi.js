// Implementation of vi-mode.

function ExFlags() {
  this.flags = '';
}

ExFlags.prototype.setFlag = function (flag) {
  if (this.flags.indexOf(flag) === -1) {
    this.flags += flag;
  }
};

ExFlags.prototype.clearFlag = function (flag) {
  var pos = this.flags.indexOf(flag);
  if (pos) {
    this.flags.splice(pos, 1);
  }
}

ExFlags.prototype.check = function (flag) {
  return (this.flags.indexOf(flag) !== -1);
}

var exFlags = new ExFlags();

// The accumulator is what makes commands like 5j or 3dd work. It accumulates
// digits entered, and runs commands repeatedly. There will only be one instance
// of the Accumulator class created in normal operation, the variable
// `accumulator'.
function Accumulator() {
  this.count = 0;
}

// Add a digit to the accumulator.
Accumulator.prototype.addDigit = function (digit) {
  if (this.count === 0) {
    this.count = digit;
  } else {
    this.count = this.count * 10 + digit;
  }
};

// Run a function the appropriate number of times (which is once if the
// accumulator is empty).
Accumulator.prototype.run = function (func) {
  var count = this.count || 1;
  for (var i = 0; i < count; i++) {
    func();
  }
  this.count = 0;
}

// The global instance of the accumulator.
var accumulator = new Accumulator();

var pasteBuffer = '';

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

  var isWhitespace = function (c) {
    var val;
    switch (c) {
    case ' ':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
      val = true;
      break;
    default:
      val = false;
      break;
    }
    return val;
  };

  var motion = null;
  switch (wch) {
  case 'a':
    core.move(0, 1);
    core.switchMode("insert");
    break;
  case 'A':
    core.move.right();
    core.switchMode("insert");
    break;
  case 'b':
  case 'B':
    // XXX: this isn't quite right for a couple of reasons. One is that we need
    // to be able to move up a line if we reach the beginning of a line. The
    // other is that 'b' and 'B' are actually slightly different (this behaves
    // like B not b).
    core.warningText.set("'" + wch + "' not properly implemented");
    motion = function () {
      var line = core.currentLine().value();
      var col = core.column - 1;
      while (col > 0) {
        col--;
        if (isWhitespace(line[col])) {
          col++;
          break;
        }
      }
      core.move(0, col - core.column);
    };
    break;
  case 'd':
    exFlags.setFlag('d');
    core.warningText.set("'d' not properly implemented");
    break;
  case 'h':
    motion = function () { core.move(0, -1); };
    break;
  case 'j':
    motion = function () { core.move(1); };
    break;
  case 'k':
    motion = function () { core.move(-1); };
    break;
  case 'l':
    motion = function () { core.move(0, 1); };
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
  case 'y':
    exFlags.setFlag('y');
    core.warningText.set("'y' not properly implemented");
    break;
  case '^':
    motion = core.move.left;
    break;
  case '0':
    if (accumulator.count !== 0) {
      accumulator.addDigit(0);
    } else {
      motion = core.move.left;
    }
    break;
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    accumulator.addDigit(parseInt(wch));
    break;
  case '$':
    motion = function () {core.move.right(false, true, -1) };
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
    core.errorText.set("vi command not implemented: " + wchName);
  }

  if (motion !== null) {
    accumulator.run(motion);
  }
});
