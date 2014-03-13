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

#include "md5.h"


int md5_calculate_hash(const char *filename, unsigned char hash[MD5_DIGEST_LENGTH])
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

#ifdef _DEBUG
	for(int i = 0; i < MD5_DIGEST_LENGTH; i++)
		printf("%02x", hash[i]);
	printf (" %s\n", filename);
#endif
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

#ifdef _DEBUG
	for(int i = 0; i < MD5_DIGEST_LENGTH; i++)
		printf("%02x", hash[i]);
	printf (" \"%s\"\n", string);
#endif

	return 0;
}

char *md5_sanitized_hash (unsigned char hash[MD5_DIGEST_LENGTH])
{
	char *sanitized_hash = malloc(MD5_DIGEST_LENGTH*2+1);
    	
	static char buffer[MD5_DIGEST_LENGTH*2+1] = {0};
    	for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        	sprintf(&buffer[i*2], "%02x", (unsigned int)hash[i]);
	strncpy(sanitized_hash, buffer, MD5_DIGEST_LENGTH*2+1);
	return sanitized_hash;
}

#ifdef _TEST

int main()
{
	unsigned char hash[MD5_DIGEST_LENGTH];
	md5_calculate_hash("./md5.c",hash);
	return 0;
}

#endif


