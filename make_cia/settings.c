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
#include "srl.h"
#include "ncsd.h"
#include "tmd.h"

void InitialiseSettings(USER_CONTEXT *ctx)
{
	memset(ctx,0x0,sizeof(USER_CONTEXT));
}

int GetSettings(USER_CONTEXT *ctx, int argc, char *argv[])
{	
	if(SetBooleanSettings(ctx,argc,argv)!=0)
		return Fail;
	if(SetCryptoSettings(ctx,argc,argv)!=0)
		return Fail;
	if(GetCoreData(ctx,argc,argv)!=0)
		return Fail;
	if(SetBuildSettings(ctx,argc,argv)!=0)
		return Fail;
	if(GetContentData(ctx,argc,argv)!=0)
		return Fail;
		
	for(int i = 1; i < argc; i++){
		if(strncmp(argv[i],"--out=",6) == 0 && ctx->outfile.arg_len == 0){
			ctx->outfile.arg_len = strlen(argv[i]+5);
			ctx->outfile.argument = malloc(ctx->outfile.arg_len+1);
			if(ctx->outfile.argument == NULL){
				printf("[!] Memory Allocation Failure\n");
				return Fail;
			}
			strcpy(ctx->outfile.argument,argv[i]+5);
		}
		else if(strncmp(argv[i],"-o",2) == 0 && ctx->outfile.arg_len == 0){
			ctx->outfile.arg_len = strlen(argv[i+1]);
			ctx->outfile.argument = malloc(ctx->outfile.arg_len+1);
			if(ctx->outfile.argument == NULL){
				printf("[!] Memory Allocation Failure\n");
				return Fail;
			}
			strcpy(ctx->outfile.argument,argv[i+1]);
		}
	}
	
	if(ctx->flags[showkeys] == True){
		printf("\n[+] AES Key Data\n");
		memdump(stdout,"CommonKey:   ",ctx->keys.common_key,0x10);
		printf("CommonKeyID: %02x\n",ctx->keys.common_key_id);
		memdump(stdout,"TitleKey:    ",ctx->keys.title_key,0x10);
		memdump(stdout,"CXIKey:      ",ctx->keys.cxi_key[Secure],0x10);
		printf("[+] RSA Key Data\n");
		printf(" > Ticket:\n");
		PrintRSAKeyData(&ctx->keys.ticket);
		printf("\n > Title Meta Data:\n");
		PrintRSAKeyData(&ctx->keys.tmd);
		printf("\n > Dev NCSD/CFA:\n");
		PrintRSAKeyData(&ctx->keys.NcsdCfa);
	}
	
	if(ctx->flags[info] == True){
		printf("[+] Content Data:\n");
		memdump(stdout, "Title ID:               ", ctx->core.TitleID, 0x8);
		memdump(stdout, "Ticket ID:              ", ctx->core.TicketID, 0x8);
		printf("Title Version:          v%d\n",u8_to_u16(ctx->core.TitleVersion,BE));
		printf("Ticket Version:         v%d\n",u8_to_u16(ctx->core.TicketVersion,BE));
		printf("SaveData Size:          %d KB\n",u8_to_u32(ctx->core.save_data_size,LE)/1024);
		memdump(stdout, "TitleType:              ", ctx->core.Title_type, 0x4);
		printf("Ticket Issuer:          %s\n",ctx->core.TicketIssuer);
		printf("TMD Issuer:             %s\n",ctx->core.TMDIssuer);
	}
	return 0;
}

