/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __TIDE__TCDATAFACTORY_H__
#define __TIDE__TCDATAFACTORY_H__

#include <tide/TC_Error_Code.h>
#include <wx/string.h>

namespace tide {

class IDX_entry;

enum { REGION = 1, COUNTRY = 2, STATE = 3 };

class TCDataFactory
{
public:
	TCDataFactory();
	virtual ~TCDataFactory();

	virtual TC_Error_Code LoadData(const wxString& data_file_path) = 0;
	virtual int GetMaxIndex(void) const = 0;
	virtual IDX_entry* GetIndexEntry(int n_index) = 0;
	virtual TC_Error_Code LoadHarmonicData(IDX_entry* pIDX) = 0;

protected:
	enum unit_type {
		LENGTH,
		VELOCITY,
		BOGUS
	};

	struct UnitInfo
	{
		char* name;
		char* abbrv;
		unit_type type;
		double conv_factor;
	};

	int findunit(const char* unit) const;
	const UnitInfo& get_unit(int) const;

	wxString source_ident;

private:
	UnitInfo known_units[4];
};

}

#endif
