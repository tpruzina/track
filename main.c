#undef _TEST
#include "database/sqlite.h"
#include "inotify/inotify.h"
#include "md5/md5.h"


int main(int argc, char **argv)
{
	inotify_initialize();
	db_open("db.sql");
	return 0;
}

