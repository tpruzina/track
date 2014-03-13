LDFLAGS=-lssl -lcrypto -lsqlite3
CFLAGS=-O2 -g

CLIENT_SOURCES=client.c database/sqlite.c md5/md5.c inotify/inotify.c
DAEMON_SOURCES=daemon.c database/sqlite.c md5/md5.c inotify/inotify.c

all: client daemon

client: $(CLIENT_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CLIENT_SOURCES) -o tbackup

daemon: $(DAEMON_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(DAEMON_SOURCES) -o tbackup_daemon

clean:
	rm -f tbackup
