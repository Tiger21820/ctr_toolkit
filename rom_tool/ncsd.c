#include "lib.h"
#include "ncsd.h"

int NCSDProcess(ROM_CONTEXT *ctx)
{
	ctx->ncsd_struct = malloc(sizeof(NCSD_STRUCT));
	if(GetNCSDData(ctx) != 0)
		return Fail;
		
	if(ctx->ncsd_struct->actual_rom_file_size < ctx->ncsd_struct->used_rom_size || ctx->ncsd_struct->actual_rom_file_size > ctx->ncsd_struct->rom_size){
		printf("[!] ROM is malformed\n");
		return Fail;
	}
		
	if(ctx->rom_trim_flag == True){
		if(TrimROM(ctx) != 0)
			return Fail;
	}
	if(ctx->rom_restore_flag == True){
		if(RestoreROM(ctx) != 0)
			return Fail;
	}
	return 0;
}

int TrimROM(ROM_CONTEXT *ctx)
{
	printf("[+] Trimming ROM\n");
	if(TruncateFile_u64(ctx->romfile.argument,ctx->ncsd_struct->used_rom_size) != 0){
		printf("[!] Failed to trim ROM\n");
		return Fail;
	}
	return 0;
}

int RestoreROM(ROM_CONTEXT *ctx)
{
	printf("[+] Restoring ROM\n");
	if(TruncateFile_u64(ctx->romfile.argument,ctx->ncsd_struct->rom_size) != 0){
		printf("[!] Failed to Restore ROM\n");
		return Fail;
	}
	FILE *rom = fopen(ctx->romfile.argument,"rb+");
	fseek_64(ctx->romfile.file.file,ctx->ncsd_struct->used_rom_size,SEEK_SET);
	WriteDummyBytes(ctx->romfile.file.file,0xff,(ctx->ncsd_struct->rom_size - ctx->ncsd_struct->used_rom_size));
	fclose(rom);
	return 0;
}

void WriteDummyBytes(FILE *file, u8 dummy_byte, u64 len)
{
	
	u8 dummy_bytes[16];
	memset(&dummy_bytes,dummy_byte,16);
	for(u64 i = 0; i < len; i += 16){
		fwrite(&dummy_bytes,16,1,file);
	}
}

