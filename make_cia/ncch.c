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
#include "ncch.h"

int GetCoreContentNCCH(USER_CONTEXT *ctx, CORE_CONTENT_INFO *core, u32 offset, FILE *ncch)
{
	NCCH_HEADER header = GetNCCHHeader(offset,ncch);
	if(CheckNCCHHeader(&header) != 0){
		printf("[!] NCCH File is corrupt\n");
		return 1;
	}
	endian_memcpy(core->TitleID,header.title_id,0x8,LE);
	
	NCCH_STRUCT *cxi_ctx = malloc(sizeof(NCCH_STRUCT));
	if(cxi_ctx == NULL){
		printf("[!] Memory Allocation Failure\n");
		return Fail;
	}
	memset(cxi_ctx,0x0,sizeof(NCCH_STRUCT));
	GetCXIStruct(cxi_ctx,offset,ncch);
	
	//if(ctx->info){
	//	printf("[+] Content0 NCCH Info\n");
	//	read_ncch(offset,ncch);	
	//}

	if(cxi_ctx->is_cfa == True)
		return 0;
	
	u8 *ExHeader = malloc(cxi_ctx->exheader_size);
	if(ExHeader == NULL){
		printf("[!] Memory Error\n");
		goto fail_cleanup;
	}
	
	ctx->flags[gen_meta] = True;
	
	fseek(ncch,cxi_ctx->exheader_offset+offset,SEEK_SET);
	fread(ExHeader,cxi_ctx->exheader_size,1,ncch);
	if(cxi_ctx->encrypted != No_Key){
		CryptNCCHSection(ExHeader,cxi_ctx,ctx->keys.cxi_key[cxi_ctx->encrypted],NCCHTYPE_EXHEADER);
		if(memcmp((ExHeader+0x200),cxi_ctx->programID,8) != 0){
			printf("[!] CXI decryption failed (Check CXIKey)\n");
			printf("[!] Actual savedata size, title version and Meta region could not be obtained\n");
			ctx->flags[gen_meta] = False;
			goto cleanup;
		}
	}
	if(memcmp((ExHeader+0x200),cxi_ctx->programID,8) != 0){
		printf("[!] CXI Corrupt\n");
		goto fail_cleanup;
	}
	
	
	u16 ver = (u8_to_u16(ExHeader+0xe,LE)*4);
	u16_to_u8(core->TitleVersion,ver,LE);
	memcpy(core->save_data_size,ExHeader+0x1c0,4);
	memset(core->priv_save_data_size,0,4);
	memset(core->twl_data,0,4);
	
cleanup:
	_free(cxi_ctx);
	_free(ExHeader);
	return 0;
fail_cleanup:
	_free(cxi_ctx);
	_free(ExHeader);
	return 1;
}

void CryptNCCHSection(u8 *buffer, NCCH_STRUCT *ctx, u8 key[16], u8 type)
{
	if(type < 1 || type > 3)
		return;
	
	u8 counter[0x10];
	ncch_get_counter(ctx,counter,type);	
	ctr_aes_context aes_ctx;
	memset(&aes_ctx,0x0,sizeof(ctr_aes_context));
	ctr_init_counter(&aes_ctx, key, counter);
	u32 size = 0;
	switch(type){
		case NCCHTYPE_EXHEADER : size = ctx->exheader_size; break;
		case NCCHTYPE_EXEFS : size = ctx->exefs_size; break;
		case NCCHTYPE_ROMFS : size = ctx->romfs_size; break;
	}
	ctr_crypt_counter(&aes_ctx, buffer, buffer, size);
	return;
}

NCCH_HEADER GetNCCHHeader(u32 offset, FILE *ncch)
{
	NCCH_HEADER header;
	memset(&header,0x0,sizeof(NCCH_HEADER));
	fseek(ncch,offset+0x100,SEEK_SET);
	fread(&header,sizeof(header),1,ncch);
	return header;
}

