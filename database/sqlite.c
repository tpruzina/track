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

void db_close(void)
{
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

	// set foreign keys on
	sqlite3_exec(pDB,"PRAGMA foreign_keys = ON",0,0,0);
	// lock database
	sqlite3_exec(pDB, "PRAGMA locking_mode = EXCLUSIVE; BEGIN EXCLUSIVE;",0,0,0);

	// create database tables if they don't already exist
	sqlite3_exec(pDB,"CREATE TABLE IF NOT EXISTS file (hash TEXT PRIMARY KEY, path TEXT, tracked INTEGER)",0,0,0);
	sqlite3_exec(pDB,"CREATE TABLE IF NOT EXISTS file_version (id INTEGER PRIMARY KEY, mtime INTEGER,md5 TEXT, hash TEXT, FOREIGN KEY(hash) REFERENCES file(hash))",0,0,0);
	sqlite3_exec(pDB,"CREATE TABLE IF NOT EXISTS snapshot (time INTEGER PRIMARY KEY, description TEXT)",0,0,0);
	sqlite3_exec(pDB,"CREATE TABLE IF NOT EXISTS snapshot_file(fv_id INTEGER, s_time INTEGER, FOREIGN KEY(fv_id) REFERENCES file_version(id), FOREIGN KEY(s_time) REFERENCES snapshot(time))",0,0,0);

	return 0;
}

int db_list_file_versions(char *hash)
{
	if(!pDB)
		exit(EXIT_FAILURE);

	int count = 0;
	sqlite3_stmt *query;

	sqlite3_prepare_v2(pDB, "select * from file_version fv where fv.hash = ?1", -1, &query, NULL);
	sqlite3_bind_text(query,1,hash, -1, NULL);

	while(sqlite3_step(query) == SQLITE_ROW)
	{
		count++;
		PRINT(NOTICE,"%d|%s|",count, sqlite3_column_text(query,2));
		print_time(sqlite3_column_int(query,1));
	}

	if(query)
		sqlite3_finalize(query);
	
	return count;
}

char *db_get_newest_md5(char *hash)
{
	if(!hash || !pDB)
		return NULL;

	sqlite3_stmt *query_md5;
	sqlite3_prepare_v2(pDB,"select md5 from file_version where hash = $1 order by mtime DESC;",-1,&query_md5,NULL);
	sqlite3_bind_text(query_md5,1,hash, -1,NULL);

	char*string = malloc(MD5_DIGEST_LENGTH*2+1);

	int ret = sqlite3_step(query_md5);
	if(ret == SQLITE_ROW || ret == SQLITE_DONE)
	{
		strcpy(string,(char*)sqlite3_column_text(query_md5,0));
		sqlite3_finalize(query_md5);
		return string;
	}
	else
		return NULL;
}

int db_showchanged_files_md5()
{
	if(!pDB)
		exit(EXIT_FAILURE);

	sqlite3_stmt *query_files;

	sqlite3_prepare_v2(pDB, "select hash,path from file;",-1, &query_files, NULL);

	while(sqlite3_step(query_files) == SQLITE_ROW)
	{
		char *hash = (char *)sqlite3_column_text(query_files,0);
		char *path = (char *)sqlite3_column_text(query_files,1);

		const char *curr = md5_sanitized_hash_of_file(path);
		const char *newest_in_db = db_get_newest_md5(hash);

		if(strcmp(newest_in_db,curr) != 0)
			PRINT(NOTICE,"changed:%s\t md5_new=%s\t md5_db: %s\n",path, curr, newest_in_db);
	}

	if(query_files)
		sqlite3_finalize(query_files);

	return 0;
}

int db_add_file_record(char *hash, char *md5, long mtime)
{
	if(!pDB)
		exit(EXIT_FAILURE);

	sqlite3_stmt *query;

	sqlite3_prepare_v2(pDB,"INSERT INTO file_version (hash, mtime, md5) VALUES (?1, ?2, ?3)",-1,&query,NULL);
	sqlite3_bind_text(query,1,hash,-1,NULL);
	sqlite3_bind_int(query,2,mtime);
	sqlite3_bind_text(query,3,md5,-1,NULL);
	
	// todo: return code
	if(sqlite3_step(query) != SQLITE_DONE)
		return -1;

	if(query)
		sqlite3_finalize(query);

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
		//sqlite3_errmsg(pDB);
		PRINT(ERROR,"Error inserting into 'snapshot' db!\n");
		exit(EXIT_FAILURE);
	}

	if(insert_query)
		sqlite3_finalize(insert_query);
	return 0;
}

