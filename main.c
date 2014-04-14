#undef _TEST
#include "common.h"

void init()
{
	PRINT(DEBUG,"=====INIT====\n");

#ifdef DEBUG
	log_level = DEBUG;
#else
	log_level = NOTICE;
#endif

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

bool parse(char *s1, char *s2)
{
	return (strcmp(s1,s2) == 0) ? true : false;
}

int parse_args(int argc, char **argv)
{
	PRINT(DEBUG,"=====ARG PARSE====\n");

	int i=1;
	for( ; i < argc; i++)
	{
		if(parse("--md5",argv[i]))
			opts.md5_enforce=true;
		else if(parse("-v",argv[i]))
			log_level = DEBUG;
		else if(parse("-q",argv[i]))
			log_level = ERROR;
		else if(parse("--data-path",argv[i]))
		{
			data_path=argv[++i];
			i++;
		}
		else if(parse("--export",argv[i]) ||
			parse("export",argv[i]))
		{
			opts.op=TRACK_EXPORT;
		}
		else if(parse("--add",argv[i]) ||
			parse("add",argv[i]))
		{
			opts.op=TRACK_ADD;
			break;
		}
		else if(parse("--rm",argv[i]) ||
			parse("rm",argv[i]))
		{
			opts.op=TRACK_RM;
			break;
		}
		else if(parse("--diff",argv[i]) ||
			parse("diff",argv[i]))
		{
			opts.op=TRACK_DIFF;
			break;
		}
		else if(parse("--snapshot",argv[i]) ||
			parse("snapshot",argv[i]))
		{
			opts.op=TRACK_SNAPSHOT;
			break;
		}
		else if(parse("--verify",argv[i]) ||
			parse("verify",argv[i]))
		{
			opts.op=TRACK_VERIFY;
			break;
		}
		else if(parse("--show", argv[i]) ||
			parse("show",argv[i]))
		{
			opts.op=TRACK_SHOW;
			break;
		}
		else if(parse("--help",argv[i]) ||
			parse("help",argv[i]) ||
			parse("-?",argv[i]))
		{
			opts.op=TRACK_HELP;
			break;
		}
	}
	opts.next_arg = &(argv[++i]);


	PRINT(DEBUG,"\n");
	return 0;
}

void print_help()
{
	// TODO: UPDATE!!!!
	printf(
	"track - simple tracking and backup tool\n"
	"track [[global args]] action [[action args]]\n"
	"\n"
	"global args:\n"
	"--md5\t\tenforce chocking md5 sums when manipulating files.\n"
	"--data-path\t\t path to backups, otherwise current dir is assumed (.track)\n"
	"-q\t\t be quiet\n"
	"-v\t\t be verbose (debug)\n\n"
	"actions: help,add,rm,show,diff,snapshot,export,verify\n"
	""
	""
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
}

void add(void)
{
	while(*opts.next_arg)
	{
		PRINT(DEBUG,"adding/updating: %s\n",*opts.next_arg);
		track_file(*opts.next_arg);
		opts.next_arg++;
	}
}

void rm()
{
	while(*opts.next_arg)
	{
		PRINT(DEBUG,"removing: %s\n",*opts.next_arg);
		remove_file(*opts.next_arg);
		opts.next_arg++;
	}
}

// valide backup (recalculate hashes and compare them with database)
int verify()
{
	// foreach file version(hash, md5) recalculate hash and compare
	return  0;
}

void show()
{
	if(!(*opts.next_arg))
		db_showchanged_files_md5();
	else
	{
		char *tmp;
		while(*opts.next_arg)
		{
			char *abs_path=realpath(*opts.next_arg,NULL);
			if(!abs_path)
				PRINT(NOTICE,"%s unrecognized.\n",*opts.next_arg);
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
			putchar('\n');
			opts.next_arg++;
		}
	}
}

void diff()
{
	db_showchanged_files_md5();
}

void snapshot()
{
	if(!(*opts.next_arg))
		create_snapshot(NULL);
	else
		create_snapshot(*opts.next_arg);
}

// exports [[ snapshot mtime (id) ]] onto [[ destination]]
// if mtime is null, exports most current backup into destination
// if destination is null, replaces files with their respective backups
int export()
{
	// these are to be parsed
//	int mtime = -1;
//	char *dest = NULL;


	return 0;
}

int main(int argc, char **argv)
{
	parse_env();
	parse_args(argc,argv);
	init();

	switch(opts.op)
	{
		case TRACK_ADD:		add();			break;
		case TRACK_RM:		rm();			break;
		case TRACK_SNAPSHOT:	snapshot();		break;
		case TRACK_VERIFY:	verify();		break;
		case TRACK_SHOW:	show();			break;
		case TRACK_DIFF:	diff();			break;

		default:
		case TRACK_HELP:	print_help();		break;
	}

	clean_up();
	// only commit to database if everything went ok
	db_commit();
	return 0;
}