int GetContentData(USER_CONTEXT *ctx, int argc, char *argv[])
{
	ctx->ContentInfo = malloc(sizeof(CONTENT_INFO)*ctx->ContentCount);
	if(ctx->ContentInfo == NULL){
		printf("[!] Memory Allocation Failure\n");
		return Fail;
	}
	memset(ctx->ContentInfo,0,sizeof(CONTENT_INFO)*ctx->ContentCount);
	if(ctx->flags[build_mode] == ctr_norm || ctx->flags[build_mode] == twl_cia){
		char path[20];
		char id[20];
		char index[20];
		char encrypt_bool[20];
		char optional_bool[20];
		char shared_bool[20];
		for(u16 i = 0; i < ctx->ContentCount; i++){
			ctx->ContentInfo[i].content_index = i;
			ctx->ContentInfo[i].file_offset = 0;
			ctx->ContentInfo[i].content_type = 0;
			GetRandomContentID(ctx->ContentInfo[i].content_id,i);
			u8 bool_set[3];
			memset(&bool_set,0,3);
			if(ctx->flags[encrypt_contents] == True){
				ctx->ContentInfo[i].content_type += Encrypted;
				ctx->ContentInfo[i].encrypted = True;
				bool_set[0] = True;
			}
			//
			sprintf(path,"--content%d=",i);
			if(ctx->flags[build_mode] == twl_cia)
				sprintf(path,"--srl=");
			sprintf(id,"--id_%d=",i);
			sprintf(index,"--index_%d=",i);
			sprintf(encrypt_bool,"--crypt_%d",i);
			sprintf(optional_bool,"--optional_%d",i);
			sprintf(shared_bool,"--shared_%d",i);
			//
			
			for(int j = 0; j < argc; j++){
				if(strncmp(argv[j],path,strlen(path)) == 0){
					memcpy(ctx->ContentInfo[i].file_path,argv[j]+strlen(path),strlen(argv[j]+strlen(path)));
				}
				else if(strncmp(argv[j],id,strlen(id)) == 0){
					u32 content_id = strtol(argv[j]+strlen(id),NULL,16);
					u32_to_u8(ctx->ContentInfo[i].content_id,content_id,BE);
				}
				else if(strncmp(argv[j],index,strlen(index)) == 0){
					ctx->ContentInfo[i].content_index = strtol(argv[j]+strlen(index),NULL,10);
				}
				else if(strncmp(argv[j],encrypt_bool,strlen(encrypt_bool)) == 0 && bool_set[0] != True){
					ctx->ContentInfo[i].content_type += Encrypted;
					bool_set[0] = True;
					ctx->ContentInfo[i].encrypted = True;
				}
				else if(strncmp(argv[j],optional_bool,strlen(optional_bool)) == 0 && bool_set[1] != True){
					ctx->ContentInfo[i].content_type += Optional;
					bool_set[1] = True;
				}
				else if(strncmp(argv[j],shared_bool,strlen(shared_bool)) == 0 && bool_set[2] != True){
					ctx->ContentInfo[i].content_type += Shared;
					bool_set[2] = True;
				}
			}
			if(ctx->ContentInfo[i].file_path[0] == 0){
				printf("[!] Content %d was not specified\n",i);
				return Fail;
			}
		}
	}
	else if(ctx->flags[build_mode] == rom_conv){
		for(int i = 0; i < ctx->ContentCount; i++){
			for(int j = 0; j < 8; j++){
				if(ctx->ncsd_struct->partition_data[j].active == True){
					if(ctx->flags[encrypt_contents] == True){
						ctx->ContentInfo[i].content_type = Encrypted;
						ctx->ContentInfo[i].encrypted = True;
					}
					GetRandomContentID(ctx->ContentInfo[i].content_id,i);
					ctx->ContentInfo[i].content_index = j;
					ctx->ContentInfo[i].file_offset = ctx->ncsd_struct->partition_data[j].offset;
					ctx->ContentInfo[i].content_size = ctx->ncsd_struct->partition_data[j].size;
					memcpy(ctx->ContentInfo[i].file_path,ctx->core_infile.argument,ctx->core_infile.arg_len);
					i++;
				}
			}
		}
	}
	
	return 0;
}

