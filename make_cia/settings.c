#include "lib.h"
#include "ctr_crypto.h"
#include "yaml.h"
#include "settings.h"
#include "srl.h"
#include "ncch.h"

void InitialiseSettings(CIA_CONTEXT *ctx)
{
	memset(ctx,0x0,sizeof(CIA_CONTEXT));
}

int GetSettings(CIA_CONTEXT *ctx)
{
	ctx->configfile.file.used = TRUE;
	ctx->configfile.file.file = fopen(ctx->configfile.argument,"rb");
	if(ctx->configfile.file.file == NULL){
		printf("[!] Failed to open '%s'\n",ctx->configfile.argument);
		ctx->configfile.file.used = FALSE;
		return 1;
	}
	
	if(LoadAESKeys(ctx) != 0){
		return 1;
	}
	if(LoadRSAKeys(ctx) != 0){
		return 1;
	}
	if(ImportCertificates(ctx) != 0){
		return 1;
	}
	if(GetCoreInfo(ctx) != 0){
		return 1;
	}
	if(SetBuildSettings(ctx) != 0){	
		return 1;
	}
	
	if(ctx->showkeys_flag){
		printf("\n[+] Imported AES Key Data\n");
		memdump(stdout,"CommonKey:   ",ctx->keys.common_key.key,0x10);
		printf("CommonKeyID: %02x\n",ctx->keys.common_key_id);
		memdump(stdout,"TitleKey:    ",ctx->keys.title_key.key,0x10);
		memdump(stdout,"CXIKey:      ",ctx->keys.ncch_key.key,0x10);
		printf("[+] Imported RSA Key Data\n");
		printf(" > Ticket:\n");
		PrintRSAKeyData(&ctx->keys.ticket);
		printf("\n > Title Meta Data:\n");
		PrintRSAKeyData(&ctx->keys.tmd);
	}
	if(ctx->verbose_flag){
		printf("[+] Content Data:\n");
		memdump(stdout, "Title ID:               ", ctx->core.TitleID, 0x8);
		memdump(stdout, "Ticket ID:              ", ctx->core.TicketID, 0x8);
		printf("Title Version:          v%d\n",u8_to_u16(ctx->core.TitleVersion,BIG_ENDIAN));
		printf("Ticket Version:         v%d\n",u8_to_u16(ctx->core.TicketVersion,BIG_ENDIAN));
		memdump(stdout, "TitleType:              ", ctx->core.Title_type, 0x4);
		printf("Ticket Issuer:          %s\n",ctx->core.TicketIssuer);
		printf("TMD Issuer:             %s\n",ctx->core.TMDIssuer);
	}
	
	return 0;
}

int LoadAESKeys(CIA_CONTEXT *ctx)
{
	char *keybuff = malloc(100);
	fseek(ctx->configfile.file.file,0x0,SEEK_SET);
	if(key_search("AESKeys",ctx->configfile.file.file) == FOUND){
		long pos = ftell(ctx->configfile.file.file);
		if(get_value(keybuff,33,"CommonKey",ctx->configfile.file.file) == FOUND){
			char_to_int_array(ctx->keys.common_key.key,keybuff,0x10,BIG_ENDIAN,16);
		}
		else{
			printf("[!] Common Key was not specified\n");
			goto failcleanup;
		}
		fseek(ctx->configfile.file.file,pos,SEEK_SET);
		if(get_value(keybuff,3,"CommonKeyID",ctx->configfile.file.file) == FOUND){
			char_to_int_array(&ctx->keys.common_key_id,keybuff,1,BIG_ENDIAN,16);
		}
		else{
			printf("[!] Common Key ID was not specified\n");
			goto failcleanup;
		}
		fseek(ctx->configfile.file.file,pos,SEEK_SET);
		if(get_value(keybuff,33,"TitleKey",ctx->configfile.file.file) == FOUND){
			char_to_int_array(ctx->keys.title_key.key,keybuff,0x10,BIG_ENDIAN,16);
		}
		else{
			printf("[+] Title Key was not specified, using default Title Key\n");
			memset(ctx->keys.title_key.key,0x0,0x10);
		}
		fseek(ctx->configfile.file.file,pos,SEEK_SET);
		if(get_value(keybuff,33,"CXIKey",ctx->configfile.file.file) == FOUND){
			char_to_int_array(ctx->keys.ncch_key.key,keybuff,0x10,BIG_ENDIAN,16);
		}
		else{
			printf("[+] CXI Key was not specified, generating meta region may not be possible\n");
			memset(ctx->keys.ncch_key.key,0x0,0x10);
		}
	}
	else{
		printf("[!] Could not locate 'AESKeys' section in %s\n",ctx->configfile.argument);
	}
	return 0;
failcleanup:
	free(keybuff);
	return 1;
}

