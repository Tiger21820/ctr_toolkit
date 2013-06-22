#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>

const unsigned char null_entry[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

void app_title(void);
void help(char *app_name);

void main(int argc, char *argv[])
{
	if(argc != 4){
		printf("[!] Bad arguments\n");
		help(argv[0]);
		return;
	}
	FILE *cache = fopen(argv[1],"rb");
	FILE *cacheD = fopen(argv[2],"rb");
	
	chdir(argv[3]);
	
	unsigned char *buffer = malloc(0x36c0);
	for(int i = 0; i < 360/*360*/; i++){
		memset(buffer,0,0x36c0);
		fseek(cache,0x8+(0x10*i),SEEK_SET);
		fseek(cacheD,0x0+(0x36c0*i),SEEK_SET);
		unsigned char tmp[8];
		fread(&tmp,8,1,cache);
		if(memcmp(&tmp,null_entry,8) != 0){
			char iconname[30];
			sprintf(iconname,"%02x%02x%02x%02x%02x%02x%02x%02x.icn",tmp[7],tmp[6],tmp[5],tmp[4],tmp[3],tmp[2],tmp[1],tmp[0]);
			fread(buffer,0x36c0,1,cacheD);
			FILE *icon = fopen(iconname,"wb");
			fwrite(buffer,0x36c0,1,icon);
			fclose(icon);
		}
	}
	
	fclose(cache);
	fclose(cacheD);
	printf("Done\n");
}

void app_title(void)
{
	printf("CTR_Toolkit - HomeMenu ICN Cache Tool\n");
	printf("Version 1.0 (C) 3DSGuy 2013\n");
}

void help(char *app_name)
{
	app_title();
	printf("\nUsage: <Cache.dat> <CacheD.dat> <Out-Dir>\n", app_name);
}