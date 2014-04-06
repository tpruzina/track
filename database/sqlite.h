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

int db_check_file_for_changes_mtime(char *abs_path, long mtime);
char *db_check_file_for_changes_md5(char *abs_path);

int db_create_snapshot_record(long t, char *desc);
int db_create_snapshot(long t);

int db_list_file_versions(char *hash);

int db_add_file_record(char *hash, char *md5, long mtime);

int db_showchanged_files_md5();

unsigned char *db_query_path_from_fv_id(int id);
unsigned char *db_query_backup_path_from_fv_id(int id);

int db_export_snapshot(int snapshot_id, char *dest_path);

#endif
