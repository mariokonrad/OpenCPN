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

#include "PositionConvert.h"
#include <wx/math.h>

#include <global/OCPN.h>
#include <global/System.h>


wxString PositionConvert::lat(double a, bool high_precision)
{
	return toSDMM(1, a, high_precision);
}

wxString PositionConvert::lon(double a, bool high_precision)
{
	return toSDMM(2, a, high_precision);
}

geo::Position PositionConvert::pos(const wxString& lat_str, const wxString& lon_str)
{
	return geo::Position(lat(lat_str), lon(lon_str));
}

double PositionConvert::lat(const wxString& s)
{
	// TODO: implement better parser
	return fromDMM(s);
}

double PositionConvert::lon(const wxString& s)
{
	// TODO: implement better parser
	return fromDMM(s);
}

/// Formats the coordinates to string.
///
/// @param[in] NEflag 1:latitude (N/S), 2:longitude (E/W)
/// @param[in] a Coordinate (latitude or longitude, depending on NEflag)
/// @param[in] hi_precision If set to not 0, the position is printed more detailed.
/// @return The position as string.
wxString PositionConvert::toSDMM(int NEflag, double a, bool hi_precision) // FIXME: this interface is silly
{
	wxString s;
	double mpy;
	short neg = 0;
	int d;
	long m;
	double ang = a;
	char c = 'N';

	if (a < 0.0) {
		a = -a;
		neg = 1;
	}
	d = (int)a;
	if (neg)
		d = -d;
	if (NEflag) {
		if (NEflag == 1) {
			c = 'N';

			if (neg) {
				d = -d;
				c = 'S';
			}
		} else if (NEflag == 2) {
			c = 'E';

			if (neg) {
				d = -d;
				c = 'W';
			}
		}
	}

	switch (global::OCPN::get().sys().config().SDMMFormat) {
		case 0:
			mpy = 600.0;
			if (hi_precision)
				mpy = mpy * 1000;

			m = (long)wxRound((a - (double)d) * mpy);

			if (!NEflag || NEflag < 1 || NEflag > 2) // Does it EVER happen?
			{
				if (hi_precision)
					s.Printf(_T("%d %02ld.%04ld'"), d, m / 10000, m % 10000);
				else
					s.Printf(_T("%d %02ld.%01ld'"), d, m / 10, m % 10);
			} else {
				if (hi_precision)
					if (NEflag == 1)
						s.Printf(_T("%02d %02ld.%04ld %c"), d, m / 10000, (m % 10000), c);
					else
						s.Printf(_T("%03d %02ld.%04ld %c"), d, m / 10000, (m % 10000), c);
				else if (NEflag == 1)
					s.Printf(_T ( "%02d %02ld.%01ld %c" ), d, m / 10, (m % 10), c);
				else
					s.Printf(_T ( "%03d %02ld.%01ld %c" ), d, m / 10, (m % 10), c);
			}
			break;

		case 1:
			if (hi_precision)
				s.Printf(_T("%03.6f"), ang); // cca 11 cm - the GPX precision is higher, but as we
											 // use hi_precision almost everywhere it would be a
											 // little too much....
			else
				s.Printf(_T("%03.4f"), ang); // cca 11m
			break;

		case 2:
			m = (long)((a - (double)d) * 60);
			mpy = 10.0;
			if (hi_precision)
				mpy = mpy * 100;
			long sec = (long)((a - (double)d - (((double)m) / 60)) * 3600 * mpy);

			if (!NEflag || NEflag < 1 || NEflag > 2) // Does it EVER happen?
			{
				if (hi_precision)
					s.Printf(_T ( "%d %ld'%ld.%ld\"" ), d, m, sec / 1000, sec % 1000);
				else
					s.Printf(_T ( "%d %ld'%ld.%ld\"" ), d, m, sec / 10, sec % 10);
			} else {
				if (hi_precision)
					if (NEflag == 1)
						s.Printf(_T ( "%02d %02ld %02ld.%03ld %c" ), d, m, sec / 1000, sec % 1000,
								 c);
					else
						s.Printf(_T ( "%03d %02ld %02ld.%03ld %c" ), d, m, sec / 1000, sec % 1000,
								 c);
				else if (NEflag == 1)
					s.Printf(_T ( "%02d %02ld %02ld.%ld %c" ), d, m, sec / 10, sec % 10, c);
				else
					s.Printf(_T ( "%03d %02ld %02ld.%ld %c" ), d, m, sec / 10, sec % 10, c);
			}
			break;
	}
	return s;
}

