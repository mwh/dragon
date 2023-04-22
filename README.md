dragon - simple drag-and-drop source/sink for X or Wayland

Many programs, particularly web applications, expect files to be dragged
into them now. If you don't habitually use a file manager that is a
problem. dragon is a lightweight drag-and-drop source for X where you
can run:
  `dragon file.tar.gz`
to get a window with just that file in it, ready to be dragged where you
need it.

What if you need to drag into something? Using:
  `dragon --target`
you get a window you can drag files and text into. Dropped items are
printed to standard output.

If you're only dealing with a single file, either source or sink, use
the `--and-exit` option to terminate as soon as one drag-and-drop
operation is complete.

If you want to keep the files you drag in around for a while, use:
  `dragon --target --keep`
and each file you drop becomes its own draggable button, which you can
drag elsewhere later.

Installing
----------
Just run `make` to compile dragon and get an executable you can run
immediately or put where you like.

To install, run `make install`, which will put it into ~/.local/bin by
default. If you want a different destination, run `make
PREFIX=/path/here install` instead. To choose a different executable
name, run `make NAME=dragon-drop install`, which will generate the
executable and man page to match the given name.

dragon requires GTK+ 3 and is distributed under the GNU GPL version 3.

![demo](https://i.imgur.com/UkmrXAC.png)
- *now take that file that showed up and just drag it anywhere you want!*
