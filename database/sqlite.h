#ifndef _SQLITE_HEADER
#define _SQLITE_HEADER

#include <sqlite3.h>
#include <stdbool.h>

void db_close(void);
void db_commit(void);

int db_open(const char *path);

int db_add_file(char *path, char *sanitized_hash, char *md5, long mtime);
int db_query_file(const char *path);

int db_set_file_tracking(const char *abs_path, const char *hash, bool value);
int db_check_file_tracking(const char *abs_path, const char *hash);

int db_file_get_newest_mtime(char *hash);
char *db_file_get_newest_md5(char *hash);

int db_create_snapshot_record(long t, char *desc);
int db_create_snapshot(long t);

int db_list_file_versions(char *hash);

int db_add_file_record(char *hash, char *md5, long mtime);

int db_showchanged_files_mtime();
int db_showchanged_files_md5();

char *db_query_path_from_fv_id(int id);
char *db_query_backup_path_from_fv_id(int id);

int db_export_snapshot(int snapshot_id, char *dest_path);

bool db_query_file_in_snapshot(char *hash);
int db_remove_file_fv(int id);
int db_remove_file(char *hash);

#endif
