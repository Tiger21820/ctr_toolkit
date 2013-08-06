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
#include "srl.h"

int GetCoreContentSRL(CORE_CONTENT_INFO *core,FILE *srl)
{
	SRL_HEADER header;
	memset(&header,0x0,sizeof(SRL_HEADER));
	
	fseek(srl,0x0,SEEK_SET);
	fread(&header,sizeof(SRL_HEADER),1,srl);
	u16 tid_platform = 0;	
	fseek(srl,0x236,SEEK_SET);
	fread(&tid_platform,0x2,1,srl);
	if(tid_platform != 0x0003){
		printf("[!] Is not a valid TWL SRL file\n");
		return 1;
	}
	
	GetTWLTitleID(header.title_id,core->TitleID); 
	
	u8 flag_bool[8];
	resolve_flag(header.reserved_flags[3],flag_bool);
	for(int i = 0; i < 3; i++){
		if(flag_bool[i] == True)
			core->twl_flag += i;
	}
	
	
	u16 ver = (u8_to_u16(header.rom_version,LE)*4);
	
	u16_to_u8(core->TitleVersion,ver,BE);
	memcpy(core->save_data_size,header.pub_save_data_size,4);
	memcpy(core->priv_save_data_size,header.priv_save_data_size,4);
	
	return 0;
}

void GetTWLTitleID(u8 *title_id_old, u8 *title_id)
{
	u8 title_id_std[8] = {0x00,0x04,0x80,0x00,0x00,0x00,0x00,0x00};
	u8 title_id_old_BE[8];
	endian_memcpy(title_id_old_BE,title_id_old,0x8,LE);
	for(int i = 2; i < 8; i++){
		if(i == 2 && title_id_old_BE[i] > 0x3F)
			title_id_std[i] += 0x3F;
		else
			title_id_std[i] += title_id_old_BE[i];
	}
	memcpy(title_id,title_id_std,0x8);
}