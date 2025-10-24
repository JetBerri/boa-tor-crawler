CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -I/usr/include/postgresql
LIBS = -lcurl -lyaml -lpq

SRCS = src/crawler.c src/log.c src/main.c src/parser.c src/verify.c src/db.c
OBJS = $(SRCS:.c=.o)

all: tor-crawler

tor-crawler: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o tor-crawler $(LIBS)

clean:
	rm -f tor-crawler *.o
