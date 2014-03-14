#ifndef _MD5_HEADER
#define _MD5_HEADER

#include <openssl/md5.h>

// these calculate raw hash into buffer 'hash'
int md5_calculate_hash(const char *filename, unsigned char hash[MD5_DIGEST_LENGTH]);
int md5_calculate_hash_from_string(const char *string, unsigned char hash[MD5_DIGEST_LENGTH]);

// return nice hash string in hexadecimal (same as md5sum)
char *md5_sanitized_hash_of_file(char *file_path);
char *md5_sanitized_hash_of_string(char *string);

char *md5_sanitized_hash (unsigned char hash[MD5_DIGEST_LENGTH]);

#endif
