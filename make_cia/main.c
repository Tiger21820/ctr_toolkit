#include "lib.h"
#include "main.h"
#include "settings.h"
#include "cia.h"
#include "ticket.h"
#include "tmd.h"
#include "meta.h"

typedef enum
{
	MAJOR = 4,
	MINOR = 0
} AppVer;

const char argv_lable[12][15] = {"-h","--help","-v","--verbose","-k","--showkeys","-c","--config_file","-o","--output","-r","--ncsd"};

void app_title(void);
void help(char *app_name);

int main(int argc, char *argv[])
{	
	//Filter Out Bad number of arguments
	if (argc < 2){
		printf("[!] Must Specify Arguments\n");
		help(argv[0]);
		return ARGC_FAIL;
	}
	
	CIA_CONTEXT *ctx = malloc(sizeof(CIA_CONTEXT));
	InitialiseSettings(ctx);
		
	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
			help(argv[0]);
			return ARGC_FAIL;
		}
		else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0){
			ctx->verbose_flag = True;
		}
		else if(strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--showkeys") == 0){
			ctx->showkeys_flag = True;
		}
		else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config_file") == 0){
			if(argv_check(argv[i+1],argc,i) == True){
				ctx->configfile.used = True;
				ctx->configfile.arg_len = strlen(argv[i+1]);
				ctx->configfile.argument = malloc(ctx->configfile.arg_len+1);
				memcpy(ctx->configfile.argument,argv[i+1],ctx->configfile.arg_len+1);
			}
			else{
				printf("[!] Option '%s' requires an argument\n",argv[i]);
				help(argv[0]);
				return ARGC_FAIL;
			}
		}
		else if(strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0){
			if(argv_check(argv[i+1],argc,i) == True){
				ctx->outfile.used = True;
				ctx->outfile.arg_len = strlen(argv[i+1]);
				ctx->outfile.argument = malloc(ctx->outfile.arg_len+1);
				memcpy(ctx->outfile.argument,argv[i+1],ctx->outfile.arg_len+1);
			}
			else{
				printf("[!] Option '%s' requires an argument\n",argv[i]);
				help(argv[0]);
				return ARGC_FAIL;
			}
		}
		else if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--ncsd") == 0){
			if(argv_check(argv[i+1],argc,i) == True){
				ctx->ncsd_convert_flag = True;
				ctx->ncsdfile.used = True;
				ctx->ncsdfile.arg_len = strlen(argv[i+1]);
				ctx->ncsdfile.argument = malloc(ctx->ncsdfile.arg_len+1);
				memcpy(ctx->ncsdfile.argument,argv[i+1],ctx->ncsdfile.arg_len+1);
				ctx->ncsdfile.file.used = True;
				ctx->ncsdfile.file.file = fopen(ctx->ncsdfile.argument,"rb");
				if(ctx->ncsdfile.file.file == NULL){
					printf("[!] Failed to open '%s' (NCSD File)\n",ctx->ncsdfile.argument);
					ctx->ncsdfile.file.used = False;
					return 1;
				}
			}
			else{
				printf("[!] Option '%s' requires an argument\n",argv[i]);
				help(argv[0]);
				return ARGC_FAIL;
			}
		}
	}
	if(ctx->ncsd_convert_flag == True)
		goto ncsd_conversion_process;
	else
		goto regular_cia_process;

ncsd_conversion_process:
#ifndef _DEBUG_KEY_BUILD_
	if(ctx->configfile.used != True){
			printf("[!] No CIA configuration file was specified\n");
			goto fail_cleanup;
	}
#endif
	if(GetSettings_NCSD(ctx) != 0){
			printf("[!] Settings Error\n");
			goto fail_cleanup;
	}
	if(SetupContentData_NCSD(ctx) != 0){
		printf("[!] Content Parsing error\n");
		goto fail_cleanup;
	}
	goto cia_data_gen;

