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
	vsxe_magic = 0x56535845,
	vsxe_magic_id = 0x30000
} VSXE_data;


//Structs
typedef struct
{
	u8 parent_folder_index[4];
    char filename[0x10];
    u8 unk0[4];
    u8 unk1[4]; 
    u8 unk2[4];
    u8 unk3[4]; 
    u8 unk4[4];
} folder_entry;

typedef struct
{
	u8 parent_folder_index[4];
    char filename[0x10];
    u8 unk0[4];
    u8 unk1[4]; // magic?
    u8 unk2[4];
    u8 unk3[8];
    u8 unk4[4]; // flags?
    u8 unk5[4];
} file_entry;

typedef struct
{
	u8 unk0[40][4];
} data_table;

typedef struct
{
	u8 magic[4];
	u8 magic_id[4];
	u8 data_table_offset[4];
	u8 unk0[4];
	u8 unk1[0x20];
	u8 last_used_file_extdata_id[4];
	u8 unk2[4];
	char last_used_file[0x100];
	data_table table;
} vsxe_header;

typedef struct
{
	u8 used_slots[4];
	u8 max_slots[4];
	u8 reserved[0x48];
} folder_table_header;

typedef struct
{
	u8 used_slots[4];
	u8 max_slots[4];
	u8 reserved[0x28];
} file_table_header;


typedef struct
{
	FILE *extdata;
	u32 offset;
	
	vsxe_header header;
	
	u32 folder_table_offset;
	u8 foldercount;
	folder_entry *folders;
	
	u32 file_table_offset;
	u8 filecount;
	file_entry *files;
} VSXE_CONTEXT;

// Prototypes
int ProcessExtData_FS(INPUT_CONTEXT *ctx);
int GetVSXEContext(VSXE_CONTEXT *vsxe, INPUT_CONTEXT *ctx);
void PrintVSXE_FS_INFO(VSXE_CONTEXT *ctx);
void SetupOutputFS(VSXE_CONTEXT *vsxe, INPUT_CONTEXT *ctx);
int WriteExtDataFiles(VSXE_CONTEXT *vsxe, INPUT_CONTEXT *ctx);
int ExportExdataImagetoFile(FILE *extdata, FILE *outfile);
int VerifyVSXE(VSXE_CONTEXT *ctx);
void Return_Dir_Path(VSXE_CONTEXT *ctx, u32 file_id, char *path, u8 platform);
void Return_ExtData_Mount_Path(VSXE_CONTEXT *ctx, u32 file_id, char *path, u8 platform);
void InterpreteFolderTable(VSXE_CONTEXT *ctx);
void InterpreteFileTable(VSXE_CONTEXT *ctx);

//void read_vsxe(FILE *file, u32 offset);
