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

#ifndef __GLOBAL__WATCHDOG__H__
#define __GLOBAL__WATCHDOG__H__

namespace global {

class WatchDog
{
	public:
		virtual ~WatchDog() {}

	public:

		struct Data
		{
			int gps_watchdog_timeout_ticks;
			int sat_watchdog_timeout_ticks;
		};

		virtual const Data & get_data() const = 0;
		virtual void set_gps_timeout_ticks(int) = 0;
		virtual void set_sat_timeout_ticks(int) = 0;
};

}

#endif
