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

#ifndef __NAVOBJECTCHANGES__H__
#define __NAVOBJECTCHANGES__H__

#include "NavObjectCollection.h"
#include <Route.h>
#include <Track.h>

class NavObjectChanges : public NavObjectCollection
{
public:
	NavObjectChanges();
	virtual ~NavObjectChanges();

	bool AddRoute(Route* pr, const char* action); // support "changes" file set
	bool AddTrack(Track* pr, const char* action);
	bool AddWP(RoutePoint* pr, const char* action);
	bool ApplyChanges(void);
};

#endif
