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
#include "types.h"

#define CBMD_MAGIC 0x444D4243
#define CWAV_MAGIC 0x56415743

#define READ_FAIL 1
#define WRITE_FAIL 2

#define SIZE_OFFSET 0x84

typedef struct
{
	DWORD magic;
	WORD endianness;
	u8 unused[6];
	DWORD file_size;
} __attribute__((__packed__)) 
CWAV_HEADER;

typedef struct
{
	DWORD magic;
	u8 unused[0x80];
	DWORD file_size;
} __attribute__((__packed__)) 
CBMD_HEADER;

typedef struct
{
	int RESULT_CODE;
	u8 verbose_bool;

	//INPUT
	FILE *cbmd; //Banner Graphics File
	CBMD_HEADER cbmd_header;
	FILE *cwav; //Banner Audio File
	CWAV_HEADER cwav_header;

	//OUTPUT	
	FILE *output; //Output .BNR file
} __attribute__((__packed__)) 
BNR_CONTEXT; //Context for BNR creation

//Prototypes
int bnr_main(BNR_CONTEXT bnr);
DWORD size_u8toDWORD(u8 *size);