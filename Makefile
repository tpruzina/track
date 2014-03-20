LDFLAGS=-lssl -lcrypto -lsqlite3
CFLAGS=-O2 -D_GNU_SOURCE -std=gnu99 
DEBUGFLAGS=-D_DEBUG -Wall -Wextra -g3 -ggdb3

SOURCES=common.c main.c database/sqlite.c database/database.c md5/md5.c file/file.c

ALL: all
all: main

main: $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o track

debug: $(SOURCES)
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(LDFLAGS) $(SOURCES) -o track

clean:
	rm -f track