int LoadRSAKeys(CIA_CONTEXT *ctx)
{
	char *filepath = malloc(100);
	fseek(ctx->configfile.file.file,0x0,SEEK_SET);
	if(key_search("RSAKeys",ctx->configfile.file.file) == FOUND){
		long pos = ftell(ctx->configfile.file.file);
		if(get_value(filepath,100,"Ticket",ctx->configfile.file.file) == FOUND){
			FILE *ticket_key = fopen(filepath,"rb");
			if(ticket_key == NULL){
				printf("[!] Could not open, %s\n",filepath);
				goto failcleanup;
			}
			u8 result = LoadRSAKeyFile(&ctx->keys.ticket,ticket_key);
			fclose(ticket_key);
			if(result != 0){
				printf("[!] Ticket Key file Error\n");
				goto failcleanup;
			}
		}
		else{
			printf("[!] Ticket RSA Key file was not specified\n");
			goto failcleanup;
		}
		fseek(ctx->configfile.file.file,pos,SEEK_SET);
		if(get_value(filepath,100,"TitleMetaData",ctx->configfile.file.file) == FOUND){
			FILE *tmd_key = fopen(filepath,"rb");
			if(tmd_key == NULL){
				printf("[!] Could not open, %s\n",filepath);
				goto failcleanup;
			}
			u8 result = LoadRSAKeyFile(&ctx->keys.tmd,tmd_key);
			fclose(tmd_key);
			if(result != 0){
				printf("[!] TMD Key file Error\n");
				goto failcleanup;
			}
		}
		else{
			printf("[!] TMD RSA Key file was not specified\n");
			goto failcleanup;
		}
	}
	else{
		printf("[!] Could not locate 'RSAKeys' section in %s\n",ctx->configfile.argument);
		goto failcleanup;
	}
	return 0;
failcleanup:
	free(filepath);
	return 1;
}

int LoadRSAKeyFile(RSA_2048_KEY *ctx, FILE *file)
{
	CRKF_HEADER header;
	memset(&header,0x0,sizeof(CRKF_HEADER));
	
	fseek(file,0x0,SEEK_SET);
	fread(&header,sizeof(CRKF_HEADER),1,file);

	if(u8_to_u32(header.magic,BIG_ENDIAN) != 0x43524B46){
		printf("[!] Key File is corrupt\n");
		return rsa_key_fail;
	}
	
	if(u8_to_u16(header.rsatype,BIG_ENDIAN) != 0x1){
		printf("[!] Only RSA-2048 is supported\n");
		return rsa_key_fail;
	}
	
	ctx->keytype = RSAKEY_PRIV;
	
	ReadFile_u32(ctx->n,u8_to_u32(header.n_offset,BIG_ENDIAN),u8_to_u32(header.n_size,BIG_ENDIAN),file);
	ReadFile_u32(ctx->e,u8_to_u32(header.e_offset,BIG_ENDIAN),u8_to_u32(header.e_size,BIG_ENDIAN),file);
	ReadFile_u32(ctx->d,u8_to_u32(header.d_offset,BIG_ENDIAN),u8_to_u32(header.d_size,BIG_ENDIAN),file);
	ReadFile_u32(ctx->name,u8_to_u32(header.name_offset,BIG_ENDIAN),u8_to_u32(header.name_size,BIG_ENDIAN),file);
	ReadFile_u32(ctx->issuer,u8_to_u32(header.issuer_offset,BIG_ENDIAN),u8_to_u32(header.issuer_size,BIG_ENDIAN),file);
	
	return 0;
}