int GetCoreData(USER_CONTEXT *ctx, int argc, char *argv[])
{
	ctx->ContentCount = 0;
	FILE *input = NULL;
	if(ctx->flags[build_mode] == ctr_norm){
		u8 Found = True;
		for(u16 i = 0; i < 0xffff && Found == True; i++){
			Found = False;
			for(u16 j = 1; j < argc && Found == False; j++){
				char option[20];
				sprintf(option,"--content%d=",i);
				//printf("%s\n",option);
				if(strncmp(argv[j],option,strlen(option)) == 0){
					ctx->ContentCount++;
					Found = True;
				}
			}
		}
		for(int i = 1; i < argc && ctx->core_infile.arg_len == 0; i++){
			if(strncmp(argv[i],"--content0=",11) == 0){
				ctx->core_infile.arg_len = strlen(argv[i]+11);
				ctx->core_infile.argument = malloc(ctx->core_infile.arg_len+1);
				if(ctx->core_infile.argument == NULL){
					printf("[!] Memory Allocation Failure\n");
					return Fail;
				}
				memcpy(ctx->core_infile.argument,argv[i]+11,ctx->core_infile.arg_len+1);
				input = fopen(ctx->core_infile.argument,"rb");
				if(input == NULL){
					printf("[!] Failed to open content0: %s\n",ctx->core_infile.argument);
					return Fail;
				}
			}
		}
		if(ctx->ContentCount < 1){
			printf("[!] There must at least one content\n");
			return Fail;
		}
		if(input == NULL){
			printf("[!] Content0 was not specified\n");
			return Fail;
		}
		if(GetCoreContentNCCH(ctx,&ctx->core,0x0,input) != 0){
			printf("[!] Failed to retrieve data from Content0: %s\n",ctx->core_infile.argument);
			return Fail;
		}
	}
	else if(ctx->flags[build_mode] == twl_cia){
		for(int i = 1; i < argc && ctx->core_infile.arg_len == 0; i++){
			if(strncmp(argv[i],"--srl=",6) == 0){
				ctx->core_infile.arg_len = strlen(argv[i]+6);
				ctx->core_infile.argument = malloc(ctx->core_infile.arg_len+1);
				if(ctx->core_infile.argument == NULL){
					printf("[!] Memory Allocation Failure\n");
					return Fail;
				}
				memcpy(ctx->core_infile.argument,argv[i]+6,ctx->core_infile.arg_len+1);
				input = fopen(ctx->core_infile.argument,"rb");
				if(input == NULL){
					printf("[!] Failed to open content0: %s\n",ctx->core_infile.argument);
					return Fail;
				}
			}
		}
		if(input == NULL){
			printf("[!] Content0 was not specified\n");
			return Fail;
		}
		if(GetCoreContentSRL(&ctx->core,input) != 0){
			printf("[!] Failed to retrieve data from Content0: %s\n",ctx->core_infile.argument);
			return Fail;
		}
		ctx->ContentCount = 1;
	}
	else if(ctx->flags[build_mode] == rom_conv){
		for(int i = 1; i < argc && ctx->core_infile.arg_len == 0; i++){
			if(strncmp(argv[i],"--rom=",6) == 0){
				ctx->core_infile.arg_len = strlen(argv[i]+6);
				ctx->core_infile.argument = malloc(ctx->core_infile.arg_len+1);
				if(ctx->core_infile.argument == NULL){
					printf("[!] Memory Allocation Failure\n");
					return Fail;
				}
				memcpy(ctx->core_infile.argument,argv[i]+6,ctx->core_infile.arg_len+1);
				input = fopen(ctx->core_infile.argument,"rb");
				if(input == NULL){
					printf("[!] Failed to open ROM: %s\n",ctx->core_infile.argument);
					return Fail;
				}
			}
		}
		if(input == NULL){
			printf("[!] ROM was not specified\n");
			return Fail;
		}
		ctx->ncsd_struct = malloc(sizeof(NCSD_STRUCT));
		if(ctx->ncsd_struct == NULL){
			printf("[!] Memory Allocation Failure\n");
			return Fail;
		}
		if(GetNCSDData(ctx,ctx->ncsd_struct,input) != 0)
			return 1;
		ctx->flags[gen_meta] = False;
		GetCoreContentNCCH(ctx,&ctx->core,ctx->ncsd_struct->partition_data[0].offset,input);
		ctx->ContentCount = 0;
		for(int i = 0; i < 8; i++){
			if(ctx->ncsd_struct->partition_data[i].active)
				ctx->ContentCount++;
		}
	}
	
	u32_to_u8(ctx->core.Title_type,TYPE_CTR,BE);
	memset(ctx->core.DeviceID,0x0,0x4);
	ctx->core.tmd_format_ver = 0x1;
	ctx->core.ticket_format_ver = 0x1;
	ctx->core.ca_crl_version = 0x0;
	ctx->core.signer_crl_version = 0x0;
	
	u8 hash[0x20];
	ctr_sha_256(ctx->core_infile.argument,100,hash);
	memcpy(ctx->core.TicketID,&hash,0x8);
	fclose(input);
	return 0;
}

