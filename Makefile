HTTP_PARSER_PATH = http-parser
PCRE_PATH = pcre

CC = gcc
CFLAGS = --std=gnu99 -ggdb -Wall -ansi
CPPFLAGS = -I$(HTTP_PARSER_PATH)/ -I$(PCRE_PATH)/include -D_GNU_SOURCE
LDFLAGS = -Wl,-rpath=$(PCRE_PATH)/lib/
LIBS = $(HTTP_PARSER_PATH)/http_parser.o $(PCRE_PATH)/lib/libpcre.so.1 -lpthread
OBJS = server.o request.o routes.o templates.o handler.o route.o response.o util.o main.o

go: $(OBJS)
	gcc -o go $(LDFLAGS) $(OBJS) $(LIBS)

routes.o: routes.c

routes.c: routes.txt
	perl genroutes.pl routes.txt > routes.c

clean:
	rm -f go
	rm -f *.o
	rm -f routes.c
