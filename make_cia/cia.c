#include "lib.h"
#include "ctr_crypto.h"
#include "settings.h"
#include "cia.h"

int SetupContentData(CIA_CONTEXT *ctx)
{	
	ctx->TotalContentSize = 0;	
	u64 ContentOffsetStart = align_value((0xB04+(0x30*ctx->ContentCount)),0x40) + align_value(ctx->ticket.size,0x40) + align_value(ctx->certchain.size,0x40) + align_value(sizeof(CIA_HEADER),0x40);
	
	ctx->ContentInfoMallocFlag = TRUE;
	ctx->ContentInfo = malloc(sizeof(CONTENT_INFO)*ctx->ContentCount);
	memset(ctx->ContentInfo,0x0,(sizeof(CONTENT_INFO)*ctx->ContentCount));
	
	for(u16 i = 0; i < ctx->ContentCount; i++){
		GetContentInfo(&ctx->ContentInfo[i],i,ctx->configfile.file.file);
		FILE *content_file = fopen(GetContentFilePath(&ctx->ContentInfo[i]),"rb");
		if(content_file == NULL){
			printf("[!] Failed to open '%s'\n",GetContentFilePath(&ctx->ContentInfo[i]));
			return 1;
		}
		u64 size = GetContentSize(content_file);
		u64 paddingsize = GetContentPaddingSize(content_file,0x10);
		u64 ContentSize = size + paddingsize;
		ctx->TotalContentSize += ContentSize;
		SetContentSize(&ctx->ContentInfo[i],ContentSize);
		u8 *buffer = malloc(ContentSize);
		memset(buffer+size,0x0,paddingsize);
		fread(buffer,size,1,content_file);
		ctr_sha_256(buffer,ContentSize,GetContentSHAHash(&ctx->ContentInfo[i]));
		if(ctx->ContentInfo->encrypted == TRUE){
			EncryptContent(buffer,buffer,ContentSize,ctx->keys.title_key.key,GetContentIndex(&ctx->ContentInfo[i]));
		}
		fseek(ctx->outfile.file.file,ContentOffsetStart,SEEK_SET);
		fwrite(buffer,ContentSize,1,ctx->outfile.file.file);
		ContentOffsetStart += ContentSize;
		free(buffer);
		fclose(content_file);
	}		
	return 0;
}

char* GetContentFilePath(CONTENT_INFO *ctx)
{
	return ctx->file_path;
}

u8* GetContentSHAHash(CONTENT_INFO *ctx)
{
	return ctx->sha_256_hash;
}

u16 GetContentIndex(CONTENT_INFO *ctx)
{
	return ctx->content_index;
}

void SetContentSize(CONTENT_INFO *ctx, u64 size)
{
	ctx->content_size = size;
}

int WriteSectionsToOutput(CIA_CONTEXT *ctx)
{
	fseek(ctx->outfile.file.file,0x0,SEEK_SET);
	fwrite(ctx->header.buffer,ctx->header.size,1,ctx->outfile.file.file);
	
	u64 offset = 0;
	
	//Write Certificate Chain
	offset = align_value(sizeof(CIA_HEADER),0x40);
	WriteBuffer(ctx->certchain.buffer,ctx->certchain.size,offset,ctx->outfile.file.file);
	
	//Write Ticket
	offset = align_value(ctx->certchain.size,0x40) + align_value(sizeof(CIA_HEADER),0x40);
	WriteBuffer(ctx->ticket.buffer,ctx->ticket.size,offset,ctx->outfile.file.file);
	
	//Write TMD
	offset = align_value(ctx->ticket.size,0x40) + align_value(ctx->certchain.size,0x40) + align_value(sizeof(CIA_HEADER),0x40);
	WriteBuffer(ctx->tmd.buffer,ctx->tmd.size,offset,ctx->outfile.file.file);
	
	//Write Meta
	if(ctx->meta.used == TRUE){
		offset = align_value(ctx->TotalContentSize,0x40) + align_value(ctx->tmd.size,0x40) + align_value(ctx->ticket.size,0x40) + align_value(ctx->certchain.size,0x40) + align_value(sizeof(CIA_HEADER),0x40);
		WriteBuffer(ctx->meta.buffer,ctx->meta.size,offset,ctx->outfile.file.file);
	}
	
	return 0;
}

void WriteBuffer(u8 *buffer, u64 size, u64 offset, FILE *output)
{
	fseek(output,offset,SEEK_SET);
	fwrite(buffer,size,1,output);
} 

int SetupCIAHeader(CIA_CONTEXT *ctx)
{
	ctx->header.used = TRUE;
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
	if(ctx->meta.used == TRUE)
		u32_to_u8(cia_header.meta_size,ctx->meta.size,LITTLE_ENDIAN);
	else
		u32_to_u8(cia_header.meta_size,0x0,LITTLE_ENDIAN);
	u64_to_u8(cia_header.content_size,ctx->TotalContentSize,LITTLE_ENDIAN);
	cia_header.magic_0 = 0x80;
	
	memcpy(ctx->header.buffer,&cia_header,ctx->header.size);
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

u64 GetContentSize(FILE *content)
{
	u64 size = 0;
	fseek(content, 0L, SEEK_END);
	size = ftell(content);
	fseek(content, 0L, SEEK_SET);
	return size;
}

u64 GetContentPaddingSize(FILE *content, u32 alignment)
{
	u64 filesize = GetContentSize(content);
	u64 adjusted_filesize = align_value(filesize,alignment);
	return (adjusted_filesize - filesize);
}