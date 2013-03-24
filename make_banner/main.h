#include "types.h"
#include "bnr.h"
#include "icn.h"

#define SUCCESS 0
#define FAIL 1

#define ARGC_FAIL 1
#define ARGV_FAIL 2

#define FILE_OPEN_FAIL 1
#define FILE_CREATE_FAIL 2

typedef struct
{
	//Command Flags
	u8 make_icn;
	u8 make_bnr;
	u8 read_icn;
	u8 command_point;
	u8 verbose_bool;
	
	//Input
	//Make ICN
	u8 *input_bsf;
	u8 *input_small_icon;
	u8 *input_large_icon;
	u8 *output_icn;
	
	//Make BNR
	u8 *input_cbmd;
	u8 *input_cwav;
	u8 *output_bnr;
	
	//Read ICN
	u8 *input_icn;
} __attribute__((__packed__))
INPUT_CONTEXT;