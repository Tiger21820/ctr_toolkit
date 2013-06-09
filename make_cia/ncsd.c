#include "lib.h"
#include "ctr_crypto.h"
#include "ncsd.h"

int GetNCSDData(CIA_CONTEXT *ctx, NCSD_STRUCT *ncsd_struct, FILE *ncsd)
{
	if(ncsd_struct == NULL)
		return Fail;
	memset(ncsd_struct,0x0,sizeof(NCSD_STRUCT));
	
	NCSD_HEADER header;
	CARD_INFO_HEADER card_info;
	DEV_CARD_INFO_HEADER dev_card_info;
	
	fseek(ncsd,0x0,SEEK_SET);
	fread(&ncsd_struct->signature,0x100,1,ncsd);
	fseek(ncsd,0x100,SEEK_SET);
	fread(&header,sizeof(NCSD_HEADER),1,ncsd);
	fseek(ncsd,0x200,SEEK_SET);
	fread(&card_info,sizeof(CARD_INFO_HEADER),1,ncsd);
	fseek(ncsd,0x1200,SEEK_SET);
	fread(&dev_card_info,sizeof(DEV_CARD_INFO_HEADER),1,ncsd);
	
	ctr_sha_256(&header,sizeof(NCSD_HEADER),ncsd_struct->ncsd_header_hash);
	
	ncsd_struct->sig_valid = ctr_rsa2048_sha256_verify(ncsd_struct->ncsd_header_hash,ncsd_struct->signature,ctx->keys.NcsdCfa.n);
	
	u32 media_size = ((header.partition_flags[6] + 1)*0x200);
	
	ncsd_struct->rom_size = u8_to_u32(header.rom_size,LITTLE_ENDIAN)*media_size;
	ncsd_struct->used_rom_size = 0;
	if(ncsd_struct->used_rom_size == 0){
		u32 tmp = u8_to_u32(header.offsetsize_table[0].offset,LITTLE_ENDIAN);
		for(int i = 0; i < 8; i++){
			tmp += u8_to_u32(header.offsetsize_table[i].size,LITTLE_ENDIAN);
		}
		ncsd_struct->used_rom_size = tmp*media_size;
	}
		
	
	for(int i = 0; i < 8; i++){
		ncsd_struct->partition_data[i].offset = u8_to_u32(header.offsetsize_table[i].offset,LITTLE_ENDIAN)*media_size;
		ncsd_struct->partition_data[i].size = u8_to_u32(header.offsetsize_table[i].size,LITTLE_ENDIAN)*media_size;
		if(ncsd_struct->partition_data[i].offset != 0 && ncsd_struct->partition_data[i].size != 0)
			ncsd_struct->partition_data[i].active = True;
		ncsd_struct->partition_data[i].title_id = u8_to_u64(header.partition_id_table[i],LITTLE_ENDIAN);
		ncsd_struct->partition_data[i].fs_type = header.partitions_fs_type[i];
		ncsd_struct->partition_data[i].crypto_type = header.partitions_crypto_type[i];
	}
	
	if(u8_to_u64(card_info.nver_title_id,LITTLE_ENDIAN) == 0){
		u8 stock_title_key[0x10] = {0x6E, 0xC7, 0x5F, 0xB2, 0xE2, 0xB4, 0x87, 0x46, 0x1E, 0xDD, 0xCB, 0xB8, 0x97, 0x11, 0x92, 0xBA};
		if(memcmp(dev_card_info.TitleKey,stock_title_key,0x10) == 0)
			ncsd_struct->type = dev_external_SDK;
		else
			ncsd_struct->type = dev_internal_SDK;
	}
	else
		ncsd_struct->type = retail;
	
	/**
	if(ncsd_struct->type != retail){
		u8 iv[16];
		u8 key[16];
		memset(&iv,0x0,16);
		memset(&key,0x0,16);
		//iv[0] = header.partition_flags[7];
		//memcpy(iv+11,header.partition_flags,5);
		u8 tmp[16] = {0xB2, 0x57, 0xA7, 0xC0, 0x24, 0xC8, 0xC1, 0xB0, 0x75, 0x91, 0xC4, 0xC5, 0x1D, 0x96, 0x67, 0x4F};
		
		memcpy(iv,tmp,16);
		memcpy(key,common_dpki_aesKey,16);
		
		ctr_aes_context aes;
		memset(&aes,0x0,sizeof(ctr_aes_context));
		
		memdump(stdout,"ENC Title Key:   ",dev_card_info.TitleKey,0x10);
		
		ctr_init_aes_cbc(&aes,key,iv,DEC);
		ctr_aes_cbc(&aes,dev_card_info.TitleKey,dev_card_info.TitleKey,0x10,DEC);
		
		memdump(stdout,"DEC Title Key:   ",dev_card_info.TitleKey,0x10);
	}
	**/
	/**
	for(int i = 0; i < 8; i++){
		if(ncsd_struct->partition_data[i].active == True){
			ncsd_struct->partition_data[i].sig_valid = VerifyNCCHSection(ctx,dev_card_info.TitleKey,ncsd_struct->partition_data[i].offset,ncsd);
		}
	}
	**/
	ncsd_struct->valid = True;
	if(ctx->verbose_flag)
		PrintNCSDData(ncsd_struct,&header,&card_info,&dev_card_info);
	return 0;
}