int CheckNCCHHeader(NCCH_HEADER *header)
{
	if(u8_to_u32(header->magic,BE) != NCCH_MAGIC)
		return 1;
	return 0;
}

int VerifyNCCH(USER_CONTEXT *ctx, u32 offset, FILE *ncch)
{
	NCCH_STRUCT *cxi_ctx = malloc(sizeof(NCCH_STRUCT));
	if(cxi_ctx == NULL){
		printf("[!] Failed to allocate memory (0x%x bytes) for CXI Context\n",sizeof(NCCH_STRUCT));
		return 1; 
	}
	memset(cxi_ctx,0x0,sizeof(NCCH_STRUCT));
	GetCXIStruct(cxi_ctx,offset,ncch);
	
	u8 HeaderSignature[0x100];
	u8 Header[0x100];
	u8 HeaderSHAHash[0x20];
	memset(&HeaderSignature,0x0,0x100);
	memset(&Header,0x0,0x100);
	memset(&HeaderSHAHash,0x0,0x20);
	fseek(ncch,offset+0x0,SEEK_SET);
	fread(HeaderSignature,0x100,1,ncch);
	fread(Header,0x100,1,ncch);
	ctr_sha_256(&Header,0x100,HeaderSHAHash);
	
	u8 modulus[0x100];
	
		
	if(cxi_ctx->is_cfa == True)
		memcpy(modulus,ctx->keys.NcsdCfa.n,0x100);
		
	else
		goto prep_cxi_validate;
		
validate_header:
	switch(ctr_rsa2048_sha256_verify(HeaderSHAHash,HeaderSignature,modulus)){
		case Good : printf("[+] NCCH header is valid\n"); break;
		case Fail : printf("[+] NCCH header is invalid\n"); break;
	}
	return 0;

prep_cxi_validate:
	//Getting Exheader
	fseek(ncch,offset+cxi_ctx->exheader_offset,SEEK_SET);
	u8 *ExHeader = malloc(0x800);
	if(ExHeader == NULL){
		printf("[!] Failed to allocate memory (0x%x bytes) for ExHeader\n",0x800);
		return 1; 
	}
	memset(ExHeader,0x0,0x800);
	fread(ExHeader,0x800,1,ncch);
	if(cxi_ctx->encrypted != No_Key){
		CryptNCCHSection(ExHeader,cxi_ctx,ctx->keys.cxi_key[cxi_ctx->encrypted],NCCHTYPE_EXHEADER);
		if(memcmp((ExHeader+0x200),cxi_ctx->programID,8) != 0){
			printf("[!] CXI decryption failed\n");
			_free(ExHeader);
			return 1;
		}
	}
	memcpy(modulus,ExHeader+0x500,0x100);
	_free(ExHeader);
	goto validate_header;
}

int GetCXIStruct(NCCH_STRUCT *ctx, u32 offset, FILE *ncch)
{
	NCCH_HEADER header = GetNCCHHeader(offset,ncch);
	if(CheckNCCHHeader(&header) != 0){
		printf("[!] Content0 is corrupt\n");
		return 1;
	}
	
	memcpy(ctx->titleID,header.title_id,8);
	memcpy(ctx->programID,header.program_id,8);
	
	u8 flag_bool[8];
	resolve_flag(header.flags[5],flag_bool);
	if(flag_bool[1] != True){
		ctx->is_cfa = True;
	}
	u32 media_unit = (0x200*(1+header.flags[6]));
	resolve_flag(header.flags[7],flag_bool);
	if(header.flags[7] == 0)
		ctx->encrypted = Secure;
	else if(header.flags[7] == 7){
		ctx->encrypted = No_Key;
	}
	else if(flag_bool[1] == True)
		ctx->encrypted = GetFixedKeyType(header.title_id);
	
	ctx->version = u8_to_u16(header.version,LE);
	if(ctx->is_cfa != True){
		ctx->exheader_offset = 0x200;
		ctx->exheader_size = u8_to_u32(header.extended_header_size,LE) + 0x400;
		ctx->exefs_offset = (u8_to_u32(header.exefs_offset,LE)*media_unit);
		ctx->exefs_size = (u8_to_u32(header.exefs_size,LE)*media_unit);
	}
	ctx->romfs_offset = (u8_to_u32(header.romfs_offset,LE)*media_unit);
	ctx->romfs_size = (u8_to_u32(header.romfs_size,LE)*media_unit);
	return 0;
}

