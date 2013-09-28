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

#include "OCPN_WatchDog.h"

namespace global {

const WatchDog::Data & OCPN_WatchDog::get_data() const
{
	return data;
}

void OCPN_WatchDog::set_gps_timeout_ticks(int ticks)
{
	data.gps_watchdog_timeout_ticks = ticks;
}

void OCPN_WatchDog::set_sat_timeout_ticks(int ticks)
{
	data.sat_watchdog_timeout_ticks = ticks;
}

void OCPN_WatchDog::set_gps_watchdog(int value)
{
	data.gps_watchdog = value;
}

void OCPN_WatchDog::decrement_gps_watchdog()
{
	data.gps_watchdog--;
}

void OCPN_WatchDog::set_hdx_watchdog(int value)
{
	data.hdx_watchdog = value;
}

void OCPN_WatchDog::decrement_hdx_watchdog()
{
	data.hdx_watchdog--;
}

void OCPN_WatchDog::set_hdt_watchdog(int value)
{
	data.hdt_watchdog = value;
}

void OCPN_WatchDog::decrement_hdt_watchdog()
{
	data.hdt_watchdog--;
}

void OCPN_WatchDog::set_var_watchdog(int value)
{
	data.var_watchdog = value;
}

void OCPN_WatchDog::decrement_var_watchdog()
{
	data.var_watchdog--;
}

void OCPN_WatchDog::set_sat_watchdog(int value)
{
	data.sat_watchdog = value;
}

void OCPN_WatchDog::decrement_sat_watchdog()
{
	data.sat_watchdog--;
}

}

