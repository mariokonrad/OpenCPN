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

#ifndef __GARMIN__CPO_SATELLITE_DATA__H__
#define __GARMIN__CPO_SATELLITE_DATA__H__

namespace garmin {

// Packet structure for Pkt_ID = 114 (Satellite Data Record)
// The status bit field represents a set of booleans described below:
//   Bit Meaning when bit is one (1)
//   0   The unit has ephemeris data for the specified satellite.
//   1   The unit has a differential correction for the specified satellite.
//   2   The unit is using this satellite in the solution.
struct cpo_sat_data
{
	unsigned char svid;   // space vehicle identification (1-32 and 33-64 for WAAS)
	short         snr;    // signal-to-noise ratio
	unsigned char elev;   // satellite elevation in degrees
	short         azmth;  // satellite azimuth in degrees
	unsigned char status; // status bit-field
};

}

#endif
