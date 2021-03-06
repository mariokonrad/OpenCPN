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

#include "MagneticVariation.h"

#include <global/OCPN.h>
#include <global/Navigation.h>
#include <global/GUI.h>

#include <wx/math.h> // FIXME: VS2010 does not know isnan (do not like the _isnan hack), but dependencies to wx seems overkill

namespace navigation
{

double GetTrueOrMag(double a)
{
	if (global::OCPN::get().gui().view().ShowMag) {
		const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
		if (!wxIsNaN(nav.var)) {
			if ((a - nav.var) > 360.0)
				return (a - nav.var - 360.0);
			else
				return ((a - nav.var) >= 0.0) ? (a - nav.var) : (a - nav.var + 360.0);
		} else {
			if ((a - nav.user_var) > 360.0)
				return (a - nav.user_var - 360.0);
			else
				return ((a - nav.user_var) >= 0.0) ? (a - nav.user_var) : (a - nav.user_var + 360.0);
		}
	}
	return a;
}

}

