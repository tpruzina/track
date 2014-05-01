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

// close database and connection (called before exit)
// does not commit changes if something goes wrong
void db_close(void)
{
	sqlite3_close(pDB);
	sqlite3_shutdown();
}

// commit database - if no error occured
void db_commit(void)
{
	sqlite3_exec(pDB,"COMMIT",0,0,0);
}

// open database file and create tables if needed
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
		PRINT(ERROR,"Failed to open db: %s\n", path);
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

	return EOK;
}

// given hash, lists all tracked revisions of file and prints them on stdout
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

// returns md5 hash of most recent file revision
// it's calees responsibility to free resulting buffer
char *db_get_newest_md5(char *hash)
{
	if(!hash || !pDB)
		return NULL;

	sqlite3_stmt *query_md5;
	sqlite3_prepare_v2(pDB,"select md5 from file_version where hash = $1 order by mtime DESC;",-1,&query_md5,NULL);
	sqlite3_bind_text(query_md5,1,hash, -1,NULL);

	int ret = sqlite3_step(query_md5);
	if(ret == SQLITE_ROW || ret == SQLITE_DONE)
	{
		char*string = malloc(MD5_DIGEST_LENGTH*2+1);
		if(!string)
			exit(EXIT_FAILURE);

		strcpy(string,(char*)sqlite3_column_text(query_md5,0));
		sqlite3_finalize(query_md5);
		return string;
	}
	else
		return NULL;
}

// returns mtime of most recent file revision (see db_get_newest_md5)
int db_get_newest_mtime(char *hash)
{
	if(!hash || !pDB)
		exit(EXIT_FAILURE);

	sqlite3_stmt *query_mtime;
	sqlite3_prepare_v2(pDB,"select mtime from file_version where hash = $1 order by mtime DESC;",-1,&query_mtime,NULL);
	sqlite3_bind_text(query_mtime,1,hash, -1,NULL);

	int ret = sqlite3_step(query_mtime);
	if(ret == SQLITE_ROW || ret == SQLITE_DONE)
	{
		int mtime = sqlite3_column_int(query_mtime,0);
		sqlite3_finalize(query_mtime);
		return mtime;
	}
	else
		return -1; // this should return something else, since -1 is legit (though in 2030) 
}

// yet another simple printing functon, prints every file that has mtime newer
// than what is in database
int db_showchanged_files_mtime()
{
	if(!pDB)
		exit(EXIT_FAILURE);

	sqlite3_stmt *query_files;

	sqlite3_prepare_v2(pDB, "select hash,path from file;",-1, &query_files, NULL);

	while(sqlite3_step(query_files) == SQLITE_ROW)
	{
		char *hash = (char *)sqlite3_column_text(query_files,0);
		char *path = (char *)sqlite3_column_text(query_files,1);

		int curr = file_get_mtime(path);
		int newest_in_db = db_get_newest_mtime(hash);

		if(curr != newest_in_db)
			PRINT(NOTICE,"changed:%s\t mtime_new=%d\t mtime_db: %d\n",path, curr, newest_in_db);
	}

	if(query_files)
		sqlite3_finalize(query_files);

	return EOK;
}

// prints every file with md5 newer than what's stored in database (this is not cheap)
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

	return EOK;
}

// adds new file revision into file_version table (it needs to be tracked)
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
	int ret = sqlite3_step(query);
	if(ret != SQLITE_DONE || ret != SQLITE_ROW)
		return EERR;

	if(query)
		sqlite3_finalize(query);

	return EOK;
}

// creates new snapshot
// desc can be set, if its NULL, then current time is used
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
	
	return EOK;
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
	
	if(SQLITE_DONE != ret)
		return EERR;

	if(file_query)
		sqlite3_finalize(file_query);

	return EOK;
}

// adds new file into database (both into table 'file' and 'file_version')
// this is used for adding new files
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
		return EERR;
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
		return EERR;
	}
	
	if (query_fv != NULL)
		sqlite3_finalize(query_fv);

	return EOK;
}

// checks wheather file hash record in 'file' table
int db_query_file(const char *abs_path)
{
	sqlite3_stmt *query;
	sqlite3_prepare_v2(pDB, "select * from file where path = ?1;", -1, &query, NULL);
	sqlite3_bind_text(query, 1, abs_path, -1, NULL);

	int ret = sqlite3_step(query);

	if (query != NULL)
		sqlite3_finalize(query);

	if(ret == SQLITE_ROW)
		return EOK;
	else
		return EERR;
}

// TODO: duplicite function????
int db_file_get_newest_mtime(char *hash)
{
	int ret=-1;
	sqlite3_stmt *query;

	sqlite3_prepare_v2(pDB, "select mtime from file_version where hash = ?1 order by mtime DESC;", -1, &query, NULL);
	sqlite3_bind_text(query, 1, hash, -1, NULL);

	if(sqlite3_step(query) == SQLITE_ROW)
	{
		ret = sqlite3_column_int(query,0);
		sqlite3_finalize(query);
	}
	return ret;
}

