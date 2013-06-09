#include "lib.h"
#include "ctr_crypto.h"
#include "meta.h"
#include "cia.h"
#include "ncsd.h"

int GenerateMeta(CIA_CONTEXT *ctx)
{	
	if(ctx->meta_flag == False)
		return 0;
	
	//Get Meta Section
	NCCH_STRUCT cxi_ctx;
	META_STRUCT meta;
	COMPONENT_STRUCT cxi_icon;
	memset(&cxi_ctx,0x0,sizeof(NCCH_STRUCT));
	memset(&meta,0x0,sizeof(META_STRUCT));
	memset(&cxi_icon,0x0,sizeof(COMPONENT_STRUCT));
	u32 offset;
	FILE *ncch;
	if(ctx->ncsd_convert_flag == False){
		offset = 0;
		ncch = fopen(ctx->ContentInfo[0].file_path,"rb");
	}
	else{
		offset = ctx->ncsd_struct->partition_data[0].offset;
		ncch = ctx->ncsdfile.file.file;
	}
	
	if(GetCXIStruct(&cxi_ctx,offset,ncch) != 0)
		goto fail_cleanup;
	if(cxi_ctx.is_cfa == True){
		printf("[!] Content0 is not a CXI, Meta region cannot be generated\n");
		goto normal_cleanup;
	}
	if(GetCXIMetaPreStruct(&meta,&cxi_ctx,ctx,offset,ncch) != 0)
		goto fail_cleanup;
	if(GetCXIIcon(&cxi_icon,&cxi_ctx,ctx,offset,ncch) != 0)
		goto fail_cleanup;
		
	ctx->meta.used = True;
	if(cxi_icon.used == False)
		ctx->meta.size = sizeof(META_STRUCT);
	else
		ctx->meta.size = (sizeof(META_STRUCT) + cxi_icon.size);
	ctx->meta.buffer = malloc(ctx->meta.size);
	
	memcpy(ctx->meta.buffer,&meta,sizeof(META_STRUCT));
	if(cxi_icon.used == True)
		memcpy((ctx->meta.buffer + sizeof(META_STRUCT)),cxi_icon.buffer,cxi_icon.size);
	
normal_cleanup:
	if(cxi_icon.used == True){
		free(cxi_icon.buffer);
	}
	fclose(ncch);
	return 0;
fail_cleanup:
	if(cxi_icon.used == True){
		free(cxi_icon.buffer);
	}
	fclose(ncch);
	return 1;
}