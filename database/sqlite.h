#ifndef _SQLITE_HEADER
#define _SQLITE_HEADER

#include <sqlite3.h>

void db_close(void);
int db_open(const char *path);

#endif
