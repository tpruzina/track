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

void db_commit(void)
{
	sqlite3_exec(pDB,"COMMIT",0,0,0);
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
		PRINT(ERROR,"Failed to open conn: %d\n", ret);
		return ret;
        }
	else
		atexit(db_close);	// close database on exit

	// create database tables if they don't already exist
	sqlite3_exec(pDB,"PRAGMA foreign_keys = ON",0,0,0);

	// lock database
	sqlite3_exec(pDB, "PRAGMA locking_mode = EXCLUSIVE; BEGIN EXCLUSIVE;",0,0,0);
	
	sqlite3_exec(pDB,"CREATE TABLE IF NOT EXISTS file (hash TEXT PRIMARY KEY, path TEXT)",0,0,0);

	sqlite3_exec(pDB,"CREATE TABLE IF NOT EXISTS file_version (id INTEGER PRIMARY KEY, mtime INTEGER,md5 TEXT, hash TEXT, FOREIGN KEY(hash) REFERENCES file(hash))",0,0,0);

	sqlite3_exec(pDB,"CREATE TABLE IF NOT EXISTS snapshot (time INTEGER PRIMARY KEY, description TEXT)",0,0,0);

	sqlite3_exec(pDB,"CREATE TABLE IF NOT EXISTS snapshot_file(fv_id INTEGER, s_time INTEGER, FOREIGN KEY(fv_id) REFERENCES file_version(id), FOREIGN KEY(s_time) REFERENCES snapshot(time))",0,0,0);

	return 0;
}

int db_update_file_record(char *hash, char *md5, long mtime)
{
	if(!pDB)
		exit(EXIT_FAILURE);
	char *qry = NULL;

	if(0 >= asprintf(
		&qry,
		"insert into file_version (hash, mtime, md5) values ('%s', %ld, '%s')",
		hash, mtime, md5))
	{
		exit(EXIT_FAILURE);
	}
	
	sqlite3_prepare_v2(pDB, qry, strlen(qry), &query, NULL);
	sqlite3_step(query);
	free(qry);

	return 0;
}

int db_create_snapshot_record(long t,char *desc)
{
	if(!pDB)
		exit(EXIT_FAILURE);

	sqlite3_stmt *insert_query;

	sqlite3_prepare_v2(pDB, "insert into snapshot (time, description) values (?1, ?2);", -1, &insert_query, NULL);
	sqlite3_bind_int(insert_query, 1, t);
	sqlite3_bind_text(insert_query, 2, desc, -1, NULL);

	if (sqlite3_step(insert_query) != SQLITE_DONE)
	{
		sqlite3_errmsg(pDB);
		PRINT(ERROR,"Error inserting into 'snapshot' db!\n");
		exit(EXIT_FAILURE);
	}

	if(insert_query)
		sqlite3_finalize(insert_query);
	return 0;
}

int db_create_snapshot(long t)
{
	/*
	 * snapshot record is created, now link newest file_versions to snapshot
	 * for each file:
	 * 	insert into snapshot_file (fv_id, s_time) values (newest(file_version_id), t)
	 */

	if(!pDB)
		exit(EXIT_FAILURE);

	sqlite3_stmt *file_query;
	sqlite3_stmt *add_fv_id_into_snapshot_query;

	//sqlite3_prepare_v2(pDB,"select fv.* from file_version fv INNER JOIN (SELECT hash, MAX(mtime) AS latestt FROM file_version GROUP BY hash) latest on fv.mtime = latest.latestt and fv.hash = latest.hash", -1, &file_query, NULL);
	sqlite3_prepare_v2(pDB,"select fv.id from file_version fv INNER JOIN (SELECT hash, MAX(mtime) AS latestt FROM file_version GROUP BY hash) latest on fv.mtime = latest.latestt and fv.hash = latest.hash", -1, &file_query, NULL);

//	sqlite3_bind_int(add_fv_id_into_snapshot_query,1,5);
//	sqlite3_bind_int(add_fv_id_into_snapshot_query,2,5);
//	sqlite3_step(add_fv_id_into_snapshot_query);

	int ret;
	while(SQLITE_ROW == (ret =sqlite3_step(file_query)))
	{
		// for each file_and matching version records, find ids that has latest mtime and add them into snapshot
		//printf("%d|%s|%s|%s\n", sqlite3_column_int(file_query,0), sqlite3_column_text(file_query,1), sqlite3_column_text(file_query,2), sqlite3_column_text(file_query,3));
		//printf("%d\n",sqlite3_column_int(file_query,0));

		sqlite3_prepare_v2(pDB,"insert into snapshot_file (fv_id, s_time) values (?1, ?2);",-1, &add_fv_id_into_snapshot_query, NULL);
		sqlite3_bind_int(add_fv_id_into_snapshot_query,1,sqlite3_column_int(file_query,0));
		sqlite3_bind_int(add_fv_id_into_snapshot_query,2,t);
		sqlite3_step(add_fv_id_into_snapshot_query);
		sqlite3_finalize(add_fv_id_into_snapshot_query);
	}

	if(file_query)
		sqlite3_finalize(file_query);
//	if(add_fv_id_into_snapshot_query)
//		sqlite3_finalize(add_fv_id_into_snapshot_query);

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
char *db_check_file_for_changes_md5(char *abs_path)
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

int db_check_file_for_changes_mtime(char *hash, long mtime)
{
	int ret;
	const char *qmsg = "SELECT * from file_version where hash == ?1 AND mtime == ?2";
	sqlite3_prepare_v2(pDB, qmsg, -1, &query, NULL);
	sqlite3_bind_text(query,1,hash,-1,NULL);
	sqlite3_bind_int(query, 2, mtime);

	if(sqlite3_step(query) == SQLITE_ROW)
	{
		PRINT(DEBUG,"mtime compares the same\n");
		ret = 0;
	}
	else
	{
		PRINT(DEBUG,"mtime differs\n");
		ret = -1;
	}
	// cleanup
	
	return ret;
}


#ifdef _TEST

int main(void)
{
	db_open("./test.sqldb");
	return 0;
}

#endif
