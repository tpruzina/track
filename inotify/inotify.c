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

#include <sys/inotify.h>
#include <stdlib.h>
#include <unistd.h>

int inotify_fd = 0;

void inotify_shutdown()
{
	close(inotify_fd);
	return;
}

int inotify_initialize()
{
	inotify_fd = inotify_init();
	if(inotify_fd >= 0)
		atexit(inotify_shutdown);
	else
	{
		//error handling
	}
	return inotify_fd;
}

#ifdef _TEST

int main()
{
	int fd = inotify_initialize();
	
}

#endif

