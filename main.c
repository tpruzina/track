#include "common.h"

void init_track()
{

	if(!data_path)
		data_path = "./.track";
	else
		data_path = realpath(data_path, NULL);
	
	// we are going to initialize data folder, call init() from main
	if(opts.op == TRACK_INIT)
		return;
	
	if(!data_path)
	{
		PRINT(ERROR,"database not initialized!\n");
		exit(EXIT_FAILURE);
	}
	PRINT(DEBUG,"TRACK_DATA_PATH = %s\n", data_path);
	
	// open database
	snprintf(db_path, sizeof(db_path), "%s/db.sql", data_path);
	PRINT(DEBUG,"DB_PATH = %s\n",db_path);
	
	if(db_open(db_path) != 0)
		exit(EXIT_FAILURE);
	return;
}

void init()
{
	PRINT(MESSAGE,"Initializing new database at %s\n", data_path);
	
	data_path = save_string_into_buffer(data_path);
	if(!data_path)
		exit(EXIT_FAILURE);

	if(mkdir(data_path,0777) != 0)
	{
		if(errno != EEXIST)
		{
			// error handling
			perror(NULL);
			exit(EXIT_FAILURE);
		}
	}
	else
		PRINT(NOTICE,"Creating %s!\n",data_path);

	init_track();
}

bool parse(char *s1, char *s2)
{
	return (strcmp(s1,s2) == 0) ? true : false;
}

int parse_args(int argc, char **argv)
{
	int i=1;
	for( ; i < argc; i++)
	{
		if(parse("--md5",argv[i]))
			opts.md5_enforce=true;
		else if(parse("-vv",argv[i]))
			log_level = DEBUG;
		else if(parse("-v",argv[i]))
			log_level= NOTICE;
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
		else if(parse("--init",argv[i]) ||
			parse("init",argv[i]))
		{
			opts.op=TRACK_INIT;
			break;
		}
	}
	opts.next_arg = &(argv[++i]);

	return EOK;
}

void print_help()
{
	// TODO: UPDATE!!!!
	printf(
	"track - simple tracking and backup tool\n"
	"track [[global args]] action [[action args]]\n"
	"\n"
	"global args:\n"
	"\t--md5\t\tenforce chocking md5 sums when manipulating files.\n"
	"\t--data-path\t\t path to backups, otherwise current dir is assumed (.track)\n"
	"\t-q\t\t be quiet\n"
	"\t-v\t\t be verbose (debug)\n\n"
	"actions: help,add,rm,show,diff,snapshot,export,verify. "
	"actions may be prefixed with --\n"
	"\thelp\n"
	"\t\tprints this. (default)\n\n"
	"\tadd <file> [[file..]]\n"
	"\trm <file> [[file..]]\n"
	"\t\tadds/removes files from [[action args]] into database and backs them up\n\n"
	"\tshow [[file...]]\n"
	"\t\tno args - prints all tracked files from database with their latest versions\n"
	"\t\twith argument - for each argument, prints all versions from database along with info\n\n"
	"\tdiff [[file]]\n"
	"\t\t no args - compares all the tracked files with their counterparts in database.\n"
	"\t\t with args - for each file: compares them with database and prints weather they changed or not."
	""
	""
	""
	""
	""
	""
	""
	""
	);
}

void parse_env()
{
	enforce_md5 = getenv("TRACK_FORCE_MD5");
	data_path = getenv("TRACK_DATA_PATH");

#ifdef _DEBUG
	log_level = DEBUG;
#else
	log_level = MESSAGE;
#endif
}

void clean_up()
{
	return;
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
	return  EOK;
}

void show()
{
	if(!(*opts.next_arg))
		PRINT(NOTICE,"please supply a parameter (file(s))\n");	
	else
	{
		char *tmp;
		do
		{
			char *abs_path=realpath(*opts.next_arg,NULL);
			if(!abs_path)
				PRINT(MESSAGE,"%s not found.\n",*opts.next_arg);
			else
			{
				tmp = md5_sanitized_hash_of_string(abs_path);
				PRINT(MESSAGE,"%s:\n",abs_path);
				if(db_query_file(abs_path) != -1)
					db_list_file_versions(tmp);
				else
					PRINT(MESSAGE,"NOT TRACKED.\n");
				free(tmp);
			}
			opts.next_arg++;
		}
		while(*opts.next_arg && putchar('\n'));
	}
}

void diff()
{
	if(opts.md5_enforce)
		db_showchanged_files_md5();
	else
		db_showchanged_files_mtime();
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


	return EOK;
}

int main(int argc, char **argv)
{
	parse_env();
	parse_args(argc,argv);
	
	if(opts.op != TRACK_HELP)
		init_track();

	switch(opts.op)
	{
		case TRACK_ADD:		add();			break;
		case TRACK_RM:		rm();			break;
		case TRACK_SNAPSHOT:	snapshot();		break;
		case TRACK_VERIFY:	verify();		break;
		case TRACK_SHOW:	show();			break;
		case TRACK_DIFF:	diff();			break;
		case TRACK_INIT:	init();			break;
		case TRACK_HELP:	print_help();		break;
		
		default:
			PRINT(MESSAGE, "please run \"track --help\" for usage.\n");
	}

	clean_up();
	// only commit to database if everything went ok
	if(opts.op != TRACK_HELP)
		db_commit();
	
	return EOK;
}
