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

namespace global {
class OCPN_GUI;
class OCPN_Navigation;
class OCPN_WatchDog;
class OCPN_System;
}

class App : public wxApp
{
		DECLARE_EVENT_TABLE()

	public:
		App();
		bool OnInit();
		int OnExit();
		void OnInitCmdLine(wxCmdLineParser & parser);
		bool OnCmdLineParsed(wxCmdLineParser & parser);
		void OnActivateApp(wxActivateEvent & event);
		void TrackOff(void);

		wxSingleInstanceChecker * m_checker;

	private:
		void establish_home_location();

		global::OCPN_GUI * gui_instance;
		global::OCPN_Navigation * nav_instance;
		global::OCPN_WatchDog * wdt_instance;
		global::OCPN_System * sys_instance;

		bool start_fullscreen;
		wxString plugin_dir;
};

#endif
