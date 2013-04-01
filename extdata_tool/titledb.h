/**
Copyright 2013 3DSGuy

This file is part of extdata_tool.

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
#define TEMPTDB_MAGIC_0 0x504D4554
#define TEMPTDB_MAGIC_1 0x424454
#define TEMPTDB_CORRUPT 10
#define BDRI_MAGIC_0 0x49524442
#define BDRI_MAGIC_1 0x30000
#define BDRI_CORRUPT 11

typedef struct
{
	u8 title_size[8];
	u32 title_type;
	u32 title_version;
	
	u32 electronic_manual;
	u32 tmd_file_id;
	u32 cmd_file_id;
	u32 save_data;	
	
	u32 extdata_id;
	u32 unknown_3;
	u32 unknown_4;
	u32 constant;
	
	u8 product_code[0x10];
	
	u8 reserved_0[0x10];
	
	u32 unknown_6;
	u8 reserved_1[0xC];
	
	u8 reserved_2[0x20];
} __attribute__((__packed__)) 
PRODUCT_CODE_STRUCT;

typedef struct
{
	u32 active_entry;
	u8 title_id[8];
	u32 unknown_1;
	u32 unknown_2;
	u32 pdc_table_X;
	u32 pdc_table_Y;
	u32 unknown_5;
	u32 unknown_6;
	u32 unknown_7;
	u32 unknown_8;
} __attribute__((__packed__)) 
TID_STRUCT;

typedef struct
{
	u8 reserved[0x48];
	u16 pdc_table_maxX;
	u16 pdc_table_maxY;
	u8 unknown_2[4];
	u8 reserved_1[0x30];
	u8 unknown_3[4];
	u8 unknown_4[4];
	u8 reserved_2[0x24];
	u8 unknown_5[4];
	u8 reserved_3[0x50];
	u32 entry_count;
	u8 unknown_6[4];
	u8 reserved_4[0x24];
} __attribute__((__packed__)) 
TITLE_TABLE_HEADER;

typedef struct
{
	u32 magic_0;
	u32 magic_1;
	u8 reserved_0[0x8];
	u8 unknown[0x48];
	u32 title_table_offset;
	u8 reserved[0x124];
} __attribute__((__packed__)) 
BDRI_STRUCT;

typedef struct
{
	u32 magic_0;
	u32 magic_1;
	u8 reserved[0x78];
	BDRI_STRUCT bdri;
} __attribute__((__packed__)) 
TEMPTDB_STRUCT;

int process_title_database(FILE *tdb, u64 offset);
