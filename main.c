#undef _TEST
#include "common.h"

void init()
{
	PRINT(DEBUG,"=====INIT====\n");

	log_level = NOTICE;

	if(!data_path)
	{
		data_path = malloc(1024);
		// prepare ~/.track directory if it doesnt exist already
		char *home = getenv ("HOME");
		if (home != NULL)
			snprintf(data_path, 1024, "%s/.track", home);
	}
	else
		data_path = realpath(data_path, NULL);

	PRINT(ERROR,"TRACK_DATA_PATH = %s\n", data_path);

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
	PRINT(DEBUG,"DB_PATH = %s\n",db_path);
	
	if(db_open(db_path) != 0)
		exit(EXIT_FAILURE);
	
	return;
}

int parse_args(int argc, char **argv)
{
	PRINT(DEBUG,"=====ARG PARSE====\n");

	if(argc <= 1)
		return TRACK_HELP;
	else
	{
		printf("%s\n",argv[1]);
	}

	PRINT(DEBUG,"\n");
	return 0;
}

void print_help()
{
	printf(
		"TRACK\n"
		"Usage:\n"
		"track --add\t\t tracks new file\n"
		"track --rm\t\t removes all backups of a file\n"
		"track --snapshot <mtime>\t\t creates snapshot of files [FILE]\n"
	);
}

void parse_env()
{
	data_path = getenv("TRACK_DATA_PATH");
}

void clean_up()
{
	free(data_path);
	// only commit to database if everything went ok
	db_commit();
}

void add(int argc, char **argv)
{
	for(int i=2; i < argc; i++)
	{
		PRINT(DEBUG,"%d %s\n",i,argv[i]);
		track_file(argv[i]);
	}

}

int main(int argc, char **argv)
{
	parse_env();
	init();
	int action = parse_args(argc,argv);

	switch(action)
	{
		case TRACK_HELP:	print_help();	break;
		case TRACK_ADD:				break;
		case TRACK_RM:				break;
		case TRACK_SNAPSHOT:	create_snapshot(NULL);		break;
	}

	time_t t= time(NULL);

	printf("%s\n",ctime(&t));
	add(argc,argv);

//	track_file("common.h");
//	track_file("common.c");

//	create_snapshot("test2");
	create_snapshot(NULL);

	list_file_versions("common.h");

	clean_up();
	return 0;
}



