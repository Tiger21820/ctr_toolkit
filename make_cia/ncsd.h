/**
Copyright 2013 3DSGuy

This file is part of make_cia.

make_cia is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

make_cia is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with make_cia.  If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef _NCSD_H_
#define _NCSD_H_

#include "ncch.h"
#define NotChecked 3

typedef enum
{
	retail = 1,
	dev_internal_SDK,
	dev_external_SDK
} ROM_TYPES;

typedef enum
{
	FW_1_0_0 = 1024,
	FW_1_1_0 = 1045,
	FW_2_0_0 = 2049,
	FW_2_1_0 = 2069,
	FW_2_2_0 = 2088,
	FW_3_0_0 = 3088,
	FW_4_0_0 = 4098,
	FW_4_1_0 = 4113,
	FW_4_2_0 = 4130,
	FW_4_3_0 = 4145,
	FW_4_4_0 = 4163,
	FW_4_5_0 = 4176,
	FW_5_0_0 = 5120,
	FW_5_1_0 = 5136
} CVER_FW_VER;

typedef enum
{
	EUR_ROM = 0x00017102,
	JPN_ROM = 0x00017202,
	USA_ROM = 0x00017302,
	CHN_ROM = 0x00017402,
	KOR_ROM = 0x00017502,
	TWN_ROM = 0x00017602
} CVER_UID_REGION;

typedef struct
{
	u8 offset[4];
	u8 size[4];
} partition_offsetsize;

typedef struct
{
	u8 magic[4];
	u8 rom_size[4];
	u8 title_id[8];
	u8 partitions_fs_type[8];
	u8 partitions_crypto_type[8];
	partition_offsetsize offsetsize_table[8];
	u8 exheader_hash[0x20];
	u8 additional_header_size[0x4];
	u8 sector_zero_offset[0x4];
	u8 partition_flags[8];
	u8 partition_id_table[8][8];
	u8 reserved[0x30];
} NCSD_HEADER;

typedef struct
{
	u8 card_info[8];
	u8 reserved_0[0xf8];
	u8 rom_size_used[8];
	u8 reserved_1[0x18];
	u8 cver_title_id[8];
	u8 cver_title_version[2];
	u8 reserved_2[0xcd6];
	u8 partition_0_title_id[8];
	u8 reserved_3[8];
	u8 initial_data[0x30];
	u8 reserved_4[0xc0];
	NCCH_HEADER partition_0_header;
} CARD_INFO_HEADER;

typedef struct
{
	u8 CardDeviceReserved1[0x200];
	u8 TitleKey[0x10];
	u8 CardDeviceReserved2[0xf0];
} DEV_CARD_INFO_HEADER;

/**
typedef struct
{
	int valid;
	int sig_valid;
	int type;
	u8 signature[0x100];
	u8 ncsd_header_hash[0x20];
	NCSD_HEADER header;
	CARD_INFO_HEADER card_info;
	DEV_CARD_INFO_HEADER dev_card_info;
	
	u64 rom_size;
	u64 used_rom_size;
	PARTITION_DATA partition_data[8];
} NCSD_STRUCT;
**/
/**
typedef struct
{
	int active;
	int sig_valid;
	u8 fs_type;
	u8 crypto_type;
	u32 offset;
	u32 size;
	u64 title_id;
} PARTITION_DATA;

**/
#endif

int NCSDToCIA(USER_CONTEXT *ctx);

int GetNCSDData(USER_CONTEXT *ctx, NCSD_STRUCT *ncsd_struct, FILE *ncsd);

int VerifyNCSD(USER_CONTEXT *ctx, FILE *ncsd);
int VerifyNCCHSection(USER_CONTEXT *ctx, u8 cxi_key[0x10], u32 offset, FILE *ncch);

void PrintNCSDData(NCSD_STRUCT *ctx, NCSD_HEADER *header, CARD_INFO_HEADER *card_info, DEV_CARD_INFO_HEADER *dev_card_info);
void GetMin3DSFW(char *FW_STRING, CARD_INFO_HEADER *card_info);