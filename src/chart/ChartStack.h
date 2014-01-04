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

#ifndef __CHART__CHARTSTACK__H__
#define __CHART__CHARTSTACK__H__

namespace chart {

class ChartStack
{
public:
	enum {
		MAXSTACK = 100
	};

private:
	int DBIndex[MAXSTACK];

public:
	bool b_valid;
	int nEntry;
	int CurrentStackEntry;

public:
	ChartStack();

	int GetCurrentEntrydbIndex(void);
	void SetCurrentEntryFromdbIndex(int current_db_index);
	int GetDBIndex(int stack_index);
	void SetDBIndex(int stack_index, int db_index);
	bool DoesStackContaindbIndex(int db_index);
	void AddChart(int db_add);
};

}

#endif
