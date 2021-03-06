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

#ifndef __GLOBAL__RUNTIME__H__
#define __GLOBAL__RUNTIME__H__

#include <wx/datetime.h>

namespace global {

class Runtime
{
public:
	virtual ~Runtime()
	{
	}

public:
	struct Data
	{
		wxDateTime app_start_time;
		wxDateTime loglast_time;
	};

	virtual const Data& data() const = 0;
	virtual void set_app_start_time(const wxDateTime&) = 0;
	virtual void set_loglast_time(const wxDateTime&) = 0;
};

}

#endif
