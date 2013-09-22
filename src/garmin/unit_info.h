/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 * Copyright (C) 2010 by David S. Register                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the                           *
 * Free Software Foundation, Inc.,                                         *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.           *
 **************************************************************************/

#ifndef __GARMIN__UNIT_INFO__H__
#define __GARMIN__UNIT_INFO__H__

typedef struct garmin_unit_info // FIXME: holy fucking repetition batman, this is defined in datastream, nmea and garmin/jeeps/gpsusb
{
	unsigned long serial_number;
	unsigned long unit_id;
	unsigned long unit_version;
	char * os_identifier; // In case the OS has another name for it.
	char * product_identifier; // From the hardware itself.
} unit_info_type;

#endif
