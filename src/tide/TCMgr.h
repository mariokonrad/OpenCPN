/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Tide and Current Manager
 * Author:   David Register
 * Todo add original author
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

#ifndef __TIDE__TCMGR_H__
#define __TIDE__TCMGR_H__

#include <wx/arrstr.h>

#include <tide/Station_Data.h>
#include <tide/TC_Error_Code.h>
#include <tide/TCDataSource.h>

#include <vector>
#include <cmath>

namespace tide {

class IDX_entry;

class TCMgr
{
public:
	TCMgr();
	~TCMgr();

	TC_Error_Code LoadDataSources(const std::vector<wxString>& sources);

	const std::vector<wxString>& GetDataSet(void) const;
	bool IsReady(void) const;

	bool GetTideOrCurrent(time_t t, int idx, float& value, float& dir);
	bool GetTideOrCurrent15(time_t t, int idx, float& tcvalue, float& dir, bool& bnew_val);
	bool GetTideFlowSens(time_t t, int sch_step, int idx, float& tcvalue_now, float& tcvalue_prev,
						 bool& w_t);
	void GetHightOrLowTide(time_t t, int sch_step_1, int sch_step_2, float tide_val, bool w_t,
						   int idx, float& tcvalue, time_t& tctime);

	int GetStationTimeOffset(IDX_entry* pIDX);
	int GetNextBigEvent(time_t* tm, int idx);

	const IDX_entry* GetIDX_entry(int index) const;
	int Get_max_IDX() const;
	int GetStationIDXbyName(const wxString& prefix, double xlat, double xlon) const;
	int GetStationIDXbyNameType(const wxString& prefix, double xlat, double xlon, char type) const;

private:
	int station_idx_name_types(const wxString& prefix, double xlat, double xlon,
							   std::vector<char> types) const;
	void PurgeData();

	void LoadMRU(void);
	void SaveMRU(void);
	void AddMRU(Station_Data* psd);
	void FreeMRU(void);

	bool bTCMReady;
	wxString pmru_file_name;

	std::vector<TCDataSource*> m_source_array;
	std::vector<wxString> m_sourcefile_array;
	std::vector<IDX_entry*> m_Combined_IDX_array;
};

}

#endif
