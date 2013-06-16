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
#include "titledb.h"

const char DB_MAGIC[4][8] = {"NANDTDB","NANDIDB","TEMPTDB","TEMPIDB"};
const u8 BDRI_MAGIC[5] = {"BDRI"};
const char TitlePlatformString[2][5] = {"CTR","TWL"};
const char TitleTypeString[2][8] = {"","System "};
const char TitleAppTypeString[9][20] = {"Application","DLP Child","Demo","Add-on Content","DLC Content","Applet","Module","Data Archive","Firmware"};

int ProcessTitleDB(FILE *tdb, int Mode, u64 offset)
{

	//TODO Establish a proper DB context
	DATABASE_CONTEXT *ctx = malloc(sizeof(DATABASE_CONTEXT));
	memset(ctx,0x0,sizeof(DATABASE_CONTEXT));
	ctx->db = tdb;
	ctx->core_data.db_offset = offset;
	
	if(GetDB_Header(ctx) != ValidDB){
		printf("[!] Error\n");
		return CorruptDB;
	}
	
	ProcessDB_Header(ctx);
	
	if(GetEntry_Header(ctx) != ValidDB){
		printf("[!] Error\n");
		return CorruptDB;
	}

	PopulateDatabase(ctx);
	
	if(Mode == Normal)
		PrintDatabase(ctx);
	if(Mode == ByTID)
		ListDatabase(ctx);
	
	if(ctx->database.BufferAllocated == TRUE){
		free(ctx->database.TitleData);
	}
	return 0;
}

int GetDB_Header(DATABASE_CONTEXT *ctx)
{
	fseek(ctx->db,ctx->core_data.db_offset,SEEK_SET);
	char db_type_magic[0x8];
	fread(db_type_magic,sizeof(db_type_magic),1,ctx->db);
	GetDB_Type(ctx,db_type_magic);
	if(ctx->core_data.db_type == Invalid){
		return CorruptDB;
	}
	fseek(ctx->db,ctx->core_data.db_offset + 0x80,SEEK_SET);
	fread(&ctx->header,sizeof(BDRI_STRUCT),1,ctx->db);
	if(CheckDB_Header(ctx) == Invalid){
		printf("[!] Embedded BDRI section is Missing or Corrupt\n");
		return CorruptDB;
	}
	return ValidDB;
}

void GetDB_Type(DATABASE_CONTEXT *ctx, char db_type_magic[8])
{
	for(int i = 0; i < 4; i++){
		if(memcmp(DB_MAGIC[i],db_type_magic,7) == 0){
			ctx->core_data.db_type = i + 1;
			return;
		}
	}
	ctx->core_data.db_type = Invalid;
}

int CheckDB_Header(DATABASE_CONTEXT *ctx)
{
	if(memcmp(ctx->header.magic_0,BDRI_MAGIC,4) != 0 || u8_to_u32(ctx->header.magic_1,BIG_ENDIAN) != 0x300)
		return Invalid;
	return Valid;
}

void ProcessDB_Header(DATABASE_CONTEXT *ctx)
{
	ctx->core_data.EntryTableOffset = ctx->core_data.db_offset + 0x80 + u8_to_u64(ctx->header.Entry_Table_Offset,LITTLE_ENDIAN);
	//ctx->core_data.EntryTableSize = (u8_to_u32(ctx->header.Entry_Table_Size,LITTLE_ENDIAN) * u8_to_u32(ctx->header.Entry_Table_Media_Size,LITTLE_ENDIAN));
	//ctx->core_data.InfoTableOffset = ctx->core_data.EntryTableOffset + (u8_to_u32(ctx->header.Info_Table_Offset,LITTLE_ENDIAN) * u8_to_u32(ctx->header.Info_Table_Offset_Medias,LITTLE_ENDIAN));
	//printf("Entry Table Offset: 0x%x\n",ctx->core_data.EntryTableOffset);
	//printf("Entry Table Size: 0x%x\n",ctx->core_data.EntryTableSize);
	//printf("Info Table Offset: 0x%x\n",ctx->core_data.InfoTableOffset);
}

