var lastline = 0;

// Get the line number of the top line in the window (zero indexed).
core.addFunction("windowTop", function () {
  return lastline - core.windows.buffer.getcury();
});

core.addFunction("windowBottom", function () {
  return core.windowTop() + core.windows.buffer.getmaxy();
});


// Get the effective y-coordinate of a given line.
core.addFunction("getY", function (linenum) {
  return linenum - core.windowTop();
});

// Low-level method to scroll a region of the screen.
//
// The rule to know the direction of scrolling is that line (n + lines) will now
// be displayed at the position formerly occupied by line n.
// @param {number} lines The number of lines to scroll
// @param {number} top The top line of the scroll region
// @param {number} bot The bottom line of the scroll region
core.addFunction("scrollRegion", function (lines, top, bot) {
  var curx = core.windows.buffer.getcurx();
  var cury = core.windows.buffer.getcury();
  var maxy = core.windows.buffer.getmaxy();
  var maxAllowed = world.buffer.length - 1;

  if (top === undefined) {
    top = 0;
  }
  if (bot === undefined) {
    bot = maxy;
  }
  log("scrollRegion(lines = " + lines + ", top = " + top + ", bot = " + bot + ")");

  var topLine = core.windowTop() + top;  // the top line that will be scrolled
  if (topLine + lines <= 0) {
    // don't allow scrolling past the top of the buffer
    lines = -topLine;
  }
  if (lines == 0) {
    return lines;
  }

  var newLine, newLinePos;
  var lineDelta = lastline - cury;
  log("line delta is " + lineDelta);
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
  core.windows.buffer.move(cury, curx);
  //core.move();
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

/**
 * This is the main interface to move the cursor. This takes of not only moving
 * the actual cursor on the screen, but also bounds checking (ensuring you can't
 * move off the end of the buffer), making sure that the screen is scrolled, and
 * that the current column/line is adjust appropriately.
 */
core.addFunction("move", function (line, col) {
  if (line === undefined) {
    line = core.line;
  }
  if (col === undefined) {
    col = core.column;
  }

  var cury = core.windows.buffer.getcury();
  var maxy = core.windows.buffer.getmaxy();
  var maxx = core.windows.buffer.getmaxx();

  // Restrict the column (to prevent scrolling off the end of the screen).
  if (col > maxx - 1) {
    col = maxx - 1;
  }

  var newy = cury + line - lastline;
  if (newy < 0) {
    // We need to scroll the screen up
    core.scrollRegion(newy);
    newy = 0;
  } else if (newy >= maxy) {
    core.scrollRegion(newy - maxy + 1);
    newy = maxy - 1;
  }
  lastline = line;
  core.windows.buffer.move(newy, col);
  curses.stdscr.move(newy + 1, col);
});
