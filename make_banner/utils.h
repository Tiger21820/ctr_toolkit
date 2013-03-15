#include "yaml.h"

#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1

#define HEX 16
#define DEC 10

void resolve_flag(unsigned char flag, unsigned char *flag_bool);

void char_to_int_array(unsigned char destination[], unsigned char source[], int size, int endianness, int base);
void endian_strcpy(unsigned char destination[], unsigned char source[], int size, int endianness);

//like fgets(), except tabs are ignored
char *search(char s[], int n, FILE *file);

//Error printing functions
void value_find_fail(const char key[]);
void key_find_fail(const char category[]);