int GetEntry_Header(DATABASE_CONTEXT *ctx)
{
	fseek(ctx->db,ctx->core_data.EntryTableOffset,SEEK_SET);
	fread(&ctx->entry_table_header,sizeof(ENTRY_TABLE_HEADER),1,ctx->db);
	if(u8_to_u32(ctx->entry_table_header.magic_0,LITTLE_ENDIAN) != 0x2 || u8_to_u32(ctx->entry_table_header.magic_1,LITTLE_ENDIAN) != 0x3 ){
		printf("[!] Embedded Title Entry Table section is Missing or Corrupt\n");
		return CorruptDB;
	}
	ctx->database.TitleCount = u8_to_u32(ctx->entry_table_header.entry_count,LITTLE_ENDIAN);
	ctx->database.MaxCount = u8_to_u32(ctx->entry_table_header.max_entry_count,LITTLE_ENDIAN);
	return ValidDB;
}

void PopulateDatabase(DATABASE_CONTEXT *ctx)
{
	ctx->database.BufferAllocated = TRUE;
	ctx->database.TitleData = malloc(sizeof(TITLE_CONTEXT)*ctx->database.MaxCount);
	memset(ctx->database.TitleData,0x0,(sizeof(TITLE_CONTEXT)*ctx->database.MaxCount));
	
	for(u32 i = 0; i < ctx->database.MaxCount; i++){
		u32 entry_offset = (ctx->core_data.EntryTableOffset + sizeof(ENTRY_TABLE_HEADER) + (i * sizeof(TITLE_INDEX_ENTRY_STRUCT)));
		TITLE_INDEX_ENTRY_STRUCT temp;
		fseek(ctx->db,entry_offset,SEEK_SET);
		fread(&temp,sizeof(TITLE_INDEX_ENTRY_STRUCT),1,ctx->db);
		if(u8_to_u32(temp.Active_Entry,LITTLE_ENDIAN) == TRUE){
			u32 Index = u8_to_u32(temp.Index,LITTLE_ENDIAN);
			u32 info_offset = (ctx->core_data.EntryTableOffset + (u8_to_u32(temp.Title_Info_Offset,LITTLE_ENDIAN) * u8_to_u32(temp.Title_Info_Offset_Media,LITTLE_ENDIAN)));
			StoreTitleEntry(&ctx->database.TitleData[Index],entry_offset,info_offset,ctx->db);
		}
	}
}

void StoreTitleEntry(TITLE_CONTEXT *TitleData, u32 entry_offset, u32 info_offset, FILE *db)
{
	fseek(db,entry_offset,SEEK_SET);
	fread(&TitleData->index,sizeof(TITLE_INDEX_ENTRY_STRUCT),1,db);
	fseek(db,info_offset,SEEK_SET);
	fread(&TitleData->info,sizeof(TITLE_INFO_ENTRY_STRUCT),1,db);
} 

void PrintDatabase(DATABASE_CONTEXT *ctx)
{
	printf("[+] Database Info:\n");
	printf(" Title Database Type: ");
	switch(ctx->core_data.db_type){
		case NANDTDB : printf("NAND Title Database \n"); break;
		case NANDIDB : printf("NAND Title Import Database \n"); break;
		case TEMPTDB : printf("SDMC Title Database \n"); break;
		case TEMPIDB : printf("NAND DLP Child Temporary Database \n"); break;
		default : printf("Unknown\n"); break;
	}
	printf(" Active Entries:      %d\n",GetValidEntryCount(ctx));
	printf(" Maximum Entries:     %d\n",ctx->database.MaxCount);
	printf("[+] Titles Entries in Database:\n");
	for(u32 i = 0; i < ctx->database.MaxCount; i++){
		PrintTitleData(&ctx->database.TitleData[i]);
	}
}

