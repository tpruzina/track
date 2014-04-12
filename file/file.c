/******************************************************************************
 *
 * TODO: Project Title
 *
 * Author: Tomas Pruzina <pruzinat@gmail.com>
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 ******************************************************************************/

#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include "file.h"

// copy file (assumes paths and permissions are resolved)
// TODO ERROR HANDLING
int local_copy(const char *src, const char *dest)
{
	struct stat ss;
	
	int src_fd = open(src, O_RDONLY);
	fstat(src_fd, &ss);

	int dest_fd = open(dest, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if(ftruncate(dest_fd, ss.st_size) != 0)
	{
		//error handling
	}

	void *src_addr = mmap(NULL, ss.st_size, PROT_READ, MAP_SHARED, src_fd, 0);
	if(!src_addr)
		exit(-2);
	
	void *dest_addr = mmap(NULL, ss.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, dest_fd, 0);
	if(!dest_addr)
		exit(-1);

	memcpy(dest_addr, src_addr,ss.st_size);

	munmap(dest_addr, ss.st_size);
	munmap(src_addr, ss.st_size);
	
	return 0;
}

// robust copy, creates directories as needed
int copy(const char *src, const char *dest)
{
	const char *delim = dest;
	char *dir_path = NULL;

	//points delim at the end of the string
	while(*delim)
		delim++;
	//moves "backwards" untill '/' is found
	while(*delim != '/')
		delim--;
	int dir_path_legth = delim - dest +1;
	dir_path = malloc(dir_path_legth);
	//todo: mem alloc checking
	strncpy(dir_path,dest,dir_path_legth);
	_mkdir(dir_path);
	free(dir_path);

	// directories are created, now copy files
	local_copy(src,dest);
	return 0;
}

// recursive mkdir
// path _must_ be writeable
int _mkdir(char *path)
{
	if(!path)
		return -1;


	for(char *p = path +1; *p; p++)
	if(*p == '/')
	{
		*p = '\0';
		mkdir(path,S_IRWXU);
		*p = '/';
	}
	return 0;
}
