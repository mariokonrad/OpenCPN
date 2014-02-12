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

#ifndef __CHART__CHARTDATABASE__H__
#define __CHART__CHARTDATABASE__H__

#include <wx/dynarray.h>
#include <wx/file.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/dir.h>
#include <wx/filename.h>

#include <vector>

#include <MainFrame.h>
#include <ChartDirectoryInfo.h>

#include <chart/ChartBase.h>
#include <chart/ChartTableHeader.h>
#include <chart/ChartTableEntry.h>
#include <chart/ChartClassDescriptor.h>
#include <chart/ChartGroup.h>

class wxProgressDialog;

namespace geo { class BoundingBox; }

namespace chart {

enum
{
	BUILTIN_DESCRIPTOR = 0,
	PLUGIN_DESCRIPTOR
};

WX_DECLARE_OBJARRAY(ChartTableEntry, ChartTable); // FIXME: use std container

class ChartDatabase
{
public:
	ChartDatabase();
	virtual ~ChartDatabase() {};

	bool Create(ChartDirectories& dir_array, wxProgressDialog* pprog);
	bool Update(ChartDirectories& dir_array, bool bForce, wxProgressDialog* pprog);

	bool Read(const wxString& filePath);
	bool Write(const wxString& filePath);

	bool AddSingleChart(wxString& fullpath);
	bool RemoveSingleChart(wxString& ChartFullPath);

	const wxString& GetDBFileName() const;
	const ChartDirectories& GetChartDirArray() const;
	const wxArrayString& GetChartDirArrayString() const;
	void SetChartDirArray(const ChartDirectories& array);

	void UpdateChartClassDescriptorArray(void);

	int GetChartTableEntries() const;
	const ChartTableEntry& GetChartTableEntry(int index) const;

	bool IsValid() const;
	int DisableChart(const wxString& PathToDisable);
	bool GetCentroidOfLargestScaleChart(double* clat, double* clon, chart::ChartFamilyEnum family) const;
	int GetDBChartType(int dbIndex) const;
	int GetDBChartFamily(int dbIndex) const;
	float GetDBChartSkew(int dbIndex) const;
	int GetDBChartProj(int dbIndex) const;
	int GetDBChartScale(int dbIndex) const;

	bool GetDBBoundingBox(int dbindex, geo::BoundingBox* box) const;
	int GetnAuxPlyEntries(int dbIndex) const;
	int GetDBPlyPoint(int dbIndex, int plyindex, float* lat, float* lon) const;
	int GetDBAuxPlyPoint(int dbIndex, int plyindex, int iAuxPly, float* lat, float* lon) const;
	int GetVersion() const;
	wxString GetFullChartInfo(ChartBase* pc, int dbIndex, int* char_width, int* line_count) const;
	int FinddbIndex(const wxString& PathToFind) const;
	wxString GetDBChartFileName(int dbIndex) const;
	void ApplyGroupArray(chart::ChartGroupArray* pGroupArray);

protected:
	typedef std::vector<chart::ChartClassDescriptor> ArrayOfChartClassDescriptor;

	virtual ChartBase* GetChart(const wxChar* theFilePath,
								const chart::ChartClassDescriptor& chart_desc) const;
	int AddChartDirectory(const wxString& theDir, bool bshow_prog);
	void SetValid(bool valid);
	ChartTableEntry* CreateChartTableEntry(const wxString& filePath,
										   const chart::ChartClassDescriptor& chart_desc);

	wxString getChartClassName(int type, const wxString& ext) const;

	ChartDirectories m_dir_array;

private:
	ChartTableEntry& GetWritableChartTableEntry(int index);
	bool IsChartDirUsed(const wxString& theDir) const;

	int SearchDirAndAddCharts(wxString& dir_name_base,
							  const chart::ChartClassDescriptor& chart_desc,
							  wxProgressDialog* pprog);

	int TraverseDirAndAddCharts(const ChartDirectoryInfo& dir_info, wxProgressDialog* pprog,
								wxString& dir_magic, bool bForce);
	bool DetectDirChange(const wxString& dir_path, const wxString& magic, wxString& new_magic,
						 wxProgressDialog* pprog) const;

	bool AddChart(wxString& chartfilename, ChartClassDescriptor& chart_desc,
				  wxProgressDialog* pprog, int isearch, bool bthis_dir_in_dB);

	bool Check_CM93_Structure(wxString dir_name) const;

	bool bValid;
	wxArrayString m_chartDirs;
	int m_dbversion;
	ChartTable chartTable;

	ArrayOfChartClassDescriptor m_ChartClassDescriptorArray;
	ChartTableEntry m_ChartTableEntryDummy; // FIXME: used for return value if database is not valid
	wxString m_DBFileName;

	int m_pdifile;
	int m_pdnFile;
};

}

#endif
