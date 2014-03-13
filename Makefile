LDFLAGS=-lssl -lcrypto -lsqlite3
SOURCES=main.c database/sqlite.c md5/md5.c inotify/inotify.c
CFLAGS=-O2 -g

all: main

main: $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o tbackup

clean:
	rm -f tbackup
