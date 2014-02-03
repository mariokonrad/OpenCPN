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

#include "AnchorDist.h"

namespace navigation {

/// A helper function to check for proper parameters of anchor watch.
///
/// @param[in] d Distacnce in meter to check against boundaries.
/// @param[in] AnchorPointMinDist Minimal distance in meter, lower bound of the range.
/// @param[in] AnchorPointMaxDist Maximal distance in meter, upper boudn of the range.
/// @return If distance below minimum, the minimum will return. If distance is above
///   the maximum, the maximum will return. In all other cases the distance itself
///   is valid and will be returned.
double AnchorDistFix(
		const double d,
		const double AnchorPointMinDist,
		const double AnchorPointMaxDist)
{
	if (d >= 0.0)
		if (d < AnchorPointMinDist)
			return AnchorPointMinDist;
		else if (d > AnchorPointMaxDist)
			return AnchorPointMaxDist;
		else
			return d;
	else if (d > -AnchorPointMinDist)
		return -AnchorPointMinDist;
	else if (d < -AnchorPointMaxDist)
		return -AnchorPointMaxDist;
	else
		return d;
}

}

