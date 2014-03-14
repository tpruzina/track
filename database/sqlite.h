#ifndef _SQLITE_HEADER
#define _SQLITE_HEADER

#include <sqlite3.h>

void db_close(void);
int db_open(const char *path);

int db_add_file(char *path, char *sanitized_hash, char *md5, long mtime);

#endif
