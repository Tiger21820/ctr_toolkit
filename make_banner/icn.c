#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "icn.h"
#include "utils.h"

int icn_main(ICN_CONTEXT icn)
{
    //fill out file to ICN size
    fseek(icn.output, (ICN_SIZE - 1), SEEK_SET);
    fputc(0x00, icn.output);

    //Write Magic
    u8 magic[4] = "SMDH";
    fseek(icn.output, 0x0, SEEK_SET);
    fwrite(&magic, 4, 1, icn.output);

    //Write Title Data
    icn_title_proccess(icn);

    //Write ICN Settings
    icn_settings_proccess(icn);

    //Write Icon Data
    icn_icon_proccess(icn);

    return 0;
}

int icn_title_proccess(ICN_CONTEXT icn)
{
    /** Set Identity Array **/
	u8 language_string[MAX_TITLE_NUM][15];
	strcpy(language_string[0], "Japanese");
	strcpy(language_string[1], "English");
	strcpy(language_string[2], "French");
	strcpy(language_string[3], "German");
	strcpy(language_string[4], "Italian");
	strcpy(language_string[5], "Spanish");
	strcpy(language_string[6], "Chinese");
	strcpy(language_string[7], "Korean");
	strcpy(language_string[8], "Dutch");
	strcpy(language_string[9], "Portuguese");
	strcpy(language_string[10], "Russian");
	
	/** Temporary Arrays **/	
	u8 *short_title = malloc(sizeof(u8)*SIZE_SHORT_TITLE);
	u8 *long_title = malloc(sizeof(u8)*SIZE_LONG_TITLE);
	u8 *publisher = malloc(sizeof(u8)*SIZE_PUBLISHER_TITLE);

	/** File Position Recorders **/
	long int pos0;
	long int pos1;
	
	/** Read Title Strings **/
	rewind(icn.bsf);
	if(key_search("ApplicationTitleData", icn.bsf) == FOUND){
		pos0 = ftell(icn.bsf);
		for(int i = 0; i < MAX_TITLE_NUM; i++){
			fseek(icn.bsf, pos0, SEEK_SET);
			if(key_search(language_string[i], icn.bsf) == FOUND){
				pos1 = ftell(icn.bsf);
				/**
				for(int j = 0; j < 3; j++){
					fseek(icn.bsf, pos1, SEEK_SET);
					if(get_value(title_set[j],title_value_size[j],title_key[j],icn.bsf) != FOUND){
						printf("[!] Warning %s was not specified for %s\n",title_key[j],language_string[i]);
					}
				}
				**/
				
				if(get_value(short_title,SIZE_SHORT_TITLE,"ShortTitle",icn.bsf) != 0){
					printf("[!]Warning, No 'ShortTitle' string for %s\n",language_string[i]);
				}
				fseek(icn.bsf, pos1, SEEK_SET);
			
				if(get_value(long_title,SIZE_LONG_TITLE,"LongTitle",icn.bsf) != 0){
					printf("[!]Warning, No 'LongTitle' string for %s\n",language_string[i]);
				}
				fseek(icn.bsf, pos1, SEEK_SET);
				
				if(get_value(publisher,SIZE_PUBLISHER_TITLE,"Publisher",icn.bsf) != 0){
					printf("[!]Warning, No 'Publisher' string for %s\n",language_string[i]);
				}
				//Writing Language Strings to Data Struct
				icn.titles.title_array[i] = string_ICN_conv(short_title,long_title,publisher);
				
			}
			else{
				printf("[!] Warning, Could Not Find Title Strings for %s, i = %d\n",language_string[i], i);
			}
		}
	}
	else{
		key_find_fail("ApplicationTitleData");
		printf("return fail = %d\n",TITLE_FAIL);
		return TITLE_FAIL;
	}
	free(short_title);
	free(long_title);
	free(publisher);
	
	/** Write To File **/
	fseek(icn.output, TITLE_OFFSET, SEEK_SET);
	fwrite(&icn.titles, sizeof(icn.titles), 1, icn.output);
    return 0;
}

