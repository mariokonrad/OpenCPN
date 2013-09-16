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

#include <wx/tokenzr.h>
#include <wx/sstream.h>
#include <wx/image.h>
#include <wx/filename.h>
#include <wx/graphics.h>
#include <wx/dir.h>
#include <wx/listimpl.cpp>
#include <wx/progdlg.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <locale>
#include <deque>

#include "dychart.h"
#include "MainFrame.h"
#include "MessageBox.h"
#include "navutil.h"
#include "Track.h"
#include "georef.h"
#include "cutil.h"
#include "StyleManager.h"
#include "Routeman.h"
#include "WayPointman.h"
#include "RouteProp.h"
#include "ocpnDC.h"
#include "Geodesic.h"
#include "datastream.h"
#include "Multiplexer.h"
#include "Route.h"
#include "Select.h"
#include "FontMgr.h"
#include "OCPN_Sound.h"
#include "Layer.h"
#include "NavObjectChanges.h"
#include "NMEALogWindow.h"
#include "MicrosoftCompatibility.h"

#include <ChartCanvas.h>

#include <chart/s52utils.h>

#include <ais/ais.h>

extern LayerList * pLayerList;
extern RouteList * pRouteList;
extern wxArrayString * pMessageOnceArray;
extern int g_iDistanceFormat;
extern int g_iSpeedFormat;
extern int g_iSDMMFormat;

wxString GetLayerName(int id)
{
	wxString name(_T("unknown layer"));
	if (id <= 0)
		return name;

	for (LayerList::iterator it = pLayerList->begin(); it != pLayerList->end(); ++it) {
		Layer * layer = (Layer *) ( *it );
		if (layer->m_LayerID == id)
			return layer->m_LayerName;
	}
	return name;
}

Route * RouteExists(const wxString & guid)
{
	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route * route = *i;
		if (guid == route->m_GUID)
			return route;
	}
	return NULL;
}

// This function formats the input date/time into a valid GPX ISO 8601
// time string specified in the UTC time zone.

wxString FormatGPXDateTime(wxDateTime dt)
{
	//      return dt.Format(wxT("%Y-%m-%dT%TZ"), wxDateTime::GMT0);
	return dt.Format( wxT("%Y-%m-%dT%H:%M:%SZ") );
}


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

