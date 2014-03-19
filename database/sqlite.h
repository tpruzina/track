#ifndef _SQLITE_HEADER
#define _SQLITE_HEADER

#include <sqlite3.h>

void db_close(void);
int db_open(const char *path);

int db_add_file(char *path, char *sanitized_hash, char *md5, long mtime);
int db_query_file(const char *path);

int db_check_file_for_changes_mtime(char *abs_path, long mtime);
char *db_check_file_for_changes_md5(char *abs_path);

int db_create_snapshot_record(long t, char *desc);
int db_create_snapshot(long t);

int db_update_file_record(char *hash, char *md5, long mtime);

#endif