int GetCXIMetaPreStruct(META_STRUCT *meta, NCCH_STRUCT *cxi_ctx, USER_CONTEXT *ctx, u32 offset, FILE *ncch)
{
	u8 *ExHeader = malloc(cxi_ctx->exheader_size);
	if(ExHeader == NULL){
		printf("[!] Failed to allocate memory (0x%x bytes) for ExeFS\n",cxi_ctx->exheader_size);
		return 1; 
	}	
	fseek(ncch,offset+cxi_ctx->exheader_offset,SEEK_SET);
	fread(ExHeader,cxi_ctx->exheader_size,1,ncch);
	if(cxi_ctx->encrypted != No_Key){
		CryptNCCHSection(ExHeader,cxi_ctx,ctx->keys.cxi_key[cxi_ctx->encrypted],NCCHTYPE_EXHEADER);
	}
	
	memcpy(meta->DependList,(ExHeader+0x40),0x180);
	memcpy(meta->CoreVersion,(ExHeader+0x208),4);
	
	_free(ExHeader);
	return 0;
}

int GetCXIIcon(COMPONENT_STRUCT *cxi_icon, NCCH_STRUCT *cxi_ctx, USER_CONTEXT *ctx, u32 offset, FILE *ncch)
{
	u8 *exefs = malloc(cxi_ctx->exefs_size);
	if(exefs == NULL){
		printf("[!] Failed to allocate memory (0x%x bytes) for ExeFS\n",cxi_ctx->exefs_size);
		return 1; 
	}
	fseek(ncch,offset+cxi_ctx->exefs_offset,SEEK_SET);
	fread(exefs,cxi_ctx->exefs_size,1,ncch);
	if(cxi_ctx->encrypted != No_Key){
		CryptNCCHSection(exefs,cxi_ctx,ctx->keys.cxi_key[cxi_ctx->encrypted],NCCHTYPE_EXEFS);
	}
	
	u8 exefs_icon_name[8] = {0x69, 0x63, 0x6F, 0x6E, 0x00, 0x00, 0x00, 0x00};
	
	u32 icon_offset;
	u32 icon_size;
	for(int i = 0; i < 10; i++){
		u32 headernameoffset = (i * 0x10);
		u32 headeroffsetoffset = ((i * 0x10) + 8);
		u32 headersizeoffset = ((i * 0x10) + 8 + 4);
		if(memcmp(exefs_icon_name,(exefs+headernameoffset),8) == 0){
			icon_offset = (u8_to_u32((exefs+headeroffsetoffset),LE) + 0x200);
			icon_size = u8_to_u32((exefs+headersizeoffset),LE);
			break;
		}
		if(i == 9){
			printf("[+] CXI has no Icon\n");
			_free(exefs);
			return 0;
		}
	}
	cxi_icon->size = icon_size;
	cxi_icon->buffer = malloc(cxi_icon->size);
	if(cxi_icon->buffer == NULL){
		printf("[!] Failed to allocate memory (0x%llx bytes) for storing Icon data\n",cxi_icon->size);
		return 1; 
	}
	memcpy(cxi_icon->buffer,(exefs+icon_offset),cxi_icon->size);
	_free(exefs);
	return 0;
}

