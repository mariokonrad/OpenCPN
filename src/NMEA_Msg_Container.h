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

#ifndef __NMEA_MSG_CONTAINER__H__
#define __NMEA_MSG_CONTAINER__H__

#include <wx/datetime.h>
#include <wx/string.h>

#include <map>

// A class to contain NMEA messages, their receipt time, and their source priority
class NMEA_Msg_Container
{
public:
	wxDateTime receipt_time;
	int current_priority;
	wxString stream_name;
};

typedef std::map<wxString, NMEA_Msg_Container*> NMEAMsgPriorityMap;

#endif
