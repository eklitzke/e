// Copyright Evan Klitzke, 2012
//
// This module serves to make interacting with curses color pairs simpler.

var curses = require("curses");
var next_pair = 1;
var pairs = {};

exports.getColorPair = function (foreground, background) {
	log("getting " + foreground + ", " + background);
	var val = foreground * 8 + background;
	if (val in pairs) {
		log("pair is " + pairs[val]);
		return curses.color_pair(pairs[val]);
	} else {
		pairs[val] = next_pair;
		log("init pair is " + curses.init_pair(next_pair, foreground, background));
		log("pair is " + next_pair);
		return curses.color_pair(next_pair++);
	}
};