/*
 * snapshot record is created, now link newest file_versions to snapshot
 * for each file:
 * 	insert into snapshot_file (fv_id, s_time) values (newest(file_version_id), t)
 */
int db_create_snapshot(long t)
{

	if(!pDB)
		exit(EXIT_FAILURE);

	sqlite3_stmt *file_query;
	sqlite3_stmt *add_fv_id_into_snapshot_query;

	//sqlite3_prepare_v2(pDB,"select fv.* from file_version fv INNER JOIN (SELECT hash, MAX(mtime) AS latestt FROM file_version GROUP BY hash) latest on fv.mtime = latest.latestt and fv.hash = latest.hash", -1, &file_query, NULL);
	sqlite3_prepare_v2(pDB,"select fv.id from file_version fv INNER JOIN (SELECT hash, MAX(mtime) AS latestt FROM file_version GROUP BY hash) latest on fv.mtime = latest.latestt and fv.hash = latest.hash", -1, &file_query, NULL);

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

	return 0;
}

int db_add_file(char *path, char *sanitized_hash, char *md5, long mtime)
{
	if(!pDB)
		exit(EXIT_FAILURE);
	int ret;
	sqlite3_stmt *query_file;
	
	// insert record into file(path,hash)
	sqlite3_prepare_v2(pDB,"INSERT INTO file (path, hash, tracked) VALUES (?1, ?2, 1)",-1,&query_file,NULL);
	sqlite3_bind_text(query_file,1,path,-1,NULL);
	sqlite3_bind_text(query_file,2,sanitized_hash,-1,NULL);
	ret = sqlite3_step(query_file);
	
	if (ret != SQLITE_DONE)
	{
		printf("ERROR inserting data: %s\n", sqlite3_errmsg(pDB));
		return -1;
	}
	else
		sqlite3_finalize(query_file);

	// insert record into file_version(hash, mtime, md5)
	sqlite3_stmt *query_fv;
	sqlite3_prepare_v2(pDB, "INSERT INTO file_version (hash,mtime,md5) VALUES (?1,?2,?3)", -1, &query_fv, NULL);
	sqlite3_bind_text(query_fv,1,sanitized_hash,-1,NULL);
	sqlite3_bind_int(query_fv,2,mtime);
	sqlite3_bind_text(query_fv,3,md5,-1,NULL);

	ret = sqlite3_step(query_fv);

	if (ret != SQLITE_DONE)
	{
		printf("ERROR inserting data: %s\n", sqlite3_errmsg(pDB));
		return -1;
	}
	
	if (query_fv != NULL)
		sqlite3_finalize(query_fv);

	return 0;
}

// checks wheather file is tracked
int db_query_file(const char *abs_path)
{
	sqlite3_stmt *query;
	sqlite3_prepare_v2(pDB, "select * from file where path = ?1;", -1, &query, NULL);
	sqlite3_bind_text(query, 1, abs_path, -1, NULL);

	int ret = sqlite3_step(query);

	if (query != NULL)
		sqlite3_finalize(query);

	if(ret == SQLITE_ROW)
		return 0;
	else
		return -1;
}

// compares latest tracked revision against current one (md5)
// contraintuively, this will return (char*) md5 of file
char *db_check_file_for_changes_md5(char *abs_path)
{
	sqlite3_stmt *query;
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
	sqlite3_stmt *query;
	sqlite3_prepare_v2(pDB, "SELECT * from file_version where hash == ?1 AND mtime == ?2", -1, &query, NULL);
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
	if(query)
		sqlite3_finalize(query);
	
	return ret;
}

// stops tracking file, but still retains data
int db_set_file_tracking(const char *abs_path, const char *hash, bool value)
{
	int ret;
	sqlite3_stmt *query;

	// either abs_path or hash should be NULL
	if((hash && abs_path) || (!hash && !abs_path))
		return -1;

	if(hash)
	{
		sqlite3_prepare_v2(pDB, "UPDATE file SET tracked = ?1 WHERE hash = ?2", -1, &query, NULL);
		sqlite3_bind_int(query, 1, (value ? 1:0));
		sqlite3_bind_text(query, 2, hash, -1, NULL);
	}
	else // if abs path
	{
		sqlite3_prepare_v2(pDB, "UPDATE file SET tracked = ?1 WHERE path = ?2", -1, &query, NULL);
		sqlite3_bind_int(query, 1, (value ? 1:0));
		sqlite3_bind_text(query, 2, abs_path,-1,NULL);
	}

	if(sqlite3_step(query) != SQLITE_DONE)
		ret = -1;
	else
		ret = 0;

	if(query)
		sqlite3_finalize(query);

	return ret;
}

