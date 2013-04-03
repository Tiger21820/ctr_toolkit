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
#else
	#include <sys/stat.h>
	#include <unistd.h>
#endif

#include "types.h"
#include "utils.h"

#define IO_FAIL 1

#define FALSE 0
#define TRUE 1

#define INVALID -1
