/********************************************************

Multi-Use API used by my CTR Toolchain

Author: 3DSGuy

********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"

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
