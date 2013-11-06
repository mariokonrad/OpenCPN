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

#ifndef __CHARTDBS_H__
#define __CHARTDBS_H__

#include <wx/dynarray.h>
#include <wx/file.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/dir.h>
#include <wx/filename.h>

#include <vector>

#include <MainFrame.h>
#include <ChartDirInfo.h>

#include <chart/ChartBase.h>
#include <chart/ChartTableHeader.h>
#include <chart/ChartTableEntry.h>
#include <chart/ChartClassDescriptor.h>

class wxProgressDialog;
class ChartDatabase;
class ChartGroupArray;

namespace geo { class BoundingBox; }

enum
{
	BUILTIN_DESCRIPTOR = 0,
	PLUGIN_DESCRIPTOR
};


///////////////////////////////////////////////////////////////////////
// Chart Database
///////////////////////////////////////////////////////////////////////

WX_DECLARE_OBJARRAY(ChartTableEntry, ChartTable); // FIXME: use std container
WX_DECLARE_OBJARRAY(ChartClassDescriptor, ArrayOfChartClassDescriptor); // FIXME: use std container

class ChartDatabase
{
	public:
		ChartDatabase();
		virtual ~ChartDatabase(){};

		bool Create(ArrayOfCDI & dir_array, wxProgressDialog *pprog);
		bool Update(ArrayOfCDI & dir_array, bool bForce, wxProgressDialog *pprog);

		bool Read(const wxString &filePath);
		bool Write(const wxString &filePath);

		const wxString & GetDBFileName() const;
		ArrayOfCDI & GetChartDirArray();
		wxArrayString &GetChartDirArrayString();

		void UpdateChartClassDescriptorArray(void);

		int GetChartTableEntries() const;
		const ChartTableEntry &GetChartTableEntry(int index) const;

		bool IsValid() const;
		int DisableChart(wxString& PathToDisable);
		bool GetCentroidOfLargestScaleChart(double *clat, double *clon, chart::ChartFamilyEnum family);
		int GetDBChartType(int dbIndex);
		int GetDBChartFamily(int dbIndex);
		float GetDBChartSkew(int dbIndex);
		int GetDBChartProj(int dbIndex);
		int GetDBChartScale(int dbIndex);

		bool GetDBBoundingBox(int dbindex, geo::BoundingBox *box);
		int  GetnAuxPlyEntries(int dbIndex);
		int  GetDBPlyPoint(int dbIndex, int plyindex, float *lat, float *lon);
		int  GetDBAuxPlyPoint(int dbIndex, int plyindex, int iAuxPly, float *lat, float *lon);
		int  GetVersion() const;
		wxString GetFullChartInfo(ChartBase *pc, int dbIndex, int *char_width, int *line_count);
		int FinddbIndex(wxString PathToFind);
		wxString GetDBChartFileName(int dbIndex);
		void ApplyGroupArray(ChartGroupArray *pGroupArray);

	protected:
		virtual ChartBase *GetChart(const wxChar *theFilePath, ChartClassDescriptor &chart_desc) const;
		int AddChartDirectory(const wxString &theDir, bool bshow_prog);
		void SetValid(bool valid) { bValid = valid; }
		ChartTableEntry *CreateChartTableEntry(const wxString &filePath, ChartClassDescriptor &chart_desc);

		ArrayOfChartClassDescriptor m_ChartClassDescriptorArray;
		ArrayOfCDI m_dir_array;

	private:
		bool IsChartDirUsed(const wxString &theDir);

		int SearchDirAndAddCharts(wxString& dir_name_base, ChartClassDescriptor &chart_desc, wxProgressDialog *pprog);

		int TraverseDirAndAddCharts(
				const ChartDirInfo & dir_info,
				wxProgressDialog * pprog,
				wxString & dir_magic,
				bool bForce);
		bool DetectDirChange(const wxString & dir_path, const wxString & magic, wxString &new_magic, wxProgressDialog *pprog);

		bool Check_CM93_Structure(wxString dir_name);

		bool bValid;
		wxArrayString m_chartDirs;
		int m_dbversion;
		ChartTable chartTable;

		ChartTableEntry m_ChartTableEntryDummy; // FIXME: used for return value if database is not valid
		wxString m_DBFileName;
};


//-------------------------------------------------------------------------------------------
//    Chart Group Structure Definitions
//-------------------------------------------------------------------------------------------
class ChartGroupElement;
class ChartGroup;

WX_DECLARE_OBJARRAY(ChartGroupElement*, ChartGroupElementArray); // FIXME: use std container
WX_DECLARE_OBJARRAY(ChartGroup*, ChartGroupArray); // FIXME: use std container

class ChartGroupElement
{
	public:
		wxString m_element_name;
		wxArrayString m_missing_name_array;
};

class ChartGroup
{
	public:
		wxString m_group_name;
		ChartGroupElementArray m_element_array;
};


#endif
