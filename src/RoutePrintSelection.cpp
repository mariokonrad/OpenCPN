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

#include "RoutePrintSelection.h"
#include "RoutePrintout.h"
#include "MessageBox.h"

#include <DimeControl.h>

#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>

// Global print data, to remember settings during the session
extern wxPrintData * g_printData;

// Global page setup data
extern wxPageSetupData * g_pageSetupData;

IMPLEMENT_DYNAMIC_CLASS(RoutePrintSelection, wxDialog)

BEGIN_EVENT_TABLE(RoutePrintSelection, wxDialog)
	EVT_BUTTON(ID_ROUTEPRINT_SELECTION_CANCEL, RoutePrintSelection::OnRoutepropCancelClick)
	EVT_BUTTON(ID_ROUTEPRINT_SELECTION_OK, RoutePrintSelection::OnRoutepropOkClick)
END_EVENT_TABLE()

RoutePrintSelection::RoutePrintSelection()
{
}

RoutePrintSelection::RoutePrintSelection(
		wxWindow * parent,
		Route * _route,
		wxWindowID id,
		const wxString & caption,
		const wxPoint & pos,
		const wxSize & size,
		long style)
{
	route = _route;

	long wstyle = style;

	Create( parent, id, caption, pos, size, wstyle );
	Centre();
}


RoutePrintSelection::~RoutePrintSelection()
{
}

bool RoutePrintSelection::Create(
		wxWindow * parent,
		wxWindowID id,
		const wxString &,
		const wxPoint & pos,
		const wxSize & size,
		long style)
{
	SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );

#ifdef __WXOSX__
	style |= wxSTAY_ON_TOP;
#endif

	wxDialog::Create( parent, id, _("Print Route Selection"), pos, size, style );
	CreateControls();
	return TRUE;
}

void RoutePrintSelection::CreateControls()
{
	RoutePrintSelection* itemDialog1 = this;

	wxStaticBox * itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Elements to print..."));

	wxStaticBoxSizer* itemBoxSizer1 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
	itemDialog1->SetSizer(itemBoxSizer1);

	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 5, 2, 0, 0 );

	m_checkBoxWPName = new wxCheckBox( itemDialog1, wxID_ANY, _( "Name" ),
			wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_checkBoxWPName->SetValue( true );
	fgSizer2->Add( m_checkBoxWPName, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText* label1 = new  wxStaticText( itemDialog1, wxID_ANY, _( "Show Waypoint name." ), wxDefaultPosition, wxDefaultSize );
	fgSizer2->Add( label1, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5 );

	m_checkBoxWPPosition = new wxCheckBox( itemDialog1, wxID_ANY, _( "Position" ),
			wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_checkBoxWPPosition->SetValue( true );
	fgSizer2->Add( m_checkBoxWPPosition, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );
	wxStaticText* label2 = new  wxStaticText( itemDialog1, wxID_ANY, _( "Show Waypoint position." ), wxDefaultPosition, wxDefaultSize );
	fgSizer2->Add( label2, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5 );

	m_checkBoxWPCourse = new wxCheckBox( itemDialog1, wxID_ANY, _( "Course" ),
			wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_checkBoxWPCourse->SetValue( true );
	fgSizer2->Add( m_checkBoxWPCourse, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );
	wxStaticText* label3 = new  wxStaticText( itemDialog1, wxID_ANY, _( "Show course from each Waypoint to the next one. " ), wxDefaultPosition, wxDefaultSize );
	fgSizer2->Add( label3, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5 );

	m_checkBoxWPDistanceToNext = new wxCheckBox( itemDialog1, wxID_ANY, _( "Distance" ),
			wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_checkBoxWPDistanceToNext->SetValue( true );
	fgSizer2->Add( m_checkBoxWPDistanceToNext, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );
	wxStaticText* label4 = new  wxStaticText( itemDialog1, wxID_ANY, _( "Show Distance from each Waypoint to the next one." ), wxDefaultPosition, wxDefaultSize );
	fgSizer2->Add( label4, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5 );

	m_checkBoxWPDescription = new wxCheckBox( itemDialog1, wxID_ANY, _( "Description" ),
			wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	m_checkBoxWPDescription->SetValue( true );
	fgSizer2->Add( m_checkBoxWPDescription, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5 );
	wxStaticText* label5 = new  wxStaticText( itemDialog1, wxID_ANY, _( "Show Waypoint description." ), wxDefaultPosition, wxDefaultSize );
	fgSizer2->Add( label5, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5 );

	itemBoxSizer1->Add( fgSizer2, 5, wxEXPAND, 5 );

	wxBoxSizer* itemBoxSizer16 = new wxBoxSizer( wxHORIZONTAL );
	itemBoxSizer1->Add( itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5 );

	m_CancelButton = new wxButton( itemDialog1, ID_ROUTEPRINT_SELECTION_CANCEL, _( "Cancel" ), wxDefaultPosition,
			wxDefaultSize, 0 );
	itemBoxSizer16->Add( m_CancelButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );

	m_OKButton = new wxButton( itemDialog1, ID_ROUTEPRINT_SELECTION_OK, _( "OK" ), wxDefaultPosition,
			wxDefaultSize, 0 );
	itemBoxSizer16->Add( m_OKButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );
	m_OKButton->SetDefault();

	SetColorScheme( ( ColorScheme )0 );
}

void RoutePrintSelection::SetColorScheme(ColorScheme)
{
	DimeControl(this);
}

bool RoutePrintSelection::ShowToolTips()
{
	return TRUE;
}

void RoutePrintSelection::SetDialogTitle(const wxString & title)
{
	SetTitle(title);
}

void RoutePrintSelection::OnRoutepropCancelClick( wxCommandEvent& event )
{
	Hide();
	event.Skip();
}

void RoutePrintSelection::OnRoutepropOkClick( wxCommandEvent& event )
{
	std::vector<bool> toPrintOut;
	toPrintOut.push_back( m_checkBoxWPName->GetValue() );
	toPrintOut.push_back( m_checkBoxWPPosition->GetValue() );
	toPrintOut.push_back( m_checkBoxWPCourse->GetValue() );
	toPrintOut.push_back( m_checkBoxWPDistanceToNext->GetValue() );
	toPrintOut.push_back( m_checkBoxWPDescription->GetValue() );

	if ( NULL == g_printData ) {
		g_printData = new wxPrintData;
		g_printData->SetOrientation( wxLANDSCAPE );
		g_pageSetupData = new wxPageSetupDialogData;
	}

	RoutePrintout*  myrouteprintout1 = new RoutePrintout( toPrintOut, route,  _( "Route Print" ) );

	wxPrintDialogData printDialogData( *g_printData );
	printDialogData.EnablePageNumbers( true );

	wxPrinter printer( &printDialogData );
	if ( !printer.Print( this, myrouteprintout1, true ) ) {
		if ( wxPrinter::GetLastError() == wxPRINTER_ERROR ) {
			OCPNMessageBox(
					NULL,
					_( "There was a problem printing.\nPerhaps your current printer is not set correctly?" ),
					_T( "OpenCPN" ), wxOK );
		}
	}

	Hide();
	event.Skip();
}

