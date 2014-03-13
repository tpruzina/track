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

#include "md5.h"


int md5_calculate_hash(const char *filename, unsigned char hash[MD5_DIGEST_LENGTH])
{
	int i;
	FILE *inFile = fopen (filename, "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];

	if (!inFile)
        	return -1;

	MD5_Init (&mdContext);
	while ((bytes = fread (data, 1, 1024, inFile)) != 0)
		MD5_Update (&mdContext, data, bytes);
	MD5_Final (hash,&mdContext);

	for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", hash[i]);
		printf (" %s\n", filename);
	fclose (inFile);
	return 0;
}


#ifdef _TEST

int main()
{
	unsigned char hash[MD5_DIGEST_LENGTH];
	md5_calculate_hash("./md5.c",hash);
	return 0;
}

#endif


