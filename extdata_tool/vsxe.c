#include "lib.h"
#include "vsxe.h"

int ProcessExtData_FS(INPUT_CONTEXT *ctx)
{
	VSXE_CONTEXT *vsxe = malloc(sizeof(VSXE_CONTEXT));
	memset(vsxe,0,sizeof(VSXE_CONTEXT));
	if(GetVSXEContext(vsxe,ctx) !=0){
		free(vsxe);
		return 1;
	}
	
	if(ctx->fs_info == True)
		PrintVSXE_FS_INFO(vsxe);
	
	if(ctx->extract == True){
		SetupOutputFS(vsxe,ctx);
		if(WriteExtDataFiles(vsxe,ctx) != 0){
			printf("[!] Failed to Extract ExtData images\n");
			free(vsxe);
			return 1;
		}
	}
	free(vsxe);
	return 0;
}

void SetupOutputFS(VSXE_CONTEXT *vsxe, INPUT_CONTEXT *ctx)
{
	//chdir(ctx->output);
	memcpy(vsxe->folders[1].filename,ctx->output,16);
	char *path = malloc(1000);
	for(int i = 1; i < vsxe->foldercount; i++){
		memset(path,0,1000);
		sprintf(path,"%s",ctx->cwd);
#ifdef _WIN32
		Return_Dir_Path(vsxe,i,path,WIN_32);
#else
		Return_Dir_Path(vsxe,i,path,UNIX);
#endif
		//printf("%s\n",path);
		chdir(path);
		for(int j = 2; j < vsxe->foldercount; j++){
			if(u8_to_u32(vsxe->folders[j].parent_folder_index,LE) == i){
				//printf(" %s\n",vsxe->folders[j].filename);
				makedir(vsxe->folders[j].filename);
			}
		}
	}
}

int WriteExtDataFiles(VSXE_CONTEXT *vsxe, INPUT_CONTEXT *ctx)
{
	chdir(ctx->cwd);
	char *inpath = malloc(1000);
	char *outpath = malloc(1000);
	for(u32 i = 2; i < vsxe->filecount; i++){
		memset(inpath,0x0,1000);
		memset(outpath,0x0,1000);
		u8 inputlen = strlen(ctx->input);
		char director = 0;
		int platform = 0;
#ifdef _WIN32
		platform = WIN_32;
		director = 0x5C;
#else
		platform = UNIX;
		director = 0x2F;
#endif
		if(ctx->input[inputlen - 1] != director) 
			sprintf(inpath,"%s%c%08x.dec",ctx->input,director,i);
		else
			sprintf(inpath,"%s%08x.dec",ctx->input,i);

		if(ctx->output[inputlen - 1] != director) 
			sprintf(outpath,"%s%c",ctx->output,director);
		else
			sprintf(outpath,"%s",ctx->output);
		Return_ExtData_Mount_Path(vsxe,i,outpath,platform);
		
		FILE *extdata = fopen(inpath,"rb");
		if(extdata == NULL){
			free(inpath);
			free(outpath);
			printf("[!] Failed to open %s\n",inpath);
			return 1;
		}
		FILE *outfile = fopen(outpath,"wb");
		if(outfile == NULL){
			free(inpath);
			free(outpath);
			printf("[!] Failed to create %s\n",outpath);
			return 1;
		}
		
		//printf("File: '%s' is extracted to '%s'\n",inpath,outpath);
		if(ExportExdataImagetoFile(extdata,outfile) != 0){
			printf("Failed to Extract '%s' to '%s'\n",inpath,outpath);
			return 1;
		}
		fclose(extdata);
		fclose(outfile);
	}
	return 0;
}

