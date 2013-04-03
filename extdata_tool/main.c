/**
Copyright 2013 3DSGuy

This file is part of extdata_tool.

extdata_tool is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

extdata_tool is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with extdata_tool.  If not, see <http://www.gnu.org/licenses/>.
**/
#include "lib.h"
#include "ExtData.h"
#include "titledb.h"
#include "main.h"

//Version
#define MAJOR 0
#define MINOR 9

void app_title(void);
void help(char *app_name);

int main(int argc, char *argv[])
{
	app_title();
	
	if (argc < 3 || argc > 18){
		printf("\n[!] Must Specify Arguments\n");
		help(argv[0]);
		return 1;
	}

	INPUT_CONTEXT ctx;
	memset(&ctx,0,sizeof(ctx));
	
	u8 empty[2] = "\0\0";
	//get options
	for(int i = 1; i < argc - 1; i++){
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0){
			help(argv[0]);
			return 1;
		}
		else if (strcmp(argv[i], "--info") == 0 || strcmp(argv[i], "-i") == 0){
			ctx.info = TRUE;
		}
		else if (strcmp(argv[i], "--titledb") == 0 || strcmp(argv[i], "-t") == 0){
			ctx.titledb_read = TRUE;
		}
		else if (strcmp(argv[i], "--extract_dir") == 0 || strcmp(argv[i], "-x") == 0){
			ctx.extract = TRUE;
			if((i+1) >= (argc - 1)){
				ctx.output_dir = empty;
				ctx.output_dir_len = 1;
			}
			else{
				ctx.output_dir = argv[i+1];
				ctx.output_dir_len = strlen(ctx.output_dir);
			}
		}
		else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0){
			ctx.verbose = TRUE;
		}
		//else{
		//	printf("[!] Unknown Option: %s\n", argv[i]);
		//}	
	}
	
	ctx.input_extdata = argv[argc - 1];
	
	FILE *extdataimg = fopen(ctx.input_extdata,"rb");
	if(extdataimg == NULL){
		printf("[!] Failed to Open '%s'\n",ctx.input_extdata);
		return IO_FAIL;
	}
	
	HEADER_CONTEXT header;
	
	memset(&header,0,sizeof(header));
	
	//Reading DIFF
	fseek(extdataimg, 0x100, SEEK_SET);
	fread(&header.DIFF,sizeof(header.DIFF),1,extdataimg);
	if(header.DIFF.magic_0 == DISA_MAGIC){
		printf("[!] This is a SaveData Image\n");
		return DIFF_CORRUPT;
	}
	else if(header.DIFF.magic_0 != DIFF_MAGIC_0 || header.DIFF.magic_1 != DIFF_MAGIC_1){
		printf("[!] DIFF Header Corrupt\n");
		return DIFF_CORRUPT;
	}
	//Reading ExtData HMAC
	fseek(extdataimg, 0x00, SEEK_SET);
	fread(header.AES_MAC,0x10,1,extdataimg);
	
	PARTITION_STRUCT partition_primary;
	PARTITION_STRUCT partition_secondary;
	
	if(header.DIFF.primary_partition_offset != 0){
		partition_primary = get_extdata_partition_header(header.DIFF.primary_partition_offset, header.DIFF.active_table_offset, extdataimg);
		if(partition_primary.valid != 0){
			printf("[!] Primary DIFI Partition Corrupt\n");
			return DIFI_CORRUPT;
		}
	}
	else{
		printf("[!] ExtData Image is Empty\n");
		return DIFI_CORRUPT;
	}
	
	if(header.DIFF.secondary_partition_offset != 0){
		partition_secondary = get_extdata_partition_header(header.DIFF.secondary_partition_offset, header.DIFF.active_table_offset, extdataimg);
		if(partition_secondary.valid != 0){
			printf("[!] Secondary DIFI Partition Corrupt\n");
			return DIFI_CORRUPT;
		}
	}
	else{
		printf("[+] No Secondary Partition Present\n");
	}
	
	if(ctx.info == TRUE){
		print_extdata_header(header);
		print_partition_info(partition_primary);
		if(header.DIFF.secondary_partition_offset != 0){
			print_partition_info(partition_secondary);
		}
	}
	
	if(ctx.extract == TRUE){
		makedir(ctx.output_dir);
		chdir(ctx.output_dir);
	}
	
	if(ctx.extract == TRUE && partition_primary.DIFI.flags[0] == 0x1){//ONE FILE		
		u64 offset = partition_primary.DIFI.data_partition_offset + partition_primary.DPFS.ivfc_offset;
		u64 size = partition_primary.IVFC.level_4_fs_size;
		get_extdata_single_blob(offset,size,extdataimg);
		printf("[*] All done\n");
	}
	
	if(ctx.extract == TRUE && partition_primary.DIFI.flags[0] == 0x0){//TWO Versions of ONE FILE	
		long int offset[2];
		long int size[2];
		memset(&offset,0x0,sizeof(offset));
		memset(&size,0x0,sizeof(size));
		//File 0 Details
		offset[0] = header.DIFF.active_table_offset + partition_primary.IVFC.level_4_fs_relative_offset + partition_primary.DPFS.ivfc_offset;
		size[0] = partition_primary.IVFC.level_4_fs_size;
		//File 1 Details
		offset[1] = header.DIFF.active_table_offset + partition_primary.DPFS.ivfc_length + partition_secondary.DPFS.ivfc_offset + partition_primary.IVFC.level_4_fs_relative_offset;
		size[1] = partition_secondary.IVFC.level_4_fs_size;
		for(int i = 0; i < 2; i++){
			get_extdata_duo_blob(offset[i],size[i],i,extdataimg);
		}
		printf("[*] All done\n");
	}
	
	if(ctx.titledb_read == TRUE){
		if(process_title_database(extdataimg,(header.DIFF.active_table_offset + partition_primary.IVFC.level_4_fs_relative_offset + partition_primary.DPFS.ivfc_offset)) != 0){
			if(partition_primary.DIFI.flags[0] == 0x0){
				if(process_title_database(extdataimg,(header.DIFF.active_table_offset + partition_primary.DPFS.ivfc_length + partition_secondary.DPFS.ivfc_offset + partition_primary.IVFC.level_4_fs_relative_offset)) != 0){
					printf("[!] %s is Corrupt, or is not a Title Database ExtData Image\n",ctx.input_extdata);
				}
			}
			else
				printf("[!] %s is Corrupt, or is not a Title Database ExtData Image\n",ctx.input_extdata);
		}
	}
	
	fclose(extdataimg);
	return 0;
}

void app_title(void)
{
	printf("CTR ExtData Tool\n");
	printf("Version %d.%d (C) 3DSGuy 2013\n",MAJOR,MINOR);
}

void help(char *app_name)
{
	printf("\nUsage: %s [options] <extdata image>\n", app_name);
	putchar('\n');
	printf("OPTIONS                 Possible Values       Explanation\n");
	//printf(" -v, --verbose                                Enable verbose output.\n");
	printf(" -h, --help                                   Print this help.\n");
	printf(" -i, --info                                   Display ExtData Info.\n");
	printf(" -x, --extract          Out-Dir               Extract Data from ExtData Image.\n");
	printf(" -t, --titledb                                Display Title Database Info\n");
}