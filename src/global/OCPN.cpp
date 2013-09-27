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

OCPN * OCPN::instance = NULL;

OCPN::OCPN()
	: gui_instance(NULL)
	, nav_instance(NULL)
	, wdt_instance(NULL)
	, sys_instance(NULL)
{}

OCPN::OCPN(const OCPN &)
{}

OCPN::~OCPN()
{}

OCPN & OCPN::operator=(const OCPN &)
{
	return *this;
}

OCPN & OCPN::get()
{
	if (!instance) {
		instance = new OCPN;
	}
	return *instance;
}

void OCPN::inject(GUI * gui)
{
	gui_instance = gui;
}

GUI & OCPN::gui()
{
	return *gui_instance;
}

void OCPN::inject(Navigation * nav)
{
	nav_instance = nav;
}

Navigation & OCPN::nav()
{
	return *nav_instance;
}

void OCPN::inject(WatchDog * wdt)
{
	wdt_instance = wdt;
}

WatchDog & OCPN::wdt()
{
	return *wdt_instance;
}

void OCPN::inject(System * sys)
{
	sys_instance = sys;
}

System & OCPN::sys()
{
	return *sys_instance;
}

}

