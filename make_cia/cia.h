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
	u8 magic_0;
	u8 magic_1[0x1fff];
} __attribute__((__packed__)) 
CIA_HEADER;

int SetupContentData(CIA_CONTEXT *ctx);
char* GetContentFilePath(CONTENT_INFO *ctx);
u8* GetContentSHAHash(CONTENT_INFO *ctx);
u16 GetContentIndex(CONTENT_INFO *ctx);
void SetContentSize(CONTENT_INFO *ctx, u64 size);
int WriteSectionsToOutput(CIA_CONTEXT *ctx);
void WriteBuffer(u8 *buffer, u64 size, u64 offset, FILE *output);
int SetupCIAHeader(CIA_CONTEXT *ctx);
int EncryptContent(u8 *EncBuffer,u8 *buffer,u64 size,u8 *title_key, u16 index);
u64 GetContentSize(FILE *content);
u64 GetContentPaddingSize(FILE *content, u32 alignment);