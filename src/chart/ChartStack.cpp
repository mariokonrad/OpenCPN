/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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

#include "ChartStack.h"
#include "chart/ChartDB.h" // FIXME: cyclic dependency

extern ChartDB * ChartData;

ChartStack::ChartStack()
{
	nEntry = 0;
	CurrentStackEntry = 0;
	b_valid = false;
}

int ChartStack::GetCurrentEntrydbIndex(void)
{
	if(nEntry /*&& b_valid*/)
		return DBIndex[CurrentStackEntry];
	else
		return -1;
}

void ChartStack::SetCurrentEntryFromdbIndex(int current_db_index)
{
	for(int i=0 ; i < nEntry ; i++)
	{
		if(current_db_index == DBIndex[i])
			CurrentStackEntry = i;
	}
}

int ChartStack::GetDBIndex(int stack_index)
{
	if((stack_index >= 0) && (stack_index < nEntry) && (stack_index < MAXSTACK))
		return DBIndex[stack_index];
	else
		return -1;
}

void ChartStack::SetDBIndex(int stack_index, int db_index)
{
	if((stack_index >= 0) && (stack_index < nEntry) && (stack_index < MAXSTACK))
		DBIndex[stack_index] = db_index;
}


bool ChartStack::DoesStackContaindbIndex(int db_index)
{
	for(int i=0 ; i < nEntry ; i++)
	{
		if(db_index == DBIndex[i])
			return true;
	}

	return false;
}


void ChartStack::AddChart( int db_add )
{
	if( !ChartData ) return;

	if( !ChartData->IsValid() ) return;

	int db_index = db_add;

	int j = nEntry;

	if(db_index >= 0) {
		j++;
		nEntry = j;
		SetDBIndex(j-1, db_index);
	}
	//    Remove exact duplicates, i.e. charts that have exactly the same file name and
	//     nearly the same mod time.
	//    These charts can be in the database due to having the exact same chart in different directories,
	//    as may be desired for some grouping schemes
	//    Note that if the target name is actually a directory, then windows fails to produce a valid
	//    file modification time.  Detect GetFileTime() == 0, and skip the test in this case
	for(int id = 0 ; id < j-1 ; id++)
	{
		if(GetDBIndex(id) != -1)
		{
			ChartTableEntry *pm = ChartData->GetpChartTableEntry(GetDBIndex(id));

			for(int jd = id+1; jd < j; jd++)
			{
				if(GetDBIndex(jd) != -1)
				{
					ChartTableEntry *pn = ChartData->GetpChartTableEntry(GetDBIndex(jd));
					if( pm->GetFileTime() && pn->GetFileTime()) {
						if( abs(pm->GetFileTime() - pn->GetFileTime()) < 60 ) {           // simple test
							if(pn->GetpFileName()->IsSameAs(*(pm->GetpFileName())))
								SetDBIndex(jd, -1);           // mark to remove
						}
					}
				}
			}
		}
	}

	int id = 0;
	while( (id < j) )
	{
		if(GetDBIndex(id) == -1)
		{
			int jd = id+1;
			while( jd < j )
			{
				int db_index = GetDBIndex(jd);
				SetDBIndex(jd-1, db_index);
				jd++;
			}

			j--;
			nEntry = j;

			id = 0;
		}
		else
			id++;
	}

	//    Sort the stack on scale
	int swap = 1;
	int ti;
	while(swap == 1)
	{
		swap = 0;
		for(int i=0 ; i<j-1 ; i++)
		{
			const ChartTableEntry &m = ChartData->GetChartTableEntry(GetDBIndex(i));
			const ChartTableEntry &n = ChartData->GetChartTableEntry(GetDBIndex(i+1));


			if(n.GetScale() < m.GetScale())
			{
				ti = GetDBIndex(i);
				SetDBIndex(i, GetDBIndex(i+1));
				SetDBIndex(i+1, ti);
				swap = 1;
			}
		}
	}
}

