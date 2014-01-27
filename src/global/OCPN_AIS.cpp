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

#include "OCPN_AIS.h"

namespace global {

const AIS::Data & OCPN_AIS::get_data() const
{
	return data;
}

void OCPN_AIS::set_CPAMax(bool value)
{
	data.CPAMax = value;
}

void OCPN_AIS::set_CPAMax_NM(double value)
{
	data.CPAMax_NM = value;
}

void OCPN_AIS::set_CPAWarn(bool value)
{
	data.CPAWarn = value;
}

void OCPN_AIS::set_CPAWarn_NM(double value)
{
	data.CPAWarn_NM = value;
}

void OCPN_AIS::set_TCPA_Max(bool value)
{
	data.TCPA_Max = value;
}

void OCPN_AIS::set_TCPA_Max_min(double value)
{
	data.TCPA_Max_min = value;
}

void OCPN_AIS::set_MarkLost(bool value)
{
	data.MarkLost = value;
}

void OCPN_AIS::set_MarkLost_Mins(double value)
{
	data.MarkLost_Mins = value;
}

void OCPN_AIS::set_RemoveLost(bool value)
{
	data.RemoveLost = value;
}

void OCPN_AIS::set_RemoveLost_Mins(double value)
{
	data.RemoveLost_Mins = value;
}

void OCPN_AIS::set_AIS_CPA_Alert(bool value)
{
	data.AIS_CPA_Alert = value;
}

void OCPN_AIS::set_AIS_CPA_Alert_Audio(bool value)
{
	data.AIS_CPA_Alert_Audio = value;
}

void OCPN_AIS::set_AIS_ACK_Timeout(bool value)
{
	data.AIS_ACK_Timeout = value;
}

void OCPN_AIS::set_AckTimeout_Mins(double value)
{
	data.AckTimeout_Mins = value;
}

void OCPN_AIS::set_AISShowTracks_Mins(double value)
{
	data.AISShowTracks_Mins = value;
}

void OCPN_AIS::set_ShowMoored(bool value)
{
	data.ShowMoored = value;
}

void OCPN_AIS::set_ShowMoored_Kts(double value)
{
	data.ShowMoored_Kts = value;
}

void OCPN_AIS::set_ShowCOG(bool value)
{
	data.ShowCOG = value;
}

void OCPN_AIS::set_ShowCOG_Mins(double value)
{
	data.ShowCOG_Mins = value;
}

void OCPN_AIS::set_AISShowTracks(bool value)
{
	data.AISShowTracks = value;
}

void OCPN_AIS::set_TrackCarryOver(bool value)
{
	data.TrackCarryOver = value;
}

void OCPN_AIS::set_AIS_Alert_Sound_File(const wxString& value)
{
	data.AIS_Alert_Sound_File = value;
}

void OCPN_AIS::set_AIS_CPA_Alert_Suppress_Moored(bool value)
{
	data.AIS_CPA_Alert_Suppress_Moored = value;
}

void OCPN_AIS::set_ShowAreaNotices(bool value)
{
	data.ShowAreaNotices = value;
}

void OCPN_AIS::set_WplIsAprsPosition(bool value)
{
	data.WplIsAprsPosition = value;
}

void OCPN_AIS::set_AISRolloverShowClass(bool value)
{
	data.AISRolloverShowClass = value;
}

void OCPN_AIS::set_AISRolloverShowCOG(bool value)
{
	data.AISRolloverShowCOG = value;
}

void OCPN_AIS::set_AISRolloverShowCPA(bool value)
{
	data.AISRolloverShowCPA = value;
}

}

