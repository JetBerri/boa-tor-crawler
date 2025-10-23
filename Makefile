CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -lcurl -lyaml
SRC = $(wildcard src/*.c)
OUT = tor-crawler

all:
	$(CC) $(SRC) -o $(OUT) $(CFLAGS)

clean:
	rm -f $(OUT)
