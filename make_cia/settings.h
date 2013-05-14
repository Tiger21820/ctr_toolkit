#define TMP_BUFFER_SIZE 1000

//Title Type
#define TYPE_CTR 0x40
#define TYPE_DATA 0x8

//Define Sig Types
#define RSA_4096_SHA1 0x00000100
#define RSA_2048_SHA1 0x01000100
#define ECC_SHA1 0x02000100
#define RSA_4096_SHA256 0x03000100
#define RSA_2048_SHA256 0x04000100
#define ECC_SHA256 0x05000100

//Define PubK Types
#define RSA_4096_PUBK 0x0
#define RSA_2048_PUBK 0x1
#define ECC_PUBK 0x2

#define ERR_UNRECOGNISED_SIG 2

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

int LoadAESKeys(CIA_CONTEXT *ctx);
int LoadRSAKeys(CIA_CONTEXT *ctx);
int LoadRSAKeyFile(RSA_2048_KEY *ctx, FILE *file);
void PrintRSAKeyData(RSA_2048_KEY *ctx);

int ImportCertificates(CIA_CONTEXT *ctx);
int ImportCertificateFile(CERT_BUFF *buff, char *cert_lable, FILE *config_file);


int GetCoreInfo(CIA_CONTEXT *ctx);
int SetBuildSettings(CIA_CONTEXT *ctx);

int SetTicketIssuer(CIA_CONTEXT *ctx);
int SetTitleMetaDataIssuer(CIA_CONTEXT *ctx);

int GetContentInfo(CONTENT_INFO *ctx, int content_index, FILE *config_file);

void ReadFile_u32(void *outbuff,u32 offset,u32 size,FILE *file);
void ReadFile_Text_u32(char *outbuff,u32 offset,u32 size,FILE *file);
