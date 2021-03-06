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

#ifndef __GLOBAL__OCPN_AIS__H__
#define __GLOBAL__OCPN_AIS__H__

#include <global/AIS.h>

namespace global {

class OCPN_AIS : public AIS
{
private:
	Data data;

public: // data
	virtual const Data& get_data() const;
	virtual void set_CPAMax(bool);
	virtual void set_CPAMax_NM(double);
	virtual void set_CPAWarn(bool);
	virtual void set_CPAWarn_NM(double);
	virtual void set_TCPA_Max(bool);
	virtual void set_TCPA_Max_min(double);
	virtual void set_MarkLost(bool);
	virtual void set_MarkLost_Mins(double);
	virtual void set_RemoveLost(bool);
	virtual void set_RemoveLost_Mins(double);
	virtual void set_AIS_CPA_Alert(bool);
	virtual void set_AIS_CPA_Alert_Audio(bool);
	virtual void set_AIS_ACK_Timeout(bool);
	virtual void set_AckTimeout_Mins(double);
	virtual void set_AISShowTracks_Mins(double);
	virtual void set_ShowMoored(bool);
	virtual void set_ShowMoored_Kts(double);
	virtual void set_ShowCOG(bool);
	virtual void set_ShowCOG_Mins(double);
	virtual void set_AISShowTracks(bool);
	virtual void set_TrackCarryOver(bool);
	virtual void set_AIS_Alert_Sound_File(const wxString&);
	virtual void set_AIS_CPA_Alert_Suppress_Moored(bool);
	virtual void set_ShowAreaNotices(bool);
	virtual void set_WplIsAprsPosition(bool);
	virtual void set_AISRolloverShowClass(bool);
	virtual void set_AISRolloverShowCOG(bool);
	virtual void set_AISRolloverShowCPA(bool);
};

}

#endif
