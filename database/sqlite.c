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

#ifdef _TEST

int main(void)
{
	db_open("./test.sqldb");
	return 0;
}

#endif
