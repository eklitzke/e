// Copyright Evan Klitzke, 2012
//
// This module serves to make interacting with curses color pairs simpler.

var curses = require("curses");
var next_pair = 1;
var pairs = {};

exports.getColorPair = function (foreground, background) {
	var val = foreground * 8 + background;
	if (val in pairs) {
		return curses.color_pair(pairs[val]);
	} else {
		pairs[val] = next_pair;
		curses.init_pair(next_pair, foreground, background);
		return curses.color_pair(next_pair++);
	}
};
