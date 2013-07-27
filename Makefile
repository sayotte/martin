CC = gcc
CFLAGS = --std=gnu99 -ggdb -Wall -ansi
CPPFLAGS = -Ihttp-parser/ -Ipcre/include -D_GNU_SOURCE
LIBS = http-parser/http_parser.o pcre/lib/libpcre.a -lpthread
OBJS = server.o request.o routes.o templates.o handler.o route.o response.o util.o main.o

go: $(OBJS)
	gcc -o go $(OBJS) $(LIBS)

routes.o: routes.c

routes.c: routes.txt
	perl genroutes.pl routes.txt > routes.c

clean:
	rm -f go
	rm -f *.o
	rm -f routes.c
