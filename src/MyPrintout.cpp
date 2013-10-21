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

#include "MyPrintout.h"
#include <ChartCanvas.h>

extern ChartCanvas * cc1;
extern bool g_bopengl;

MyPrintout::MyPrintout(const wxChar * title)
	: wxPrintout(title)
{}

bool MyPrintout::OnPrintPage(int page)
{
	wxDC *dc = GetDC();
	if( dc ) {
		if( page == 1 )
			DrawPageOne(dc);
		return true;
	} else
		return false;
}

bool MyPrintout::OnBeginDocument(int startPage, int endPage)
{
	if (!wxPrintout::OnBeginDocument(startPage, endPage))
		return false;

	return true;
}

void MyPrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
	*minPage = 1;
	*maxPage = 1;
	*selPageFrom = 1;
	*selPageTo = 1;
}

bool MyPrintout::HasPage(int pageNum)
{
	return pageNum == 1;
}

void MyPrintout::DrawPageOne(wxDC *dc)
{
	// Get the Size of the Chart Canvas
	int sx;
	int sy;
	cc1->GetClientSize( &sx, &sy );

	float maxX = sx;
	float maxY = sy;

	// Let's have at least some device units margin
	float marginX = 50;
	float marginY = 50;

	// Add the margin to the graphic size
	maxX += ( 2 * marginX );
	maxY += ( 2 * marginY );

	// Get the size of the DC in pixels
	int w, h;
	dc->GetSize( &w, &h );

	// Calculate a suitable scaling factor
	float scaleX = (float) ( w / maxX );
	float scaleY = (float) ( h / maxY );

	// Use x or y scaling factor, whichever fits on the DC
	float actualScale = wxMin(scaleX,scaleY);

	// Calculate the position on the DC for centring the graphic
	float posX = (float) ( ( w - ( maxX * actualScale ) ) / 2.0 );
	float posY = (float) ( ( h - ( maxY * actualScale ) ) / 2.0 );

	posX = wxMax(posX, marginX);
	posY = wxMax(posY, marginY);

	// Set the scale and origin
	dc->SetUserScale( actualScale, actualScale );
	dc->SetDeviceOrigin( (long) posX, (long) posY );

	//  Get the latest bitmap as rendered by the ChartCanvas

	if(g_bopengl) {
#ifdef ocpnUSE_GL
		int gsx = cc1->GetglCanvas()->GetSize().x;
		int gsy = cc1->GetglCanvas()->GetSize().y;

		unsigned char *buffer = (unsigned char *)malloc( gsx * gsy * 3 );
		glReadPixels(0, 0, gsx, gsy, GL_RGB, GL_UNSIGNED_BYTE, buffer );
		wxImage image( gsx,gsy );
		image.SetData(buffer);
		wxImage mir_imag = image.Mirror( false );
		wxBitmap bmp( mir_imag );
		wxMemoryDC mdc;
		mdc.SelectObject( bmp );
		dc->Blit( 0, 0, bmp.GetWidth(), bmp.GetHeight(), &mdc, 0, 0 );
		mdc.SelectObject( wxNullBitmap );
#endif
	}
	else {

		//  And Blit/scale it onto the Printer DC
		wxMemoryDC mdc;
		mdc.SelectObject( *( cc1->pscratch_bm ) );

		dc->Blit( 0, 0, cc1->pscratch_bm->GetWidth(), cc1->pscratch_bm->GetHeight(), &mdc, 0, 0 );

		mdc.SelectObject( wxNullBitmap );
	}
}

