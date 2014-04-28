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

#include <chart/ChartType.h>
#include <chart/ChartFamily.h>
#include <wx/string.h>
#include <string>
#include <vector>

class wxInputStream;
class wxOutputStream;

namespace chart {

class ChartBase;

struct ChartTableEntry
{
public:
	ChartTableEntry();
	ChartTableEntry(const ChartBase& theChart);
	~ChartTableEntry();

	bool IsEqualTo(const ChartTableEntry& cte) const;
	bool IsEarlierThan(const ChartTableEntry& cte) const;
	bool read(int version, wxInputStream& is);
	bool write(wxOutputStream& os);
	void Disable();
	void SetValid(bool valid);
	time_t GetFileTime() const;

	void Clear();

	int GetnPlyEntries() const;
	const float* GetpPlyTable() const;

	int GetnAuxPlyEntries() const;
	const float* GetpAuxPlyTableEntry(int index) const;
	int GetAuxCntTableEntry(int index) const;

	int GetnNoCovrPlyEntries() const;
	const float* GetpNoCovrPlyTableEntry(int index) const;
	int GetNoCovrCntTableEntry(int index) const;

	wxString GetFullPath() const;
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
	std::vector<int>& GetGroupArray(void); // FIXME: breaks encapsulation
	const wxString& GetFileName(void) const;

private:
	std::string read_path(wxInputStream& is) const;
	void read_18(wxInputStream& is);
	void read_17(wxInputStream& is);
	void read_16(wxInputStream& is);
	void read_15(wxInputStream& is);
	void read_14(wxInputStream& is);

	void read_ply_table(wxInputStream& is);
	void read_aux_ply_table(wxInputStream& is);
	void read_nocovr_ply(wxInputStream& is);

private:
	int EntryOffset;
	ChartTypeEnum ChartType;
	ChartFamilyEnum ChartFamily;
	float LatMax;
	float LatMin;
	float LonMax;
	float LonMin;
	std::string fullpath;
	int Scale;
	time_t edition_date;
	time_t file_date;

	// ply table, data and number of entries. the entries are always
	// pairs of floats.
	std::vector<float> plytable;

	int nAuxPlyEntries;
	float** pAuxPlyTable; // FIXME: use std::vector
	int* pAuxCntTable; // FIXME: use std::vector

	float Skew;
	int ProjectionType;
	bool bValid;

	int nNoCovrPlyEntries;
	int* pNoCovrCntTable; // FIXME: use std::vector
	float** pNoCovrPlyTable; // FIXME: use std::vector

	std::vector<int> m_GroupArray;
	wxString m_filename; // a helper member, not on disk
};

}

#endif
