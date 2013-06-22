/**
Copyright 2013 3DSGuy

This file is part of extdata_tool.

extdata_tool is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

extdata_tool is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with extdata_tool.  If not, see <http://www.gnu.org/licenses/>.
**/
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#ifdef _WIN32
	#include <io.h>
	#include <direct.h>
	#include <windows.h>
	#include <wchar.h>
#else
	#include <sys/stat.h>
	#include <unistd.h>
#endif

#include "types.h"
#include "utils.h"
#include "ExtData.h"
#include "ctx.h"


#define IO_FAIL 1

#ifndef _VARIABLES_
#define _VARIABLES_
typedef enum
{
	False = 0,
	True = 1
} _bool;

typedef enum
{
	BIG_ENDIAN = 0,
	LITTLE_ENDIAN = 1,
	BE = 0,
	LE = 1
} endian_types;


typedef enum
{
	Invalid = -1,
	Valid = 0
}Validity;
#endif