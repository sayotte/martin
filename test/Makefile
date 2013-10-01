CC = gcc
CPPFLAGS = -I../ -I../http-parser/ -I../pcre/include/
CFLAGS = -ggdb -Wall -Wextra -Werror -std=gnu99 
VPATH = ../ ../http-parser/ ../pcre/lib/

all: request.pass message.pass route.pass plugin.pass
request.pass : request-test
	./request-test
request-test: test.h request.o fake-syslog.o http_parser.o message.o
message.pass: message-test
	./message-test
message-test: test.h message.o fake-syslog.o
route.pass: route-test
	./route-test
route-test: test.h route.o fake-syslog.o libpcre.a util.o plugin.o
plugin.pass: plugin-test
	./plugin-test
plugin-test: test.h plugin.o libpcre.a | badplugin.so goodplugin1.so goodplugin2.so
%.so: %.c
	gcc -shared -o $@ $*.c
badplugin.so: 
	touch badplugin.so
clean :
	rm -f *.o
	rm -f *.so
	rm -rf *.dSYM
	rm -f *.pass
	rm -f *-test
