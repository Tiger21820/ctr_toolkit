#include "lib.h"
#include "ctr_crypto.h"
#include "meta.h"
#include "cia.h"
#include "ncch.h"

int GenerateMeta(CIA_CONTEXT *ctx)
{
	if(ctx->meta_flag == FALSE)
		return 0;
	
	//Get Meta Section
	CXI_STRUCT cxi_ctx;
	META_STRUCT meta;
	COMPONENT_STRUCT cxi_icon;
	memset(&cxi_ctx,0x0,sizeof(CXI_STRUCT));
	memset(&meta,0x0,sizeof(META_STRUCT));
	memset(&cxi_icon,0x0,sizeof(COMPONENT_STRUCT));
	FILE *ncch = fopen(GetContentFilePath(&ctx->ContentInfo[0]),"rb");
	if(GetCXIStruct(&cxi_ctx,ncch) != 0)
		goto fail_cleanup;
	if(GetCXIMetaPreStruct(&meta,&cxi_ctx,ctx,ncch) != 0)
		goto fail_cleanup;
	if(GetCXIIcon(&cxi_icon,&cxi_ctx,ctx,ncch) != 0)
		goto fail_cleanup;
		
	ctx->meta.used = TRUE;
	if(cxi_icon.used == FALSE)
		ctx->meta.size = sizeof(META_STRUCT);
	else
		ctx->meta.size = (sizeof(META_STRUCT) + cxi_icon.size);
	ctx->meta.buffer = malloc(ctx->meta.size);
	
	memcpy(ctx->meta.buffer,&meta,sizeof(META_STRUCT));
	if(cxi_icon.used == TRUE)
		memcpy((ctx->meta.buffer + sizeof(META_STRUCT)),cxi_icon.buffer,cxi_icon.size);
	
	
	
	if(cxi_icon.used == TRUE){
		free(cxi_icon.buffer);
	}
	fclose(ncch);
	return 0;
fail_cleanup:
	if(cxi_icon.used == TRUE){
		free(cxi_icon.buffer);
	}
	fclose(ncch);
	return 1;
}