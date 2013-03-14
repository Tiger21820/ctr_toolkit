#include <stdio.h>
//#include <stdlib.h>
//#include <stdint.h>
#include <string.h>

#include "types.h"
#include "bnr.h"
#include "icn.h"

#define SUCCESS 0
#define FAIL 1

#define ARGC_FAIL 1
#define ARGV_FAIL 2

#define FILE_OPEN_FAIL 1
#define FILE_CREATE_FAIL 2

void icn_process(int argc, char *argv[]);
void read_icn_process(int argc, char *argv[]);
void bnr_process(int argc, char *argv[]);
void app_title(void);
void help(char *app_name);

int main(int argc, char *argv[])
{
	app_title();
	if (argc < 2){
		printf("[!] Must Specify Arguments\n");
		help(argv[0]);
		return ARGC_FAIL;
	}
	if (strcmp(argv[1], "--bnr") == 0){
		bnr_process(argc, argv);
	} 
	else if (strcmp(argv[1], "--icn") == 0){
		icn_process(argc, argv);
	}
	else if (strcmp(argv[1], "--readicn") == 0){
		read_icn_process(argc, argv);
	}
	else if (strcmp(argv[1], "--help") == 0){
		help(argv[0]);
		return ARGV_FAIL;
	}
	else{
		printf("[!] Bad Arguments\n");
		help(argv[0]);
		return ARGV_FAIL;
	} 
	return 0;
}

void icn_process(int argc, char *argv[])
{
	if (argc != 10){
		printf("[!] Bad Arguments\n");
		help(argv[0]);
		exit(1);
	}	
	//Creating ICN Context
	ICN_CONTEXT icn;
	memset(&icn, 0x00, sizeof(icn));
	char *outname;

	//Opening Input/Output File(s)
	for (int i = 2; i < argc; i++){
		if(strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0){
			outname = malloc((strlen(argv[i+1]) + 1)*sizeof(char));
			strcpy(outname,argv[i+1]);
			icn.output = fopen(argv[i+1], "wb");
			if (icn.output == NULL){
				printf("[!] Could not create [%s]\n", argv[i+1]);
				exit(FILE_CREATE_FAIL);
			}
		}
		else if(strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--bsf") == 0){
			icn.bsf = fopen(argv[i+1], "rb");
			if (icn.bsf == NULL){
				printf("[!] Could not open [%s]\n", argv[i+1]);
				exit(FILE_OPEN_FAIL);
			}
		}
		else if(strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--small_icon") == 0){
			icn.small = fopen(argv[i+1], "rb");
			if (icn.small == NULL){
				printf("[!] Could not open [%s]\n", argv[i+1]);
				exit(FILE_OPEN_FAIL);
			}
		}
		else if(strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--large_icon") == 0){
			icn.large = fopen(argv[i+1], "rb");
			if (icn.large == NULL){
				printf("[!] Could not open [%s]\n", argv[i+1]);
				exit(FILE_OPEN_FAIL);
			}
		}
	}
	//Final Existance Check
	if(icn.output == NULL || icn.bsf == NULL || icn.small == NULL || icn.large == NULL){
		printf("[!] File(s) could not be created/opened\n");
		exit(FILE_OPEN_FAIL);
	}
	
	//Creating ICN
	int icn_result = icn_main(icn);

	//Closing File Streams
	fclose(icn.output);
	fclose(icn.bsf);
	fclose(icn.small);
	fclose(icn.large);
	
	//Result
	if(icn_result == SUCCESS)
		printf("[*] ICN File Built Successfully\n");
	else{
		remove(outname);
		printf("[!] ICN File Failed to Build. Error code: %d\n", icn_result);
	}
	free(outname);
}

void read_icn_process(int argc, char *argv[])
{
	if (argc != 3){
		printf("[!] Bad Arguments\n");
		help(argv[0]);
		exit(1);
	}	

	//Opening Input File
	FILE *icn = fopen(argv[2],"rb");
	if (icn == NULL){
		printf("[!] Could not create [%s]\n", argv[2]);
		exit(FILE_OPEN_FAIL);
	}
	
	//Creating ICN
	int icn_result = icn_read(icn);

	//Closing File Streams
	fclose(icn);
	
	//Result
	if(icn_result == SUCCESS)
		printf("[*] ICN File Parsed Successfully\n");
	else{
		printf("[!] ICN File Could not be Parsed. Error code: %d\n", icn_result);
	}
}

