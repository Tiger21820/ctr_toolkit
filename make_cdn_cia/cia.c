/**
Copyright 2013 3DSGuy

This file is part of make_cdn_cia.

make_cdn_cia is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

make_cdn_cia is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with make_cdn_cia.  If not, see <http://www.gnu.org/licenses/>.
**/
#include "lib.h"
#include "cia.h"

int generate_cia(TMD_CONTEXT tmd_context, TIK_CONTEXT tik_context, FILE *output)
{
	write_cia_header(tmd_context,tik_context,output);
	write_cert_chain(tmd_context,tik_context,output);
	write_tik(tmd_context,tik_context,output);
	write_tmd(tmd_context,tik_context,output);
	write_content(tmd_context,tik_context,output);
	fclose(output);
	fclose(tik_context.tik);
	fclose(tmd_context.tmd);
	free(tmd_context.content_struct);
	free(tmd_context.content);
	return 0;
}

TIK_CONTEXT process_tik(FILE *tik)
{
	TIK_CONTEXT tik_context;
	memset(&tik_context,0x0,sizeof(tik_context));
	
	tik_context.tik = tik;
	
	u32 sig_size = get_sig_size(0x0,tik);
	if(sig_size == ERR_UNRECOGNISED_SIG){
		printf("[!] The CETK signature could not be recognised\n");
		tik_context.result = ERR_UNRECOGNISED_SIG;
		return tik_context;
	}
	
	TIK_STRUCT tik_struct = get_tik_struct(sig_size,tik);
	tik_context.tik_size = get_tik_size(sig_size);
	tik_context.title_version = u8_to_u16(tik_struct.title_version,BIG_ENDIAN);
	
	if(tik_context.tik_size == ERR_UNRECOGNISED_SIG){
		tik_context.result = ERR_UNRECOGNISED_SIG;
		return tik_context;
	}
	
	tik_context.cert_offset[0] = tik_context.tik_size;
	tik_context.cert_size[0] = get_cert_size(tik_context.tik_size,tik);
	tik_context.cert_offset[1] = tik_context.tik_size + tik_context.cert_size[0];
	tik_context.cert_size[1] = get_cert_size(tik_context.cert_offset[1],tik);
	
	if(tik_context.cert_size[0] == ERR_UNRECOGNISED_SIG || tik_context.cert_size[1] == ERR_UNRECOGNISED_SIG){
		printf("[!] One or both of the signatures in the CETK 'Cert Chain' are unrecognised\n");
		tik_context.result = ERR_UNRECOGNISED_SIG;
		return tik_context;
	}
	memcpy(tik_context.title_id,tik_struct.title_id,8);
	
	//printf("[+] CETK Title ID: "); u8_hex_print_be(tik_context.title_id,0x8); printf("\n");
	//printf("[+] CETK Size:     0x%x\n",tik_context.tik_size);
	//printf("[+] CERT Size:     0x%x\n",tik_context.cert_size);
	
	return tik_context;
}

