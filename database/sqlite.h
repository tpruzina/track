#ifndef _SQLITE_HEADER
#define _SQLITE_HEADER

#include <sqlite3.h>
#include <stdbool.h>

// manipulate database (open,close,commit)
void db_close(void);
void db_commit(void);
int db_open(const char *path);

// adding new files into db
int db_add_file(char *path, char *sanitized_hash, char *md5, long mtime);
int db_add_file_record(char *hash, char *md5, long mtime);

// query file record by path (on negative retval, file isnt tracked)
int db_query_file(const char *path);

// track/untrack file (record is kept because it may persist in snapshot)
int db_set_file_tracking(const char *abs_path, const char *hash, bool value);
int db_check_file_tracking(const char *abs_path, const char *hash);

// returns mtime/md5 of newest file revisions stored in db
int db_file_get_newest_mtime(char *hash);
char *db_file_get_newest_md5(char *hash);

// creates snapshot (t=mtime)
int db_create_snapshot_record(long t, char *desc);
// same as above, used if no description is given
int db_create_snapshot(long t);

// prints all tracked revisions given hash (file.hash)
int db_list_file_versions(char *hash);

// printing functions (used by track --diff)
int db_showchanged_files_mtime();
int db_showchanged_files_md5();

// snapshot export related functions
// returning paths to original file and backups
char *db_query_path_from_fv_id(int id);
char *db_query_backup_path_from_fv_id(int id);

// copies everything in snapshot onto destination path
// todo: extend by strip option
// if dest_path is NULL, then overwrites original files
int db_export_snapshot(int snapshot_id, char *dest_path);

// cleanup functions
bool db_query_file_in_snapshot(char *hash);
int db_remove_file_fv(int id);
int db_remove_file(char *hash);

#endif
