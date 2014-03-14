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
	/*
	char *zSQL = sqlite3_mprintf("INSERT INTO file VALUES ('%s,%s')", path, sanitized_hash);
	sqlite3_exec(pDB, zSQL, 0, 0, 0);
	sqlite3_free(zSQL);
	*/

	char *qry= NULL;
	asprintf(&qry, "insert into file (path, hash) values ('%s', '%s');", path, sanitized_hash);
	sqlite3_prepare_v2(pDB, qry, strlen(qry), &query, NULL);
	int ret = sqlite3_step(query);
	free(qry);
	
	if (ret != SQLITE_DONE)
	{
		printf("ERROR inserting data: %s\n", sqlite3_errmsg(pDB));
		return -1;
	}

	qry=NULL;
	asprintf(
		&qry,
		"insert into file_version (hash, mtime, md5) values ('%s', %ld, '%s')",
		sanitized_hash, mtime,md5
	);
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
int query_file(const char *path)
{
	int ret = 0;
	char *file_path = realpath(path, NULL);

	sqlite3_prepare_v2(pDB, "select * from file where path = ?1;", -1, &query, NULL);

	sqlite3_bind_text(query, 1, file_path, -1, NULL);

	if((ret = sqlite3_step(query)) == SQLITE_ROW)
		ret = 0;
	else
		ret = -1;

	// cleanup
	free(file_path);
	return ret;
}

int check_file_for_changes(const char *path)
{
	int ret = 0;
	char *file_path = realpath(path, NULL);

	sqlite3_prepare_v2(pDB, "select * from file where path = ?1;", -1, &query, NULL);

	sqlite3_bind_text(query, 1, file_path, -1, NULL);

	if((ret = sqlite3_step(query)) == SQLITE_ROW)
		ret = 0;
	else
		ret = -1;

	// cleanup
	free(file_path);
	return ret;
}

#ifdef _TEST

int main(void)
{
	db_open("./test.sqldb");
	return 0;
}

#endif
