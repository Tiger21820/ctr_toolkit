typedef enum
{
	common = 1,
	unique
} ticket_type;

typedef enum
{
	ctr_norm = 1,
	twl_cia,
	rom_conv
} cia_build_mode;

typedef enum
{
	TYPE_CTR = 0x40,
	TYPE_DATA = 0x8
} title_type;

typedef enum
{
	RSA_4096_SHA1 = 0x00010000,
	RSA_2048_SHA1 = 0x00010001,
	ECC_SHA1 = 0x00010002,
	RSA_4096_SHA256 = 0x00010003,
	RSA_2048_SHA256 = 0x00010004,
	ECC_SHA256 = 0x00010005
} sig_types;

typedef enum
{
	NCCH_MAGIC = 0x4E434348,
	NCSD_MAGIC = 0x4E435344
} file_magic;

typedef enum
{
	KB = 1024,
	MB = 1048576,
	GB = 1073741824
} file_size;

//Define PubK Types
typedef enum
{
	RSA_4096_PUBK = 0,
	RSA_2048_PUBK,
	ECC_PUBK
} pubk_types;

//Content Platforms
typedef enum
{
	TWL = 1,
	CTR
} content_platform;

typedef enum
{
	Encrypted = 0x0001,
	Optional = 0x4000,
	Shared = 0x8000
} content_types;

typedef enum
{
	lic_Permanent = 0,
	lic_Demo = 1,
	lic_Trial = 2,
	lic_Rental = 3,
	lic_Subscription = 4,
	lic_Service = 5,
	lic_Mask = 15
} ticket_license_type;

typedef enum
{
	right_Permanent = 1,
	right_Subscription = 2,
	right_Content = 3,
	right_ContentConsumption = 4,
	right_AccessTitle = 5
} ticket_item_rights;

typedef enum
{
	_unknown = 0,
	CXI,
	CFA_Manual,
	CFA_DLPChild,
	CFA_Update
} ncch_types;


//Key Types
typedef enum
{
	No_Key = 0,
	ZerosFixed = 1,
	SystemFixed = 2,
	Secure = 3
} ncch_key;

//Modes
typedef enum
{
	dev = 1,
	prod,
	test
} cia_type;

//Key Struct

typedef struct
{
	u8 used;
	u8 key[0x10];
} __attribute__((__packed__)) 
AES_128_KEY;

typedef enum
{
	ENC,
	DEC
} aescbcmode;

typedef enum
{
	RSAKEY_INVALID,
	RSAKEY_PRIV,
	RSAKEY_PUB
} rsakeytype;

typedef struct
{
	//Public
	unsigned char n[256];
	unsigned char e[3];
	unsigned char d[256];
	unsigned char p[128];
	unsigned char q[128];
	unsigned char dp[128];
	unsigned char dq[128];
	unsigned char qp[128];
	rsakeytype keytype;
	
	//Key Data
	char name[0x40];
	char issuer[0x40];
} __attribute__((__packed__)) 
RSA_2048_KEY;


typedef struct
{
	//AES Keys
	u8 common_key_id;
	AES_128_KEY common_key;
	AES_128_KEY title_key;
	AES_128_KEY ncch_key;
	
	//RSA Keys
	RSA_2048_KEY ticket;
	RSA_2048_KEY tmd;
	RSA_2048_KEY NcsdCfa;
} __attribute__((__packed__)) 
KEY_STORE;

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

typedef struct
{
	int active;
	int sig_valid;
	u8 content_type;
	u8 fs_type;
	u8 crypto_type;
	u32 offset;
	u32 size;
	u64 title_id;
} PARTITION_DATA;

typedef struct
{
	int valid;
	int sig_valid;
	int type;
	u8 signature[0x100];
	u8 ncsd_header_hash[0x20];
	/**
	NCSD_HEADER header;
	CARD_INFO_HEADER card_info;
	DEV_CARD_INFO_HEADER dev_card_info;
	**/
	
	u64 rom_size;
	u64 used_rom_size;
	PARTITION_DATA partition_data[8];
} NCSD_STRUCT;

typedef struct
{
	u8 valid;
	u8 encrypted;
	char file_path[100];
	u64 file_offset;
	u8 content_id[4];
	u16 content_index;
	u16 content_type;
	u64 content_size;
	u8 sha_256_hash[0x20];
} __attribute__((__packed__)) 
CONTENT_INFO;

typedef struct
{
	u8 TitleID[8];
	u8 TicketID[8];
	u8 TicketVersion[2];
	u8 TitleVersion[2];
	u8 Title_type[4];
	u8 DeviceID[4];
	
	u8 Platform;
	
	u8 ca_crl_version;
	u8 signer_crl_version;
	
	//Ticket Data
	char TicketIssuer[0x40];
	u8 ticket_format_ver;
	//TMD Data
	char TMDIssuer[0x40];
	u8 tmd_format_ver;
	u8 save_data_size[4];
	u8 unknown_data_0[4];
	u8 twl_data[4];
} __attribute__((__packed__)) 
CORE_CONTENT_INFO;

typedef struct
{
	//Components
	COMPONENT_STRUCT header;
	COMPONENT_STRUCT certchain;
	COMPONENT_STRUCT ticket;
	COMPONENT_STRUCT tmd;
	COMPONENT_STRUCT content;
	COMPONENT_STRUCT meta;
	
	//Content Data
	u16 ContentCount;
	u8 ContentInfoMallocFlag;
	CONTENT_INFO *ContentInfo; //Content Info
	
	//Key Data
	KEY_STORE keys;
	
	//Content Data For TMD/Ticket and Encrypt/Decrypt etc
	CORE_CONTENT_INFO core;
	
	//Input Info
	OPTION_CTX core_infile;
	OPTION_CTX configfile;
	OPTION_CTX outfile;
	OPTION_CTX ncsdfile;
	
	//NCSD Data
	u8 ncsd_struct_malloc_flag;
	NCSD_STRUCT *ncsd_struct;
	
	//Settings
	u8 encrypt_contents;
	u8 rand_titlekey;
	u8 build_mode;
	u8 cia_type;
	u8 verbose_flag;
	u8 meta_flag;
	u8 showkeys_flag;
} __attribute__((__packed__)) 
CIA_CONTEXT;

typedef struct
{
	u8 magic[4];
	u8 rsatype[2];
	u8 reserved[2];
	u8 n_offset[4];
	u8 n_size[4];
	u8 e_offset[4];
	u8 e_size[4];
	u8 d_offset[4];
	u8 d_size[4];
	u8 name_offset[4];
	u8 name_size[4];
	u8 issuer_offset[4];
	u8 issuer_size[4];
} __attribute__((__packed__)) 
CRKF_HEADER;
