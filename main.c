#undef _TEST
#include "common.h"

void init()
{
	PRINT(DEBUG,"=====INIT====\n");

	log_level = DEBUG;

	if(!data_path)
	{
		data_path = malloc(1024);
		// prepare ~/.track directory if it doesnt exist already
		//char *home = getenv ("HOME");
		//if (home != NULL)
		//	snprintf(data_path, 1024, "%s/.track", home);
		snprintf(data_path,1024,"./.track");
	}
	else
		data_path = realpath(data_path, NULL);

	PRINT(ERROR,"TRACK_DATA_PATH = %s\n", data_path);

//	if(enforce_md5)
//		check_file_for_changes = check_file_for_changes_md5;
//	else
		check_file_for_changes = check_file_for_changes_mtime;

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
		if(strcmp("--add",argv[1]) == 0)
			return TRACK_ADD;
		else if(strcmp("--rm",argv[1]) == 0)
			return TRACK_RM;
		else if(strcmp("--snapshot", argv[1]) == 0)
			return TRACK_SNAPSHOT;
		else if(strcmp("--show", argv[1]) == 0)
			return TRACK_SHOW;
		//todo: else...
	}
	PRINT(DEBUG,"\n");
	return 0;
}

void print_help()
{
	// TODO: UPDATE!!!!
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
	enforce_md5 = getenv("TRACK_FORCE_MD5");
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
		PRINT(DEBUG,"%d adding/updating: %s\n",i,argv[i]);
		track_file(argv[i]);
	}
}

void rm(int argc, char **argv)
{
	for(int i=2; i < argc; i++)
	{
		PRINT(DEBUG, "%d removing: %s\n",i,argv[i]);
		remove_file(argv[i]);
	}
}

// valide backup (recalculate hashes and compare them with database)
int validate()
{
	// foreach file version(hash, md5) recalculate hash and compare
	return  0;
}

int show(int argc, char **argv)
{
	char *abs_path;
	if(argc <= 2)
		return db_showchanged_files_md5();
	else
	{
		char *tmp;
		for(int i=2; i < argc; i++)
		{
			abs_path=realpath(argv[i],NULL);
			if(!abs_path)
				PRINT(NOTICE,"%s unrecognized.\n",argv[i]);
			else
			{
				tmp = md5_sanitized_hash_of_string(abs_path);
				PRINT(NOTICE,"%s:\n",abs_path);
				if(db_query_file(abs_path) != -1)
					db_list_file_versions(tmp);
				else
					PRINT(NOTICE,"NOT TRACKED.\n");
				free(tmp);
			}
		}
	}
}

void snapshot(int argc, char **argv)
{
	if(argc <= 1)
		create_snapshot(NULL);
	else
		create_snapshot(argv[2]);
}

// exports [[ snapshot mtime (id) ]] onto [[ destination]]
// if mtime is null, exports most current backup into destination
// if destination is null, replaces files with their respective backups
int export(int argc, char **argv)
{
	// these are to be parsed
	int mtime = -1;
	char *dest = NULL;

//	for(int i=2; i < argc; i++)
//	{
//		if(stcmp)
//	}
}

int main(int argc, char **argv)
{
	parse_env();
	init();
	int action = parse_args(argc,argv);

	switch(action)
	{
		case TRACK_ADD:		add(argc,argv);		break;
		case TRACK_RM:		rm(argc,argv);		break;
		case TRACK_SNAPSHOT:	snapshot(argc,argv);	break;
		case TRACK_VERIFY:	validate();		break;
		case TRACK_SHOW:	show(argc,argv);	break;
		default:
		case TRACK_HELP:	print_help();		break;
	}

	clean_up();
	return 0;
}
