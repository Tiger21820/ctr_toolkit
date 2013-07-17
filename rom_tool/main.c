#include "lib.h"
#include "main.h"
#include "ncsd.h"

typedef enum
{
	MAJOR = 2,
	MINOR = 1
} AppVer;

const char argv_lable[8][20] = {"-h","--help","-i","--info","-r","--restore","-t","--trim"};

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
	
	ROM_CONTEXT *ctx = malloc(sizeof(ROM_CONTEXT));
	memset(ctx,0x0,sizeof(ROM_CONTEXT));
		
	for(int i = 1; i < (argc - 1); i++){
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
			help(argv[0]);
			return ARGC_FAIL;
		}
		else if(strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--info") == 0)
			ctx->info_flag = True;
		else if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--restore") == 0)
			ctx->rom_restore_flag = True;
		else if(strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trim") == 0)
			ctx->rom_trim_flag = True;
	}
	
	if(ctx->rom_restore_flag == True && ctx->rom_trim_flag == True){
		printf("[!] You cannot trim and restore a ROM at the same time\n");
		help(argv[0]);
		return 1;
	}
	
	ctx->romfile.used = True;
	ctx->romfile.arg_len = strlen(argv[argc-1]);
	ctx->romfile.argument = malloc(ctx->romfile.arg_len+1);
	memcpy(ctx->romfile.argument,argv[argc-1],ctx->romfile.arg_len+1);
	ctx->romfile.file.used = True;
	ctx->romfile.file.file = fopen(ctx->romfile.argument,"rb+");
	if(ctx->romfile.file.file == NULL){
		printf("[!] Failed to open '%s'\n",ctx->romfile.argument);
		ctx->romfile.file.used = False;
		return 1;
	}
	ctx->romfile.file.used = False;
	fclose(ctx->romfile.file.file);
	
	if(NCSDProcess(ctx) != 0)
		goto fail_cleanup;
	
	printf("[*] Completed Successfully\n");
	free_buffers(ctx);
	return 0;
fail_cleanup:
	printf("[!] Failed\n");
	free_buffers(ctx);
	return 1;
}

int argv_check(char *string, int argc, int index)
{
	for(int i = 0; i < 8; i++){
		if(strcmp(string,argv_lable[i]) == 0)
			return False;
	}
	return True;
}

void free_buffers(ROM_CONTEXT *ctx)
{
	//Closing Files
	if(ctx->romfile.file.used == True)
		fclose(ctx->romfile.file.file);
	
	//Freeing Arguments
	if(ctx->romfile.used)
		free(ctx->romfile.argument);
	
	//Freeing ROM Data buffers
	if(ctx->ncsd_struct_malloc_flag)
		free(ctx->ncsd_struct);
	
	//Freeing Main context
	free(ctx);
}

void app_title(void)
{
	printf("CTR_Toolkit - ROM TOOL\n");
	printf("Version %d.%d (C) 3DSGuy 2013\n",MAJOR,MINOR);
}

void help(char *app_name)
{
	app_title();
	printf("Usage: %s [options] <rom filepath>\n", app_name);
	printf("OPTIONS                 Explanation\n");
	printf(" -h, --help             Print this help.\n");
	printf(" -i, --info             Print 3DS ROM Info\n");
	printf(" -r, --restore          Restore(Un-Trim) 3DS ROM File.\n");
	printf(" -t, --trim             Trim 3DS ROM File.\n");
}