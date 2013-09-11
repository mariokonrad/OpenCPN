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

#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>
#include <wx/sound.h>

#ifdef __WXMSW__
	#include <wx/msw/regconf.h>
	#include <wx/msw/iniconf.h>
#endif

#ifdef OCPN_USE_PORTAUDIO
	#include "portaudio.h"
#endif

#include "s52s57.h"
#include "chcanv.h"
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
extern void AlphaBlending(
		ocpnDC & dc,
		int x, int y,
		int size_x, int size_y,
		float radius,
		wxColour color,
		unsigned char transparency);

extern double fromDMM(wxString sdms);

class Route;
class wxProgressDialog;
class ocpnDC;
class NavObjectCollection;
class NavObjectChanges;
class GpxWptElement;
class GpxRteElement;
class GpxTrkElement;

RoutePoint * WaypointExists(const wxString & name, double lat, double lon);
RoutePoint * WaypointExists(const wxString & guid);
Route * RouteExists(const wxString & guid);
Route * RouteExists(Route * pTentRoute);
const wxChar * ParseGPXDateTime(wxDateTime & dt, const wxChar * datetime);

class MyConfig : public wxFileConfig
{
	private:
		void load_view();
		void load_frame();
		void load_toolbar();
		void load_ais_alert_dialog();
		void load_ais_query_dialog();
		void load_system_config(int);

		void write_view();
		void write_frame();
		void write_toolbar();
		void write_ais_alert_dialog();
		void write_ais_query_dialog();
		void write_system_config();

	public:

		MyConfig(
				const wxString & appName,
				const wxString & vendorName,
				const wxString & LocalFileName);

		int LoadMyConfig(int iteration);

		virtual bool AddNewRoute(Route * pr, int ConfigRouteNum = -1);
		virtual bool UpdateRoute(Route * pr);
		virtual bool DeleteConfigRoute(Route * pr);

		virtual bool AddNewWayPoint(RoutePoint * pWP, int ConfigRouteNum = -1);
		virtual bool UpdateWayPoint(RoutePoint * pWP);
		virtual bool DeleteWayPoint(RoutePoint * pWP);

		virtual void CreateConfigGroups(ChartGroupArray * pGroupArray);
		virtual void DestroyConfigGroups(void);
		virtual void LoadConfigGroups(ChartGroupArray * pGroupArray);


		virtual bool UpdateChartDirs(ArrayOfCDI & dirarray);
		virtual bool LoadChartDirArray(ArrayOfCDI & ChartDirArray);
		virtual void UpdateSettings();
		virtual void UpdateNavObj();
		virtual void StoreNavObjChanges();

		bool LoadLayers(wxString &path);

		void ExportGPX(
				wxWindow * parent,
				bool bviz_only = false,
				bool blayer = false);

		void UI_ImportGPX(
				wxWindow * parent,
				bool islayer = false,
				wxString dirpath = _T(""),
				bool isdirectory = true);

		bool ExportGPXRoutes(
				wxWindow * parent,
				RouteList * pRoutes,
				const wxString suggestedName = _T("routes"));

		bool ExportGPXWaypoints(
				wxWindow * parent,
				RoutePointList * pRoutePoints,
				const wxString suggestedName = _T("waypoints"));

		void CreateRotatingNavObjBackup();

		double st_lat;
		double st_lon;
		double st_view_scale;
		bool st_bFollow;

		wxString m_gpx_path;

		wxString m_sNavObjSetFile;
		wxString m_sNavObjSetChangesFile;

		NavObjectChanges * m_pNavObjectChangesSet;
		NavObjectCollection * m_pNavObjectInputSet;
		bool m_bSkipChangeSetUpdate;
		bool  m_bShowDebugWindows;
};

#endif
