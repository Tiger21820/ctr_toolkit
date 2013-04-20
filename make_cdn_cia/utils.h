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
#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1

void u8_hex_print_be(u8 *array, int len);
void u8_hex_print_le(u8 *array, int len);

void endian_strcpy(unsigned char destination[], unsigned char source[], int size, int endianness);

int makedir(const char* dir);

char *getcwdir(char *buffer,int maxlen);

u64 u8_to_u64(u8 *value, u8 endianness);
u32 u8_to_u32(u8 *value, u8 endianness);
u16 u8_to_u16(u8 *value, u8 endianness);
u32 align_value(u32 value, u32 alignment);
