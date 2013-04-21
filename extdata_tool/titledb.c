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

int process_title_database(FILE *tdb, u64 offset)
{

	//TODO Establish a proper DB context
	
	
	//DB_HEADER_CONTEXT ctx = process_db_header(FILE *tdb, u64 offset);
	//if (ctx.result != 0)
	//	return ctx.result;
	TEMPTDB_STRUCT tdb_header;
	memset(&tdb_header, 0x0, sizeof(TEMPTDB_STRUCT));
	
	// Reading Title Database file header
	fseek(tdb,offset,SEEK_SET);
	fread(&tdb_header,sizeof(TEMPTDB_STRUCT),1,tdb);
	
	u8 database_type = tdb_magic_check(tdb_header.magic);
	
	if(database_type == INVALID){
		printf("[!] This Database is corrupt\n");
		return DB_CORRUPT;
	}
	
	// Checking embedded BDRI Magic
	if(tdb_header.bdri.magic_0 != BDRI_MAGIC_0 || tdb_header.bdri.magic_1 != BDRI_MAGIC_1){
		printf("[!] Embedded BDRI section is Missing or Corrupt\n");
		return BDRI_CORRUPT;
	}
	
	// Reading title entry table header
	fseek(tdb,(offset + PREHEADER_SIZE + tdb_header.bdri.title_table_offset),SEEK_SET);
	TITLE_TABLE_HEADER title_table_header;
	memset(&title_table_header,0x0,sizeof(title_table_header));
	fread(&title_table_header,sizeof(title_table_header),1,tdb);
	
	// Printing Information
	print_db_header(tdb_header,database_type);
	print_index_entry_header(title_table_header);
	
	
	
	printf("[+] Title List\n");
	TITLE_INDEX_ENTRY_STRUCT title_entry;
	TITLE_INFO_ENTRY_STRUCT title_info;
	for(int i = 0; i < title_table_header.entry_count; i++){
		memset(&title_entry,0x0,sizeof(title_entry));
		memset(&title_info,0x0,sizeof(title_info));
		fseek(tdb,(offset + PREHEADER_SIZE + tdb_header.bdri.title_table_offset + sizeof(title_table_header) + (sizeof(TITLE_INDEX_ENTRY_STRUCT)*i)),SEEK_SET);
		fread(&title_entry,sizeof(title_entry),1,tdb);
		if(title_entry.active_entry == TRUE){
			//Getting Title Info
			fseek(tdb,(offset + PREHEADER_SIZE + tdb_header.bdri.title_table_offset + (title_entry.title_info_offset_X*title_entry.title_info_offset_Y)),SEEK_SET);
			fread(&title_info,sizeof(title_info),1,tdb);
			printf("[%d]:\n",i);
			print_title_data(title_entry,title_info);
		}
	}
	printf("[*] Done\n");
	return 0;
}

void print_db_header(TEMPTDB_STRUCT tdb_header, u8 database_type)
{
	printf("[+] Title Data Base Header\n");
	printf("  Database Type:             ");
	switch(database_type){
		case(NANDTDB):
			printf("NAND Title Database\n");
			break;
		case(NANDIDB):
			printf("NAND Import Database\n");
			break;
		case(TEMPTDB):
			printf("SD Card Title Database\n");
			break;
		case(TEMPIDB):
			printf("NAND DLP Child Temporary Database\n");
			break;
	}
	//BDRI appears to be similar to SAVE paritions. This is just temporary for now, and appears to be partly wrong :/
	printf("  Data Offset:               %x\n",tdb_header.bdri.dataoffset);
	printf("  Database Size:             %x\n",(tdb_header.bdri.database_size*tdb_header.bdri.database_media_size));
	printf("  Folder Map Offset:         %x\n",PREHEADER_SIZE + tdb_header.bdri.FolderMap_Offset);
	printf("  Folder Map Size:           %x\n",(tdb_header.bdri.FolderMap_Size*tdb_header.bdri.FolderMap_Media_Size));
	printf("  File Map Offset:           %x\n",PREHEADER_SIZE + tdb_header.bdri.FileMap_Offset);
	printf("  File Map Size:             %x\n",(tdb_header.bdri.FileMap_Size*tdb_header.bdri.FileMap_Media_Size));
	printf("  Block Map Offset:          %x\n",PREHEADER_SIZE + tdb_header.bdri.BlockMap_Offset);
	printf("  Block Map Size:            %x\n",(tdb_header.bdri.BlockMap_Size*tdb_header.bdri.BlockMap_Media_Size));
	printf("  Entry Table Offset:        %x\n",PREHEADER_SIZE + tdb_header.bdri.title_table_offset);
	printf("  Entry Table Size:          %x\n",(tdb_header.bdri.Entry_Table_Size*tdb_header.bdri.Entry_Table_Media_Size));
	printf("  Folder Table Offset:       %x\n",PREHEADER_SIZE + tdb_header.bdri.FolderTable_Offset);
	//printf("  Folder Table Size:         %x\n",(tdb_header.bdri.FolderMap_Size*tdb_header.bdri.FolderMap_Media_Size));
	printf("  Info Table Offset:         %x\n",PREHEADER_SIZE + tdb_header.bdri.FolderMap_Offset);
	//printf("  Info Table Size:           %x\n",(tdb_header.bdri.FolderMap_Size*tdb_header.bdri.FolderMap_Media_Size));
	/**
	printf("  Title Entry Table Offset:  %x\n",(PREHEADER_SIZE + tdb_header.bdri.title_table_offset));
	printf("  Title Info Table Offset:   %x\n",(PREHEADER_SIZE + tdb_header.bdri.title_table_offset + tdb_header.bdri.title_info_table_offset_X*tdb_header.bdri.title_info_table_offset_Y));
	**/
}

