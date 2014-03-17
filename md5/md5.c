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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "md5.h"


int md5_calculate_hash_from_file(const char *filename, unsigned char hash[MD5_DIGEST_LENGTH])
{
	FILE *inFile = fopen (filename, "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];

	if (!inFile)
        	return -1;

	MD5_Init(&mdContext);
	while ((bytes = fread (data, 1, 1024, inFile)) != 0)
		MD5_Update(&mdContext, data, bytes);
	MD5_Final(hash,&mdContext);

	fclose (inFile);
	return 0;
}

int md5_calculate_hash_from_string(const char *string, unsigned char hash[MD5_DIGEST_LENGTH])
{
	if(!string)
		return -1;
	MD5_CTX mdContext;
	MD5_Init(&mdContext);
	MD5_Update(&mdContext, string, strlen(string));
	MD5_Final(hash, &mdContext);

	return 0;
}

char *md5_sanitized_hash_of_file (char *file_path)
{
	unsigned char buffer[MD5_DIGEST_LENGTH];
	md5_calculate_hash_from_file(file_path, buffer);

	// take byte-hash from buffer and make nice string
	char *sanitized_hash = malloc(MD5_DIGEST_LENGTH*2+1);
	for(int i=0; i < MD5_DIGEST_LENGTH; i++)
		sprintf(&sanitized_hash[i*2], "%02x", (unsigned char)buffer[i]);
	// '0' terminate
	sanitized_hash[32]='\0';
	return sanitized_hash;
	
}

char *md5_sanitized_hash_of_string(char *string)
{
	unsigned char buffer[MD5_DIGEST_LENGTH];
	md5_calculate_hash_from_string(string, buffer);

	char *sanitized_hash = malloc(MD5_DIGEST_LENGTH*2+1);
	for(int i=0; i < MD5_DIGEST_LENGTH; i++)
		sprintf(&sanitized_hash[i*2], "%02x", (unsigned char)buffer[i]);
	// '0' terminate
	sanitized_hash[32]='\0';
	return sanitized_hash;
}

#ifdef _TEST

int main()
{
	printf("%s\n",md5_sanitized_hash_of_file("md5.c"));
	printf("%s\n",md5_sanitized_hash_of_string("md5.c"));
	return 0;
}

#endif


