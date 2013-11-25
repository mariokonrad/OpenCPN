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

#ifndef __CONFIG__H__
#define __CONFIG__H__

#include <ChartDirInfo.h>
#include <chart/ChartDatabase.h>
#include <RoutePoint.h>
#include <Route.h>

#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>

class NavObjectChanges;
class NavObjectCollection;
class wxWindow;

class Config : public wxFileConfig
{
public:
	Config(const wxString& appName, const wxString& vendorName, const wxString& LocalFileName);

	int LoadConfig(int iteration);

	virtual bool AddNewRoute(Route* pr, int ConfigRouteNum = -1);
	virtual bool UpdateRoute(Route* pr);
	virtual bool DeleteConfigRoute(Route* pr);

	virtual bool AddNewWayPoint(RoutePoint* pWP, int ConfigRouteNum = -1);
	virtual bool UpdateWayPoint(RoutePoint* pWP);
	virtual bool DeleteWayPoint(RoutePoint* pWP);

	virtual void CreateConfigGroups(chart::ChartGroupArray* pGroupArray);
	virtual void DestroyConfigGroups(void);
	virtual void LoadConfigGroups(chart::ChartGroupArray* pGroupArray);

	virtual bool UpdateChartDirs(ArrayOfCDI& dirarray);
	virtual bool LoadChartDirArray(ArrayOfCDI& ChartDirArray);
	virtual void UpdateSettings();
	virtual void UpdateNavObj();
	virtual void StoreNavObjChanges();

	bool LoadLayers(wxString& path);

	void ExportGPX(wxWindow* parent, bool bviz_only = false, bool blayer = false);

	void UI_ImportGPX(wxWindow* parent, bool islayer = false, wxString dirpath = _T(""),
					  bool isdirectory = true);

	bool ExportGPXRoutes(wxWindow* parent, RouteList* pRoutes,
						 const wxString suggestedName = _T("routes"));

	bool ExportGPXWaypoints(wxWindow* parent, RoutePointList* pRoutePoints,
							const wxString suggestedName = _T("waypoints"));

	void CreateRotatingNavObjBackup();

	// FIXME: move public attributes to private

	double st_lat;
	double st_lon;
	double st_view_scale;
	bool st_bFollow;

	wxString m_gpx_path;

	wxString m_sNavObjSetFile;
	wxString m_sNavObjSetChangesFile;

	NavObjectChanges* m_pNavObjectChangesSet;
	NavObjectCollection* m_pNavObjectInputSet;
	bool m_bSkipChangeSetUpdate;
	bool m_bShowDebugWindows;

private:
	void load_view();
	void load_frame();
	void load_toolbar();
	void load_ais_alert_dialog();
	void load_ais_query_dialog();
	void load_system_config(int);
	void load_watchdog();
	void load_cm93(int, int);

	void write_view();
	void write_frame();
	void write_toolbar();
	void write_ais_alert_dialog();
	void write_ais_query_dialog();
	void write_system_config();
	void write_cm93();

	static bool WptIsInRouteList(const RoutePoint* pr);

	wxString visibleLayers;
	wxString invisibleLayers;
};

#endif
