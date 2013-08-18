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

#include "ToolTipWin.h"
#include "FontMgr.h"

BEGIN_EVENT_TABLE(ToolTipWin, wxDialog)
	EVT_PAINT(ToolTipWin::OnPaint)
END_EVENT_TABLE()

// Define a constructor
ToolTipWin::ToolTipWin( wxWindow *parent )
	: wxDialog(parent, wxID_ANY, _T(""), wxPoint( 0, 0 ), wxSize( 1, 1 ), wxNO_BORDER | wxSTAY_ON_TOP)
{
	m_pbm = NULL;

	m_back_color = GetGlobalColor(_T("UIBCK"));
	m_text_color = GetGlobalColor(_T("UITX1"));

	SetBackgroundStyle( wxBG_STYLE_CUSTOM );
	SetBackgroundColour( m_back_color );
	Hide();
}

ToolTipWin::~ToolTipWin()
{
	delete m_pbm;
}

void ToolTipWin::SetColorScheme( ColorScheme cs )
{
	m_back_color = GetGlobalColor( _T ( "UIBCK" ) );
	m_text_color = FontMgr::Get().GetFontColor( _("ToolTips") );
}

void ToolTipWin::SetBitmap()
{
	int h, w;

	wxClientDC cdc( GetParent() );

	wxFont *plabelFont = FontMgr::Get().GetFont( _("ToolTips") );
	cdc.GetTextExtent( m_string, &w, &h, NULL, NULL, plabelFont );

	m_size.x = w + 8;
	m_size.y = h + 4;

	wxMemoryDC mdc;

	delete m_pbm;
	m_pbm = new wxBitmap( m_size.x, m_size.y, -1 );
	mdc.SelectObject( *m_pbm );

	wxPen pborder( m_text_color );
	wxBrush bback( m_back_color );
	mdc.SetPen( pborder );
	mdc.SetBrush( bback );

	mdc.DrawRectangle( 0, 0, m_size.x, m_size.y );

	//    Draw the text
	mdc.SetFont( *plabelFont );
	mdc.SetTextForeground( m_text_color );
	mdc.SetTextBackground( m_back_color );

	mdc.DrawText( m_string, 4, 2 );

	int parent_width;
	cdc.GetSize( &parent_width, NULL );
	SetSize( m_position.x, m_position.y, m_size.x, m_size.y );

}

void ToolTipWin::OnPaint( wxPaintEvent& event )
{
	int width, height;
	GetClientSize( &width, &height );
	wxPaintDC dc( this );

	if( m_string.Len() ) {
        wxMemoryDC mdc;
        mdc.SelectObject( *m_pbm );
        dc.Blit( 0, 0, width, height, &mdc, 0, 0 );
    }
}
