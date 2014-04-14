LDFLAGS=-lssl -lcrypto -lsqlite3
CFLAGS=-O2 -D_GNU_SOURCE -std=gnu99 
DEBUGFLAGS=-std=gnu99 -Og -D_GNU_SOURCE -D_DEBUG -Wall -Wextra -g3 -ggdb3

SOURCES=common.c main.c track.c database/sqlite.c md5/md5.c file/file.c

ALL: all
all: main

main: $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o track

debug: $(SOURCES)
	$(CC) $(DEBUGFLAGS) $(LDFLAGS) $(SOURCES) -o track

clean:
	rm -f track
