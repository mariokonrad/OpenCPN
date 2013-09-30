/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 * Copyright (C) 2010 by David S. Register                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the                           *
 * Free Software Foundation, Inc.,                                         *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.           *
 **************************************************************************/

#include "UserColors.h"
#include <s52s57.h>
#include <cstring>
#include <wx/dynarray.h>

#ifdef USE_S57
#include <s52plib.h>
extern s52plib * ps52plib;
#endif

static wxArrayPtrVoid * UserColorTableArray;
static wxArrayPtrVoid * UserColourHashTableArray;
static wxColorHashMap * pcurrent_user_color_hash;


wxColour GetGlobalColor(wxString colorName)
{
	wxColour ret_color;

#ifdef USE_S57
	// Use the S52 Presentation library if present
	if (ps52plib) {
		ret_color = ps52plib->getwxColour(colorName);

		if (!ret_color.Ok()) { //261 likes Ok(), 283 likes IsOk()...
			if (NULL != pcurrent_user_color_hash)
				ret_color = (*pcurrent_user_color_hash)[colorName];
		}
	} else
#endif
	{
		if (NULL != pcurrent_user_color_hash)
			ret_color = (*pcurrent_user_color_hash)[colorName];
	}

	// Default
	if (!ret_color.Ok())
		ret_color.Set(128, 128, 128); // Simple Grey

	return ret_color;
}

void setup_current_user_color(const wxString & scheme)
{
	//Search the user color table array to find the proper hash table
	int Usercolortable_index = 0;
	for (unsigned int i = 0; i < UserColorTableArray->GetCount(); ++i) {
		colTable * ct = (colTable *) UserColorTableArray->Item(i);
		if (scheme.IsSameAs(*ct->tableName)) {
			Usercolortable_index = i;
			break;
		}
	}

	// Set up a pointer to the proper hash table
	pcurrent_user_color_hash = (wxColorHashMap *)UserColourHashTableArray->Item(Usercolortable_index);
}

static int get_static_line(char * d, const char ** p, int index, int n)
{
	if (!strcmp(p[index], "*****"))
		return 0;

	strncpy(d, p[index], n);
	return strlen(d);
}

