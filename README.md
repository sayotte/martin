martin
======

A RESTful server framework in C, inspired by Sinatra.

Using martin is a bad idea; if you find yourself writing web services in C, you should probably ask yourself why. A server written in C will probably be faster, yes, but if speed is your goal you might want to look at Protocol Buffers or Thrift rather than REST. At the same time, a server written in C will be vulnerable to many classic mistakes that result in instability and security problems that are categorically absent from modern interpreted languages (buffer overflows, string formatting problems, segfaults rather than meaningful exceptions, so on).

Then again, maybe it's a really good idea-- if you already have a codebase written in C, re-using the code within martin to give it a RESTful API would save a lot of trouble.

Martin's goal is to insulate the developer from the hassle of writing an HTTP server, allowing them to focus on the API for their application.

If you still want to take a look... well, I haven't written any documentation yet, but here are some points to get started on:
   * main() lives in server.c
   * martin uses http-parser (https://github.com/joyent/http-parser) to parse requests
   * martin uses PCRE (http://www.pcre.org/) to match and route requests to appropriate handlers
   * martin will soon be using libev (http://software.schmorp.de/pkg/libev) for its event-loop

