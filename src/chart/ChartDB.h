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

#ifndef __CHART__CHARTDB__H__
#define __CHART__CHARTDB__H__

#include <wx/file.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/xml/xml.h>

#include <chart/ChartDatabase.h>
#include <chart/s52s57.h>
#include <chart/CacheEntry.h>

class MainFrame;

namespace chart {

struct MyFlPoint
{
	float y;
	float x;
};

class ChartBase;
class ChartStack;

class ChartDB : public ChartDatabase
{
public:
	ChartDB(MainFrame* parent);
	virtual ~ChartDB();

	bool LoadBinary(const wxString& filename, ArrayOfCDI& dir_array_check);
	bool SaveBinary(const wxString& filename);

	int BuildChartStack(ChartStack* cstk, float lat, float lon);
	int BuildChartStack(ChartStack* cstk, float lat, float lon, int db_add);
	bool EqualStacks(ChartStack*, ChartStack*);
	bool CopyStack(ChartStack* pa, ChartStack* pb);
	wxString GetFullPath(ChartStack* ps, int stackindex);
	int GetStackChartScale(ChartStack* ps, int stackindex, char* buf, int nbuf);
	int GetCSPlyPoint(ChartStack* ps, int stackindex, int plyindex, float* lat, float* lon);
	ChartTypeEnum GetCSChartType(ChartStack* ps, int stackindex);
	chart::ChartFamilyEnum GetCSChartFamily(ChartStack* ps, int stackindex);
	bool SearchForChartDir(const wxString& dir);
	ChartBase* OpenStackChartConditional(ChartStack* ps, int start_index, bool bLargest,
										 ChartTypeEnum New_Type,
										 chart::ChartFamilyEnum New_Family_Fallback);

	void get_chart_cache_copy(std::vector<CacheEntry>&) const;
	std::vector<int> GetCSArray(ChartStack* ps);

	int GetStackEntry(ChartStack* ps, wxString fp);
	bool IsChartInCache(int dbindex);
	bool IsChartInGroup(const int db_index, const int group);

	ChartBase* OpenChartFromStack(ChartStack* pStack, int StackEntry,
								  ChartInitFlag iflag = FULL_INIT);

	ChartBase* OpenChartFromDB(int index, ChartInitFlag init_flag);

	void ApplyColorSchemeToCachedCharts(global::ColorScheme cs);
	void PurgeCache();
	bool DeleteCacheChart(ChartBase* pChart);

	void LockCache(bool bl);
	void LockCache();
	void UnLockCache();
	bool IsCacheLocked() const;
	wxXmlDocument GetXMLDescription(int dbIndex, bool b_getGeom);

	void ClearCacheInUseFlags(void);
	void PurgeCacheUnusedCharts(bool b_force = false);

protected:
	virtual ChartBase* GetChart(const wxChar* theFilePath,
								const chart::ChartClassDescriptor& chart_desc) const;

private:
	InitReturn CreateChartTableEntry(wxString full_name, ChartTableEntry* pEntry);

	int SearchDirAndAddSENC(wxString& dir, bool bshow_prog, bool bupdate);
	bool CreateS57SENCChartTableEntry(wxString full_name, ChartTableEntry* pEntry, Extent* pext);
	bool CheckPositionWithinChart(int index, float lat, float lon);
	ChartBase* OpenChartUsingCache(int dbindex, ChartInitFlag init_flag);

	wxArrayPtrVoid* pChartCache; // FIXME: use std::vector instead of array to void*

	MainFrame* pParent;
	bool m_b_locked;
};

}

#endif
