/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#include "PlugInToolbarToolContainer.h"
#include <wx/bitmap.h>

PlugInToolbarToolContainer::PlugInToolbarToolContainer()
	: bitmap_day(NULL)
	, bitmap_dusk(NULL)
	, bitmap_night(NULL)
	, bitmap_Rollover(NULL)
{}

PlugInToolbarToolContainer::~PlugInToolbarToolContainer()
{
	if (bitmap_dusk) {
		delete bitmap_dusk;
		bitmap_dusk = NULL;
	}
	if (bitmap_night) {
		delete bitmap_night;
		bitmap_night = NULL;
	}
	if (bitmap_day) {
		delete bitmap_day;
		bitmap_day = NULL;
	}
	if (bitmap_Rollover) {
		delete bitmap_Rollover;
		bitmap_Rollover = NULL;
	}
}

