Overview
========

This is the source code for e, a text editor. It's implemented using C++ and
JavaScript (via Google's [V8](http://code.google.com/p/v8/)).

How well does it work?
----------------------

It doesn't work very well at all. You can compile it and type stuff, but it's
not very functional. The project is not yet self-hosting (I'm editing the code
in Emacs).

Why JavaScript?
---------------

Love it or hate it, one of the things that makes Emacs a really, really great
editor is that it's not written in C or C++, it's written in elisp. Only the
lowest-level I/O bits are written in C, and exported to elisp code.

This makes Emacs really extensible. The author of e normally uses Emacs, and
runs it using viper-mode, a minor mode that makes Emacs behave like vi. That's
pretty crazy -- using Emacs, you can pretend like you're using vi, but also tap
into this huge millions-of-lines-of-code Emacs elisp ecosystem. The fact that
you can completely customize Emacs down to the lowest level of keyboard handling
to tack on a brand new editing system -- and have it still be fast -- shows the
power of this approach.

That said, if you've written much elisp you know it's not a very good
language. It's very antiquated, and very complicated (and I'm not just saying
that because it's a Lisp -- it's antiquated and complicated compared to pretty
much every other Lisp implementation in wide use today). Elisp also has the
problem that it doesn't have a state-of-the-art bytecode interpreter. This can
make elisp rather slow, particularly when loading new files and while garbage
collections are running.

Editors like [vim](http://www.vim.org/) feel fast and have great editing
capabilities, but are hard to customize. If you've ever tried to really dive
into [vimscript](http://vimdoc.sourceforge.net/htmldoc/usr_41.html) or its
Python bindings you'll recognize that the scripting features feel tacked on and
clunky.

JavaScript solves all (or nearly all of these problems). It's really easy (and
fun!) to use. Tons of people already know how to read, write, and debug
JavaScript. And there are a
[number](https://developer.mozilla.org/en/SpiderMonkey) of
[great](http://www.webkit.org/projects/javascript/index.html) open source
[implementations](http://trac.webkit.org/wiki/SquirrelFish). This project uses
[Google's V8](http://code.google.com/apis/v8/intro.html) implementation of
JavaScript.

The goal of e is to keep the amount of C/C++ code to a minimum, and try to
provide a super-fast ultra-customizable text editor using JavaScript. Out of the
box it will probably look and feel like a lot like [vim](http://www.vim.org/).

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

Uses Ctrl-C to exit the editor.


Javascript API
==============

Try running the editor as

    e --list-api

to look at the available JavaScript API.

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

    gyp --toplevel-dir=. --depth=src/ e.gyp --generator-output=gyp.out

and get a `Makefile` in the current directory (at the root of the source tree).
Then use `make` as usual. If you have build problems, you might want to try
invoking `make` as

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