int SetBooleanSettings(USER_CONTEXT *ctx, int argc, char *argv[])
{
	memset(ctx->flags,False,sizeof(ctx->flags));
	for(int i = 1; i < argc; i++){
		if(strncmp(argv[i],"-e",2) == 0 || strncmp(argv[i],"--encrypt",9) == 0){
			ctx->flags[encrypt_contents] = True;
		}
		else if(strncmp(argv[i],"-p",2) == 0 || strncmp(argv[i],"--info",6) == 0){
			ctx->flags[info] = True;
		}
		else if(strncmp(argv[i],"-k",2) == 0 || strncmp(argv[i],"--showkeys",10) == 0){
			ctx->flags[showkeys] = True;
		}
		else if(strncmp(argv[i],"-v",2) == 0 || strncmp(argv[i],"--verbose",9) == 0){
			ctx->flags[verbose] = True;
		}
		else if(strncmp(argv[i],"--content0=",11) == 0){
			ctx->flags[build_mode] = ctr_norm;
		}
		else if(strncmp(argv[i],"--srl=",6) == 0){
			ctx->flags[build_mode] = twl_cia;
		}
		else if(strncmp(argv[i],"--rom=",6) == 0){
			ctx->flags[build_mode] = rom_conv;
		}
	}
	if(ctx->flags[build_mode] == 0){
		printf("[!] Nothing to do\n");
		return Fail;
	}
	return 0;
}

