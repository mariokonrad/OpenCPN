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

#ifndef __GLOBAL__AIS__H__
#define __GLOBAL__AIS__H__

#include <wx/string.h>

namespace global {

class AIS
{
public:
	virtual ~AIS()
	{
	}

public:
	struct Data
	{
		bool CPAMax;
		double CPAMax_NM;
		bool CPAWarn;
		double CPAWarn_NM;
		bool TCPA_Max;
		double TCPA_Max_min;
		bool MarkLost;
		double MarkLost_Mins;
		bool RemoveLost;
		double RemoveLost_Mins;
		bool AIS_CPA_Alert;
		bool AIS_CPA_Alert_Audio;
		bool AIS_ACK_Timeout;
		double AckTimeout_Mins;
		double AISShowTracks_Mins;
		bool ShowMoored;
		double ShowMoored_Kts;
		bool ShowCOG;
		double ShowCOG_Mins;
		bool AISShowTracks;
		bool TrackCarryOver;
		wxString AIS_Alert_Sound_File;
		bool AIS_CPA_Alert_Suppress_Moored;
		bool ShowAreaNotices;
		bool WplIsAprsPosition;
		bool AISRolloverShowClass;
		bool AISRolloverShowCOG;
		bool AISRolloverShowCPA;
	};

	virtual const Data& get_data() const = 0;
	virtual void set_CPAMax(bool) = 0;
	virtual void set_CPAMax_NM(double) = 0;
	virtual void set_CPAWarn(bool) = 0;
	virtual void set_CPAWarn_NM(double) = 0;
	virtual void set_TCPA_Max(bool) = 0;
	virtual void set_TCPA_Max_min(double) = 0;
	virtual void set_MarkLost(bool) = 0;
	virtual void set_MarkLost_Mins(double) = 0;
	virtual void set_RemoveLost(bool) = 0;
	virtual void set_RemoveLost_Mins(double) = 0;
	virtual void set_AIS_CPA_Alert(bool) = 0;
	virtual void set_AIS_CPA_Alert_Audio(bool) = 0;
	virtual void set_AIS_ACK_Timeout(bool) = 0;
	virtual void set_AckTimeout_Mins(double) = 0;
	virtual void set_AISShowTracks_Mins(double) = 0;
	virtual void set_ShowMoored(bool) = 0;
	virtual void set_ShowMoored_Kts(double) = 0;
	virtual void set_ShowCOG(bool) = 0;
	virtual void set_ShowCOG_Mins(double) = 0;
	virtual void set_AISShowTracks(bool) = 0;
	virtual void set_TrackCarryOver(bool) = 0;
	virtual void set_AIS_Alert_Sound_File(const wxString&) = 0;
	virtual void set_AIS_CPA_Alert_Suppress_Moored(bool) = 0;
	virtual void set_ShowAreaNotices(bool) = 0;
	virtual void set_WplIsAprsPosition(bool) = 0;
	virtual void set_AISRolloverShowClass(bool) = 0;
	virtual void set_AISRolloverShowCOG(bool) = 0;
	virtual void set_AISRolloverShowCPA(bool) = 0;
};

}

#endif