int icn_settings_proccess(ICN_CONTEXT icn)
{
	/** Creating Buffer **/	
	u8 buff[100];
	
	/** Processing Bitmask Flags **/
	//Creating Identity Arrays
	u8 bit_flag_key[MAX_BIT_NUM][20] = {"Base", "AutoBoot", "Flag3DEffect", "RequireAcceptEULA", "AutoSaveOnExit", "UseExtendedBanner", "UseAgeRestrictions", "UseSaveData"};
	u8 bit_flag_value[MAX_BIT_NUM] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
	
	//Creating bit flag 0 counter
	u8 bit_flag[MAX_BIT_NUM];
	//memset(bit_flag, 0, MAX_BIT_NUM);
	
	//Setting Static Flags
	icn.settings.byte_flag[0] = 0x01;
	icn.settings.byte_flag[1] = 0x01; // Some Dev Apps have this byte flag set to 0x00
	icn.settings.byte_flag[2] = 0x00;
	icn.settings.byte_flag[3] = 0x00;
	icn.settings.byte_flag[4] = 0x00;
	icn.settings.byte_flag[5] = 0x01;
	icn.settings.byte_flag[6] = 0x00;
	icn.settings.byte_flag[7] = 0x00;
	
	//Region Age Rating flag
	int use_age_ratings = FALSE;
	
	fseek(icn.bsf, 0x00, SEEK_SET);
	//Current Research allows only 'icn.settings.byte_flag[0]' to be customised...
	if(key_search("Options", icn.bsf) == FOUND){
		if(key_search("Flags", icn.bsf) == FOUND){
			long int pos0 = ftell(icn.bsf);
			for(int i = 1; i < MAX_BIT_NUM; i++){
				fseek(icn.bsf, pos0, SEEK_SET);
				if (get_boolean(bit_flag_key[i], icn.bsf) == TRUE){
					icn.settings.byte_flag[0] += bit_flag_value[i];
					if(i == 6)
						use_age_ratings = TRUE;
				}
			}
		}
		else{
			key_find_fail("Flags");
			return FLAG_FAIL;
		}
	}
	else{
		key_find_fail("Options");
		return FLAG_FAIL;
	}
	
	
	/** Processing Region Ratings **/
		/*** Setting Up Array for Storing the processed Age Restrictions ***/
	u8 region_rating[MAX_RATING_NUM];
	//memset(region_rating, 0x00, sizeof(region_rating));
		
		/*** Seting Up Region Key Array and Input Array***/
	u8 region_rating_key[MAX_RATING_NUM][12] = {"Japan", "USA", "German", "Europe", "Portugual", "England", "Australia"};
	u8 region_rating_struct_index[MAX_RATING_NUM] = {0,1,3,4,6,7,8};
	u8 region_rating_string[2];
	//memset(region_rating_string, 0x00, sizeof(region_rating_string));
	
		/*** Setting Up Age Restriction Rules ***/
	u8 region_age[MAX_RATING_NUM][10];
	int region_age_num[MAX_RATING_NUM] = {5,5,5,5,5,8,5};
	memcpy(region_age[RATING_JPN], (u8[]) {0,12,15,17,18}, region_age_num[RATING_JPN]);
	memcpy(region_age[RATING_USA], (u8[]) {3,6,10,13,17}, region_age_num[RATING_USA]);
	memcpy(region_age[RATING_GER], (u8[]) {0,6,12,16,17}, region_age_num[RATING_GER]);
	memcpy(region_age[RATING_EUR], (u8[]) {3,7,12,16,18}, region_age_num[RATING_EUR]);
	memcpy(region_age[RATING_PRT], (u8[]) {4,6,12,16,18}, region_age_num[RATING_PRT]);
	memcpy(region_age[RATING_ENG], (u8[]) {3,4,7,8,12,15,16,18}, region_age_num[RATING_ENG]);
	memcpy(region_age[RATING_AUS], (u8[]) {0,7,14,15,18}, region_age_num[RATING_AUS]);
	
		
		/*** Iterating Through Input File for Age Restrictions ***/
	if(use_age_ratings == TRUE){//Checking If Age Restictions Are To be used
		fseek(icn.bsf, 0x00, SEEK_SET);
		if(key_search("RegionRatings", icn.bsf) == FOUND){
			long int pos0 = ftell(icn.bsf);
			for(int i = 0; i < MAX_RATING_NUM; i++){
				fseek(icn.bsf, pos0, SEEK_SET);
				if(get_value(region_rating_string,2,region_rating_key[i],icn.bsf) == FOUND){
					char_to_int_array(&region_rating[i], region_rating_string, 1, BIG_ENDIAN, DEC);
					/**/
					u8 rating_index = region_rating_struct_index[i];
					/**/
					int rating_type = INVALID_RATING;
					for(int j = 0; j < region_age_num[i]; j++){						
						if(region_age[i][j] == region_rating[i]){
							rating_type = VALID_RATING;
							break;
						}
					}
					if(rating_type == VALID_RATING){
						region_rating[i] += 0x80;
						icn.settings.ratings.rating[rating_index] = region_rating[i];
					}
					else{
						printf("[!] Age Restriction for %s %d, is Invalid.\n",region_rating_key[i], region_rating[i]);
						return REGION_RATING_FAIL;
					}
				}
				else{
					value_find_fail(region_rating_key[i]);
					return REGION_RATING_FAIL;
				}
			}
		}
	}
	else{//If Age Restrictions are not to be used
		for(int i = 0; i < MAX_RATING_NUM; i++){
			u8 rating_index = region_rating_struct_index[i];
			icn.settings.ratings.rating[rating_index] = 0x80;
			}
	}
		
	/** Processing Region Lock **/
	u8 region_lock_list_1[MAX_REGION_LOCK_NUM][15] = {"Region_Free", "Japan", "America", "Europe", "Korea", "Taiwan", "China"};
	u8 region_lock_list_2[MAX_REGION_LOCK_NUM][5] = {"All", "JPN", "USA", "EUR", "KOR", "TWN", "CHN"};
	u8 region_lock_hex[MAX_REGION_LOCK_NUM][8] = {"7FFFFFFF", "00000001", "00000002", "0000000C", "00000020", "00000040", "0000050"};
	fseek(icn.bsf, 0x00, SEEK_SET);
	if(key_search("Options", icn.bsf) == FOUND){
		if (get_value(buff,15,"RegionLockout", icn.bsf) == FOUND){
			for(int i = 0; i < MAX_REGION_LOCK_NUM; i++){
				if(strcmp(buff, region_lock_list_1[i]) == 0 || strcmp(buff, region_lock_list_2[i]) == 0){
					char_to_int_array(icn.settings.region_lock, region_lock_hex[i], 4, LITTLE_ENDIAN, HEX);
					break;
				}
				else if( i == (MAX_REGION_LOCK_NUM - 1)){
					printf("[!] Invalid Region-Lockout Code: '%s'\n", buff);
					return REGION_LOCKOUT_FAIL;
				}
			}
		}
		else{
			printf("[!] No Region-Lockout Code was Specified\n");
			return REGION_LOCKOUT_FAIL;
		}
	}
	else{
		key_find_fail("Options");
		return REGION_LOCKOUT_FAIL;
	}
	
	/** Processing Default Banner Frame **/
	u8 default_frame[MAX_READ_LEN];
	fseek(icn.bsf, 0x00, SEEK_SET);
	if(key_search("Options", icn.bsf) == FOUND){
		if (get_value(default_frame,100,"OptimalBNRFrame",icn.bsf) == FOUND){
			union
			{
				float          f;
				unsigned char  b[sizeof(float)];
			} v = { atof(default_frame) };
			u8 default_frame_hex[8];
			sprintf(default_frame_hex, "%02X%02X%02X%02X\0",v.b[3],v.b[2],v.b[1],v.b[0]);
			char_to_int_array(icn.settings.optimal_bnr_frame, default_frame_hex, 4, LITTLE_ENDIAN, HEX);
		}
		else{
			value_find_fail("Default BNR Frame");
			return OPTM_BNR_FAIL;
		}
	}
	else{
		key_find_fail("Options");
		return OPTM_BNR_FAIL;
	}
    
	/** Processing IDs **/
	fseek(icn.bsf, 0x00, SEEK_SET);
	if(key_search("IDs", icn.bsf) == FOUND){
		long int pos0 = ftell(icn.bsf);
		//MatchMaker ID
		if(get_value(buff,0x8,"MatchMakerID",icn.bsf) == FOUND){
			char_to_int_array(icn.settings.match_maker_id, buff, sizeof(icn.settings.match_maker_id), LITTLE_ENDIAN, HEX);
		}
		else{
			value_find_fail("MatchMakerID");
			return ID_FAIL;
		}
		//MatchMakerBIT ID
		fseek(icn.bsf, pos0, SEEK_SET);
		if(get_value(buff,0x10,"MatchMakerBITID",icn.bsf) == FOUND){
			char_to_int_array(icn.settings.match_maker_bit_id, buff, sizeof(icn.settings.match_maker_bit_id), LITTLE_ENDIAN, HEX);
		}
		else{
			value_find_fail("MatchMakerBITID");
			return ID_FAIL;
		}
		//CEC ID
		fseek(icn.bsf, pos0, SEEK_SET);
		if(get_value(buff,0x8,"CECID",icn.bsf) == FOUND){
			char_to_int_array(icn.settings.cec_id, buff, sizeof(icn.settings.cec_id), LITTLE_ENDIAN, HEX);
		}
		else{
			value_find_fail("CECID");
			return ID_FAIL;
		}
	}
	else{
		key_find_fail("IDs");
		return ID_FAIL;
	}
	
    /** Writing To File **/
    fseek(icn.output, FLAG_OFFSET, SEEK_SET);
    fwrite(&icn.settings, sizeof(icn.settings), 1, icn.output);

    /** Return, all is good **/
    return 0;
}