void print_index_entry_header(TITLE_TABLE_HEADER title_table_header)
{
	printf("[+] Title Entry Table Header\n");
	printf("  Unknown_0:                 %d\n",title_table_header.unknown_0);
	printf("  Unknown_1:                 %d\n",title_table_header.unknown_1);
	printf("  Number of Titles:          %d\n",title_table_header.entry_count);//Can give weird values, perhaps checking all slots is better?
	printf("  MAX Number of Titles:      %d\n",title_table_header.max_entry_count);
	printf("  Unknown_2:                 %x\n",title_table_header.unknown_2);
}

void print_title_data(TITLE_INDEX_ENTRY_STRUCT title_entry, TITLE_INFO_ENTRY_STRUCT title_info)
{
	printf(" > Title Info Details\n");
	print_title_index_entry(title_entry);
	printf(" > Title Entry Details\n");
	print_title_info_entry(title_info);
}

void print_title_index_entry(TITLE_INDEX_ENTRY_STRUCT title_entry)
{
	printf("  Title ID:                  "); u8_hex_print_le(title_entry.title_id, 8); printf("\n");
	//printf("  Active Entry:              %08x\n",title_entry.active_entry);
	printf("  Unknown_1:                 %08x\n",title_entry.unknown_0);
	printf("  Title Index:               %d\n",title_entry.index);
	printf("  Unknown_2:                 %08x\n",title_entry.unknown_2);
	printf("  Title Info Offset X:       %08x\n",title_entry.title_info_offset_X);
	printf("  Title Info Offset Y:       %08x\n",title_entry.title_info_offset_Y);
	printf("  Unknown_5:                 %08x\n",title_entry.unknown_5);			
	printf("  Unknown_6:                 %08x\n",title_entry.unknown_6);
	printf("  Unknown_7:                 %08x\n",title_entry.unknown_7);
}

void print_title_info_entry(TITLE_INFO_ENTRY_STRUCT title_info)
{
	printf("  Product Code:              "); print_product_code(title_info.product_code); putchar('\n');
	printf("  Title Size:                "); u8_hex_print_le(title_info.title_size, 8); putchar('\n');
	printf("  Title Type:                %x\n",title_info.title_type);
	printf("  Title Version:             v%d\n",title_info.title_version);
	printf("  Flags_0:                   "); u8_hex_print_be(title_info.flags_0,0x4);putchar('\n');
	printf("   > Manual:                 %s\n",title_info.flags_0[0]? "YES" : "NO");
	//printf("   > UNK_1:                    %s\n",title_info.flags_0[1]? "YES" : "NO");
	//printf("   > UNK_2:                    %s\n",title_info.flags_0[2]? "YES" : "NO");
	//printf("   > UNK_3:                    %s\n",title_info.flags_0[3]? "YES" : "NO");
	printf("  TMD Content ID:            %08x\n",title_info.tmd_file_id);
	printf("  CMD Content ID:            %08x\n",title_info.cmd_file_id);
	printf("  Flags_1:                   "); u8_hex_print_be(title_info.flags_1,0x4);putchar('\n');
	printf("   > SD Save Data:           %s\n",title_info.flags_1[0]? "YES" : "NO");
	//printf("   > UNK_1:                    %s\n",title_info.flags_1[1]? "YES" : "NO");
	//printf("   > UNK_2:                    %s\n",title_info.flags_1[2]? "YES" : "NO");
	//printf("   > UNK_3:                    %s\n",title_info.flags_1[3]? "YES" : "NO");
	printf("  ExtdataID low:             %08x\n",title_info.extdata_id);
	printf("  Flags_2:                   "); u8_hex_print_be(title_info.flags_2,0x8);putchar('\n');
	printf("   > DSiWare Related:        %s\n",title_info.flags_2[0]? "YES" : "NO");//DSiWare Related, Export Flag?
	//printf("   > UNK_1:                    %s\n",title_info.flags_3[1]? "YES" : "NO");
	//printf("   > UNK_2:                    %s\n",title_info.flags_3[2]? "YES" : "NO");
	//printf("   > UNK_3:                    %s\n",title_info.flags_3[3]? "YES" : "NO");
	printf("   > 'Application' TID:      %s\n",title_info.flags_2[4]? "YES" : "NO");//Only present on titles with an Application TID
	printf("   > DSiWare Related:        %s\n",title_info.flags_2[5]? "YES" : "NO");//DSiWare Related, Export Flag?
	//printf("   > UNK_6:                    %s\n",title_info.flags_4[2]? "YES" : "NO");
	//printf("   > UNK_7:                    %s\n",title_info.flags_4[3]? "YES" : "NO");
	printf("  Unknown:                   %08x\n",title_info.unknown_6);
}

u8 tdb_magic_check(u32 *magic)
{
	// Checking Type of Database
	if(magic[0] == NANDTDB_MAGIC_0 && magic[1] == NANDTDB_MAGIC_1)
		return NANDTDB;
	else if(magic[0] == NANDIDB_MAGIC_0 && magic[1] == NANDIDB_MAGIC_1)
		return NANDIDB;	
	else if(magic[0] == TEMPTDB_MAGIC_0 && magic[1] == TEMPTDB_MAGIC_1)
		return TEMPTDB;
	else if(magic[0] == TEMPIDB_MAGIC_0 && magic[1] == TEMPIDB_MAGIC_1)
		return TEMPIDB;
	else
		return INVALID;
}