int ExportExdataImagetoFile(FILE *extdata, FILE *outfile)
{
	EXTDATA_HEADER_CONTEXT header;
	PARTITION_STRUCT partition[2];
	
	memset(&header,0,sizeof(header));
	
	//Reading DIFF
	fseek(extdata, 0x100, SEEK_SET);
	fread(&header.DIFF,sizeof(DIFF_STRUCT),1,extdata);
	if(header.DIFF.magic_0 == DISA_MAGIC){
		printf("[!] This is a SaveData Image\n");
		return DIFF_CORRUPT;
	}
	else if(header.DIFF.magic_0 != DIFF_MAGIC_0 || header.DIFF.magic_1 != DIFF_MAGIC_1){
		printf("[!] DIFF Header Corrupt\n");
		return DIFF_CORRUPT;
	}
	//Reading ExtData HMAC
	fseek(extdata, 0x00, SEEK_SET);
	fread(header.AES_MAC,0x10,1,extdata);
	
	if(header.DIFF.primary_partition_offset != 0){
		partition[primary] = get_extdata_partition_header(header.DIFF.primary_partition_offset, header.DIFF.active_table_offset, extdata);
		if(partition[primary].valid != 0){
			printf("[!] Primary DIFI Partition Corrupt\n");
			return DIFI_CORRUPT;
		}
	}
	else{
		printf("[!] ExtData Image is Empty\n");
		return DIFI_CORRUPT;
	}
	
	if(header.DIFF.secondary_partition_offset != 0){
		partition[secondary] = get_extdata_partition_header(header.DIFF.secondary_partition_offset, header.DIFF.active_table_offset, extdata);
		if(partition[secondary].valid != 0){
			printf("[!] Secondary DIFI Partition Corrupt\n");
			return DIFI_CORRUPT;
		}
	}
	else{
		printf("[+] No Secondary Partition Present\n");
	}
	
	if(partition[primary].DIFI.flags[0] != 1)
		return 1;
	
	u64 offset = partition[primary].DIFI.data_partition_offset + partition[primary].DPFS.ivfc_offset;
	u64 size = partition[primary].IVFC.level_4_fs_size;
	
	u8 *buffer = malloc(size);
	
	fseek(extdata,offset,SEEK_SET);
	fseek(outfile,0,SEEK_SET);
	fread(buffer,size,1,extdata);
	fwrite(buffer,size,1,outfile);
	
	free(buffer);
	return 0;
}



int GetVSXEContext(VSXE_CONTEXT *vsxe, INPUT_CONTEXT *ctx)
{
	vsxe->extdata = ctx->extdataimg;
	vsxe->offset = (ctx->header.DIFF.active_table_offset + ctx->partition[primary].IVFC.level_4_fs_relative_offset + ctx->partition[primary].DPFS.ivfc_offset);
	
	fseek(vsxe->extdata,vsxe->offset,SEEK_SET);
	fread(&vsxe->header,sizeof(vsxe_header),1,vsxe->extdata);
	
	if(VerifyVSXE(vsxe) != 0){
		printf("[!] Is not a VSXE File Table\n");
		return 1;
	}
	
	vsxe->folder_table_offset = 0x1000 + vsxe->offset;
	vsxe->file_table_offset = 0x2000 + vsxe->offset;
	
	InterpreteFolderTable(vsxe);
	InterpreteFileTable(vsxe);
	
	return 0;
}

void PrintVSXE_FS_INFO(VSXE_CONTEXT *ctx)
{
	printf("Last Extdata Mount Details:\n");
	u32 last_extdata_id = u8_to_u32(ctx->header.last_used_file_extdata_id,LE);
	printf(" ExtData Image ID:     %08x\n",last_extdata_id);
	printf(" ExtData Mount Path:   %s\n",ctx->header.last_used_file); 
	
	printf("ExtData Image Mount locations\n");
	char *path = malloc(0x100);
	for(u32 i = 2; i < ctx->filecount; i++){
		memset(path,0x0,0x100);
		sprintf(path,"/");
		Return_ExtData_Mount_Path(ctx,i,path,UNIX);
		printf(" Image %08x is mounted at: '%s'\n",i,path);
	}
	free(path);
}