void icn_icon_proccess(ICN_CONTEXT icn)
{
    //Temporarally Storing Icon Data
    u8 small_buff[SMALL_ICON_SIZE];
    u8 large_buff[LARGE_ICON_SIZE];
    fseek(icn.small, 0x0, SEEK_SET);
    fseek(icn.large, 0x0, SEEK_SET);
    fread(&small_buff, SMALL_ICON_SIZE, 1, icn.small);
    fread(&large_buff, LARGE_ICON_SIZE, 1, icn.large);

    //Writing Small Icon to output
    fseek(icn.output, SMALL_ICON_OFFSET, SEEK_SET);
    fwrite(&small_buff, SMALL_ICON_SIZE, 1, icn.output);

    //Writing Large Icon to output
    fseek(icn.output, LARGE_ICON_OFFSET, SEEK_SET);
    fwrite(&large_buff, LARGE_ICON_SIZE, 1, icn.output);
}

ICN_APP_TITLE_STRUCT string_ICN_conv(u8* short_title, u8* long_title, u8* publisher)
{
    ICN_APP_TITLE_STRUCT tmp;
    memset(&tmp, 0, 0x200);
    //Short Title
    if (strlen(short_title) <= 0x40){
        for (int i = 0, n = strlen(short_title); i < n; i++){
            tmp.short_title[i * 2] = short_title[i];
			//tmp.short_title[(i * 2) + 1] = 0x0;
        }
    }
	else{
		printf("[!] 'Short Title' Too Long: [%s]\n", short_title);
	}
    //Long Title
    if (strlen(long_title) <= 0x50){
        for (int i = 0, n = strlen(long_title); i < n; i++){
            tmp.long_title[i * 2] = long_title[i];
			//tmp.long_title[(i * 2) + 1] = 0x0;
        }
    }
	else{
		printf("[!] 'Long Title' Too Long: [%s]\n", long_title);
	}
    //Publisher
    if (strlen(publisher) <= 0x40){
        for (int i = 0, n = strlen(publisher); i < n; i++){
            tmp.publisher[i * 2] = publisher[i];
			//tmp.publisher[(i * 2) + 1] = 0x0;
        }
    }
	else{
		printf("[!] 'Publisher' Too Long: [%s]\n", publisher);
	}
    return tmp;
}

