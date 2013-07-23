#include "lib.h"
#include "main.h"
#include "settings.h"
#include "cia.h"
#include "ticket.h"
#include "tmd.h"
#include "meta.h"

typedef enum
{
	MAJOR = 5,
	MINOR = 1
} AppVer;

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
	
	for(int i = 0; i < argc; i++){
		if(strncmp(argv[i],"-h",2) == 0 || strncmp(argv[i],"--help",6) == 0){
			help(argv[0]);
			return ARGC_FAIL;
		}
	}
	
	CIA_CONTEXT *ctx = malloc(sizeof(CIA_CONTEXT));
	InitialiseSettings(ctx);
	if(GetSettings(ctx,argc,argv) != 0){
		printf("[!] Input Error, see '%s -h' for more info\n",argv[0]);
		//help(argv[0]);
		goto fail_cleanup;
	}
	if(SetupContent(ctx) != 0){
		printf("[!] Content could not be setup for CIA\n");
		goto fail_cleanup;
	}
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

void free_buffers(CIA_CONTEXT *ctx)
{
	//Closing Files
	if(ctx->outfile.file.used == True)
		fclose(ctx->outfile.file.file);
	if(ctx->configfile.file.used == True)
		fclose(ctx->configfile.file.file);
	if(ctx->core_infile.file.used == True)
		fclose(ctx->core_infile.file.file);
	
	//Freeing Arguments
	if(ctx->outfile.used)
		free(ctx->outfile.argument);
	if(ctx->configfile.used)
		free(ctx->configfile.argument);
	if(ctx->core_infile.used)
		free(ctx->core_infile.argument);
	
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
	printf(" -e, --encrypt                                Globally Encrypt CIA Contents\n");
	printf(" -o, --out=             File-out              CIA Output\n");
	printf("CONTENT OPTIONS\n");
	printf(" --contentX=            File-in               Content X path\n");
	printf(" --id_X=                Value                 Content X ID\n");
	printf(" --index_X=             Value                 Content X Index\n");
	printf(" --crypt_X                                    Encrypt Content X\n");
	printf(" --optional_X                                 Flag Content X as 'Optional'\n");
	printf(" --shared_X                                   Flag Content X as 'Shared'\n");
	printf("EXTRA OPTIONS\n");
	printf(" -0, --savesize=        Value                 Savedata Size in KB\n");
	printf(" -1, --tikID=           Value                 Ticket ID\n");
	printf(" -2, --titleID=         Value                 Title ID\n");
	printf(" -3, --major=           Value                 TMD Version Major\n");
	printf(" -4, --minor=           Value                 TMD Version Minor\n");
	printf(" -5, --micro=           Value                 TMD Version Micro\n");
	printf(" -6, --tikmajor=        Value                 TIK Version Major\n");
	printf(" -7, --tikminor=        Value                 TIK Version Minor\n");
	printf(" -8, --tikmicro=        Value                 TIK Version Micro\n");
	printf("CRYPTOGRAPHY OPTIONS\n");
	printf(" --ckey=                Value                 Common Key\n");
	printf(" --ckeyID=              Value                 Common Key ID\n");
	printf(" --cxikey=              Value                 CXI Key\n");
	printf(" --titlekey=            Value                 Title Key\n");
	printf(" --rand                                       Use a Random Title Key\n");
	printf(" --tmdkey=              File-in               TMD RSA Keyfile\n");
	printf(" --tikkey=              File-in               TIK RSA Keyfile\n");
	printf(" --certs=               File-in               Certificate Chain File\n");
	printf("SPECIAL BUILD OPTIONS\n");
	printf(" --srl=                 File-in               Specify a SRL for content0.\n");
	printf(" --rom=                 File-in               Convert ROM to CIA.\n");
}
