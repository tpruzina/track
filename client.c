#undef _TEST
#include "database/sqlite.h"
#include "inotify/inotify.h"
#include "md5/md5.h"
#include "common.h"

void init()
{
	// prepare ~/.track directory if it doesnt exist already
	char *home = getenv ("HOME");
	if (home != NULL)
		snprintf(data_path, sizeof(data_path), "%s/.track", home);
	DEBUG_PRINT("DATA_DIR = %s\n", data_path);

	if(mkdir(data_path, 0777) != 0)
	{
		if(errno != EEXIST)
		{
			// error handling
			perror(NULL);
			exit(EXIT_FAILURE);
		}
	}
	else
		printf("creating %s!\n",data_path);

	// open database (create new one if needed)
	snprintf(db_path, sizeof(db_path), "%s/db.sql", data_path);
	DEBUG_PRINT("DB_PATH = %s\n",db_path);
	
	if(db_open(db_path) != 0)
		exit(EXIT_FAILURE);
	return;
}

int parse_args(int argc, char **argv)
{
	return 0;
}

int main(int argc, char **argv)
{
	parse_args(argc,argv);
	init();

	track_file("client.c");

	//untrack_file("client.c");
	//delete_file("client.c");

	return 0;
}