void PrintTitleData(TITLE_CONTEXT *TitleData)
{
	if(EntryUsed(TitleData) == Invalid){
		printf(" [+]: Unused\n");
		return;
	}
	if(EntryUsed(TitleData) == Invalid){
		printf(" [+]: Invalid\n");
		return;
	}
	PrintTitleIndexData(&TitleData->index);
	PrintTitleInfoData(&TitleData->info);
}

int EntryUsed(TITLE_CONTEXT *TitleData)
{
	if(u8_to_u32(TitleData->index.Active_Entry,LITTLE_ENDIAN) != TRUE)
		return Invalid;
	return Valid;
}

int EntryValid(TITLE_CONTEXT *TitleData)
{
	if(u8_to_u32(TitleData->info.Title_Type,LITTLE_ENDIAN) == 0x0)
		return Invalid;
	return Valid;
}

void PrintTitleIndexData(TITLE_INDEX_ENTRY_STRUCT *index)
{
	printf(" [+]: %d\n",u8_to_u32(index->Index,LITTLE_ENDIAN));
	printf(" TitleID:                    "); u8_hex_print_le(index->Title_ID, 8); printf("\n");
	GetTitleType(index->Title_ID);
}

void PrintTitleInfoData(TITLE_INFO_ENTRY_STRUCT *info)
{
	printf(" Product Code:               "); print_product_code(info->Product_Code); putchar('\n');
	printf(" Title Type:                 %x\n",u8_to_u32(info->Title_Type,LITTLE_ENDIAN));
	printf(" Title Version:              v%d\n",u8_to_u16(info->Title_Version,LITTLE_ENDIAN));
	printf(" TMD Content ID:             %08x\n",u8_to_u32(info->TMD_File_ID,LITTLE_ENDIAN));
	printf(" CMD Content ID:             %08x\n",u8_to_u32(info->CMD_File_ID,LITTLE_ENDIAN));
	printf(" ExtdataID low:              %08x\n",u8_to_u32(info->ExtData_ID,LITTLE_ENDIAN));
	printf(" Manual:                     %s\n",info->Flags_0[0]? "YES" : "NO");
	printf(" SD Save Data:               %s\n",info->Flags_1[0]? "YES" : "NO");
	printf(" Is DSiWare:                 %s\n",info->Flags_2[0]? "YES" : "NO");
	/**
	printf("  Flags_0:                   "); u8_hex_print_be(info->Flags_0,0x4);putchar('\n');
	//printf("   > Manual:                 %s\n",info->Flags_0[0]? "YES" : "NO");
	printf("   > UNK_1:                    %s\n",info->Flags_0[1]? "YES" : "NO");
	printf("   > UNK_2:                    %s\n",info->Flags_0[2]? "YES" : "NO");
	printf("   > UNK_3:                    %s\n",info->Flags_0[3]? "YES" : "NO");
	//printf("  TMD Content ID:            %08x\n",info->TMD_File_ID);
	//printf("  CMD Content ID:            %08x\n",info->CMD_File_ID);
	printf("  Flags_1:                   "); u8_hex_print_be(info->Flags_1,0x4);putchar('\n');
	//printf("   > SD Save Data:           %s\n",info->Flags_1[0]? "YES" : "NO");
	printf("   > UNK_1:                    %s\n",info->Flags_1[1]? "YES" : "NO");
	printf("   > UNK_2:                    %s\n",info->Flags_1[2]? "YES" : "NO");
	printf("   > UNK_3:                    %s\n",info->Flags_1[3]? "YES" : "NO");
	printf("  ExtdataID low:             %08x\n",info->ExtData_ID);
	printf("  Flags_2:                   "); u8_hex_print_be(info->Flags_2,0x8);putchar('\n');
	//printf("   > DSiWare:                %s\n",info->Flags_2[0]? "YES" : "NO");//DSiWare Related, Export Flag?
	printf("   > UNK_1:                    %s\n",info->Flags_2[1]? "YES" : "NO");
	printf("   > UNK_2:                    %s\n",info->Flags_2[2]? "YES" : "NO");
	printf("   > UNK_3:                    %s\n",info->Flags_2[3]? "YES" : "NO");
	//printf("   > Type:           %s\n",info->Flags_2[4]? "Regular" : "System");
	//printf("   > DSiWare Related:        %s\n",info->Flags_2[5]? "YES" : "NO");//DSiWare Related, Export Flag?
	printf("   > UNK_6:                    %s\n",info->Flags_2[6]? "YES" : "NO");
	printf("   > UNK_7:                    %s\n",info->Flags_2[7]? "YES" : "NO");
	**/
	//printf(" '0xAC' Increment:           %08x\n",info->unknown_6);
}

