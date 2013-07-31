/**
Copyright 2013 3DSGuy

This file is part of make_cia.

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
typedef struct
{
	u8 header_size[4];
	u8 type[2];
	u8 version[2];
	u8 cert_size[4];
	u8 tik_size[4];
	u8 tmd_size[4];
	u8 meta_size[4];
	u8 content_size[8];
	u8 content_index[0x2000];
} __attribute__((__packed__)) 
CIA_HEADER;

int SetupContent(USER_CONTEXT *ctx);
int SetupCIAHeader(USER_CONTEXT *ctx);
int WriteSectionsToOutput(USER_CONTEXT *ctx);
int EncryptContent(u8 *EncBuffer,u8 *buffer,u64 size,u8 *title_key, u16 index);
u64 GetContentPaddingSize(u64 ContentSize, u32 alignment);