int GetNCSDData(ROM_CONTEXT *ctx)
{
	ctx->romfile.file.file = fopen(ctx->romfile.argument,"rb");
	if(ctx->ncsd_struct == NULL)
		return Fail;
	memset(ctx->ncsd_struct,0x0,sizeof(NCSD_STRUCT));
	
	ctx->ncsd_struct->actual_rom_file_size = GetFileSize_u64(ctx->romfile.argument);
	
	NCSD_HEADER header;
	CARD_INFO_HEADER card_info;
	DEV_CARD_INFO_HEADER dev_card_info;
	
	fseek(ctx->romfile.file.file,0x0,SEEK_SET);
	fread(&ctx->ncsd_struct->signature,0x100,1,ctx->romfile.file.file);
	fseek(ctx->romfile.file.file,0x100,SEEK_SET);
	fread(&header,sizeof(NCSD_HEADER),1,ctx->romfile.file.file);
	fseek(ctx->romfile.file.file,0x200,SEEK_SET);
	fread(&card_info,sizeof(CARD_INFO_HEADER),1,ctx->romfile.file.file);
	fseek(ctx->romfile.file.file,0x1200,SEEK_SET);
	fread(&dev_card_info,sizeof(DEV_CARD_INFO_HEADER),1,ctx->romfile.file.file);
	
	if(u8_to_u32(header.magic,BE) != NCSD_MAGIC){
		printf("[!] ROM File is corrupt\n");
		goto fail;
	}
	
	u32 media_size = ((header.partition_flags[6] + 1)*0x200);
	
	u64 ROM_SIZE_MEDIAS = u8_to_u32(header.rom_size,LITTLE_ENDIAN);
	ctx->ncsd_struct->rom_size = (ROM_SIZE_MEDIAS)*(media_size);
	u32 tmp = u8_to_u32(header.offsetsize_table[0].offset,LITTLE_ENDIAN);
	for(int i = 0; i < 8; i++){
		tmp += u8_to_u32(header.offsetsize_table[i].size,LITTLE_ENDIAN);
	}
	ctx->ncsd_struct->used_rom_size = tmp*media_size;
		
	
	for(int i = 0; i < 8; i++){
		ctx->ncsd_struct->partition_data[i].offset = u8_to_u32(header.offsetsize_table[i].offset,LITTLE_ENDIAN)*media_size;
		ctx->ncsd_struct->partition_data[i].size = u8_to_u32(header.offsetsize_table[i].size,LITTLE_ENDIAN)*media_size;
		if(ctx->ncsd_struct->partition_data[i].offset != 0 && ctx->ncsd_struct->partition_data[i].size != 0)
			ctx->ncsd_struct->partition_data[i].active = True;
		ctx->ncsd_struct->partition_data[i].title_id = u8_to_u64(header.partition_id_table[i],LITTLE_ENDIAN);
		ctx->ncsd_struct->partition_data[i].fs_type = header.partitions_fs_type[i];
		ctx->ncsd_struct->partition_data[i].crypto_type = header.partitions_crypto_type[i];
		
		u8 magic[4];
		fseek_64(ctx->romfile.file.file,(ctx->ncsd_struct->partition_data[i].offset + 0x100),SEEK_SET);
		fread(&magic,4,1,ctx->romfile.file.file);
		if(u8_to_u32(magic,BE) == NCCH_MAGIC){
			u8 flags[8];
			u8 flag_bool[8];
			fseek_64(ctx->romfile.file.file,(ctx->ncsd_struct->partition_data[i].offset + 0x188),SEEK_SET);
			fread(&flags,8,1,ctx->romfile.file.file);
			resolve_flag(flags[5],flag_bool);
			if(flag_bool[1] == False && flag_bool[0] == True){
				if(flag_bool[2] == False && flag_bool[3] == True)
					ctx->ncsd_struct->partition_data[i].content_type = CFA_Manual;
				else if(flag_bool[2] == True && flag_bool[3] == True)
					ctx->ncsd_struct->partition_data[i].content_type = CFA_DLPChild;
				else if(flag_bool[2] == True && flag_bool[3] == False)
					ctx->ncsd_struct->partition_data[i].content_type = CFA_Update;
				else
					ctx->ncsd_struct->partition_data[i].content_type = _unknown;
			}
			else if(flag_bool[1] == True)
				ctx->ncsd_struct->partition_data[i].content_type = CXI;
			else
				ctx->ncsd_struct->partition_data[i].content_type = _unknown;
			fseek_64(ctx->romfile.file.file,(ctx->ncsd_struct->partition_data[i].offset + 0x150),SEEK_SET);
			fread(ctx->ncsd_struct->partition_data[i].product_code,16,1,ctx->romfile.file.file);
		}
		else
			ctx->ncsd_struct->partition_data[i].content_type = _unknown;
	}
	
	if(u8_to_u64(card_info.cver_title_id,LITTLE_ENDIAN) == 0){
		u8 stock_title_key[0x10] = {0x6E, 0xC7, 0x5F, 0xB2, 0xE2, 0xB4, 0x87, 0x46, 0x1E, 0xDD, 0xCB, 0xB8, 0x97, 0x11, 0x92, 0xBA};
		if(memcmp(dev_card_info.TitleKey,stock_title_key,0x10) == 0)
			ctx->ncsd_struct->type = dev_external_SDK;
		else
			ctx->ncsd_struct->type = dev_internal_SDK;
	}
	else
		ctx->ncsd_struct->type = retail;
	
	
	if(ctx->info_flag)
		PrintNCSDData(ctx->ncsd_struct,&header,&card_info,&dev_card_info);
	
	fclose(ctx->romfile.file.file);
	return 0;
fail:
	fclose(ctx->romfile.file.file);
	return Fail;

}

