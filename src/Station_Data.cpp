/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#include "Station_Data.h"
#include <stdlib.h>

Station_Data::Station_Data()
	: station_name(NULL)
	, amplitude(NULL)
	, epoch(NULL)
{}

Station_Data::~Station_Data()
{
	if (station_name) {
		::free(station_name);
		station_name = NULL;
	}
	if (amplitude) {
		::free(amplitude);
		amplitude = NULL;
	}
	if (epoch) {
		::free(epoch);
		epoch = NULL;
	}
}

