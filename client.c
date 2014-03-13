#undef _TEST
#include "database/sqlite.h"
#include "inotify/inotify.h"
#include "md5/md5.h"


int main(int argc, char **argv)
{
	inotify_initialize();
	db_open("db.sql");

	static char array[1000];
	md5_calculate_hash("main.c",array);

	return 0;
}

