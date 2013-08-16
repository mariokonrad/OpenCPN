/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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

#include <iostream>
using namespace std;

#include "wx/print.h"
#include "wx/printdlg.h"
#include "wx/artprov.h"
#include "wx/stdpaths.h"
#include <wx/intl.h>
#include <wx/listctrl.h>
#include <wx/aui/aui.h>
#include <wx/dialog.h>
#include <wx/progdlg.h>
#include <wx/brush.h>
#include <wx/colour.h>


#if wxCHECK_VERSION( 2, 9, 0 )
	#include <wx/dialog.h>
#endif

#include "dychart.h"

#ifdef __WXMSW__
	#include <stdlib.h>
	#include <math.h>
	#include <time.h>
	#include <psapi.h>
#endif

#ifndef __WXMSW__
	#include <signal.h>
	#include <setjmp.h>
#endif

#include "RoutePrintout.h"

#define PRINT_WP_NAME 0
#define PRINT_WP_POSITION 1
#define PRINT_WP_COURSE 2
#define PRINT_WP_DISTANCE 3
#define PRINT_WP_DESCRIPTION 4

RoutePrintout::RoutePrintout(
		std::vector<bool> _toPrintOut,
		Route * route,
		const wxChar * title)
	: MyPrintout(title)
	, myRoute(route)
	, toPrintOut(_toPrintOut)
{
	// Let's have at least some device units margin
	marginX = 5;
	marginY = 5;

	// Offset text from the edge of the cell (Needed on Linux)
	textOffsetX = 5;
	textOffsetY = 8;

	table.StartFillHeader();
	// setup widths for columns
	if ( toPrintOut[ PRINT_WP_NAME ] ) {
		table << (const char *)wxString(_("Name")).mb_str();
	}
	if ( toPrintOut[ PRINT_WP_POSITION ] ) {
		table << (const char *)wxString(_("Position")).mb_str();
	}
	if ( toPrintOut[ PRINT_WP_COURSE ] ) {
		table << (const char *)wxString(_("Course")).mb_str();
	}
	if ( toPrintOut[ PRINT_WP_DISTANCE ] ) {
		table << (const char *)wxString(_("Distance")).mb_str();
	}
	if ( toPrintOut[ PRINT_WP_DESCRIPTION ] ) {
		table << (const char *)wxString(_("Description")).mb_str();
	}

	table.StartFillWidths();
	// setup widths for columns
	if ( toPrintOut[ PRINT_WP_NAME ] ) {
		table << 23;
	}
	if ( toPrintOut[ PRINT_WP_POSITION ] ) {
		table << 40;
	}
	if ( toPrintOut[ PRINT_WP_COURSE ] ) {
		table << 30;
	}
	if ( toPrintOut[ PRINT_WP_DISTANCE ] ) {
		table << 38;
	}
	if ( toPrintOut[ PRINT_WP_DESCRIPTION ] ) {
		table << 100;
	}

	table.StartFillData();

	for ( int n = 1; n <= myRoute->GetnPoints(); n++ ) {
		RoutePoint* point = myRoute->GetPoint( n );

		if ( toPrintOut[ PRINT_WP_NAME ] ) {
			string cell( point->GetName().mb_str() );
			table << cell;
		}
		if ( toPrintOut[ PRINT_WP_POSITION ] ) {
			wxString point_position = toSDMM( 1, point->m_lat, point->m_bIsInTrack ) + _T( "\n" ) + toSDMM( 2, point->m_lon, point->m_bIsInTrack );
			string   cell( point_position.mb_str() );
			table << cell;
		}
		if ( toPrintOut[ PRINT_WP_COURSE ] ) {
			wxString point_course;
			point_course.Printf( _T( "%03.0f Deg" ), point->GetCourse() );
			string   cell( point_course.mb_str() );
			table << cell;
		}
		if ( toPrintOut[ PRINT_WP_DISTANCE ] ) {
			wxString point_distance;
			point_distance.Printf( _T( "%6.2f" + getUsrDistanceUnit() ), toUsrDistance( point->GetDistance() ) );
			string   cell( point_distance.mb_str() );
			table << cell;
		}
		if ( toPrintOut[ PRINT_WP_DESCRIPTION ] ) {
			string cell( point->GetDescription().mb_str() );
			table << cell;
		}
		table << "\n";
	}
}


