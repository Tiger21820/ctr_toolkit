#include <stdio.h>
//#include <stdlib.h>
//#include <stdint.h>
#include <string.h>

#include "main.h"

//Version Number
#define MAJOR 1
#define MINOR 2


int icn_process(INPUT_CONTEXT ctx);
int read_icn_process(INPUT_CONTEXT ctx);
int bnr_process(INPUT_CONTEXT ctx);
void app_title(void);
void help(char *app_name);

int main(int argc, char *argv[])
{
	app_title();
	if (argc < 2 || argc > 18){
		printf("\n[!] Must Specify Arguments\n");
		help(argv[0]);
		return ARGC_FAIL;
	}
	INPUT_CONTEXT ctx;
	memset(&ctx,0,sizeof(ctx));
	ctx.command_point = 20;
	//getting commands
	for(int i = (argc - 1); i > 0; i--){
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0){
			help(argv[0]);
			return ARGV_FAIL;
		}
		else if (strcmp(argv[i], "--makebnr") == 0){
			if(i < (argc - 1)){
				ctx.make_bnr = TRUE;
				ctx.output_bnr = argv[i+1];
				if (i < ctx.command_point)
					ctx.command_point = i;
			}
			else{
				printf("\n[!] Missing Parameter for '--makebnr'\n");
				return ARGV_FAIL;
			}
		} 
		else if (strcmp(argv[i], "--makeicn") == 0){
			if(i < (argc - 1)){
				ctx.make_icn = TRUE;
				ctx.output_icn = argv[i+1];
				if (i < ctx.command_point)
					ctx.command_point = i;
			}
			else{
				printf("\n[!] Missing Parameter for '--makeicn'\n");
				return ARGV_FAIL;
			}
		}
		else if (strcmp(argv[i], "--readicn") == 0){
			if(i < (argc - 1)){
				ctx.read_icn = TRUE;
				ctx.input_icn = argv[i+1];
				if (i < ctx.command_point)
					ctx.command_point = i;
			}
			else{
				printf("\n[!] Missing Parameter for '--readicn'\n");
				return ARGV_FAIL;
			}
		}
	}
	//Collecting options
	u8 option_count;
	u8 bool_input[3];
	//Make ICN
	memset(&bool_input,FALSE,sizeof(bool_input));
	if(ctx.make_icn == TRUE){
		for(int i = 1; i < (ctx.command_point - 1); i++){
			if(strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--bsf") == 0){
				bool_input[0] = TRUE;
				ctx.input_bsf = argv[i+1];
			}
			else if(strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--small_icon") == 0){
				bool_input[1] = TRUE;
				ctx.input_small_icon = argv[i+1];
			}
			else if(strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--large_icon") == 0){
				bool_input[2] = TRUE;
				ctx.input_large_icon = argv[i+1];
			}
		}
		option_count = bool_input[0] + bool_input[1] + bool_input[2];
		if(option_count != 3){
			printf("\n[!] Incorrect Options for command '--makeicn'\n");
			help(argv[0]);
			return 1;
		}
	}
	//Make BNR
	memset(&bool_input,FALSE,sizeof(bool_input));
	if(ctx.make_bnr == TRUE){
		for(int i = 1; i < (ctx.command_point - 1); i++){
			if(strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--cwav") == 0){
				bool_input[0] = TRUE;
				ctx.input_cwav = argv[i+1];
			}
			else if(strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--cbmd") == 0){
				bool_input[1] = TRUE;
				ctx.input_cbmd = argv[i+1];
			}
		}
		option_count = bool_input[0] + bool_input[1];
		if(option_count != 2){
			printf("\n[!] Incorrect Options for command '--makebnr'\n");
			help(argv[0]);
			return 1;
		}
	}
	//Verbose?
	for(int i = 1; i < ctx.command_point; i++){
		if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0){
			ctx.verbose_bool = TRUE;
		}
	}
	
	//Performing commands
	int result;
	if(ctx.make_icn == TRUE){
		result = icn_process(ctx);
		printf("\n");
		if(result != 0)
			printf("[!] ICN File Failed to Build. Error code: %d\n", result);
		else
			printf("[*] ICN File Built Successfully\n");
	}
	if(ctx.make_bnr == TRUE){
		result = bnr_process(ctx);
		printf("\n");
		if(result != 0)
			printf("[!] BNR File Failed to Build. Error code: %d\n", result);
		else
			printf("[*] BNR File Built Successfully\n");
	}
	if(ctx.read_icn == TRUE){
		result = read_icn_process(ctx);
		printf("\n");
		if(result != 0)
			printf("[!] ICN File Could not be Parsed. Error code: %d\n", result);
		else
			printf("[*] ICN File Parsed Successfully\n");
	}
	return 0;
}

void app_title(void)
{
	printf("CTR-SDK Development Tool - make_banner - Generates ICN/BNR files\n");
	printf("Version: %d.%d (C) 2013 3DSGuy\n",MAJOR,MINOR);
}

