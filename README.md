Overview
========

This is the source code for e, a text editor. It's implemented using C++ and
JavaScript (via Google's [V8](http://code.google.com/p/v8/)).

How well does it work?
----------------------

It doesn't work very well at all. You can compile it and type stuff, but it's
not very functional. The project is not yet self-hosting (I'm editing the code
in Emacs).

Why this project? Why not use Node.js?
--------------------------------------

I think Node.js is a great project. It has I/O capabilities, and there's at
least one curses binding floating around, so there's no reason you couldn't
write the same thing using node.

The main answer is I wanted a project to expand my C++ skills, hence this
project. That said, I think there are some other advantages to a separate V8
embedding for an editor:

* After compiling, you get a real bona-fide binary; you can copy it around on
  different machines if they have the right shared libraries built.
* It's easy to statically link things, creating a stand-alone binary; this makes
  it even easier to copy around and install on different machines.
* The I/O model is a lot simpler (mostly due to doing synchronous I/O).
* You can still do asynchronous I/O if you want; as a matter of fact, behind the
  scenes e is using boost::asio to process keypresses asynchronously (to allow
  various timers to run in the background). Look at the implementation of
  `TermiosWindow::Loop` if you want to learn more about this.

What does it look like?
-----------------------

Here's a screenshot. It's not very exciting.

![](http://github.com/eklitzke/e/raw/master/static/hello_from_e.png) 

Help! How do I quit?
--------------------

Uses Ctrl-Q to exit the editor.


Javascript API
==============

The JavaScript API is somewhat (poorly) documented at `js/README.md`. The best
places in the code to look is at `src/js_curses.cc` (particularly the
implementation of `GetCallbacks()`), and to a lesser degree in `src/state.cc`.

Code Layout
===========

C++ code all lives in the `src` directory. From time to time I process the
source files there with `third_party/cpplint.py` and ensure that they pass all
of the checks (even when it's annoying to do so).

JavaScript code (well, ECMAScript really) lives in the `js/` directory.

Third party code, regardless of language or origin, is in the `third_party/`
directory. This also inclused some third party "data", such as the terminfo
capabilities file.

Building
========

Read on to get the appropriate instructions for your operating system.

Building on Linux
-----------------

You'll need [GYP](http://code.google.com/p/gyp/) installed. You ought to then be
able to run something like

    gyp --depth=src/ e.gyp

and get a `Makefile`. Then use `make` as usual. If you have build problems, you
might want to try invoking `make` as

    make V=1

which sets verbose build output.

You'll probably also need at least the following libraries:

* http://www.boost.org/ (any recent-ish version)
* http://code.google.com/p/google-glog/ (you might need an older version)
* http://code.google.com/p/v8/ (built as a shared library, `v8.so`)
* http://www.gnu.org/software/ncurses/ (you probably already have this)

There might be more or fewer libraries as the code changes; check out `e.gyp` to
see what shared libraries it's trying to link in.

Building on Mac OS X
--------------------

This should be pretty similar to building on Linux, except that before it will
work someone needs to modify `e.gyp`. It's probably just a matter of instructing
GYP to correctly link things.

Building on Windows
-------------------

This is unlikely to work. There are a fair number of Unix-isms in the code
(although nothing too nasty, they should be fixable by using various Boost
interfaces).

Probably the more difficult thing about this would be getting a working curses
implementation. There seem to be a couple out there but who knows how well they
work.
