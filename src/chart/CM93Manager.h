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

#ifndef __CHART__CM93MANAGER__H__
#define __CHART__CM93MANAGER__H__

#include <wx/string.h>

namespace chart {

class cm93_dictionary;

class cm93manager
{
public:
	cm93manager();
	~cm93manager();
	bool Loadcm93Dictionary(const wxString& name);
	cm93_dictionary* FindAndLoadDict(const wxString& file);

	cm93_dictionary* m_pcm93Dict;

	//  Member variables used to record the calling of cm93chart::CreateHeaderDataFromCM93Cell()
	//  for each available scale value.  This allows that routine to return quickly with no error
	//  for all cells other than the first, at each scale....

	bool m_bfoundA;
	bool m_bfoundB;
	bool m_bfoundC;
	bool m_bfoundD;
	bool m_bfoundE;
	bool m_bfoundF;
	bool m_bfoundG;
	bool m_bfoundZ;
};

}

#endif