void GetTitleType(u8 TitleID[8])
{
	u8 FlagBool[16];
	u16 TitleTypeID = u8_to_u16(TitleID+4,LITTLE_ENDIAN);
	resolve_flag_u16(TitleTypeID,FlagBool);
	
	int TitlePlatform_FLAG = Invalid;
	int TitleType_FLAG = Invalid;
	int TitleAppType_FLAG = Application;
	
	if(TitleTypeID == 0){
		TitlePlatform_FLAG = CTR;
		TitleType_FLAG = Regular;
		TitleAppType_FLAG = Application;
		goto print_info;
	}
	
	if(FlagBool[15] == TRUE && FlagBool[14] == FALSE)
		goto twl_content;
	else
		goto ctr_content;
	
	if(TitlePlatform_FLAG == Invalid){
		return;
	}

ctr_content:
	TitlePlatform_FLAG = CTR;
	if(FlagBool[14] == FALSE && FlagBool[15] == FALSE && FlagBool[4] == TRUE)
		TitleType_FLAG = System;
	else
		TitleType_FLAG = Regular;
	
	if(FlagBool[0] == TRUE)
		TitleAppType_FLAG = DLP_Child;
	
	if(FlagBool[1] == TRUE)
		TitleAppType_FLAG = Demo;
		
	if(FlagBool[1] == TRUE && FlagBool[2] == TRUE && FlagBool[3] == TRUE)
		TitleAppType_FLAG = Addon_Content;
	
	if(FlagBool[2] == TRUE && FlagBool[3] == TRUE && FlagBool[7] == TRUE)
		TitleAppType_FLAG = DLC_Content;
	
	if(TitleType_FLAG == System){
		if(FlagBool[5] == TRUE)
			TitleAppType_FLAG = Applet;
		if(FlagBool[5] == TRUE && FlagBool[8] == TRUE)
			TitleAppType_FLAG = Module;
		if(FlagBool[5] == TRUE && FlagBool[8] == TRUE && FlagBool[3] == TRUE)
			TitleAppType_FLAG = Firmware;
	}
	
	if(FlagBool[0] == TRUE && FlagBool[1] == TRUE && FlagBool[2] == FALSE && FlagBool[3] == TRUE)
		TitleAppType_FLAG = Data_Archive;
		
	goto print_info;
	
twl_content:
	TitlePlatform_FLAG = TWL;
	if(FlagBool[0] == TRUE)
		TitleType_FLAG = System;
	else
		TitleType_FLAG = Regular;
	
	if(FlagBool[2] == TRUE)
		TitleAppType_FLAG = Application;
	
	if(FlagBool[1] == TRUE && FlagBool[2] == TRUE)
		TitleAppType_FLAG = Applet;
		
	if(FlagBool[1] == TRUE && FlagBool[2] == TRUE && FlagBool[3] == TRUE)
		TitleAppType_FLAG = Data_Archive;
		
	goto print_info;
	
print_info:
	printf(" Platform:                   %s\n",TitlePlatformString[TitlePlatform_FLAG]);
	printf(" Content Type:               %s%s\n",TitleTypeString[TitleType_FLAG],TitleAppTypeString[TitleAppType_FLAG]);
	if(TitlePlatform_FLAG == TWL){
		printf(" Region Lock:                ");
		switch(TitleID[0]){
			case 0x41 : printf("Region Free\n"); break;
			case 0x45 : printf("America\n"); break;
			case 0x4A : printf("Japan\n"); break;
			case 0x56 : printf("PAL\n"); break;
		}
	}
	if(TitlePlatform_FLAG == CTR && TitleType_FLAG == System && TitleAppType_FLAG != Application){
		printf(" Kernel:                     ");
		switch(TitleID[0]){
			case 0x2 : printf("NATIVE_FIRM\n"); break;
			case 0x3 : printf("SAFE_MODE_FIRM\n"); break;
		}
	}
}

