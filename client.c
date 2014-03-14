#undef _TEST
#include "common.h"

void init()
{
	DEBUG_PRINT("=====INIT====\n");

	if(!data_path)
	{
		data_path = malloc(1024);
		// prepare ~/.track directory if it doesnt exist already
		char *home = getenv ("HOME");
		if (home != NULL)
			snprintf(data_path, 1024, "%s/.track", home);
	}

	DEBUG_PRINT("TRACK_DATA_PATH = %s\n", data_path);

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
		printf("Creating %s!\n",data_path);

	// open database (create new one if needed)
	snprintf(db_path, sizeof(db_path), "%s/db.sql", data_path);
	DEBUG_PRINT("DB_PATH = %s\n",db_path);
	
	if(db_open(db_path) != 0)
		exit(EXIT_FAILURE);
	
	DEBUG_PRINT("\n");
	return;
}

int parse_args(int argc, char **argv)
{
	DEBUG_PRINT("=====ARG PARSE====\n");

	if(argc <= 1)
	{
		printf(
			"TRACK\n"
			"Usage:\n"
			"track --add\t\t tracks new file\n"
			"track --commit\n"
			"track --sync\t\t synchronizes new files\n"
			"track --untrack\t\t stops tracking file (backup persists)\n"
			"track --rm\t\t removes all backups of a file\n"
			"track --snapshot <mtime>\t\t creates snapshot of files [FILE]\n\n"
		);
			
	}
	else
	{
		printf("%s\n",argv[1]);
	}

	DEBUG_PRINT("\n");
	return 0;
}

void parse_env()
{
	data_path = getenv("TRACK_DATA_PATH");
}

int main(int argc, char **argv)
{
	parse_env();
	parse_args(argc,argv);
	init();

	track_file("client.c");

	//local_copy("client.c","client.b");

	//untrack_file("client.c");
	//delete_file("client.c");

	free(data_path);
	return 0;
}

