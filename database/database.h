#ifndef _DB_HEADER
#define _DB_HEADER

#include "../common.h"
#include "sqlite.h"

int track_file(const char *path);

int create_snapshot(char *desc);

int check_file_for_changes_mtime(char *abs_path, char *hash);
char *check_file_for_changes_md5(char *abs_path);


#endif
