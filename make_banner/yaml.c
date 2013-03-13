/********************************************************

YAML Interpeter

Author: 3DSGuy © 2013

********************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "yaml.h"

int key_scan_yaml(char *output, char tmp_store, int scan_size, FILE *file);
int get_value_string(char *output, char tmp_store, int scan_size, FILE *file);

int key_search(char *key, FILE *file)
{
	//Setting Up for search
	unsigned char *output;
	output = malloc(sizeof(unsigned char)*100);
	char tmp;
	
	//Scan for key
	do{
		if(key_scan_yaml(output,tmp,100,file) == EOF){
			free(output);
			return NOT_FOUND;
		}
	}while(strcmp(key,output) != 0);
	free(output);
	return FOUND;
}

int get_value(unsigned char *output, int max_size, char *key, FILE *file)
{
	if(key_search(key,file) == FOUND){
		char tmp;
		if(get_value_string(output,tmp,max_size,file) == NOT_FOUND)
			return VALUE_ERROR;
		return KEY_FOUND;
	}
	else
		return KEY_ERROR;
}
int get_boolean(char *key, FILE *file)
{
	int result = KEY_ERROR;
	if(key_search(key,file) == FOUND){
		unsigned char *output;
		output = malloc(sizeof(unsigned char)*10);
		char tmp;
		if(get_value_string(output,tmp,10,file) == NOT_FOUND)
			result = VALUE_ERROR;
		else if(strcmp(output,"true") == 0)
			result = TRUE;
		else if(strcmp(output,"false") == 0)
			result = FALSE;
		else
			result = VALUE_ERROR;
		free(output);
	}
	return result;
}

//Sub Functions

int get_value_string(char *output, char tmp_store, int scan_size, FILE *file)
{
	int search_state = SEEK_VALUE_START;
	for(int i = 0; i < scan_size; i++){
		switch(search_state){
			case(SEEK_VALUE_START):
				tmp_store = fgetc(file);
				if(tmp_store == 0x20 || tmp_store == 0x09){
					while(tmp_store == 0x20 || tmp_store == 0x09){tmp_store = fgetc(file);}
						if(tmp_store == '"'){
							i = -1;
							search_state = STORE_VALUE_QUO;
							break;
						}
						else if(tmp_store == EOF)
							return NOT_FOUND;
						else if(tmp_store == '#')
							return NOT_FOUND;
						else{
							i = -1;
							search_state = STORE_VALUE_REG;
							break;
						}
					}
					else if(tmp_store == '\n')
						return NOT_FOUND;
					else if(tmp_store == EOF)
						return NOT_FOUND;
					else
						return NOT_FOUND;
			case(STORE_VALUE_REG):
				if(tmp_store == 0x09 || tmp_store == 0x20 || tmp_store == 0x0a || tmp_store == '#' || i == scan_size || tmp_store == ':' || tmp_store == EOF){
					output[i] = '\0';
					return FOUND;
					break;
				}
				else{
					output[i] = tmp_store;
					tmp_store = fgetc(file);
				}
				break;
			case(STORE_VALUE_QUO):
				tmp_store = fgetc(file);
				if(tmp_store == '"' || i == scan_size || tmp_store == EOF){
					output[i] = '\0';
					return FOUND;
					break;
				}
				else
					output[i] = tmp_store;
			break;
		}
	}
}

int key_scan_yaml(char *output, char tmp_store, int scan_size, FILE *file)
{
	int search_state = SEEK_NEWLINE;
	for(int i = 0; i < scan_size; i++){
		/** Key Ground rules **
		* They do not have spaces(' ')
		* They are UTF-8 Strings
		* All characters between a '#' and a '\n' are to be ignored
		* A 'category' is not inclusive of the ':' character, hense also cannot have one 
		* Skip over '-' values
		****************************/
		switch(search_state){
			case(SEEK_NEWLINE): // This is done to skipover any key values
				tmp_store = fgetc(file);
				long int pos0 = ftell(file);
				if(pos0 == 0x1 && tmp_store != EOF){
					i = -1;
					search_state = SEEK_KEY_START;
					break;
				}
				if(tmp_store == '\n'){
					while(tmp_store == '\n')
						tmp_store = fgetc(file);
					i = -1;
					search_state = SEEK_KEY_START;
					break;
				}
				else if(tmp_store == EOF)
					return EOF;
				else
					i--;
				break;
			case(SEEK_KEY_START)://Scan for start of new key
				if(tmp_store == '#')
					while(tmp_store != '\n'){tmp_store = fgetc(file);}
				if(tmp_store == '-')
					while(tmp_store != '\n'){tmp_store = fgetc(file);}
				if(tmp_store == 0x09 || tmp_store == 0x20 || tmp_store == 0x0a){
					i = -1;
					tmp_store = fgetc(file);
				}
				else if(tmp_store == EOF)
					return EOF;
				else{
					i = -1;
					search_state = STORE_KEY;
				}
				break;
			case(STORE_KEY):// Key found, now store
				if(tmp_store == 0x09 || tmp_store == 0x20 || tmp_store == 0x0a || tmp_store == '#' || i == (scan_size - 1)|| tmp_store == ':'){
					output[i] = '\0';
					search_state = SEEK_COLON;
				}
				else if(tmp_store == EOF)
					return EOF;
				else{
					output[i] = tmp_store;
					tmp_store = fgetc(file);
				}
				break;
			case(SEEK_COLON)://Seek out next start point(after ':' symbols)
				if(tmp_store == ':')
					return 0;
				else if(tmp_store == EOF)
					return EOF;
				else
					tmp_store = fgetc(file);
			break;
		}
	}
}
