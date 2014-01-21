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
 ***************************************************************************

 ***************************************************************************
 *  Parts of this file were adapted from source code found in              *
 *  John F. Waers (jfwaers@csn.net) public domain program MacGPS45         *
 **************************************************************************/

#include "GeoRef.h"
#include <geo/lmfit.h>
#include <MicrosoftCompatibility.h>

#include <vector>
#include <utility>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctype.h>

namespace geo {


//  ellipsoid: index into the gEllipsoid[] array, in which
//             a is the ellipsoid semimajor axis
//             invf is the inverse of the ellipsoid flattening f
//  dx, dy, dz: ellipsoid center with respect to WGS84 ellipsoid center
//    x axis is the prime meridian
//    y axis is 90 degrees east longitude
//    z axis is the axis of rotation of the ellipsoid

// The following values for dx, dy and dz were extracted from the output of
// the GARMIN PCX5 program. The output also includes values for da and df, the
// difference between the reference ellipsoid and the WGS84 ellipsoid semi-
// major axis and flattening, respectively. These are replaced by the
// data contained in the structure array gEllipsoid[], which was obtained from
// the Defence Mapping Agency document number TR8350.2, "Department of Defense
// World Geodetic System 1984."

struct DATUM
{
	const char * name;
	short ellipsoid;
	double dx;
	double dy;
	double dz;
};

struct ELLIPSOID
{
	const char * name; // name of ellipsoid
	double a; // semi-major axis, meters
	double invf; // 1/f
};

static struct DATUM const gDatum[] = {
//     name               ellipsoid   dx      dy       dz
      { "Adindan",                5,   -162,    -12,    206 },    // 0
      { "Afgooye",               15,    -43,   -163,     45 },    // 1
      { "Ain el Abd 1970",       14,   -150,   -251,     -2 },    // 2
      { "Anna 1 Astro 1965",      2,   -491,    -22,    435 },    // 3
      { "Arc 1950",               5,   -143,    -90,   -294 },    // 4
      { "Arc 1960",               5,   -160,     -8,   -300 },    // 5
      { "Ascension Island  58",  14,   -207,    107,     52 },    // 6
      { "Astro B4 Sorol Atoll",  14,    114,   -116,   -333 },    // 7
      { "Astro Beacon  E ",      14,    145,     75,   -272 },    // 8
      { "Astro DOS 71/4",        14,   -320,    550,   -494 },    // 9
      { "Astronomic Stn  52",    14,    124,   -234,    -25 },    // 10
      { "Australian Geod  66",    2,   -133,    -48,    148 },    // 11
      { "Australian Geod  84",    2,   -134,    -48,    149 },    // 12
      { "Bellevue (IGN)",        14,   -127,   -769,    472 },    // 13
      { "Bermuda 1957",           4,    -73,    213,    296 },    // 14
      { "Bogota Observatory",    14,    307,    304,   -318 },    // 15
      { "Campo Inchauspe",       14,   -148,    136,     90 },    // 16
      { "Canton Astro 1966",     14,    298,   -304,   -375 },    // 17
      { "Cape",                   5,   -136,   -108,   -292 },    // 18
      { "Cape Canaveral",         4,     -2,    150,    181 },    // 19
      { "Carthage",               5,   -263,      6,    431 },    // 20
      { "CH-1903",                3,    674,     15,    405 },    // 21
      { "Chatham 1971",          14,    175,    -38,    113 },    // 22
      { "Chua Astro",            14,   -134,    229,    -29 },    // 23
      { "Corrego Alegre",        14,   -206,    172,     -6 },    // 24
      { "Djakarta (Batavia)",     3,   -377,    681,    -50 },    // 25
      { "DOS 1968",              14,    230,   -199,   -752 },    // 26
      { "Easter Island 1967",    14,    211,    147,    111 },    // 27
      { "European 1950",         14,    -87,    -98,   -121 },    // 28
      { "European 1979",         14,    -86,    -98,   -119 },    // 29
      { "Finland Hayford",       14,    -78,   -231,    -97 },    // 30
      { "Gandajika Base",        14,   -133,   -321,     50 },    // 31
      { "Geodetic Datum  49",    14,     84,    -22,    209 },    // 32
      { "Guam 1963",              4,   -100,   -248,    259 },    // 33
      { "GUX 1 Astro",           14,    252,   -209,   -751 },    // 34
      { "Hermannskogel Datum",    3,    682,   -203,    480 },    // 35
      { "Hjorsey 1955",          14,    -73,     46,    -86 },    // 36
      { "Hong Kong 1963",        14,   -156,   -271,   -189 },    // 37
      { "Indian Bangladesh",      6,    289,    734,    257 },    // 38
      { "Indian Thailand",        6,    214,    836,    303 },    // 39
      { "Ireland 1965",           1,    506,   -122,    611 },    // 40
      { "ISTS 073 Astro  69",    14,    208,   -435,   -229 },    // 41
      { "Johnston Island",       14,    191,    -77,   -204 },    // 42
      { "Kandawala",              6,    -97,    787,     86 },    // 43
      { "Kerguelen Island",      14,    145,   -187,    103 },    // 44
      { "Kertau 1948",            7,    -11,    851,      5 },    // 45
      { "L.C. 5 Astro",           4,     42,    124,    147 },    // 46
      { "Liberia 1964",           5,    -90,     40,     88 },    // 47
      { "Luzon Mindanao",         4,   -133,    -79,    -72 },    // 48
      { "Luzon Philippines",      4,   -133,    -77,    -51 },    // 49
      { "Mahe 1971",              5,     41,   -220,   -134 },    // 50
      { "Marco Astro",           14,   -289,   -124,     60 },    // 51
      { "Massawa",                3,    639,    405,     60 },    // 52
      { "Merchich",               5,     31,    146,     47 },    // 53
      { "Midway Astro 1961",     14,    912,    -58,   1227 },    // 54
      { "Minna",                  5,    -92,    -93,    122 },    // 55
      { "NAD27 Alaska",           4,     -5,    135,    172 },    // 56
      { "NAD27 Bahamas",          4,     -4,    154,    178 },    // 57
      { "NAD27 Canada",           4,    -10,    158,    187 },    // 58
      { "NAD27 Canal Zone",       4,      0,    125,    201 },    // 59
      { "NAD27 Caribbean",        4,     -7,    152,    178 },    // 60
      { "NAD27 Central",          4,      0,    125,    194 },    // 61
      { "NAD27 CONUS",            4,     -8,    160,    176 },    // 62
      { "NAD27 Cuba",             4,     -9,    152,    178 },    // 63
      { "NAD27 Greenland",        4,     11,    114,    195 },    // 64
      { "NAD27 Mexico",           4,    -12,    130,    190 },    // 65
      { "NAD27 San Salvador",     4,      1,    140,    165 },    // 66
      { "NAD83",                 11,      0,      0,      0 },    // 67
      { "Nahrwn Masirah Ilnd",    5,   -247,   -148,    369 },    // 68
      { "Nahrwn Saudi Arbia",     5,   -231,   -196,    482 },    // 69
      { "Nahrwn United Arab",     5,   -249,   -156,    381 },    // 70
      { "Naparima BWI",          14,     -2,    374,    172 },    // 71
      { "Observatorio 1966",     14,   -425,   -169,     81 },    // 72
      { "Old Egyptian",          12,   -130,    110,    -13 },    // 73
      { "Old Hawaiian",           4,     61,   -285,   -181 },    // 74
      { "Oman",                   5,   -346,     -1,    224 },    // 75
      { "Ord Srvy Grt Britn",     0,    375,   -111,    431 },    // 76
      { "Pico De Las Nieves",    14,   -307,    -92,    127 },    // 77
      { "Pitcairn Astro 1967",   14,    185,    165,     42 },    // 78
      { "Prov So Amrican  56",   14,   -288,    175,   -376 },    // 79
      { "Prov So Chilean  63",   14,     16,    196,     93 },    // 80
      { "Puerto Rico",            4,     11,     72,   -101 },    // 81
      { "Qatar National",        14,   -128,   -283,     22 },    // 82
      { "Qornoq",                14,    164,    138,   -189 },    // 83
      { "Reunion",               14,     94,   -948,  -1262 },    // 84
      { "Rome 1940",             14,   -225,    -65,      9 },    // 85
      { "RT 90",                  3,    498,    -36,    568 },    // 86
      { "Santo (DOS)",           14,    170,     42,     84 },    // 87
      { "Sao Braz",              14,   -203,    141,     53 },    // 88
      { "Sapper Hill 1943",      14,   -355,     16,     74 },    // 89
      { "Schwarzeck",            21,    616,     97,   -251 },    // 90
      { "South American  69",    16,    -57,      1,    -41 },    // 91
      { "South Asia",             8,      7,    -10,    -26 },    // 92
      { "Southeast Base",        14,   -499,   -249,    314 },    // 93
      { "Southwest Base",        14,   -104,    167,    -38 },    // 94
      { "Timbalai 1948",          6,   -689,    691,    -46 },    // 95
      { "Tokyo",                  3,   -128,    481,    664 },    // 96
      { "Tristan Astro 1968",    14,   -632,    438,   -609 },    // 97
      { "Viti Levu 1916",         5,     51,    391,    -36 },    // 98
      { "Wake-Eniwetok  60",     13,    101,     52,    -39 },    // 99
      { "WGS 72",                19,      0,      0,      5 },    // 100
      { "WGS 84",                20,      0,      0,      0 },    // 101
      { "Zanderij",              14,   -265,    120,   -358 }     // 102
};

static struct ELLIPSOID const gEllipsoid[] = {
//      name                               a        1/f
      {  "Airy 1830",                  6377563.396, 299.3249646   },    // 0
      {  "Modified Airy",              6377340.189, 299.3249646   },    // 1
      {  "Australian National",        6378160.0,   298.25        },    // 2
      {  "Bessel 1841",                6377397.155, 299.1528128   },    // 3
      {  "Clarke 1866",                6378206.4,   294.9786982   },    // 4
      {  "Clarke 1880",                6378249.145, 293.465       },    // 5
      {  "Everest (India 1830)",       6377276.345, 300.8017      },    // 6
      {  "Everest (1948)",             6377304.063, 300.8017      },    // 7
      {  "Modified Fischer 1960",      6378155.0,   298.3         },    // 8
      {  "Everest (Pakistan)",         6377309.613, 300.8017      },    // 9
      {  "Indonesian 1974",            6378160.0,   298.247       },    // 10
      {  "GRS 80",                     6378137.0,   298.257222101 },    // 11
      {  "Helmert 1906",               6378200.0,   298.3         },    // 12
      {  "Hough 1960",                 6378270.0,   297.0         },    // 13
      {  "International 1924",         6378388.0,   297.0         },    // 14
      {  "Krassovsky 1940",            6378245.0,   298.3         },    // 15
      {  "South American 1969",        6378160.0,   298.25        },    // 16
      {  "Everest (Malaysia 1969)",    6377295.664, 300.8017      },    // 17
      {  "Everest (Sabah Sarawak)",    6377298.556, 300.8017      },    // 18
      {  "WGS 72",                     6378135.0,   298.26        },    // 19
      {  "WGS 84",                     6378137.0,   298.257223563 },    // 20
      {  "Bessel 1841 (Namibia)",      6377483.865, 299.1528128   },    // 21
      {  "Everest (India 1956)",       6377301.243, 300.8017      }     // 22
};

static const short nDatums = sizeof(gDatum) / sizeof(gDatum[0]);

static const double WGSinvf = 298.257223563; // WGS84 1/f

static int datumNameCmp(const char* n1, const char* n2)
{
	while (*n1 || *n2) {
		if (*n1 == ' ')
			n1++;
		else if (*n2 == ' ')
			n2++;
		else if (toupper(*n1) == toupper(*n2))
			n1++, n2++;
		else
			return 1; // No string match
	}
	return 0; // String match
}

int GetDatumIndex(const char* str)
{
	int i = 0;
	while (i < (int)nDatums) {
		if (!datumNameCmp(str, gDatum[i].name))
			return i;
		i++;
	}

	return -1;
}

// Convert degrees to dd mm'ss.s" (DMS-Format)
static void toDMS(double a, char* bufp, int bufplen)
{
	bool neg = a < 0.0;
	a = fabs(a);
	int n = (int)((a - (int)a) * 36000.0);
	int m = n / 600;
	int s = n % 600;
	snprintf(bufp, bufplen, "%d%02d'%02d.%01d\"", (int)(neg ? -a : a), m, s / 10, s % 10);
}

// Convert dd mm'ss.s" (DMS-Format) to degrees.
double fromDMS(char* dms)
{
	int d = 0;
	int m = 0;
	double s = 0.0;
	char buf[20] = { '\0' };

	sscanf(dms, "%d%[ ]%d%[ ']%lf%[ \"NSWEnswe]", &d, buf, &m, buf, &s, buf);

	s = (double)(abs(d)) + ((double)m + s / 60.0) / 60.0;

	if (d >= 0 && strpbrk(buf, "SWsw") == NULL)
		return s;

	return -s;
}

// Convert degrees to dd mm.mmm' (DMM-Format)
static void todmm(int flag, double a, char* bufp, int bufplen)
{
	bool bNeg = a < 0.0;
	a = fabs(a);

	int m = (int)((a - (int)a) * 60000.0);

	if (!flag)
		snprintf(bufp, bufplen, "%d %02d.%03d'", (int)a, m / 1000, m % 1000);
	else {
		if (flag == 1) {
			snprintf(bufp, bufplen, "%02d %02d.%03d %c", (int)a, m / 1000, (m % 1000),
					 bNeg ? 'S' : 'N');
		} else if (flag == 2) {
			snprintf(bufp, bufplen, "%03d %02d.%03d %c", (int)a, m / 1000, (m % 1000),
					 bNeg ? 'W' : 'E');
		}
	}
}

static void toDMM(double a, char* bufp, int bufplen)
{
	todmm(0, a, bufp, bufplen);
	return;
}

// Convert Lat/Lon <-> Simple Mercator
void toSM(double lat, double lon, double lat0, double lon0, double* x, double* y)
{
	double xlon = lon;

	// Make sure lon and lon0 are same phase

	if ((lon * lon0 < 0.0) && (fabs(lon - lon0) > 180.0)) {
		lon < 0.0 ? xlon += 360.0 : xlon -= 360.0;
	}

	const double z = WGS84_semimajor_axis_meters * mercator_k0;

	*x = (xlon - lon0) * (M_PI / 180.0) * z;

	// y =.5 ln( (1 + sin t) / (1 - sin t) )
	const double s = sin(lat * (M_PI / 180.0));
	const double y3 = (.5 * log((1 + s) / (1 - s))) * z;

	const double s0 = sin(lat0 * (M_PI / 180.0));
	const double y30 = (.5 * log((1 + s0) / (1 - s0))) * z;
	*y = y3 - y30;
}

Position fromSM(double x, double y, const Position& pos0)
{
	const double z = WGS84_semimajor_axis_meters * mercator_k0;
	const double s0 = sin(pos0.lat() * (M_PI / 180.0));
	const double y0 = (0.5 * log((1 + s0) / (1 - s0))) * z;

	double tlat = (2.0 * atan(exp((y0 + y) / z)) - M_PI / 2.0) / (M_PI / 180.0);
	double tlon = pos0.lon() + (x / ((M_PI / 180.0) * z));

	return Position(tlat, tlon);
}

void toSM_ECC(const Position& pos, const Position& pos0, double* x, double* y)
{
	const double f = 1.0 / WGSinvf; // WGS84 ellipsoid flattening parameter
	const double e2 = 2 * f - f * f; // eccentricity^2  .006700
	const double e = sqrt(e2);

	const double z = WGS84_semimajor_axis_meters * mercator_k0;

	*x = (pos.lon() - pos0.lon()) * (M_PI / 180.0) * z;

	const double s = sin(pos.lat() * (M_PI / 180.0));
	const double y3 = (0.5 * log((1 + s) / (1 - s))) * z;

	const double s0 = sin(pos0.lat() * (M_PI / 180.0));
	const double y30 = (0.5 * log((1 + s0) / (1 - s0))) * z;
	const double y4 = y3 - y30;

	// Add eccentricity terms

	const double falsen = z * log(tan(M_PI / 4 + pos0.lat() * (M_PI / 180.0) / 2)
								  * pow((1.0 - e * s0) / (1.0 + e * s0), e / 2.0));
	const double test = z * log(tan(M_PI / 4 + pos.lat() * (M_PI / 180.0) / 2)
								* pow((1.0 - e * s) / (1.0 + e * s), e / 2.0));
	*y = test - falsen;
}

Position fromSM_ECC(double x, double y, const Position& pos0)
{
	const double f = 1.0 / WGSinvf; // WGS84 ellipsoid flattening parameter
	const double es = 2 * f - f * f; // eccentricity^2  .006700
	const double e = sqrt(es);

	const double z = WGS84_semimajor_axis_meters * mercator_k0;

	double tlon = pos0.lon() + (x / ((M_PI / 180.0) * z));

	const double s0 = sin(pos0.lat() * (M_PI / 180.0));

	const double falsen = z * log(tan(M_PI / 4 + pos0.lat() * (M_PI / 180.0) / 2)
								  * pow((1.0 - e * s0) / (1.0 + e * s0), e / 2.0));
	const double t = exp((y + falsen) / (z));
	const double xi = (M_PI / 2.0) - 2.0 * atan(t);

	// Add eccentricity terms

	double esf = (es / 2.0 + (5 * es * es / 24.0) + (es * es * es / 12.0)
				  + (13.0 * es * es * es * es / 360.0)) * sin(2.0 * xi);
	esf += ((7.0 * es * es / 48.0) + (29.0 * es * es * es / 240.0)
			+ (811.0 * es * es * es * es / 11520.0)) * sin(4.0 * xi);
	esf += ((7.0 * es * es * es / 120.0) + (81 * es * es * es * es / 1120.0)
			+ (4279.0 * es * es * es * es / 161280.0)) * sin(8.0 * xi);

	double tlat = -(xi + esf) / (M_PI / 180.0);

	return Position(tlat, tlon);
}

#define TOL 1e-10
#define CONV      1e-10
#define N_ITER    10
#define I_ITER 20
#define ITOL 1.e-12

void toPOLY(const Position& pos, const Position& pos0, double* x, double* y)
{
	const double z = WGS84_semimajor_axis_meters * mercator_k0;

	if (fabs((pos.lat() - pos0.lat()) * (M_PI / 180.0)) <= TOL) {
		*x = (pos.lon() - pos0.lon()) * (M_PI / 180.0) * z;
		*y = 0.0;

	} else {
		const double E = (pos.lon() - pos0.lon()) * (M_PI / 180.0);
		const double cot = 1.0 / tan(pos.lat() * (M_PI / 180.0));
		*x = sin(E * sin((pos.lat() * (M_PI / 180.0)))) * cot;
		*y = (pos.lat() * (M_PI / 180.0)) - (pos0.lat() * (M_PI / 180.0)) + cot * (1.0 - cos(E));

		*x *= z;
		*y *= z;
	}
}

Position fromPOLY(double x, double y, const Position& pos0)
{
	const double z = WGS84_semimajor_axis_meters * mercator_k0;

	double yp = y - (pos0.lat() * (M_PI / 180.0) * z);
	if (fabs(yp) <= TOL) {
		return geo::Position(pos0.lat(), pos0.lon() + (x / ((M_PI / 180.0) * z)));
	}

	yp = y / z;
	const double xp = x / z;

	double lat3 = yp;
	const double B = (xp * xp) + (yp * yp);
	int i = N_ITER;
	double dphi;

	do {
		double tp = tan(lat3);
		dphi = ((yp) * (lat3 * tp + 1.0) - lat3 - 0.5 * (lat3 * lat3 + B) * tp);
		lat3 -= (dphi / ((lat3 - (yp)) / tp - 1.0));
	} while (fabs(dphi) > CONV && --i);

	if (!i) {
		return geo::Position(0.0, 0.0);
	}

	double t = asin(xp * tan(lat3)) / sin(lat3);
	t /= (M_PI / 180.0);
	t += pos0.lon();
	return geo::Position(lat3 / (M_PI / 180.0), t);
}

// Convert Lat/Lon <-> Transverse Mercator

// converts lat/long to TM coords.  Equations from USGS Bulletin 1532
// East Longitudes are positive, West longitudes are negative.
// North latitudes are positive, South latitudes are negative
// Lat and Long are in decimal degrees.
// Written by Chuck Gantz- chuck.gantz@globalstar.com
// Adapted for opencpn by David S. Register
void toTM(float lat, float lon, float lat0, float lon0, double* x, double* y)
{
	// constants for WGS-84
	const double f = 1.0 / WGSinvf; // WGS84 ellipsoid flattening parameter
	const double a = WGS84_semimajor_axis_meters;
	const double k0 = 1.0; // Scaling factor

	const double eccSquared = 2 * f - f * f;
	const double eccPrimeSquared = (eccSquared) / (1 - eccSquared);
	const double LatRad = lat * (M_PI / 180.0);
	const double LongOriginRad = lon0 * (M_PI / 180.0);
	const double LongRad = lon * (M_PI / 180.0);

	const double N = a / sqrt(1 - eccSquared * sin(LatRad) * sin(LatRad));
	const double T = tan(LatRad) * tan(LatRad);
	const double C = eccPrimeSquared * cos(LatRad) * cos(LatRad);
	const double A = cos(LatRad) * (LongRad - LongOriginRad);

	const double MM = a * ((1 - eccSquared / 4 - 3 * eccSquared * eccSquared / 64
							- 5 * eccSquared * eccSquared * eccSquared / 256) * LatRad
						   - (3 * eccSquared / 8 + 3 * eccSquared * eccSquared / 32
							  + 45 * eccSquared * eccSquared * eccSquared / 1024) * sin(2 * LatRad)
						   + (15 * eccSquared * eccSquared / 256
							  + 45 * eccSquared * eccSquared * eccSquared / 1024) * sin(4 * LatRad)
						   - (35 * eccSquared * eccSquared * eccSquared / 3072) * sin(6 * LatRad));

	*x = (k0 * N
		  * (A + (1 - T + C) * A * A * A / 6 + (5 - 18 * T + T * T + 72 * C - 58 * eccPrimeSquared)
											   * A * A * A * A * A / 120));

	*y = (k0 * (MM + N * tan(LatRad) * (A * A / 2 + (5 - T + 9 * C + 4 * C * C) * A * A * A * A / 24
										+ (61 - 58 * T + T * T + 600 * C - 330 * eccPrimeSquared)
										  * A * A * A * A * A * A / 720)));
}

// converts TM coords to lat/long.  Equations from USGS Bulletin 1532
// East Longitudes are positive, West longitudes are negative.
// North latitudes are positive, South latitudes are negative
// Lat and Long are in decimal degrees
// Written by Chuck Gantz- chuck.gantz@globalstar.com
// Adapted for opencpn by David S. Register
Position fromTM(double x, double y, const Position& pos0)
{
	const double rad2deg = 1.0 / (M_PI / 180.0);
	// constants for WGS-84

	const double f = 1.0 / WGSinvf; // WGS84 ellipsoid flattening parameter
	const double a = WGS84_semimajor_axis_meters;
	const double k0 = 1.0; //  Scaling factor

	const double eccSquared = 2 * f - f * f;

	const double eccPrimeSquared = (eccSquared) / (1 - eccSquared);
	const double e1 = (1.0 - sqrt(1.0 - eccSquared)) / (1.0 + sqrt(1.0 - eccSquared));

	const double MM = y / k0;
	const double mu = MM / (a * (1 - eccSquared / 4 - 3 * eccSquared * eccSquared / 64
								 - 5 * eccSquared * eccSquared * eccSquared / 256));

	const double phi1Rad = mu + (3 * e1 / 2 - 27 * e1 * e1 * e1 / 32) * sin(2 * mu)
						   + (21 * e1 * e1 / 16 - 55 * e1 * e1 * e1 * e1 / 32) * sin(4 * mu)
						   + (151 * e1 * e1 * e1 / 96) * sin(6 * mu);

	const double N1 = a / sqrt(1 - eccSquared * sin(phi1Rad) * sin(phi1Rad));
	const double T1 = tan(phi1Rad) * tan(phi1Rad);
	const double C1 = eccPrimeSquared * cos(phi1Rad) * cos(phi1Rad);
	const double R1 = a * (1 - eccSquared) / pow(1 - eccSquared * sin(phi1Rad) * sin(phi1Rad), 1.5);
	const double D = x / (N1 * k0);

	double tlat = phi1Rad - (N1 * tan(phi1Rad) / R1)
					 * (D * D / 2 - (5 + 3 * T1 + 10 * C1 - 4 * C1 * C1 - 9 * eccPrimeSquared) * D
									* D * D * D / 24 + (61 + 90 * T1 + 298 * C1 + 45 * T1 * T1
														- 252 * eccPrimeSquared - 3 * C1 * C1) * D
													   * D * D * D * D * D / 720);
	tlat = pos0.lat() + (tlat * rad2deg);

	double tlon = (D - (1 + 2 * T1 + C1) * D * D * D / 6
			+ (5 - 2 * C1 + 28 * T1 - 3 * C1 * C1 + 8 * eccPrimeSquared + 24 * T1 * T1) * D * D * D
			  * D * D / 120) / cos(phi1Rad);
	tlon = pos0.lon() + tlon * rad2deg;

	return Position(tlat, tlon);
}

/* --------------------------------------------------------------------------------- *

 *Molodensky
 *In the listing below, the class GeodeticPosition has three members, lon, lat, and h.
 *They are double-precision values indicating the longitude and latitude in radians,
 * and height in meters above the ellipsoid.

 * The source code in the listing below may be copied and reused without restriction,
 * but it is offered AS-IS with NO WARRANTY.

 * Adapted for opencpn by David S. Register

 * --------------------------------------------------------------------------------- */

void MolodenskyTransform(const Position& pos, Position& to, int from_datum_index,
						 int to_datum_index)
{
	const double from_lat = pos.lat() * (M_PI / 180.0);
	const double from_lon = pos.lon() * (M_PI / 180.0);
	const double from_f = 1.0 / gEllipsoid[gDatum[from_datum_index].ellipsoid].invf; // flattening
	const double from_esq = 2 * from_f - from_f * from_f; // eccentricity^2
	const double from_a = gEllipsoid[gDatum[from_datum_index].ellipsoid].a; // semimajor axis
	const double dx = gDatum[from_datum_index].dx;
	const double dy = gDatum[from_datum_index].dy;
	const double dz = gDatum[from_datum_index].dz;
	const double to_f = 1.0 / gEllipsoid[gDatum[to_datum_index].ellipsoid].invf; // flattening
	const double to_a = gEllipsoid[gDatum[to_datum_index].ellipsoid].a; // semimajor axis
	const double da = to_a - from_a;
	const double df = to_f - from_f;
	const double from_h = 0;

	const double slat = sin(from_lat);
	const double clat = cos(from_lat);
	const double slon = sin(from_lon);
	const double clon = cos(from_lon);
	const double ssqlat = slat * slat;
	const double adb = 1.0 / (1.0 - from_f); // "a divided by b"

	const double rn = from_a / sqrt(1.0 - from_esq * ssqlat);
	const double rm = from_a * (1. - from_esq) / pow((1.0 - from_esq * ssqlat), 1.5);

	const double dlat = (((((-dx * slat * clon - dy * slat * slon) + dz * clat)
						   + (da * ((rn * from_esq * slat * clat) / from_a)))
						  + (df * (rm * adb + rn / adb) * slat * clat))) / (rm + from_h);

	const double dlon = (-dx * slon + dy * clon) / ((rn + from_h) * clat);

	const double dh = (dx * clat * clon) + (dy * clat * slon) + (dz * slat) - (da * (from_a / rn))
					  + ((df * rn * ssqlat) / adb);

	to = Position(pos.lat() + dlat / (M_PI / 180.0), pos.lon() + dlon / (M_PI / 180.0));
}

/* --------------------------------------------------------------------------------- */
/*
      Geodesic Forward and Reverse calculation functions
      Abstracted and adapted from PROJ-4.5.0 by David S.Register

      Original source code contains the following license:

      Copyright (c) 2000, Frank Warmerdam

 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/
/* --------------------------------------------------------------------------------- */




#define DTOL    1e-12

#define HALFPI  1.5707963267948966
#define SPI     3.14159265359
#define TWOPI   6.2831853071795864769
#define ONEPI   3.14159265358979323846
#define MERI_TOL 1e-9

double adjlon(double lon)
{
	if (fabs(lon) <= SPI)
		return (lon);
	lon += ONEPI; /* adjust to 0..2pi rad */
	lon -= TWOPI * floor(lon / TWOPI); /* remove integral # of 'revolutions'*/
	lon -= ONEPI; /* adjust back to -pi..pi rad */
	return (lon);
}

// Given the lat/long of starting point, and traveling a specified distance,
// at an initial bearing, calculates the lat/long of the resulting location.
// using elliptic earth model.
Position ll_gc_ll(const Position& pos, double brg, double dist)
{
	double th1;
	double costh1;
	double sinth1;
	double sina12;
	double cosa12;
	double M;
	double N;
	double c1;
	double c2;
	double D;
	double P;
	double s1;
	int merid;
	int signS;

	// Input/Output from geodesic functions
	double al12; // Forward azimuth
	double al21; // Back azimuth
	double geod_S; // Distance
	double phi1;
	double lam1;
	double phi2;
	double lam2;

	int ellipse;
	double geod_f;
	double geod_a;
	double es;
	double onef;
	double f;
	double f64;
	double f2;
	double f4;

	// Setup the static parameters
	phi1 = pos.lat() * (M_PI / 180.0); // initial position
	lam1 = pos.lon() * (M_PI / 180.0);
	al12 = brg * (M_PI / 180.0); // Forward azimuth
	geod_S = dist * 1852.0; // Distance

	// void geod_pre(struct georef_state *state)
	{
		// Stuff the WGS84 projection parameters as necessary
		// To avoid having to include <geodesic.h>
		ellipse = 1;
		f = 1.0 / WGSinvf; // WGS84 ellipsoid flattening parameter
		geod_a = WGS84_semimajor_axis_meters;

		es = 2 * f - f * f;
		onef = sqrt(1. - es);
		geod_f = 1 - onef;
		f2 = geod_f / 2;
		f4 = geod_f / 4;
		f64 = geod_f * geod_f / 64;

		al12 = adjlon(al12); /* reduce to  +- 0-pi */
		signS = fabs(al12) > HALFPI ? 1 : 0;
		th1 = ellipse ? atan(onef * tan(phi1)) : phi1;
		costh1 = cos(th1);
		sinth1 = sin(th1);
		if ((merid = fabs(sina12 = sin(al12)) < MERI_TOL)) {
			sina12 = 0.;
			cosa12 = fabs(al12) < HALFPI ? 1. : -1.;
			M = 0.;
		} else {
			cosa12 = cos(al12);
			M = costh1 * sina12;
		}
		N = costh1 * cosa12;
		if (ellipse) {
			if (merid) {
				c1 = 0.;
				c2 = f4;
				D = 1. - c2;
				D *= D;
				P = c2 / D;
			} else {
				c1 = geod_f * M;
				c2 = f4 * (1. - M * M);
				D = (1. - c2) * (1. - c2 - c1 * M);
				P = (1. + .5 * c1 * M) * c2 / D;
			}
		}
		if (merid)
			s1 = HALFPI - th1;
		else {
			s1 = (fabs(M) >= 1.) ? 0. : acos(M);
			s1 = sinth1 / sin(s1);
			s1 = (fabs(s1) >= 1.) ? 0. : acos(s1);
		}
	}

	// void  geod_for(struct georef_state *state)
	{
		double d;
		double sind;
		double u;
		double V;
		double X;
		double ds;
		double cosds;
		double sinds;
		double ss;
		double de;

		ss = 0.;

		if (ellipse) {
			d = geod_S / (D * geod_a);
			if (signS)
				d = -d;
			u = 2. * (s1 - d);
			V = cos(u + d);
			X = c2* c2*(sind = sin(d)) * cos(d) * (2. * V * V - 1.);
			ds = d + X - 2. * P * V * (1. - 2. * P * cos(u)) * sind;
			ss = s1 + s1 - ds;
		} else {
			ds = geod_S / geod_a;
			if (signS)
				ds = -ds;
		}
		cosds = cos(ds);
		sinds = sin(ds);
		if (signS)
			sinds = -sinds;
		al21 = N * cosds - sinth1 * sinds;
		if (merid) {
			phi2 = atan(tan(HALFPI + s1 - ds) / onef);
			if (al21 > 0.) {
				al21 = M_PI;
				if (signS)
					de = M_PI;
				else {
					phi2 = -phi2;
					de = 0.;
				}
			} else {
				al21 = 0.;
				if (signS) {
					phi2 = -phi2;
					de = 0;
				} else
					de = M_PI;
			}
		} else {
			al21 = atan(M / al21);
			if (al21 > 0)
				al21 += M_PI;
			if (al12 < 0.)
				al21 -= M_PI;
			al21 = adjlon(al21);
			phi2 = atan(-(sinth1 * cosds + N * sinds) * sin(al21) / (ellipse ? onef * M : M));
			de = atan2(sinds * sina12, (costh1 * cosds - sinth1 * sinds * cosa12));
			if (ellipse) {
				if (signS)
					de += c1 * ((1. - c2) * ds + c2 * sinds * cos(ss));
				else
					de -= c1 * ((1. - c2) * ds - c2 * sinds * cos(ss));
			}
		}
		lam2 = adjlon(lam1 + de);
	}

	return Position(phi2 / (M_PI / 180.0), lam2 / (M_PI / 180.0));
}

void ll_gc_ll_reverse(const Position& pos1, const Position& pos2, double* bearing, double* dist)
{
	// Input/Output from geodesic functions
	double al12; // Forward azimuth
	double al21; // Back azimuth
	double geod_S; // Distance
	double phi1;
	double lam1;
	double phi2;
	double lam2;

	int ellipse;
	double geod_f;
	double geod_a;
	double es;
	double onef;
	double f;
	double f64;
	double f2;
	double f4;

	// Setup the static parameters
	phi1 = pos1.lat() * (M_PI / 180.0); // initial position
	lam1 = pos1.lon() * (M_PI / 180.0);
	phi2 = pos2.lat() * (M_PI / 180.0);
	lam2 = pos2.lon() * (M_PI / 180.0);

	// void geod_inv(struct georef_state *state)
	{
		double th1;
		double th2;
		double thm;
		double dthm;
		double dlamm;
		double dlam;
		double sindlamm;
		double costhm;
		double sinthm;
		double cosdthm;
		double sindthm;
		double L;
		double E;
		double cosd;
		double d;
		double X;
		double Y;
		double T;
		double sind;
		double tandlammp;
		double u;
		double v;
		double D;
		double A;
		double B;

		// Stuff the WGS84 projection parameters as necessary
		// To avoid having to include <geodesic.h>

		ellipse = 1;
		f = 1.0 / WGSinvf; // WGS84 ellipsoid flattening parameter
		geod_a = WGS84_semimajor_axis_meters;

		es = 2 * f - f * f;
		onef = sqrt(1. - es);
		geod_f = 1 - onef;
		f2 = geod_f / 2;
		f4 = geod_f / 4;
		f64 = geod_f * geod_f / 64;

		if (ellipse) {
			th1 = atan(onef * tan(phi1));
			th2 = atan(onef * tan(phi2));
		} else {
			th1 = phi1;
			th2 = phi2;
		}
		thm = .5 * (th1 + th2);
		dthm = .5 * (th2 - th1);
		dlamm = .5 *(dlam = adjlon(lam2 - lam1));
		if (fabs(dlam) < DTOL && fabs(dthm) < DTOL) {
			al12 = al21 = geod_S = 0.;
			if (bearing)
				*bearing = 0.;
			if (dist)
				*dist = 0.;
			return;
		}
		sindlamm = sin(dlamm);
		costhm = cos(thm);
		sinthm = sin(thm);
		cosdthm = cos(dthm);
		sindthm = sin(dthm);
		L = sindthm * sindthm + (cosdthm * cosdthm - sinthm * sinthm) * sindlamm * sindlamm;
		d = acos(cosd = 1 - L - L);
		if (ellipse) {
			E = cosd + cosd;
			sind = sin(d);
			Y = sinthm * cosdthm;
			Y *= (Y + Y) / (1. - L);
			T = sindthm * costhm;
			T *= (T + T) / L;
			X = Y + T;
			Y -= T;
			T = d / sind;
			D = 4. * T * T;
			A = D * E;
			B = D + D;
			geod_S = geod_a * sind * (T - f4 * (T * X - Y) + f64 * (X * (A + (T - .5 * (A - E)) * X)
																	- Y * (B + E * Y) + D * X * Y));
			tandlammp
				= tan(.5 * (dlam - .25 * (Y + Y - E * (4. - X))
								   * (f2 * T + f64 * (32. * T - (20. * T - A) * X - (B + 4.) * Y))
								   * tan(dlam)));
		} else {
			geod_S = geod_a * d;
			tandlammp = tan(dlamm);
		}
		u = atan2(sindthm, (tandlammp * costhm));
		v = atan2(cosdthm, (tandlammp * sinthm));
		al12 = adjlon(TWOPI + v - u);
		al21 = adjlon(TWOPI - v - u);
	}

	if (al12 < 0)
		al12 += 2 * M_PI;

	if (bearing)
		*bearing = al12 / (M_PI / 180.0);
	if (dist)
		*dist = geod_S / 1852.0;
}

Position PositionBearingDistanceMercator(const Position& pos, double brg, double dist)
{
	return ll_gc_ll(pos, brg, dist);
}

// Given the lat/long of starting point and ending point,
// calculates the distance along a geodesic curve, using elliptic earth model.
double DistGreatCircle(const Position& start, const Position& destination)
{
	// Input/Output from geodesic functions
	double al12; // Forward azimuth
	double al21; // Back azimuth
	double geod_S; // Distance
	double phi1;
	double lam1;
	double phi2;
	double lam2;

	int ellipse;
	double geod_f;
	double geod_a;
	double es;
	double onef;
	double f;
	double f64;
	double f2;
	double f4;

	double d5;
	phi1 = start.lat() * (M_PI / 180.0);
	lam1 = start.lon() * (M_PI / 180.0);
	phi2 = destination.lat() * (M_PI / 180.0);
	lam2 = destination.lon() * (M_PI / 180.0);

	// void geod_inv(struct georef_state *state)
	{
		double th1;
		double th2;
		double thm;
		double dthm;
		double dlamm;
		double dlam;
		double sindlamm;
		double costhm;
		double sinthm;
		double cosdthm;
		double sindthm;
		double L;
		double E;
		double cosd;
		double d;
		double X;
		double Y;
		double T;
		double sind;
		double tandlammp;
		double u;
		double v;
		double D;
		double A;
		double B;

		// Stuff the WGS84 projection parameters as necessary
		// To avoid having to include <geodesic,h>

		ellipse = 1;
		f = 1.0 / WGSinvf; /* WGS84 ellipsoid flattening parameter */
		geod_a = WGS84_semimajor_axis_meters;

		es = 2 * f - f * f;
		onef = sqrt(1. - es);
		geod_f = 1 - onef;
		f2 = geod_f / 2;
		f4 = geod_f / 4;
		f64 = geod_f * geod_f / 64;

		if (ellipse) {
			th1 = atan(onef * tan(phi1));
			th2 = atan(onef * tan(phi2));
		} else {
			th1 = phi1;
			th2 = phi2;
		}
		thm = .5 * (th1 + th2);
		dthm = .5 * (th2 - th1);
		dlamm = .5 *(dlam = adjlon(lam2 - lam1));
		if (fabs(dlam) < DTOL && fabs(dthm) < DTOL) {
			al12 = al21 = geod_S = 0.;
			return 0.0;
		}
		sindlamm = sin(dlamm);
		costhm = cos(thm);
		sinthm = sin(thm);
		cosdthm = cos(dthm);
		sindthm = sin(dthm);
		L = sindthm * sindthm + (cosdthm * cosdthm - sinthm * sinthm) * sindlamm * sindlamm;
		d = acos(cosd = 1 - L - L);
		if (ellipse) {
			E = cosd + cosd;
			sind = sin(d);
			Y = sinthm * cosdthm;
			Y *= (Y + Y) / (1. - L);
			T = sindthm * costhm;
			T *= (T + T) / L;
			X = Y + T;
			Y -= T;
			T = d / sind;
			D = 4. * T * T;
			A = D * E;
			B = D + D;
			geod_S = geod_a * sind * (T - f4 * (T * X - Y) + f64 * (X * (A + (T - .5 * (A - E)) * X)
																	- Y * (B + E * Y) + D * X * Y));
			tandlammp
				= tan(.5 * (dlam - .25 * (Y + Y - E * (4. - X))
								   * (f2 * T + f64 * (32. * T - (20. * T - A) * X - (B + 4.) * Y))
								   * tan(dlam)));
		} else {
			geod_S = geod_a * d;
			tandlammp = tan(dlamm);
		}
		u = atan2(sindthm, (tandlammp * costhm));
		v = atan2(cosdthm, (tandlammp * sinthm));
		al12 = adjlon(TWOPI + v - u);
		al21 = adjlon(TWOPI - v - u);
	}

	d5 = geod_S / 1852.0;

	return d5;
}

void DistanceBearingMercator(const Position& pos0, const Position& pos1, double* brg, double* dist)
{
	// Calculate bearing by conversion to SM (Mercator) coordinates, then simple trigonometry

	double lon0x = pos0.lon();
	double lon1x = pos1.lon();

	// Make lon points the same phase
	if ((lon0x * lon1x) < 0.) {
		lon0x < 0.0 ? lon0x += 360.0 : lon1x += 360.0;
		//    Choose the shortest distance
		if (fabs(lon0x - lon1x) > 180.) {
			lon0x > lon1x ? lon0x -= 360.0 : lon1x -= 360.0;
		}

		//    Make always positive
		lon1x += 360.0;
		lon0x += 360.0;
	}

	// Classic formula, which fails for due east/west courses....
	if (dist) {
		// In the case of exactly east or west courses
		// we must make an adjustment if we want true Mercator distances

		// This idea comes from Thomas(Cagney)
		// We simply require the dlat to be (slightly) non-zero, and carry on.
		// MAS022210 for HamishB from 1e-4 && .001 to 1e-9 for better precision
		// on small latitude diffs
		const double mlat0 = fabs(pos1.lat() - pos0.lat()) < 1e-9 ? pos0.lat() + 1e-9 : pos0.lat();

		double east, north;
		toSM_ECC(Position(pos1.lat(), lon1x), Position(mlat0, lon0x), &east, &north);
		const double C = atan2(east, north);
		if (cos(C)) {
			const double dlat = (pos1.lat() - mlat0) * 60.0; // in minutes
			*dist = (dlat / cos(C));
		} else {
			*dist = DistGreatCircle(pos0, pos1);
		}
	}

	// Calculate the bearing using the un-adjusted original latitudes and Mercator Sailing
	if (brg) {
		double east, north;
		toSM_ECC(Position(pos1.lat(), lon1x), Position(pos0.lat(), lon0x), &east, &north);

		const double C = atan2(east, north);
		const double brgt = 180.0 + (C * 180.0 / M_PI);
		if (brgt < 0)
			*brg = brgt + 360.0;
		else if (brgt > 360.0)
			*brg = brgt - 360.0;
		else
			*brg = brgt;
	}
}

/* --------------------------------------------------------------------------------- */
/*
 * lmfit
 *
 * Solves or minimizes the sum of squares of m nonlinear
 * functions of n variables.
 *
 * From public domain Fortran version
 * of Argonne National Laboratories MINPACK
 *     argonne national laboratory. minpack project. march 1980.
 *     burton s. garbow, kenneth e. hillstrom, jorge j. more
 * C translation by Steve Moshier
 * Joachim Wuttke converted the source into C++ compatible ANSI style
 * and provided a simplified interface
 */


#define _LMDIF

///=================================================================================
///     Customized section for openCPN georeferencing

double my_fit_function(double tx, double ty, int n_par, double* p)
{
	double ret = p[0] + p[1] * tx;

	if (n_par > 2)
		ret += p[2] * ty;
	if (n_par > 3) {
		ret += p[3] * tx * tx;
		ret += p[4] * tx * ty;
		ret += p[5] * ty * ty;
	}
	if (n_par > 6) {
		ret += p[6] * tx * tx * tx;
		ret += p[7] * tx * tx * ty;
		ret += p[8] * tx * ty * ty;
		ret += p[9] * ty * ty * ty;
	}

	return ret;
}

// n_points : number of sample points
// n_par :  3, 6, or 10,  6 is probably good
// tx:  sample data independent variable 1
// ty:  sample data independent variable 2
// y:   sample data dependent result
// p:   curve fit result coefficients
int Georef_Calculate_Coefficients_Onedir(int n_points, int n_par, double* tx, double* ty, double* y,
										 double* p, double hintp0, double hintp1, double hintp2)
{
	lm_control_type control;
	lm_data_type data;

	lm_initialize_control(&control);

	for (int i = 0; i < 12; i++)
		p[i] = 0.0;

	// Insert hints
	p[0] = hintp0;
	p[1] = hintp1;
	p[2] = hintp2;

	data.user_func = my_fit_function;
	data.user_tx = tx;
	data.user_ty = ty;
	data.user_y = y;
	data.n_par = n_par;
	data.print_flag = 0;

	// perform the fit:
	lm_minimize(n_points, n_par, p, lm_evaluate_default, lm_print_default, &data, &control);

	// Control.info results [1,2,3] are success, other failure
	return control.info;
}

int Georef_Calculate_Coefficients(struct GeoRef* cp, int nlin_lon)
{
	// Zero out the points
	for (int i = 0; i < 10; ++i)
		cp->pwx[i] = cp->pwy[i] = cp->wpx[i] = cp->wpy[i] = 0.0;

	int mp = 3;

	switch (cp->order) {
		case 1:
			mp = 3;
			break;
		case 2:
			mp = 6;
			break;
		case 3:
			mp = 10;
			break;
		default:
			mp = 3;
			break;
	}

	const int mp_lat = mp;

	// Force linear fit for longitude if nlin_lon > 0
	const int mp_lon = nlin_lon ? 2 : mp;

	// Make a dummy double array
	std::vector<double> pnull(cp->count, 1.0);

	// pixel(tx,ty) to (lat,lon)
	// Calculate and use a linear equation for p[0..2] to hint the solver

	int r1 = Georef_Calculate_Coefficients_Onedir(
		cp->count, mp_lon, cp->tx, cp->ty, cp->lon, cp->pwx,
		cp->lonmin - (cp->txmin * (cp->lonmax - cp->lonmin) / (cp->txmax - cp->txmin)),
		(cp->lonmax - cp->lonmin) / (cp->txmax - cp->txmin), 0.);

	// if blin_lon > 0, force cross terms in latitude equation coefficients to be zero by
	// making lat not dependent on tx,
	double* px = nlin_lon ? &pnull[0] : cp->tx;

	int r2 = Georef_Calculate_Coefficients_Onedir(
		cp->count, mp_lat, px, cp->ty, cp->lat, cp->pwy,
		cp->latmin - (cp->tymin * (cp->latmax - cp->latmin) / (cp->tymax - cp->tymin)), 0.,
		(cp->latmax - cp->latmin) / (cp->tymax - cp->tymin));

	// (lat,lon) to pixel(tx,ty)
	// Calculate and use a linear equation for p[0..2] to hint the solver

	int r3 = Georef_Calculate_Coefficients_Onedir(
		cp->count, mp_lon, cp->lon, cp->lat, cp->tx, cp->wpx,
		cp->txmin - ((cp->txmax - cp->txmin) * cp->lonmin / (cp->lonmax - cp->lonmin)),
		(cp->txmax - cp->txmin) / (cp->lonmax - cp->lonmin), 0.0);

	// if blin_lon > 0, force cross terms in latitude equation coefficients to be zero by
	// making ty not dependent on lon,
	px = nlin_lon ? &pnull[0] : cp->lon;

	int r4 = Georef_Calculate_Coefficients_Onedir(
		cp->count, mp_lat, &pnull[0] /*cp->lon*/, cp->lat, cp->ty, cp->wpy,
		cp->tymin - ((cp->tymax - cp->tymin) * cp->latmin / (cp->latmax - cp->latmin)), 0.0,
		(cp->tymax - cp->tymin) / (cp->latmax - cp->latmin));

	if ((r1) && (r1 < 4) && (r2) && (r2 < 4) && (r3) && (r3 < 4) && (r4) && (r4 < 4))
		return 0;

	return 1;
}

int Georef_Calculate_Coefficients_Proj(struct GeoRef* cp)
{
	int r1, r2, r3, r4;
	int mp;

	// Zero out the points
	cp->pwx[6] = cp->pwy[6] = cp->wpx[6] = cp->wpy[6] = 0.0;
	cp->pwx[7] = cp->pwy[7] = cp->wpx[7] = cp->wpy[7] = 0.0;
	cp->pwx[8] = cp->pwy[8] = cp->wpx[8] = cp->wpy[8] = 0.0;
	cp->pwx[9] = cp->pwy[9] = cp->wpx[9] = cp->wpy[9] = 0.0;
	cp->pwx[3] = cp->pwy[3] = cp->wpx[3] = cp->wpy[3] = 0.0;
	cp->pwx[4] = cp->pwy[4] = cp->wpx[4] = cp->wpy[4] = 0.0;
	cp->pwx[5] = cp->pwy[5] = cp->wpx[5] = cp->wpy[5] = 0.0;
	cp->pwx[0] = cp->pwy[0] = cp->wpx[0] = cp->wpy[0] = 0.0;
	cp->pwx[1] = cp->pwy[1] = cp->wpx[1] = cp->wpy[1] = 0.0;
	cp->pwx[2] = cp->pwy[2] = cp->wpx[2] = cp->wpy[2] = 0.0;

	mp = 3;

	// pixel(tx,ty) to (northing,easting)
	// Calculate and use a linear equation for p[0..2] to hint the solver

	r1 = Georef_Calculate_Coefficients_Onedir(
		cp->count, mp, cp->tx, cp->ty, cp->lon, cp->pwx,
		cp->lonmin - (cp->txmin * (cp->lonmax - cp->lonmin) / (cp->txmax - cp->txmin)),
		(cp->lonmax - cp->lonmin) / (cp->txmax - cp->txmin), 0.);

	r2 = Georef_Calculate_Coefficients_Onedir(
		cp->count, mp, cp->tx, cp->ty, cp->lat, cp->pwy,
		cp->latmin - (cp->tymin * (cp->latmax - cp->latmin) / (cp->tymax - cp->tymin)), 0.,
		(cp->latmax - cp->latmin) / (cp->tymax - cp->tymin));

	// (northing/easting) to pixel(tx,ty)
	// Calculate and use a linear equation for p[0..2] to hint the solver

	r3 = Georef_Calculate_Coefficients_Onedir(
		cp->count, mp, cp->lon, cp->lat, cp->tx, cp->wpx,
		cp->txmin - ((cp->txmax - cp->txmin) * cp->lonmin / (cp->lonmax - cp->lonmin)),
		(cp->txmax - cp->txmin) / (cp->lonmax - cp->lonmin), 0.0);

	r4 = Georef_Calculate_Coefficients_Onedir(
		cp->count, mp, cp->lon, cp->lat, cp->ty, cp->wpy,
		cp->tymin - ((cp->tymax - cp->tymin) * cp->latmin / (cp->latmax - cp->latmin)), 0.0,
		(cp->tymax - cp->tymin) / (cp->latmax - cp->latmin));

	if ((r1) && (r1 < 4) && (r2) && (r2 < 4) && (r3) && (r3 < 4) && (r4) && (r4 < 4))
		return 0;
	else
		return 1;
}

}

