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
#include <stdio.h>
#include <string.h>

#include "sqlite.h"
#include "../common.h"

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

int db_add_file(char *path, char *sanitized_hash, char *md5, long mtime)
{
	if(!pDB)
		exit(EXIT_FAILURE);
	int ret;
	char *qry= NULL;
	
	// insert record into file(path,hash)
	ret = asprintf(&qry, "insert into file (path, hash) values ('%s', '%s');", path, sanitized_hash);
	if(ret <= 0)
	{
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	sqlite3_prepare_v2(pDB, qry, strlen(qry), &query, NULL);
	ret = sqlite3_step(query);
	free(qry);
	
	if (ret != SQLITE_DONE)
	{
		printf("ERROR inserting data: %s\n", sqlite3_errmsg(pDB));
		return -1;
	}

	// insert record into file_version(hash, mtime, md5)
	qry=NULL;
	ret = asprintf(
		&qry,
		"insert into file_version (hash, mtime, md5) values ('%s', %ld, '%s')",
		sanitized_hash, mtime,md5
	);
	if(ret <= 0)
	{
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	sqlite3_prepare_v2(pDB, qry, strlen(qry), &query, NULL);
	ret = sqlite3_step(query);
	free(qry);	

	if (ret != SQLITE_DONE)
	{
		printf("ERROR inserting data: %s\n", sqlite3_errmsg(pDB));
		return -1;
	}
	
	return 0;
}

// checks wheather file is tracked
int db_query_file(const char *abs_path)
{
	sqlite3_prepare_v2(pDB, "select * from file where path = ?1;", -1, &query, NULL);
	sqlite3_bind_text(query, 1, abs_path, -1, NULL);

	if(sqlite3_step(query) == SQLITE_ROW)
		return 0;
	else
		return -1;
}

// compares latest tracked revision against current one (md5)
// contraintuively, this will return (char*) md5 of file
char *db_check_file_for_changes(char *abs_path)
{
	sqlite3_prepare_v2(pDB, "select * from file where path = ?1;", -1, &query, NULL);

	sqlite3_bind_text(query, 1, abs_path, -1, NULL);

	if(sqlite3_step(query) == SQLITE_ROW)
	{
		const unsigned char *hash = sqlite3_column_text(query, 1);
		sqlite3_prepare_v2(pDB, "select md5 from file_version where hash = ?1 order by mtime;", -1, &query, NULL);
		sqlite3_bind_text(query,1,(char *)hash, -1, NULL);
		if(sqlite3_step(query) == SQLITE_ROW)
		{
			const unsigned char *md5_old = sqlite3_column_text(query,0);
			char *md5_new = md5_sanitized_hash_of_file(abs_path);

			if(strncmp((char *)md5_old, md5_new, MD5_DIGEST_LENGTH) == 0)
				return NULL;
			else
				return md5_new;
		}
			else exit(EXIT_FAILURE);
	}
	else
	{
		// this shouldnt happen under normal circumstances since we
		// already checked that file is tracked
		exit(EXIT_FAILURE);
	}
}

#ifdef _TEST

int main(void)
{
	db_open("./test.sqldb");
	return 0;
}

#endif