// Modified from the code posted by Andy Ross at
//     http://www.mail-archive.com/flightgear-devel@flightgear.org/msg06702.html
// Basically, it looks for a list of decimal numbers embedded in the
// string and uses the first three as degree, minutes and seconds.  The
// presence of a "S" or "W character indicates that the result is in a
// hemisphere where the final answer must be negated.  Non-number
// characters are treated as whitespace separating numbers.
//
// So there are lots of bogus strings you can feed it to get a bogus
// answer, but that's not surprising.  It does, however, correctly parse
// all the well-formed strings I can thing of to feed it.  I've tried all
// the following:
//
// 37°54.204' N
// N37 54 12
// 37°54'12"
// 37.9034
// 122°18.621' W
// 122w 18 37
// -122.31035
double PositionConvert::fromDMM(const wxString& position)
{
	wchar_t buf[64];
	char narrowbuf[64];
	double sign = 1.0;

	wxString sdms = position;

	// First round of string modifications to accomodate some known strange formats
	wxString replhelper;
	replhelper = wxString::FromUTF8("´·"); // UKHO PDFs
	sdms.Replace(replhelper, _T("."));
	replhelper = wxString::FromUTF8("\"·"); // Don't know if used, but to make sure
	sdms.Replace(replhelper, _T("."));
	replhelper = wxString::FromUTF8("·");
	sdms.Replace(replhelper, _T("."));

	// Another example: cs.wikipedia.org (someone was too active translating...)
	replhelper = wxString::FromUTF8("s. š.");
	sdms.Replace(replhelper, _T("N"));
	replhelper = wxString::FromUTF8("j. š.");
	sdms.Replace(replhelper, _T("S"));
	sdms.Replace(_T("v. d."), _T("E"));
	sdms.Replace(_T("z. d."), _T("W"));

	// If the string contains hemisphere specified by a letter, then '-' is for sure a separator...
	sdms.UpperCase();
	if (sdms.Contains(_T("N")) || sdms.Contains(_T("S")) || sdms.Contains(_T("E"))
		|| sdms.Contains(_T("W")))
		sdms.Replace(_T("-"), _T(" "));

	wcsncpy(buf, sdms.wc_str(wxConvUTF8), 64);
	int len = wcslen(buf);

	for (int i = 0; i < len; i++) {
		wchar_t c = buf[i];
		if ((c >= '0' && c <= '9') || c == '-' || c == '.' || c == '+') {
			narrowbuf[i] = c;
			continue; // Digit characters are cool as is
		}
		if (c == ',') {
			narrowbuf[i] = '.'; // convert to decimal dot
			continue;
		}
		if ((c | 32) == 'w' || (c | 32) == 's')
			sign = -1.0; // These mean "negate" (note case insensitivity)
		narrowbuf[i] = 0; // Replace everything else with nuls
	}

	// Build a stack of doubles
	unsigned int top = 0;
	double stk[3] = { 0.0, 0.0, 0.0 };
	for (int i = 0; (i < len) && (top < sizeof(stk) / sizeof(stk[0])); ++i) {
		while (i < len && narrowbuf[i] == 0)
			i++;
		if (i != len) {
			stk[top++] = atof(narrowbuf + i);
			i += strlen(narrowbuf + i);
		}
	}

	return sign * (stk[0] + (stk[1] + stk[2] / 60) / 60);
}

