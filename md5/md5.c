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

// fills hash buffer with raw md5 (given path to file)
int md5_calculate_hash_from_file(const char *filename, unsigned char hash[MD5_DIGEST_LENGTH])
{
	FILE *inFile = fopen (filename, "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];

	if (!inFile)
        	return EERR;

	MD5_Init(&mdContext);
	while ((bytes = fread (data, 1, 1024, inFile)) != 0)
		MD5_Update(&mdContext, data, bytes);
	MD5_Final(hash,&mdContext);

	fclose (inFile);
	return EOK;
}

// fills hash buffer with raw md5 (given string)
int md5_calculate_hash_from_string(const char *string, unsigned char hash[MD5_DIGEST_LENGTH])
{
	if(!string)
		return EERR;
	MD5_CTX mdContext;
	MD5_Init(&mdContext);
	MD5_Update(&mdContext, string, strlen(string));
	MD5_Final(hash, &mdContext);

	return EOK;
}

// sanitizes md5 (byte array -> alphanum ascii)
char *md5_sanitized_hash_of_file (char *file_path)
{
	unsigned char buffer[MD5_DIGEST_LENGTH]= {0};
	md5_calculate_hash_from_file(file_path, buffer);

	// take byte-hash from buffer and make nice string
	char *sanitized_hash = malloc(MD5_DIGEST_LENGTH*2+1);
	for(int i=0; i < MD5_DIGEST_LENGTH; i++)
		sprintf(&sanitized_hash[i*2], "%02x", (unsigned char)buffer[i]);
	// '0' terminate
	sanitized_hash[32]='\0';
	return sanitized_hash;
	
}

// sanitizes md5 (byte array -> alphanum ascii)
char *md5_sanitized_hash_of_string(char *string)
{
	unsigned char buffer[MD5_DIGEST_LENGTH] = {0};
	md5_calculate_hash_from_string(string, buffer);

	char *sanitized_hash = malloc(MD5_DIGEST_LENGTH*2+1);
	for(int i=0; i < MD5_DIGEST_LENGTH; i++)
		sprintf(&sanitized_hash[i*2], "%02x", (unsigned char)buffer[i]);
	// '0' terminate
	sanitized_hash[32]='\0';
	return sanitized_hash;
}

