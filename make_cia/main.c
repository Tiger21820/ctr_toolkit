#include "lib.h"
#include "main.h"
#include "settings.h"
#include "cia.h"
#include "ticket.h"
#include "tmd.h"
#include "meta.h"

#define MAJOR 3
#define MINOR 00

void app_title(void);
void help(char *app_name);

int main(int argc, char *argv[])
{
	app_title();
	
	//Filter Out Bad number of arguments
	if (argc < 2 || argc > 7){
		printf("[!] Must Specify Arguments\n");
		help(argv[0]);
		return ARGC_FAIL;
	}
	
	CIA_CONTEXT *ctx = malloc(sizeof(CIA_CONTEXT));
	InitialiseSettings(ctx);
	
	if (getcwdir(ctx->cwd, 1024) == NULL){
		printf("[!] Could not store Current Working Directory\n");
		return IO_FAIL;
	}
		
	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
			help(argv[0]);
			return ARGC_FAIL;
		}
		else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0){
			ctx->verbose_flag = TRUE;
		}
		else if(strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--showkeys") == 0){
			ctx->showkeys_flag = TRUE;
		}
		else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config_file") == 0){
			if(argv_check(argv[i+1],argc,i) == TRUE){
				ctx->configfile.used = TRUE;
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
			if(argv_check(argv[i+1],argc,i) == TRUE){
				ctx->outfile.used = TRUE;
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
	}
	
	if(ctx->configfile.used != TRUE){
		printf("[!] No CIA configuration file was specified\n");
		goto fail_cleanup;
	}
	
	if(GetSettings(ctx) != 0){
		printf("[!] Settings Error\n");
		goto fail_cleanup;
	}
	
	if(ctx->outfile.used == FALSE){
		ctx->outfile.used = TRUE;
		ctx->outfile.arg_len = 20;
		ctx->outfile.argument = malloc(ctx->outfile.arg_len);
		sprintf(ctx->outfile.argument,"%x%02x%02x.cia",ctx->core.TitleID[4],ctx->core.TitleID[5],ctx->core.TitleID[6]);
	}
	
	ctx->outfile.file.used = TRUE;
	ctx->outfile.file.file = fopen(ctx->outfile.argument,"wb");
	if(ctx->outfile.file.file == NULL){
		printf("[!] IO ERROR: Failed to create '%s'\n",ctx->outfile.argument);
		ctx->outfile.file.used = FALSE;
		goto fail_cleanup;
	}
	
	if(GenerateTicket(ctx) != 0){
		printf("[!] Ticket region could not be generated\n");
		goto fail_cleanup;
	}
	
	if(SetupContentData(ctx) != 0){
		printf("[!] Content Parsing error\n");
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
	
	printf("[+] %s generated Successfully\n",ctx->outfile.argument);
	free_buffers(ctx);
	return 0;
fail_cleanup:
	if(ctx->outfile.used == FALSE)
		printf("[!] Failed to generate cia\n");
	else
		printf("[!] Failed to generate %s\n",ctx->outfile.argument);
	free_buffers(ctx);
	return 1;
}

int argv_check(char *string, int argc, int index)
{
	if(index >= (argc - 1))
		return FALSE;
	char argv_lable[arg_num][arg_max_len] = {"-h","--help","-v","--verbose","-k","--showkeys","-c","--config_file","-o","--output"};
	for(int i = 0; i < arg_num; i++){
		if(strcmp(string,argv_lable[i]) == 0)
			return FALSE;
	}
	return TRUE;
}

void free_buffers(CIA_CONTEXT *ctx)
{
	//Closing Files
	if(ctx->outfile.file.used == TRUE)
		fclose(ctx->outfile.file.file);
	if(ctx->configfile.file.used == TRUE)
		fclose(ctx->configfile.file.file);
	
	//Freeing CIA section buffers
	if(ctx->header.used)
		free(ctx->header.buffer);
	if(ctx->certchain.used)
		free(ctx->certchain.buffer);
	if(ctx->ticket.used)
		free(ctx->ticket.buffer);
	if(ctx->tmd.used)
		free(ctx->tmd.buffer);
	if(ctx->meta.used)
		free(ctx->meta.buffer);
	if(ctx->ContentInfoMallocFlag)
		free(ctx->ContentInfo);
	
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
	printf("\nUsage: %s <options>\n", app_name);
	putchar('\n');
	printf("OPTIONS                 Possible Values       Explanation\n");
	printf(" -h, --help                                   Print this help.\n");
	printf(" -v, --verbose                                Enable verbose output.\n");
	printf(" -k, --showkeys                               Show the keys being used.\n");
	printf(" -c, --config_file      File-in               CIA Configuration File.\n");
	printf(" -o, --output           File-out              Output CIA file.\n");
}
