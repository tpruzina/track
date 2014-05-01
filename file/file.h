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

#ifndef __FILE_H__
#define __FILE_H__

#include "../common.h"

// copy file by using mmap
int local_copy(const char *src, const char *dest);

// uses function above, but also creates directories as necessary
int copy(const char *src, const char *dest);

// recursive mkdir (much like 'mkdir -p')
int _mkdir(char *path);

// returns mtime of file given path (-1 on error)
int file_get_mtime(char *path);

// execute (*function) on every file in directory
// for recursively adding files inside directories
void do_in_dir(char *dirpath, int (*f)(const char *));

#endif /* __FILE_H__ */
