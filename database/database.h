#ifndef _DB_HEADER
#define _DB_HEADER

#include "../common.h"
#include "sqlite.h"

int track_file(const char *path);
int untrack_file(const char *file);
int delete_file(const char *file);

#endif
