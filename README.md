Sawyer
======

Sawyer is a library that provides functionality that's often needed
when writing other C++ libraries and tools.  It's main components are:

+ Streams for conditionally emitting diagnostics, including logic
  assertions and text-based progress bars

+ Parsers for program command lines and automatically generating
  Unix manual pages.

+ Containers, including Graph, IndexedList, IntervalSet,
  IntervalMap, Map, and BitVector.

+ Miscellaneous: Memory pool allocators, small object support,
  stopwatch-like timers, optional values.

Documentation
=============

The Saywer user manual and API reference manual are combined in a
single document.  A version of the documentation can be found
[here](http://rpm.is/sawyer), or users can run

    $ doxygen docs/doxygen.conf

and then browse to docs/html/index.html.

Installing
==========

Installation instructions are
[here](http://www.hoosierfocus.com/~matzke/sawyer/group__installation.html). In
summary:

    $ SAWYER_SRC=/my/sawyer/source/code
    $ git clone https://github.com/matzke1/sawyer $SAWYER_SRC
    $ SAWYER_BLD=/my/sawyer/build/directory
    $ mkdir $SAWYER_BLD
    $ cd $SAWYER_BLD
    $ cmake $SAWYER_SRC -DCMAKE_INSTALL_PREFIX:PATH="/some/directory"
    $ make install

One commonly also needs
"-DBOOST_ROOT=/the/boost/installation/directory", and
"-DSQLITE_ROOT=/the/sqlite/installation/directory", and
"-DLIBPQ_ROOT=/the/postgresql/installation/directory", and
"-DLIBPQXX_ROOT=/the/libpqxx/installation/directory", and
"-DCMAKE_BUILD_TYPE=Debug" is useful during development.

If you have Spock installed, you don't need to download or compile boost yourself. Just give these commands:

    $ spock-shell --with gnu-system-compilers,c++11-compiler,boost-1.63 --install=yes
    $ mkdir _build && cd _build
    $ cmake .. -DBOOST_ROOT=$BOOST_ROOT
    $ make install
    $ exit
    
