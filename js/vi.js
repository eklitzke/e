// Implementation of vi-mode.

var curses = require("curses");

// this is the pending command that will be shown in the statusbar
exports.pendingCommand = '';

function ExFlags() {
  this.flags = '';
}

ExFlags.prototype.setFlag = function (flag) {
  if (this.flags.indexOf(flag) === -1) {
    this.flags += flag;
  }
};

ExFlags.prototype.clear = function () {
  this.flags = '';
}

ExFlags.prototype.check = function (flags) {
  for (var i = 0; i < flags.length; i++) {
    if (this.flags.indexOf(flags[i]) !== - 1) {
      return true;
    }
  }
  return false;
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
Accumulator.prototype.run = function (func, data) {
  var count = this.count || 1;
  for (var i = 0; i < count; i++) {
    func(data);
  }
  this.count = 0;
}

// The global instance of the accumulator.
var accumulator = new Accumulator();

var pasteBuffer = '';

var handlerMap = {};
var incompatibilityMap = {};

function addHandler(code) {
  if (arguments.length == 2) {
    var callback = arguments[1];
    for (var i = 0; i < code.length; i++) {
      handlerMap[code[i]] = callback;
    }
  } else {
    var incompatibleFlags = arguments[1];
    var callback = arguments[2];
    incompatibilityMap[code] = incompatibleFlags;
    for (var i = 0; i < code.length; i++) {
      handlerMap[code[i]] = callback;
    }
  }
}

addHandler('a', 'cd', function (line) {
  if (core.column < line.length) {
    core.column++;
    core.move();
  }
  core.switchMode('insert');
});

addHandler('A', 'cd', function (line) {
  core.column = line.length;
  core.move();
  core.switchMode('insert');
});

addHandler('d', 'c', function () {
  if (exFlags.check('d')) {
    world.buffer.deleteLine(core.line);
    core.column = 0;
    if (!world.buffer.length) {
      world.buffer.addLine(0, '');
    }
    if (core.line >= world.buffer.length) {
      core.line--;
    }
    exFlags.clear();
    var cury = core.windows.buffer.getcury();
    core.windows.buffer.move(cury, 0);
    core.windows.buffer.clrtoeol();
    core.scrollRegion(1, cury);
  } else {
    exFlags.setFlag('d');
    return true;
  }
});

addHandler(['h', curses.keycodes['KEY_LEFT']], function () {
  if (core.column > 0) {
    core.column--;
  }
});

addHandler('i', 'cd', function () {
  core.switchMode('insert');
});


addHandler(['j', curses.keycodes['KEY_DOWN']], function () {
  if (core.line < world.buffer.length - 1) {
    core.line++;
  }
});

addHandler(['k', curses.keycodes['KEY_UP']], function () {
  if (core.line > 0) {
    core.line--;
  }
});

addHandler(['l', curses.keycodes['KEY_RIGHT']], function (line) {
  if (core.column < line.length - 1) {
    core.column++;
  }
});

addHandler('o', 'cd', function () {
  world.buffer.addLine(core.line + 1)
  core.redrawBuffer();
  core.line++;
  core.column = 0;
  core.switchMode('insert');
});

addHandler('O', 'cd', function () {
  world.buffer.addLine(core.line)
  core.column = 0;
  core.redrawBuffer();
  core.switchMode('insert');
});

addHandler(':', 'cd', function (line) {
  core.switchMode('ex');
});

addHandler('$', function (line) {
  core.column = line.length - 1;
});

// FIXME: need to check modes more carefully,
addHandler('0', function () {
  if (accumulator.count !== 0) {
    accumulator.addDigit(0);
    return true;
  } else {
    core.column = 0;
  }
});

// set a handler for each digit
(function () {
  for (var i = 1; i < 10; i++) {
    (function (i, digit) {
      addHandler(digit, 'cd', function () {
        accumulator.addDigit(i);
        return true;
      });
    })(i, new String(i));
  }
})();


// The main keypress listener for command mode.
core.addKeypressListener("command", function (event) {
  var ch = event.getChar();

  // get the handler function for this character
  var isMovement = true;
  var handler = handlerMap[ch];
  if (handler === undefined) {
    handler = handlerMap[event.getCode()];
  }

  if (handler === undefined) {
    // the key couldn't be handled
    var chName;
    if (event.isPrintable()) {
      chName = "'" + ch + "'";
    } else {
      chName = "keycode " + event.getCode();
    }
    core.warningText.set(chName + ' not implemented');
    exFlags.clear();
  } else {
    // Movement commands (e.g. hjkl) are compatible with all modifiers, e.g. dj
    // is valid. non-movement commands are usually not compatible with some or
    // all modifiers -- e.g. dJ is an invalid sequence. If we see any
    // incompatibilities, then the command is considered to not be a movement
    // command. Otherwise it is one.
    //
    // Note that some characters are a bit more complicated to handle. For
    // instance, 0 can be a movment, or it can be entered as part of an
    // accumulator sequence, e.g. 10j
    var line = core.currentLine();
    var incompatibilities = incompatibilityMap[ch];
    if (incompatibilities) {
      isMovement = false;
       if (exFlags.check(incompatibilities)) {
         // If an invalid sequence like dJ was entered, let the user know, and
         // clear all of the flags.
         core.warningText.set('invalid commmand "' + exFlags.flags + ch + '"');
         core.notificationText.clear();
         exFlags.clear();
         exports.pendingCommand = '';
       } else {
         var waiting = handler(line);
         if (waiting) {
           exports.pendingCommand += ch;
         } else {
           exports.pendingCommand = '';
         }
       }
    } else {
      exports.pendingCommand = '';
      var change = false;
      var origCol = core.column;
      var origLine = core.line;
      accumulator.run(handler, line);
      if (exFlags.check('c')) {
        exFlags.clear();
        exFlags.setFlag('d');
        change = true;
      }
      if (exFlags.check('d')) {
        if (origLine !== core.line) {
          // we need to delete all of the affected lines
          var smaller = origLine < core.line ? origLine : core.line;
          var toDelete = 1 + Math.abs(core.line - origLine);
          log("deleting " + toDelete + " lines");
          if (origLine < core.line) {
            // dk moves the line, but dj doesn't. weird, right?
            core.line = origLine;
          }
          core.glitch("about to scroll");
          core.scrollRegion(toDelete, core.getY(smaller));
          core.glitch("done with scroll");
          for (var i = 0; i < toDelete; i++) {
            world.buffer.deleteLine(smaller);
          }
        } else {
          log("well shucks");
        }
        exFlags.clear()
      }
      core.notificationText.clear();
      if (change) {
        core.switchMode('insert');
      }
    }
  }
});

require("js/ex.js");
