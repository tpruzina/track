#ifndef _MD5_HEADER
#define _MD5_HEADER

#include <openssl/md5.h>

int md5_calculate_hash(const char *filename, unsigned char hash[MD5_DIGEST_LENGTH]);
int md5_calculate_hash_from_string(const char *string, unsigned char hash[MD5_DIGEST_LENGTH]);


char *md5_sanitized_hash (unsigned char hash[MD5_DIGEST_LENGTH]);

#endif
