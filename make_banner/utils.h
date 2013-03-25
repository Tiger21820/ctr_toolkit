/**
Copyright 2013 3DSGuy

This file is part of make_banner.

make_banner is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

make_banner is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with make_banner.  If not, see <http://www.gnu.org/licenses/>.
**/
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
