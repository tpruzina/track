LDFLAGS=-lssl -lcrypto -lsqlite3
CFLAGS=-O2 -D_GNU_SOURCE -std=gnu99 
DEBUGFLAGS=-D_DEBUG -Wall -Wextra -g3 -ggdb3

CLIENT_SOURCES=common.c client.c database/sqlite.c database/database.c md5/md5.c file/file.c

ALL: all
all: client

client: $(CLIENT_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CLIENT_SOURCES) -o track

debug: $(CLIENT_SOURCES)
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(LDFLAGS) $(CLIENT_SOURCES) -o track

clean:
	rm -f track
