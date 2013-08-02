HTTP_PARSER_PATH = http-parser
PCRE_PATH = pcre
EV_PATH = libev

CC = gcc
CFLAGS = --std=gnu99 -ggdb -Wall -fPIC
CPPFLAGS = -I$(HTTP_PARSER_PATH)/ -I$(PCRE_PATH)/include -I$(EV_PATH)/include
LDFLAGS = -Wl,-rpath=$(PCRE_PATH)/lib/ -Wl,-rpath=$(EV_PATH)/lib/ -Wl,-rpath=.
LIBS = $(HTTP_PARSER_PATH)/http_parser.o $(PCRE_PATH)/lib/libpcre.so.1 -lm $(EV_PATH)/lib/libev.so.4
OBJS = server.o request.o routes.o templates.o handler.o route.o response.o util.o

go: martin.so main.o 
	gcc -o go $(LDFLAGS) main.o martin.so $(LIBS)

martin.so: $(OBJS)
	gcc -o martin.so -shared $(OBJS)

routes.o: routes.c

routes.c: routes.txt
	perl genroutes.pl routes.txt > routes.c

clean:
	rm -f go
	rm -f *.o
	rm -f routes.c
