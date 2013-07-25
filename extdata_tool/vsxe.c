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
	char *path = malloc(IO_PATH_LEN);
	for(int i = 1; i < vsxe->foldercount; i++){
		memset(path,0,IO_PATH_LEN);
		sprintf(path,"%s",ctx->output);
		Return_Dir_Path(vsxe,i,path,ctx->platform);
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
	char *inpath = malloc(IO_PATH_LEN);
	char *outpath = malloc(IO_PATH_LEN);
	for(u32 i = 2; i < vsxe->filecount; i++){
		memset(inpath,0x0,IO_PATH_LEN);
		memset(outpath,0x0,IO_PATH_LEN);
		sprintf(inpath,"%s%c%08x.dec",ctx->input,ctx->platform,i);
		sprintf(outpath,"%s%c",ctx->output,ctx->platform);
		Return_ExtData_Mount_Path(vsxe,i,outpath,ctx->platform);
		
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
		if(ExportExdataImagetoFile(extdata,outfile) != 0){
			printf("[!] Failed to Extract '%s' to '%s'\n",inpath,outpath);
			return 1;
		}
		fclose(extdata);
		fclose(outfile);
	}
	free(inpath);
	free(outpath);
	return 0;
}

int ExportExdataImagetoFile(FILE *extdata, FILE *outfile)
{
	EXTDATA_CONTEXT ctx;
	
	if(GetExtDataContext(&ctx,extdata) != 0)
		return 1;
	
	if(ctx.partition[primary].DIFI.flags[0] != 1)
		return UNEXPECTED_MULTIPLE_DATA_IN_EXTDATA;
	
	u64 offset = ctx.partition[primary].DIFI.data_partition_offset + ctx.partition[primary].DPFS.ivfc_offset;
	u64 size = ctx.partition[primary].IVFC.level_4_fs_size;
	
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
	vsxe->offset = (ctx->data.header.DIFF.active_table_offset + ctx->data.partition[primary].IVFC.level_4_fs_relative_offset + ctx->data.partition[primary].DPFS.ivfc_offset);
	
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
	char *path = malloc(EXTDATA_FS_MAX_PATH_LEN);
	for(u32 i = 2; i < ctx->filecount; i++){
		memset(path,0x0,EXTDATA_FS_MAX_PATH_LEN);
		sprintf(path,"/");
		Return_ExtData_Mount_Path(ctx,i,path,UNIX);
		printf(" Image %08x is mounted at: '%s'\n",i,path);
	}
	free(path);
	/**
	printf("Data Table Info:\n");
	for(int i = 1; i < 40; i += 4){
		//printf("%03d : %08x %08x %08x %08x\n",i,u8_to_u32(ctx->header.table.unk0[i],LE));
		for(int j = 0; j < 4; j++)
			printf(" %08x",u8_to_u32(ctx->header.table.unk0[i+j],LE));
		printf("\n");
	}
	**/
}

void Return_Dir_Path(VSXE_CONTEXT *ctx, u32 file_id, char *path, u8 platform)
{
	u8 path_part_count = 0;
	u8 present_dir = u8_to_u32(ctx->folders[file_id].parent_folder_index,LE);
	while(present_dir > 1){
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

	sprintf(path,"%s%c",path,platform);
	
	for(int i = 1; i < path_part_count + 1; i++){
		u8 folder_id = folderlocation[i];	
		sprintf(path,"%s%s%c",path,ctx->folders[folder_id].filename,platform);
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
		sprintf(path,"%s%s%c",path,ctx->folders[folder_id].filename,platform);
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
