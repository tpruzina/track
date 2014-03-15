/******************************************************************************
 *
 * TODO: Project Title
 *
 * Author: Tomas Pruzina <pruzinat@gmail.com>
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "database.h"
#include "sqlite.h"


int delete_file(const char *file)
{
	// untrack file
	
	// delete all instances of a file
	return 0;
}

int untrack_file(const char *file)
{
	return 0;
}

int track_file(const char *path)
{
	// get canonical (full) path to file
	char *abs_path = realpath(path, NULL);

	// precalculate hash of file's canonical path (primary key)
	char *hash = md5_sanitized_hash_of_string(abs_path);
	char *md5 = NULL;

	char backup_path[1024] = {0};
	char dir_path[1024] = {0};

	// verify rights and open file
	struct stat st;
	if(stat(abs_path, &st) == -1)
	{
		perror(NULL);
		exit(EXIT_FAILURE);
		//handle error
	}

	// check wheather file isn't tracked already
	if(db_query_file(abs_path) == 0)
	{
		DEBUG_PRINT("found %s in database\n", abs_path);
		if((md5 = check_file_for_changes_md5(abs_path)) != NULL)
		{
			// file has changed - add new version to database!
			DEBUG_PRINT("%s has changed\n",abs_path);

			snprintf(backup_path, sizeof(backup_path), "%s/%s/%s",data_path,hash,md5);

			local_copy(abs_path, backup_path);
			
			db_update_file_record(hash, md5, st.st_mtime);
		}
		else	// file is the same
		{
			DEBUG_PRINT("%s hasn't changed\n",abs_path);
			// update record in database
		}
	}
	else // file isn't tracked yet - track it!
	{
	
		// 1. calculate hashes and prepare filesystem paths

		
		md5 = md5_sanitized_hash_of_file(abs_path);

		snprintf(dir_path, sizeof(dir_path), "%s/%s", data_path, hash);
		snprintf(backup_path, sizeof(backup_path), "%s/%s/%s",data_path,hash,md5);
		
		// 2. create directory named "hash of filepath"
		if(mkdir(dir_path, 0777) != 0)
		{
			perror(NULL);
			exit(EXIT_FAILURE);
		}
		
		// 3. copy file into "hash of filepath"/"md5 of file";
		local_copy(abs_path,backup_path);
		
		// 4. add into database
		// FILE: <PK file_path> <hash>
		// FILE_VERSION: <PK hash> <mtime> <md5>
		db_add_file(abs_path, hash, md5, st.st_mtime);

		//cleanup
		free(md5);
		free(hash);
	}
	free(abs_path);

	return 0;
}

// cheap check
int check_file_for_changes_mtime(char *abs_path)
{
	return db_check_file_for_changes_mtime(abs_path);
}

// hard check
// computes md5 hash
// returns NULL when it's the same as last entry in db
// returns new md5 else
char *check_file_for_changes_md5(char *abs_path)
{
	return db_check_file_for_changes_md5(abs_path);
}

int update_record(const char *file)
{
	return 0;
}





