var vi = require('js/vi.js');

function HighlightText() {
  this.text = ''
};

HighlightText.prototype.clear = function () {
  this.text = '';
};

HighlightText.prototype.set = function (text) {
  this.text = text;
};

HighlightText.prototype.value = function () {
  return this.text;
};

core.errorText = new HighlightText();
core.warningText = new HighlightText();
core.notificationText = new HighlightText();

core.addFunction("computeStatusSplits", function (index) {
  var maxx = core.windows.tab.getmaxx();
  var third = parseInt(maxx / 3);
  if (index === 0) {
    return {'left': 0, 'right': third};
  } else if (index === 1) {
    return {'left': third, 'right': 2 * third};
  } else {
    return {'left': 2 * third, 'right': maxx};
  }
});

core.addFunction("drawStatus", function () {
  var maxx = core.windows.status.getmaxx();
  core.windows.status.standout();
  core.windows.status.mvaddstr(0, 0, "  ");
  var split = core.computeStatusSplits(0);
  var tabStr = "" + (core.line + 1) + "," + core.column;
  var ratio = core.windowBottom() * 100 / world.buffer.length;
  if (ratio > 100) {
    ratio = 100;
  }
  ratio = parseInt(ratio);
  tabStr += "  (" + ratio + "%)";

  core.windows.status.addstr(tabStr);

  var spacesNeeded = split.right - split.left - tabStr.length;
  var spaces = "";
  for (var i = 0; i < spacesNeeded; i++) {
    spaces += " ";
  }
  core.windows.status.addstr(spaces);
  core.windows.status.standend();

  // resetCursor is true if we ultimately need to reset the cursor, or false
  // if we don't want to reset it (i.e. to keep it in the status area)
  var resetCursor = true;

  // draw the exBuffer (: commands in vi-mode)
  core.windows.status.move(1, 0);

  // draw the bottom line; this wrapper helps deal with correctly adding padding
  var drawn = false;
  var drawBottom = function (bottom, leaveCursor) {
    drawn = true;
    var len = bottom.length;
    while (bottom.length < maxx - 10) {
      bottom += " ";
    }
    core.windows.status.mvaddstr(1, 0, bottom);
    if (leaveCursor) {
      resetCursor = false;
      curses.move(curses.stdscr.getmaxy() - 1, bottom.length);
    }
  };

  var bottomline = '';
  core.windows.status.clrtoeol();
  if (core.curmode == 'ex') {
    drawBottom(':' + core.exBuffer, true);
  }

  var drawStatusLine = function (fg, bg, text) {
    core.windows.status.attron(curses.A_BOLD);
    var colorPair = colors.getColorPair(fg, bg);
    drawBottom(text);
    core.windows.status.attron(colorPair);
    core.windows.status.mvaddstr(1, 0, text);
    core.windows.status.attroff(colorPair);
    core.windows.status.attroff(curses.A_BOLD);
  };

  if (core.errorText.value()) {
    drawStatusLine(curses.COLOR_WHITE, curses.COLOR_RED, "ERROR: " +
                   core.errorText.value());
  } else if (core.warningText.value()) {
    drawStatusLine(curses.COLOR_RED, -1, "WARNING: " + core.warningText.value());
  } else if (core.notificationText.value()) {
    drawStatusLine(curses.COLOR_GREEN, -1, core.notificationText.value());
  } else if (core.curmode == "insert") {
    drawStatusLine(curses.COLOR_YELLOW, -1, "-- INSERT --");
  } else if (drawn === false) {
    drawBottom('');
  }
  core.windows.status.addstr(vi.pendingCommand);

  if (resetCursor) {
    // move the cursor back to the main editing buffer
    core.move()
  }
});

core.addFunction("drawStatusRight", function () {
  var split = core.computeStatusSplits(2);
  var fmtTime = function (n) {
    if (n < 10) {
      return new String("0" + n);
    } else {
      return new String(n);
    }
  };
  var d = new Date();
  var statusEnd = "";
  if (core.clockMode == "12") {
    var h = d.getHours();
    var modifier = h < 12 ? "AM" : "PM";
    if (h === 0) {
      h = "12";
    } else if (h > 12) {
      h = new String(h - 12);
    } else {
      h = new String(h);
    }
    statusEnd += h + ":" + fmtTime(d.getMinutes());
    if (core.clockShowSeconds) {
      statusEnd += ":" + fmtTime(d.getSeconds());
    }
    statusEnd += " " + modifier + " ";
  } else {
    statusEnd += fmtTime(d.getHours()) + ":" + fmtTime(d.getMinutes());
    if (core.clockShowSeconds) {
      statusEnd += ":" + fmtTime(d.getSeconds());
    }
    statusEnd += " ";
  }

  while (statusEnd.length < split.right - split.left) {
    statusEnd = " " + statusEnd;
  }
  core.windows.status.standout();
  core.windows.status.mvaddstr(0, split.left, statusEnd);
  core.windows.status.standend();
});
