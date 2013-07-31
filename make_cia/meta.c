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
#include "ctr_crypto.h"
#include "meta.h"
#include "cia.h"
#include "ncsd.h"

void MetaCleanup(COMPONENT_STRUCT *icon, FILE *ncch);

int GenerateMeta(USER_CONTEXT *ctx)
{	
	if(ctx->flags[gen_meta] == False)
		return 0;
	
	if(ctx->flags[verbose])	{ printf("[+] Generating CIA Metadata\n"); }

	//Get Meta Section
	NCCH_STRUCT cxi_ctx;
	META_STRUCT meta_ctx;
	COMPONENT_STRUCT cxi_icon;
	memset(&cxi_ctx,0x0,sizeof(NCCH_STRUCT));
	memset(&meta_ctx,0x0,sizeof(META_STRUCT));
	memset(&cxi_icon,0x0,sizeof(COMPONENT_STRUCT));
	u32 offset;
	FILE *ncch;
	if(ctx->flags[verbose])	{ printf(" > Opening Content0\n"); }
	if(ctx->flags[build_mode] == ctr_norm){
		offset = 0;
		ncch = fopen(ctx->ContentInfo[0].file_path,"rb");
	}
	else if(ctx->flags[build_mode] == rom_conv){
		offset = ctx->ncsd_struct->partition_data[0].offset;
		ncch = fopen(ctx->core_infile.argument,"rb");
	}

	if(GetCXIStruct(&cxi_ctx,offset,ncch) != 0){
		MetaCleanup(&cxi_icon,ncch);
		return 1;
	}
	if(cxi_ctx.is_cfa == True){
		printf("[!] Content0 is not a CXI, Meta region cannot be generated\n");
		MetaCleanup(&cxi_icon,ncch);
		return 0;
	}
	if(ctx->flags[verbose])	{ printf(" > Retrieving Dependency List & Core Version\n"); }
	if(GetCXIMetaPreStruct(&meta_ctx,&cxi_ctx,ctx,offset,ncch) != 0){
		MetaCleanup(&cxi_icon,ncch);
		return 1;
	}
	if(ctx->flags[verbose])	{ printf(" > Retrieving Icon data\n"); }
	if(GetCXIIcon(&cxi_icon,&cxi_ctx,ctx,offset,ncch) != 0){
		MetaCleanup(&cxi_icon,ncch);
		return 1;
	}
		
	if(cxi_icon.size == 0)
		ctx->cia_section[meta].size = sizeof(META_STRUCT);
	else
		ctx->cia_section[meta].size = (sizeof(META_STRUCT) + cxi_icon.size);
	ctx->cia_section[meta].buffer = malloc(ctx->cia_section[meta].size);
	if(ctx->cia_section[meta].buffer == NULL){
		printf("[!] Memory Allocation Failure\n");
		return Fail;
	}
	//if(ctx->flags[verbose])	{ printf(" > Building CIA Metadata\n"); }
	memcpy(ctx->cia_section[meta].buffer,&meta_ctx,sizeof(META_STRUCT));
	if(cxi_icon.size > 0)
		memcpy((ctx->cia_section[meta].buffer + sizeof(META_STRUCT)),cxi_icon.buffer,cxi_icon.size);

	MetaCleanup(&cxi_icon,ncch);
	return 0;
}

void MetaCleanup(COMPONENT_STRUCT *icon, FILE *ncch)
{
	if(icon->size > 0){
		_free(icon->buffer);
	}
	fclose(ncch);
}