int icn_read(FILE *icn)
{
	DWORD magic;
	fseek(icn,0x0,SEEK_SET);
	fread(&magic, sizeof(magic), 1, icn);
	if(magic != 0x48444D53){
		printf("[!] ICN is corrupt (Magic not 'SMDH')\n");
		return MAGIC_FAIL;
	}
	else
		printf("[*] MAGIC = SMDH\n");
	
	
	/** Application Title Strings **/
	
	u8 language_string[MAX_TITLE_NUM][20];
	strcpy(language_string[0], "Japanese");
	strcpy(language_string[1], "English");
	strcpy(language_string[2], "French");
	strcpy(language_string[3], "German");
	strcpy(language_string[4], "Italian");
	strcpy(language_string[5], "Spanish");
	strcpy(language_string[6], "Chinese");
	strcpy(language_string[7], "Korean");
	strcpy(language_string[8], "Dutch");
	strcpy(language_string[9], "Portuguese");
	strcpy(language_string[10], "Russian");
	/**
	strcpy(language_string[11], "Unknown Language 0");
	strcpy(language_string[12], "Unknown Language 1");
	strcpy(language_string[13], "Unknown Language 2");
	strcpy(language_string[14], "Unknown Language 3");
	strcpy(language_string[15], "Unknown Language 4");
	**/
	
	printf("\n[*] Application Title Strings:\n\n");
	for(int i = 0; i < 11; i++){
		printf("%s:\n",language_string[i]);
		printf(" > Short Title:	");
		print_title((0x8+(0x200*i)),0x40,icn);
		printf("\n > Long Title:	");
		print_title((0x8+(0x200*i)+0x80),0x50,icn);
		printf("\n > Publisher:	");
		print_title((0x8+(0x200*i)+0x180),0x40,icn);
		putchar('\n');
		//putchar('\n');
	}
	
	/** BitMask Flags **/
	printf("\n[*] Application Settings\n\nFlags:");
	u8 region_rating_key[MAX_RATING_NUM][30] = {"Japan:			", "USA:			", "German:			", "Europe:			", "Portugual:			", "England:			", "Australia:			"};
	u8 ratingindex[MAX_RATING_NUM] = {0,1,3,4,6,7,8};
	fseek(icn, 0x2028, SEEK_SET);
	u8 byte_flags[8];
	fread(&byte_flags, 0x8,1,icn);
	for(int i = 0; i < 8; i++){
		printf(" %02x",byte_flags[i]);
	}
	printf("\n");
	u8 flag_bool[8];
	//byte[0] flags
	resolve_flag(byte_flags[0],flag_bool);
	//printf("Visability Flag:		%s\n",flag_bool[0]? "YES" : "NO");
	printf(" > AutoBoot Application:	%s\n",flag_bool[1]? "YES" : "NO");
	printf(" > Uses 3D Effect:		%s\n",flag_bool[2]? "YES" : "NO");
	printf(" > Requires Accepting EULA:	%s\n",flag_bool[3]? "YES" : "NO");
	printf(" > Autosave On Exit:		%s\n",flag_bool[4]? "YES" : "NO");
	printf(" > Use an Extended Banner:	%s\n",flag_bool[5]? "YES" : "NO");
	printf(" > Use Age Restrictions:	%s\n",flag_bool[6]? "YES" : "NO");
	/** Age Restrictions, If used **/
	if(flag_bool[6] == TRUE){
		for(int i = 0; i < MAX_RATING_NUM; i++){
			fseek(icn,(0x2008+ratingindex[i]),SEEK_SET);
			u8 tmp = fgetc(icn);
			if(tmp >= 0x80)
				printf("   > %s%d\n",region_rating_key[i], (tmp - 0x80));
		}
	}
	printf(" > Use Save Data:		%s\n",flag_bool[7]? "YES" : "NO");
	//printf("\n\n");

	/** Region Lock **/
	// Well my current theory which is Region locking is just bitmask magic. Will have to confirm.
	// From SDK 1.2.0 'China' encompases Taiwan 3DSs and 'Europe' encompases the now non-existant Australian 3DSs
	fseek(icn,0x2018,SEEK_SET);
	u8 region_lock[4];
	fread(&region_lock,sizeof(region_lock),1,icn);
	printf("\nRegion Lock: ");
	for(int i = 0; i < 4; i++)
		printf("%02x",region_lock[3-i]);
	printf("\n");
	u8 region_lock_bool[8];
	resolve_flag(region_lock[0],region_lock_bool);
	printf(" > Japan:			%s\n",region_lock_bool[0]? "YES" : "NO");
	printf(" > USA:				%s\n",region_lock_bool[1]? "YES" : "NO");
	printf(" > Europe:			%s\n",region_lock_bool[2]? "YES" : "NO");
	//printf(" > Australia:			%s\n",region_lock_bool[3]? "YES" : "NO");
	printf(" > China:			%s\n",region_lock_bool[4]? "YES" : "NO");
	printf(" > Korea:			%s\n",region_lock_bool[5]? "YES" : "NO");
	printf(" > Taiwan:			%s\n",region_lock_bool[6]? "YES" : "NO");
	
	printf("\n[*] Application IDs\n\n");
	/** Chance Encounter Communications ID **/
	fseek(icn,0x2034,SEEK_SET);
	DWORD CECID;
	fread(&CECID,sizeof(CECID),1,icn);
	printf("CEC ID:				%08x\n",CECID);
	
	/** Match Maker ID **/
	fseek(icn,0x201C,SEEK_SET);
	DWORD mmID;
	fread(&mmID,sizeof(mmID),1,icn);
	printf("Match Maker ID:			%08x\n",mmID);
	
	/** Match Maker BIT ID **/
	fseek(icn,0x2020,SEEK_SET);
	DWORD mmBITID1;
	DWORD mmBITID2;
	fread(&mmBITID1,sizeof(mmBITID1),1,icn);
	fread(&mmBITID2,sizeof(mmBITID2),1,icn);
	printf("Match Maker BIT ID:		%08x%08x\n\n",mmBITID2,mmBITID1);
	return 0;
}

void resolve_flag(u8 flag, u8 *flag_bool)
{
	u8 bit_mask[8] = {0x80,0x40,0x20,0x10,0x8,0x4,0x2,0x1};
	for(int i = 0; i < 8; i++){
		if (flag >= bit_mask[i]){
			flag_bool[7-i] = TRUE;
			flag -= bit_mask[i];
		}
		else
			flag_bool[7-i] = FALSE;
	}
}

void print_title(int offset, int size, FILE *file)
{
	for(int i = 0; i < size; i++){
		fseek(file,(offset + (i*2)),SEEK_SET);
		u8 tmp = fgetc(file);
		if(tmp == 0x0)
			break;
		else
			printf("%c",tmp);
		
	}
}