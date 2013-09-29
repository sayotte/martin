CC = gcc
CPPFLAGS = -I../ -I../http-parser/ -I../pcre/include/
CFLAGS = -ggdb -Wall -Wextra -Werror -std=gnu99 
VPATH = ../ ../http-parser/ ../pcre/lib/

all: request.pass message.pass route.pass
request.pass : request-test
	./request-test
request-test: test.h request.o fake-syslog.o http_parser.o message.o
message.pass: message-test
	./message-test
message-test: test.h message.o fake-syslog.o
route.pass: route-test
	./route-test
route-test: test.h route.o fake-syslog.o libpcre.a util.o
clean :
	rm -f *.o
	rm -rf *.dSYM
	rm -f *.pass
	rm -f *-test
