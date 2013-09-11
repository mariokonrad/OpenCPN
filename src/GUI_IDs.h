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

#ifndef __GUI_IDS__HPP__
#define __GUI_IDS__HPP__

#define TIMER_GFRAME_1 999

enum
{
	ID_ZOOMIN = 1550,
	ID_ZOOMOUT,
	ID_STKUP,
	ID_STKDN,
	ID_ROUTE,
	ID_FOLLOW,
	ID_SETTINGS,
	ID_AIS,           // pjotrc 2010.02.09
	ID_TEXT,
	ID_CURRENT,
	ID_TIDE,
	ID_HELP,
	ID_TBEXIT,
	ID_TBSTAT,
	ID_PRINT,
	ID_COLSCHEME,
	ID_ROUTEMANAGER,
	ID_TRACK,
	ID_TBSTATBOX,
	ID_MOB,
	ID_PLUGIN_BASE
};

// A global definition for window, timer and other ID's as needed.
enum
{
	FRAME_TIMER_1 = wxID_HIGHEST,
	TIMER_AIS1,
	TIMER_AISAUDIO,
	FRAME_TC_TIMER,
	FRAME_COG_TIMER,
	MEMORY_FOOTPRINT_TIMER,
};

#endif