void PrintRSAKeyData(RSA_2048_KEY *ctx)
{
	printf("Key Name:    %s\n", ctx->name);
	printf("Key Issuer:  %s\n", ctx->issuer);
	printf("\n");
	memdump(stdout, "n:           ", ctx->n, 0x100);
	printf("\n");
	memdump(stdout, "e:           ", ctx->e, 0x3);
	printf("\n");
	memdump(stdout, "d:           ", ctx->d, 0x100);
}

int ImportCertificates(CIA_CONTEXT *ctx)
{
	CERT_CONTEXT *cert_ctx = malloc(sizeof(CERT_CONTEXT));
	memset(cert_ctx,0x0,sizeof(CERT_CONTEXT));
	
	fseek(ctx->configfile.file.file,0x0,SEEK_SET);
	if(key_search("Cerificates",ctx->configfile.file.file) == FOUND){
		u8 result;
		long pos = ftell(ctx->configfile.file.file);
		result = ImportCertificateFile(&cert_ctx->ca,"CA",ctx->configfile.file.file);
		if(result != 0){
			printf("[!] Failed to import CA certificate\n");
			goto failcleanup;
		}
		fseek(ctx->configfile.file.file,pos,SEEK_SET);
		result = ImportCertificateFile(&cert_ctx->ticket,"Ticket",ctx->configfile.file.file);
		if(result != 0){
			printf("[!] Failed to import Ticket certificate\n");
			goto failcleanup;
		}
		fseek(ctx->configfile.file.file,pos,SEEK_SET);
		result = ImportCertificateFile(&cert_ctx->tmd,"TitleMetaData",ctx->configfile.file.file);
		if(result != 0){
			printf("[!] Failed to import TMD certificate\n");
			goto failcleanup;
		}
	}
	else{
		printf("[!] Could not locate 'Certificate' section in %s\n",ctx->configfile.argument);
		goto failcleanup;
	}
	
	ctx->certchain.used = TRUE;
	ctx->certchain.size = cert_ctx->ca.size + cert_ctx->ticket.size + cert_ctx->tmd.size;
	ctx->certchain.buffer = malloc(ctx->certchain.size);
	memcpy((ctx->certchain.buffer+0),cert_ctx->ca.data,cert_ctx->ca.size);
	memcpy((ctx->certchain.buffer+cert_ctx->ca.size),cert_ctx->ticket.data,cert_ctx->ticket.size);
	memcpy((ctx->certchain.buffer+cert_ctx->ca.size+cert_ctx->ticket.size),cert_ctx->tmd.data,cert_ctx->tmd.size);
	free(cert_ctx);
	
	return 0;
failcleanup:
	free(cert_ctx);
	return 1;
}

