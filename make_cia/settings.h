#define TMP_BUFFER_SIZE 1000

typedef struct
{
	u8 *data;
	u32 size;
} __attribute__((__packed__))
CERT_BUFF;


typedef struct
{
	CERT_BUFF ca;
	CERT_BUFF ticket;
	CERT_BUFF tmd;
} __attribute__((__packed__))
CERT_CONTEXT;


typedef struct
{
	u8 modulus[0x100];
	u8 exponent[0x4];
} __attribute__((__packed__))
RSA_2048_PUB_KEY;

typedef struct
{
	char issuer[0x40];
	u8 type[4];
	char name[0x40];
	u8 unknown[4];
} __attribute__((__packed__))
CERT_DATA_STRUCT;

void InitialiseSettings(CIA_CONTEXT *ctx);
int GetSettings(CIA_CONTEXT *ctx);
int GetSettings_NCSD(CIA_CONTEXT *ctx);

int LoadAESKeys(CIA_CONTEXT *ctx);
#ifndef _DEBUG_KEY_BUILD_	
int LoadRSAKeys(CIA_CONTEXT *ctx);
int LoadRSAKeyFile(RSA_2048_KEY *ctx, FILE *file);
#endif
void PrintRSAKeyData(RSA_2048_KEY *ctx);

#ifndef _DEBUG_KEY_BUILD_	
int ImportCertificates(CIA_CONTEXT *ctx);
int ImportCertificateFile(CERT_BUFF *buff, char *cert_lable, FILE *config_file);
#endif


int GetCoreInfo(CIA_CONTEXT *ctx);
int GetCoreInfo_NCSD(CIA_CONTEXT *ctx);
int SetBuildSettings(CIA_CONTEXT *ctx);

int SetTicketIssuer(CIA_CONTEXT *ctx);
int SetTitleMetaDataIssuer(CIA_CONTEXT *ctx);

int GetContentInfo(CONTENT_INFO *ctx, int content_index, FILE *config_file);

void ReadFile_u32(void *outbuff,u32 offset,u32 size,FILE *file);
void ReadFile_Text_u32(char *outbuff,u32 offset,u32 size,FILE *file);
