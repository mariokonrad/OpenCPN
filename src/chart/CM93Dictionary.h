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

#ifndef __CHART__CM93DICTIONARY__H__
#define __CHART__CM93DICTIONARY__H__

#include <wx/string.h>
#include <wx/arrstr.h>

namespace chart {

/// Encapsulating the conversion between binary cm_93 object class, attributes, etc
/// to standard S57 text conventions
class cm93_dictionary
{
	public:
		cm93_dictionary();
		~cm93_dictionary();

		bool LoadDictionary(const wxString & dictionary_dir);
		bool IsOk(void) const;
		wxString GetDictDir(void) const;
		wxString GetClassName(int iclass);
		wxString GetAttrName(int iattr);
		char GetAttrType(int iattr);

	private:
		int m_max_class;
		int m_max_attr;
		wxArrayString * m_S57ClassArray; // FIXME: replace wxArrayString with std containers
		wxArrayString * m_AttrArray; // FIXME: replace wxArrayString with std containers
		int * m_GeomTypeArray;
		char * m_ValTypeArray;
		bool m_ok;
		wxString m_dict_dir;
};

}

#endif