int ImportCertificateFile(CERT_BUFF *buff, char *cert_lable, FILE *config_file)
{	
	char *file_name = malloc(100);
	char *name = malloc(0x40);
	char *issuer = malloc(0x40);
	u8 result;
	
	long orig_pos = ftell(config_file);
	
	if(key_search(cert_lable,config_file) == FOUND){
		long pos = ftell(config_file);
		result = get_value(file_name,100,"FilePath",config_file);
		if(result != 0){
			printf("[!] %s Certificate file location was not specified\n",cert_lable);
			goto failcleanup;
		}
		fseek(config_file,pos,SEEK_SET);
		result = get_value(name,0x40,"CertName",config_file);
		if(result != 0){
			printf("[!] %s Certificate name was not specified\n",cert_lable);
			goto failcleanup;
		}
		fseek(config_file,pos,SEEK_SET);
		result = get_value(issuer,0x40,"CertIssuer",config_file);
		if(result != 0){
			printf("[!] %s Certificate issuer was not specified\n",cert_lable);
			goto failcleanup;
		}
	}
	else{
		printf("[!] Could not locate '%s' section in Settings File\n",cert_lable);
		goto failcleanup;
	}
	
	fseek(config_file,orig_pos,SEEK_SET);
	
	FILE *cert = fopen(file_name,"rb");
	if(cert == NULL){
		printf("[!] Failed to open '%s'\n",file_name);
		goto failcleanup;
	}
	
	u32 SigType;
	u32 SigSize;
	u32 SigPaddingSize;
	fread(&SigType,0x4,1,cert);
	switch(SigType){
		case(RSA_4096_SHA1): SigSize = 0x200; SigPaddingSize = 0x3C; break;
		case(RSA_2048_SHA1): SigSize = 0x100; SigPaddingSize = 0x3C; break;
		case(ECC_SHA1): SigSize = 0x3C; SigPaddingSize = 0x40; break;
		case(RSA_4096_SHA256): SigSize = 0x200; SigPaddingSize = 0x3C; break;
		case(RSA_2048_SHA256): SigSize = 0x100; SigPaddingSize = 0x3C; break;
		case(ECC_SHA256): SigSize = 0x3C; SigPaddingSize = 0x40; break;
		default: printf("[!] Certificate '%s' is corrupt\n",file_name); goto failcleanup2;
	}
	
	fseek(cert,(0x4+SigSize+SigPaddingSize),SEEK_SET);
	
	CERT_DATA_STRUCT header;
	fread(&header,sizeof(CERT_DATA_STRUCT),1,cert);
	if(strcmp(header.issuer,issuer) != 0 || strcmp(header.name,name) != 0 ){
		printf("[!] Certificate '%s' is corrupt\n",file_name);
		goto failcleanup2;
	}
	
	u32 PublicKeySize;
	u32 PublicKeyPaddingSize;
	switch(u8_to_u32(header.type,BIG_ENDIAN)){
		case(RSA_4096_PUBK): PublicKeySize = 0x204; PublicKeyPaddingSize = 0x34; break;
		case(RSA_2048_PUBK): PublicKeySize = 0x104; PublicKeyPaddingSize = 0x34; break;
		case(ECC_PUBK): PublicKeySize = 0x3C; PublicKeyPaddingSize = 0x3C; break;
		default: printf("[!] Certificate '%s' is corrupt\n",file_name); goto failcleanup2;
	}
	
	buff->size = (0x4+SigSize+SigPaddingSize+sizeof(CERT_DATA_STRUCT)+PublicKeySize+PublicKeyPaddingSize);
	buff->data = malloc(buff->size);
	
	fseek(cert,0x0,SEEK_SET);
	fread(buff->data,buff->size,1,cert);
	
	fclose(cert);
	free(file_name);
	free(name);
	free(issuer);
	
	return 0;
failcleanup:
	free(file_name);
	free(name);
	free(issuer);
	return 1;
failcleanup2:
	fclose(cert);
	free(file_name);
	free(name);
	free(issuer);
	return 1;
}

