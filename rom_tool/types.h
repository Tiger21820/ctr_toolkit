#include <stdlib.h>
#include <stdint.h>
//Bools
typedef enum
{
	False,
	True
} _boolean;

typedef enum
{
	Good,
	Fail
} return_basic;

typedef enum
{
	ARGC_FAIL = 1,
	ARGV_FAIL,
	aes_key_fail,
	rsa_key_fail,
	cia_type_fail,
	cert_gen_fail,
	content_mismatch,
	ticket_gen_fail,
	tmd_gen_fail,
	cia_header_gen_fail
} errors;

typedef enum
{
	BIG_ENDIAN = 0,
	LITTLE_ENDIAN = 1,
	BE = 0,
	LE = 1
} endianness_flag;

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef unsigned long long      u64;

typedef signed char     s8;
typedef signed short    s16;
typedef signed int      s32;
typedef signed long long        s64;
