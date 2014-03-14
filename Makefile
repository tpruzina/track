LDFLAGS=-lssl -lcrypto -lsqlite3
CFLAGS=-O2 -g -D_DEBUG -D_GNU_SOURCE -std=gnu99

CLIENT_SOURCES=common.c client.c database/sqlite.c database/database.c md5/md5.c file/file.c

all: client

client: $(CLIENT_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CLIENT_SOURCES) -o track

clean:
	rm -f track
