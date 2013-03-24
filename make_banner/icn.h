#include "types.h"
//File Related Data
#define ICN_SIZE 0x36C0
#define TITLE_OFFSET 0x8
#define FLAG_OFFSET 0x2008
#define SMALL_ICON_SIZE 0x480
#define SMALL_ICON_OFFSET 0x2040
#define LARGE_ICON_SIZE 0x1200
#define LARGE_ICON_OFFSET 0x24C0

//Error Codes
#define TITLE_FAIL 1
#define FLAG_FAIL 2
#define REGION_RATING_FAIL 3
#define REGION_LOCKOUT_FAIL 4
#define OPTM_BNR_FAIL 5
#define ID_FAIL 6
#define MAGIC_FAIL 7

//Array Max Values
#define MAX_READ_LEN 100
#define MAX_TITLE_NUM 11
#define SIZE_SHORT_TITLE 0x40
#define SIZE_LONG_TITLE 0x50
#define SIZE_PUBLISHER_TITLE 0x40
#define MAX_BIT_NUM 8 
#define MAX_RATING_NUM 7
#define MAX_REGION_LOCK_NUM 7

//Ratings
#define RATING_JPN 0
#define RATING_USA 1
#define RATING_GER 2
#define RATING_EUR 3
#define RATING_PRT 4
#define RATING_ENG 5
#define RATING_AUS 6

#define VALID_RATING 0
#define INVALID_RATING 1

typedef struct
{
	u8 short_title[0x80];
	u8 long_title[0x100];
	u8 publisher[0x80];
} __attribute__((__packed__)) 
ICN_APP_TITLE_STRUCT; //Application Title Name Structure

typedef struct
{
	ICN_APP_TITLE_STRUCT title_array[0x10];
	// 0 = Japanese Title
	// 1 = English Title
	// 2 = French Title
	// 3 = German Title
	// 4 = Italian Title
	// 5 = Spansih Title
	// 6 = Chinese Title
	// 7 = Korean Title
	// 8 = Dutch Title
	// 9 = Portuguese
	// 10 = Russian
	// 11 = Reserved
	// 12 = Reserved
	// 13 = Reserved
	// 14 = Reserved
	// 15 = Reserved
} __attribute__((__packed__)) 
ICN_APP_TITLES;

typedef struct
{
	u8 rating[0x10];
	// 0 = CERO (Japan)
	// 1 = ESRB (USA)
	// 2 = RSV
	// 3 = USK (GER)
	// 4 = PEGI GEN (Europe)
	// 5 = RSV
	// 6 = PEGI PRT (Portugual)
	// 7 = PEGI BBFC (England)
	// 8 = COB (Australia)
	// 9 = Unknown/Unused
	// 10 = RSV
	// 11 = Unknown/Unused
	// 12 = Unknown/Unused
	// 13 = RSV
	// 14 = Unknown/Unused
	// 15 = Unknown/Unused
} __attribute__((__packed__)) 
ICN_AGE_RATINGS;

typedef struct
{
	ICN_AGE_RATINGS ratings;
	u8 region_lock[4];
	u8 match_maker_id[4];
	u8 match_maker_bit_id[8];
	u8 byte_flag[8];
	u8 optimal_bnr_frame[4];
	u8 cec_id[4];
} __attribute__((__packed__)) 
ICN_SETTINGS;

typedef struct
{
	int RESULT_CODE;
	u8 verbose_bool;

	//INPUT
	FILE *small; //Small Icon
	FILE *large; //Large Icon
	FILE *bsf; //Banner Specification Format

	//OUTPUT	
	FILE *output; //Output .ICN file
	ICN_APP_TITLES titles;// ICN Title Data
	ICN_SETTINGS settings;//ICN Flags
} __attribute__((__packed__)) 
ICN_CONTEXT; //Context for ICN creation

/**
typedef struct
{
	DWORD magic;
	u8 reserved[4];
	ICN_APP_TITLE_STRUCT title_array[0x10];
	ICN_SETTINGS settings;
} __attribute__((__packed__)) 
ICN_STRUCTURE;
**/

//Prototypes
int icn_read(FILE *icn, u8 verbose);
int icn_main(ICN_CONTEXT icn);
int icn_title_proccess(ICN_CONTEXT icn);
int icn_settings_proccess(ICN_CONTEXT icn);
void icn_icon_proccess(ICN_CONTEXT icn);
ICN_APP_TITLE_STRUCT string_ICN_conv(u8 *short_title, u8 *long_title, u8 *publisher);
void print_title(int offset, int size, FILE *file);

