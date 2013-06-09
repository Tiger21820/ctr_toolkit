//Variable Structs

typedef struct
{
	u8 used;
	FILE *file;
} __attribute__((__packed__)) 
F_OPTION_CTX;

typedef struct
{
	u8 used;
	char *argument;
	u8 arg_len;
	F_OPTION_CTX file;
} __attribute__((__packed__)) 
OPTION_CTX;

typedef struct
{
	u8 used;
	u8 *buffer;
	u64 size;
} __attribute__((__packed__)) 
COMPONENT_STRUCT;


