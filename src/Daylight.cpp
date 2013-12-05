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

#include "Daylight.h"
#include <wx/intl.h>
#include <MicrosoftCompatibility.h>
#include <cmath>

// Sunrise/twilight calculation for route properties.
// limitations: latitude below 60, year between 2000 and 2100
// riset is +1 for rise -1 for set
// adapted by author's permission from QBASIC source as published at
//     http://www.stargazing.net/kepler
wxString GetDaylightString(Daylight status)
{
	switch (status) {
		case UNKNOWN_DAYLIGHT:
			return _T(" - ");
		case MOTWILIGHT:
			return _("MoTwilight");
		case SUNRISE:
			return _("Sunrise");
		case DAY:
			return _("Daytime");
		case SUNSET:
			return _("Sunset");
		case EVTWILIGHT:
			return _("EvTwilight");
		case NIGHT:
			return _("Nighttime");
		default:
			return _T("");
	}
}

static double sign(double x)
{
	if (x < 0.0)
		return -1.0;
	else
		return 1.0;
}

static double FNipart(double x)
{
	return sign(x) * (int)(fabs(x));
}

static double FNday(int y, int m, int d, int h)
{
	long fd = (367 * y - 7 * (y + (m + 9) / 12) / 4 + 275 * m / 9 + d);
	return ((double)fd - 730531.5 + h / 24.0);
}

static double FNrange(double x)
{
	double b = x / (2.0 * M_PI);
	double a = (2.0 * M_PI)* (b - FNipart(b));
	if (a < 0.0)
		a = (2.0 * M_PI) + a;
	return a;
}

static double getDaylightEvent(const Position& pos, int riset, double altitude, int y, int m, int d)
{
	double day = FNday(y, m, d, 0);
	double days, correction;
	double utold = M_PI;
	double utnew = 0.0;
	double sinalt = sin(altitude * (M_PI / 180.0)); // go for the sunrise/sunset altitude first
	double sinphi = sin(pos.lat() * (M_PI / 180.0));
	double cosphi = cos(pos.lon() * (M_PI / 180.0));
	double g = pos.lon() * (M_PI / 180.0);
	double t;
	double L;
	double G;
	double ec;
	double lambda;
	double E;
	double obl;
	double delta;
	double GHA;
	double cosc;
	int limit = 12;
	while ((fabs(utold - utnew) > 0.001)) {
		if (limit-- <= 0)
			return (-1.0);
		days = day + utnew / (2.0 * M_PI);
		t = days / 36525.0;
		// get arguments of Sun's orbit
		L = FNrange(4.8949504201433 + 628.331969753199 * t);
		G = FNrange(6.2400408 + 628.3019501 * t);
		ec = 0.033423 * sin(G) + 0.00034907 * sin(2 * G);
		lambda = L + ec;
		E = -1.0 * ec + 0.0430398 * sin(2 * lambda) - 0.00092502 * sin(4.0 * lambda);
		obl = 0.409093 - 0.0002269 * t;
		delta = asin(sin(obl) * sin(lambda));
		GHA = utold - M_PI + E;
		cosc = (sinalt - sinphi * sin(delta)) / (cosphi * cos(delta));
		if (cosc > 1.0)
			correction = 0.0;
		else if (cosc < -1.0)
			correction = M_PI;
		else
			correction = acos(cosc);
		double tmp = utnew;
		utnew = FNrange(utold - (GHA + g + riset * correction));
		utold = tmp;
	}
	return utnew * (180.0 / M_PI) / 15.0; // returns decimal hours UTC
}

static double getLMT(double ut, const Position& pos)
{
	double t = ut + pos.lon() / 15.0;
	if (t >= 0.0)
		if (t <= 24.0)
			return (t);
		else
			return (t - 24.0);
	else
		return (t + 24.0);
}

Daylight getDaylightStatus(const Position& pos, wxDateTime utcDateTime)
{
	if (fabs(pos.lat()) > 60.0)
		return UNKNOWN_DAYLIGHT;
	int y = utcDateTime.GetYear();
	int m = utcDateTime.GetMonth() + 1; // wxBug? months seem to run 0..11 ?
	int d = utcDateTime.GetDay();
	int h = utcDateTime.GetHour();
	int n = utcDateTime.GetMinute();
	int s = utcDateTime.GetSecond();
	if (y < 2000 || y > 2100)
		return UNKNOWN_DAYLIGHT;

	double ut = (double)h + (double)n / 60.0 + (double)s / 3600.0;
	double lt = getLMT(ut, pos);
	double rsalt = -0.833;
	double twalt = -12.0;

	// wxString msg;

	if (lt <= 12.0) {
		double sunrise = getDaylightEvent(pos, +1, rsalt, y, m, d);
		if (sunrise < 0.0)
			return UNKNOWN_DAYLIGHT;
		else
			sunrise = getLMT(sunrise, pos);

		if (fabs(lt - sunrise) < 0.15)
			return SUNRISE;
		if (lt > sunrise)
			return DAY;
		double twilight = getDaylightEvent(pos, +1, twalt, y, m, d);
		if (twilight < 0.0)
			return UNKNOWN_DAYLIGHT;
		else
			twilight = getLMT(twilight, pos);
		if (lt > twilight)
			return MOTWILIGHT;
		else
			return NIGHT;
	} else {
		double sunset = getDaylightEvent(pos, -1, rsalt, y, m, d);
		if (sunset < 0.0)
			return UNKNOWN_DAYLIGHT;
		else
			sunset = getLMT(sunset, pos);
		if (fabs(lt - sunset) < 0.15)
			return SUNSET;
		if (lt < sunset)
			return DAY;
		double twilight = getDaylightEvent(pos, -1, twalt, y, m, d);
		if (twilight < 0.0)
			return UNKNOWN_DAYLIGHT;
		else
			twilight = getLMT(twilight, pos);
		if (lt < twilight)
			return EVTWILIGHT;
		else
			return NIGHT;
	}
}

