typedef enum
{
	vsxe_magic = 0x56535845,
	vsxe_magic_id = 0x30000
} VSXE_data;

typedef enum
{
	WIN_32 = 1,
	UNIX
} platform;

//Structs
typedef struct
{
	u8 parent_folder_index[4];
    u8 filename[0x10];
    u8 unk0[4];
    u8 unk1[4]; 
    u8 unk2[4];
    u8 unk3[4]; 
    u8 unk4[4];
} folder_entry;

typedef struct
{
	u8 parent_folder_index[4];
    u8 filename[0x10];
    u8 unk0[4];
    u8 unk1[4]; // magic?
    u8 unk2[4];
    u8 unk3[8];
    u8 unk4[4]; // flags?
    u8 unk5[4];
} file_entry;

typedef struct
{
	//TODO
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