TMD_CONTEXT process_tmd(FILE *tmd)
{
	TMD_CONTEXT tmd_context;
	memset(&tmd_context,0x0,sizeof(tmd_context));
	
	tmd_context.tmd = tmd;
	
	u32 sig_size = get_sig_size(0x0,tmd);
	if(sig_size == ERR_UNRECOGNISED_SIG){
		printf("[!] The TMD signature could not be recognised\n");
		tmd_context.result = ERR_UNRECOGNISED_SIG;
		return tmd_context;
	}
	
	
	TMD_STRUCT tmd_struct = get_tmd_struct(sig_size,tmd);
	tmd_context.content_count = u8_to_u16(tmd_struct.content_count,BIG_ENDIAN);
	tmd_context.tmd_size = get_tmd_size(sig_size,tmd_context.content_count);
	tmd_context.title_version = u8_to_u16(tmd_struct.title_version,BIG_ENDIAN);
	
	tmd_context.cert_offset[0] = tmd_context.tmd_size;
	tmd_context.cert_size[0] = get_cert_size(tmd_context.tmd_size,tmd);
	tmd_context.cert_offset[1] = tmd_context.tmd_size + tmd_context.cert_size[0];
	tmd_context.cert_size[1] = get_cert_size(tmd_context.cert_offset[1],tmd);
	
	if(tmd_context.cert_size[0] == ERR_UNRECOGNISED_SIG || tmd_context.cert_size[1] == ERR_UNRECOGNISED_SIG){
		printf("[!] One or both of the signatures in the TMD 'Cert Chain' are unrecognised\n");
		tmd_context.result = ERR_UNRECOGNISED_SIG;
		return tmd_context;
	}
	memcpy(tmd_context.title_id,tmd_struct.title_id,8);
	
	tmd_context.content_struct = malloc(sizeof(TMD_CONTENT_CHUNK_STRUCT)*tmd_context.content_count);
	tmd_context.content = malloc(0x4*tmd_context.content_count);
	for(u8 i = 0; i < tmd_context.content_count; i++){
		tmd_context.content_struct[i] = get_tmd_content_struct(sig_size,i,tmd);
		u8 content_id[16];
		sprintf(content_id,"%08x",get_content_id(tmd_context.content_struct[i]));
		
		#ifdef _WIN32 //Windows is not Case Sensitive
		tmd_context.content[i] = fopen(content_id,"rb");
		if(tmd_context.content[i] == NULL){
			printf("[!] Content: '%s' could not be opened\n",content_id);
			tmd_context.result = IO_FAIL;
			return tmd_context;
		}
		#else //Everything else is case sensitive. 
		tmd_context.content[i] = fopen(content_id,"rb");
		if(tmd_context.content[i] == NULL){
			for(int i = 0; i < 16; i++){
				if(islower(content_id[i]) != 0 && isalpha(content_id[i]) != 0)
					content_id[i] = toupper(content_id[i]);
			}
			tmd_context.content[i] = fopen(content_id,"rb");
			if(tmd_context.content[i] == NULL){
				printf("[!] Content: '%s' could not be opened\n",content_id);
				tmd_context.result = IO_FAIL;
				return tmd_context;
			}
		}
		#endif
		//print_content_chunk_info(tmd_context.content_struct[i]);
	}
	return tmd_context;
}

CIA_HEADER set_cia_header(TMD_CONTEXT tmd_context, TIK_CONTEXT tik_context)
{
	CIA_HEADER cia_header;
	memset(&cia_header,0,sizeof(cia_header));
	cia_header.header_size = sizeof(CIA_HEADER);
	cia_header.type = 0;
	cia_header.version = 0;
	cia_header.cert_size = get_total_cert_size(tmd_context,tik_context);
	cia_header.tik_size = tik_context.tik_size;
	cia_header.tmd_size = tmd_context.tmd_size;
	cia_header.meta_size = 0;
	cia_header.content_size = get_content_size(tmd_context);
	u64 index = 0;
	for(int i = 0; i < tmd_context.content_count; i++){
		index += (0x8000000000000000/(2<<u8_to_u16(tmd_context.content_struct[i].content_index,BE)))*2;
	}
	u64_to_u8(cia_header.content_index,index,BE);
	return cia_header;
}

u32 get_tik_size(u32 sig_size)
{
	return (0x4 + sig_size + sizeof(TIK_STRUCT));
}

u32 get_tmd_size(u32 sig_size, u16 content_count)
{
	return (0x4 + sig_size + sizeof(TMD_STRUCT) + sizeof(TMD_CONTENT_CHUNK_STRUCT)*content_count);
}

u32 get_sig_size(u32 offset, FILE *file)
{
	fseek(file,offset,SEEK_SET);
	u32 sig_type;
	fread(&sig_type,0x4,1,file);
	switch(sig_type){
		/**
		case(RSA_4096_SHA1): return 0x200;
		case(RSA_2048_SHA1): return 0x100;
		case(Elliptic_Curve_0): return 0x3C;
		**/
		case(RSA_4096_SHA256): return 0x200;
		case(RSA_2048_SHA256): return 0x100;
		//case(Elliptic_Curve_1): return 0x3C;
	}
	return ERR_UNRECOGNISED_SIG;
}