void ListDatabase(DATABASE_CONTEXT *ctx)
{
	u32 ContentCount = GetValidEntryCount(ctx);
	u64 *TitleID_DB = malloc(sizeof(u64)*ContentCount);
	memset(TitleID_DB,0x0,(sizeof(u64)*ContentCount));
	CollectTitleIDs(TitleID_DB,ContentCount,ctx);
	merge_sort(TitleID_DB,ContentCount);
	//SortTitleIDs(TitleID_DB,ContentCount);
	ListTitleIDs(TitleID_DB,ContentCount);
	free(TitleID_DB);
}

u32 GetValidEntryCount(DATABASE_CONTEXT *ctx)
{
	u32 counter = 0;
	for(u32 i = 0; i < ctx->database.MaxCount; i++){
		if(EntryUsed(&ctx->database.TitleData[i]) == Valid && EntryValid(&ctx->database.TitleData[i]) == Valid)
			counter++;
	}
	return counter;
}

void CollectTitleIDs(u64 *TitleID_DB, u32 ContentCount, DATABASE_CONTEXT *ctx)
{
	u32 TID_Count = 0;
	for(u32 i = 0, TID_Count = 0; i < ctx->database.MaxCount, TID_Count < ContentCount; i++){
		if(EntryUsed(&ctx->database.TitleData[i]) == Valid && EntryValid(&ctx->database.TitleData[i]) == Valid){
			TitleID_DB[TID_Count] = ReturnTitleID(&ctx->database.TitleData[i]);
			TID_Count++;
		}
	}
}

u64 ReturnTitleID(TITLE_CONTEXT *TitleData)
{
	return u8_to_u64(TitleData->index.Title_ID,LITTLE_ENDIAN);
}


void ListTitleIDs(u64 *TitleID_DB, u32 ContentCount)
{
	printf("[+] Title Count: %d\n",ContentCount);
	printf("[+] Title List:\n");
	for(u32 i = 0; i < ContentCount; i++){
		printf(" %016llx\n",TitleID_DB[i]);
	}
}

// Adapted from http://rosettacode.org/wiki/Sorting_algorithms/Merge_sort#C

void merge(u64 *left, int l_len, u64 *right, int r_len, u64 *out)
{
	int i, j, k;
	for (i = j = k = 0; i < l_len && j < r_len; )
		out[k++] = left[i] < right[j] ? left[i++] : right[j++];
 
	while (i < l_len) out[k++] = left[i++];
	while (j < r_len) out[k++] = right[j++];
}
 
/* inner recursion of merge sort */
void recur(u64 *buf, u64 *tmp, int len)
{
	int l = len / 2;
	if (len <= 1) return;
 
	/* note that buf and tmp are swapped */
	recur(tmp, buf, l);
	recur(tmp + l, buf + l, len - l);
 
	merge(tmp, l, tmp + l, len - l, buf);
}
 
/* preparation work before recursion */
void merge_sort(u64 *buf, int len)
{
	/* call alloc, copy and free only once */
	u64 *tmp = malloc(sizeof(u64) * len);
	memcpy(tmp, buf, sizeof(u64) * len);
 
	recur(buf, tmp, len);
 
	free(tmp);
}