#include "lib.h"
#include "ctr_crypto.h"
#include "yaml.h"
#include "settings.h"
#include "srl.h"
#include "ncsd.h"

void InitialiseSettings(CIA_CONTEXT *ctx)
{
	memset(ctx,0x0,sizeof(CIA_CONTEXT));
}

int GetSettings(CIA_CONTEXT *ctx, int argc, char *argv[])
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
		if(strncmp(argv[i],"-out=",5) == 0){
			ctx->outfile.used = True;
			ctx->outfile.arg_len = strlen(argv[i]+5);
			ctx->outfile.argument = malloc(ctx->outfile.arg_len+1);
			strcpy(ctx->outfile.argument,argv[i]+5);
		}
	}
	
	if(ctx->showkeys_flag == True){
		printf("\n[+] AES Key Data\n");
		memdump(stdout,"CommonKey:   ",ctx->keys.common_key.key,0x10);
		printf("CommonKeyID: %02x\n",ctx->keys.common_key_id);
		memdump(stdout,"TitleKey:    ",ctx->keys.title_key.key,0x10);
		memdump(stdout,"CXIKey:      ",ctx->keys.ncch_key.key,0x10);
		printf("[+] RSA Key Data\n");
		printf(" > Ticket:\n");
		PrintRSAKeyData(&ctx->keys.ticket);
		printf("\n > Title Meta Data:\n");
		PrintRSAKeyData(&ctx->keys.tmd);
		printf("\n > Dev NCSD/CFA:\n");
		PrintRSAKeyData(&ctx->keys.NcsdCfa);
	}
	
	if(ctx->verbose_flag == True){
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
	
	fclose(ctx->core_infile.file.file);
	
	return 0;
}

