
all: ALL

ALL: md5.c
	gcc -lcrypto -lssl -std=gnu99 -D_TEST $< -o md5_test
