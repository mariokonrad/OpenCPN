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

#ifndef __LOOKUP__H__
#define __LOOKUP__H__

#include "s52plib.h"

class Lookup
{
	public:
		int RCID;
		int id;
		wxString name;
		Object_t type;  // 'A' Area, 'L' Line, 'P' Point
		DisPrio displayPrio; // Display Priority
		RadPrio radarPrio; // 'O' or 'S', Radar Priority
		LUPname tableName; // FTYP:  areas, points, lines
		wxArrayString * attributeCodeArray; // ArrayString of LUP Attributes
		wxString instruction; // Instruction Field (rules)
		DisCat displayCat; // Display Categorie: D/S/O, DisplayBase, Standard, Other
		int comment; // Look-Up Comment (PLib3.x put 'groupes' here,
};

#endif
