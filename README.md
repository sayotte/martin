martin
======
An HTTP server that can be extended with small plugins, which can be invoked via simple regex-based routing rules.

Martin is now dead code. It was a pet project that I used to learn more deeply: 

  * HTTP and the http-parser library
  * the reactor pattern and libevent/libev
  * dynamic-loading of libs at runtime
  * autoconf 
  * libpcre
  * recovering from segfaults
  * sub-process control (in the early multi-process model)
  * SysV IPC (in the early multi-process model)
  * ... and some other ambitions I didn't get around to:
    * embedding a Perl or Python interpreter
    * interacting with Protocol Buffers in C
    * incorporating a small (as opposed to bloaty, like JSON or YAML) config-file language

After working on it in my spare time for a few months I learned some things that made further development without a top-down rewrite seem... a bad choice, technically. The project was really for learning to begin with, and a rewrite sounded very boring, so I've put it down for now / likely forever.

Some of the valuable things I learned along the way were these:

   * Writing DSLs in C that can be compiled without a first-pass by something else is very awkward
      * Apple's anonymous function extension can help, but is limited to Apple of course 
         * See https://github.com/tyler/Bogart for an effort that uses this feature to MUCH more closely mimic Sinatra's ease-of-use
      * Preprocessor abuse coupled with exploiting ELF or Mach file formats can get you there perhaps more portably, but is of course more brittle
   * Catching exceptions (even segfaults!) in C is easy, but handling them correctly in a callback-based architecture is hard in every language
      * In C, the possibility of a corrupted heap makes handling exceptions reasonably every time (the service remains stable) harder than most languages
      * Writing a service in C which uses a callback-based architecture, and which can isolate faults that might corrupt the heap, is Very Hard
   * Libtool is useful for handling plugins at runtime in a portable way
   * libevent has a more functionality than libev
      * For example, its "ev_http" sub-library obviates most of the code in Martin! (haha)

If you still want to take a look... 

   * main() lives in main.c, so that the code can be used as a library; most of the top-level action is in server.c
   * martin uses http-parser (https://github.com/joyent/http-parser) to parse requests; this is a very cool library
   * martin uses PCRE (http://www.pcre.org/) to match and route requests to appropriate handlers
   * martin uses libev (http://software.schmorp.de/pkg/libev) for its event-loop
   * routes are loaded at run-time from the file "routes.txt"
      * each line has an HTTP method, a regex path to match against, and the name of a handler routine (e.g. "GET /static get_static")
      * handler routines are found at runtime using dlsym() so they can be anything you link in
      * some handler examples are found in builtin.c and the plugins/ sub-directory
   * The code builds on both Linux and Mac as of this writing; the unit tests in test/ pass, minimal as they are
