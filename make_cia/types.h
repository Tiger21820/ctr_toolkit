/**
Copyright 2013 3DSGuy

This file is part of make_cdn_cia.

make_cdn_cia is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

make_cdn_cia is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with make_cdn_cia.  If not, see <http://www.gnu.org/licenses/>.
**/
#include <stdlib.h>
#include <stdint.h>
//Bools
#define TRUE 1
#define FALSE 0

//Errors
#define IO_FAIL 1
#define FILE_PROCESS_FAIL 2
#define ARGC_FAIL 3
#define ARGV_FAIL 4
#define Good 0
#define Fail 1

#define MAKE 1
#define READ 2
#define EXTRACT 3

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef unsigned long long      u64;

typedef signed char     s8;
typedef signed short    s16;
typedef signed int      s32;
typedef signed long long        s64;
