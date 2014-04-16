#ifndef _DB_HEADER
#define _DB_HEADER

#include "common.h"
#include "database/sqlite.h"


// given id, restores full snapshot into given directory
int restore_snapshot(int id);

// add file into backup or update record
int track_file(const char *path);

int remove_file(const char *path);

// create current snapshot of files
int create_snapshot(char *desc);

// compare file with newest database record
int check_file_for_changes(char *abs_path, bool enforce_md5);


// list all backed up 
int list_file_versions(char *path);

// print statistics (todo)
int print_stats();

// export file from database onto specified location
int export_fv(int id, char *dest_path);

// export whole snapshot onto specified destination
int export_snapshot(int id, char *dest_path);

#endif
