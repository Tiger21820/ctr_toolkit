/********************************************************

Multi-Use API used by my CTR Toolchain

Author: 3DSGuy

This file is part of make_banner.

make_banner is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

make_banner is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with make_banner.  If not, see <http://www.gnu.org/licenses/>.

********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"

void resolve_flag(unsigned char flag, unsigned char *flag_bool)
{
	unsigned char bit_mask[8] = {0x80,0x40,0x20,0x10,0x8,0x4,0x2,0x1};
	for(int i = 0; i < 8; i++){
		if (flag >= bit_mask[i]){
			flag_bool[7-i] = TRUE;
			flag -= bit_mask[i];
		}
		else
			flag_bool[7-i] = FALSE;
	}
}

void char_to_int_array(unsigned char destination[], unsigned char source[], int size, int endianness, int base)
{	
	unsigned char tmp[size][2];
    	unsigned char *byte_array = malloc(size*sizeof(unsigned char));
	memset(byte_array, 0, size);
	memset(destination, 0, size);
	memset(tmp, 0, size*2);
    
    	for (int i = 0; i < size; i ++){
        	tmp[i][0] = source[(i*2)];
        	tmp[i][1] = source[((i*2)+1)];
		tmp[i][2] = '\0';
        	byte_array[i] = (unsigned char)strtol(tmp[i], NULL, base);
    	}
	for (int i = 0; i < size; i++){
        	switch (endianness){
        		case(BIG_ENDIAN):
        		destination[i] = byte_array[i];
        		break;
        		case(LITTLE_ENDIAN):
        		destination[i] = byte_array[((size-1)-i)];
        		break;
        	}
    	}
	free(byte_array);
}

void endian_strcpy(unsigned char destination[], unsigned char source[], int size, int endianness)
{ 
    for (int i = 0; i < size; i++){
        switch (endianness){
            case(BIG_ENDIAN):
                destination[i] = source[i];
                break;
            case(LITTLE_ENDIAN):
                destination[i] = source[((size-1)-i)];
                break;
        }
    }
}

char *search(char s[], int n, FILE *file)
{
	for(int i = 0, tmp; i < n; i++){
		tmp = fgetc(file);
		//printf("%d", tmp);
		if(tmp == '\n' || i == (n - 1) || tmp == '#'){ //newlines & reached string max length
			s[i] = '\0';
			break;
		}
		else if(tmp == 0x09){ //tabs
			i--; //so the string doesn't look weird 	
		}
		else if(tmp == EOF){ //can't have EOF strings
			s[0] = tmp;
			break;
		}
		else{ //what I want
			s[i] = tmp;
		}
	}
	if(s[0] == EOF){
		memset(s, 0x00, sizeof(s));
	}
	return s;
}

void value_find_fail(const char key[]){
	printf("[!] %s Was Not Specified\n", key);
}
void key_find_fail(const char category[]){
	printf("[!] %s Was Not Found\n", category);
}