void RoutePrintout::GetPageInfo( int* minPage, int* maxPage, int* selPageFrom, int* selPageTo )
{
	*minPage     = 1;
	*maxPage     = numberOfPages;
	*selPageFrom = 1;
	*selPageTo   = numberOfPages;
}


void RoutePrintout::OnPreparePrinting()
{
	pageToPrint = 1;
	wxDC*  dc = GetDC();
	wxFont routePrintFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
	dc->SetFont( routePrintFont );

	// Get the size of the DC in pixels
	int w, h;
	dc->GetSize( &w, &h );

	// We don't know before hand what size the Print DC will be, in pixels.  Varies by host.
	// So, if the dc size is greater than 1000 pixels, we scale accordinly.

	int maxX = wxMin(w, 1000);
	int maxY = wxMin(h, 1000);

	// Calculate a suitable scaling factor
	double scaleX = ( double )( w / maxX );
	double scaleY = ( double )( h / maxY );

	// Use x or y scaling factor, whichever fits on the DC
	double actualScale = wxMin( scaleX, scaleY );

	// Set the scale and origin
	dc->SetUserScale( actualScale, actualScale );
	dc->SetDeviceOrigin( ( long )marginX, ( long )marginY );

	table.AdjustCells( dc, marginX, marginY );
	numberOfPages = table.GetNumberPages();
}


bool RoutePrintout::OnPrintPage( int page )
{
	wxDC* dc = GetDC();
	if( dc ) {
		if( page <= numberOfPages ){
			pageToPrint = page;
			DrawPage( dc );
			return true;
		}
		else
			return false;
	} else
		return false;
}

void RoutePrintout::DrawPage( wxDC* dc )
{
	wxFont routePrintFont_bold( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD );
	dc->SetFont( routePrintFont_bold );
	wxBrush brush( wxColour(255,255,255),  wxTRANSPARENT );
	dc->SetBrush( brush );

	int header_textOffsetX = 2;
	int header_textOffsetY = 2;

	int currentX = marginX;
	int currentY = marginY;
	const PrintTable::ContentRow & header_content = table.GetHeader();
	for ( size_t j = 0; j < header_content.size(); j++ ) {
		const PrintCell & cell = header_content[ j ];
		dc->DrawRectangle( currentX, currentY, cell.GetWidth(), cell.GetHeight() );
		dc->DrawText( cell.GetText(),  currentX +header_textOffsetX, currentY + header_textOffsetY );
		currentX += cell.GetWidth();
	}

	wxFont  routePrintFont_normal( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
	dc->SetFont( routePrintFont_normal );

	const PrintTable::Content & cells = table.GetContent();
	currentY = marginY + table.GetHeaderHeight();
	int currentHeight = 0;
	for ( size_t i = 0; i < cells.size(); i++ ) {
		const PrintTable::ContentRow & content_row = cells[ i ];
		currentX = marginX;
		for ( size_t j = 0; j < content_row.size(); j++ ) {
			const PrintCell& cell = content_row[ j ];
			if ( cell.GetPage() == pageToPrint ) {
				wxRect r( currentX, currentY, cell.GetWidth(), cell.GetHeight() );
				dc->DrawRectangle( r );
				r.Offset( textOffsetX, textOffsetY );
				dc->DrawLabel(cell.GetText(), r);
				currentX     += cell.GetWidth();
				currentHeight = cell.GetHeight();
			}
		}
		currentY += currentHeight;
	}
}

