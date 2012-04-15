// The absolute lowest level interface to keypresses is callbacks registered by
// world.addEventListener("keypress", ...). That's not really a useful
// abstraction though, because it doesn't deal with modes. The key handling for
// the character 'x' is different if you're in vi emulation command mode, if
// you're in insert mode, if you're typing in the minibuffer, etc. If you wanted
// to deal with these details directly in an event handler for raw keypresses,
// there would be a *lot* of complication caused by checking which mode you're
// in, etc. There are also complications with the keycode abstraction provided
// by TTYs (for instance, "meta" characters (e.g. holding down Alt while typing
// another character) are sent as two separate characters.
//
// For this reason, the editor is separated into differnt "modes". An example of
// a mode is "insert" for insert mode in the main buffer, "command" for
// emulating vi's command mode, etc. There can be arbitrarily many modes, and
// the current mode is set by the value of core.curmode.
//
// When a keypress happens, any handlers that have registered themself for the
// mode using core.addKeypressListener will be called.
core.addKeypressListener = function (mode, handler) {
  core.listeners[mode] = core.listeners[mode] || new EventListener();
  core.listeners[mode].addEventListener("keypress", handler);
};

// This method checks for Ctrl-C. We add it as its own top level handler to
// prevent the editor from getting "stuck" due other JavaScript errors (IOW, no
// matter what else happens you'll be able to exit using Ctrl-C).
world.addEventListener("keypress", function (event) {
  if (event.getCode() == 3) {
    log("Caught Ctrl-C, stopping the main loop");
    world.stopLoop();
  }
});

// Core routine called on each keypress; this essentially just dispatches to
// listeners registered by core.addKeypressListener (see the comments at that
// function for the details).
world.addEventListener("keypress", function (event) {
  var code = event.getCode();
  core.errorText.clear();
  core.warningText.clear();

  // Escape sequences are sent as two separate characters. For instance, if
  // you type M-x (the "Alt" key followed by the letter 'x'), the sequence
  // sent to the terminal is the two character sequence ^[ x.
  //
  // When the character ^[ is sent we set core.inEscape to true, and skip any
  // other processing. After the next character is entered and handled
  // core.inEscape will be reset to false.
  var setEscape = false;
  switch (code) {
  case 27: // ^[ a.k.a. Ctrl-[ a.k.a. escape
    if (core.viMode) {
      core.switchMode("command");
      if (core.column > 0) {
        core.column--;  // like vi
      }
    } else {
      core.inEscape = true;
    }
    setEscape = true;
    break;
  }
  if (!setEscape) {
    var listener = core.listeners[core.curmode] || new EventListener();
    listener.dispatch("keypress", event);
    core.inEscape = false;
  }

  // refresh the status bar
  core.drawStatus();

  if (core.logContents) {
    log("");
    log("<<<<<< START <<<<<<");
    var lines = world.buffer.getContents();
    for (var i = 0; i < lines.length; i++) {
      log('"' + lines[i] + '"');
    }
    log(">>>>>>> END >>>>>>>");
  }
});

// This is the callback that specifically causes curses to flush all of its
// drawing operations. It *must* be called after any functions that may do
// drawing operations, which is why it has its own special event. In general,
// it's not recommended that other functions register to listen to the
// "after_keypress" event, and if they do they must not do any drawing
// operations.
//
// It might be possible to just fold this into the main event listener for
// "keypress" events, which users should not be hooking into. But for extra
// safety, it's split out like this.
world.addEventListener("after_keypress", function (e) {
  core.updateAllWindows();
});

require("js/insert_mode.js");
require("js/vi.js");
