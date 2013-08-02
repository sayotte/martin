HTTP_PARSER_PATH = http-parser
PCRE_PATH = pcre
EV_PATH = libev

CC = gcc
CFLAGS = --std=gnu99 -ggdb -Wall
CPPFLAGS = -I$(HTTP_PARSER_PATH)/ -I$(PCRE_PATH)/include -I$(EV_PATH)/include
LDFLAGS = -Wl,-rpath=$(PCRE_PATH)/lib/ -Wl,-rpath=$(EV_PATH)/lib/
LIBS = $(HTTP_PARSER_PATH)/http_parser.o $(PCRE_PATH)/lib/libpcre.so.1 -lm $(EV_PATH)/lib/libev.so.4
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
