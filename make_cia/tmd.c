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
#include "tmd.h"

int GenerateTitleMetaData(USER_CONTEXT *ctx)
{
	if(ctx->flags[verbose]) { printf("[+] Generating Title Metadata\n"); }
	ctx->cia_section[tmd].size = (0xB04+(0x30*ctx->ContentCount));
	ctx->cia_section[tmd].buffer = malloc(ctx->cia_section[tmd].size);
	if(ctx->cia_section[tmd].buffer == NULL){
		printf("[!] Memory Allocation Failure\n");
		return Fail;
	}

	TMD_STRUCT header;
	TMD_2048_SIG_CONTEXT sig;
	memset(&header,0x0,sizeof(TMD_STRUCT));
	memset(&sig,0x0,sizeof(TMD_2048_SIG_CONTEXT));
	
	if(ctx->flags[verbose]) { printf(" > Collecting Data\n"); }	
	SetTMDHeader(&header,ctx);
	
	if(ctx->flags[verbose]) { printf(" > Building Content Info Record\n"); }
	TMD_CONTENT_INFO_RECORD *info_record = malloc(sizeof(TMD_CONTENT_INFO_RECORD)*0x40);
	if(info_record == NULL){
		printf("[!] Memory Allocation Failure\n");
		return Fail;
	}
	memset(info_record,0x0,sizeof(TMD_CONTENT_INFO_RECORD)*0x40);
	u16_to_u8(info_record->content_index_offset,0x0,BE);
	u16_to_u8(info_record->content_command_count,ctx->ContentCount,BE);
	
	if(ctx->flags[verbose]) { printf(" > Building Content Chunk Record\n"); }
	TMD_CONTENT_CHUNK_STRUCT *info_chunk = malloc(sizeof(TMD_CONTENT_CHUNK_STRUCT)*ctx->ContentCount);
	if(info_chunk == NULL){
		printf("[!] Memory Allocation Failure\n");
		return Fail;
	}
	
	for(int i = 0; i < ctx->ContentCount; i++){
		SetInfoChunk(&info_chunk[i],&ctx->ContentInfo[i]);
	}
	
	if(ctx->flags[verbose]) { printf(" > Collecting Hashes\n"); }
	u32_to_u8(sig.sig_type,RSA_2048_SHA256,BE);
	u8 hash[0x20];
	memset(&hash,0x0,0x20);
	ctr_sha_256(info_chunk,(sizeof(TMD_CONTENT_CHUNK_STRUCT)*ctx->ContentCount),info_record->sha_256_hash);
	//memdump(stdout,"Info Chunk Hash:       ",info_record->sha_256_hash,0x20);
	ctr_sha_256(info_record,(sizeof(TMD_CONTENT_INFO_RECORD)*0x40),header.sha_256_hash);
	//memdump(stdout,"Info Record Hash:       ",header.sha_256_hash,0x20);
	ctr_sha_256(&header,sizeof(TMD_STRUCT),hash);
	//memdump(stdout,"Header Hash:       ",hash,0x20);

	if(ctx->flags[verbose]) { printf(" > Signing TMD\n"); }
	if(ctr_rsa2048_sha256_sign(hash,sig.data,ctx->keys.tmd.n,ctx->keys.tmd.d) != Good){
		printf("[!] Failed to sign tmd\n");
		_free(info_record);
		_free(info_chunk);
		return ticket_gen_fail;
	}

	if(ctx->flags[info]){
		memdump(stdout,"[+] TMD Signature:      ",sig.data,0x100);
	}
	
	memcpy(ctx->cia_section[tmd].buffer,&sig,sizeof(TMD_2048_SIG_CONTEXT));
	memcpy((ctx->cia_section[tmd].buffer + sizeof(TMD_2048_SIG_CONTEXT)),&header,sizeof(TMD_STRUCT));
	memcpy((ctx->cia_section[tmd].buffer + sizeof(TMD_2048_SIG_CONTEXT) + sizeof(TMD_STRUCT)),info_record,(sizeof(TMD_CONTENT_INFO_RECORD)*0x40));
	memcpy((ctx->cia_section[tmd].buffer + sizeof(TMD_2048_SIG_CONTEXT) + sizeof(TMD_STRUCT) + (sizeof(TMD_CONTENT_INFO_RECORD)*0x40)),info_chunk,(sizeof(TMD_CONTENT_CHUNK_STRUCT)*ctx->ContentCount));
	
	_free(info_record);
	_free(info_chunk);
	
	return 0;
}

void SetInfoChunk(TMD_CONTENT_CHUNK_STRUCT *info_chunk,CONTENT_INFO *ContentInfo)
{
	memcpy(info_chunk->content_id,ContentInfo->content_id,0x4);
	u16_to_u8(info_chunk->content_index,ContentInfo->content_index,BE);
	u16_to_u8(info_chunk->content_type,ContentInfo->content_type,BE);
	u64_to_u8(info_chunk->content_size,ContentInfo->content_size,BE);
	memcpy(info_chunk->sha_256_hash,ContentInfo->sha_256_hash,0x20);
}

void SetTMDHeader(TMD_STRUCT *header,USER_CONTEXT *ctx)
{
	memcpy(header->issuer,ctx->core.TMDIssuer,0x40);
	header->version = ctx->core.tmd_format_ver;
	header->ca_crl_version = ctx->core.ca_crl_version;
	header->signer_crl_version = ctx->core.signer_crl_version;
	//System Version...
	memcpy(header->title_id,ctx->core.TitleID,0x8);
	memcpy(header->title_type,ctx->core.Title_type,0x4);
	//Access Rights...
	memcpy(header->save_data_size,ctx->core.save_data_size,0x4);
	memcpy(header->priv_save_data_size,ctx->core.priv_save_data_size,0x4);
	memcpy(header->twl_data,ctx->core.twl_data,0x4);
	memcpy(header->title_version,ctx->core.TitleVersion,0x2);
	u16_to_u8(header->content_count,ctx->ContentCount,BE);
}
