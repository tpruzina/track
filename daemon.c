#undef _TEST
#include "database/sqlite.h"
#include "inotify/inotify.h"
#include "md5/md5.h"


struct options
{
	const char *data_path;
	const char *db_path;
} global_opts;

int parse_options()
{
	global_opts.data_path = "test_data";
	global_opts.db_path = "db.sql";
	return 0;
}

int init()
{
	// check if data_path exist
	db_open(global_opts.db_path);
	
	// initialize inotify
	inotify_initialize();}

int main(int argc, char **argv)
{
	init();
	static char array[1000];
	md5_calculate_hash("main.c",array);

	return 0;
}

