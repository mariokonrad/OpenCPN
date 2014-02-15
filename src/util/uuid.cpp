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

#include "uuid.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace util {

static int random_number(int high, int low)
{
	int d = high - low;
	long u = (long)round(((double)rand() / ((double)(RAND_MAX) + 1) * d) + low);
	return (int)u;
}

// RFC4122 version 4 compliant random UUIDs generator.
std::string uuid()
{
	struct UUID
	{
		int time_low;
		int time_mid;
		int time_hi_and_version;
		int clock_seq_hi_and_rsv;
		int clock_seq_low;
		int node_hi;
		int node_low;
	};

	UUID uuid;

	// FIXME: the max should be set to something like MAXINT32, but it doesn't compile un gcc...
	uuid.time_low = random_number(0, 2147483647);
	uuid.time_mid = random_number(0, 65535);
	uuid.time_hi_and_version = random_number(0, 65535);
	uuid.clock_seq_hi_and_rsv = random_number(0, 255);
	uuid.clock_seq_low = random_number(0, 255);
	uuid.node_hi = random_number(0, 65535);
	uuid.node_low = random_number(0, 2147483647);

	// Set the two most significant bits (bits 6 and 7) of the
	// clock_seq_hi_and_rsv to zero and one, respectively.
	uuid.clock_seq_hi_and_rsv = (uuid.clock_seq_hi_and_rsv & 0x3F) | 0x80;

	// Set the four most significant bits (bits 12 through 15) of the
	// time_hi_and_version field to 4
	uuid.time_hi_and_version = (uuid.time_hi_and_version & 0x0fff) | 0x4000;

	char buf[64];
	snprintf(buf, sizeof(buf), "%08x-%04x-%04x-%02x%02x-%04x%08x", uuid.time_low, uuid.time_mid,
			 uuid.time_hi_and_version, uuid.clock_seq_hi_and_rsv, uuid.clock_seq_low, uuid.node_hi,
			 uuid.node_low);

	return std::string(buf);
}

}

