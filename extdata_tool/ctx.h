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
//Global Sizes
typedef enum
{
	IO_PATH_LEN = 0x400,
	EXTDATA_FS_MAX_PATH_LEN = 0x100
} global_sizes;

//Variable Structs

typedef struct
{
	u8 used;
	FILE *file;
} F_OPTION_CTX;

typedef struct
{
	u8 used;
	char *argument;
	u8 arg_len;
	F_OPTION_CTX file;
} OPTION_CTX;

typedef struct
{
	u8 used;
	u8 *buffer;
	u64 size;
} COMPONENT_STRUCT;

typedef enum
{
	Image = 1,
	Directory
} modes;

typedef enum
{
	WIN_32 = 0x5C,
	UNIX = 0x2F
} platform;


typedef enum
{
	primary = 0,
	secondary = 1
} difi_partitions;

typedef struct
{
	//Regular Options
	u8 info;
	u8 extract;
	u8 fs_info;
	u8 titledb_read;
	u8 listdb;
	
	char *input;
	char *output;	
	
	
	
	//Stored Data
	char cwd[IO_PATH_LEN];
	u8 platform;
	u8 mode;
	char *extdataimg_path;
	FILE *extdataimg;
	EXTDATA_CONTEXT data;
} INPUT_CONTEXT;

