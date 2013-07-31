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
#ifndef _CTR_CRYPTO_H_
#define _CTR_CRYPTO_H_

#include "polarssl/aes.h"
#include "polarssl/rsa.h"
#include "polarssl/sha2.h"

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
	RSA_4096_PUBK = 0,
	RSA_2048_PUBK,
	ECC_PUBK
} pubk_types;

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
	u8 ctr[16];
	u8 iv[16];
	aes_context aes;
} ctr_aes_context;

typedef struct
{
	rsa_context rsa;
} ctr_rsa_context;

#ifdef __cplusplus
extern "C" {
#endif
void ctr_sha_256(void* data, u64 size, u8 hash[0x20]);
void ctr_add_counter(ctr_aes_context* ctx, u32 carry);
void ctr_init_counter(ctr_aes_context* ctx, u8 key[16],u8 ctr[16]);
void ctr_crypt_counter_block(ctr_aes_context* ctx, u8 input[16], u8 output[16]);
void ctr_crypt_counter(ctr_aes_context* ctx, u8* input,  u8* output, u32 size);
void ctr_init_aes_cbc(ctr_aes_context* ctx,u8 key[16],u8 iv[16], u8 mode);
void ctr_aes_cbc(ctr_aes_context* ctx,u8* input,u8* output,u32 size,u8 mode);
int ctr_rsa_init(ctr_rsa_context* ctx, RSA_2048_KEY* key, u8 mode);
void ctr_rsa_free(ctr_rsa_context* ctx);
int ctr_rsa2048_sha256_sign(const u8 hash[0x20], u8 signature[0x100], const u8 modulus[0x100], const u8 priv_exp[0x100]);
int ctr_rsa2048_sha256_verify(const u8 hash[0x20], u8 signature[0x100], const u8 modulus[0x100]);
int rsa_sha256_sign( rsa_context *ctx,const unsigned char *hash,unsigned char *sig );

#ifdef __cplusplus
}
#endif

#endif
