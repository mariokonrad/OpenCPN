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

#include "ParseGPXDateTime.h"

#include <wx/datetime.h>

namespace gpx {

// This function parses a string containing a GPX time representation
// and returns a wxDateTime containing the UTC corresponding to the
// input. The function return value is a pointer past the last valid
// character parsed (if successful) or NULL (if the string is invalid).
//
// Valid GPX time strings are in ISO 8601 format as follows:
//
//   [-]<YYYY>-<MM>-<DD>T<hh>:<mm>:<ss>Z|(+|-<hh>:<mm>)
//
// For example, 2010-10-30T14:34:56Z and 2010-10-30T14:34:56-04:00
// are the same time. The first is UTC and the second is EDT.
const wxChar* ParseGPXDateTime(wxDateTime& dt, const wxChar* datetime)
{
	long sign;
	long hrs_west;
	long mins_west;
	const wxChar* end;

	// Skip any leading whitespace
	while (isspace(*datetime))
		datetime++;

	// Skip (and ignore) leading hyphen
	if (*datetime == wxT('-'))
		datetime++;

	// Parse and validate ISO 8601 date/time string
	if ((end = dt.ParseFormat(datetime, wxT("%Y-%m-%dT%T"))) != NULL) {

		// Invalid date/time
		if (*end == 0)
			return NULL;

		// ParseFormat outputs in UTC if the controlling
		// wxDateTime class instance has not been initialized.

		// Date/time followed by UTC time zone flag, so we are done
		else if (*end == wxT('Z')) {
			end++;
			return end;
		}

		// Date/time followed by given number of hrs/mins west of UTC
		else if (*end == wxT('+') || *end == wxT('-')) {

			// Save direction from UTC
			if (*end == wxT('+'))
				sign = 1;
			else
				sign = -1;
			end++;

			// Parse hrs west of UTC
			if (isdigit(*end) && isdigit(*(end + 1)) && *(end + 2) == wxT(':')) {

				// Extract and validate hrs west of UTC
				wxString(end).ToLong(&hrs_west);
				if (hrs_west > 12)
					return NULL;
				end += 3;

				// Parse mins west of UTC
				if (isdigit(*end) && isdigit(*(end + 1))) {

					// Extract and validate mins west of UTC
					wxChar mins[3];
					mins[0] = *end;
					mins[1] = *(end + 1);
					mins[2] = 0;
					wxString(mins).ToLong(&mins_west);
					if (mins_west > 59)
						return NULL;

					// Apply correction
					dt -= sign * wxTimeSpan(hrs_west, mins_west, 0, 0);
					return end + 2;
				} else
					// Missing mins digits
					return NULL;
			} else
				// Missing hrs digits or colon
				return NULL;
		} else
			// Unknown field after date/time (not UTC, not hrs/mins
			//  west of UTC)
			return NULL;
	} else
		// Invalid ISO 8601 date/time
		return NULL;
}

}

