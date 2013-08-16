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

#include "ToolbarMOBDialog.h"
#include <wx/sizer.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>

ToolbarMOBDialog::ToolbarMOBDialog(wxWindow * parent)
	: wxDialog(parent, wxID_ANY, _("OpenCPN Alert"), wxDefaultPosition, wxSize(250, 230))
{
	wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* sizer = new wxBoxSizer( wxVERTICAL );
	topSizer->Add( sizer, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );

	choices.push_back( new wxRadioButton( this, 0,  _("No, I don't want to hide it."),
				wxDefaultPosition, wxDefaultSize, wxRB_GROUP ) );

	choices.push_back( new wxRadioButton( this, 1, _("No, and permanently remove the option to hide it."),
				wxDefaultPosition) );

	choices.push_back( new wxRadioButton( this, 2, _("Yes, hide it."),
				wxDefaultPosition) );

	wxStdDialogButtonSizer* buttonSizer = CreateStdDialogButtonSizer( wxOK | wxCANCEL );


	wxStaticText* textCtrl = new wxStaticText( this, wxID_ANY, _("The Man Over Board button could be an important safety feature.\nAre you sure you want to hide it?") );

	sizer->Add( textCtrl, 0, wxEXPAND | wxALL, 5 );
	sizer->Add( choices[0], 0, wxEXPAND | wxALL, 5 );
	sizer->Add( choices[1], 0, wxEXPAND | wxALL, 5 );
	sizer->Add( choices[2], 0, wxEXPAND | wxALL, 5 );
	sizer->Add( buttonSizer, 0, wxEXPAND | wxTOP, 5 );

	topSizer->SetSizeHints(this);
	SetSizer( topSizer );
}

int ToolbarMOBDialog::GetSelection() const
{
	for( unsigned int i=0; i<choices.size(); i++ ) {
		if( choices[i]->GetValue() ) return choices[i]->GetId();
	}
	return 0;
}

