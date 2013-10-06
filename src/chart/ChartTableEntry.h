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

#ifndef __CHART__CHARTTABLEENTRY__H__
#define __CHART__CHARTTABLEENTRY__H__

#include <vector>
#include <wx/string.h>

class wxInputStream;
class wxOutputStream;
class ChartDatabase;
class ChartBase;

struct ChartTableEntry
{
	public:
		ChartTableEntry();
		ChartTableEntry(ChartBase &theChart);
		~ChartTableEntry();

		bool IsEqualTo(const ChartTableEntry &cte) const;
		bool IsEarlierThan(const ChartTableEntry &cte) const;
		bool Read(const ChartDatabase * pDb, wxInputStream & is);
		bool Write(const ChartDatabase * pDb, wxOutputStream & os);
		void Disable();
		void SetValid(bool valid);
		time_t GetFileTime() const;

		void Clear();

		int GetnPlyEntries() const;
		float *GetpPlyTable() const;

		int GetnAuxPlyEntries() const;
		float *GetpAuxPlyTableEntry(int index) const;
		int GetAuxCntTableEntry(int index) const;

		int GetnNoCovrPlyEntries() const;
		float *GetpNoCovrPlyTableEntry(int index) const;
		int GetNoCovrCntTableEntry(int index) const;

		char * GetpFullPath() const;
		float GetLonMax() const;
		float GetLonMin() const;
		float GetLatMax() const;
		float GetLatMin() const;
		int GetScale() const;
		int GetChartType() const;
		int GetChartFamily() const;
		int GetChartProjectionType() const;
		float GetChartSkew() const;

		bool GetbValid() const;
		void SetEntryOffset(int n);
		std::vector<int> & GetGroupArray(void);
		const wxString & GetFileName(void) const;

	private:
		void read_17(wxInputStream &is);
		void read_16(wxInputStream &is);
		void read_15(wxInputStream &is);
		void read_14(wxInputStream &is);

	private:
		int         EntryOffset;
		int         ChartType; // FIXME: use enum
		float       LatMax;
		float       LatMin;
		float       LonMax;
		float       LonMin;
		char        *pFullPath; // FIXME: use string
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
		wxString m_filename; // a helper member, not on disk
};



#endif
