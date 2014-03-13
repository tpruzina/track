#ifndef _SQLITE_HEADER
#define _SQLITE_HEADER

#include <sqlite3.h>

int db_open(const char *path);

int track_file(const char *file);
int untrack_file(const char *file);
int delete_file(const char *file);

#endif