void help(char *app_name)
{
	printf("\nUsage: %s [options] commands\n", app_name);
	putchar('\n');
	printf("COMMANDS                Parameters            Explanation\n");
	printf(" -h, --help                                   Print this help.\n");
	printf("     --makeicn      	File-out              Generate an ICN file.\n");
	printf("     --makebnr      	File-out              Generate a BNR file.\n");
	printf("     --readicn          File-in               Parse an ICN file.\n");
	printf("OPTIONS                 Possible Values       Explanation\n");
	printf(" -v, --verbose                                Enable verbose output.\n");
	printf(" -b, --bsf              File-in               BSF file.\n");
	printf(" -s, --small_icon       File-in               'Small Icon' file.\n");
	printf(" -l, --large_icon       File-in               'Large Icon' file.\n");
	printf(" -g, --cbmd             File-in               CBMD file.\n");
	printf(" -a, --cwav             File-in	              (B)CWAV file.\n");
}

int icn_process(INPUT_CONTEXT ctx)
{	
	//Creating ICN Context
	ICN_CONTEXT icn;
	memset(&icn, 0x00, sizeof(icn));
	
	icn.output = fopen(ctx.output_icn, "wb");
	if (icn.output == NULL){
		printf("[!] Could not create [%s]\n", ctx.output_icn);
		return FILE_CREATE_FAIL;
	}
	icn.bsf = fopen(ctx.input_bsf, "rb");
	if (icn.bsf == NULL){
		printf("[!] Could not open [%s]\n", ctx.input_bsf);
		return FILE_OPEN_FAIL;
	}
	icn.small = fopen(ctx.input_small_icon, "rb");
	if (icn.small == NULL){
		printf("[!] Could not open [%s]\n", ctx.input_small_icon);
		return FILE_OPEN_FAIL;
	}
	icn.large = fopen(ctx.input_large_icon, "rb");
	if (icn.large == NULL){
		printf("[!] Could not open [%s]\n", ctx.input_large_icon);
		return FILE_OPEN_FAIL;
	}
	
	icn.verbose_bool = ctx.verbose_bool;
	
	//Creating ICN
	int icn_result = icn_main(icn);

	//Closing File Streams
	fclose(icn.output);
	fclose(icn.bsf);
	fclose(icn.small);
	fclose(icn.large);
	
	//Result
	if(icn_result != SUCCESS)
		remove(ctx.output_icn);
	return icn_result;
}

int bnr_process(INPUT_CONTEXT ctx)
{
	//Creating BNR Context
	BNR_CONTEXT bnr;
	memset(&bnr, 0x00, sizeof(bnr));

	bnr.output = fopen(ctx.output_bnr, "wb");
	if (bnr.output == NULL){
		printf("[!] Could not create [%s]\n", ctx.output_bnr);
		return FILE_CREATE_FAIL;
	}
	bnr.cwav = fopen(ctx.input_cwav, "rb");
	if (bnr.cwav == NULL){
		printf("[!] Could not open [%s]\n", ctx.input_cwav);
		return FILE_OPEN_FAIL;
	}
	bnr.cbmd = fopen(ctx.input_cbmd, "rb");
	if (bnr.cbmd == NULL){
		printf("[!] Could not open [%s]\n", ctx.input_cbmd);
		return FILE_OPEN_FAIL;
	}
	
	bnr.verbose_bool = ctx.verbose_bool;
	
	//File Header reading
	fseek(bnr.cwav,0x0,SEEK_SET);
	fread(&bnr.cwav_header, sizeof(CWAV_HEADER), 1, bnr.cwav);
	fseek(bnr.cbmd,0x0,SEEK_SET);
	fread(&bnr.cbmd_header, sizeof(CBMD_HEADER), 1, bnr.cbmd);

	if(bnr.cwav_header.magic != CWAV_MAGIC){
		printf("[!] (B)CWAV (Audio) File is corrupt\n");
		return FILE_OPEN_FAIL;
	}
	
	if(bnr.cbmd_header.magic != CBMD_MAGIC){
		printf("[!] CBMD (Graphics) File is corrupt\n");
		return FILE_OPEN_FAIL;
	}		

	//Creating BNR
	int bnr_result = bnr_main(bnr);
	
	//Closing File Streams
	fclose(bnr.output);
	fclose(bnr.cbmd);
	fclose(bnr.cwav);	

	//Result
	if(bnr_result != SUCCESS)
		remove(ctx.output_bnr);
	return bnr_result;
}

int read_icn_process(INPUT_CONTEXT ctx)
{
	//Opening Input File
	FILE *icn = fopen(ctx.input_icn,"rb");
	if (icn == NULL){
		printf("[!] Could not open [%s]\n", ctx.input_icn);
		return FILE_OPEN_FAIL;
	}
	
	//Parsing ICN
	int read_result = icn_read(icn,ctx.verbose_bool);

	//Closing File Streams
	fclose(icn);
	
	//Result
	return read_result;
}
