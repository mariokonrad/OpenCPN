/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 * Copyright (C) 2010 by David S. Register                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the                           *
 * Free Software Foundation, Inc.,                                         *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.           *
 **************************************************************************/

#include "DimeControl.h"
#include <ChartCanvas.h> // FIXME: meh...
#include <UserColors.h>

#include <wx/bmpcbox.h>
#include <wx/listbook.h>
#include <wx/listctrl.h>
#include <wx/tglbtn.h>
#include <wx/radiobut.h>
#include <wx/button.h>

extern ChartCanvas * cc1;

void DimeControl(wxWindow * ctrl)
{
	if (NULL == ctrl)
		return;

	wxColour col = GetGlobalColor(_T("DILG0"));       // Dialog Background white
	wxColour col1 = GetGlobalColor(_T("DILG1"));      // Dialog Background
	wxColour back_color = GetGlobalColor(_T("DILG1"));      // Control Background
	wxColour text_color = GetGlobalColor(_T("DILG3"));      // Text
	wxColour uitext = GetGlobalColor(_T("UITX1"));    // Menu Text, derived from UINFF
	wxColour udkrd = GetGlobalColor(_T("UDKRD"));
	wxColour gridline = GetGlobalColor(_T("GREY2"));

	DimeControl(ctrl, col, col1, back_color, text_color, uitext, udkrd, gridline);
}

void DimeControl(
		wxWindow * ctrl,
		wxColour col,
		wxColour col1,
		wxColour back_color,
		wxColour text_color,
		wxColour uitext,
		wxColour udkrd,
		wxColour gridline)
{
	ColorScheme cs = cc1->GetColorScheme(); // FIXME: is ChartCanvas really needed here? MainFrame vs. ChartCanvas GetColorScheme??

	if (cs != GLOBAL_COLOR_SCHEME_DAY && cs != GLOBAL_COLOR_SCHEME_RGB)
		ctrl->SetBackgroundColour(back_color);
	else
		ctrl->SetBackgroundColour(wxNullColour);

#ifdef __WXMAC__
#if wxCHECK_VERSION(2,9,0)
	if( cs != GLOBAL_COLOR_SCHEME_DAY && cs != GLOBAL_COLOR_SCHEME_RGB )
		ctrl->SetBackgroundColour( back_color );
	else
		ctrl->SetBackgroundColour( wxColour( 0xff, 0xff, 0xff ));
#endif
#endif

	wxWindowList kids = ctrl->GetChildren();
	for (unsigned int i = 0; i < kids.GetCount(); ++i) {
		wxWindowListNode *node = kids.Item(i);
		wxWindow *win = node->GetData();

		if( win->IsKindOf( CLASSINFO(wxListBox) ) )
			( (wxListBox*) win )->SetBackgroundColour( col );

		if( win->IsKindOf( CLASSINFO(wxListCtrl) ) )
			( (wxListCtrl*) win )->SetBackgroundColour( col1 );

		if( win->IsKindOf( CLASSINFO(wxTextCtrl) ) )
			( (wxTextCtrl*) win )->SetBackgroundColour( col );

		if( win->IsKindOf( CLASSINFO(wxStaticText) ) )
			( (wxStaticText*) win )->SetForegroundColour( uitext );

		else if( win->IsKindOf( CLASSINFO(wxBitmapComboBox) ) ) {
#if wxCHECK_VERSION(2,9,0)
			if( ( ( wxBitmapComboBox*) win )->GetTextCtrl() )
				( (wxBitmapComboBox*) win )->GetTextCtrl()->SetBackgroundColour(col);
#else
			( (wxBitmapComboBox*) win )->SetBackgroundColour( col );
#endif
		}

		else if( win->IsKindOf( CLASSINFO(wxChoice) ) )
			( (wxChoice*) win )->SetBackgroundColour( col );

		else if( win->IsKindOf( CLASSINFO(wxComboBox) ) )
			( (wxComboBox*) win )->SetBackgroundColour( col );

		else if( win->IsKindOf( CLASSINFO(wxScrolledWindow) ) )
			( (wxScrolledWindow*) win )->SetBackgroundColour( col1 );

		else if( win->IsKindOf( CLASSINFO(wxGenericDirCtrl) ) )
			( (wxGenericDirCtrl*) win )->SetBackgroundColour( col1 );

		else if( win->IsKindOf( CLASSINFO(wxListbook) ) )
			( (wxListbook*) win )->SetBackgroundColour( col1 );

		else if( win->IsKindOf( CLASSINFO(wxTreeCtrl) ) )
			( (wxTreeCtrl*) win )->SetBackgroundColour( col );

		else if( win->IsKindOf( CLASSINFO(wxRadioButton) ) )
			( (wxRadioButton*) win )->SetBackgroundColour( col1 );

		else if( win->IsKindOf( CLASSINFO(wxNotebook) ) ) {
			( (wxNotebook*) win )->SetBackgroundColour( col1 );
			( (wxNotebook*) win )->SetForegroundColour( text_color );
		}

		else if( win->IsKindOf( CLASSINFO(wxButton) ) ) {
			( (wxButton*) win )->SetBackgroundColour( col1 );
		}

		else if( win->IsKindOf( CLASSINFO(wxToggleButton) ) ) {
			( (wxToggleButton*) win )->SetBackgroundColour( col1 );
		}

		else if( win->IsKindOf( CLASSINFO(wxPanel) ) ) {
			//                  ((wxPanel*)win)->SetBackgroundColour(col1);
			if( cs != GLOBAL_COLOR_SCHEME_DAY
					&& cs != GLOBAL_COLOR_SCHEME_RGB ) ( (wxPanel*) win )->SetBackgroundColour(
						back_color );
			else
				( (wxPanel*) win )->SetBackgroundColour(
						wxNullColour );
		}

		else if( win->IsKindOf( CLASSINFO(wxHtmlWindow) ) ) {
			if( cs != GLOBAL_COLOR_SCHEME_DAY
					&& cs != GLOBAL_COLOR_SCHEME_RGB ) ( (wxPanel*) win )->SetBackgroundColour(
						back_color );
			else
				( (wxPanel*) win )->SetBackgroundColour(
						wxNullColour );

		}

		else if( win->IsKindOf( CLASSINFO(wxGrid) ) ) {
			( (wxGrid*) win )->SetDefaultCellBackgroundColour(
					col1 );
			( (wxGrid*) win )->SetDefaultCellTextColour(
					uitext );
			( (wxGrid*) win )->SetLabelBackgroundColour(
					col );
			( (wxGrid*) win )->SetLabelTextColour(
					uitext );
			( (wxGrid*) win )->SetDividerPen(
					wxPen( col ) );
			( (wxGrid*) win )->SetGridLineColour(
					gridline );
		}

		else {
			;
		}

		if (win->GetChildren().GetCount() > 0) {
			wxWindow * w = win;
			DimeControl(w, col, col1, back_color, text_color, uitext, udkrd, gridline);
		}
	}
}

