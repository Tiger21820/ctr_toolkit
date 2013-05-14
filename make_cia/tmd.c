#include "lib.h"
#include "ctr_crypto.h"
#include "tmd.h"

int GenerateTitleMetaData(CIA_CONTEXT *ctx)
{
	ctx->tmd.used = TRUE;
	ctx->tmd.size = (0xB04+(0x30*ctx->ContentCount));
	ctx->tmd.buffer = malloc(ctx->tmd.size);

	TMD_STRUCT header;
	TMD_2048_SIG_CONTEXT sig;
	memset(&header,0x0,sizeof(TMD_STRUCT));
	memset(&sig,0x0,sizeof(TMD_2048_SIG_CONTEXT));
	SetTMDHeader(&header,ctx);
	
	TMD_CONTENT_INFO_RECORD *info_record = malloc(sizeof(TMD_CONTENT_INFO_RECORD)*0x40);
	memset(info_record,0x0,sizeof(TMD_CONTENT_INFO_RECORD)*0x40);
	u16_to_u8(info_record->content_index_offset,0x0,BIG_ENDIAN);
	u16_to_u8(info_record->content_command_count,ctx->ContentCount,BIG_ENDIAN);
	
	TMD_CONTENT_CHUNK_STRUCT *info_chunk = malloc(sizeof(TMD_CONTENT_CHUNK_STRUCT)*ctx->ContentCount);
	
	for(int i = 0; i < ctx->ContentCount; i++){
		SetInfoChunk(&info_chunk[i],&ctx->ContentInfo[i]);
	}
	
	u32_to_u8(sig.sig_type,0x00010004,BIG_ENDIAN);
	u8 hash[0x20];
	memset(&hash,0x0,0x20);
	ctr_sha_256(info_chunk,(sizeof(TMD_CONTENT_CHUNK_STRUCT)*ctx->ContentCount),info_record->sha_256_hash);
	//memdump(stdout,"Info Chunk Hash:       ",info_record->sha_256_hash,0x20);
	ctr_sha_256(info_record,(sizeof(TMD_CONTENT_INFO_RECORD)*0x40),header.sha_256_hash);
	//memdump(stdout,"Info Record Hash:       ",header.sha_256_hash,0x20);
	ctr_sha_256(&header,sizeof(TMD_STRUCT),hash);
	//memdump(stdout,"Header Hash:       ",hash,0x20);
	if(ctr_rsa_sign_hash(hash,sig.data, &ctx->keys.tmd) != Good){
		printf("[!] Failed to sign ticket\n");
		free(info_record);
		free(info_chunk);
		return ticket_gen_fail;
	}

	if(ctx->verbose_flag){
		memdump(stdout,"[+] TMD Signature:      ",sig.data,0x100);
	}
	
	memcpy(ctx->tmd.buffer,&sig,sizeof(TMD_2048_SIG_CONTEXT));
	memcpy((ctx->tmd.buffer + sizeof(TMD_2048_SIG_CONTEXT)),&header,sizeof(TMD_STRUCT));
	memcpy((ctx->tmd.buffer + sizeof(TMD_2048_SIG_CONTEXT) + sizeof(TMD_STRUCT)),info_record,(sizeof(TMD_CONTENT_INFO_RECORD)*0x40));
	memcpy((ctx->tmd.buffer + sizeof(TMD_2048_SIG_CONTEXT) + sizeof(TMD_STRUCT) + (sizeof(TMD_CONTENT_INFO_RECORD)*0x40)),info_chunk,(sizeof(TMD_CONTENT_CHUNK_STRUCT)*ctx->ContentCount));
	
	free(info_record);
	free(info_chunk);
	
	return 0;
}

void SetInfoChunk(TMD_CONTENT_CHUNK_STRUCT *info_chunk,CONTENT_INFO *ContentInfo)
{
	memcpy(info_chunk->content_id,ContentInfo->content_id,0x4);
	u16_to_u8(info_chunk->content_index,ContentInfo->content_index,BIG_ENDIAN);
	u16_to_u8(info_chunk->content_type,ContentInfo->content_type,BIG_ENDIAN);
	u64_to_u8(info_chunk->content_size,ContentInfo->content_size,BIG_ENDIAN);
	memcpy(info_chunk->sha_256_hash,ContentInfo->sha_256_hash,0x20);
}

void SetTMDHeader(TMD_STRUCT *header,CIA_CONTEXT *ctx)
{
	memcpy(header->issuer,ctx->core.TMDIssuer,0x40);
	header->version = ctx->core.tmd_format_ver;
	header->ca_crl_version = ctx->core.ca_crl_version;
	header->signer_crl_version = ctx->core.signer_crl_version;
	//System Version...
	memcpy(header->title_id,ctx->core.TitleID,0x8);
	memcpy(header->title_type,ctx->core.Title_type,0x4);
	//Access Rights...
	memcpy(header->title_version,ctx->core.TitleVersion,0x2);
	u16_to_u8(header->content_count,ctx->ContentCount,BIG_ENDIAN);
}
