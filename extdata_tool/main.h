/**
Copyright 2013 3DSGuy

This file is part of extdata_tool.

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
**/
typedef struct
{
	//Option Flags
	u8 verbose;
	u8 info;
	u8 extract;
	u8 titledb_read;
	
	//Input
	u8 *input_extdata;
	
	//Output Dir
	u8 *output_dir;
	int output_dir_len;
} __attribute__((__packed__))
INPUT_CONTEXT;