int db_check_file_tracking(const char *abs_path, const char *hash)
{
	int ret;
	sqlite3_stmt *query;

	// either abs_path or hash should be NULL
	if((hash && abs_path) || (!hash && !abs_path))
		return -1;

	if(hash)
	{
		sqlite3_prepare_v2(pDB, "SELECT * FROM file WHERE hash = ?1 AND tracked = 1;", -1, &query, NULL);
		sqlite3_bind_text(query, 1, hash, -1, NULL);
	}
	else // if abs path
	{
		sqlite3_prepare_v2(pDB, "SELECT * FROM file WHERE path = ?1 AND tracked = 1;", -1, &query, NULL);
		sqlite3_bind_text(query, 1, abs_path,-1,NULL);
	}

	int sql_ret = sqlite3_step(query);

	if(sql_ret != SQLITE_ROW || sql_ret != SQLITE_ROW)
		ret = -1;
	else
		ret = 0;

	if(query)
		sqlite3_finalize(query);

	return ret;
}

unsigned char *db_query_path_from_fv_id(int id)
{
	sqlite3_stmt *query;
	unsigned char *ret = NULL;

	sqlite3_prepare_v2(pDB,
	                   "SELECT f.path FROM file f 				\
	                   INNER JOIN						\
	                   (SELECT hash FROM file_version WHERE id = ?1) fv	\
	                   ON fv.hash == f.hash",
	                   -1,&query, NULL);
	sqlite3_bind_int(query,1,id);

	if(sqlite3_step(query) != SQLITE_ROW)
	{
		//error, this should return exactly one record
	}
	else
	{
		// get filepath length (extra 1byte for terminating null character)
		int bytes=sqlite3_column_bytes(query,0) +1;
		// todo: checking
		ret = malloc(bytes);
		//copy query content into return buffer
		strncpy(ret,sqlite3_column_text(query,0),bytes);

		// todo this shouldn't be necessary
		ret[bytes-1]= '\0';
		//cleanup
		sqlite3_finalize(query);
	}
	PRINT(DEBUG,"db_query_path_from_fv_id(%d) returns '%s'\n",id,ret);
	return ret;
}

unsigned char *db_query_backup_path_from_fv_id(int id)
{
	sqlite3_stmt *query;
	unsigned char *ret = NULL;

	sqlite3_prepare_v2(pDB,"SELECT hash,md5 FROM file_version WHERE id = ?1",
	                   -1,&query,NULL);
	sqlite3_bind_int(query,1,id);

	// forge path string via asprintf and return it
	if(sqlite3_step(query) == SQLITE_ROW)
		asprintf(&ret,"%s/%s/%s",data_path,sqlite3_column_text(query,0), sqlite3_column_text(query,1));
	PRINT(DEBUG,"db_query_backup_path_from_fv_id(%d) returns '%s'\n",id,ret);
	return ret;
}

// copies files from tracked by snapshot into dest path
// if dest_path is NULL, then overwrites originals with backed up copies\
// this function more or less belongs to database/database.c, but
// its in database/sqlite.c due to need for db queries
// returns positive value (inc zero) if any files were recovered
int db_export_snapshot(int snapshot_id, char *dest_path)
{
	sqlite3_stmt *query_fvs;
	int ret;

	// return 'id's of all file versions contained by snapshot with 'snapshot_id'
	sqlite3_prepare_v2(pDB,"\
	                   SELECT fv.id FROM file_version fv 		\
	                   INNER JOIN 					\
	                   (						\
	                   	   SELECT fv_id FROM snapshot_file 	\
	                   	   WHERE s_time = ?1 			\
			   ) fvs					\
	                   ON fvs.fv_id = fv.id",
	                   -1, &query_fvs, NULL);
	sqlite3_bind_int(query_fvs,1,snapshot_id);

	while(sqlite3_step(query_fvs) == SQLITE_ROW)
	{
		ret ++;
		export_fv(sqlite3_column_int(query_fvs,0), dest_path);
	}
	return ret;
}


#ifdef _TEST

int main(void)
{
	db_open("./test.sqldb");
	return 0;
}

#endif
