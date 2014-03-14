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
	char *file_path = realpath(path, NULL);

	// verify rights and open file
	struct stat st;
	if(stat(file_path, &st) == -1)
	{
		exit(EXIT_FAILURE);
		//handle error
	}
	

	// check wheather file isn't tracked already
	if(query_file(file_path) == 0)
	{
		DEBUG_PRINT("found %s in database\n", file_path);
	}
	else // file isn't tracked yet - track it!
	{
	
		// 1. calculate hashes and prepare filesystem paths
		unsigned char md5[MD5_DIGEST_LENGTH];
		unsigned char hash[MD5_DIGEST_LENGTH];
		char backup_path[1024] = {0};
		char dir_path[1024] = {0};
		
		md5_calculate_hash(file_path, md5);
		char *sanitized_md5 = md5_sanitized_hash(md5);
		
		md5_calculate_hash_from_string(file_path, hash);
		char *sanitized_hash = md5_sanitized_hash(hash);
		
		snprintf(dir_path, sizeof(dir_path), "%s/%s", data_path, sanitized_hash);
		snprintf(backup_path, sizeof(backup_path), "%s/%s/%s",data_path,sanitized_hash,sanitized_md5);
		
		// 2. create directory named "hash of filepath"
		if(mkdir(dir_path, 0777) != 0)
		{
			perror(NULL);
			exit(EXIT_FAILURE);
		}
		
		// 3. copy file into "hash of filepath"/"md5 of file";
		local_copy(file_path,backup_path);
		
		// 4. add into database
		// FILE: <PK file_path> <hash>
		// FILE_VERSION: <PK hash> <mtime> <md5>
		db_add_file(file_path, sanitized_hash, sanitized_md5, st.st_mtime);

		//cleanup
		free(sanitized_md5);
		free(sanitized_hash);
	}
	free(file_path);

	return 0;
}





