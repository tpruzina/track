#ifndef _MD5_HEADER
#define _MD5_HEADER

#include <openssl/md5.h>

int md5_calculate_hash(const char *filename, unsigned char hash[MD5_DIGEST_LENGTH]);

#endif
