/******************************************************************************
 *
 * TRACK
 * database/database.c
 * This file should contain handling functions for work with database,
 * and call sqlite3 functions from database/sqlite.c
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
#include <time.h>
#include <stdbool.h>

#include "track.h"
#include "database/sqlite.h"

int restore_snapshot(int id)
{
	// works the same as export_snapshot, except it works "in place"
	// e.g. no export directory prefix is used.
	export_snapshot(id,"");
	return EOK;
}


int create_snapshot(char *desc)
{
	//todo: error handling
	long t = time(NULL);
	if(desc == NULL)
		desc = ctime(&t);

	db_create_snapshot_record(t,desc);
	db_create_snapshot(t);

	return EOK;
}

int list_file_versions(char *path)
{
	char *abs_path = realpath(path,NULL);
	char *hash = md5_sanitized_hash_of_string(abs_path);

	int ret = db_list_file_versions(hash);

	free(hash);
	free(abs_path);

	return ret;
}

int print_stats()
{
	return EOK;
}

/* 
 * Add file into database and make backup
 * if file is already in database, then check for changes and commit them
  (if any)
 */
int track_file(const char *path)
{
	// get canonical (full) path to file
	char *abs_path = realpath(path, NULL);
	// if anything went wrong, yield error and exit function
	if(!abs_path && (errno == EACCES || errno == ENOENT || errno == EEXIST))
	{
		fflush(stdout);
		fprintf(stderr,"%s: %s\n",
		        path,
		        (errno == EACCES) ? "no read permissions." :
		        (errno == ENOENT) ? "doesn't exist." :
		        "unknown error."
		);
		return EERR;
	}

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
	else
	{
		if(!S_ISREG(st.st_mode))
		{
			PRINT(DEBUG,"%s isn't a regular file!\n",abs_path);
			return EOK;
		}
	}

	// check wheather file isn't tracked already
	if(db_query_file(abs_path) == 0)
	{
		PRINT(DEBUG, "found %s in database\n", abs_path);
		//if((md5 = check_file_for_changes_md5(abs_path)) != NULL)
		if(check_file_for_changes(abs_path, opts.md5_enforce))
		{
			// file has changed - add new version to database!
			PRINT(NOTICE,"updating %s\n",abs_path);

			md5 = md5_sanitized_hash_of_file(abs_path);
			snprintf(backup_path, sizeof(backup_path), "%s/%s/%s",data_path,hash,md5);

			local_copy(abs_path, backup_path);
			
			db_add_file_record(hash, md5, st.st_mtime);
		}
		else	// file is the same
			// e.g. do nothing
			PRINT(DEBUG,"%s hasn't changed\n",abs_path);
	}
	else // file isn't tracked yet - track it!
	{
		PRINT(DEBUG,"%s not found in database\n", abs_path);
	
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

		PRINT(NOTICE,"%s added to database\n", path);

	}
	free(abs_path);

	if(hash)
		free(hash);
	if(md5)
		free(md5);

	return EOK;
}

int remove_file(const char *path)
{
	// todo: since multiple files may have same md5 (copies),
	// make damn sure we don't destroy backup copies that are linked by something else

	char *abs_path = realpath(path,NULL);

	// if file isn't tracked.. nothing to do
	if(db_query_file(abs_path) == -1)
		goto cleanup;


	db_set_file_tracking(abs_path,NULL,false);

	//TODO: physically remove files
	// if file is tracked, attempt to remove each version of the file
	// from both backup folder and database

	char *hash = md5_sanitized_hash_of_string(realpath(path,NULL));

	
cleanup:
	free(abs_path);
	return EOK;
}

// compares current file revision at abs_path with latest tracked file
// returns 0 if file hasnt changed, -1 else
// @enforce_md5, if true, compares by md5, if false, compares by mtime (cheap)
int check_file_for_changes(char *abs_path, bool enforce_md5)
{
	int ret;
	char *hash = md5_sanitized_hash_of_string(abs_path);
	if(!hash)
		ret = -1;

	if(enforce_md5)
	{
		char *md5_old = db_file_get_newest_md5(hash);
		char *md5_new = md5_sanitized_hash_of_file(abs_path);

		if(strcmp(md5_new,md5_old) == 0)
			ret = 0;
		else
			ret = -1;

		free(md5_old);
		free(md5_new);
	}
	else
	{
		struct stat st;
		if(stat(abs_path, &st) == -1)
			ret = -1;
		else
		{
			int mtime_new = db_file_get_newest_mtime(hash);
			if(mtime_new == st.st_mtime)
				ret = 0;
			else
				ret = -1;
		}
	}

	free(hash);
	return ret;
}

// assumes dest_path exists and user has permissions (see export_snapshot())
int export_fv(int id, char *dest_path)
{
	// this aint gonna work
	if(!dest_path || id <= 0)
		return EERR;

	// full destination path of file
	char *dest = NULL;
	// full source path of file (backup path)
	char *src = db_query_backup_path_from_fv_id(id);

	// now forge dest path
	// 1. get original path from db
	char *orig_path = db_query_path_from_fv_id(id);
	if(!orig_path)
		return EERR;

	// dest = $dest_path/$(original path)
	if(0 > asprintf(&dest,"%s%s",dest_path, orig_path))
		goto cleanup;

	copy(src,dest);

cleanup:
	// cleanup
	free(dest);
	free(src);
	free(orig_path);

	return EOK;
}

int export_snapshot(int snapshot_id, char *dest_path)
{
	int ret = -1;
	// verify snapshot exist

	// make sure that destination path exists (or create it)
	// due to hackish nature of _mkdir, writeable path is required - make one
	char *writeable_dest_path = malloc(strlen(dest_path));
	strcpy(writeable_dest_path,dest_path);

	_mkdir(writeable_dest_path);

	// for each tracked file:
	//	from snapshot_file, copy backed up file onto new location
	ret = db_export_snapshot(snapshot_id, dest_path);

	free(writeable_dest_path);
	return ret;
}