int SetCryptoSettings(USER_CONTEXT *ctx, int argc, char *argv[])
{
	//AES Keys
	ctx->keys.common_key_id = 0;
	memcpy(ctx->keys.common_key,ctr_aes_common_key_dev0,16);
	memcpy(ctx->keys.title_key,zeros_fixed_aesKey,16);
	memcpy(ctx->keys.cxi_key[ZerosFixed],zeros_fixed_aesKey,16);
	memcpy(ctx->keys.cxi_key[SystemFixed],system_fixed_aesKey,16);
	memset(ctx->keys.cxi_key[Secure],0,16);
	
	//RSA Keys
	memcpy(ctx->keys.ticket.n,xs9_dpki_rsa_pubMod,0x100);
	memcpy(ctx->keys.ticket.d,xs9_dpki_rsa_privExp,0x100);
	memcpy(ctx->keys.tmd.n,cpA_dpki_rsa_pubMod,0x100);
	memcpy(ctx->keys.tmd.d,cpA_dpki_rsa_privExp,0x100);
	memcpy(ctx->keys.NcsdCfa.n,DevNcsdCfa_pubMod,0x100);
	memcpy(ctx->keys.NcsdCfa.d,DevNcsdCfa_privExp,0x100);
	
	//RSA Key Issuer Tags
	memcpy(ctx->core.TicketIssuer,xs9_Issuer,64);
	memcpy(ctx->core.TMDIssuer,cpA_Issuer,64);
	
	//Importing user's keys
	for(int i = 1; i < argc; i++){
		if(strncmp(argv[i],"--cxikey=",9) == 0){
			if(strlen(argv[i]+9) == 32)
				char_to_int_array(ctx->keys.cxi_key[Secure],argv[i]+9,16,BE,16);
			else
				printf("[!] Invalid Size for CXI key\n");
		}
		else if(strncmp(argv[i],"--titlekey=",11) == 0){
			if(strlen(argv[i]+11) == 32)
				char_to_int_array(ctx->keys.title_key,argv[i]+11,16,BE,16);
			else
				printf("[!] Invalid Size for title key\n");
		}
		else if(strncmp(argv[i],"--ckey=",7) == 0){
			if(strlen(argv[i]+7) == 32)
				char_to_int_array(ctx->keys.common_key,argv[i]+7,16,BE,16);
			else
				printf("[!] Invalid Size for common key\n");
		}
		else if(strncmp(argv[i],"--ckeyID=",9) == 0){
			ctx->keys.common_key_id = strtol(argv[i]+9,NULL,10);
		}
		else if(strncmp(argv[i],"--tmdkey=",9) == 0){
			FILE *tmd_key = fopen(argv[i]+9,"rb");
			if(tmd_key != NULL){
				if(LoadRSAKeyFile(&ctx->keys.tmd,tmd_key) == 0)
					SetTitleMetaDataIssuer(ctx);
				else
					printf("[!] TMD Key file Error\n");
				fclose(tmd_key);
			}
			else
				printf("[!] Could not open, %s\n",argv[i]+9);
			
		}
		else if(strncmp(argv[i],"--tikkey=",9) == 0){
			FILE *tik_key = fopen(argv[i]+9,"rb");
			if(tik_key != NULL){
				if(LoadRSAKeyFile(&ctx->keys.ticket,tik_key) == 0)
					SetTicketIssuer(ctx);
				else
					printf("[!] TIK Key file Error\n");
				fclose(tik_key);
			}
			else
				printf("[!] Could not open, %s\n",argv[i]+9);
			
		}
		else if(strncmp(argv[i],"--romkey=",9) == 0){
			FILE *rom_key = fopen(argv[i]+9,"rb");
			if(rom_key != NULL){
				if(LoadRSAKeyFile(&ctx->keys.NcsdCfa,rom_key) != 0){
					printf("[!] NCSD Key file Error\n");
				}
				fclose(rom_key);					
			}
			else
				printf("[!] Could not open, %s\n",argv[i]+9);
			
		}
		else if(strncmp(argv[i],"--certs=",8) == 0){
			FILE *certs = fopen(argv[i]+8,"rb");
			if(certs != NULL){
				ctx->cia_section[certchain].size = GetFileSize_u32(certs);
				ctx->cia_section[certchain].buffer = malloc(ctx->cia_section[certchain].size);
				if(ctx->cia_section[certchain].buffer == NULL){
					printf("[!] Memory Allocation Failure\n");
					return Fail;
				}
				fread(ctx->cia_section[certchain].buffer,ctx->cia_section[certchain].size,1,certs);
				fclose(certs);
			}
			else
				printf("[!] Could not open, %s\n",argv[i]+8);
		}
		else if(strncmp(argv[i],"--rand",6) == 0){
			u8 *buff = malloc(16);
			if(buff == NULL){
				printf("[!] Memory Allocation Failure\n");
				return Fail;
			}
			u8 hash[0x20];
			ctr_sha_256(buff,16,hash);
			memcpy(ctx->keys.title_key,hash,16);
			_free(buff);
		}
		else if(strncmp(argv[i],"--forcecxikey",6) == 0){
			memcpy(ctx->keys.cxi_key[ZerosFixed],ctx->keys.cxi_key[Secure],16);
			memcpy(ctx->keys.cxi_key[SystemFixed],ctx->keys.cxi_key[Secure],16);
		}
	}
	if(ctx->cia_section[certchain].size == 0){
		ctx->cia_section[certchain].size = 0xA00;
		ctx->cia_section[certchain].buffer = malloc(ctx->cia_section[certchain].size);
		if(ctx->cia_section[certchain].buffer == NULL){
			printf("[!] Memory Allocation Failure\n");
			return Fail;
		}
		memcpy(ctx->cia_section[certchain].buffer,CIA_DEV_Certificate_Chain,ctx->cia_section[certchain].size);
	}
	
	return 0;
}

