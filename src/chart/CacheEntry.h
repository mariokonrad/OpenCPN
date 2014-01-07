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

#ifndef __CHART__CACHE_ENTRY__H__
#define __CHART__CACHE_ENTRY__H__

#include <wx/string.h>

namespace chart {

class CacheEntry
{
public:
	struct OldestFirst
	{
		bool operator()(const CacheEntry& a, const CacheEntry& b) const
		{
			return a.RecentTime < b.RecentTime;
		}
	};

public:
	wxString FullPath;
	void* pChart;
	int RecentTime;
	int dbIndex;
	bool b_in_use;
};

}

#endif
