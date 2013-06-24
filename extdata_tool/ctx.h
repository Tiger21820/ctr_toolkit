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
	EXTDATA_HEADER_CONTEXT header;
	PARTITION_STRUCT partition[2];	
} INPUT_CONTEXT;

