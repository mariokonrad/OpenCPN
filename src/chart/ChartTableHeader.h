/***************************************************************************
 *
 * Project:  ChartManager
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

#ifndef __CHART__CHARTTABLEHEADER__H__
#define __CHART__CHARTTABLEHEADER__H__

class wxInputStream;
class wxOutputStream;

struct ChartTableHeader
{
	public:
		ChartTableHeader();
		ChartTableHeader(int dirEntries, int tableEntries);

		void Read(wxInputStream &is);
		void Write(wxOutputStream &os);
		bool CheckValid();
		int GetDirEntries() const;
		int GetTableEntries() const;
		char *GetDBVersionString();

	private:
		// NOTE: on-disk structure - cannot add, remove, or reorder!
		char dbVersion[4];
		int nTableEntries;
		int nDirEntries;
};

#endif
