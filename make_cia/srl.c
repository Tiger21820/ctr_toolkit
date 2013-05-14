#include "lib.h"
#include "ctr_crypto.h"
#include "srl.h"

int GetCoreContentSRL(CORE_CONTENT_INFO *core,u8 SRLType,FILE *srl)
{
	SRL_HEADER header;
	memset(&header,0x0,sizeof(SRL_HEADER));
	
	fseek(srl,0x0,SEEK_SET);
	fread(&header,sizeof(SRL_HEADER),1,srl);
	u16 tid_platform = 0;
	switch(SRLType){
		case(NTR): GetNTRTitleID(header.game_code,core->TitleID); break;
		case(TWL): 
			fseek(srl,0x236,SEEK_SET);
			fread(&tid_platform,0x2,1,srl);
			GetTWLTitleID(header.title_id,core->TitleID); 
			break;
	}
	if(tid_platform != 0x0003 && SRLType == TWL){
		printf("[!] Is not a valid TWL file\n");
		return 1;
	}
	
	endian_strcpy(core->TitleVersion,header.rom_version,0x2,LITTLE_ENDIAN);
	
	return 0;
}

void GetNTRTitleID(u8 *game_code, u8 *title_id)
{
	u8 title_id_std[8] = {0x00,0x04,0x80,0x04,0x00,0x00,0x00,0x00};
	for(int i = 0; i < 4; i++){
		title_id_std[i+4] = game_code[i];
	}
	memcpy(title_id,title_id_std,0x8);
}

void GetTWLTitleID(u8 *title_id_old, u8 *title_id)
{
	u8 title_id_std[8] = {0x00,0x04,0x80,0x00,0x00,0x00,0x00,0x00};
	u8 title_id_old_BE[8];
	endian_strcpy(title_id_old_BE,title_id_old,0x8,LITTLE_ENDIAN);
	for(int i = 2; i < 8; i++){
		title_id_std[i] += title_id_old_BE[i];
	}
	memcpy(title_id,title_id_std,0x8);
}