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

#ifndef __GLOBAL__OCPN__H__
#define __GLOBAL__OCPN__H__

namespace navigation {
class RouteTracker;
class RouteManager;
class WaypointManager;
}

namespace tide {
class TideCurrentManager;
}

namespace global {

class GUI;
class Navigation;
class AIS;
class WatchDog;
class System;
class Runtime;
class ColorManager;

class OCPN
{
public:
	static OCPN& get();
	void clear();

	void inject(GUI*);
	GUI& gui();

	void inject(Navigation*);
	Navigation& nav();

	void inject(AIS*);
	AIS& ais();

	void inject(WatchDog*);
	WatchDog& wdt();

	void inject(System*);
	System& sys();

	void inject(Runtime*);
	Runtime& run();

	void inject(ColorManager*);
	ColorManager& color();

	void inject(navigation::RouteTracker*);
	navigation::RouteTracker& tracker();

	void inject(navigation::RouteManager*);
	navigation::RouteManager& routeman();

	void inject(navigation::WaypointManager*);
	navigation::WaypointManager& waypointman();

	void inject(tide::TideCurrentManager*);
	tide::TideCurrentManager& tidecurrentman();

private:
	OCPN();
	OCPN(const OCPN&);
	~OCPN();
	OCPN& operator=(const OCPN&);

	static OCPN* instance;

	GUI* gui_instance;
	Navigation* nav_instance;
	AIS* ais_instance;
	WatchDog* wdt_instance;
	System* sys_instance;
	Runtime* run_instance;
	ColorManager* color_instance;
	navigation::RouteTracker* tracker_instance;
	navigation::RouteManager* route_manager_instance;
	navigation::WaypointManager* waypoint_manager_instance;
	tide::TideCurrentManager* tidecurrent_manager_instance;
};

}

#endif
