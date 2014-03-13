#undef _TEST
#include "database/sqlite.h"
#include "inotify/inotify.h"
#include "md5/md5.h"

// global variables
static char hash[MD5_DIGEST_LENGTH];
const char *data_path = "test_data";
const char *db_path = "db.sql";

int init()
{
	db_open(db_path);
	return 0;
}

int parse_args(int argc, char **argv)
{
	return 0;
}

int main(int argc, char **argv)
{
	init();
	
	//parse_args(argc,argv);

	track_file("client.c");

	//untrack_file("client.c");
	//delete_file("client.c");

	return 0;
}

