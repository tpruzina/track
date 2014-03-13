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
}



