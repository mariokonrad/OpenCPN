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

#ifndef __TIDE__TIDECURRENTMANAGER__H__
#define __TIDE__TIDECURRENTMANAGER__H__

#include <tide/TC_Error_Code.h>
#include <wx/string.h>
#include <vector>

namespace geo { class Position; }

namespace tide {

class IDX_entry;

/// Interface to the tides manager.
class TideCurrentManager
{
public:
	virtual ~TideCurrentManager()
	{
	}

	virtual TC_Error_Code LoadDataSources(const std::vector<wxString>& sources) = 0;
	virtual const std::vector<wxString>& GetDataSet(void) const = 0;
	virtual bool IsReady(void) const = 0;

	virtual bool GetTideOrCurrent(time_t t, int idx, float& value, float& dir) = 0;
	virtual bool GetTideOrCurrent15(time_t t, int idx, float& tcvalue, float& dir, bool& bnew_val)
		= 0;
	virtual bool GetTideFlowSens(time_t t, int sch_step, int idx, float& tcvalue_now,
								 float& tcvalue_prev, bool& w_t) = 0;
	virtual void GetHightOrLowTide(time_t t, int sch_step_1, int sch_step_2, float tide_val,
								   bool w_t, int idx, float& tcvalue, time_t& tctime) = 0;

	virtual int GetStationTimeOffset(IDX_entry* pIDX) = 0;
	virtual int GetNextBigEvent(time_t* tm, int idx) = 0;

	virtual int GetStationIDXbyName(const wxString& prefix, const geo::Position& pos) const = 0;

	virtual const IDX_entry* GetIDX_entry(int index) const = 0;
	virtual int Get_max_IDX() const = 0;
};

}

#endif
