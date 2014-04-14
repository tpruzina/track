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

#include "common.h"

// global variables
char *data_path;
char *enforce_md5;
char db_path[1024];

int log_level;

struct options opts = { TRACK_HELP, false, NULL };

// random string generator
char *generate_random_string(ssize_t length)
{
	int urandom_fd = open("/dev/urandom", O_RDONLY);
	
	char *result = malloc(length+1);
	if(!result)
		exit(EXIT_FAILURE);

	if(read(urandom_fd, result, length) != length)
		exit(EXIT_FAILURE);
	
	// TODO test correctness of approach bellow (off by one)
	// convert random bytes into alfa format (a-z,A-Z) (usable by filesystems)
	for(unsigned int i=0; i < length; i++)
		result[i] = ((unsigned char) result[i] % 57) + 'A';
	
	// zero terminate string
	result[length] = '\0';

	close(urandom_fd);

	return result;
}

void print_time(time_t time)
{
	const char *t = ctime(&time);
	printf("%s",t);

}
