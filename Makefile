LDFLAGS=-lssl -lcrypto -lsqlite3
SOURCES=main.c database/sqlite.c md5/md5.c inotify/inotify.c


all: main

main: $(SOURCES)
	$(CC) $(LDFLAGS) $(SOURCES) -o tbackup
