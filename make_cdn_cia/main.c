/**
Copyright 2013 3DSGuy

This file is part of make_cdn_cia.

make_cdn_cia is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

make_cdn_cia is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with make_cdn_cia.  If not, see <http://www.gnu.org/licenses/>.
**/
#include "lib.h"
#include "cia.h"

//Version
#define MAJOR 1
#define MINOR 00

void app_title(void);
void help(u8 *app_name);

int main(int argc, char *argv[])
{
	app_title();
	
	//Argument Checks
	if(argc < 3){
		printf("[!] Not Enough Arguments\n");
		help(argv[0]);
		return 1;
	}
	if(argc > 3){
		printf("[!] Too Many Arguments\n");
		help(argv[0]);
		return 1;
	}
	
	//Storing Current Working Directory
	char cwd[1024];
	if (getcwdir(cwd, sizeof(cwd)) == NULL){
		printf("[!] Could not store Current Working Directory\n");
		return IO_FAIL;
	}
	
	//Changing to CDN Content Directory
	chdir(argv[1]);
	
	//Processing CETK
	FILE *cetk = fopen("cetk","rb");
	if(cetk == NULL){
		printf("[!] Could not open 'cetk'\n");
		return IO_FAIL;
	}
	CETK_CONTEXT cetk_context = process_cetk(cetk);
	
	//Processing TMD
	FILE *tmd = fopen("tmd","rb");
	if(tmd == NULL){
		printf("[!] Could not open 'tmd'\n");
		return IO_FAIL;
	}
	TMD_CONTEXT tmd_context = process_tmd(tmd);
	
	//Error Checking
	if(cetk_context.result != 0 || tmd_context.result != 0){
		printf("[!] Input files could not be processed successfully\n");
		free(tmd_context.content_struct);
		free(tmd_context.content);
		fclose(cetk);
		fclose(tmd);
		return FILE_PROCESS_FAIL;
	}
	//TID comparison check
	if(check_tid(cetk_context.title_id,tmd_context.title_id) != TRUE){
		printf("[!] Caution, Ticket and TMD Title IDs do not match\n");
		printf("[!] CETK Title ID:  "); u8_hex_print_be(cetk_context.title_id,0x8); printf("\n");
		printf("[!] TMD Title ID:   "); u8_hex_print_be(tmd_context.title_id,0x8); printf("\n");
	}
	//Title Version comparison
	if(cetk_context.title_version != tmd_context.title_version){
		printf("[!] Caution, Ticket and TMD Title Versions do not match\n");
		printf("[!] CETK Title Ver: %d\n",cetk_context.title_version);
		printf("[!] TMD Title Ver:  %d\n",tmd_context.title_version);
	}
	
	//Returning to Original Working Directory
	chdir(cwd);
	
	//Opening Output file
	FILE *output = fopen(argv[2],"wb");
	if(output == NULL){
		printf("[!] Could not create '%s'\n",argv[2]);
		return IO_FAIL;
	}
	
	int result = generate_cia(tmd_context,cetk_context,output);
	if(result != 0){
		printf("[!] Failed to Generate %s\n",argv[2]);
		remove(argv[2]);
	}
	else
		printf("[*] %s Generated Sucessfully\n",argv[2]);
	
	return 0;
}

void app_title(void)
{
	printf("CTR_Toolkit - CIA Generator for CDN Content\n");
	printf("Version %d.%d (C) 3DSGuy 2013\n\n",MAJOR,MINOR);
}

void help(u8 *app_name)
{
	printf("\nUsage: %s <CDN Content Dir> <output CIA file>\n", app_name);
}