int VerifyNCSD(CIA_CONTEXT *ctx, FILE *ncsd)
{	
	u8 HeaderSignature[0x100];
	u8 Header[0x100];
	u8 HeaderSHAHash[0x20];
	memset(&HeaderSignature,0x0,0x100);
	memset(&Header,0x0,0x100);
	memset(&HeaderSHAHash,0x0,0x20);
	fseek(ncsd,0x0,SEEK_SET);
	fread(HeaderSignature,0x100,1,ncsd);
	fread(Header,0x100,1,ncsd);
	ctr_sha_256(&Header,0x100,HeaderSHAHash);	
	switch(ctr_rsa2048_sha256_verify(HeaderSHAHash,HeaderSignature,ctx->keys.NcsdCfa.n)){
		case Good : printf("[+] NCSD header is valid\n"); break;
		case Fail : printf("[+] NCSD header is invalid\n"); break;
	}
	return 0;
}

int VerifyNCCHSection(CIA_CONTEXT *ctx, u8 ncch_key[0x10], u32 offset, FILE *ncch)
{
	NCCH_STRUCT *cxi_ctx = malloc(sizeof(NCCH_STRUCT));
	memset(cxi_ctx,0x0,sizeof(NCCH_STRUCT));
	GetCXIStruct(cxi_ctx,offset,ncch);
	
	u8 HeaderSignature[0x100];
	u8 Header[0x100];
	u8 HeaderSHAHash[0x20];
	u8 ExHeader[0x800];
	memset(&HeaderSignature,0x0,0x100);
	memset(&Header,0x0,0x100);
	memset(&HeaderSHAHash,0x0,0x20);
	fseek(ncch,offset+0x0,SEEK_SET);
	fread(HeaderSignature,0x100,1,ncch);
	fread(Header,0x100,1,ncch);
	ctr_sha_256(&Header,0x100,HeaderSHAHash);
	
	RSA_2048_KEY HeaderRSA;
	memset(&HeaderRSA,0x0,sizeof(RSA_2048_KEY));
	u8 Exponent[0x3] = {0x01,0x00,0x01};
	memcpy(HeaderRSA.e,Exponent,0x3);
		
	if(cxi_ctx->is_cfa == True)
		memcpy(HeaderRSA.n,ctx->keys.NcsdCfa.n,0x100);
		
	else
		goto prep_cxi_validate;
		
validate_header:
	return ctr_rsa2048_sha256_verify(HeaderSHAHash,HeaderSignature,ctx->keys.NcsdCfa.n);

prep_cxi_validate:
	//Getting Exheader
	memset(&ExHeader,0x0,0x800);
	fseek(ncch,offset+cxi_ctx->exheader_offset,SEEK_SET);
	fread(&ExHeader,0x800,1,ncch);
	if(cxi_ctx->encrypted == True){
		u8 counter[0x10];
		ncch_get_counter(cxi_ctx,counter,NCCHTYPE_EXHEADER);
		ctr_aes_context aes_ctx;
		memset(&aes_ctx,0x0,sizeof(ctr_aes_context));
		ctr_init_counter(&aes_ctx, ncch_key, counter);
		ctr_crypt_counter(&aes_ctx, ExHeader, ExHeader, 0x800);
		if(memcmp((ExHeader+0x200),cxi_ctx->programID,8) != 0){
			printf("[!] CXI decryption failed\n");
			return Fail;
		}
	}
	
	memcpy(HeaderRSA.n,ExHeader+0x500,0x100);
	goto validate_header;
}