const wxChar *ParseGPXDateTime(wxDateTime & dt, const wxChar * datetime)
{
	long sign;
	long hrs_west;
	long mins_west;
	const wxChar *end;

	// Skip any leading whitespace
	while( isspace( *datetime ) )
		datetime++;

	// Skip (and ignore) leading hyphen
	if( *datetime == wxT('-') ) datetime++;

	// Parse and validate ISO 8601 date/time string
	if( ( end = dt.ParseFormat( datetime, wxT("%Y-%m-%dT%T") ) ) != NULL ) {

		// Invalid date/time
		if( *end == 0 ) return NULL;

		// ParseFormat outputs in UTC if the controlling
		// wxDateTime class instance has not been initialized.

		// Date/time followed by UTC time zone flag, so we are done
		else
			if( *end == wxT('Z') ) {
				end++;
				return end;
			}

		// Date/time followed by given number of hrs/mins west of UTC
			else
				if( *end == wxT('+') || *end == wxT('-') ) {

					// Save direction from UTC
					if( *end == wxT('+') ) sign = 1;
					else
						sign = -1;
					end++;

					// Parse hrs west of UTC
					if( isdigit( *end ) && isdigit( *( end + 1 ) ) && *( end + 2 ) == wxT(':') ) {

						// Extract and validate hrs west of UTC
						wxString( end ).ToLong( &hrs_west );
						if( hrs_west > 12 ) return NULL;
						end += 3;

						// Parse mins west of UTC
						if( isdigit( *end ) && isdigit( *( end + 1 ) ) ) {

							// Extract and validate mins west of UTC
							wxChar mins[3];
							mins[0] = *end;
							mins[1] = *( end + 1 );
							mins[2] = 0;
							wxString( mins ).ToLong( &mins_west );
							if( mins_west > 59 ) return NULL;

							// Apply correction
							dt -= sign * wxTimeSpan( hrs_west, mins_west, 0, 0 );
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


/**************************************************************************/
/*          LogMessageOnce                                                */
/**************************************************************************/

bool LogMessageOnce(const wxString & msg)
{
	for (unsigned int i = 0; i < pMessageOnceArray->GetCount(); ++i) {
		if (msg.IsSameAs(pMessageOnceArray->Item(i)))
			return false;
	}

	pMessageOnceArray->Add( msg );
	wxLogMessage(msg);
	return true;
}

/**************************************************************************/
/*          Converts the distance to the units selected by user           */
/**************************************************************************/
double toUsrDistance(double nm_distance, int unit)
{
	if (unit == -1)
		unit = g_iDistanceFormat;
	switch (unit) {
		case DISTANCE_NMI: return nm_distance; // Nautical miles
		case DISTANCE_MI:  return nm_distance * 1.15078; // statute miles
		case DISTANCE_KM:  return nm_distance * 1.852;
		case DISTANCE_M:   return nm_distance * 1852;
	}
	return 0.0;
}

/**************************************************************************/
/*          Converts the distance from the units selected by user to NMi  */
/**************************************************************************/
double fromUsrDistance(double usr_distance, int unit)
{
	if (unit == -1)
		unit = g_iDistanceFormat;
	switch (unit) {
		case DISTANCE_NMI: return usr_distance; // nautical miles
		case DISTANCE_MI:  return usr_distance / 1.15078; // statute miles
		case DISTANCE_KM:  return usr_distance / 1.852;
		case DISTANCE_M:   return usr_distance / 1852;
	}
	return 0.0;
}

/**************************************************************************/
/*          Returns the abbreviation of user selected distance unit       */
/**************************************************************************/
wxString getUsrDistanceUnit(int unit)
{
	if (unit == -1)
		unit = g_iDistanceFormat;
	switch (unit) {
		case DISTANCE_NMI: return _T("NMi"); // nautical miles
		case DISTANCE_MI:  return _T("mi");  // statute miles
		case DISTANCE_KM:  return _T("km");
		case DISTANCE_M:   return _T("m");
	}
	return wxString();;
}

/**************************************************************************/
/*          Converts the speed to the units selected by user              */
/**************************************************************************/
double toUsrSpeed(double kts_speed, int unit)
{
	if (unit == -1)
		unit = g_iSpeedFormat;
	switch (unit) {
		case SPEED_KTS: return kts_speed; //kts
		case SPEED_MPH: return kts_speed * 1.15078; //mph
		case SPEED_KMH: return kts_speed * 1.852; //km/h
		case SPEED_MS:  return kts_speed * 0.514444444; //m/s
	}
	return 0.0;
}

/**************************************************************************/
/*          Converts the speed from the units selected by user to knots   */
/**************************************************************************/
double fromUsrSpeed(double usr_speed, int unit)
{
	if (unit == -1)
		unit = g_iSpeedFormat;
	switch (unit) {
		case SPEED_KTS: return usr_speed; //kts
		case SPEED_MPH: return usr_speed / 1.15078; //mph
		case SPEED_KMH: return usr_speed / 1.852; //km/h
		case SPEED_MS:  return usr_speed / 0.514444444; //m/s
	}
	return 0.0;
}

/**************************************************************************/
/*          Returns the abbreviation of user selected speed unit          */
/**************************************************************************/
wxString getUsrSpeedUnit(int unit)
{
	if (unit == -1)
		unit = g_iSpeedFormat;
	switch (unit) {
		case SPEED_KTS: return _T("kts"); //kts
		case SPEED_MPH: return _T("mph"); //mph
		case SPEED_KMH: return _T("km/h");
		case SPEED_MS:  return _T("m/s");
	}
	return wxString();
}

/**************************************************************************/
/*          Formats the coordinates to string                             */
/**************************************************************************/
wxString toSDMM( int NEflag, double a, bool hi_precision )
{
	wxString s;
	double mpy;
	short neg = 0;
	int d;
	long m;
	double ang = a;
	char c = 'N';

	if( a < 0.0 ) {
		a = -a;
		neg = 1;
	}
	d = (int) a;
	if( neg ) d = -d;
	if( NEflag ) {
		if( NEflag == 1 ) {
			c = 'N';

			if( neg ) {
				d = -d;
				c = 'S';
			}
		} else
			if( NEflag == 2 ) {
				c = 'E';

				if( neg ) {
					d = -d;
					c = 'W';
				}
			}
	}

	switch( g_iSDMMFormat ){
		case 0:
			mpy = 600.0;
			if( hi_precision ) mpy = mpy * 1000;

			m = (long) wxRound( ( a - (double) d ) * mpy );

			if( !NEflag || NEflag < 1 || NEflag > 2 ) //Does it EVER happen?
			{
				if( hi_precision ) s.Printf( _T ( "%d %02ld.%04ld'" ), d, m / 10000, m % 10000 );
				else
					s.Printf( _T ( "%d %02ld.%01ld'" ), d, m / 10, m % 10 );
			} else {
				if( hi_precision )
					if (NEflag == 1)
						s.Printf( _T ( "%02d %02ld.%04ld %c" ), d, m / 10000, ( m % 10000 ), c );
					else
						s.Printf( _T ( "%03d %02ld.%04ld %c" ), d, m / 10000, ( m % 10000 ), c );
				else
					if (NEflag == 1)
						s.Printf( _T ( "%02d %02ld.%01ld %c" ), d, m / 10, ( m % 10 ), c );
					else
						s.Printf( _T ( "%03d %02ld.%01ld %c" ), d, m / 10, ( m % 10 ), c );
			}
			break;
		case 1:
			if( hi_precision ) s.Printf( _T ( "%03.6f" ), ang ); //cca 11 cm - the GPX precision is higher, but as we use hi_precision almost everywhere it would be a little too much....
			else
				s.Printf( _T ( "%03.4f" ), ang ); //cca 11m
			break;
		case 2:
			m = (long) ( ( a - (double) d ) * 60 );
			mpy = 10.0;
			if( hi_precision ) mpy = mpy * 100;
			long sec = (long) ( ( a - (double) d - ( ( (double) m ) / 60 ) ) * 3600 * mpy );

			if( !NEflag || NEflag < 1 || NEflag > 2 ) //Does it EVER happen?
			{
				if( hi_precision ) s.Printf( _T ( "%d %ld'%ld.%ld\"" ), d, m, sec / 1000,
						sec % 1000 );
				else
					s.Printf( _T ( "%d %ld'%ld.%ld\"" ), d, m, sec / 10, sec % 10 );
			} else {
				if( hi_precision )
					if (NEflag == 1)
						s.Printf( _T ( "%02d %02ld %02ld.%03ld %c" ), d, m, sec / 1000, sec % 1000, c );
					else
						s.Printf( _T ( "%03d %02ld %02ld.%03ld %c" ), d, m, sec / 1000, sec % 1000, c );
				else
					if (NEflag == 1)
						s.Printf( _T ( "%02d %02ld %02ld.%ld %c" ), d, m, sec / 10, sec % 10, c );
					else
						s.Printf( _T ( "%03d %02ld %02ld.%ld %c" ), d, m, sec / 10, sec % 10, c );
			}
			break;
	}
	return s;
}

/****************************************************************************/
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
/****************************************************************************/
double fromDMM( wxString sdms )
{
	wchar_t buf[64];
	char narrowbuf[64];
	int i, len, top = 0;
	double stk[32], sign = 1;

	//First round of string modifications to accomodate some known strange formats
	wxString replhelper;
	replhelper = wxString::FromUTF8( "´·" ); //UKHO PDFs
	sdms.Replace( replhelper, _T(".") );
	replhelper = wxString::FromUTF8( "\"·" ); //Don't know if used, but to make sure
	sdms.Replace( replhelper, _T(".") );
	replhelper = wxString::FromUTF8( "·" );
	sdms.Replace( replhelper, _T(".") );

	replhelper = wxString::FromUTF8( "s. š." ); //Another example: cs.wikipedia.org (someone was too active translating...)
	sdms.Replace( replhelper, _T("N") );
	replhelper = wxString::FromUTF8( "j. š." );
	sdms.Replace( replhelper, _T("S") );
	sdms.Replace( _T("v. d."), _T("E") );
	sdms.Replace( _T("z. d."), _T("W") );

	//If the string contains hemisphere specified by a letter, then '-' is for sure a separator...
	sdms.UpperCase();
	if( sdms.Contains( _T("N") ) || sdms.Contains( _T("S") ) || sdms.Contains( _T("E") )
			|| sdms.Contains( _T("W") ) ) sdms.Replace( _T("-"), _T(" ") );

	wcsncpy( buf, sdms.wc_str( wxConvUTF8 ), 64 );
	len = wcslen( buf );

	for( i = 0; i < len; i++ ) {
		wchar_t c = buf[i];
		if( ( c >= '0' && c <= '9' ) || c == '-' || c == '.' || c == '+' ) {
			narrowbuf[i] = c;
			continue; /* Digit characters are cool as is */
		}
		if( c == ',' ) {
			narrowbuf[i] = '.'; /* convert to decimal dot */
			continue;
		}
		if( ( c | 32 ) == 'w' || ( c | 32 ) == 's' ) sign = -1; /* These mean "negate" (note case insensitivity) */
		narrowbuf[i] = 0; /* Replace everything else with nuls */
	}

	/* Build a stack of doubles */
	stk[0] = stk[1] = stk[2] = 0;
	for( i = 0; i < len; i++ ) {
		while( i < len && narrowbuf[i] == 0 )
			i++;
		if( i != len ) {
			stk[top++] = atof( narrowbuf + i );
			i += strlen( narrowbuf + i );
		}
	}

	return sign * ( stk[0] + ( stk[1] + stk[2] / 60 ) / 60 );
}

/* render a rectangle at a given color and transparency */
void AlphaBlending( ocpnDC &dc, int x, int y, int size_x, int size_y, float radius, wxColour color,
		unsigned char transparency )
{
	wxDC *pdc = dc.GetDC();
	if( pdc ) {
		//    Get wxImage of area of interest
		wxBitmap obm( size_x, size_y );
		wxMemoryDC mdc1;
		mdc1.SelectObject( obm );
		mdc1.Blit( 0, 0, size_x, size_y, pdc, x, y );
		mdc1.SelectObject( wxNullBitmap );
		wxImage oim = obm.ConvertToImage();

		//    Create destination image
		wxBitmap olbm( size_x, size_y );
		wxMemoryDC oldc( olbm );
		oldc.SetBackground( *wxBLACK_BRUSH );
		oldc.SetBrush( *wxWHITE_BRUSH );
		oldc.Clear();

		if( radius > 0.0 )
			oldc.DrawRoundedRectangle( 0, 0, size_x, size_y, radius );

		wxImage dest = olbm.ConvertToImage();
		unsigned char *dest_data = (unsigned char *) malloc(
				size_x * size_y * 3 * sizeof(unsigned char) );
		unsigned char *bg = oim.GetData();
		unsigned char *box = dest.GetData();
		unsigned char *d = dest_data;

		float alpha = 1.0 - (float)transparency / 255.0;
		int sb = size_x * size_y;
		for( int i = 0; i < sb; i++ ) {
			float a = alpha;
			if( *box == 0 && radius > 0.0 ) a = 1.0;
			int r = ( ( *bg++ ) * a ) + (1.0-a) * color.Red();
			*d++ = r; box++;
			int g = ( ( *bg++ ) * a ) + (1.0-a) * color.Green();
			*d++ = g; box++;
			int b = ( ( *bg++ ) * a ) + (1.0-a) * color.Blue();
			*d++ = b; box++;
		}

		dest.SetData( dest_data );

		//    Convert destination to bitmap and draw it
		wxBitmap dbm( dest );
		dc.DrawBitmap( dbm, x, y, false );

		// on MSW, the dc Bounding box is not updated on DrawBitmap() method.
		// Do it explicitely here for all platforms.
		dc.CalcBoundingBox( x, y );
		dc.CalcBoundingBox( x + size_x, y + size_y );
	} else {
		/* opengl version */
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		glColor4ub( color.Red(), color.Green(), color.Blue(), transparency );

		glBegin( GL_QUADS );
		glVertex2i( x, y );
		glVertex2i( x + size_x, y );
		glVertex2i( x + size_x, y + size_y );
		glVertex2i( x, y + size_y );
		glEnd();

		glDisable( GL_BLEND );
	}
}