int GetCoreInfo(CIA_CONTEXT *ctx)
{
	char file_name[100];
	char typebuff[10];
	char CountBuff[10];
	u8 result;
	
	fseek(ctx->configfile.file.file,0x0,SEEK_SET);
	if(key_search("ContentSettings",ctx->configfile.file.file) == FOUND){
		long pos = ftell(ctx->configfile.file.file);
		result = get_value(typebuff,10,"ContentPlatform",ctx->configfile.file.file);
		if(result != 0){
			printf("[!] Content Type was not specified\n");
			return 1;
		}
		fseek(ctx->configfile.file.file,pos,SEEK_SET);
		result = get_value(CountBuff,5,"ContentCount",ctx->configfile.file.file);
		ctx->ContentCount = strtol(CountBuff, NULL, 10);
		if(result != 0 || ctx->ContentCount == 0){
			printf("[!] Content Count was not specified or invalid\n");
			return 1;
		}
		fseek(ctx->configfile.file.file,pos,SEEK_SET);
		if(key_search("Content0",ctx->configfile.file.file) == FOUND){
			result = get_value(file_name,100,"FilePath",ctx->configfile.file.file);
			if(result != 0){
				printf("[!] Content0 file location was not specified\n");
				return 1;
			}
		}
		else{
			printf("[!] Could not locate 'Content0' section in %s\n",ctx->configfile.argument);
			return 1;
		}
	}
	else{
		printf("[!] Could not locate 'ContentSettings' section in %s\n",ctx->configfile.argument);
		return 1;
	}
	
	if(ctx->ContentCount == 0){
		printf("[!] Content Count must be greater than 0, to create a CIA\n");
		return 1;
	}
	
	if(strcmp(typebuff,"NTR") == 0)
		ctx->core.Platform = NTR;
	else if(strcmp(typebuff,"TWL") == 0)
		ctx->core.Platform = TWL;
	else if(strcmp(typebuff,"CTR") == 0)
		ctx->core.Platform = CTR;
	else{
		printf("[!] Unrecognised Content Type: '%s'\n",typebuff);
		return 1;
	}
	
	if(ctx->core.Platform != CTR && ctx->ContentCount != 1){
		printf("[!] There can only be one Content for Non-CTR titles\n");
		return 1;
	}
	
	FILE *content = fopen(file_name,"rb");
	
	u8 core_get_result = 1;
	
	switch(ctx->core.Platform){
		case(NTR): core_get_result = GetCoreContentSRL(&ctx->core,NTR,content); break;
		case(TWL): core_get_result = GetCoreContentSRL(&ctx->core,TWL,content); break;
		case(CTR): core_get_result = GetCoreContentNCCH(&ctx->core,content); break;
	}
	
	if(core_get_result != 0){
		printf("[!] Failed to retrieve Content0 info\n");
		fclose(content);
		return 1;
	}
	
	u32_to_u8(ctx->core.Title_type,TYPE_CTR,BIG_ENDIAN);
	memset(ctx->core.DeviceID,0x0,0x4);
	ctx->core.tmd_format_ver = 0x1;
	ctx->core.ticket_format_ver = 0x1;
	ctx->core.ca_crl_version = 0x0;
	ctx->core.signer_crl_version = 0x0;
	
	u8 hash[0x20];
	ctr_sha_256(file_name,100,hash);
	memcpy(ctx->core.TicketID,&hash,0x8);
	
	SetTicketIssuer(ctx);
	SetTitleMetaDataIssuer(ctx);
	fclose(content);
	
	return 0;
}