void PrintNCSDData(NCSD_STRUCT *ctx, NCSD_HEADER *header, CARD_INFO_HEADER *card_info, DEV_CARD_INFO_HEADER *dev_card_info)
{
	printf("[+] ROM Info\n");
	memdump(stdout,"Signature:              ",ctx->signature,0x100);
	printf("Magic:                  NCSD\n");
	switch(ctx->type){
		case retail : 
			printf("Target:                 Retail/Production\n"); 
			printf("CVer Title ID:          %016llx\n",u8_to_u64(card_info->cver_title_id,LITTLE_ENDIAN));
			printf("CVer Title Ver:         v%d\n",u8_to_u16(card_info->cver_title_version,LITTLE_ENDIAN));
			char *FW_STRING = malloc(10);
			memset(FW_STRING,0,10);
			GetMin3DSFW(FW_STRING,card_info);
			printf("Min 3DS Firm:           %s\n",FW_STRING);
			free(FW_STRING);
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
		printf("ROM Cart Size:          %lld GB",ctx->rom_size/GB); printf(" (%lld Gbit)\n",(ctx->rom_size/GB)*8);
	}
	else{
		printf("ROM Cart Size:          %lld MB",ctx->rom_size/MB); 
		u32 tmp = (ctx->rom_size/MB)*8;
		if(tmp >= 1024)
			printf(" (%d Gbit)\n",tmp/1024);
		else
			printf(" (%d Mbit)\n",tmp);
	}
	if(ctx->used_rom_size >= MB){
		printf("ROM Used Size:          %lld MB",ctx->used_rom_size/MB); printf(" (0x%llx bytes)\n",ctx->used_rom_size);
	}
	else if(ctx->used_rom_size >= KB){
		printf("ROM Used Size:          %lld KB",ctx->used_rom_size/KB); printf(" (0x%llx bytes)\n",ctx->used_rom_size);
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
			printf(" Product Code:          %s\n",ctx->partition_data[i].product_code);
			printf(" Content Type:          ");
			switch(ctx->partition_data[i].content_type){
				case _unknown : printf("Unknown\n"); break;
				case CXI : printf("Application\n"); break;
				case CFA_Manual : printf("Electronic Manual\n"); break;
				case CFA_DLPChild : printf("Download Play Child\n"); break;
				case CFA_Update : printf("Software Update Partition\n"); break;
			}
			
			printf(" FS Type:               %x\n",ctx->partition_data[i].fs_type);
			printf(" Crypto Type:           %x\n",ctx->partition_data[i].crypto_type);
			printf(" Offset:                0x%x\n",ctx->partition_data[i].offset);
			printf(" Size:                  0x%x\n",ctx->partition_data[i].size);
			printf("\n");
		}
	}
}

void GetMin3DSFW(char *FW_STRING, CARD_INFO_HEADER *card_info)
{
	u8 MAJOR = 0;
	u8 MINOR = 0;
	u8 BUILD = 0;
	char REGION_CHAR = 'X';

	u16 CVer_ver = u8_to_u16(card_info->cver_title_version,LE);
	u32 CVer_UID = u8_to_u32(card_info->cver_title_id,LE);
		
	switch(CVer_UID){
		case EUR_ROM : REGION_CHAR = 'E'; break;
		case JPN_ROM : REGION_CHAR = 'J'; break;
		case USA_ROM : REGION_CHAR = 'U'; break;
		case CHN_ROM : REGION_CHAR = 'C'; break;
		case KOR_ROM : REGION_CHAR = 'K'; break;
		case TWN_ROM : REGION_CHAR = 'T'; break;
	}
	
	
	switch(CVer_ver){
		case 3088 : MAJOR = 3; MINOR = 0; BUILD = 0; break;
		default : MAJOR = CVer_ver/1024; MINOR = (CVer_ver - 1024*(CVer_ver/1024))/0x10; break;//This tends to work 98% of the time, use above for manual overides
	}
	sprintf(FW_STRING,"%d.%d.%d-X%c",MAJOR,MINOR,BUILD,REGION_CHAR);
}
