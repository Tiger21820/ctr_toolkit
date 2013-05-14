#include "lib.h"
#include "ctr_crypto.h"
#include "ncch.h"

int GetCoreContentNCCH(CORE_CONTENT_INFO *core,FILE *ncch)
{
	NCCH_HEADER header = GetNCCHHeader(ncch);
	if(CheckNCCHHeader(&header) != 0){
		printf("[!] NCCH File is corrupt\n");
		return 1;
	}
	endian_strcpy(core->TitleID,header.title_id,0x8,LITTLE_ENDIAN);
	//endian_strcpy(core->TitleVersion,header.version,0x2,LITTLE_ENDIAN);
	return 0;
}

NCCH_HEADER GetNCCHHeader(FILE *ncch)
{
	NCCH_HEADER header;
	memset(&header,0x0,sizeof(NCCH_HEADER));
	fread(&header,sizeof(header),1,ncch);
	return header;
}

int CheckNCCHHeader(NCCH_HEADER *header)
{
	u32 magic = u8_to_u32(header->magic,BIG_ENDIAN);
	if(magic != NCCH_MAGIC)
		return 1;
	return 0;
}

int GetCXIStruct(CXI_STRUCT *ctx, FILE *ncch)
{
	NCCH_HEADER header = GetNCCHHeader(ncch);
	if(CheckNCCHHeader(&header) != 0){
		printf("[!] Content0 is corrupt\n");
		return 1;
	}
	
	memcpy(ctx->titleID,header.title_id,8);
	memcpy(ctx->programID,header.program_id,8);
	
	u8 flag_bool[8];
	resolve_flag(header.flags[5],flag_bool);
	if(flag_bool[1] != TRUE){
		printf("[!] Content0 is not a CXI file\n");
		return 1;
	}
	u32 media_unit = (0x200*(1+header.flags[6]));
	resolve_flag(header.flags[7],flag_bool);
	if(header.flags[7] != 0 && flag_bool[2] == TRUE){
		ctx->encrypted = FALSE;
	}
	else
		ctx->encrypted = TRUE;
	
	ctx->version = u8_to_u16(header.version,LITTLE_ENDIAN);
	ctx->exheader_offset = 0x200;
	ctx->exheader_size = u8_to_u32(header.extended_header_size,LITTLE_ENDIAN);
	ctx->exefs_offset = (u8_to_u32(header.exefs_offset,LITTLE_ENDIAN)*media_unit);
	ctx->exefs_size = (u8_to_u32(header.exefs_size,LITTLE_ENDIAN)*media_unit);
	return 0;
}

int GetCXIMetaPreStruct(META_STRUCT *meta, CXI_STRUCT *cxi_ctx, CIA_CONTEXT *ctx, FILE *ncch)
{
	u8 *ExHeader = malloc(cxi_ctx->exheader_size);
	fseek(ncch,cxi_ctx->exheader_offset,SEEK_SET);
	fread(ExHeader,cxi_ctx->exheader_size,1,ncch);
	if(cxi_ctx->encrypted == TRUE){
		u8 counter[0x10];
		ncch_get_counter(cxi_ctx,counter,NCCHTYPE_EXHEADER);
		ctr_aes_context aes_ctx;
		memset(&aes_ctx,0x0,sizeof(ctr_aes_context));
		ctr_init_counter(&aes_ctx, ctx->keys.ncch_key.key, counter);
		ctr_crypt_counter(&aes_ctx, ExHeader, ExHeader, cxi_ctx->exheader_size);
		
		if(memcmp((ExHeader+0x200),cxi_ctx->programID,8) != 0){
			printf("[!] CXI decryption failed\n");
			goto fail_cleanup;
		}
	}
	
	if(memcmp((ExHeader+0x200),cxi_ctx->programID,8) != 0){
		printf("[!] CXI is invalid\n");
		goto fail_cleanup;
	}
	
	memcpy(meta->DependList,(ExHeader+0x40),0x180);
	memcpy(&meta->CoreVersion,(ExHeader+0x3ff),1);
	
	free(ExHeader);
	return 0;
fail_cleanup:
	free(ExHeader);
	return 1;
}

