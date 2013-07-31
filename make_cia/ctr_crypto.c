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
#include "lib.h"
#include "ctr_crypto.h"


// API for polarssl adapted/copied from CTRTOOL by neimod
void ctr_sha_256(void* data, u64 size, u8 hash[0x20])
{
	sha2(data, size, hash, 0);
}

void ctr_add_counter(ctr_aes_context* ctx, u32 carry)
{
	u32 counter[4];
	u32 sum;
	int i;

	for(i=0; i<4; i++)
		counter[i] = (ctx->ctr[i*4+0]<<24) | (ctx->ctr[i*4+1]<<16) | (ctx->ctr[i*4+2]<<8) | (ctx->ctr[i*4+3]<<0);

	for(i=3; i>=0; i--)
	{
		sum = counter[i] + carry;

		if (sum < counter[i])
			carry = 1;
		else
			carry = 0;

		counter[i] = sum;
	}

	for(i=0; i<4; i++)
	{
		ctx->ctr[i*4+0] = counter[i]>>24;
		ctx->ctr[i*4+1] = counter[i]>>16;
		ctx->ctr[i*4+2] = counter[i]>>8;
		ctx->ctr[i*4+3] = counter[i]>>0;
	}
}

void ctr_init_counter(ctr_aes_context* ctx, u8 key[16], u8 ctr[16])
{
	aes_setkey_enc(&ctx->aes, key, 128);
	memcpy(ctx->ctr, ctr, 16);
}

void ctr_crypt_counter_block(ctr_aes_context* ctx, u8 input[16], u8 output[16])
{
	int i;
	u8 stream[16];


	aes_crypt_ecb(&ctx->aes, AES_ENCRYPT, ctx->ctr, stream);


	if (input)
	{
		for(i=0; i<16; i++)
		{
			output[i] = stream[i] ^ input[i];
		}
	}
	else
	{
		for(i=0; i<16; i++)
			output[i] = stream[i];
	}

	ctr_add_counter(ctx, 1);
}

void ctr_crypt_counter(ctr_aes_context* ctx, u8* input,  u8* output, u32 size)
{
	u8 stream[16];
	u32 i;

	while(size >= 16)
	{
		ctr_crypt_counter_block(ctx, input, output);

		if (input)
			input += 16;
		if (output)
			output += 16;

		size -= 16;
	}

	if (size)
	{
		memset(stream, 0, 16);
		ctr_crypt_counter_block(ctx, stream, stream);

		if (input)
		{
			for(i=0; i<size; i++)
				output[i] = input[i] ^ stream[i];
		}
		else
		{
			memcpy(output, stream, size);
		}
	}
}

void ctr_init_aes_cbc(ctr_aes_context* ctx,u8 key[16],u8 iv[16], u8 mode)
{
	switch(mode){
		case(ENC): aes_setkey_enc(&ctx->aes, key, 128); break;
		case(DEC): aes_setkey_dec(&ctx->aes, key, 128); break;
	}
	memcpy(ctx->iv, iv, 16);
}

void ctr_aes_cbc(ctr_aes_context* ctx,u8* input,u8* output,u32 size,u8 mode)
{
	switch(mode){
		case(ENC): aes_crypt_cbc(&ctx->aes, AES_ENCRYPT, size, ctx->iv, input, output); break;
		case(DEC): aes_crypt_cbc(&ctx->aes, AES_DECRYPT, size, ctx->iv, input, output); break;
	}
}

int ctr_rsa_init(ctr_rsa_context* ctx, RSA_2048_KEY* key, u8 mode)
{
	rsa_init(&ctx->rsa, RSA_PKCS_V15, 0);
	ctx->rsa.len = 0x100;
	switch(mode){
		case(RSAKEY_PUB):
			if (mpi_read_binary(&ctx->rsa.N, key->n, sizeof(key->n)))
				goto clean;
			if (mpi_read_binary(&ctx->rsa.E, key->e, sizeof(key->e)))
				goto clean;
			break;
		case(RSAKEY_PRIV):
			if (mpi_read_binary(&ctx->rsa.N, key->n, sizeof(key->n)))
				goto clean;
			if (mpi_read_binary(&ctx->rsa.D, key->d, sizeof(key->d)))
				goto clean;
			break;
	}

	return 1;
clean:
	return 0;
}

void ctr_rsa_free(ctr_rsa_context* ctx)
{
	rsa_free(&ctx->rsa);
}

int ctr_rsa2048_sha256_sign(const u8 hash[0x20], u8 signature[0x100], const u8 modulus[0x100], const u8 priv_exp[0x100])
{
	RSA_2048_KEY key;
	ctr_rsa_context ctx;
	memcpy(key.d,priv_exp,0x100);
	memcpy(key.n,modulus,0x100);
	ctr_rsa_init(&ctx,&key,RSAKEY_PRIV);
	u32 result = rsa_sha256_sign(&ctx.rsa,hash,signature);
	ctr_rsa_free(&ctx);
	
	if (result == 0)
		return 0;
	else
		return 1;
}

int ctr_rsa2048_sha256_verify(const u8 hash[0x20], u8 signature[0x100], const u8 modulus[0x100])
{
	RSA_2048_KEY key;
	ctr_rsa_context ctx;
	u8 exponent[3] = {0x01,0x00,0x01};
	memcpy(key.e,exponent,3);
	memcpy(key.n,modulus,0x100);
	ctr_rsa_init(&ctx,&key,RSAKEY_PUB);
	u32 result = rsa_pkcs1_verify(&ctx.rsa, RSA_PUBLIC, SIG_RSA_SHA256, 0x20, hash, (u8*)signature);
	ctr_rsa_free(&ctx);
	
	if (result == 0)
		return 0;
	else
		return 1;
}

/**
*  Made from rsa.c, especially crafted for generating signatures for SHA-256 hashes when only D and N are present
**/
int rsa_sha256_sign( rsa_context *ctx,const unsigned char *hash,unsigned char *sig )
{
    int nb_pad, olen, ret;
    unsigned char *p = sig;
    olen = ctx->len;  
    nb_pad = olen - 3 - 51;
    *p++ = 0;
    *p++ = RSA_SIGN;
    memset( p, 0xFF, nb_pad );
    p += nb_pad;
    *p++ = 0;
    memcpy( p, ASN1_HASH_SHA2X, 19 );
    memcpy( p + 19, hash, 32 );
    p[1] += 32; 
	p[14] = 1; 
	p[18] += 32;

    mpi T, T1, T2;

    mpi_init( &T, &T1, &T2, NULL );

    MPI_CHK( mpi_read_binary( &T, sig, ctx->len ) );

    if( mpi_cmp_mpi( &T, &ctx->N ) >= 0 )
    {
        mpi_free( &T, NULL );
        return( POLARSSL_ERR_RSA_BAD_INPUT_DATA );
    }	
	
	MPI_CHK( mpi_exp_mod( &T, &T, &ctx->D, &ctx->N, &ctx->RN ) );
	
    MPI_CHK( mpi_write_binary( &T, sig, olen ) );

cleanup:

    mpi_free( &T, &T1, &T2, NULL );

    return( 0 );
}
