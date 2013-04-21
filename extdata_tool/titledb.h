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
#define NANDTDB_MAGIC_0 0x444e414e
#define NANDTDB_MAGIC_1 0x424454
#define NANDIDB_MAGIC_0 0x444E414E
#define NANDIDB_MAGIC_1 0x424449
#define TEMPTDB_MAGIC_0 0x504D4554
#define TEMPTDB_MAGIC_1 0x424454
#define TEMPIDB_MAGIC_0 0x504D4554
#define TEMPIDB_MAGIC_1 0x424449

#define PREHEADER_SIZE 0x80

#define NANDTDB 20
#define NANDIDB 21
#define TEMPTDB 22
#define TEMPIDB 23

#define DB_CORRUPT 10
#define BDRI_MAGIC_0 0x49524442
#define BDRI_MAGIC_1 0x30000
#define BDRI_CORRUPT 11

typedef struct
{
	u8 title_size[8];
	u32 title_type;
	u32 title_version;
	u8 flags_0[4];
	u32 tmd_file_id;
	u32 cmd_file_id;
	u8 flags_1[4];
	u32 extdata_id;
	u8 reserved[4];
	u8 flags_2[8];
	u8 product_code[0x10];
	u8 reserved_0[0x10];
	u32 unknown_6;
	u8 reserved_1[0xC];
	u8 reserved_2[0x20];//perhaps padding between info entries
} __attribute__((__packed__)) 
TITLE_INFO_ENTRY_STRUCT;

typedef struct
{
	u32 unknown_0;
	u32 active_entry;
	u8 title_id[8];
	u32 index;
	u32 unknown_2;
	u32 title_info_offset_X;
	u32 title_info_offset_Y;
	u32 unknown_5;
	u32 unknown_6;
	u32 unknown_7;
} __attribute__((__packed__)) 
TITLE_INDEX_ENTRY_STRUCT;

typedef struct
{
	u32 unknown_0;
	u32 unknown_1;
	u8 reserved_2[0x24];
	u32 entry_count;
	u8 reserved_3[0x50];
	u32 max_entry_count;
	u32 unknown_2;
	u8 reserved_4[0x20];
} __attribute__((__packed__)) 
TITLE_TABLE_HEADER;

/**
typedef struct
{
	u32 magic_0;
	u32 magic_1;
	u8 reserved_0[0x8];
	u32 database_size;
	u8 reserved_1[4];
	u32 database_media_size;
	u8 reserved_2[4];
	u8 unknown_fixed[0x20];
	u8 unknown_0[0x18];
	u32 title_table_offset;
	u8 unknown_1[0x20];
	u32 title_info_table_offset_X;
	u8 reserved_3[4];
	u32 title_info_table_offset_Y;
} __attribute__((__packed__)) 
BDRI_STRUCT;
**/

typedef struct
{
	u32 magic_0;
	u32 magic_1;
	u32 dataoffset;
	u8 reserved_0[4];
	//
	u32 database_size;
	u8 reserved_1[4];
	u32 database_media_size;
	u8 reserved_2[0xC];
	//
	u32 FolderMap_Offset;
	u8 reserved_3[4];
	u32 FolderMap_Size;
	u32 FolderMap_Media_Size;
	//
	u32 FileMap_Offset;
	u8 reserved_4[4];
	u32 FileMap_Size;
	u32 FileMap_Media_Size;
	//
	u32 BlockMap_Offset;
	u8 reserved_5[4];
	u32 BlockMap_Size;
	u32 BlockMap_Media_Size;
	//
	u32 title_table_offset;
	u8 reserved_6[4];
	u32 Entry_Table_Size;
	u32 Entry_Table_Media_Size;
	//
	u32 FolderTable_Offset;
	u8 reserved_7[4];
	u32 FolderTable_Unknown;
	u32 FolderTable_Media_Size;
	//
	u32 Info_Table_Offset;
	u8 reserved_8[4];
	u32 Info_Table_Offset_Medias;
	u32 FileTable_Media_Size;
	} __attribute__((__packed__)) 
BDRI_STRUCT;

typedef struct
{
	u32 magic[2];
	u8 reserved[0x78];
	BDRI_STRUCT bdri;
} __attribute__((__packed__)) 
TEMPTDB_STRUCT;

typedef struct
{
	u8 result;
	u8 database_type;
	TEMPTDB_STRUCT pre_header;
	BDRI_STRUCT db_header;
} __attribute__((__packed__)) 
DB_HEADER_CONTEXT;

int process_title_database(FILE *tdb, u64 offset);
DB_HEADER_CONTEXT process_db_header(FILE *tdb, u64 offset);
void print_db_header(TEMPTDB_STRUCT tdb_header, u8 database_type);
void print_index_entry_header(TITLE_TABLE_HEADER title_table_header);
void print_title_data(TITLE_INDEX_ENTRY_STRUCT title_entry, TITLE_INFO_ENTRY_STRUCT title_info);
void print_title_index_entry(TITLE_INDEX_ENTRY_STRUCT title_entry);
void print_title_info_entry(TITLE_INFO_ENTRY_STRUCT title_info);
u8 tdb_magic_check(u32 *magic);