int GetCXIIcon(COMPONENT_STRUCT *cxi_icon, CXI_STRUCT *cxi_ctx, CIA_CONTEXT *ctx, FILE *ncch)
{
	u8 *exefs = malloc(cxi_ctx->exefs_size);
	fseek(ncch,cxi_ctx->exefs_offset,SEEK_SET);
	fread(exefs,cxi_ctx->exefs_size,1,ncch);
	if(cxi_ctx->encrypted == TRUE){
		u8 counter[0x10];
		ncch_get_counter(cxi_ctx,counter,NCCHTYPE_EXEFS);
		ctr_aes_context aes_ctx;
		memset(&aes_ctx,0x0,sizeof(ctr_aes_context));
		ctr_init_counter(&aes_ctx, ctx->keys.ncch_key.key, counter);
		ctr_crypt_counter(&aes_ctx, exefs, exefs, cxi_ctx->exefs_size);
	}
	
	u8 exefs_icon_name[8] = {0x69, 0x63, 0x6F, 0x6E, 0x00, 0x00, 0x00, 0x00};
	
	u32 icon_offset;
	u32 icon_size;
	for(int i = 0; i < 8; i++){
		u32 headernameoffset = (i * 0x10);
		u32 headeroffsetoffset = ((i * 0x10) + 8);
		u32 headersizeoffset = ((i * 0x10) + 8 + 4);
		if(memcmp(exefs_icon_name,(exefs+headernameoffset),8) == 0){
			icon_offset = (u8_to_u32((exefs+headeroffsetoffset),LITTLE_ENDIAN) + 0x200);
			icon_size = u8_to_u32((exefs+headersizeoffset),LITTLE_ENDIAN);
			break;
		}
		if(i == 7){
			printf("[+] CXI has no Icon\n");
			free(exefs);
			return 0;
		}
	}
	cxi_icon->used = TRUE;
	cxi_icon->size = icon_size;
	cxi_icon->buffer = malloc(cxi_icon->size);
	memcpy(cxi_icon->buffer,(exefs+icon_offset),cxi_icon->size);
	free(exefs);
	return 0;
}

void ncch_get_counter(CXI_STRUCT *ctx, u8 counter[16], u8 type)
{
	u8* titleID = ctx->titleID;
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
		if (type == NCCHTYPE_EXHEADER)
			x = ctx->exheader_offset;
		else if (type == NCCHTYPE_EXEFS)
			x = ctx->exefs_offset;

		for(i=0; i<8; i++)
			counter[i] = titleID[i];
		for(i=0; i<4; i++)
			counter[12+i] = x>>((3-i)*8);
	}
}


void read_ncch(FILE *ncch)
{
	NCCH_HEADER header = GetNCCHHeader(ncch);
	
	printf("Product Code:       %s\n",header.product_code);
	
	memdump(stdout,"Flags :",header.flags,8);
	u8 flag_bool[8];
	if(header.flags[7] != 0){
		resolve_flag(header.flags[7],flag_bool);
		if(flag_bool[2] == TRUE)
			printf("Key:                None - Not Encrypted\n");
		else if(flag_bool[0] == TRUE)
			printf("Key:                Fixed\n");
	}
	else
		printf("Key:                Secure\n");
	
	u32 media_unit = (0x200*(1+header.flags[6]));
	printf("Media Unit:         0x%x\n",media_unit);
	if(header.flags[5] == 0){
		printf("[!] CXI Error\n");
		return;
	}
	resolve_flag(header.flags[5],flag_bool);
	printf("ROMFS Partition:    ");
	switch(flag_bool[0]){
		case TRUE : printf("Yes\n"); break;
		case FALSE : printf("No\n"); break;
	}
	printf("EXEFS Partition:    ");
	switch(flag_bool[1]){
		case TRUE : printf("Yes\n"); break;
		case FALSE : printf("No\n"); printf("[!] Error, NCCH file is a CFA\n"); return;
	}
	printf("NCCH Type:          ");
	if(flag_bool[1] == TRUE){
		switch(flag_bool[0]){
			case TRUE : printf("CXI (With ROMFS)"); break;
			case FALSE : printf("CXI"); break;
		}
	}
	else
		printf("CFA");
	
	if(flag_bool[2] == FALSE && flag_bool[3] == FALSE)
		printf(" (Normal)\n");
	if(flag_bool[2] == FALSE && flag_bool[3] == TRUE)
		printf(" (Manual)\n");
	if(flag_bool[2] == TRUE && flag_bool[3] == TRUE)
		printf(" (Child)\n");
	if(flag_bool[2] == TRUE && flag_bool[3] == FALSE)
		printf(" (Retail Update Container)\n");
	
	printf("ExHeader Offset:    0x%x\n",0x200);
	printf("ExHeader Size:      0x%x\n",u8_to_u32(header.extended_header_size,LITTLE_ENDIAN));
	
	printf("EXEFS Offset:       0x%x\n",u8_to_u32(header.exefs_offset,LITTLE_ENDIAN)*media_unit);
	printf("EXEFS Size:         0x%x\n",u8_to_u32(header.exefs_size,LITTLE_ENDIAN)*media_unit);
}