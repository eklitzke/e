// Low-level method to scroll a region of the screen.
//
// The rule to know the direction of scorlling is that line (n + lines) will now
// be displayed at the position formerly occupied by line n.
// @param {number} lines The number of lines to scroll
// @param {number} top The top line of the scroll region
// @param {number} bot The bottom line of the scroll region
core.addFunction("scrollRegion", function (lines, top, bot) {
  var curx = core.windows.buffer.getcurx();
  var cury = core.windows.buffer.getcury();
  var maxy = core.windows.buffer.getmaxy();
  var maxAllowed = world.buffer.length - 1;

  var wt = core.windowTop();  // the top line currently displayed

  var topLine = core.windowTop() + top;  // the top line that will be scrolled
  if (topLine + lines <= 0) {
    // don't allow scrolling past the top of the buffer
    lines = -topLine;
  }
  if (lines == 0) {
    return lines;
  }

  var newLine, newLinePos;
  var lineDelta = core.line - cury;
  var tildePair = colors.getColorPair(curses.COLOR_BLUE, -1);
  for (var i = top; i <= bot; i++) {
    newLinePos = i + lineDelta + lines;
    if (newLinePos > maxAllowed) {
      core.windows.buffer.attron(tildePair);
      core.windows.buffer.mvaddstr(i, 0, "~");
      core.windows.buffer.clrtoeol();
      core.windows.buffer.attroff(tildePair);
    } else {
      newLine = world.buffer.getLine(i + lineDelta + lines).value();
      core.windows.buffer.mvaddstr(i, 0, newLine);
      core.windows.buffer.clrtoeol();
    }
  }
  core.move.absolute(cury, curx);
  return lines;
});

// higher-level method to scroll a region
//
// partition -- the line to partition on; always included in scrolling
// tophalf -- if true, top half is scrolled down, if false, bottom half is
//            scrolled up
// lines -- the number of lines to scroll by, always positive
core.addFunction("scroll", function (partition, tophalf, lines) {
  var top, bot;
  if (tophalf) {
    top = 0;
    bot = partition;
  } else {
    top = partition;
    bot = core.windows.buffer.getmaxy();
  }
  core.scrollRegion(top, bot, lines);
});

// move to an absolute window position (specified relative to the
// core.windows.buffer window)
core.addFunction("moveAbsolute", function (y, x) {
  core.windows.buffer.move(y, x);
  curses.stdscr.move(y + 1, x);
});

/**
 * This is the main interface to move the cursor. This takes of not only moving
 * the actual cursor on the screen, but also bounds checking (ensuring you can't
 * move off the end of the buffer), making sure that the screen is scrolled, and
 * that the current column/line is adjust appropriately.
 *
 * It is expected that this will be the main interface that almost all functions
 * that want to do movement commands will do. There should be almost no reason
 * for user-supplied JavaScript to be manually scrolling the screen, moving the
 * curses cursor, or using any of the other movement commands.
 *
 * @param {number} down The number of lines to scroll odwn; positive down means
 *                      moving DOWN, negative down means moving UP.
 * @param {number} [over] The number of columns to scroll horizontally. Positive
 *                        over means moving to the right, negative over means
 *                        moving to the left.
 * @param {bool} [restrictRight] If not explicitly set to false, we'll restrict
 *                               the cursor from scrolling past the right edge
 *                               of the new line.
 */
core.addFunction("move", function (down, over, restrictRight) {
  down = parseInt(down || 0);
  over = parseInt(over || 0);
  restrictRight = core.toBool(restrictRight, true);
  var cury = core.windows.buffer.getcury();
  var newy = cury, newx;

  if (down) {
    var newLine = core.line + down;

    // don't allow moving the cursor past the end of the buffer
    if (newLine >= world.buffer.length) {
      newLine = world.buffer.length - 1;
    } else if (newLine < 0) {
      newLine = 0;
    }

    // Compute the actual delta, after restricting scrolling past the end of
    // the buffer. Don't update core.line until after we've called
    // core.scrollRegion.
    var actualDelta = newLine - core.line;
    if (actualDelta != 0) {
      var maxy = core.windows.buffer.getmaxy();
      var targety = cury + actualDelta;
      if (targety < 0) {
        // we tried to move too far up, need to scroll the screen DOWN
        core.scrollRegion(targety, 0, maxy);
        core.line = newLine;
        newy = 0;
      } else if (targety >= maxy) {
        // we tried to move too far down, need to scroll the screen UP
        core.scrollRegion(targety - maxy + 1, 0, maxy);
        core.line = newLine;
        newy = maxy;
      } else {
        // just move the cursor
        core.line = newLine;
        newy = cury + actualDelta;
      }
    }
  }

  newx = core.restrictX(core.column + over);
  core.column = newx;
  var maxx = core.windows.buffer.getmaxx();
  core.move.absolute(newy, maxx < newx ? maxx : newx);
});

core.move.absolute = core.moveAbsolute;

core.move.left = function (updateBuffer) {
  updateBuffer = core.toBool(updateBuffer, true);
  var cury = core.windows.buffer.getcury();
  core.move.absolute(cury, 0);
  if (updateBuffer === true) {
    core.column = 0;
  }
};

core.move.right = function (pastText, updateBuffer, extraDelta) {
  pastText = core.toBool(pastText, false);
  updateBuffer = core.toBool(updateBuffer, true);
  extraDelta = extraDelta || 0;
  var cury = core.windows.buffer.getcury();
  var newx;
  var maxx = core.windows.buffer.getmaxx();
  if (pastText) {
    newx = maxx;
  } else {
    var desired = core.currentLine().length + extraDelta;
    newx = desired < maxx ? desired : maxx;
    if (newx < 0) {
      newx = 0;
    }
  }
  core.move.absolute(cury, newx);
  if (updateBuffer === true) {
    core.column = newx;
  }
};

core.addFunction("restrictX", function (newx, lineNum) {
  var line;
  if (lineNum === undefined) {
    lineNum = core.line;
  }
  line = world.buffer.getLine(lineNum);
  if (newx < 0) {
    newx = 0;
  } else if (newx > line.length) {
    newx = line.length;
  }
  var maxx = core.windows.buffer.getmaxx();
  if (newx > maxx) {
    newx = maxx;
  }
  core.column = newx;
  return newx;
});