regular_cia_process:
	if(ctx->configfile.used != True){
		printf("[!] No CIA configuration file was specified\n");
		goto fail_cleanup;
	}
	if(GetSettings(ctx) != 0){
		printf("[!] Settings Error\n");
		goto fail_cleanup;
	}
	if(SetupContentData(ctx) != 0){
		printf("[!] Content Parsing error\n");
		goto fail_cleanup;
	}
	goto cia_data_gen;
	
cia_data_gen:
	if(GenerateTicket(ctx) != 0){
		printf("[!] Ticket region could not be generated\n");
		goto fail_cleanup;
	}
	if(GenerateTitleMetaData(ctx) != 0){
		printf("[!] TMD region could not be generated\n");
		goto fail_cleanup;
	}
	if(GenerateMeta(ctx) != 0){
		printf("[!] Meta region could not be generated\n");
		goto fail_cleanup;
	}
	if(SetupCIAHeader(ctx) != 0){
		printf("[!] CIA file header could not be generated\n");
		goto fail_cleanup;
	}
	if(WriteSectionsToOutput(ctx) != 0){
		printf("[!] Failed to write sections to CIA file\n");
		goto fail_cleanup;
	}
	
	printf("[*] %s generated Successfully\n",ctx->outfile.argument);
	free_buffers(ctx);
	return 0;
fail_cleanup:
	if(ctx->outfile.used == False)
		printf("[!] Failed to generate cia\n");
	else
		printf("[!] Failed to generate %s\n",ctx->outfile.argument);
	free_buffers(ctx);
	return 1;
}

int argv_check(char *string, int argc, int index)
{
	if(index >= (argc - 1))
		return False;
	for(int i = 0; i < 12; i++){
		if(strcmp(string,argv_lable[i]) == 0)
			return False;
	}
	return True;
}

void free_buffers(CIA_CONTEXT *ctx)
{
	//Closing Files
	if(ctx->outfile.file.used == True)
		fclose(ctx->outfile.file.file);
	if(ctx->configfile.file.used == True)
		fclose(ctx->configfile.file.file);
	if(ctx->ncsdfile.file.used == True)
		fclose(ctx->ncsdfile.file.file);
	
	//Freeing Arguments
	if(ctx->outfile.used)
		free(ctx->outfile.argument);
	if(ctx->configfile.used)
		free(ctx->configfile.argument);
	if(ctx->ncsdfile.used)
		free(ctx->ncsdfile.argument);
	
	//Freeing CIA section buffers
	if(ctx->header.used)
		free(ctx->header.buffer);
	if(ctx->certchain.used)
		free(ctx->certchain.buffer);
	if(ctx->ticket.used)
		free(ctx->ticket.buffer);
	if(ctx->tmd.used)
		free(ctx->tmd.buffer);
	if(ctx->content.used)
		free(ctx->content.buffer);
	if(ctx->meta.used)
		free(ctx->meta.buffer);
	if(ctx->ContentInfoMallocFlag)
		free(ctx->ContentInfo);
	if(ctx->ncsd_struct_malloc_flag)
		free(ctx->ncsd_struct);
	
	//Freeing Main context
	free(ctx);
}

void app_title(void)
{
	printf("CTR_Toolkit - CIA Generator\n");
	printf("Version %d.%d (C) 3DSGuy 2013\n",MAJOR,MINOR);
}

void help(char *app_name)
{
	app_title();
	printf("Usage: %s <options>\n", app_name);
	printf("OPTIONS                 Possible Values       Explanation\n");
	printf(" -h, --help                                   Print this help.\n");
	printf(" -v, --verbose                                Enable verbose output.\n");
	printf(" -k, --showkeys                               Show the keys being used.\n");
	printf(" -c, --config_file      File-in               CIA Configuration File.\n");
	printf(" -r, --ncsd             File-in               Convert NCSD Image to CIA.\n");
	printf(" -o, --output           File-out              Output CIA file.\n");
}
