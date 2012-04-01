// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// An implementation of setInterval()

exports.setInterval = function (func, millis) {
	var inner = function () {
		func();
		setTimeout(inner, millis);
	};
	setTimeout(inner, millis);
};
