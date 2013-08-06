/**
Copyright 2013 3DSGuy

This file is part of make_cia.

make_cia is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

make_cia is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with make_cia.  If not, see <http://www.gnu.org/licenses/>.
**/
#include "lib.h"
#include "main.h"
#include "settings.h"
#include "cia.h"
#include "ticket.h"
#include "tmd.h"
#include "meta.h"

typedef enum
{
	MAJOR = 6,
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
	
	USER_CONTEXT *ctx = malloc(sizeof(USER_CONTEXT));
	if(ctx == NULL){
		printf("[!] Memory Allocation Failure\n");
		return Fail;
	}
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
	if(ctx->outfile.arg_len == 0)
		printf("[!] Failed to generate cia\n");
	else
		printf("[!] Failed to generate %s\n",ctx->outfile.argument);
	free_buffers(ctx);
	return 1;

}

void free_buffers(USER_CONTEXT *ctx)
{
	//Freeing Arguments
	_free(ctx->outfile.argument);
	_free(ctx->core_infile.argument);
	
	//Freeing CIA section buffers
	_free(ctx->cia_section[header].buffer);
	_free(ctx->cia_section[certchain].buffer);
	_free(ctx->cia_section[tik].buffer);
	_free(ctx->cia_section[tmd].buffer);
	_free(ctx->cia_section[content].buffer);
	_free(ctx->cia_section[meta].buffer);
	_free(ctx->ContentInfo);
	_free(ctx->ncsd_struct);
	
	//Freeing Main context
	_free(ctx);
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
	printf(" -p, --info                                   Print Info.\n");
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
	printf(" --forcecxikey                                Overide Fixed CXI keyslots with user CXI Key\n");
	printf(" --titlekey=            Value                 Title Key\n");
	printf(" --rand                                       Use a Random Title Key\n");
	printf(" --tmdkey=              File-in               TMD RSA Keyfile\n");
	printf(" --tikkey=              File-in               TIK RSA Keyfile\n");
	printf(" --certs=               File-in               Certificate Chain File\n");
	printf("SPECIAL BUILD OPTIONS\n");
	printf(" --srl=                 File-in               Specify a SRL for content0.\n");
	printf(" --rom=                 File-in               Convert ROM to CIA.\n");
}
