/******************************************************************************
 * Author: Tomas Pruzina <pruzinat@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL SIMON TATHAM BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "md5/md5.h"
#include "track.h"
#include "file/file.h"

extern char *data_path;
extern char *enforce_md5;
extern char db_path[1024];

struct options
{
	int op;
	bool md5_enforce;
	char **next_arg;

} opts;


// PRINT MACRO
#define DEBUG 0
#define NOTICE 1
#define MESSAGE 2
#define ERROR 3

// global variable defining verbosity of printing functions
// todo: move this into struct options???
extern int log_level;
#define PRINT(x, ...) do { if(log_level <= (x)) fprintf(stdout, __VA_ARGS__ ); } while ( 0 )


// RETURN CODES (to prevent confusion wheather f(arg) == 0 means OK/ERROR)
#define EOK 0
#define EERR -1

// main action enum
enum actions
{
	TRACK_NULL, // do nothing -- default
	TRACK_HELP, // print help
	TRACK_ADD, // add file/directory
	TRACK_EXPORT, // export backups from snapshot (or without argument, export most recent backups)
	TRACK_DIFF, // print out files that changed since last commit to db
	TRACK_SHOW, // print various info about object (file) -- not fully implemented just yet
	TRACK_RM, // remove/stop tracking file (files covered by snapshots are not destroyed, nor are their records)
	TRACK_SNAPSHOT, // create snapshot of all files that are currently tracked
	TRACK_VERIFY, // verify integrity of backups and print potential problems (todo)
	TRACK_INIT, // create new database and backup directory (./.track by default)
	TRACK_GC // remove all backups that arent linked by snapshots and are NOT tracked
};

char *generate_random_string(ssize_t length);
void print_time(time_t time);

// takes copies content of str into dynamically allocated buffer and returns
// pointer to it (this is required for environment variables that are RO)
char *save_string_into_buffer(const char *str);

#endif /* __COMMON_H__ */
