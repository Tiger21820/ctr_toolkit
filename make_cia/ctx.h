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
#ifndef _CTX_H_
#define _CTX_H_

typedef enum
{
	header = 0,
	certchain = 1,
	tik = 2,
	tmd = 3,
	content = 4,
	meta = 5,
} cia_sections;

typedef enum
{
	ctr_norm = 1,
	twl_cia,
	rom_conv
} cia_build_mode;

typedef enum
{
	NCCH_MAGIC = 0x4E434348,
	NCSD_MAGIC = 0x4E435344
} file_magic;

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
	//AES Keys
	u8 common_key_id;
	u8 common_key[16];
	u8 title_key[16];
	u8 cxi_key[3][16];
	
	//RSA Keys
	RSA_2048_KEY ticket;
	RSA_2048_KEY tmd;
	RSA_2048_KEY NcsdCfa;
} __attribute__((__packed__)) 
KEY_STORE;

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
	u8 priv_save_data_size[4];
	u8 twl_flag;
} __attribute__((__packed__)) 
CORE_CONTENT_INFO;

typedef enum
{
	encrypt_contents = 0,
	build_mode,
	gen_meta,
	verbose,
	showkeys,
	info,
} arg_flag_index;

typedef struct
{
	//Components
	COMPONENT_STRUCT cia_section[6];
	
	//Content Data
	u16 ContentCount;
	CONTENT_INFO *ContentInfo; //Content Info, where content specific data for TMD is kept
	CORE_CONTENT_INFO core; // For storing data relating to TMD/TIK generation
	NCSD_STRUCT *ncsd_struct; // For storing ROM Info
	
	//Input Info (path to input/output)
	OPTION_CTX core_infile;
	OPTION_CTX outfile;	
	
	//Settings
	KEY_STORE keys;
	u8 flags[6];
	
} USER_CONTEXT;

#endif