void Return_Dir_Path(VSXE_CONTEXT *ctx, u32 file_id, char *path, u8 platform)
{
	u8 path_part_count = 0;
	u8 present_dir = u8_to_u32(ctx->folders[file_id].parent_folder_index,LE);
	while(present_dir > 0){
		path_part_count++;
		present_dir = u8_to_u32(ctx->folders[present_dir].parent_folder_index,LE);
	}
	u8 folderlocation[path_part_count];
	present_dir = u8_to_u32(ctx->folders[file_id].parent_folder_index,LE);
	folderlocation[path_part_count] = present_dir;
	for(int i = path_part_count - 1; present_dir > 1; i--){
		present_dir = u8_to_u32(ctx->folders[present_dir].parent_folder_index,LE);
		folderlocation[i] = present_dir;
	}
	
	u8 divider = 0;
	switch(platform){
		case(WIN_32): divider = 0x5C; break;
		default: divider = 0x2F; break;
	}
	sprintf(path,"%s%c",path,divider);
	
	for(int i = 1; i < path_part_count + 1; i++){
		u8 folder_id = folderlocation[i];	
		sprintf(path,"%s%s%c",path,ctx->folders[folder_id].filename,divider);
	}
	sprintf(path,"%s%s",path,ctx->folders[file_id].filename);	
}

void Return_ExtData_Mount_Path(VSXE_CONTEXT *ctx, u32 file_id, char *path, u8 platform)
{
	u8 path_part_count = 0;
	u8 present_dir = u8_to_u32(ctx->files[file_id].parent_folder_index,LE);
	while(present_dir > 1){
		path_part_count++;
		present_dir = u8_to_u32(ctx->folders[present_dir].parent_folder_index,LE);
	}
	u8 folderlocation[path_part_count];
	present_dir = u8_to_u32(ctx->files[file_id].parent_folder_index,LE);
	folderlocation[path_part_count] = present_dir;
	for(int i = path_part_count - 1; present_dir > 1; i--){
		present_dir = u8_to_u32(ctx->folders[present_dir].parent_folder_index,LE);
		folderlocation[i] = present_dir;
	}
	for(int i = 1; i < path_part_count + 1; i++){
		u8 folder_id = folderlocation[i];
		u8 divider = 0;
		switch(platform){
			case(WIN_32): divider = 0x5C; break;
			default: divider = 0x2F; break;
		}
		sprintf(path,"%s%s%c",path,ctx->folders[folder_id].filename,divider);
	}
	sprintf(path,"%s%s",path,ctx->files[file_id].filename);
	
	//memcpy(
}

int VerifyVSXE(VSXE_CONTEXT *ctx)
{
	if(u8_to_u32(ctx->header.magic,BE) != vsxe_magic || u8_to_u32(ctx->header.magic_id,LE) != vsxe_magic_id)
		return 1;
	return 0;
}

void InterpreteFolderTable(VSXE_CONTEXT *ctx)
{
	fseek(ctx->extdata,ctx->folder_table_offset,SEEK_SET);
	folder_table_header header;
	fread(&header,sizeof(folder_table_header),1,ctx->extdata);
	ctx->foldercount = u8_to_u32(header.used_slots,LE); // -1
	ctx->folders = malloc(sizeof(folder_entry)*ctx->foldercount);
	memset(ctx->folders,0x0,(sizeof(folder_entry)*ctx->foldercount));
	
	//memcpy(ctx->folders[1].filename,"root:",5);
	
	for(int i = 2; i < ctx->foldercount; i++){
		fread(&ctx->folders[i],sizeof(folder_entry),1,ctx->extdata);
	}
	return;
}

void InterpreteFileTable(VSXE_CONTEXT *ctx)
{
	fseek(ctx->extdata,ctx->file_table_offset,SEEK_SET);
	file_table_header header;
	fread(&header,sizeof(file_table_header),1,ctx->extdata);
	ctx->filecount = u8_to_u32(header.used_slots,LE)+1; // -1
	ctx->files = malloc(sizeof(file_entry)*ctx->filecount);
	memset(ctx->files,0x0,(sizeof(file_entry)*ctx->filecount));
	for(int i = 2; i < ctx->filecount; i++){
		fread(&ctx->files[i],sizeof(file_entry),1,ctx->extdata);
	}
	return;
}

