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
#include <stdint.h>

namespace {

struct Description
{
	const char* name;
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

static const Description DAY[32] =
{
	{ "GREEN1", 120, 255, 120 },
	{ "GREEN2",  45, 150,  45 },
	{ "GREEN3", 200, 220, 200 },
	{ "GREEN4",   0, 255,   0 },
	{ "BLUE1",  170, 170, 255 },
	{ "BLUE2",   45,  45, 170 },
	{ "BLUE3",    0,   0, 255 },
	{ "GREY1",  200, 200, 200 },
	{ "GREY2",  230, 230, 230 },
	{ "RED1",   220, 200, 200 },
	{ "UBLCK",    0,   0,   0 },
	{ "UWHIT",  255, 255, 255 },
	{ "URED",   255,   0,   0 },
	{ "UGREN",    0, 255,   0 },
	{ "YELO1",  243, 229,  47 },
	{ "YELO2",  128,  80,   0 },
	{ "TEAL1",    0, 128, 128 },
	{ "DILG0",  238, 239, 242 }, // Dialog Background white
	{ "DILG1",  212, 208, 200 }, // Dialog Background
	{ "DILG2",  255, 255, 255 }, // Control Background
	{ "DILG3",    0,   0,   0 }, // Text
	{ "UITX1",    0,   0,   0 }, // Menu Text, derived from UINFF
	{ "UDKRD",  124,  16,   0 },
	{ "UARTE",  200,   0,   0 }, // Active Route, Grey on Dusk/Night
	{ "DASHB",  255, 255, 255 }, // Dashboard Instr background
	{ "DASHL",  190, 190, 190 }, // Dashboard Instr Label
	{ "DASHF",   50,  50,  50 }, // Dashboard Foreground
	{ "DASHR",  200,   0,   0 }, // Dashboard Red
	{ "DASHG",    0, 200,   0 }, // Dashboard Green
	{ "DASHN",  200, 120,   0 }, // Dashboard Needle
	{ "DASH1",  204, 204, 255 }, // Dashboard Illustrations
	{ "DASH2",  122, 131, 172 }, // Dashboard Illustrations
};

static const Description DUSK[32] =
{
	{ "GREEN1",  60, 128,  60 },
	{ "GREEN2",  22,  75,  22 },
	{ "GREEN3",  80, 100,  80 },
	{ "GREEN4",   0, 128,   0 },
	{ "BLUE1",   80,  80, 160 },
	{ "BLUE2",   30,  30, 120 },
	{ "BLUE3",    0,   0, 128 },
	{ "GREY1",  100, 100, 100 },
	{ "GREY2",  128, 128, 128 },
	{ "RED1 ",  150, 100, 100 },
	{ "UBLCK",    0,   0,   0 },
	{ "UWHIT",  255, 255, 255 },
	{ "URED ",  120,  54,  11 },
	{ "UGREN",   35, 110,  20 },
	{ "YELO1",  120, 115,  24 },
	{ "YELO2",   64,  40,   0 },
	{ "TEAL1",    0,  64,  64 },
	{ "DILG0",  110, 110, 110 }, // Dialog Background
	{ "DILG1",  110, 110, 110 }, // Dialog Background
	{ "DILG2",    0,   0,   0 }, // Control Background
	{ "DILG3",  130, 130, 130 }, // Text
	{ "UITX1",   41,  46,  46 }, // Menu Text, derived from UINFF
	{ "UDKRD",   80,   0,   0 },
	{ "UARTE",   64,  64,  64 }, // Active Route, Grey on Dusk/Night
	{ "DASHB",   77,  77,  77 }, // Dashboard Instr background
	{ "DASHL",   54,  54,  54 }, // Dashboard Instr Label
	{ "DASHF",    0,   0,   0 }, // Dashboard Foreground
	{ "DASHR",   58,  21,  21 }, // Dashboard Red
	{ "DASHG",   21,  58,  21 }, // Dashboard Green
	{ "DASHN",  100,  50,   0 }, // Dashboard Needle
	{ "DASH1",   76,  76, 113 }, // Dashboard Illustrations
	{ "DASH2",   48,  52,  72 }, // Dashboard Illustrations
};

static const Description NIGHT[32] =
{
	{ "GREEN1",  30,  80,  30 },
	{ "GREEN2",  15,  60,  15 },
	{ "GREEN3",  12,  23,   9 },
	{ "GREEN4",   0,  64,   0 },
	{ "BLUE1",   60,  60, 100 },
	{ "BLUE2",   22,  22,  85 },
	{ "BLUE3",    0,   0,  40 },
	{ "GREY1",   48,  48,  48 },
	{ "GREY2",   64,  64,  64 },
	{ "RED1 ",  100,  50,  50 },
	{ "UWHIT",  255, 255, 255 },
	{ "UBLCK",    0,   0,   0 },
	{ "URED ",   60,  27,   5 },
	{ "UGREN",   17,  55,  10 },
	{ "YELO1",   60,  65,  12 },
	{ "YELO2",   32,  20,   0 },
	{ "TEAL1",    0,  32,  32 },
	{ "DILG0",   80,  80,  80 }, // Dialog Background
	{ "DILG1",   80,  80,  80 }, // Dialog Background
	{ "DILG2",    0,   0,   0 }, // Control Background
	{ "DILG3",   65,  65,  65 }, // Text
	{ "UITX1",   31,  34,  35 }, // Menu Text, derived from UINFF
	{ "UDKRD",   50,   0,   0 },
	{ "UARTE",   64,  64,  64 }, // Active Route, Grey on Dusk/Night
	{ "DASHB",    0,   0,   0 }, // Dashboard Instr background
	{ "DASHL",   20,  20,  20 }, // Dashboard Instr Label
	{ "DASHF",   64,  64,  64 }, // Dashboard Foreground
	{ "DASHR",   70,  15,  15 }, // Dashboard Red
	{ "DASHG",   15,  70,  15 }, // Dashboard Green
	{ "DASHN",   17,  80,  56 }, // Dashboard Needle
	{ "DASH1",   48,  52,  72 }, // Dashboard Illustrations
	{ "DASH2",   36,  36,  53 }, // Dashboard Illustrations
};

}


UserColors::UserColors()
	: current_scheme(global::GLOBAL_COLOR_SCHEME_DAY)
	, chart_color_provider(NULL)
{
	for (unsigned int i = 0; i < sizeof(DAY) / sizeof(DAY[0]); ++i) {
		const Description& desc = DAY[i];
		Table& table = color_schemes[global::GLOBAL_COLOR_SCHEME_DAY];
		table[wxString(desc.name, wxConvUTF8)] = wxColour(desc.r, desc.g, desc.b);
	}
	for (unsigned int i = 0; i < sizeof(DUSK) / sizeof(DUSK[0]); ++i) {
		const Description& desc = DUSK[i];
		Table& table = color_schemes[global::GLOBAL_COLOR_SCHEME_DUSK];
		table[wxString(desc.name, wxConvUTF8)] = wxColour(desc.r, desc.g, desc.b);
	}
	for (unsigned int i = 0; i < sizeof(NIGHT) / sizeof(NIGHT[0]); ++i) {
		const Description& desc = NIGHT[i];
		Table& table = color_schemes[global::GLOBAL_COLOR_SCHEME_NIGHT];
		table[wxString(desc.name, wxConvUTF8)] = wxColour(desc.r, desc.g, desc.b);
	}
}

UserColors::~UserColors()
{
}

void UserColors::inject_chart_color_provider(global::ColorProvider* provider)
{
	chart_color_provider = provider;
}

wxColour UserColors::get_color(const wxString& color_name) const
{
	wxColour result;

	if (chart_color_provider) {
		result = chart_color_provider->get_color(color_name);
	}

	// read color from mapping
	if (!result .Ok()) {
		// color schemes are already checked (see set_current)
		Schemes::const_iterator scheme = color_schemes.find(current_scheme);
		Table::const_iterator color = scheme->second.find(color_name);
		if (color != scheme->second.end()) {
			result = color->second;
		}
	}

	// Default
	if (!result.Ok()) {
		result.Set(128, 128, 128); // Simple Grey
	}

	return result;
}

void UserColors::set_current(global::ColorScheme scheme)
{
	switch (scheme) {
		case global::GLOBAL_COLOR_SCHEME_DAY:
		case global::GLOBAL_COLOR_SCHEME_DUSK:
		case global::GLOBAL_COLOR_SCHEME_NIGHT:
			current_scheme = scheme;
			break;

		case global::GLOBAL_COLOR_SCHEME_INVALID:
		case global::GLOBAL_COLOR_SCHEME_RGB:
		case global::GLOBAL_COLOR_SCHEME_MAX:
			current_scheme = global::GLOBAL_COLOR_SCHEME_DAY; // default value
			break;
	}
}

global::ColorScheme UserColors::get_current() const
{
	return current_scheme;
}

