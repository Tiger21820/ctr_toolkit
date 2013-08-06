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
#ifndef _TMD_H_
#define _TMD_H_

typedef enum
{
	TYPE_CTR = 0x40,
	TYPE_DATA = 0x8
} title_type;

typedef enum
{
	Encrypted = 0x0001,
	Optional = 0x4000,
	Shared = 0x8000
} content_types;

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
	u8 priv_save_data_size[4]; // Zero for CXI Content0
	u8 reserved_0[4];
	u8 twl_flag; // Zero for CXI Content0
	u8 reserved_1[0x31];
	u8 access_rights[4];
	u8 title_version[2];
	u8 content_count[2];
	u8 boot_content[2];
	u8 padding[2];
	u8 sha_256_hash[0x20];	
} __attribute__((__packed__)) 
TMD_STRUCT;

#endif

int GenerateTitleMetaData(USER_CONTEXT *ctx);
void SetInfoChunk(TMD_CONTENT_CHUNK_STRUCT *info_chunk,CONTENT_INFO *ContentInfo);
void SetTMDHeader(TMD_STRUCT *header,USER_CONTEXT *ctx);