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

#define TMP_BUFFER_SIZE 1000
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

typedef enum
{
	TWL = 1,
	CTR
} content_platform;

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

#endif

int GetSettings(USER_CONTEXT *ctx, int argc, char *argv[]);
int SetBooleanSettings(USER_CONTEXT *ctx, int argc, char *argv[]);
int SetCryptoSettings(USER_CONTEXT *ctx, int argc, char *argv[]);
int GetCoreData(USER_CONTEXT *ctx, int argc, char *argv[]);
int SetBuildSettings(USER_CONTEXT *ctx, int argc, char *argv[]);
int GetContentData(USER_CONTEXT *ctx, int argc, char *argv[]);
void InitialiseSettings(USER_CONTEXT *ctx);
int LoadRSAKeyFile(RSA_2048_KEY *ctx, FILE *file);
void PrintRSAKeyData(RSA_2048_KEY *ctx);
int SetTicketIssuer(USER_CONTEXT *ctx);
int SetTitleMetaDataIssuer(USER_CONTEXT *ctx);
void GetRandomContentID(u8 *contentID, u16 value);