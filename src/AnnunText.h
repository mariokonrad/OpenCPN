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

#ifndef __ANNUNTEXT__H__
#define __ANNUNTEXT__H__

#include <wx/window.h>
#include <ColorScheme.h>

class AnnunText : public wxWindow
{
		DECLARE_EVENT_TABLE()

	public:
		AnnunText(
				wxWindow * parent,
				wxWindowID id,
				const wxString & LegendElement,
				const wxString & ValueElement);

		virtual ~AnnunText();

		void SetALabel(const wxString & l);
		void SetAValue(const wxString & v);
		void OnPaint(wxPaintEvent & event);
		void RefreshFonts(void);
		void SetLegendElement(const wxString & element);
		void SetValueElement(const wxString & element);
		void SetColorScheme(ColorScheme cs);

	private:
		void CalculateMinSize(void);

		wxBrush * m_pbackBrush;
		wxColour m_text_color;

		wxString m_label;
		wxString m_value;
		wxFont * m_plabelFont;
		wxFont * m_pvalueFont;

		wxString m_LegendTextElement;
		wxString m_ValueTextElement;
};

#endif
