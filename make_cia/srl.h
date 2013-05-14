typedef struct
{
	u8 game_title[0xC];
	u8 game_code[4];
	u8 maker_code[2];
	u8 unit_code;
	u8 encryption_seed_select;
	u8 device_capacity;
	u8 reserved_0[8];
	u8 rom_version[2];
	u8 internal_flag;
	u8 arm9_rom_offset[4];
	u8 arm9_entry_address[4];
	u8 arm9_ram_address[4];
	u8 arm9_size[4];
	u8 arm7_rom_offset[4];
	u8 arm7_entry_address[4];
	u8 arm7_ram_address[4];
	u8 arm7_size[4];
	u8 fnt_offset[4];
	u8 fnt_size[4];
	u8 fat_offset[4];
	u8 fat_size[4];
	u8 arm9_overlay_offset[4];
	u8 arm9_overlay_size[4];
	u8 arm7_overlay_offset[4];
	u8 arm7_overlay_size[4];
	u8 normal_card_control_reg_settings[4];
	u8 secure_card_control_reg_settings[4];
	u8 icon_banner_offset[4];
	u8 secure_area_crc[2];
	u8 secure_transfer_timeout[2];
	u8 arm9_autoload[4];
	u8 arm7_autoload[4];
	u8 secure_disable[8];
	u8 ntr_rom_size[4];
	u8 header_size[4];
	u8 reserved_1[0x38];
	u8 nintendo_logo[0x9C];
	u8 nintendo_logo_crc[2];
	u8 header_crc[2];
	u8 debug_reserved[0x20];
	
	//TWL Only Data
	u8 config_settings[0x34];
	u8 access_control[4];
	u8 arm7_scfg_ext_mask[4];
	u8 reserved_flags[4];
	u8 arm9i_rom_offset[4];
	u8 reserved_2[4];
	u8 arm9i_load_address[4];
	u8 arm9i_size[4];
	u8 arm7i_rom_offset[4];
	u8 struct_param_base_address[4];
	u8 arm7i_load_address[4];
	u8 arm7i_size[4];
	u8 digest_ntr_region_offset[4];
	u8 digest_ntr_region_size[4];
	u8 digest_twl_region_offset[4];
	u8 digest_twl_region_size[4];
	u8 digest_sector_hashtable_offset[4];
	u8 digest_sector_hashtable_size[4];
	u8 digest_block_hashtable_offset[4];
	u8 digest_block_hashtable_size[4];
	u8 digest_sector_size[4];
	u8 digest_block_sectorcount[4];
	u8 reserved_3[8];
	u8 twl_rom_size[8];
	u8 unknown[8];
	u8 modcrypt_area_1_offset[4];
	u8 modcrypt_area_1_size[4];
	u8 modcrypt_area_2_offset[4];
	u8 modcrypt_area_2_size[4];
	u8 title_id[8];
	u8 reserved_4[0xC8];
	
	// TWL and Signed NTR
	u8 arm9_with_sec_area_sha1_hmac[0x14];
	u8 arm7_sha1_hmac[0x14];
	u8 digest_master_sha1_hmac[0x14];
	u8 banner_sha1_hmac[0x14];
	u8 arm9i_sha1_hmac[0x14];
	u8 arm7i_sha1_hmac[0x14];
	u8 reserved_5[0x28];
	u8 arm9_sha1_hmac[0x14];
	u8 reserved_6[0xA4C];
	u8 reserved_7[0x180];
	u8 signature[0x80];
} __attribute__((__packed__)) 
SRL_HEADER;

int GetCoreContentSRL(CORE_CONTENT_INFO *core,u8 SRLType,FILE *srl);
void GetNTRTitleID(u8 *game_code, u8 *title_id);
void GetTWLTitleID(u8 *title_id_old, u8 *title_id);


