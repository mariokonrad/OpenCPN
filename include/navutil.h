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

#ifndef __NAVUTIL__
#define __NAVUTIL__

#include <wx/sound.h>

#ifdef __WXMSW__
	#include <wx/msw/regconf.h>
	#include <wx/msw/iniconf.h>
#endif

#ifdef OCPN_USE_PORTAUDIO
	#include "portaudio.h"
#endif

#include "s52s57.h"
#include "tinyxml/tinyxml.h"
#include "chart/ChartDatabase.h"
#include "RoutePoint.h"
#include "Vector2D.h"
#include "Route.h"
#include "SelectItem.h"

enum
{
	DISTANCE_NMI = 0,
	DISTANCE_MI,
	DISTANCE_KM,
	DISTANCE_M
};

enum
{
	SPEED_KTS = 0,
	SPEED_MPH,
	SPEED_KMH,
	SPEED_MS
};

extern bool LogMessageOnce(const wxString & msg);
extern double toUsrDistance(double nm_distance, int unit = -1);
extern double fromUsrDistance(double usr_distance, int unit = -1);
extern double toUsrSpeed(double kts_speed, int unit = -1);
extern double fromUsrSpeed(double usr_speed, int unit = -1);
extern wxString getUsrDistanceUnit(int unit = -1);
extern wxString getUsrSpeedUnit(int unit = -1);
extern wxString toSDMM(int NEflag, double a, bool hi_precision = true);
extern double fromDMM(wxString sdms);

class Route;
class wxProgressDialog;
class ocpnDC;
class NavObjectCollection;
class NavObjectChanges;
class GpxWptElement;
class GpxRteElement;
class GpxTrkElement;

Route * RouteExists(const wxString & guid);
Route * RouteExists(Route * pTentRoute);

#endif
