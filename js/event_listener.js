// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// An implementation of the event listener interface.

var addListener = function (callbacks, name, callback) {
	callbacks[name] = callbacks[name] || [];
	callbacks[name].push(callback);
};

var removeListener = function (callbacks, name, callback) {
	if (name in callbacks) {
		for (var i = 0; i < callbacks.length; i++) {
			if (callbacks[i] === callback) {
				callbacks.splice(i, 1);
				i--;  // correct for the splice
			}
		}

	}
};

var dispatch = function (obj, name, args) {
	var list = obj[name] || [];
	for (var i = 0; i < list.length; i++) {
		var callback = list[i];
		if (callback.handleEvent) {
			callback.handleEvent.apply(this, args);
		} else {
			callback.apply(this, args);
		}
	}
};

var EventListener = function () {
	this.captures = {};
	this.bubbles = {};
};

// add a callback to the event listener
EventListener.prototype.addEventListener = function (name, callback, use_capture) {
	var list = use_capture ? this.captures : this.bubbles;
	addListener(list, name, callback);
};

// remove a callback from the event listener
EventListener.prototype.removeEventListener = function (name, callback, use_capture) {
	var list = use_capture ? this.captures : this.bubbles;
	removeListener(list, name, callback);
};

// dispatch an event to the event listener
EventListener.prototype.dispatch = function (eventName) {
	var event = {
		'bubbles': true,
		'cancelable': false,
		'currentTarget': eventName,
		'target': eventName,
		'timestamp': (new Date()).valueOf(),
		'type': eventName
	};
	var args = Array.prototype.slice.call(arguments)
	args = args.slice(1);

	// right now the event is pretty useless so we don't actually add it in
	if (false) {
		args.unshift(event);
	}

	dispatch(this.captures, eventName, args);
	dispatch(this.bubbles, eventName, args);
};

exports.EventListener = EventListener;
