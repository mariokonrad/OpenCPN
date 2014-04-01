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

#include <global/OCPN.h>
#include <global/GUI.h>
#include <global/ColorManager.h>

#include <wx/bmpcbox.h>
#include <wx/listbook.h>
#include <wx/listctrl.h>
#include <wx/tglbtn.h>
#include <wx/radiobut.h>
#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/grid.h>
#include <wx/treectrl.h>
#include <wx/dirctrl.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/pen.h>
#include <wx/html/htmlwin.h>

void DimeControl(wxWindow* ctrl)
{
	if (NULL == ctrl)
		return;

	const global::ColorManager& colors = global::OCPN::get().color();
	wxColour col = colors.get_color(_T("DILG0")); // Dialog Background white
	wxColour col1 = colors.get_color(_T("DILG1")); // Dialog Background
	wxColour back_color = colors.get_color(_T("DILG1")); // Control Background
	wxColour text_color = colors.get_color(_T("DILG3")); // Text
	wxColour uitext = colors.get_color(_T("UITX1")); // Menu Text, derived from UINFF
	wxColour udkrd = colors.get_color(_T("UDKRD"));
	wxColour gridline = colors.get_color(_T("GREY2"));

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
	global::ColorScheme cs = global::OCPN::get().gui().view().color_scheme;

	if (cs != global::GLOBAL_COLOR_SCHEME_DAY && cs != global::GLOBAL_COLOR_SCHEME_RGB)
		ctrl->SetBackgroundColour(back_color);
	else
		ctrl->SetBackgroundColour(wxNullColour);

#ifdef __WXMAC__
#if wxCHECK_VERSION(2, 9, 0)
	if (cs != global::GLOBAL_COLOR_SCHEME_DAY && cs != global::GLOBAL_COLOR_SCHEME_RGB)
		ctrl->SetBackgroundColour(back_color);
	else
		ctrl->SetBackgroundColour(wxColour(0xff, 0xff, 0xff));
#endif
#endif

	wxWindowList kids = ctrl->GetChildren();
	for (unsigned int i = 0; i < kids.size(); ++i) {
		wxWindowListNode* node = kids.Item(i);
		wxWindow* win = node->GetData();

		if (win->IsKindOf(CLASSINFO(wxListBox)))
			((wxListBox*)win)->SetBackgroundColour(col);

		if (win->IsKindOf(CLASSINFO(wxListCtrl)))
			((wxListCtrl*)win)->SetBackgroundColour(col1);

		if (win->IsKindOf(CLASSINFO(wxTextCtrl)))
			((wxTextCtrl*)win)->SetBackgroundColour(col);

		if (win->IsKindOf(CLASSINFO(wxStaticText)))
			((wxStaticText*)win)->SetForegroundColour(uitext);

		else if (win->IsKindOf(CLASSINFO(wxBitmapComboBox))) {
#if wxCHECK_VERSION(2, 9, 0) && !wxCHECK_VERSION(3, 0, 0)
			if (((wxBitmapComboBox*)win)->GetTextCtrl())
				((wxBitmapComboBox*)win)->GetTextCtrl()->SetBackgroundColour(col);
#else
			((wxBitmapComboBox*)win)->SetBackgroundColour(col);
#endif
		} else if (win->IsKindOf(CLASSINFO(wxChoice)))
			((wxChoice*)win)->SetBackgroundColour(col);

		else if (win->IsKindOf(CLASSINFO(wxComboBox)))
			((wxComboBox*)win)->SetBackgroundColour(col);

		else if (win->IsKindOf(CLASSINFO(wxScrolledWindow)))
			((wxScrolledWindow*)win)->SetBackgroundColour(col1);

		else if (win->IsKindOf(CLASSINFO(wxGenericDirCtrl)))
			((wxGenericDirCtrl*)win)->SetBackgroundColour(col1);

		else if (win->IsKindOf(CLASSINFO(wxListbook)))
			((wxListbook*)win)->SetBackgroundColour(col1);

		else if (win->IsKindOf(CLASSINFO(wxTreeCtrl)))
			((wxTreeCtrl*)win)->SetBackgroundColour(col);

		else if (win->IsKindOf(CLASSINFO(wxRadioButton)))
			((wxRadioButton*)win)->SetBackgroundColour(col1);

		else if (win->IsKindOf(CLASSINFO(wxNotebook))) {
			((wxNotebook*)win)->SetBackgroundColour(col1);
			((wxNotebook*)win)->SetForegroundColour(text_color);
		} else if (win->IsKindOf(CLASSINFO(wxButton))) {
			((wxButton*)win)->SetBackgroundColour(col1);
		} else if (win->IsKindOf(CLASSINFO(wxToggleButton))) {
			((wxToggleButton*)win)->SetBackgroundColour(col1);
		} else if (win->IsKindOf(CLASSINFO(wxPanel))) {
			if (cs != global::GLOBAL_COLOR_SCHEME_DAY && cs != global::GLOBAL_COLOR_SCHEME_RGB)
				((wxPanel*)win)->SetBackgroundColour(back_color);
			else
				((wxPanel*)win)->SetBackgroundColour(wxNullColour);
		} else if (win->IsKindOf(CLASSINFO(wxHtmlWindow))) {
			if (cs != global::GLOBAL_COLOR_SCHEME_DAY && cs != global::GLOBAL_COLOR_SCHEME_RGB)
				((wxPanel*)win)->SetBackgroundColour(back_color);
			else
				((wxPanel*)win)->SetBackgroundColour(wxNullColour);

		} else if (win->IsKindOf(CLASSINFO(wxGrid))) {
			wxGrid* grid = static_cast<wxGrid*>(win);
			grid->SetDefaultCellBackgroundColour(col1);
			grid->SetDefaultCellTextColour(uitext);
			grid->SetLabelBackgroundColour(col);
			grid->SetLabelTextColour(uitext);
			grid->SetDividerPen(wxPen(col));
			grid->SetGridLineColour(gridline);
		}

		if (win->GetChildren().size() > 0) {
			wxWindow* w = win;
			DimeControl(w, col, col1, back_color, text_color, uitext, udkrd, gridline);
		}
	}
}