int SetBuildSettings(CIA_CONTEXT *ctx)
{
	char settingbuff[100];
	fseek(ctx->configfile.file.file,0x0,SEEK_SET);
	if(key_search("BuildSettings",ctx->configfile.file.file) == FOUND){
		long pos = ftell(ctx->configfile.file.file);
		if(get_value(settingbuff,17,"OverrideTitleID",ctx->configfile.file.file) == FOUND){
			printf("\n[+] Overriding original TitleID with '%s'\n",settingbuff);
			char_to_int_array(ctx->core.TitleID,settingbuff,0x8,BIG_ENDIAN,16);
		}
		fseek(ctx->configfile.file.file,pos,SEEK_SET);
		if(get_value(settingbuff,17,"OverrideTicketID",ctx->configfile.file.file) == FOUND){
			printf("\n[+] Overriding original TicketID with '%s'\n",settingbuff);
			char_to_int_array(ctx->core.TicketID,settingbuff,0x8,BIG_ENDIAN,16);
		}
		fseek(ctx->configfile.file.file,pos,SEEK_SET);
		if(get_value(settingbuff,5,"TicketVersion",ctx->configfile.file.file) == FOUND){
			u16 ver = strtol(settingbuff, NULL, 10);
			u16_to_u8(ctx->core.TicketVersion,ver,BIG_ENDIAN);
			printf("\n[+] Manually Setting Title Version to v%d\n",ver);
		}
		fseek(ctx->configfile.file.file,pos,SEEK_SET);
		if(get_value(settingbuff,5,"TitleVersion",ctx->configfile.file.file) == FOUND){
			u16 ver = strtol(settingbuff, NULL, 10);
			u16_to_u8(ctx->core.TitleVersion,ver,BIG_ENDIAN);
			printf("\n[+] Manually Setting Title Version to v%d\n",ver);
		}
		fseek(ctx->configfile.file.file,pos,SEEK_SET);
		ctx->meta_flag = get_boolean("GenerateMeta",ctx->configfile.file.file);
	}
	else{
		printf("[!] Could not locate 'BuildSettings' section in %s\n",ctx->configfile.argument);
		return 1;
	}
	
	if(ctx->core.Platform != CTR && ctx->meta_flag == TRUE){
		printf("[!] Meta Region cannot be generated for Non-CTR titles\n");
		return 1;
	}
	
	return 0;
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

int GetContentInfo(CONTENT_INFO *ctx, int content_index, FILE *config_file)
{
	memset(ctx,0x0,sizeof(CONTENT_INFO));
	
	char buffer[0x500];
	u8 result;
	char ContentName[15];
	sprintf(ContentName,"Content%d",content_index);
	
	ctx->valid = TRUE;
	ctx->content_index = content_index;
	
	fseek(config_file,0x0,SEEK_SET);
	if(key_search("ContentSettings",config_file) == FOUND){
		if(key_search(ContentName,config_file) == FOUND){
			long pos = ftell(config_file);
			result = get_value(ctx->file_path,100,"FilePath",config_file);
			if(result != 0){
				printf("[!] %s file location was not specified\n",ContentName);
				return 1;
			}
			
			fseek(config_file,pos,SEEK_SET);
			if(get_value(buffer,10,"ContentID",config_file) == FOUND){
				char_to_int_array(ctx->content_id,buffer,0x4,BIG_ENDIAN,16);
			}
			else{
				printf("[!] %s Content ID was not specified\n",ContentName);
				return 1;
			}
			fseek(config_file,pos,SEEK_SET);
			if(key_search("ContentFlags",config_file) == FOUND){
				pos = ftell(config_file);
				if(get_boolean("Encrypted",config_file) == TRUE){
					ctx->encrypted = TRUE;
					ctx->content_type += Encrypted;
				}
				fseek(config_file,pos,SEEK_SET);
				if(get_boolean("Optional",config_file) == TRUE){
					ctx->content_type += Optional;
				}
				fseek(config_file,pos,SEEK_SET);
				if(get_boolean("Shared",config_file) == TRUE){
					ctx->content_type += Shared;
				}
			}
			else{
				printf("[!] No Content Flags for %s where specified\n",ContentName);
				return 1;
			}
		}
		else{
			printf("[!] Could not locate '%s' section in settings file\n",ContentName);
			return 1;
		}
	}
	else{
		printf("[!] Could not locate 'ContentSettings' section in settings file\n");
		return 1;
	}
	return 0;
}

void ReadFile_u32(void *outbuff,u32 offset,u32 size,FILE *file)
{
	fseek(file,offset,SEEK_SET);
	fread(outbuff,size,1,file);
}
