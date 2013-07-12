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
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#ifdef _WIN32
	#include <io.h>
	#include <direct.h>
	#include <windows.h>
	#include <wchar.h>
#else
	#include <sys/stat.h>
	#include <sys/types.h>
#endif

#include "types.h"
#include "utils.h"


