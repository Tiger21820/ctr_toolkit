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

// Retrieve Settings from argc/argv
int GetSettings(CIA_CONTEXT *ctx, int argc, char *argv[]);
int SetBooleanSettings(CIA_CONTEXT *ctx, int argc, char *argv[]);
int SetCryptoSettings(CIA_CONTEXT *ctx, int argc, char *argv[]);
int GetCoreData(CIA_CONTEXT *ctx, int argc, char *argv[]);
int SetBuildSettings(CIA_CONTEXT *ctx, int argc, char *argv[]);
int GetContentData(CIA_CONTEXT *ctx, int argc, char *argv[]);
//
void InitialiseSettings(CIA_CONTEXT *ctx);
int LoadRSAKeyFile(RSA_2048_KEY *ctx, FILE *file);
void PrintRSAKeyData(RSA_2048_KEY *ctx);
int SetTicketIssuer(CIA_CONTEXT *ctx);
int SetTitleMetaDataIssuer(CIA_CONTEXT *ctx);
void ReadFile_u32(void *outbuff,u32 offset,u32 size,FILE *file);