void InitializeUserColors(void)
{
	static const char * usercolors[] =
	{
		"Table:DAY",
		"GREEN1;120;255;120;",
		"GREEN2; 45;150; 45;",
		"GREEN3;200;220;200;",
		"GREEN4;  0;255;  0;",
		"BLUE1; 170;170;255;",
		"BLUE2;  45; 45;170;",
		"BLUE3;   0;  0;255;",
		"GREY1; 200;200;200;",
		"GREY2; 230;230;230;",
		"RED1;  220;200;200;",
		"UBLCK;   0;  0;  0;",
		"UWHIT; 255;255;255;",
		"URED;  255;  0;  0;",
		"UGREN;   0;255;  0;",
		"YELO1; 243;229; 47;",
		"YELO2; 128; 80;  0;",
		"TEAL1;   0;128;128;",
		"DILG0; 238;239;242;", // Dialog Background white
		"DILG1; 212;208;200;", // Dialog Background
		"DILG2; 255;255;255;", // Control Background
		"DILG3;   0;  0;  0;", // Text
		"UITX1;   0;  0;  0;", // Menu Text, derived from UINFF
		"UDKRD; 124; 16;  0;",
		"UARTE; 200;  0;  0;", // Active Route, Grey on Dusk/Night
		"DASHB; 255;255;255;", // Dashboard Instr background
		"DASHL; 190;190;190;", // Dashboard Instr Label
		"DASHF;  50; 50; 50;", // Dashboard Foreground
		"DASHR; 200;  0;  0;", // Dashboard Red
		"DASHG;   0;200;  0;", // Dashboard Green
		"DASHN; 200;120;  0;", // Dashboard Needle
		"DASH1; 204;204;255;", // Dashboard Illustrations
		"DASH2; 122;131;172;", // Dashboard Illustrations

		"Table:DUSK",
		"GREEN1; 60;128; 60;",
		"GREEN2; 22; 75; 22;",
		"GREEN3; 80;100; 80;",
		"GREEN4;  0;128;  0;",
		"BLUE1;  80; 80;160;",
		"BLUE2;  30; 30;120;",
		"BLUE3;   0;  0;128;",
		"GREY1; 100;100;100;",
		"GREY2; 128;128;128;",
		"RED1;  150;100;100;",
		"UBLCK;   0;  0;  0;",
		"UWHIT; 255;255;255;",
		"URED;  120; 54; 11;",
		"UGREN;  35;110; 20;",
		"YELO1; 120;115; 24;",
		"YELO2;  64; 40;  0;",
		"TEAL1;   0; 64; 64;",
		"DILG0; 110;110;110;", // Dialog Background
		"DILG1; 110;110;110;", // Dialog Background
		"DILG2;   0;  0;  0;", // Control Background
		"DILG3; 130;130;130;", // Text
		"UITX1;  41; 46; 46;", // Menu Text, derived from UINFF
		"UDKRD;  80;  0;  0;",
		"UARTE;  64; 64; 64;", // Active Route, Grey on Dusk/Night
		"DASHB;  77; 77; 77;", // Dashboard Instr background
		"DASHL;  54; 54; 54;", // Dashboard Instr Label
		"DASHF;   0;  0;  0;", // Dashboard Foreground
		"DASHR;  58; 21; 21;", // Dashboard Red
		"DASHG;  21; 58; 21;", // Dashboard Green
		"DASHN; 100; 50;  0;", // Dashboard Needle
		"DASH1;  76; 76;113;", // Dashboard Illustrations
		"DASH2;  48; 52; 72;", // Dashboard Illustrations

		"Table:NIGHT",
		"GREEN1; 30; 80; 30;",
		"GREEN2; 15; 60; 15;",
		"GREEN3; 12; 23;  9;",
		"GREEN4;  0; 64;  0;",
		"BLUE1;  60; 60;100;",
		"BLUE2;  22; 22; 85;",
		"BLUE3;   0;  0; 40;",
		"GREY1;  48; 48; 48;",
		"GREY2;  64; 64; 64;",
		"RED1;  100; 50; 50;",
		"UWHIT; 255;255;255;",
		"UBLCK;   0;  0;  0;",
		"URED;   60; 27;  5;",
		"UGREN;  17; 55; 10;",
		"YELO1;  60; 65; 12;",
		"YELO2;  32; 20;  0;",
		"TEAL1;   0; 32; 32;",
		"DILG0;  80; 80; 80;", // Dialog Background
		"DILG1;  80; 80; 80;", // Dialog Background
		"DILG2;   0;  0;  0;", // Control Background
		"DILG3;  65; 65; 65;", // Text
		"UITX1;  31; 34; 35;", // Menu Text, derived from UINFF
		"UDKRD;  50;  0;  0;",
		"UARTE;  64; 64; 64;", // Active Route, Grey on Dusk/Night
		"DASHB;   0;  0;  0;", // Dashboard Instr background
		"DASHL;  20; 20; 20;", // Dashboard Instr Label
		"DASHF;  64; 64; 64;", // Dashboard Foreground
		"DASHR;  70; 15; 15;", // Dashboard Red
		"DASHG;  15; 70; 15;", // Dashboard Green
		"DASHN;  17; 80; 56;", // Dashboard Needle
		"DASH1;  48; 52; 72;", // Dashboard Illustrations
		"DASH2;  36; 36; 53;", // Dashboard Illustrations

		"*****" };

	const char ** p = usercolors;
	colTable * ct;

	UserColorTableArray = new wxArrayPtrVoid;
	UserColourHashTableArray = new wxArrayPtrVoid;

	// Create 3 color table entries
	ct = new colTable;
	ct->tableName = new wxString(_T("DAY"));
	ct->color = new wxArrayPtrVoid;
	UserColorTableArray->Add((void *)ct);

	ct = new colTable;
	ct->tableName = new wxString(_T("DUSK"));
	ct->color = new wxArrayPtrVoid;
	UserColorTableArray->Add((void *)ct);

	ct = new colTable;
	ct->tableName = new wxString(_T("NIGHT"));
	ct->color = new wxArrayPtrVoid;
	UserColorTableArray->Add((void *)ct);

	int index = 0;
	char buf[80];
	while ((get_static_line(buf, p, index, sizeof(buf)))) {
		if (!strncmp(buf, "Table", 5)) {
			char TableName[20];
			sscanf(buf, "Table:%s", TableName);

			for (unsigned int it = 0; it < UserColorTableArray->GetCount(); it++) {
				colTable * ctp = (colTable *)(UserColorTableArray->Item(it));
				if (!strcmp(TableName, ctp->tableName->mb_str())) {
					ct = ctp;
					break;
				}
			}
		} else {
			char name[80];
			int j = 0;
			while (buf[j] != ';') {
				name[j] = buf[j];
				j++;
			}
			name[j] = 0;

			S52color * c = new S52color;
			strcpy(c->colName, name);

			int R = 0;
			int G = 0;
			int B = 0;
			sscanf(&buf[j], ";%i;%i;%i", &R, &G, &B);
			c->R = (char)R;
			c->G = (char)G;
			c->B = (char)B;

			ct->color->Add(c);
		}
		index++;
	}

	// Now create the Hash tables

	for (unsigned int its = 0; its < UserColorTableArray->GetCount(); ++its) {
		wxColorHashMap *phash = new wxColorHashMap;
		UserColourHashTableArray->Add((void *)phash);

		colTable * ctp = (colTable *)(UserColorTableArray->Item(its));

		for (unsigned int ic = 0; ic < ctp->color->GetCount(); ++ic) {
			S52color * c2 = (S52color *)(ctp->color->Item(ic));

			wxColour c(c2->R, c2->G, c2->B);
			wxString key(c2->colName, wxConvUTF8);
			(*phash)[key] = c;
		}
	}

	// Establish a default hash table pointer
	// in case a color is needed before ColorScheme is set
	pcurrent_user_color_hash = (wxColorHashMap *)UserColourHashTableArray->Item(0);
}

void DeInitializeUserColors(void)
{
	for (unsigned int i = 0; i < UserColorTableArray->GetCount(); ++i) {
		colTable *ct = (colTable *) UserColorTableArray->Item(i);

		for( unsigned int j = 0; j < ct->color->GetCount(); ++j) {
			S52color * c = (S52color *) ct->color->Item(j);
			delete c; //color
		}

		delete ct->tableName; // wxString
		delete ct->color; // wxArrayPtrVoid
		delete ct; // colTable
	}

	delete UserColorTableArray;

	for (unsigned int i = 0; i < UserColourHashTableArray->GetCount(); ++i) {
		wxColorHashMap * phash = (wxColorHashMap *) UserColourHashTableArray->Item(i);
		delete phash;
	}

	delete UserColourHashTableArray;
}
