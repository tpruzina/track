#ifndef _DB_HEADER
#define _DB_HEADER

#include "../common.h"
#include "sqlite.h"

// extern pointer to hard/soft (mtime vs md5) compare function
extern int (*check_file_for_changes)(char*, char*);

// given id, restores full snapshot into given directory
int restore_snapshot(int id);

// add file into backup or update record
int track_file(const char *path);

int remove_file(const char *path);

// create current snapshot of files
int create_snapshot(char *desc);

// compare file with newest database record
int check_file_for_changes_mtime(char *abs_path, char *hash);
char *check_file_for_changes_md5(char *abs_path);

// list all backed up 
int list_file_versions(char *path);

// print statistics (todo)
int print_stats();

int export_fv(int id, char *dest_path);
int export_snapshot(int id, char *dest_path);

#endif
