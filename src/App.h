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

#ifndef __APP__H__
#define __APP__H__

#include <wx/app.h>

class wxCmdLineParser;
class wxActivateEvent;
class wxSingleInstanceChecker;
class wxLog;

namespace global
{
class OCPN_GUI;
class OCPN_Navigation;
class OCPN_AIS;
class OCPN_WatchDog;
class OCPN_System;
class OCPN_Runtime;
class ColorManager;
class ColorProvider;
}

namespace navigation
{
class RouteTracker;
class RouteManager;
class WaypointManager;
}

namespace tide
{
class TideCurrentManager;
}

namespace gui
{
class StyleManager;
}

class App : public wxApp
{
	DECLARE_EVENT_TABLE()

public:
	App();
	bool OnInit();
	int OnExit();
	void OnInitCmdLine(wxCmdLineParser& parser);
	bool OnCmdLineParsed(wxCmdLineParser& parser);
	void OnActivateApp(wxActivateEvent& event);

	wxSingleInstanceChecker* m_checker;

private:
	void inject_global_instances();
	void establish_home_location();
	void determine_config_file();
	void install_crash_reporting();
	void seed_random_generator();
	void determine_world_map_location();
	void determine_chartlist_filename();
	void set_init_chart_dir();

	void install_signal_handler();
	bool create_opencpn_home();
	bool create_opencpn_log();
	wxString constrain_logfile_size();
	void validate_OpenGL();
	void setup_s57();
	void setup_for_empty_config(bool novicemode);
	void check_tide_current();
	void check_ais_alarm_sound_file();
	void setup_frame_size_and_position(wxPoint& position, wxSize& new_frame_size);
	void setup_gps_watchdog();
	void setup_layers();

	global::OCPN_GUI* gui_instance;
	global::OCPN_Navigation* nav_instance;
	global::OCPN_AIS* ais_instance;
	global::OCPN_WatchDog* wdt_instance;
	global::OCPN_System* sys_instance;
	global::OCPN_Runtime* run_instance;
	global::ColorManager* colors_instance;
	global::ColorProvider* s52_color_provider;
	navigation::RouteTracker* tracker_instance;
	navigation::RouteManager* route_manager_instance;
	navigation::WaypointManager* waypoint_manager_instance;
	tide::TideCurrentManager* tidecurrent_manager_instance;
	gui::StyleManager* style_manager_instance;

	bool start_fullscreen;
	bool first_run;
	wxLog* logger;
	wxLog* old_logger;
	FILE* file_log;

	bool win_console;
};

#endif
