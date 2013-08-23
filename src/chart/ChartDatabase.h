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

#include "chartbase.h"
#include "chart/ChartTableHeader.h"
#include "chart/ChartClassDescriptor.h"
#include "chart1.h"

class wxProgressDialog;
class ChartDatabase;
class ChartGroupArray;
class BoundingBox;


struct ChartTableEntry
{
	ChartTableEntry();
	ChartTableEntry(ChartBase &theChart);
	~ChartTableEntry();

	bool IsEqualTo(const ChartTableEntry &cte) const;
	bool IsEarlierThan(const ChartTableEntry &cte) const;
	bool Read(const ChartDatabase *pDb, wxInputStream &is);
	bool Write(const ChartDatabase *pDb, wxOutputStream &os);
	void Clear();
	void Disable();
	void SetValid(bool valid);
	time_t GetFileTime() const;

	int GetnPlyEntries() const;
	float *GetpPlyTable() const;

	int GetnAuxPlyEntries() const;
	float *GetpAuxPlyTableEntry(int index) const;
	int GetAuxCntTableEntry(int index) const;

	int GetnNoCovrPlyEntries() const;
	float *GetpNoCovrPlyTableEntry(int index) const;
	int GetNoCovrCntTableEntry(int index) const;

	char *GetpFullPath() const;
	float GetLonMax() const;
	float GetLonMin() const;
	float GetLatMax() const;
	float GetLatMin() const;
	int GetScale() const;
	int GetChartType() const;
	int GetChartFamily() const;
	int GetChartProjectionType() const;
	float GetChartSkew() const;

	bool GetbValid();
	void SetEntryOffset(int n);
	std::vector<int> &GetGroupArray(void);
	wxString *GetpFileName(void);

	private:
	int         EntryOffset;
	int         ChartType;
	float       LatMax;
	float       LatMin;
	float       LonMax;
	float       LonMin;
	char        *pFullPath;
	int         Scale;
	time_t      edition_date;
	time_t      file_date;
	float       *pPlyTable;
	int         nPlyEntries;
	int         nAuxPlyEntries;
	float       **pAuxPlyTable;
	int         *pAuxCntTable;
	float       Skew;
	int         ProjectionType;
	bool        bValid;
	int         nNoCovrPlyEntries;
	int         *pNoCovrCntTable;
	float       **pNoCovrPlyTable;

	std::vector<int> m_GroupArray;
	wxString    *m_pfilename;             // a helper member, not on disk
};

enum
{
	BUILTIN_DESCRIPTOR = 0,
	PLUGIN_DESCRIPTOR
};


///////////////////////////////////////////////////////////////////////
// Chart Database
///////////////////////////////////////////////////////////////////////

WX_DECLARE_OBJARRAY(ChartTableEntry, ChartTable);
WX_DECLARE_OBJARRAY(ChartClassDescriptor, ArrayOfChartClassDescriptor);

class ChartDatabase
{
	public:
		ChartDatabase();
		virtual ~ChartDatabase(){};

		bool Create(ArrayOfCDI& dir_array, wxProgressDialog *pprog);
		bool Update(ArrayOfCDI& dir_array, bool bForce, wxProgressDialog *pprog);

		bool Read(const wxString &filePath);
		bool Write(const wxString &filePath);

		const wxString & GetDBFileName() const { return m_DBFileName; }
		ArrayOfCDI& GetChartDirArray(){ return m_dir_array; }
		wxArrayString &GetChartDirArrayString(){ return m_chartDirs; }

		void UpdateChartClassDescriptorArray(void);

		int GetChartTableEntries() const { return chartTable.size(); }
		const ChartTableEntry &GetChartTableEntry(int index) const;
		ChartTableEntry *GetpChartTableEntry(int index) const;

		bool IsValid() const { return bValid; }
		int DisableChart(wxString& PathToDisable);
		bool GetCentroidOfLargestScaleChart(double *clat, double *clon, ChartFamilyEnum family);
		int GetDBChartType(int dbIndex);
		int GetDBChartFamily(int dbIndex);
		float GetDBChartSkew(int dbIndex);
		int GetDBChartProj(int dbIndex);
		int GetDBChartScale(int dbIndex);

		bool GetDBBoundingBox(int dbindex, BoundingBox *box);
		int  GetnAuxPlyEntries(int dbIndex);
		int  GetDBPlyPoint(int dbIndex, int plyindex, float *lat, float *lon);
		int  GetDBAuxPlyPoint(int dbIndex, int plyindex, int iAuxPly, float *lat, float *lon);
		int  GetVersion(){ return m_dbversion; }
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

		int TraverseDirAndAddCharts(ChartDirInfo& dir_info, wxProgressDialog *pprog, wxString& dir_magic, bool bForce);
		bool DetectDirChange(const wxString & dir_path, const wxString & magic, wxString &new_magic, wxProgressDialog *pprog);

		bool Check_CM93_Structure(wxString dir_name);

		bool bValid;
		wxArrayString m_chartDirs;
		int m_dbversion;
		ChartTable chartTable;

		ChartTableEntry m_ChartTableEntryDummy;   // used for return value if database is not valid
		wxString m_DBFileName;
};


//-------------------------------------------------------------------------------------------
//    Chart Group Structure Definitions
//-------------------------------------------------------------------------------------------
class ChartGroupElement;
class ChartGroup;

WX_DECLARE_OBJARRAY(ChartGroupElement*, ChartGroupElementArray);
WX_DECLARE_OBJARRAY(ChartGroup*, ChartGroupArray);

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
