LDFLAGS=-lssl -lcrypto -lsqlite3
CFLAGS=-O2 -g -D_DEBUG -std=gnu99

CLIENT_SOURCES=common.c client.c database/sqlite.c md5/md5.c file/file.c
DAEMON_SOURCES=common.c daemon.c database/sqlite.c md5/md5.c inotify/inotify.c file/file.c

all: client daemon

client: $(CLIENT_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CLIENT_SOURCES) -o track

daemon: $(DAEMON_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DAEMON_SOURCES) -o track_daemon

clean:
	rm -f track
