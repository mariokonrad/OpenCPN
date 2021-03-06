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

#include <tide/TideCurrentManager.h>
#include <tide/Station_Data.h>
#include <tide/TCDataSource.h>

namespace tide {

class TCMgr : public TideCurrentManager
{
public:
	TCMgr();
	virtual ~TCMgr();

	virtual TC_Error_Code LoadDataSources(const std::vector<wxString> & sources);

	virtual const std::vector<wxString>& GetDataSet(void)const;
	virtual bool IsReady(void)const;

	virtual bool GetTideOrCurrent(time_t t, int idx, float & value, float & dir);
	virtual bool GetTideOrCurrent15(time_t t, int idx, float & tcvalue, float & dir,
									bool & bnew_val);
	virtual bool GetTideFlowSens(time_t t, int sch_step, int idx, float & tcvalue_now,
								 float & tcvalue_prev, bool & w_t);
	virtual void GetHightOrLowTide(time_t t, int sch_step_1, int sch_step_2, float tide_val,
								   bool w_t, int idx, float & tcvalue, time_t & tctime);

	virtual int GetStationTimeOffset(IDX_entry * pIDX);
	virtual int GetNextBigEvent(time_t * tm, int idx);

	virtual const IDX_entry* GetIDX_entry(int index) const;
	virtual int Get_max_IDX() const;

	virtual int GetStationIDXbyName(const wxString & prefix, const geo::Position & pos) const;

	int GetStationIDXbyNameType(const wxString & prefix, const geo::Position & pos,
								char type) const; // FIXME: unused

private:
	int station_idx_name_types(const wxString & prefix, const geo::Position & pos,
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
