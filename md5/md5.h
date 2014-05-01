#ifndef _MD5_HEADER
#define _MD5_HEADER

#include <openssl/md5.h>

#include "../common.h"

// return nice hash string in hexadecimal (same as md5sum)
char *md5_sanitized_hash_of_file(char *file_path);
char *md5_sanitized_hash_of_string(char *string);


#endif
