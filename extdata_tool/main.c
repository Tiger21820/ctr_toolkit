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
#include "vsxe.h"
#include "main.h"

//Version
typedef enum
{
	MAJOR = 1,
	MINOR = 2
} app_version;

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
	
	for(int i = 1; i < argc - 1; i++){
		if (strcmp(argv[i], "--mode") == 0 || strcmp(argv[i], "-m") == 0){
			if(strcmp(argv[i+1],"IMG") == 0)
				ctx.mode = Image;
			else if(strcmp(argv[i+1],"FS") == 0)
				ctx.mode = Directory;
			else{
				printf("[!] Unrecognised mode: '%s'\n",argv[i+1]);
				return 1;
			}
		}
	}
	if(ctx.mode == 0){
		printf("[!] No mode was specified\n");
		return 1;
	}
	
	ctx.extdataimg_path = malloc(100);
	switch(ctx.mode){
		case Image:
			ctx.input = argv[argc - 1];
			ctx.extdataimg_path = ctx.input;
			break;
		case Directory:
			ctx.input = argv[argc - 1];
			u8 inputlen = strlen(ctx.input);
#ifdef _WIN32
			if(ctx.input[inputlen - 1] != 0x5C) 
				sprintf(ctx.extdataimg_path,"%s%c00000001.dec",ctx.input,0x5C);
			else
				sprintf(ctx.extdataimg_path,"%s00000001.dec",ctx.input);
#else
			if(ctx.input[inputlen - 1] != 0x2F) 
				sprintf(ctx.extdataimg_path,"%s%c00000001.dec",ctx.input,0x2F);
			else
				sprintf(ctx.extdataimg_path,"%s00000001.dec",ctx.input);
#endif
			break;
	}
	ctx.extdataimg = fopen(ctx.extdataimg_path,"rb");
	if(ctx.extdataimg == NULL){
		printf("[!] Failed to Open '%s'\n",ctx.extdataimg_path);
		return IO_FAIL;
	}
	
	
	if (getcwdir(ctx.cwd, 0x400) == NULL){
		printf("[!] Could not store Current Working Directory\n");
		return IO_FAIL;
	}
	
	for(int i = 1; i < argc - 1; i++){
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0){
			help(argv[0]);
			return 1;
		}
		else if (strcmp(argv[i], "--info") == 0 || strcmp(argv[i], "-i") == 0){
			ctx.info = True;
		}
		else if (strcmp(argv[i], "--extract") == 0 || strcmp(argv[i], "-x") == 0){
			ctx.extract = True;
			ctx.output = argv[i+1];
		}
		else if (strcmp(argv[i], "--viewFS") == 0 || strcmp(argv[i], "-v") == 0){
			ctx.fs_info = True;
		}
		else if (strcmp(argv[i], "--titledb") == 0 || strcmp(argv[i], "-t") == 0){
			ctx.titledb_read = True;
		}
		else if (strcmp(argv[i], "--listdb") == 0){
			ctx.listdb = True;
		}
	}

	EXTDATA_HEADER_CONTEXT header;
	
	memset(&ctx.header,0,sizeof(header));
	
	//Reading DIFF
	fseek(ctx.extdataimg, 0x100, SEEK_SET);
	fread(&ctx.header.DIFF,sizeof(DIFF_STRUCT),1,ctx.extdataimg);
	if(ctx.header.DIFF.magic_0 == DISA_MAGIC){
		printf("[!] This is a SaveData Image\n");
		return DIFF_CORRUPT;
	}
	else if(ctx.header.DIFF.magic_0 != DIFF_MAGIC_0 || ctx.header.DIFF.magic_1 != DIFF_MAGIC_1){
		printf("[!] DIFF Header Corrupt\n");
		return DIFF_CORRUPT;
	}
	//Reading ExtData HMAC
	fseek(ctx.extdataimg, 0x00, SEEK_SET);
	fread(ctx.header.AES_MAC,0x10,1,ctx.extdataimg);
	
	if(ctx.header.DIFF.primary_partition_offset != 0){
		ctx.partition[primary] = get_extdata_partition_header(ctx.header.DIFF.primary_partition_offset, ctx.header.DIFF.active_table_offset, ctx.extdataimg);
		if(ctx.partition[primary].valid != 0){
			printf("[!] Primary DIFI Partition Corrupt\n");
			return DIFI_CORRUPT;
		}
	}
	else{
		printf("[!] ExtData Image is Empty\n");
		return DIFI_CORRUPT;
	}
	
	if(ctx.header.DIFF.secondary_partition_offset != 0){
		ctx.partition[secondary] = get_extdata_partition_header(ctx.header.DIFF.secondary_partition_offset, ctx.header.DIFF.active_table_offset, ctx.extdataimg);
		if(ctx.partition[secondary].valid != 0){
			printf("[!] Secondary DIFI Partition Corrupt\n");
			return DIFI_CORRUPT;
		}
	}
	else{
		printf("[+] No Secondary Partition Present\n");
	}
	
	//Performing Functions Now
	
	if(ctx.info == True){
		print_extdata_header(ctx.header);
		print_partition_info(ctx.partition[primary]);
		if(header.DIFF.secondary_partition_offset != 0){
			print_partition_info(ctx.partition[secondary]);
		}
	}
	
	if(ctx.titledb_read == True || ctx.listdb == True){
		if(ctx.mode != Image){
			printf("[!] Database ExtData images (*.db) do not support being stored in Title ExtData directories\n");
			return 1;
		}
		int db_mode = Normal;
		if(ctx.listdb == True)
			db_mode = ByTID;
		if(ProcessTitleDB(ctx.extdataimg, db_mode,(ctx.header.DIFF.active_table_offset + ctx.partition[primary].IVFC.level_4_fs_relative_offset + ctx.partition[primary].DPFS.ivfc_offset)) != 0){
			if(ctx.partition[primary].DIFI.flags[0] == 0x0){
				if(ProcessTitleDB(ctx.extdataimg, db_mode,(ctx.header.DIFF.active_table_offset + ctx.partition[primary].DPFS.ivfc_length + ctx.partition[secondary].DPFS.ivfc_offset + ctx.partition[primary].IVFC.level_4_fs_relative_offset)) != 0){
					printf("[!] %s is Corrupt, or is not a Title Database ExtData Image\n",ctx.extdataimg_path);
				}
			}
			else
				printf("[!] %s is Corrupt, or is not a Title Database ExtData Image\n",ctx.extdataimg_path);
		}
	}
	
	if(ctx.extract == True && ctx.mode == Image){
		if(ctx.partition[primary].DIFI.flags[0] == 1){//ONE FILE
			u64 offset = ctx.partition[primary].DIFI.data_partition_offset + ctx.partition[primary].DPFS.ivfc_offset;
			u64 size = ctx.partition[primary].IVFC.level_4_fs_size;
			get_extdata_single_blob(ctx.output,offset,size,ctx.extdataimg);
		}
		else if(ctx.partition[primary].DIFI.flags[0] == 0){//TWO Versions of ONE FILE
			long int offset[2];
			long int size[2];
			memset(&offset,0x0,sizeof(offset));
			memset(&size,0x0,sizeof(size));
			//File 0 Details
			offset[0] = ctx.header.DIFF.active_table_offset + ctx.partition[primary].IVFC.level_4_fs_relative_offset + ctx.partition[primary].DPFS.ivfc_offset;
			size[0] = ctx.partition[primary].IVFC.level_4_fs_size;
			//File 1 Details
			offset[1] = ctx.header.DIFF.active_table_offset + ctx.partition[primary].DPFS.ivfc_length + ctx.partition[secondary].DPFS.ivfc_offset + ctx.partition[primary].IVFC.level_4_fs_relative_offset;
			size[1] = ctx.partition[secondary].IVFC.level_4_fs_size;
			for(int i = 0; i < 2; i++)
				get_extdata_duo_blob(ctx.output,offset[i],size[i],i,ctx.extdataimg);
		}
	}
	
	if((ctx.extract == True || ctx.fs_info == True) && ctx.mode == Directory){
		if(ProcessExtData_FS(&ctx) != 0){
			printf("[!] ExtData FileSystem could not be processed\n");
			return 1;
		}
	}
	
	//free(ctx.input);
	//if(ctx.extract == True)
	//	free(ctx.output);
	//free(ctx.extdataimg_path);
	fclose(ctx.extdataimg);
	printf("[*] Done\n");
	return 0;
	/**
	u8 empty[2] = "\0\0";
	//get options
	for(int i = 1; i < argc - 1; i++){
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0){
			help(argv[0]);
			return 1;
		}
		else if (strcmp(argv[i], "--info") == 0 || strcmp(argv[i], "-i") == 0){
			ctx.info = True;
		}
		else if (strcmp(argv[i], "--extract") == 0 || strcmp(argv[i], "-x") == 0){
			ctx.extract = True;
		}
		else if (strcmp(argv[i], "--mode") == 0 || strcmp(argv[i], "-m") == 0){
			if(strcmp(argv[i+1],"Image") == 0)
				ctx.mode = Image;
			else if(strcmp(argv[i+1],"Dir") == 0)
				ctx.mode = Directory;
			else{
				printf("[!] Unrecognised mode: '%s'\n",argv[i+1]);
				return 1;
			}
		}
		else if (strcmp(argv[i], "--viewFS") == 0 || strcmp(argv[i], "-v") == 0){
			ctx.extract = True;
		}
		else if (strcmp(argv[i], "--titledb") == 0 || strcmp(argv[i], "-t") == 0){
			ctx.titledb_read = True;
		}
		else if (strcmp(argv[i], "--listdb") == 0){
			ctx.listdb = True;
		}
		//else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0){
		//	ctx.verbose = True;
		//}
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
	
	EXTDATA_HEADER_CONTEXT header;
	
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
	
	if(ctx.info == True){
		print_extdata_header(header);
		print_partition_info(partition_primary);
		if(header.DIFF.secondary_partition_offset != 0){
			print_partition_info(partition_secondary);
		}
	}
	
	if(ctx.vsxe == True){
		u64 offset = (header.DIFF.active_table_offset + partition_primary.IVFC.level_4_fs_relative_offset + partition_primary.DPFS.ivfc_offset);
		read_vsxe(extdataimg,offset);
	}
	
	if(ctx.extract == True){
		makedir(ctx.output_dir);
		chdir(ctx.output_dir);
	}
	
	if(ctx.extract == True && partition_primary.DIFI.flags[0] == 0x1){//ONE FILE		
		u64 offset = partition_primary.DIFI.data_partition_offset + partition_primary.DPFS.ivfc_offset;
		u64 size = partition_primary.IVFC.level_4_fs_size;
		get_extdata_single_blob(offset,size,extdataimg);
		printf("[*] All done\n");
	}
	
	if(ctx.extract == True && partition_primary.DIFI.flags[0] == 0x0){//TWO Versions of ONE FILE	
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
	
	if(ctx.titledb_read == True){
		int db_mode = Normal;
		if(ctx.listdb == True)
			db_mode = ByTID;
		if(ProcessTitleDB(extdataimg, db_mode,(header.DIFF.active_table_offset + partition_primary.IVFC.level_4_fs_relative_offset + partition_primary.DPFS.ivfc_offset)) != 0){
			if(partition_primary.DIFI.flags[0] == 0x0){
				if(ProcessTitleDB(extdataimg, db_mode,(header.DIFF.active_table_offset + partition_primary.DPFS.ivfc_length + partition_secondary.DPFS.ivfc_offset + partition_primary.IVFC.level_4_fs_relative_offset)) != 0){
					printf("[!] %s is Corrupt, or is not a Title Database ExtData Image\n",ctx.input_extdata);
				}
			}
			else
				printf("[!] %s is Corrupt, or is not a Title Database ExtData Image\n",ctx.input_extdata);
		}
	}
	**/
}

void app_title(void)
{
	printf("CTR_Toolkit - ExtData Tool\n");
	printf("Version %d.%d (C) 3DSGuy 2013\n",MAJOR,MINOR);
}

void help(char *app_name)
{
	printf("\nUsage: %s [options] <extdata image/directory>\n", app_name);
	putchar('\n');
	printf("OPTIONS                 Possible Values       Explanation\n");
	//printf(" -v, --verbose                                Enable verbose output.\n");
	printf(" -m, --mode             IMG/FS                Specify mode, Single Image or ExtData FS\n");
	printf(" -h, --help                                   Print this help.\n");
  	printf(" -i, --info                                   Display ExtData Info.\n");
	printf(" -x, --extract          Out-dir/Out-file      Extract Data from an ExtData Image or FS.\n");
	printf("ExtData FS Options:\n");
	printf(" -v, --viewFS                                 Display ExtData FS.\n");
	printf("Database ExtData Options:\n");
	printf(" -t, --titledb                                Display Data in Title Database\n");
	printf(" -l, --listdb                                 Generate a Title List from TDB(use with '-t' option)\n");
}