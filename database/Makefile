all: ALL
ALL: sqlite

sqlite: sqlite.c
	gcc -D_TEST -o sqlite_test $< -lsqlite3

sqlite_obj: sqlite.c
	gcc -c $< -lsqlite3
