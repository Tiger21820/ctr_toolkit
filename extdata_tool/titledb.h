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

typedef enum
{
	Normal,
	ByTID
}DB_READ_MODES;

typedef enum
{
	NANDTDB = 1,
	NANDIDB,
	TEMPTDB,
	TEMPIDB
}DB_TYPES;

typedef enum
{
	ValidDB,
	CorruptDB
}DB_RETURN_CODES;

typedef enum
{
	CTR = 0,
	TWL
}TitlePlatform;

typedef enum
{
	Regular = 0,
	System
}TitleType;

typedef enum
{
	Application = 0,
	DLP_Child,
	Demo,
	Addon_Content,
	DLC_Content,
	Applet,
	Module,
	Data_Archive,
	Firmware
}TitleAppType;

typedef struct
{
	u8 unknown_0[4];
	u8 Active_Entry[4];
	u8 Title_ID[8];
	u8 Index[4];
	u8 unknown_2[4];
	u8 Title_Info_Offset[4];
	u8 Title_Info_Offset_Media[4];
	u8 unknown_5[4];
	u8 unknown_6[4];
	u8 unknown_7[4];
} __attribute__((__packed__)) 
TITLE_INDEX_ENTRY_STRUCT;

typedef struct
{
	u8 Title_Size[8];
	u8 Title_Type[4];
	u8 Title_Version[2];
	u8 reserved_0[2];
	u8 Flags_0[4];
	u8 TMD_File_ID[4];
	u8 CMD_File_ID[4];
	u8 Flags_1[4];
	u8 ExtData_ID[4];
	u8 reserved_1[4];
	u8 Flags_2[8];
	u8 Product_Code[0x10];
	u8 reserved_2[0x10];
	u8 unknown_6[4];
	u8 reserved_3[0xC];
	u8 reserved_4[0x20];//perhaps padding between info entries
} __attribute__((__packed__)) 
TITLE_INFO_ENTRY_STRUCT;

typedef struct
{
	TITLE_INDEX_ENTRY_STRUCT index;
	TITLE_INFO_ENTRY_STRUCT info;
}TITLE_CONTEXT;

typedef struct
{
	u8 magic_0[4];
	u8 magic_1[4];
	u8 reserved_0[0x24];
	u8 entry_count[4];
	u8 reserved_1[0x50];
	u8 max_entry_count[4];
	u8 unknown_0[4];
	u8 reserved_2[0x20];
} __attribute__((__packed__)) 
ENTRY_TABLE_HEADER;

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
	u8 magic_0[4];
	u8 magic_1[4];
	u8 dataoffset[8];
	//
	u8 database_size[8];
	u8 database_media_size[4];
	u8 reserved_2[0xC];
	//
	u8 FolderMap_Offset[8];
	u8 FolderMap_Size[4];
	u8 FolderMap_Media_Size[4];
	//
	u8 FileMap_Offset[8];
	u8 FileMap_Size[4];
	u8 FileMap_Media_Size[4];
	//
	u8 BlockMap_Offset[8];
	u8 BlockMap_Size[4];
	u8 BlockMap_Media_Size[4];
	//
	u8 Entry_Table_Offset[8];
	u8 Entry_Table_Size[4];
	u8 Entry_Table_Media_Size[4];
	//
	u8 FolderTable_Offset[8];
	u8 FolderTable_Unknown[4];
	u8 FolderTable_Media_Size[4];
	//
	u8 Unknown_1[4];
	u8 Info_Table_Offset[4];
	u8 Unknown_2[4];
	u8 Info_Table_Offset_Medias[4];
	} __attribute__((__packed__)) 
BDRI_STRUCT;

typedef struct
{
	u8 BufferAllocated;
	u32 TitleCount;
	u32 MaxCount;
	TITLE_CONTEXT *TitleData;
}TITLE_DATABASE;

typedef struct
{
	int db_type;
	u64 db_offset;
	u64 EntryTableOffset;
}CORE_DB_DATA;

typedef struct
{
	FILE *db;
	CORE_DB_DATA core_data;
	//
	BDRI_STRUCT header;
	ENTRY_TABLE_HEADER entry_table_header;
	//
	TITLE_DATABASE database;
}DATABASE_CONTEXT;

int ProcessTitleDB(FILE *tdb, int Mode, u64 offset);

int GetDB_Header(DATABASE_CONTEXT *ctx);
void GetDB_Type(DATABASE_CONTEXT *ctx, char db_type_magic[8]);
int CheckDB_Header(DATABASE_CONTEXT *ctx);
void ProcessDB_Header(DATABASE_CONTEXT *ctx);
int GetEntry_Header(DATABASE_CONTEXT *ctx);
void PopulateDatabase(DATABASE_CONTEXT *ctx);
void StoreTitleEntry(TITLE_CONTEXT *TitleData, u32 entry_offset,u32 info_offset, FILE *db);
void PrintDatabase(DATABASE_CONTEXT *ctx);
void PrintTitleData(TITLE_CONTEXT *TitleData);
int EntryUsed(TITLE_CONTEXT *TitleData);
int EntryValid(TITLE_CONTEXT *TitleData);
void PrintTitleIndexData(TITLE_INDEX_ENTRY_STRUCT *index);
void PrintTitleInfoData(TITLE_INFO_ENTRY_STRUCT *info);
void GetTitleType(u8 TitleID[8]);
void ListDatabase(DATABASE_CONTEXT *ctx);
u32 GetValidEntryCount(DATABASE_CONTEXT *ctx);
void CollectTitleIDs(u64 *TitleID_DB, u32 ContentCount, DATABASE_CONTEXT *ctx);
u64 ReturnTitleID(TITLE_CONTEXT *TitleData);
void SortTitleIDs(u64 *TitleID_DB, u32 ContentCount);
void ListTitleIDs(u64 *TitleID_DB, u32 ContentCount);

/**
DB_HEADER_CONTEXT process_db_header(FILE *tdb, u64 offset);
void print_db_header(TEMPTDB_STRUCT tdb_header, u8 database_type);
void print_index_entry_header(TITLE_TABLE_HEADER title_table_header);
void print_title_data(TITLE_INDEX_ENTRY_STRUCT title_entry, TITLE_INFO_ENTRY_STRUCT title_info);
void print_title_index_entry(TITLE_INDEX_ENTRY_STRUCT title_entry);
void print_title_info_entry(TITLE_INFO_ENTRY_STRUCT title_info);
u8 tdb_magic_check(u32 *magic);
**/