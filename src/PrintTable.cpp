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
#include <sstream>
#include <vector>

#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/artprov.h>
#include <wx/stdpaths.h>
#include <wx/intl.h>
#include <wx/listctrl.h>
#include <wx/aui/aui.h>
#include <wx/dialog.h>
#include <wx/progdlg.h>
#include <wx/brush.h>
#include <wx/colour.h>
#include <wx/tokenzr.h>

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

#include "PrintTable.h"



PrintTable::PrintTable()
	: Table()
{
	rows_heights.clear();
}

void PrintTable::AdjustCells(wxDC * dc, int marginX, int marginY)
{
	number_of_pages = -1;
	contents.clear();
	int sum = 0;
	for ( size_t j = 0; j < widths.size(); j++ ) {
		sum += widths[ j ];
	}

	int w, h;
	dc->GetSize( &w, &h );

	double scale_x, scale_y;
	dc->GetUserScale(&scale_x, &scale_y);
	w /= scale_x;
	h /= scale_y;

	int width = w - 4 * marginX;
	header_height = -1;
	for ( size_t j = 0; j < header.size(); j++ ) {
		int cell_width = ( int )( ( double )width * widths[ j ] / sum );
		PrintCell cell_content;
		cell_content.Init( header[ j ], dc, cell_width, 10, true );
		header_content.push_back( cell_content );
		header_height = std::max( header_height, cell_content.GetHeight() );
	}

	for ( size_t i = 0; i < data.size(); i++ ) {
		const Row & row = data[ i ];
		ContentRow contents_row;
		int max_height = -1;
		for ( size_t j = 0; j < row.size(); j++ ) {
			int cell_width = ( int )( ( double )width * widths[ j ] / sum );
			PrintCell cell_content;
			cell_content.Init( row[ j ], dc, cell_width, 10 );
			contents_row.push_back( cell_content );
			max_height = std::max( max_height, cell_content.GetHeight() );
		}
		rows_heights.push_back( max_height );
		contents.push_back( contents_row );
	}

	int stripped_page = h - 4 * marginY - header_height;
	int current_page = 1;
	int current_y = 0;
	for ( size_t i = 0; i < data.size(); i++ ) {
		int row_height = rows_heights[ i ];
		if ( row_height + current_y > stripped_page ) {
			current_page++;
			current_y = row_height;
		} else {
			current_y += row_height;
		}
		int row_page = current_page;
		ContentRow & contents_row = contents[ i ];
		for ( size_t j = 0; j < contents_row.size(); j++ ) {
			contents_row[ j ].SetPage( row_page );
			contents_row[ j ].SetHeight( row_height );
		}
		number_of_pages = std::max( row_page, number_of_pages );
	}
}

// delivers content of the table
const PrintTable::Content & PrintTable::GetContent() const
{
	return contents;
}

// delivers header  of the table
const PrintTable::ContentRow & PrintTable::GetHeader() const
{
	return header_content;
}

// returns the total number of needed pages;
int PrintTable::GetNumberPages() const
{
	return number_of_pages;
}

// Returns the height of the header
int PrintTable::GetHeaderHeight() const
{
	return header_height;
}

