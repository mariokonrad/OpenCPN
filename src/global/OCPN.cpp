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

#include "OCPN.h"
#include <cstdlib>

namespace global {

OCPN* OCPN::instance = NULL;

OCPN::OCPN()
	: gui_instance(NULL)
	, nav_instance(NULL)
	, ais_instance(NULL)
	, wdt_instance(NULL)
	, sys_instance(NULL)
	, run_instance(NULL)
	, color_instance(NULL)
	, tracker_instance(NULL)
	, route_manager_instance(NULL)
	, waypoint_manager_instance(NULL)
	, style_manager_instance(NULL)
	, font_manager_instance(NULL)
{}

OCPN::OCPN(const OCPN&)
{
}

OCPN::~OCPN()
{
}

OCPN& OCPN::operator=(const OCPN&)
{
	return *this;
}

OCPN& OCPN::get()
{
	if (!instance) {
		instance = new OCPN;
	}
	return *instance;
}

void OCPN::clear()
{
	gui_instance = NULL;
	nav_instance = NULL;
	ais_instance = NULL;
	wdt_instance = NULL;
	sys_instance = NULL;
	run_instance = NULL;
	color_instance = NULL;
	tracker_instance = NULL;
	route_manager_instance = NULL;
	waypoint_manager_instance = NULL;
	style_manager_instance = NULL;
	font_manager_instance = NULL;
}

void OCPN::inject(GUI* instance)
{
	gui_instance = instance;
}

GUI& OCPN::gui()
{
	return *gui_instance;
}

void OCPN::inject(Navigation* instance)
{
	nav_instance = instance;
}

Navigation& OCPN::nav()
{
	return *nav_instance;
}

void OCPN::inject(AIS* instance)
{
	ais_instance = instance;
}

AIS& OCPN::ais()
{
	return *ais_instance;
}

void OCPN::inject(WatchDog* instance)
{
	wdt_instance = instance;
}

WatchDog& OCPN::wdt()
{
	return *wdt_instance;
}

void OCPN::inject(System* instance)
{
	sys_instance = instance;
}

System& OCPN::sys()
{
	return *sys_instance;
}

void OCPN::inject(Runtime* instance)
{
	run_instance = instance;
}

Runtime& OCPN::run()
{
	return *run_instance;
}

void OCPN::inject(ColorManager* instance)
{
	color_instance = instance;
}

ColorManager& OCPN::color()
{
	return *color_instance;
}

void OCPN::inject(navigation::RouteTracker* instance)
{
	tracker_instance = instance;
}

navigation::RouteTracker& OCPN::tracker()
{
	return *tracker_instance;
}

void OCPN::inject(navigation::RouteManager* instance)
{
	route_manager_instance = instance;
}

navigation::RouteManager& OCPN::routeman()
{
	return *route_manager_instance;
}

void OCPN::inject(navigation::WaypointManager* instance)
{
	waypoint_manager_instance = instance;
}

navigation::WaypointManager& OCPN::waypointman()
{
	return *waypoint_manager_instance;
}

void OCPN::inject(tide::TideCurrentManager* instance)
{
	tidecurrent_manager_instance = instance;
}

tide::TideCurrentManager& OCPN::tidecurrentman()
{
	return *tidecurrent_manager_instance;
}

void OCPN::inject(gui::StyleManager* instance)
{
	style_manager_instance = instance;
}

gui::StyleManager& OCPN::styleman()
{
	return *style_manager_instance;
}

void OCPN::inject(gui::FontManager* instance)
{
	font_manager_instance = instance;
}

gui::FontManager& OCPN::font()
{
	return *font_manager_instance;
}

}

