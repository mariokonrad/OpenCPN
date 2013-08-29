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

#ifndef __CHART__CM93OFFSETLISTCTRL__H__
#define __CHART__CM93OFFSETLISTCTRL__H__

#include <wx/dialog.h>
#include <wx/string.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <chart/M_COVR_Desc.h>

class wxButton;
class wxSpinCtrl;
class OCPNOffsetListCtrl;
class ViewPort;
class cm93compchart;

class CM93OffsetDialog : public wxDialog
{
		DECLARE_CLASS( CM93OffsetDialog )
		DECLARE_EVENT_TABLE()

	public:
		CM93OffsetDialog( wxWindow * parent, cm93compchart * pchart);
		virtual ~CM93OffsetDialog();

		void OnClose(wxCloseEvent & event);
		void OnOK(wxCommandEvent & event);

		void SetColorScheme();
		void UpdateMCOVRList(const ViewPort & vpt);     // Rebuild MCOVR list

		OCPNOffsetListCtrl * m_pListCtrlMCOVRs;
		Array_Of_M_COVR_Desc_Ptr m_pcovr_array;

		wxString m_selected_chart_scale_char;

	private:
		void OnCellSelected(wxListEvent & event);
		void OnOffSetSet(wxCommandEvent & event);

		void UpdateOffsets(void);

		wxSpinCtrl * m_pSpinCtrlXoff;
		wxSpinCtrl * m_pSpinCtrlYoff;
		wxButton * m_OKButton;

		wxWindow * m_pparent; // FIXME: redundant?
		cm93compchart * m_pcompchart;

		int m_xoff;
		int m_yoff;
		int m_selected_cell_index;
		int m_selected_object_id;
		int m_selected_subcell;
		int m_selected_list_index;
};

#endif
