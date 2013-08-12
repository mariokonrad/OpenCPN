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

#include "OCPN_AlertDialog.h"
#include "ocpn_types.h"

extern ColorScheme global_color_scheme;
extern bool g_bopengl;

IMPLEMENT_CLASS(OCPN_AlertDialog, wxDialog)

BEGIN_EVENT_TABLE(OCPN_AlertDialog, wxDialog)
END_EVENT_TABLE()

OCPN_AlertDialog::OCPN_AlertDialog()
{
    Init();
}

OCPN_AlertDialog::~OCPN_AlertDialog()
{
}

void OCPN_AlertDialog::Init(void)
{
    m_pparent = NULL;
}

bool OCPN_AlertDialog::Create(
		wxWindow * parent,
		wxWindowID id,
		const wxString & caption,
		const wxPoint & pos,
		const wxSize & size,
		long style)
{
    //    As a display optimization....
    //    if current color scheme is other than DAY,
    //    Then create the dialog ..WITHOUT.. borders and title bar.
    //    This way, any window decorations set by external themes, etc
    //    will not detract from night-vision

    long wstyle = wxDEFAULT_FRAME_STYLE;
    if( ( global_color_scheme != GLOBAL_COLOR_SCHEME_DAY )
        && ( global_color_scheme != GLOBAL_COLOR_SCHEME_RGB ) ) wstyle |= ( wxNO_BORDER );

    wxSize size_min = size;
    size_min.IncTo( wxSize( 500, 600 ) );
    if( !wxDialog::Create( parent, id, caption, pos, size_min, wstyle ) ) return false;

    m_pparent = parent;

    if( !g_bopengl && CanSetTransparent() ) SetTransparent( 192 );

    return true;
}