void PrintNCSDData(NCSD_STRUCT *ctx, NCSD_HEADER *header, CARD_INFO_HEADER *card_info, DEV_CARD_INFO_HEADER *dev_card_info)
{
	if(ctx->valid != True){
		printf("[!] NCSD Corrupt\n");
	}
	printf("[+] NCSD\n");
#ifndef _DEBUG_KEY_BUILD_
	switch(ctx->sig_valid){
		case Good : memdump(stdout,"Signature(Good):        ",ctx->signature,0x100); break;
		case Fail : memdump(stdout,"Signature(Fail):        ",ctx->signature,0x100); break;
		case NotChecked: memdump(stdout,"Signature:              ",ctx->signature,0x100); break;
	}
#endif

#ifdef _DEBUG_KEY_BUILD_
	if(ctx->type == retail){
		memdump(stdout,"Signature:              ",ctx->signature,0x100);
	}
	else{
		switch(ctx->sig_valid){
			case Good : memdump(stdout,"Signature(Good):        ",ctx->signature,0x100); break;
			case Fail : memdump(stdout,"Signature(Fail):        ",ctx->signature,0x100); break;
			case NotChecked: memdump(stdout,"Signature:              ",ctx->signature,0x100); break;
		}
	}
#endif
	switch(ctx->type){
		case retail : 
			printf("Target:                 Retail/Production\n"); 
			printf("NVer Title ID:          %016llx\n",u8_to_u64(card_info->nver_title_id,LITTLE_ENDIAN));
			printf("NVer Title Ver:         v%d\n",u8_to_u16(card_info->nver_title_version,LITTLE_ENDIAN));
			break;
		case dev_internal_SDK :
			printf("Target:                 Debug/Development\n");
			printf("SDK Type:               Nintendo Internal SDK\n");
			memdump(stdout,"Title Key:              ",dev_card_info->TitleKey,0x10);
			break;
		case dev_external_SDK :
			printf("Target:                 Debug/Development\n");
			printf("SDK Type:               Nintendo 3RD Party SDK\n");
			memdump(stdout,"Title Key:              ",dev_card_info->TitleKey,0x10);
			break;
	}
	if(ctx->rom_size >= GB){
		printf("NCSD ROM Cart Size:     %d GB",ctx->rom_size/GB); printf(" (%d Gbit)\n",(ctx->rom_size/GB)*8);
	}
	else{
		printf("NCSD ROM Cart Size:     %d MB",ctx->rom_size/MB); 
		u32 tmp = (ctx->rom_size/MB)*8;
		if(tmp >= 1024)
			printf(" (%d Gbit)\n",tmp/1024);
		else
			printf(" (%d Mbit)\n",tmp);
	}
	if(ctx->used_rom_size >= MB){
		printf("NCSD File Size:         %d MB",ctx->used_rom_size/MB); printf(" (0x%x bytes)\n",ctx->used_rom_size);
	}
	else if(ctx->used_rom_size >= KB){
		printf("NCSD File Size:         %d KB",ctx->used_rom_size/KB); printf(" (0x%x bytes)\n",ctx->used_rom_size);
	}
	printf("NCSD Title ID:          %016llx\n",u8_to_u64(header->title_id,LITTLE_ENDIAN));
	memdump(stdout,"ExHeader Hash:          ",header->exheader_hash,0x20);
	printf("AddHeader Size:         0x%x\n",u8_to_u32(header->additional_header_size,LITTLE_ENDIAN));
	printf("Sector 0 Offset:        0x%x\n",u8_to_u32(header->sector_zero_offset,LITTLE_ENDIAN));
	memdump(stdout,"Flags:                  ",header->partition_flags,8);
	printf("\n");
	for(int i = 0; i < 8; i++){
		if(ctx->partition_data[i].active == True){
			printf("Partition %d\n",i);
			printf(" Title ID:              %016llx\n",ctx->partition_data[i].title_id);
			printf(" FS Type:               %x\n",ctx->partition_data[i].fs_type);
			printf(" Crypto Type:           %x\n",ctx->partition_data[i].crypto_type);
			printf(" Offset:                0x%x\n",ctx->partition_data[i].offset);
			printf(" Size:                  0x%x\n",ctx->partition_data[i].size);
			printf("\n");
		}
	}
}