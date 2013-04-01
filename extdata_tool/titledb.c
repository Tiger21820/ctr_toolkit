/**
Copyright 2013 3DSGuy

This file is part of extdata_tool.

make_banner is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

make_banner is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with make_banner.  If not, see <http://www.gnu.org/licenses/>.
**/
#include "lib.h"
#include "titledb.h"

int process_title_database(FILE *tdb, u64 offset)
{
	TEMPTDB_STRUCT tdb_header;
	memset(&tdb_header, 0x0, sizeof(TEMPTDB_STRUCT));
	
	// Reading Title Database file header
	fseek(tdb,offset,SEEK_SET);
	fread(&tdb_header,sizeof(TEMPTDB_STRUCT),1,tdb);
	
	// Checking TEMPTDB Magic
	if(tdb_header.magic_0 != TEMPTDB_MAGIC_0 || tdb_header.magic_1 != TEMPTDB_MAGIC_1){
		printf("[!] Title Database File is Corrupt\n");
		return TEMPTDB_CORRUPT;
	}
	
	// Checking embedded BDRI Magic
	if(tdb_header.bdri.magic_0 != BDRI_MAGIC_0 || tdb_header.bdri.magic_1 != BDRI_MAGIC_1){
		printf("[!] Embedded BDRI section is Missing or Corrupt\n");
		return BDRI_CORRUPT;
	}
	
	// Reading title table header
	fseek(tdb,(offset + tdb_header.bdri.title_table_offset),SEEK_SET);
	TITLE_TABLE_HEADER title_table_header;
	memset(&title_table_header,0x0,sizeof(title_table_header));
	fread(&title_table_header,sizeof(title_table_header),1,tdb);
	printf("[+] Title List\n");
	for(int i = 0; i < title_table_header.entry_count; i++){
		fseek(tdb,(offset + tdb_header.bdri.title_table_offset + sizeof(title_table_header) + (sizeof(TID_STRUCT)*i)),SEEK_SET);
		TID_STRUCT tid;
		memset(&tid,0x0,sizeof(tid));
		fread(&tid,sizeof(tid),1,tdb);
		if(tid.active_entry == TRUE){
			printf("[%d]:\n",i);
			printf("  Title ID:           "); u8_hex_print_le(tid.title_id, 8); printf("\n");
			//printf("  Active Entry :      %08x\n",tid.active_entry);
			printf("  Title Index :       %d\n",tid.unknown_1);
			printf("  Unknown_2 :         %08x\n",tid.unknown_2);
			printf("  PDC Address X :     %08x\n",tid.pdc_table_X);
			printf("  PDC Address Y :     %08x\n",tid.pdc_table_Y);
			printf("  Unknown_5 :         %08x\n",tid.unknown_5);
			printf("  Unknown_6 :         %08x\n",tid.unknown_6);
			printf("  Unknown_7 :         %08x\n",tid.unknown_7);
			printf("  Unknown_8 :         %08x\n",tid.unknown_8);
			fseek(tdb,(offset + tdb_header.bdri.title_table_offset + (tid.pdc_table_X*tid.pdc_table_Y) + tid.pdc_table_Y),SEEK_SET);
			PRODUCT_CODE_STRUCT pdc;
			memset(&pdc,0x0,sizeof(pdc));
			fread(&pdc,sizeof(pdc),1,tdb);
			printf("  Product Code:       %s\n",pdc.product_code);
			printf("  Title Size:         "); u8_hex_print_le(pdc.title_size, 8); printf("\n");
			printf("  Title Type:         %x\n",pdc.title_type);
			printf("  Title Version:      v%d\n",pdc.title_version);
			printf("  TMD Content ID:     %08x\n",pdc.tmd_file_id);
			printf("  CMD Content ID:     %08x\n",pdc.cmd_file_id);
			printf("  ExtdataID low:      %08x\n",pdc.extdata_id);
			printf("  Manual:             %s\n",pdc.electronic_manual? "YES" : "NO");
			printf("  Save Data:          %s\n",pdc.save_data? "YES" : "NO");
			printf("  Unknown_0:          %x\n",pdc.unknown_3);
			printf("  Unknown_1:          %x\n",pdc.unknown_4);
			printf("  Unknown_2:          %x\n",pdc.constant);
			printf("  Unknown:            %08x\n",pdc.unknown_6);
		}
	}
	return 0;
}