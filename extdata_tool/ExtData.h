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
#ifndef _EXTDATA_H_
#define _EXTDATA_H_
#include "types.h"

typedef enum
{
	DIFF_MAGIC_0 = 0x46464944,
	DIFF_MAGIC_1 = 0x30000,
	DISA_MAGIC = 0x41534944,
} DIFF_data;

typedef enum
{
	DIFI_MAGIC_0 = 0x49464944,
	DIFI_MAGIC_1 = 0x10000,
} DIFI_data;

typedef enum
{
	IVFC_MAGIC_0 = 0x43465649,
	IVFC_MAGIC_1 = 0x20000,
} IVFC_data;

typedef enum
{
	DPFS_MAGIC_0 = 0x53465044,
	DPFS_MAGIC_1 = 0x10000,
} DPFS_data;

typedef enum
{
	PRIMARY,
	SECONDARY
} partition;

typedef enum
{
	DIFF_CORRUPT = 10,
	DIFI_CORRUPT = 11,
	IVFC_CORRUPT = 12,
	DPFS_CORRUPT = 13,
	UNEXPECTED_MULTIPLE_DATA_IN_EXTDATA = 14,
} extdata_errors;

typedef struct
{
	u32 magic_0;
	u32 magic_1;
	u64 secondary_partition_offset;
	u64 primary_partition_offset;
	u64 partition_table_length;
	u64 active_table_offset;
	u64 file_base_size;
	u8 reserved_0[0x4];
	u8 active_table_hash[0x20];
	u8 reserved_1[0xAC];
} DIFF_STRUCT;

typedef struct
{
	u32 magic_0;
	u32 magic_1;
	u64 ivfc_blob_offset;
	u64 ivfc_blob_size;
	u64 dpfs_blob_offset;
	u64 dpfs_blob_size;
	u64 hash_offset;
	u64 hash_size;
	u8 flags[4];
	u64 data_partition_offset;
} DIFI_STRUCT;

typedef struct
{
	u32 magic_0;
	u32 magic_1;
	u64 master_hash_size;
	u64 level_1_relative_offset;
	u64 level_1_hashdata_size;
	u32 level_1_block_size;
	u32 reserved_0;
	u64 level_2_relative_offset;
	u64 level_2_hashdata_size;
	u32 level_2_block_size;
	u32 reserved_1;
	u64 level_3_relative_offset;
	u64 level_3_hashdata_size;
	u32 level_3_block_size;
	u32 reserved_2;
	u64 level_4_fs_relative_offset;
	u64 level_4_fs_size;
	u64 level_4_fs_block_size;
	u64 unknown_0;
} IVFC_STRUCT;

typedef struct
{
	u32 magic_0;
	u32 magic_1;
	u64 table_1_offset;
	u64 table_1_length;
	u64 table_1_block_size;
	u64 table_2_offset;
	u64 table_2_length;
	u64 table_2_block_size;
	u64 ivfc_offset;
	u64 ivfc_length;
	u64 ivfc_block_size;
} DPFS_STRUCT;

typedef struct
{
	u8 AES_MAC[0x10];
	DIFF_STRUCT DIFF;
} EXTDATA_HEADER_CONTEXT;

typedef struct
{
	u8 valid;
	u64 DIFI_offset;
	u32 active_table_offset; 
	DIFI_STRUCT DIFI;
	u8 DIFI_HASH[0x20];
	IVFC_STRUCT IVFC;
	DPFS_STRUCT DPFS;
} PARTITION_STRUCT;

typedef struct
{
	EXTDATA_HEADER_CONTEXT header;
	PARTITION_STRUCT partition[2];
} EXTDATA_CONTEXT;

int GetExtDataContext(EXTDATA_CONTEXT *ctx, FILE *extdataimg);

void print_extdata_header(EXTDATA_HEADER_CONTEXT header);
void print_partition_info(PARTITION_STRUCT partition);
void print_DIFI(PARTITION_STRUCT partition);
void print_IVFC(PARTITION_STRUCT partition);
void print_DPFS(PARTITION_STRUCT partition);

PARTITION_STRUCT get_extdata_partition_header(u64 offset, u32 active_table_offset, FILE *extdataimg);

int get_extdata_single_blob(char *filepath, u64 offset, u64 size, FILE *extdataimg);
int get_extdata_duo_blob(char *filepath, u64 offset, u64 size, int suffix, FILE *extdataimg);

#endif