void bnr_process(int argc, char *argv[])
{
	if (argc != 8){
		printf("[!] Bad Arguments\n");
		help(argv[0]);
		exit(1);
	}
	//Creating BNR Context
	BNR_CONTEXT bnr;
	memset(&bnr, 0x00, sizeof(bnr));
	char *outname;

	//Opening Input/Output File(s)
	for (int i = 2; i < argc; i++){
		if(strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0){
			outname = malloc((strlen(argv[i+1]) + 1) * sizeof(char));
			strcpy(outname,argv[i+1]);
			bnr.output = fopen(argv[i+1], "wb");
			if (bnr.output == NULL){
				printf("[!] Could not create [%s]\n", argv[i+1]);
				exit(FILE_CREATE_FAIL);
			}
		}
		else if(strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--cwav") == 0){
			bnr.cwav = fopen(argv[i+1], "rb");
			if (bnr.cwav == NULL){
				printf("[!] Could not open [%s]\n", argv[i+1]);
				exit(FILE_OPEN_FAIL);
			}
		}
		else if(strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--cbmd") == 0){
			bnr.cbmd = fopen(argv[i+1], "rb");
			if (bnr.cbmd == NULL){
				printf("[!] Could not open [%s]\n", argv[i+1]);
				exit(FILE_OPEN_FAIL);
			}
		}
	}

	//Final Existance Check
	if(bnr.output == NULL || bnr.cwav == NULL || bnr.cbmd == NULL){
		printf("[!] File(s) could not be created/opened\n");
		exit(FILE_OPEN_FAIL);
	}
	
	//File Header reading
	fseek(bnr.cwav,0x0,SEEK_SET);
	fread(&bnr.cwav_header, sizeof(CWAV_HEADER), 1, bnr.cwav);
	fseek(bnr.cbmd,0x0,SEEK_SET);
	fread(&bnr.cbmd_header, sizeof(CBMD_HEADER), 1, bnr.cbmd);

	if(bnr.cwav_header.magic != CWAV_MAGIC){
		printf("[!] (B)CWAV (Audio) File is corrupt\n");
		exit(FILE_OPEN_FAIL);
	}
	
	if(bnr.cbmd_header.magic != CBMD_MAGIC){
		printf("[!] CBMD (Graphics) File is corrupt\n");
		exit(FILE_OPEN_FAIL);
	}		

	//Creating BNR
	int bnr_result = bnr_main(bnr);
	
	//Closing File Streams
	fclose(bnr.output);
	fclose(bnr.cbmd);
	fclose(bnr.cwav);	

	//Result
	if(bnr_result == SUCCESS)
		printf("[*] BNR File Built Successfully\n");
	else{
		remove(outname);
		printf("[!] BNR File Failed to Build\n");
	}
	free(outname);
}

void app_title(void)
{
	printf("CTR-SDK Development Tool - make_banner - Generates ICN/BNR files\n");
	printf("Version: 1.1\n(c) 2013 3DSGuy\n\n");
}

void help(char *app_name)
{
	printf("\nUsage: %s [mode] <arguments>\n", app_name);
	putchar('\n');
	printf("Modes:\n");
	printf("  --icn			  Generate an ICN file.\n");
	printf("  --bnr			  Generate a BNR file.\n");
	printf("  --readicn file  	  Parse an ICN file.\n");
	printf("  --help		  Show this text.\n");
	putchar('\n');
	printf("'ICN' Mode Arguments:\n");
	printf("  -b  --bsf file 	  Specify Input BSF file.\n");
	printf("  -s  --small_icon file	  Specify Input 'Small Icon' file.\n");
	printf("  -l  --large_icon file	  Specify Input 'Large Icon' file.\n");
	putchar('\n');
	printf("'BNR' Mode Arguments:\n");
	printf("  -g  --cbmd file 	  Specify Input .CBMD file.\n");
	printf("  -a  --cwav file	  Specify Input .(B)CWAV file.\n");
}
