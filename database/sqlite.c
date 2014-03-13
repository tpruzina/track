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

#include "sqlite.h"

sqlite3 *pDB = NULL;
sqlite3_stmt* query = NULL;

void db_close(void)
{
	if (query != NULL)
		sqlite3_finalize(query);
	sqlite3_close(pDB);
	sqlite3_shutdown();
}

int db_open(const char *path)
{
	int ret;

	// initialize sqlite
	if (SQLITE_OK != (ret = sqlite3_initialize()))
	{
		//handle init error
		return ret;
	}

	// open/create database
	if (SQLITE_OK != (ret = sqlite3_open_v2(path, &pDB, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)))
        {
		// handle open error
		printf("Failed to open conn: %d\n", ret);
		return ret;
        }
	else
		atexit(db_close);	// close database on exit

	// create file table if it does not exist already
	sqlite3_exec(pDB,"CREATE TABLE IF NOT EXISTS file (path TEXT PRIMARY KEY, hash TEXT)",0,0,0);
	sqlite3_exec(pDB,"CREATE TABLE IF NOT EXISTS file_version (hash TEXT PRIMARY KEY, mtime INTEGER,md5 TEXT)",0,0,0);

	return 0;
}


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

int track_file(const char *file_path)
{
	// verify rights and open file
	struct stat st;
	if(stat(file_path, &st) == -1)
	{
		exit(EXIT_FAILURE);
		//handle error
	}
		
	// check wheather file isn't tracked already
	if(0)
	{
		// TODO
	}
	else // file isn't tracked yet - track it!
	{
		// FILE: <PK file_path> <hash>
		// FILE_VERSION: <PK hash> <mtime> <md5>
		
		// 1. calculate hashes and prepare filesystem paths
		unsigned char md5[MD5_DIGEST_LENGTH];
		unsigned char hash[MD5_DIGEST_LENGTH];
		char backup_path[1024] = {0};
		char dir_path[1024] = {0};
		
		md5_calculate_hash(file_path, md5);
		char *sanitized_file_hash = md5_sanitized_hash(md5);
		
		md5_calculate_hash_from_string(file_path, hash);
		char *sanitized_hash = md5_sanitized_hash(hash);
		
		snprintf(dir_path, sizeof(dir_path), "%s/%s", data_path, sanitized_hash);
		snprintf(backup_path, sizeof(backup_path), "%s/%s/%s",data_path,sanitized_hash,sanitized_file_hash);
		
		// 2. create directory named "hash of filepath"
		if(mkdir(dir_path, 0777) != 0)
		{
			perror(NULL);
			exit(EXIT_FAILURE);
		}
		
		// 3. copy file into "hash of filepath"/"md5 of file";
		local_copy(file_path,backup_path);
		
		// 4. add into database
		

		//cleanup
		free(sanitized_file_hash);
		free(sanitized_hash);
	}
	return 0;
}

int query_file(const char *file)
{
	return 0;
}

#ifdef _TEST

int main(void)
{
	db_open("./test.sqldb");
	return 0;
}

#endif

