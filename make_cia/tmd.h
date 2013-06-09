typedef struct
{
	u8 content_id[4];
	u8 content_index[2];
	u8 content_type[2];
	u8 content_size[8];
	u8 sha_256_hash[0x20];
} __attribute__((__packed__))
TMD_CONTENT_CHUNK_STRUCT;

typedef struct
{
	u8 content_index_offset[2];
	u8 content_command_count[2];
	u8 sha_256_hash[0x20];
} __attribute__((__packed__))
TMD_CONTENT_INFO_RECORD;

typedef struct
{
	u8 sig_type[4];
	u8 data[0x100];
	u8 padding[0x3C];
} __attribute__((__packed__)) 
TMD_2048_SIG_CONTEXT;

typedef struct
{
	u8 issuer[0x40];
	u8 version;
	u8 ca_crl_version;
	u8 signer_crl_version;
	u8 padding_1;
	u8 system_version[8];
	u8 title_id[8];
	u8 title_type[4];
	u8 group_id[2];
	u8 save_data_size[4];
	u8 unknown_data_0[4];
	u8 reserved_0[2];
	u8 twl_data[4];
	u8 reserved[0x30];
	u8 access_rights[4];
	u8 title_version[2];
	u8 content_count[2];
	u8 boot_content[2];
	u8 padding[2];
	u8 sha_256_hash[0x20];	
} __attribute__((__packed__)) 
TMD_STRUCT;

int GenerateTitleMetaData(CIA_CONTEXT *ctx);
void SetInfoChunk(TMD_CONTENT_CHUNK_STRUCT *info_chunk,CONTENT_INFO *ContentInfo);
void SetTMDHeader(TMD_STRUCT *header,CIA_CONTEXT *ctx);