/**
void read_vsxe(FILE *file, u32 offset)
{
	VSXE_CONTEXT ctx;
	ctx.extdata = file;
	ctx.offset = offset;
	
	fseek(ctx.extdata,ctx.offset,SEEK_SET);
	fread(&ctx.header,sizeof(vsxe_header),1,ctx.extdata);
	
	if(VerifyVSXE(&ctx) != 0){
		printf("[!] Is not a VSXE File Table\n");
		return;
	}
	
	ctx.folder_table_offset = 0x1000 + ctx.offset;
	ctx.file_table_offset = 0x2000 + ctx.offset;
	
	InterpreteFolderTable(&ctx);
	InterpreteFileTable(&ctx);
	
	printf("Last Extdata Mount Details:\n");
	u32 last_extdata_id = u8_to_u32(ctx.header.last_used_file_extdata_id,LE);
	printf(" ExtData Image ID:     %08x\n",last_extdata_id);
	printf(" ExtData Mount Path:   %s\n",ctx.header.last_used_file); 
	
	printf("ExtData Image Mount locations\n");
	char *path = malloc(0x100);
	for(u32 i = 2; i < ctx.filecount; i++){
		memset(path,0x0,0x100);
		sprintf(path,"/");
		Return_ExtData_Mount_Path(&ctx,i,path,UNIX);
		printf(" Image %08x is mounted at: '%s'\n",i,path);
	}
	free(path);
	
	
	free(ctx.folders);
	free(ctx.files);

	fseek(ctx.extdata,ctx.offset+0x1000+(sizeof(folder_entry)*2),SEEK_SET);
	printf("\nFolders\n");
	for(int i = 0; i < ctx.foldercount - 2; i++){
		folder_entry tmp;
		fread(&tmp,sizeof(folder_entry),1,ctx.extdata);
		printf("------------------------------------\n\n");
		printf("Parent Folder Index:   %d\n",u8_to_u32(tmp.parent_folder_index,LE));
		printf("Folder Name:           %s\n",tmp.filename);
		printf("Folder Index:          %d\n",u8_to_u32(tmp.folder_index,LE));
		printf("UNK1:                  %d\n",u8_to_u32(tmp.unk1,LE));
		printf("Last File Index:       %d\n",u8_to_u32(tmp.last_file_index,LE));
		printf("UNK2:                  %d\n",u8_to_u32(tmp.unk2,LE));
		printf("UNK3:                  %d\n",u8_to_u32(tmp.unk3,LE));
		printf("\n------------------------------------\n");
	}
	fseek(ctx.extdata,ctx.offset+0x2000+(sizeof(file_entry)*1),SEEK_SET);
	printf("\nFiles\n");
	for(int i = 0; i < ctx.filecount - 2; i++){
		file_entry tmp;
		fread(&tmp,sizeof(file_entry),1,ctx.extdata);
		printf("------------------------------------\n\n");
		printf("Parent Folder Index:   %d\n",u8_to_u32(tmp.parent_folder_index,LE));
		printf("File Name:             %s\n",tmp.filename);
		printf("File Index:            %d\n",u8_to_u32(tmp.index,LE));
		printf("UNK1:                  %x\n",u8_to_u32(tmp.unk1,LE));
		printf("Block Offset:          %x\n",u8_to_u32(tmp.block_offset,LE));
		printf("File Size:             %llx\n",u8_to_u64(tmp.file_size,LE));
		printf("UNK2:                  %x\n",u8_to_u32(tmp.unk2,LE));
		printf("UNK3:                  %x\n",u8_to_u32(tmp.unk3,LE));
		printf("\n------------------------------------\n");
	}
}
**/