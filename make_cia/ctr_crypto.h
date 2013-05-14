#ifndef _CTR_CRYPTO_H_
#define _CTR_CRYPTO_H_

#include "polarssl/aes.h"
#include "polarssl/rsa.h"
#include "polarssl/sha2.h"

#define RSAKEY_INVALID 0
#define RSAKEY_PUB 1
#define RSAKEY_PRIV 2

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
int ctr_rsa_verify_hash(const u8 signature[0x100], const u8 hash[0x20], RSA_2048_KEY* key);
int ctr_rsa_sign_hash(const u8 hash[0x20], u8 signature[0x100], RSA_2048_KEY* key);
int ctr_rsa_sha256_sign( rsa_context *ctx,const unsigned char *hash,unsigned char *sig );

#ifdef __cplusplus
}
#endif

#endif