int SetBuildSettings(USER_CONTEXT *ctx, int argc, char *argv[])
{
	u8 vermod[2];
	memset(&vermod,False,2);
	u8 tmdverdata[3];
	u8 tikverdata[3];
	memset(&tmdverdata,0,3);
	memset(&tikverdata,0,3);
	for(int i = 1; i < argc; i++){
		if(strncmp(argv[i],"--tikID=",8) == 0){
			if(strlen(argv[i]+8) == 16)
				char_to_int_array(ctx->core.TicketID,argv[i]+8,0x8,BE,16);
			else
				printf("[!] Invalid length for Ticket ID\n");
		}
		else if(strncmp(argv[i],"--savesize=",11) == 0){
			u32 savesize = strtol(argv[i]+11, NULL, 10)*1024;
			u32_to_u8(ctx->core.save_data_size,savesize,LE);
		}
		else if(strncmp(argv[i],"--titleID=",10) == 0){
			if(strlen(argv[i]+10) == 16)
				char_to_int_array(ctx->core.TitleID,argv[i]+10,0x8,BE,16);
			else
				printf("[!] Invalid length for Ticket ID\n");
		}
		else if(strncmp(argv[i],"--major=",8) == 0){
			tmdverdata[0] = strtol(argv[i]+8, NULL, 10);
			vermod[0] = True;
		}
		else if(strncmp(argv[i],"--minor=",8) == 0){
			tmdverdata[1] = strtol(argv[i]+8, NULL, 10);
			vermod[0] = True;
		}
		else if(strncmp(argv[i],"--micro=",8) == 0){
			tmdverdata[2] = strtol(argv[i]+8, NULL, 10);
			vermod[0] = True;
		}
		else if(strncmp(argv[i],"--tikmajor=",11) == 0){
			tikverdata[0] = strtol(argv[i]+11, NULL, 10);
			vermod[1] = True;
		}
		else if(strncmp(argv[i],"--tikminor=",11) == 0){
			tikverdata[1] = strtol(argv[i]+11, NULL, 10);
			vermod[1] = True;
		}
		else if(strncmp(argv[i],"--tikmicro=",11) == 0){
			tikverdata[2] = strtol(argv[i]+11, NULL, 10);
			vermod[1] = True;
		}
		else if(strncmp(argv[i],"-1",2) == 0){
			if(strlen(argv[i+1]) == 16)
				char_to_int_array(ctx->core.TicketID,argv[i+1],0x8,BE,16);
			else
				printf("[!] Invalid length for Ticket ID\n");
		}
		else if(strncmp(argv[i],"-0",2) == 0){
			u32 savesize = strtol(argv[i+1], NULL, 10)*1024;
			u32_to_u8(ctx->core.save_data_size,savesize,LE);
		}
		else if(strncmp(argv[i],"-2",2) == 0){
			if(strlen(argv[i+1]) == 16)
				char_to_int_array(ctx->core.TitleID,argv[i+1],0x8,BE,16);
			else
				printf("[!] Invalid length for Ticket ID\n");
		}
		else if(strncmp(argv[i],"-3",2) == 0){
			tmdverdata[0] = strtol(argv[i+1], NULL, 10);
			vermod[0] = True;
		}
		else if(strncmp(argv[i],"-4",2) == 0){
			tmdverdata[1] = strtol(argv[i+1], NULL, 10);
			vermod[0] = True;
		}
		else if(strncmp(argv[i],"-5",2) == 0){
			tmdverdata[2] = strtol(argv[i+1], NULL, 10);
			vermod[0] = True;
		}
		else if(strncmp(argv[i],"-6",2) == 0){
			tikverdata[0] = strtol(argv[i+1], NULL, 10);
			vermod[1] = True;
		}
		else if(strncmp(argv[i],"-7",2) == 0){
			tikverdata[1] = strtol(argv[i+1], NULL, 10);
			vermod[1] = True;
		}
		else if(strncmp(argv[i],"-8",2) == 0){
			tikverdata[2] = strtol(argv[i+1], NULL, 10);
			vermod[1] = True;
		}
	}
	if(vermod[0] == True)
		u16_to_u8(ctx->core.TitleVersion,(tmdverdata[0]*1024 + tmdverdata[1]*16 + tmdverdata[2]),BE);
	if(vermod[1] == True)
		u16_to_u8(ctx->core.TicketVersion,(tikverdata[0]*1024 + tikverdata[1]*16 + tikverdata[2]),BE);
	
	return 0;
}