u32 get_cert_size(u32 offset, FILE *file)
{
	u32 sig_size = get_sig_size(offset,file);
	if(sig_size == ERR_UNRECOGNISED_SIG)
		return ERR_UNRECOGNISED_SIG;
	return (0x4+sig_size+sizeof(CERT_2048KEY_DATA_STRUCT));
}

u32 get_total_cert_size(TMD_CONTEXT tmd_context, TIK_CONTEXT tik_context)
{
	return (tik_context.cert_size[1] + tik_context.cert_size[0] + tmd_context.cert_size[0]);
}

u64 get_content_size(TMD_CONTEXT tmd_context)
{
	u64 content_size = 0x0;
	for(int i = 0; i < tmd_context.content_count; i++)
		content_size += read_content_size(tmd_context.content_struct[i]);
	return content_size;
}

u64 read_content_size(TMD_CONTENT_CHUNK_STRUCT content_struct)
{
	return u8_to_u64(content_struct.content_size,BIG_ENDIAN);
}

u32 get_content_id(TMD_CONTENT_CHUNK_STRUCT content_struct)
{
	return u8_to_u32(content_struct.content_id,BIG_ENDIAN);
}

int write_cia_header(TMD_CONTEXT tmd_context, TIK_CONTEXT tik_context, FILE *output)
{
	CIA_HEADER cia_header = set_cia_header(tmd_context,tik_context);
	fseek(output,0x0,SEEK_SET);
	fwrite(&cia_header,sizeof(cia_header),1,output);
	return 0;
}

int write_cert_chain(TMD_CONTEXT tmd_context, TIK_CONTEXT tik_context, FILE *output)
{
	u8 cert[0x1000];
	
	//Seeking Offset in output
	u32 offset = align_value(sizeof(CIA_HEADER),0x40);
	fseek(output,offset,SEEK_SET);
	
	//The order of Certs in CIA goes, Root Cert, Cetk Cert, TMD Cert. In CDN format each file has it's own cert followed by a Root cert
	
	//Taking Root Cert from Cetk Cert chain(can be taken from TMD Cert Chain too)
	memset(cert,0x0,tik_context.cert_size[1]);
	fseek(tik_context.tik,tik_context.cert_offset[1],SEEK_SET);
	fread(&cert,tik_context.cert_size[1],1,tik_context.tik);
	fwrite(&cert,tik_context.cert_size[1],1,output);
	
	//Writing Cetk Cert
	memset(cert,0x0,tik_context.cert_size[0]);
	fseek(tik_context.tik,tik_context.cert_offset[0],SEEK_SET);
	fread(&cert,tik_context.cert_size[0],1,tik_context.tik);
	fwrite(&cert,tik_context.cert_size[0],1,output);
	
	//Writing TMD Cert
	memset(cert,0x0,tmd_context.cert_size[0]);
	fseek(tmd_context.tmd,tmd_context.cert_offset[0],SEEK_SET);
	fread(&cert,tmd_context.cert_size[0],1,tmd_context.tmd);
	fwrite(&cert,tmd_context.cert_size[0],1,output);
	
	return 0;
}

int write_tik(TMD_CONTEXT tmd_context, TIK_CONTEXT tik_context, FILE *output)
{
	u8 tik[tik_context.tik_size];
	
	u32 cert_size = get_total_cert_size(tmd_context,tik_context);
	
	//Seeking Offset in output
	u32 offset = align_value(get_total_cert_size(tmd_context,tik_context),0x40) + align_value(sizeof(CIA_HEADER),0x40);
	fseek(output,offset,SEEK_SET);
	
	memset(tik,0x0,tik_context.tik_size);
	fseek(tik_context.tik,0x0,SEEK_SET);
	fread(&tik,tik_context.tik_size,1,tik_context.tik);
	fwrite(&tik,tik_context.tik_size,1,output);
	
	return 0;
}

