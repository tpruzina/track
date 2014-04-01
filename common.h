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

#include "md5/md5.h"
#include "database/database.h"
#include "file/file.h"

extern char *data_path;
extern char db_path[1024];

// PRINT MACRO
#define DEBUG 0
#define NOTICE 1
#define MESSAGE 2
#define ERROR 3

extern int log_level;
#define PRINT(x, ...) do { if(log_level <= (x)) fprintf(stdout, __VA_ARGS__ ); } while ( 0 )


enum actions
{
	TRACK_HELP,
	TRACK_ADD,
	TRACK_SYNC,
	TRACK_RM,
	TRACK_SNAPSHOT,
	TRACK_VERIFY
};

char *generate_random_string(ssize_t length);

void print_time(time_t time);

#endif /* __COMMON_H__ */