int GetFixedKeyType(u8 TitleID[8])
{
	u16 tid_cat = u8_to_u16(TitleID+4,LE);
	u8 bit_flag[16];
	resolve_flag_u16(tid_cat,bit_flag);
	if(bit_flag[15] == False && bit_flag[14] == False && bit_flag[4] == True){
		return SystemFixed;
	}
	else{
		return ZerosFixed;
	}
}

void ncch_get_counter(NCCH_STRUCT *ctx, u8 counter[16], u8 type)
{
	u8 *titleID = ctx->titleID;
	u32 i;
	u32 x = 0;

	memset(counter, 0, 16);

	if (ctx->version == 2 || ctx->version == 0)
	{
		for(i=0; i<8; i++)
			counter[i] = titleID[7-i];
		counter[8] = type;
	}
	else if (ctx->version == 1)
	{
		switch(type){
			case NCCHTYPE_EXHEADER : x = ctx->exheader_offset; break;
			case NCCHTYPE_EXEFS : x = ctx->exefs_offset; break;
			case NCCHTYPE_ROMFS : x = ctx->romfs_offset; break;
		}
		for(i=0; i<8; i++)
			counter[i] = titleID[i];
		for(i=0; i<4; i++)
			counter[12+i] = x>>((3-i)*8);
	}
}


void read_ncch(u32 offset, FILE *ncch)
{
	NCCH_HEADER header = GetNCCHHeader(offset,ncch);
	
	printf(" Product Code:       %s\n",header.product_code);
	
	memdump(stdout," Flags :             ",header.flags,8);
	u8 flag_bool[8];
	if(header.flags[7]){
		resolve_flag(header.flags[7],flag_bool);
		if(header.flags[7] == 4)
			printf(" > Key:                None - Not Encrypted\n");
		else if(flag_bool[0] == True)
			printf(" > Key:                Fixed\n");
	}
	else
		printf(" > Key:                Secure\n");
	
	u32 media_unit = (0x200*(1+header.flags[6]));
	printf(" > Media Unit:         0x%x\n",media_unit);
	if(header.flags[5] == 0){
		printf("[!] NCCH Error\n");
		return;
	}
	resolve_flag(header.flags[5],flag_bool);
	printf(" > RomFS Partition:    ");
	switch(flag_bool[0]){
		case True : printf("Yes\n"); break;
		case False : printf("No\n"); break;
	}
	printf(" > ExeFS Partition:    ");
	switch(flag_bool[1]){
		case True : printf("Yes\n"); break;
		case False : printf("No\n"); printf("[!] Error, NCCH file is a CFA\n"); return;
	}
	printf(" > Content Type:       ");
	if(flag_bool[1] == True){
		switch(flag_bool[0]){
			case True : printf("CXI (With ROMFS)"); break;
			case False : printf("CXI"); break;
		}
	}
	else
		printf("CFA");
	
	if(flag_bool[2] == False && flag_bool[3] == False)
		printf(" (Normal)\n");
	if(flag_bool[2] == False && flag_bool[3] == True)
		printf(" (Manual)\n");
	if(flag_bool[2] == True && flag_bool[3] == True)
		printf(" (Child)\n");
	if(flag_bool[2] == True && flag_bool[3] == False)
		printf(" (Retail Update Container)\n");
	
	printf(" ExHeader Offset:    0x%x\n",0x200);
	printf(" ExHeader Size:      0x%x\n",u8_to_u32(header.extended_header_size,LE));
	
	printf(" ExeFS Offset:       0x%x\n",u8_to_u32(header.exefs_offset,LE)*media_unit);
	printf(" ExeFS Size:         0x%x\n",u8_to_u32(header.exefs_size,LE)*media_unit);
	printf(" RomFS Offset:       0x%x\n",u8_to_u32(header.romfs_offset,LE)*media_unit);
	printf(" RomFS Size:         0x%x\n",u8_to_u32(header.romfs_size,LE)*media_unit);
}
