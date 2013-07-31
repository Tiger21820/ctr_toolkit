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
#include "settings.h"
#include "cia.h"

int SetupContent(USER_CONTEXT *ctx)
{
	if(ctx->flags[verbose]) { printf("[+] Preparing CIA Content\n"); }
	u64 TotalContentSize = 0;
	for(u16 i = 0; i < ctx->ContentCount; i++){
		if(ctx->flags[build_mode] != rom_conv){
			u64 Size = GetFileSize_u64(ctx->ContentInfo[i].file_path);
			ctx->ContentInfo[i].content_size = Size + GetContentPaddingSize(Size,0x10);
		}
		TotalContentSize += ctx->ContentInfo[i].content_size;
	}
	ctx->cia_section[content].size = TotalContentSize;
	ctx->cia_section[content].buffer = malloc(ctx->cia_section[content].size);
	if(ctx->cia_section[content].buffer == NULL){
		printf("[!] Memory Allocation Failure\n");
		return Fail;
	}
	memset(ctx->cia_section[content].buffer,0xff,ctx->cia_section[content].size);
	
	u64 ContentOffsetStart = 0;
	for(u16 i = 0; i < ctx->ContentCount; i++){
		u64 TrueSize = 0;
		if(ctx->flags[build_mode] == rom_conv)
			TrueSize = ctx->ContentInfo[i].content_size;
		else
			TrueSize = GetFileSize_u64(ctx->ContentInfo[i].file_path);
		
		//printf("Content %d has a True size of 0x%llx and an actual Size of 0x%llx @ offset 0x%llx\n",i,TrueSize,ctx->ContentInfo[i].content_size,ctx->ContentInfo[i].file_offset);
		if(ctx->flags[verbose]) { printf(" > Content%d",i); }

		FILE *content_file = fopen(ctx->ContentInfo[i].file_path,"rb");
		if(content_file == NULL){
			printf("[!] Failed to open '%s'\n",ctx->ContentInfo[i].file_path);
			return 1;
		}
		fseek_64(content_file,ctx->ContentInfo[i].file_offset,SEEK_SET);
		fread((ctx->cia_section[content].buffer + ContentOffsetStart),TrueSize,1,content_file);
		ctr_sha_256((ctx->cia_section[content].buffer + ContentOffsetStart),ctx->ContentInfo[i].content_size,ctx->ContentInfo[i].sha_256_hash);
		if(ctx->ContentInfo[i].encrypted == True){
			if(ctx->flags[verbose]) { printf(" [Encrypted]"); }
			EncryptContent((ctx->cia_section[content].buffer + ContentOffsetStart),(ctx->cia_section[content].buffer + ContentOffsetStart),ctx->ContentInfo[i].content_size,ctx->keys.title_key,ctx->ContentInfo[i].content_index);
		}
		else
			if(ctx->flags[verbose]) { printf(" [Plaintext]"); }
		if(ctx->flags[verbose]) { printf(" [0x%llx]\n",ctx->ContentInfo[i].content_size); }
		ContentOffsetStart += ctx->ContentInfo[i].content_size;
		fclose(content_file);
	}	
	
	return 0;
}

int SetupCIAHeader(USER_CONTEXT *ctx)
{
	if(ctx->flags[verbose]) { printf("[+] Generating CIA Header\n"); }
	ctx->cia_section[header].size = sizeof(CIA_HEADER);
	ctx->cia_section[header].buffer = malloc(ctx->cia_section[header].size);
	if(ctx->cia_section[header].buffer == NULL){
		printf("[!] Memory Allocation Failure\n");
		return Fail;
	}
	
	CIA_HEADER cia_header;
	memset(&cia_header,0x0,sizeof(cia_header));
	u32_to_u8(cia_header.header_size,sizeof(cia_header),LITTLE_ENDIAN);
	u16_to_u8(cia_header.type,0x0,LITTLE_ENDIAN);
	u16_to_u8(cia_header.version,0x0,LITTLE_ENDIAN);
	u32_to_u8(cia_header.cert_size,ctx->cia_section[certchain].size,LITTLE_ENDIAN);
	u32_to_u8(cia_header.tik_size,ctx->cia_section[tik].size,LITTLE_ENDIAN);
	u32_to_u8(cia_header.tmd_size,ctx->cia_section[tmd].size,LITTLE_ENDIAN);
	u32_to_u8(cia_header.meta_size,ctx->cia_section[meta].size,LITTLE_ENDIAN);
	u64_to_u8(cia_header.content_size,ctx->cia_section[content].size,LITTLE_ENDIAN);
	
	//SetCIAContentIndex
	u64 content_index_flag = 0;
	for(int i = 0; i < ctx->ContentCount; i++){
		content_index_flag += (0x8000000000000000/(2<<ctx->ContentInfo[i].content_index))*2;
	}
	u64_to_u8(cia_header.content_index,content_index_flag,BE);
	
	memcpy(ctx->cia_section[header].buffer,&cia_header,ctx->cia_section[header].size);
	return 0;
}

int WriteSectionsToOutput(USER_CONTEXT *ctx)
{
	if(ctx->flags[verbose]) { printf("[+] Writing CIA to File\n"); }
	if(ctx->outfile.arg_len == 0){
		ctx->outfile.arg_len = 1024;
		ctx->outfile.argument = malloc(ctx->outfile.arg_len);
		if(ctx->outfile.argument == NULL){
			printf("[!] Memory Allocation Failure\n");
			return Fail;
		}
		if(append_filextention(ctx->outfile.argument,ctx->outfile.arg_len,ctx->core_infile.argument,".cia") != 0)
			return Fail;
	}
	
	FILE *output = fopen(ctx->outfile.argument,"wb");	
	if(output == NULL){
		printf("[!] IO ERROR: Failed to create '%s'\n",ctx->outfile.argument);
		return 1;
	}
	
	u64 offset = 0;
	for(int i = 0; i < 6; i++){
		if(ctx->cia_section[i].size > 0){
			if(i > 0)
				offset += align_value(ctx->cia_section[i-1].size,0x40);
			WriteBuffer(ctx->cia_section[i].buffer,ctx->cia_section[i].size,offset,output);
		}
	}
	
	fclose(output);
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

