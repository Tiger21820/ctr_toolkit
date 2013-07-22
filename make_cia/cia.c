#include "lib.h"
#include "ctr_crypto.h"
#include "settings.h"
#include "cia.h"

int SetupContent(CIA_CONTEXT *ctx)
{
	ctx->content.used = True;
	u64 TotalContentSize = 0;
	for(u16 i = 0; i < ctx->ContentCount; i++){
		if(ctx->build_mode != rom_conv){
			u64 Size = GetFileSize_u64(ctx->ContentInfo[i].file_path);
			ctx->ContentInfo[i].content_size = Size + GetContentPaddingSize(Size,0x10);
		}
		TotalContentSize += ctx->ContentInfo[i].content_size;
	}
	ctx->content.size = TotalContentSize;
	ctx->content.buffer = malloc(ctx->content.size);
	memset(ctx->content.buffer,0xff,ctx->content.size);
	
	u64 ContentOffsetStart = 0;
	for(u16 i = 0; i < ctx->ContentCount; i++){
		u64 TrueSize = 0;
		if(ctx->build_mode == rom_conv)
			TrueSize = ctx->ContentInfo[i].content_size;
		else
			TrueSize = GetFileSize_u64(ctx->ContentInfo[i].file_path);
		
		//printf("Content %d has a True size of 0x%llx and an actual Size of 0x%llx @ offset 0x%llx\n",i,TrueSize,ctx->ContentInfo[i].content_size,ctx->ContentInfo[i].file_offset);
		
		FILE *content_file = fopen(ctx->ContentInfo[i].file_path,"rb");
		if(content_file == NULL){
			printf("[!] Failed to open '%s'\n",ctx->ContentInfo[i].file_path);
			return 1;
		}
		fseek_64(content_file,ctx->ContentInfo[i].file_offset,SEEK_SET);
		fread((ctx->content.buffer + ContentOffsetStart),TrueSize,1,content_file);
		ctr_sha_256((ctx->content.buffer + ContentOffsetStart),ctx->ContentInfo[i].content_size,ctx->ContentInfo[i].sha_256_hash);
		if(ctx->ContentInfo[i].encrypted == True){
			EncryptContent((ctx->content.buffer + ContentOffsetStart),(ctx->content.buffer + ContentOffsetStart),ctx->ContentInfo[i].content_size,ctx->keys.title_key.key,ctx->ContentInfo[i].content_index);
		}
		ContentOffsetStart += ctx->ContentInfo[i].content_size;
		fclose(content_file);
	}	
	
	return 0;
}

int SetupCIAHeader(CIA_CONTEXT *ctx)
{
	ctx->header.used = True;
	ctx->header.size = sizeof(CIA_HEADER);
	ctx->header.buffer = malloc(ctx->header.size);
	
	CIA_HEADER cia_header;
	memset(&cia_header,0x0,sizeof(cia_header));
	
	u32_to_u8(cia_header.header_size,sizeof(cia_header),LITTLE_ENDIAN);
	u16_to_u8(cia_header.type,0x0,LITTLE_ENDIAN);
	u16_to_u8(cia_header.version,0x0,LITTLE_ENDIAN);
	u32_to_u8(cia_header.cert_size,ctx->certchain.size,LITTLE_ENDIAN);
	u32_to_u8(cia_header.tik_size,ctx->ticket.size,LITTLE_ENDIAN);
	u32_to_u8(cia_header.tmd_size,ctx->tmd.size,LITTLE_ENDIAN);
	if(ctx->meta.used == True)
		u32_to_u8(cia_header.meta_size,ctx->meta.size,LITTLE_ENDIAN);
	else
		u32_to_u8(cia_header.meta_size,0x0,LITTLE_ENDIAN);
	u64_to_u8(cia_header.content_size,ctx->content.size,LITTLE_ENDIAN);
	
	//SetCIAContentIndex
	
	u64 content_index_flag = 0;
	for(int i = 0; i < ctx->ContentCount; i++){
		content_index_flag += (0x8000000000000000/(2<<ctx->ContentInfo[i].content_index))*2;
	}
	u64_to_u8(cia_header.content_index,content_index_flag,BE);
	
	memcpy(ctx->header.buffer,&cia_header,ctx->header.size);
	return 0;
}

