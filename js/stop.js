// Copyright 2012, Evan Klitzke <evan@eklitzke.org>
//
// This is a test script for testing how fast it is to start/stop the
// editor. The way you'd use it would be to invoke the editor like:
//
//    time e -s js/stop.js
//
// Since this script will immediately stop the I/O loop the editor will exit
// immediately without waiting for any input from the user.
//
// This will get you the complete, end-to-end time of starting the binary,
// initializing V8, initializing curses, running all of the code in core.js, and
// then shutting things down.
//
// You might find it interesting to compare to
//    time vim --cmd :q
// or
//    time emacs -nw --eval '(save-buffers-kill-emacs)'

world.stopLoop();
