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
#ifndef _TIK_STATIC_DATA_
#define _TIK_STATIC_DATA_
static const unsigned char dev_static_ticket_data[0x30] =
{
	0x00, 0x01, 0x00, 0x14, 0x00, 0x00, 0x00, 0xAC, 
	0x00, 0x00, 0x00, 0x14, 0x00, 0x01, 0x00, 0x14, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 
	0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x84, 
	0x00, 0x00, 0x00, 0x84, 0x00, 0x03, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
};

static const unsigned char prod_static_ticket_data[0x30] =
{
	0x00, 0x01, 0x00, 0x14, 0x00, 0x00, 0x00, 0xAC, 
	0x00, 0x00, 0x00, 0x14, 0x00, 0x01, 0x00, 0x14, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 
	0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x84, 
	0x00, 0x00, 0x00, 0x84, 0x00, 0x03, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00
};

typedef enum
{
	common = 1,
	unique
} ticket_type;

typedef enum
{
	lic_Permanent = 0,
	lic_Demo = 1,
	lic_Trial = 2,
	lic_Rental = 3,
	lic_Subscription = 4,
	lic_Service = 5,
	lic_Mask = 15
} ticket_license_type;

typedef enum
{
	right_Permanent = 1,
	right_Subscription = 2,
	right_Content = 3,
	right_ContentConsumption = 4,
	right_AccessTitle = 5
} ticket_item_rights;

typedef enum
{
	dev = 1,
	prod,
	test
} cia_type;

typedef struct
{
	u8 sig_type[4];
	u8 data[0x100];
	u8 padding[0x3C];
} __attribute__((__packed__)) 
TIK_2048_SIG_CONTEXT;

typedef struct
{
	u8 Issuer[0x40];
	u8 ECDH[0x3c];
	u8 TicketFormatVersion;
	u8 ca_crl_version;
	u8 signer_crl_version;
	u8 EncryptedTitleKey[0x10];
	u8 unknown_0;
	u8 TicketID[8];
	u8 DeviceID[4];
	u8 TitleID[8];
	u8 unknown_1[2];
	u8 TicketVersion[2];
	u8 unused_0[8];
	u8 unused_1;
	u8 CommonKeyID;
	u8 unused_2[0x2F];
	u8 unknown_2;
	u8 unused_3[0x82];
	u8 StaticData[0x30];
	u8 unused_4[0x7C];
} __attribute__((__packed__)) 
TICKET_STRUCTURE;

#endif

int GenerateTicket(USER_CONTEXT *ctx);
int EncryptTitleKey(u8 EncTitleKey[0x10], u8 *DecTitleKey, u8 *CommonKey, u8 *TitleID);
int SetStaticData(u8 mode, u8 section[0x30]);