int WriteSectionsToOutput(CIA_CONTEXT *ctx)
{
	/**
	if(ctx->outfile.used == False){
		ctx->outfile.used = True;
		ctx->outfile.arg_len = 20;
		ctx->outfile.argument = malloc(ctx->outfile.arg_len);
		sprintf(ctx->outfile.argument,"%x%02x%02x.cia",ctx->core.TitleID[4],ctx->core.TitleID[5],ctx->core.TitleID[6]);
	}
	**/
	if(ctx->outfile.used != True){
		ctx->outfile.used = True;
		ctx->outfile.arg_len = ctx->core_infile.arg_len + 4;
		ctx->outfile.argument = malloc(ctx->outfile.arg_len+1);
		sprintf(ctx->outfile.argument,"%s.cia",ctx->core_infile.argument);
	}
	
	ctx->outfile.file.used = True;
	ctx->outfile.file.file = fopen(ctx->outfile.argument,"wb");
	if(ctx->outfile.file.file == NULL){
		printf("[!] IO ERROR: Failed to create '%s'\n",ctx->outfile.argument);
		ctx->outfile.file.used = False;
		return 1;
	}

	u64 offset = 0;
	//Writing Header
	offset = 0;
	WriteBuffer(ctx->header.buffer,ctx->header.size,offset,ctx->outfile.file.file);
	
	//Write Certificate Chain
	offset = align_value(sizeof(CIA_HEADER),0x40);
	WriteBuffer(ctx->certchain.buffer,ctx->certchain.size,offset,ctx->outfile.file.file);
	
	//Write Ticket
	offset = align_value(ctx->certchain.size,0x40) + align_value(sizeof(CIA_HEADER),0x40);
	WriteBuffer(ctx->ticket.buffer,ctx->ticket.size,offset,ctx->outfile.file.file);
	
	//Write TMD
	offset = align_value(ctx->ticket.size,0x40) + align_value(ctx->certchain.size,0x40) + align_value(sizeof(CIA_HEADER),0x40);
	WriteBuffer(ctx->tmd.buffer,ctx->tmd.size,offset,ctx->outfile.file.file);
	
	//Write Contents
	offset = align_value(ctx->tmd.size,0x40) + align_value(ctx->ticket.size,0x40) + align_value(ctx->certchain.size,0x40) + align_value(sizeof(CIA_HEADER),0x40);
	WriteBuffer(ctx->content.buffer,ctx->content.size,offset,ctx->outfile.file.file);
	
	//Write Meta
	if(ctx->meta.used == True){
		offset = align_value(ctx->content.size,0x40) + align_value(ctx->tmd.size,0x40) + align_value(ctx->ticket.size,0x40) + align_value(ctx->certchain.size,0x40) + align_value(sizeof(CIA_HEADER),0x40);
		WriteBuffer(ctx->meta.buffer,ctx->meta.size,offset,ctx->outfile.file.file);
	}
	
	return 0;
}

int EncryptContent(u8 *EncBuffer,u8 *buffer,u64 size,u8 *title_key, u16 index)
{
	//generating IV
	u8 iv[16];
	memset(&iv,0x0,16);
	iv[0] = (index >> 8) & 0xff;
	iv[1] = index & 0xff;
	//Encrypting content
	ctr_aes_context ctx;
	memset(&ctx,0x0,sizeof(ctr_aes_context));
	ctr_init_aes_cbc(&ctx,title_key,iv,ENC);
	ctr_aes_cbc(&ctx,buffer,EncBuffer,size,ENC);
	return 0;
}

u64 GetContentPaddingSize(u64 ContentSize, u32 alignment)
{
	return (align_value(ContentSize,alignment) - ContentSize);
}