int LoadRSAKeyFile(RSA_2048_KEY *ctx, FILE *file)
{
	CRKF_HEADER header;
	memset(&header,0x0,sizeof(CRKF_HEADER));
	
	fseek(file,0x0,SEEK_SET);
	fread(&header,sizeof(CRKF_HEADER),1,file);

	if(u8_to_u32(header.magic,BE) != 0x43524B46){
		printf("[!] Key File is corrupt\n");
		return rsa_key_fail;
	}
	
	if(u8_to_u16(header.rsatype,BE) != 0x1){
		printf("[!] Only RSA-2048 is supported\n");
		return rsa_key_fail;
	}
	
	ctx->keytype = RSAKEY_PRIV;
	
	ReadFile_64(ctx->n,u8_to_u32(header.n_size,BE),u8_to_u32(header.n_offset,BE),file);
	ReadFile_64(ctx->e,u8_to_u32(header.e_size,BE),u8_to_u32(header.e_offset,BE),file);
	ReadFile_64(ctx->d,u8_to_u32(header.d_size,BE),u8_to_u32(header.d_offset,BE),file);
	ReadFile_64(ctx->name,u8_to_u32(header.name_size,BE),u8_to_u32(header.name_offset,BE),file);
	ReadFile_64(ctx->issuer,u8_to_u32(header.issuer_size,BE),u8_to_u32(header.issuer_offset,BE),file);
	
	return 0;
}

void PrintRSAKeyData(RSA_2048_KEY *ctx)
{
	printf("Key Name:    %s\n", ctx->name);
	printf("Key Issuer:  %s\n", ctx->issuer);
	printf("\n");
	memdump(stdout, "n:           ", ctx->n, 0x100);
	printf("\n");
	//memdump(stdout, "e:           ", ctx->e, 0x3);
	//printf("\n");
	memdump(stdout, "d:           ", ctx->d, 0x100);
}

int SetTicketIssuer(USER_CONTEXT *ctx)
{
	memset(ctx->core.TicketIssuer,0x0,0x40);
	u8 old_issuer_len = strlen(ctx->keys.ticket.issuer);
	u8 name_len = strlen(ctx->keys.ticket.name);
	ctx->core.TicketIssuer[old_issuer_len] = 0x2D;
	memcpy(ctx->core.TicketIssuer,ctx->keys.ticket.issuer,old_issuer_len);
	memcpy((ctx->core.TicketIssuer+old_issuer_len+1),ctx->keys.ticket.name,name_len);
	return 0;
}

int SetTitleMetaDataIssuer(USER_CONTEXT *ctx)
{
	memset(ctx->core.TMDIssuer,0x0,0x40);
	u8 old_issuer_len = strlen(ctx->keys.tmd.issuer);
	u8 name_len = strlen(ctx->keys.tmd.name);
	ctx->core.TMDIssuer[old_issuer_len] = 0x2D;
	memcpy(ctx->core.TMDIssuer,ctx->keys.tmd.issuer,old_issuer_len);
	memcpy((ctx->core.TMDIssuer+old_issuer_len+1),ctx->keys.tmd.name,name_len);
	return 0;
}

void GetRandomContentID(u8 *contentID, u16 value)
{
	u16 *rand = malloc(8*sizeof(u16));
	rand[0] = value*4;
	u8 hash[0x20];
	ctr_sha_256(rand,8*sizeof(u16),hash);
	memcpy(contentID,&hash,0x4);
}