int write_tmd(TMD_CONTEXT tmd_context, TIK_CONTEXT tik_context, FILE *output)
{
	//Seeking Offset in output
	u32 offset = align_value(tik_context.tik_size,0x40) + align_value(get_total_cert_size(tmd_context,tik_context),0x40) + align_value(sizeof(CIA_HEADER),0x40);
	fseek(output,offset,SEEK_SET);

	u8 tmd[tmd_context.tmd_size];
	memset(tmd,0x0,tmd_context.tmd_size);
	fseek(tmd_context.tmd,0x0,SEEK_SET);
	fread(&tmd,tmd_context.tmd_size,1,tmd_context.tmd);
	fwrite(&tmd,tmd_context.tmd_size,1,output);
	
	return 0;
}

int write_content(TMD_CONTEXT tmd_context, TIK_CONTEXT tik_context, FILE *output)
{
	//Seeking Offset in output
	u32 offset = align_value(tmd_context.tmd_size,0x40) + align_value(tik_context.tik_size,0x40) + align_value(get_total_cert_size(tmd_context,tik_context),0x40) + align_value(sizeof(CIA_HEADER),0x40);
	fseek(output,offset,SEEK_SET);
	
	for(int i = 0; i < tmd_context.content_count; i++){
		write_content_data(tmd_context.content[i],read_content_size(tmd_context.content_struct[i]),output);
	}
	return 0;
}

int write_content_data(FILE *content, u64 content_size, FILE *output)
{
	u32 buffer_size = 0x100000;
	u8 buffer[buffer_size];
	memset(&buffer,0x0,buffer_size);
	while(content_size > buffer_size){
		memset(&buffer,0x0,buffer_size);
		fread(&buffer,buffer_size,1,content);
		fwrite(&buffer,buffer_size,1,output);
		content_size -= buffer_size;
	}
	memset(&buffer,0x0,content_size);
	fread(&buffer,content_size,1,content);
	fwrite(&buffer,content_size,1,output);
	
}

TIK_STRUCT get_tik_struct(u32 sig_size, FILE *tik)
{
	TIK_STRUCT tik_struct;
	fseek(tik,(0x4+sig_size),SEEK_SET);
	fread(&tik_struct,sizeof(tik_struct),1,tik);
	return tik_struct;
}

TMD_STRUCT get_tmd_struct(u32 sig_size, FILE *tmd)
{
	TMD_STRUCT tmd_struct;
	fseek(tmd,(0x4+sig_size),SEEK_SET);
	fread(&tmd_struct,sizeof(tmd_struct),1,tmd);
	return tmd_struct;
}

TMD_CONTENT_CHUNK_STRUCT get_tmd_content_struct(u32 sig_size, u8 index, FILE *tmd)
{
	fseek(tmd,(0x4+sig_size+sizeof(TMD_STRUCT)+sizeof(TMD_CONTENT_CHUNK_STRUCT)*index),SEEK_SET);
	TMD_CONTENT_CHUNK_STRUCT content_struct;
	fread(&content_struct,sizeof(content_struct),1,tmd);
	return content_struct;
}

void print_content_chunk_info(TMD_CONTENT_CHUNK_STRUCT content_struct)
{
	printf("\n[+] Content ID:    %08x\n",u8_to_u32(content_struct.content_id,BIG_ENDIAN));
	printf("[+] Content Index: %d\n",u8_to_u16(content_struct.content_index,BIG_ENDIAN));
	printf("[+] Content Type:  %d\n",u8_to_u16(content_struct.content_type,BIG_ENDIAN));
	printf("[+] Content Size:  0x%x\n",u8_to_u64(content_struct.content_size,BIG_ENDIAN));
	printf("[+] SHA-256 Hash:  "); u8_hex_print_be(content_struct.sha_256_hash,0x20); printf("\n");
}

int check_tid(u8 *tid_0, u8 *tid_1)
{
	for(int i = 0; i < 8; i++){
		if(tid_0[i] != tid_1[i])
			return FALSE;
	}
	return TRUE;
}