// TODO: duplicite function????
char *db_file_get_newest_md5(char *hash)
{
	sqlite3_stmt *query;
	char *ret = NULL;

	sqlite3_prepare_v2(pDB, "select md5 from file_version where hash = ?1 order by mtime DESC;", -1, &query, NULL);
	sqlite3_bind_text(query, 1, hash, -1, NULL);

	if(sqlite3_step(query) == SQLITE_ROW)
	{
		unsigned const char *new_md5 = sqlite3_column_text(query, 0);
		ret = malloc(sqlite3_column_bytes(query,0));
		if(ret)
			strcpy(ret,(char *)new_md5);
		sqlite3_finalize(query);
	}
	return ret;
}

// stops tracking file, but still retains data
int db_set_file_tracking(const char *abs_path, const char *hash, bool value)
{
	int ret;
	sqlite3_stmt *query;

	// either abs_path or hash should be NULL
	if((hash && abs_path) || (!hash && !abs_path))
		return EERR;

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

// query tracking status of a file (search by either hash or absolute_path)
int db_check_file_tracking(const char *abs_path, const char *hash)
{
	int ret;
	sqlite3_stmt *query;

	// either abs_path or hash should be NULL
	if((hash && abs_path) || (!hash && !abs_path))
		return EERR;

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

	if(sql_ret != SQLITE_ROW)
		ret = -1;
	else
		ret = 0;

	if(query)
		sqlite3_finalize(query);

	return ret;
}

// given file_version.id (revision), returns file path 
// used in exporting backups from snapshots
// returns dynamically allocated string on success, NULL on failure
char *db_query_path_from_fv_id(int id)
{
	sqlite3_stmt *query;
	char *ret = NULL;

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
		strncpy(ret,((char *)sqlite3_column_text(query,0)),bytes);

		// todo this shouldn't be necessary
		ret[bytes-1]= '\0';
		//cleanup
		sqlite3_finalize(query);
	}
	PRINT(DEBUG,"db_query_path_from_fv_id(%d) returns '%s'\n",id,ret);
	return ret;
}

// given file_version.id, returns string containing path to actuall
// backup of file (from .track directory)
// returns dynamically allocated string on success, NULL on failure
char *db_query_backup_path_from_fv_id(int id)
{
	sqlite3_stmt *query;
	char *ret = NULL;

	sqlite3_prepare_v2(pDB,"SELECT hash,md5 FROM file_version WHERE id = ?1",
	                   -1,&query,NULL);
	sqlite3_bind_int(query,1,id);

	// forge path string via asprintf and return it
	if(sqlite3_step(query) == SQLITE_ROW)
	{
		if(0 > asprintf(&ret,"%s/%s/%s",
		                data_path,sqlite3_column_text(query,0),
		                sqlite3_column_text(query,1))
		)
		return NULL;	// todo, better error handling
	}
	PRINT(DEBUG,"db_query_backup_path_from_fv_id(%d) returns '%s'\n",id,ret);
	return ret;
}

// copies files from tracked by snapshot into dest path
// if dest_path is NULL, then overwrites originals with backed up copies
// this function more or less belongs to database/database.c, but
// its in database/sqlite.c due to need for db queries
// returns positive value (inc zero) if any files were recovered
int db_export_snapshot(int snapshot_id, char *dest_path)
{
	sqlite3_stmt *query_fvs;
	int ret=-1;

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

// returns true if file exists in one or more snapshots
// false otherwise
// this is beneficial when we want to remove backups when cleaining up
// TODO: write par function querying doing the same for file_version rev.
bool db_query_file_in_snapshot(char *hash)
{
	if(!hash)
		return false;

	int ret;

	sqlite3_stmt *query;
	sqlite3_prepare_v2(pDB,"				\
			SELECT fv.id FROM file_version fv	\
	                INNER JOIN				\
			(					\
				SELECT fv_id FROM snapshot_file	\
			) fvs					\
			ON fvs.fv_id = fv.id AND fv.hash = ?1",
			-1, &query, NULL
	);
	sqlite3_bind_text(query, 1,hash,-1,NULL);

	ret = sqlite3_step(query);
	sqlite3_finalize(query);

	if (ret == SQLITE_ROW)
		return true;
	else
		return false;
}

// remove row from db:file
int db_remove_file(char *hash)
{
	sqlite3_stmt *delete_query;
	int ret=0;

	sqlite3_prepare_v2(pDB, "DELETE FROM file WHERE hash = ?1;",
	                   -1, &delete_query, NULL);
	sqlite3_bind_text(delete_query,1,hash,-1,NULL);

	ret = sqlite3_step(delete_query);
	sqlite3_finalize(delete_query);

	if(ret == SQLITE_DONE || ret == SQLITE_ROW)
		return 0;
	else
		return -1;

}

// removes a record from file_version
int db_remove_file_fv(int id)
{
	sqlite3_stmt *delete_query;
	int ret=0;

	sqlite3_prepare_v2(pDB,"DELETE FROM file_version fv WHERE id = ?1",
	                   -1,&delete_query,NULL);
	sqlite3_bind_int(delete_query,1,id);

	ret = sqlite3_step(delete_query);
	sqlite3_finalize(delete_query);
	
	if(ret == SQLITE_ROW || ret == SQLITE_DONE)
		return 0;
	else
		return -1;
}
