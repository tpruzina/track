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
	sqlite3_exec(pDB,"CREATE TABLE IF NOT EXISTS files (path TEXT PRIMARY KEY, mtime INTEGER)",0,0,0);

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

int track_file(const char *file)
{
	// verify rights and open file
	struct stat st;
	if(stat(file, &st) == -1)
	{
		exit(-1);
		//handle error
	}
	
	// check wheather file isn't tracked already
	
	// calculate md5
	
	// add into database
	
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

