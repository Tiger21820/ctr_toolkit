typedef struct
{
	u8 header_size[4];
	u8 type[2];
	u8 version[2];
	u8 cert_size[4];
	u8 tik_size[4];
	u8 tmd_size[4];
	u8 meta_size[4];
	u8 content_size[8];
	u8 content_index[0x2000];
} __attribute__((__packed__)) 
CIA_HEADER;

int SetupContentData(CIA_CONTEXT *ctx);
int SetupContentData_NCSD(CIA_CONTEXT *ctx);
int SetupCIAHeader(CIA_CONTEXT *ctx);
int WriteSectionsToOutput(CIA_CONTEXT *ctx);
int EncryptContent(u8 *EncBuffer,u8 *buffer,u64 size,u8 *title_key, u16 index);
u64 GetContentPaddingSize(u64 ContentSize, u32 alignment);