int GetContentData(CIA_CONTEXT *ctx, int argc, char *argv[])
{
	ctx->ContentInfoMallocFlag = True;
	ctx->ContentInfo = malloc(sizeof(CONTENT_INFO)*ctx->ContentCount);
	memset(ctx->ContentInfo,0,sizeof(CONTENT_INFO)*ctx->ContentCount);
	if(ctx->build_mode == ctr_norm){
		char path[20];
		char id[20];
		char index[20];
		char encrypt_bool[20];
		char optional_bool[20];
		char shared_bool[20];
		for(u16 i = 0; i < ctx->ContentCount; i++){
			u32_to_u8(ctx->ContentInfo[i].content_id,i,BE);
			ctx->ContentInfo[i].content_index = i;
			ctx->ContentInfo[i].file_offset = 0;
			ctx->ContentInfo[i].content_type = 0;
			u8 bool_set[3];
			memset(&bool_set,0,3);
			if(ctx->encrypt_contents == True){
				ctx->ContentInfo[i].content_type += Encrypted;
				ctx->ContentInfo[i].encrypted = True;
				bool_set[0] = True;
			}
			//
			sprintf(path,"-content%d=",i);
			sprintf(id,"-id_%d=",i);
			sprintf(index,"-index_%d=",i);
			sprintf(encrypt_bool,"-encrypt_%d",i);
			sprintf(optional_bool,"-optional_%d",i);
			sprintf(shared_bool,"-shared_%d",i);
			//
			
			for(int j = 0; j < argc; j++){
				if(strncmp(argv[j],path,strlen(path)) == 0){
					memcpy(ctx->ContentInfo[i].file_path,argv[j]+strlen(path),strlen(argv[j]+strlen(path)));
				}
				else if(strncmp(argv[j],id,strlen(id)) == 0){
					u32 content_id = strtol(argv[j]+6,NULL,16);
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
	else if(ctx->build_mode == twl_cia){
		u32_to_u8(ctx->ContentInfo[0].content_id,0,BE);
		ctx->ContentInfo[0].content_index = 0;
		ctx->ContentInfo[0].file_offset = 0;
		memcpy(ctx->ContentInfo[0].file_path,ctx->core_infile.argument,ctx->core_infile.arg_len);
		u8 bool_set[3];
		memset(&bool_set,0,3);
		if(ctx->encrypt_contents == True){
			ctx->ContentInfo[0].content_type += Encrypted;
			bool_set[0] = True;
			ctx->ContentInfo[0].encrypted = True;
		}
		
		for(int i = 0; i < argc; i++){
			if(strncmp(argv[i],"-id_0=",6) == 0){
				u32 content_id = strtol(argv[i]+6,NULL,16);
				u32_to_u8(ctx->ContentInfo[0].content_id,content_id,BE);
			}
			else if(strncmp(argv[i],"-index_0=",9) == 0){
				ctx->ContentInfo[0].content_index = strtol(argv[i]+9,NULL,10);
			}
			else if(strncmp(argv[i],"-encrypt_0",10) == 0 && bool_set[0] != True){
				ctx->ContentInfo[0].content_type += Encrypted;
				bool_set[0] = True;
				ctx->ContentInfo[0].encrypted = True;
			}
			else if(strncmp(argv[i],"-optional_0",11) == 0 && bool_set[1] != True){
				ctx->ContentInfo[0].content_type += Optional;
				bool_set[1] = True;
			}
			else if(strncmp(argv[i],"-shared_0",9) == 0 && bool_set[2] != True){
				ctx->ContentInfo[0].content_type += Shared;
				bool_set[2] = True;
			}
		}
		
	}
	else if(ctx->build_mode == rom_conv){
		for(int i = 0; i < ctx->ContentCount; i++){
			for(int j = 0; j < 8; j++){
				if(ctx->ncsd_struct->partition_data[j].active == True){
					if(ctx->encrypt_contents == True){
						ctx->ContentInfo[i].content_type = Encrypted;
						ctx->ContentInfo[i].encrypted = True;
					}
					u32_to_u8(ctx->ContentInfo[i].content_id,i,BE);
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

int GetCoreData(CIA_CONTEXT *ctx, int argc, char *argv[])
{
	ctx->ContentCount = 0;
	if(ctx->build_mode == ctr_norm){
		for(u16 i = 0; i < 0xffff; i++){
			u8 Found = False;
			for(u16 j = 1; j < argc; j++){
				char option[20];
				sprintf(option,"-content%d=",i);
				//printf("%s\n",option);
				if(strncmp(argv[j],option,strlen(option)) == 0){
					ctx->ContentCount++;
					Found = True;
				}
			}
			if(Found != True)
					break;
		}
		for(int i = 1; i < argc; i++){
			if(strncmp(argv[i],"-content0=",10) == 0){
				ctx->core_infile.used = True;
				ctx->core_infile.file.used = True;
				ctx->core_infile.arg_len = strlen(argv[i]+10);
				ctx->core_infile.argument = malloc(ctx->core_infile.arg_len+1);
				memcpy(ctx->core_infile.argument,argv[i]+10,ctx->core_infile.arg_len+1);
				ctx->core_infile.file.file = fopen(ctx->core_infile.argument,"rb");
				if(ctx->core_infile.file.file == NULL){
					printf("[!] Failed to open content0: %s\n",argv[i]+10);
					return Fail;
				}
			}
		}
		if(ctx->ContentCount < 1){
			printf("[!] There must at least one content\n");
			return Fail;
		}
		if(ctx->core_infile.file.file == NULL){
			printf("[!] Content0 was not specified\n");
			return Fail;
		}
		if(GetCoreContentNCCH(ctx,&ctx->core,0x0,ctx->core_infile.file.file) != 0){
			printf("[!] Failed to retrieve data from Content0: %s\n",ctx->core_infile.argument);
			return Fail;
		}
	}
	else if(ctx->build_mode == twl_cia){
		for(int i = 1; i < argc; i++){
			if(strncmp(argv[i],"-srl=",5) == 0){
				ctx->core_infile.used = True;
				ctx->core_infile.file.used = True;
				ctx->core_infile.arg_len = strlen(argv[i]+5);
				ctx->core_infile.argument = malloc(ctx->core_infile.arg_len+1);
				memcpy(ctx->core_infile.argument,argv[i]+5,ctx->core_infile.arg_len+1);
				ctx->core_infile.file.file = fopen(ctx->core_infile.argument,"rb");
				if(ctx->core_infile.file.file == NULL){
					printf("[!] Failed to open content0: %s\n",ctx->core_infile.argument);
					return Fail;
				}
			}
		}
		if(ctx->core_infile.file.file == NULL){
			printf("[!] Content0 was not specified\n");
			return Fail;
		}
		if(GetCoreContentSRL(&ctx->core,ctx->core_infile.file.file) != 0){
			printf("[!] Failed to retrieve data from Content0: %s\n",ctx->core_infile.argument);
			return Fail;
		}
		ctx->ContentCount = 1;
	}
	else if(ctx->build_mode == rom_conv){
		for(int i = 1; i < argc; i++){
			if(strncmp(argv[i],"-rom=",5) == 0){
				ctx->core_infile.used = True;
				ctx->core_infile.file.used = True;
				ctx->core_infile.arg_len = strlen(argv[i]+5);
				ctx->core_infile.argument = malloc(ctx->core_infile.arg_len+1);
				memcpy(ctx->core_infile.argument,argv[i]+5,ctx->core_infile.arg_len+1);
				ctx->core_infile.file.file = fopen(ctx->core_infile.argument,"rb");
				if(ctx->core_infile.file.file == NULL){
					printf("[!] Failed to open ROM: %s\n",ctx->core_infile.argument);
					return Fail;
				}
			}
		}
		if(ctx->core_infile.file.file == NULL){
			printf("[!] ROM was not specified\n");
			return Fail;
		}
		ctx->ncsd_struct_malloc_flag = True;
		ctx->ncsd_struct = malloc(sizeof(NCSD_STRUCT));
		if(GetNCSDData(ctx,ctx->ncsd_struct,ctx->core_infile.file.file) != 0)
			return 1;
		ctx->meta_flag = False;
		GetCoreContentNCCH(ctx,&ctx->core,ctx->ncsd_struct->partition_data[0].offset,ctx->core_infile.file.file);
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
	return 0;
}

int SetBooleanSettings(CIA_CONTEXT *ctx, int argc, char *argv[])
{
	/*
	ctx->encrypt_contents = False;
	ctx->rand_titlekey = False;
	ctx->showkeys_flag = False;
	ctx->verbose_flag = False;
	ctx->build_mode = False;
	*/
	for(int i = 1; i < argc; i++){
		if(strncmp(argv[i],"-e",2) == 0 || strncmp(argv[i],"--encrypt",9) == 0){
			ctx->encrypt_contents = True;
		}
		else if(strncmp(argv[i],"-rand",5) == 0){
			ctx->rand_titlekey = True;
		}
		else if(strncmp(argv[i],"-k",2) == 0 || strncmp(argv[i],"--showkeys",10) == 0){
			ctx->showkeys_flag = True;
		}
		else if(strncmp(argv[i],"-v",2) == 0 || strncmp(argv[i],"--verbose",9) == 0){
			ctx->verbose_flag = True;
		}
		else if(strncmp(argv[i],"-content0=",10) == 0){
			ctx->build_mode = ctr_norm;
		}
		else if(strncmp(argv[i],"-srl=",5) == 0){
			ctx->build_mode = twl_cia;
		}
		else if(strncmp(argv[i],"-rom=",5) == 0){
			ctx->build_mode = rom_conv;
		}
	}
	if(ctx->build_mode == 0){
		printf("[!] Nothing to do\n");
		return Fail;
	}
	return 0;
}

int SetCryptoSettings(CIA_CONTEXT *ctx, int argc, char *argv[])
{
	//AES Keys
	memcpy(ctx->keys.common_key.key,ctr_aes_common_key_dev0,16);
	memcpy(ctx->keys.title_key.key,zeros_fixed_aesKey,16);
	memcpy(ctx->keys.ncch_key.key,zeros_fixed_aesKey,16);
	ctx->keys.common_key_id = 0;
	
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
		if(strncmp(argv[i],"-cxikey=",8) == 0){
			if(strlen(argv[i]+6) == 32)
				char_to_int_array(ctx->keys.ncch_key.key,argv[i]+6,16,BE,16);
			else
				printf("[!] Invalid Size for CXI key\n");
		}
		else if(strncmp(argv[i],"-titlekey=",10) == 0){
			if(strlen(argv[i]+6) == 32)
				char_to_int_array(ctx->keys.title_key.key,argv[i]+6,16,BE,16);
			else
				printf("[!] Invalid Size for title key\n");
		}
		else if(strncmp(argv[i],"-ckey=",6) == 0){
			if(strlen(argv[i]+6) == 32)
				char_to_int_array(ctx->keys.common_key.key,argv[i]+6,16,BE,16);
			else
				printf("[!] Invalid Size for common key\n");
		}
		else if(strncmp(argv[i],"-ckeyID=",8) == 0){
			ctx->keys.common_key_id = strtol(argv[i]+8,NULL,10);
		}
		else if(strncmp(argv[i],"-tmdkey=",8) == 0){
			FILE *tmd_key = fopen(argv[i]+8,"rb");
			if(tmd_key != NULL){
				if(LoadRSAKeyFile(&ctx->keys.tmd,tmd_key) == 0)
					SetTitleMetaDataIssuer(ctx);
				else
					printf("[!] TMD Key file Error\n");
				fclose(tmd_key);
			}
			else
				printf("[!] Could not open, %s\n",argv[i]+8);
			
		}
		else if(strncmp(argv[i],"-tikkey=",8) == 0){
			FILE *tik_key = fopen(argv[i]+8,"rb");
			if(tik_key != NULL){
				if(LoadRSAKeyFile(&ctx->keys.ticket,tik_key) == 0)
					SetTicketIssuer(ctx);
				else
					printf("[!] TIK Key file Error\n");
				fclose(tik_key);
			}
			else
				printf("[!] Could not open, %s\n",argv[i]+8);
			
		}
		else if(strncmp(argv[i],"-romkey=",8) == 0){
			FILE *rom_key = fopen(argv[i]+8,"rb");
			if(rom_key != NULL){
				if(LoadRSAKeyFile(&ctx->keys.NcsdCfa,rom_key) != 0){
					printf("[!] NCSD Key file Error\n");
				}
				fclose(rom_key);					
			}
			else
				printf("[!] Could not open, %s\n",argv[i]+8);
			
		}
		else if(strncmp(argv[i],"-certs=",7) == 0){
			FILE *certs = fopen(argv[i]+7,"rb");
			if(certs != NULL){
				ctx->certchain.used = True;
				ctx->certchain.size = GetFileSize_u32(certs);
				ctx->certchain.buffer = malloc(ctx->certchain.size);
				fread(ctx->certchain.buffer,ctx->certchain.size,1,certs);
				fclose(certs);
			}
			else
				printf("[!] Could not open, %s\n",argv[i]+6);
		}
	}
	if(ctx->certchain.used != True){
		ctx->certchain.used = True;
		ctx->certchain.size = 0xA00;
		ctx->certchain.buffer = malloc(ctx->certchain.size);
		memcpy(ctx->certchain.buffer,CIA_Certificate_Chain,ctx->certchain.size);
	}
	if(ctx->rand_titlekey == True){
		u8 *buff = malloc(16);
		u8 hash[0x20];
		ctr_sha_256(buff,16,hash);
		memcpy(ctx->keys.title_key.key,hash,16);
		free(buff);
	}
	
	return 0;
}

int SetBuildSettings(CIA_CONTEXT *ctx, int argc, char *argv[])
{
	u8 vermod[2];
	memset(&vermod,False,2);
	u8 tmdverdata[3];
	u8 tikverdata[3];
	memset(&tmdverdata,0,3);
	memset(&tikverdata,0,3);
	for(int i = 1; i < argc; i++){
		if(strncmp(argv[i],"-tikID=",7) == 0){
			if(strlen(argv[i]+7) == 16)
				char_to_int_array(ctx->core.TicketID,argv[i]+7,0x8,BE,16);
			else
				printf("[!] Invalid length for Ticket ID\n");
		}
		else if(strncmp(argv[i],"-savesize=",10) == 0){
			u32 savesize = strtol(argv[i]+10, NULL, 10)*1024;
			u32_to_u8(ctx->core.save_data_size,savesize,LE);
		}
		else if(strncmp(argv[i],"-titleID=",9) == 0){
			if(strlen(argv[i]+9) == 16)
				char_to_int_array(ctx->core.TitleID,argv[i]+9,0x8,BE,16);
			else
				printf("[!] Invalid length for Ticket ID\n");
		}
		else if(strncmp(argv[i],"-major=",7) == 0){
			tmdverdata[0] = strtol(argv[i]+7, NULL, 10);
			vermod[0] = True;
		}
		else if(strncmp(argv[i],"-minor=",7) == 0){
			tmdverdata[1] = strtol(argv[i]+7, NULL, 10);
			vermod[0] = True;
		}
		else if(strncmp(argv[i],"-micro=",7) == 0){
			tmdverdata[2] = strtol(argv[i]+7, NULL, 10);
			vermod[0] = True;
		}
		else if(strncmp(argv[i],"-tikmajor=",10) == 0){
			tikverdata[0] = strtol(argv[i]+10, NULL, 10);
			vermod[1] = True;
		}
		else if(strncmp(argv[i],"-tikminor=",10) == 0){
			tikverdata[1] = strtol(argv[i]+10, NULL, 10);
			vermod[1] = True;
		}
		else if(strncmp(argv[i],"-tikmicro=",10) == 0){
			tikverdata[2] = strtol(argv[i]+10, NULL, 10);
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
	
	ReadFile_u32(ctx->n,u8_to_u32(header.n_offset,BE),u8_to_u32(header.n_size,BE),file);
	ReadFile_u32(ctx->e,u8_to_u32(header.e_offset,BE),u8_to_u32(header.e_size,BE),file);
	ReadFile_u32(ctx->d,u8_to_u32(header.d_offset,BE),u8_to_u32(header.d_size,BE),file);
	ReadFile_u32(ctx->name,u8_to_u32(header.name_offset,BE),u8_to_u32(header.name_size,BE),file);
	ReadFile_u32(ctx->issuer,u8_to_u32(header.issuer_offset,BE),u8_to_u32(header.issuer_size,BE),file);
	
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

int SetTicketIssuer(CIA_CONTEXT *ctx)
{
	memset(ctx->core.TicketIssuer,0x0,0x40);
	u8 old_issuer_len = strlen(ctx->keys.ticket.issuer);
	u8 name_len = strlen(ctx->keys.ticket.name);
	ctx->core.TicketIssuer[old_issuer_len] = 0x2D;
	memcpy(ctx->core.TicketIssuer,ctx->keys.ticket.issuer,old_issuer_len);
	memcpy((ctx->core.TicketIssuer+old_issuer_len+1),ctx->keys.ticket.name,name_len);
	return 0;
}

int SetTitleMetaDataIssuer(CIA_CONTEXT *ctx)
{
	memset(ctx->core.TMDIssuer,0x0,0x40);
	u8 old_issuer_len = strlen(ctx->keys.tmd.issuer);
	u8 name_len = strlen(ctx->keys.tmd.name);
	ctx->core.TMDIssuer[old_issuer_len] = 0x2D;
	memcpy(ctx->core.TMDIssuer,ctx->keys.tmd.issuer,old_issuer_len);
	memcpy((ctx->core.TMDIssuer+old_issuer_len+1),ctx->keys.tmd.name,name_len);
	return 0;
}

void ReadFile_u32(void *outbuff,u32 offset,u32 size,FILE *file)
{
	fseek(file,offset,SEEK_SET);
	fread(outbuff,size,1,file);
}
