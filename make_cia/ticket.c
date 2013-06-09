#include "lib.h"
#include "ctr_crypto.h"
#include "ticket.h"

int GenerateTicket(CIA_CONTEXT *ctx)
{
	ctx->ticket.used = True;
	ctx->ticket.size = (sizeof(TICKET_STRUCTURE)+ sizeof(TIK_2048_SIG_CONTEXT));
	ctx->ticket.buffer = malloc(ctx->ticket.size);
	if(ctx->ticket.buffer == NULL){
		printf("[!] Failed to allocated memory for ticket\n");
		return 1;
	}
	
	TICKET_STRUCTURE ticket;
	TIK_2048_SIG_CONTEXT sig;
	memset(&sig,0x0,sizeof(TIK_2048_SIG_CONTEXT));
	memset(&ticket,0x0,sizeof(TICKET_STRUCTURE));
	
	ticket.TicketFormatVersion = ctx->core.ticket_format_ver;
	ticket.ca_crl_version = ctx->core.ca_crl_version;
	ticket.signer_crl_version = ctx->core.signer_crl_version;
	ticket.CommonKeyID = ctx->keys.common_key_id;
	
	memcpy(ticket.Issuer,ctx->core.TicketIssuer,0x40);
	memcpy(ticket.TicketID,ctx->core.TicketID,0x8);
	memcpy(ticket.DeviceID,ctx->core.DeviceID,0x4);
	memcpy(ticket.TitleID,ctx->core.TitleID,0x8);
	memcpy(ticket.TicketVersion,ctx->core.TicketVersion,0x2);
	
	if(EncryptTitleKey(ticket.EncryptedTitleKey,ctx->keys.title_key.key,ctx->keys.common_key.key,ctx->core.TitleID) != 0){
		printf("[!] Failed to encrypt titlekey\n");
		return Fail;
	}
	if(ctx->showkeys_flag)
		memdump(stdout,"\n[+] Encrypted Title Key:   ",ticket.EncryptedTitleKey,0x10);
	
	if(SetStaticData(dev,ticket.StaticData) != 0){
		printf("[!] ERROR in Generating Ticket\n");
		return Fail;
	}
	
	u32_to_u8(sig.sig_type,RSA_2048_SHA256,BE);
	u8 hash[0x20];
	ctr_sha_256(&ticket,sizeof(TICKET_STRUCTURE),hash);
	if(ctr_rsa2048_sha256_sign(hash,sig.data,ctx->keys.ticket.n,ctx->keys.ticket.d) != Good){
		printf("[!] Failed to sign ticket\n");
		return ticket_gen_fail;
	}
	
	if(ctx->verbose_flag){
		memdump(stdout,"[+] Ticket Signature:   ",sig.data,0x100);
	}
	
	memcpy(ctx->ticket.buffer,&sig,sizeof(TIK_2048_SIG_CONTEXT));
	memcpy((ctx->ticket.buffer + sizeof(TIK_2048_SIG_CONTEXT)),&ticket,sizeof(TICKET_STRUCTURE));
	return 0;
}

int EncryptTitleKey(u8 EncTitleKey[0x10], u8 *DecTitleKey, u8 *CommonKey, u8 *TitleID)
{
	//generating IV
	u8 iv[16];
	memset(&iv,0x0,16);
	memcpy(iv,TitleID,0x8);
	
	
	//Encrypting titlekey
	ctr_aes_context ctx;
	memset(&ctx,0x0,sizeof(ctr_aes_context));
	
	ctr_init_aes_cbc(&ctx,CommonKey,iv,ENC);
	ctr_aes_cbc(&ctx,DecTitleKey,EncTitleKey,0x10,ENC);
		
	//checking titlekey encrypted properly
	u8 tmp[0x10];
	ctr_init_aes_cbc(&ctx,CommonKey,iv,DEC);
	ctr_aes_cbc(&ctx,EncTitleKey,tmp,0x10,DEC);
	
	/**
	memdump(stdout, "TitleKey:        ", DecTitleKey, 0x10);
	memdump(stdout, "Enc_TitleKey:    ", EncTitleKey, 0x10);
	memdump(stdout, "Dec_TitleKey:    ", tmp, 0x10);
	**/
	
	return memcmp(DecTitleKey,tmp,0x10);
}

int SetStaticData(u8 mode, u8 section[0x30])
{
	switch(mode){
		case dev : memcpy(section,dev_static_ticket_data,0x30); break;
		case prod : memcpy(section,prod_static_ticket_data,0x30); break;
		case test : memset(section,0xff,0x30); break;
		default: printf("[!] Mode not recogised\n"); return ticket_gen_fail;
	}
	return 0;
}