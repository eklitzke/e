// Copyright 2012, Evan Klitzke <evan@eklitzke.org>

var curses = require("curses");
var errno = require("errno");
var signal = require("signal");
var sys = require("sys");

var EventListener = require("js/event_listener.js").EventListener;
var colors = require("js/colors.js");

// You can easily override most of these attributes in your ~/.e.js file (e.g.
// to change the clock mode or refresh rate).
var core = {
	column: 0,
	line: 0,
	clockMode: "12", // 12 or 24
	clockShowSeconds: false,
	clockRefresh: null, // delay loading this until after ~/.e.js has been parsed
	exBuffer: '', // the buffer for : commands in vi-mode
	inEscape: false, // true when part of an escape sequence
	viMode: true,
	logContents: false, // when true, log the file contents after each keypress
	curmode: "command",
	parser: require("js/parser.js").parser,
	listeners: {},
	windows: {},
};

/**
 * Adds a function to the core object, and adds a displayName attribute.
 *
 * @param {string} name The name of the function
 * @param {function} func The function object
 */
core.addFunction = function (name, func) {
	func.displayName = name;
	core[name] = func;
	return func;
};
core.addFunction.displayName = "addFunction";

/**
 * Gets the current line in the buffer.
 *
 * @param {number} [line] the "current" line
 */
core.addFunction("currentLine", function (line) {
	if (line === undefined) {
		line = core.line;
	}
	return world.buffer.getLine(core.line);
});

/**
 * Converts a value to a boolean, optionally with a default value for undefined
 * values.
 *
 * @param val The current value
 * @param {boolean} [defaultValue] the default value, if val is undefined
 */
core.addFunction("toBool", function (val, defaultValue) {
	if (val === undefined) {
		val = defaultValue;
	}
	return !!val;
});

/**
 * Get the line number of the top line in the window (zero indexed).
 */
core.addFunction("windowTop", function () {
	return core.line - core.windows.buffer.getcury();
});

/**
 * Get the line number of the bottom line in the window (zero indexed).
 */
core.addFunction("windowBottom", function () {
	return core.windowTop() + core.windows.buffer.getmaxy();
});

/**
 * Attempt to log a traceback.
 */
core.addFunction("logTrace", function () {
	var currentFunction = arguments.callee.caller;
	log("STACK TRACE");
	log("===========");
	while (currentFunction) {
		var fname = currentFunction.displayName;
		if (!fname) {
			var fn = currentFunction.toString();
			fname = fn.substring(fn.indexOf("function") + 8, fn.indexOf("")) || "anonymous";
		}
		log(fname + "()");
		currentFunction = currentFunction.caller;
	}
});

core.addFunction("switchMode", function (newMode) {
	core.curmode = newMode;
	core.drawStatus();
});

core.addFunction("updateAllWindows", function (doupdate) {
	if (doupdate === undefined) {
		doupdate = true;
	}
	var w;
	for (w in core.windows) {
		if (core.windows.hasOwnProperty(w) && core.windows[w].noutrefresh) {
			// update each stdscr subwindow
			core.windows[w].noutrefresh();
		}
	}
	// update stdscr
	curses.stdscr.noutrefresh();

	// do the update
	curses.doupdate();
});

require("js/keypress.js");  // for side-effects
require("js/movement.js");  // for side-effects
require("js/statusbar.js"); // for side-effects
require("js/tabbar.js"); // for side-effects

// has to be last
require("js/onload.js");    // for side-effects
