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

#include "CM93OffsetDialog.h"
#include <chart/CM93compchart.h>
#include <chart/CM93Chart.h>
#include <chart/OCPNOffsetListCtrl.h>
#include <chart/COVR_Set.h>
#include <ChartCanvas.h>
#include <wx/sizer.h>
#include <wx/button.h>

IMPLEMENT_CLASS(CM93OffsetDialog, wxDialog)

BEGIN_EVENT_TABLE(CM93OffsetDialog, wxDialog)
	EVT_CLOSE(CM93OffsetDialog::OnClose)
END_EVENT_TABLE()


CM93OffsetDialog::CM93OffsetDialog ( wxWindow *parent, cm93compchart *pchart )
{
	m_pparent = parent;
	m_pcompchart = pchart;

	if ( m_pcompchart )
		m_pcompchart->SetOffsetDialog ( this );


	m_xoff = 0;
	m_yoff = 0;

	m_selected_list_index = -1;

	long wstyle = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;
	wxDialog::Create ( parent, -1, _ ( "OpenCPN CM93 Cell Offset Adjustments" ), wxPoint ( 0, 0 ), wxSize ( 800, 200 ), wstyle );

	// A top-level sizer
	wxBoxSizer* topSizer = new wxBoxSizer ( wxHORIZONTAL );
	SetSizer ( topSizer );

	int width;

	m_pListCtrlMCOVRs = new OCPNOffsetListCtrl ( this, -1, wxDefaultPosition, wxDefaultSize,
			wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_HRULES|wxLC_VRULES|wxBORDER_SUNKEN|wxLC_VIRTUAL );

	m_pListCtrlMCOVRs->Connect ( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler ( CM93OffsetDialog::OnCellSelected ), NULL, this );

	width = 80;
	m_pListCtrlMCOVRs->InsertColumn (OCPNOffsetListCtrl::tlCELL, _ ( "Cell" ), wxLIST_FORMAT_LEFT, width );

	width = 80;
	m_pListCtrlMCOVRs->InsertColumn (OCPNOffsetListCtrl::tlMCOVR, _ ( "M_COVR ID" ), wxLIST_FORMAT_CENTER, width );

	width = 80;
	m_pListCtrlMCOVRs->InsertColumn (OCPNOffsetListCtrl::tlSCALE, _ ( "Cell Scale" ), wxLIST_FORMAT_CENTER, width );

	width = 90;
	m_pListCtrlMCOVRs->InsertColumn (OCPNOffsetListCtrl::tlXOFF, _ ( "wgsox" ), wxLIST_FORMAT_CENTER, width );

	width = 90;
	m_pListCtrlMCOVRs->InsertColumn (OCPNOffsetListCtrl::tlYOFF, _ ( "wgsoy" ), wxLIST_FORMAT_CENTER, width );

	width = 90;
	m_pListCtrlMCOVRs->InsertColumn (OCPNOffsetListCtrl::tlUXOFF, _ ( "User X Offset" ), wxLIST_FORMAT_CENTER, width );

	width = 90;
	m_pListCtrlMCOVRs->InsertColumn (OCPNOffsetListCtrl::tlUYOFF, _ ( "User Y Offset" ), wxLIST_FORMAT_CENTER, width );


	topSizer->Add ( m_pListCtrlMCOVRs, 1, wxEXPAND|wxALL, 0 );

	wxBoxSizer* boxSizer02 = new wxBoxSizer ( wxVERTICAL );
	boxSizer02->AddSpacer ( 22 );

	wxStaticText *pStaticTextXoff = new wxStaticText ( this, wxID_ANY, _ ( "User X Offset (Metres)" ), wxDefaultPosition, wxDefaultSize, 0 );
	boxSizer02->Add ( pStaticTextXoff, 0, wxALL, 0 );

	m_pSpinCtrlXoff = new wxSpinCtrl ( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize ( 50, -1 ), wxSP_ARROW_KEYS, -10000, 10000, 0 );
	m_pSpinCtrlXoff->Connect ( wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler ( CM93OffsetDialog::OnOffSetSet ), NULL, this );
	boxSizer02->Add ( m_pSpinCtrlXoff, 0, wxEXPAND|wxALL, 0 );

	wxStaticText *pStaticTextYoff = new wxStaticText ( this, wxID_ANY, _ ( "User Y Offset (Metres)" ), wxDefaultPosition, wxDefaultSize, 0 );
	boxSizer02->Add ( pStaticTextYoff, 0, wxALL, 0 );

	m_pSpinCtrlYoff = new wxSpinCtrl ( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize ( 50, -1 ), wxSP_ARROW_KEYS, -10000, 10000, 0 );
	m_pSpinCtrlYoff->Connect ( wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler ( CM93OffsetDialog::OnOffSetSet ), NULL, this );
	boxSizer02->Add ( m_pSpinCtrlYoff, 0, wxEXPAND|wxALL, 0 );

	m_OKButton = new wxButton ( this, wxID_ANY, _ ( "OK" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_OKButton->Connect ( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler ( CM93OffsetDialog::OnOK ), NULL, this );
	boxSizer02->Add ( m_OKButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	m_OKButton->SetDefault();

	topSizer->Add ( boxSizer02, 0, wxEXPAND|wxALL, 2 );
	topSizer->Layout();

	//    This is silly, but seems to be required for __WXMSW__ build
	//    If not done, the SECOND invocation of dialog fails to expand the list to the full wxSizer size....
	SetSize ( GetSize().x, GetSize().y-1 );

	SetColorScheme();

	//      GetSizer()->SetSizeHints(this);
	Centre();
}


CM93OffsetDialog::~CM93OffsetDialog( )
{
}

void CM93OffsetDialog::OnClose(wxCloseEvent &)
{
	if ( m_pcompchart )
	{
		m_pcompchart->SetSpecialOutlineCellIndex ( 0, 0, 0 );

		m_pcompchart->InvalidateCache();

		if ( m_pparent )
			m_pparent->Refresh ( true );
	}

	if ( m_pListCtrlMCOVRs->GetItemCount() > m_selected_list_index )
		m_pListCtrlMCOVRs->SetItemState ( m_selected_list_index, 0, wxLIST_STATE_SELECTED );

	Hide();
}


void CM93OffsetDialog::OnOK(wxCommandEvent &)
{
	Close();
}


void CM93OffsetDialog::OnOffSetSet(wxCommandEvent &)
{
	m_xoff = m_pSpinCtrlXoff->GetValue();
	m_yoff = m_pSpinCtrlYoff->GetValue();

	UpdateOffsets();

}

void CM93OffsetDialog::UpdateOffsets ( void )
{
	if ( m_pcompchart )
	{
		//    Set the offsets of the selected cell/object
		m_pcompchart->SetSpecialCellIndexOffset ( m_selected_cell_index, m_selected_object_id, m_selected_subcell, m_xoff, m_yoff );

		//    Closing the current cell will record the offsets in the M_COVR cache file
		//    Re-opening will then refresh the M_COVRs in the cover set
		::wxBeginBusyCursor();
		m_pcompchart->CloseandReopenCurrentSubchart();
		::wxEndBusyCursor();

		if ( m_pparent )
			m_pparent->Refresh ( true );
	}
}

void CM93OffsetDialog::SetColorScheme()
{
	DimeControl(this);
}

void CM93OffsetDialog::OnCellSelected ( wxListEvent &event )
{
	m_selected_list_index = event.GetIndex();

	M_COVR_Desc *mcd =  m_pcovr_array.Item ( event.GetIndex() );

	if ( m_selected_list_index > m_pListCtrlMCOVRs->GetItemCount() )
		return;            // error

	cm93chart *pchart = m_pcompchart->GetCurrentSingleScaleChart();
	if ( pchart )
	{
		M_COVR_Desc * cached_mcd = pchart->GetCoverSet()->Find_MCD(mcd->m_cell_index, mcd->m_object_id, mcd->m_subcell);
		if ( cached_mcd )
		{
			m_pSpinCtrlXoff->SetValue ( wxRound ( cached_mcd->user_xoff ) );
			m_pSpinCtrlYoff->SetValue ( wxRound ( cached_mcd->user_yoff ) );
		}
	}

	m_pcompchart->SetSpecialOutlineCellIndex ( mcd->m_cell_index, mcd->m_object_id, mcd->m_subcell );

	m_selected_cell_index = mcd->m_cell_index;
	m_selected_object_id  = mcd->m_object_id;
	m_selected_subcell = mcd->m_subcell;

	m_pcompchart->InvalidateCache();

	if ( m_pparent )
		m_pparent->Refresh ( true );
}

void CM93OffsetDialog::UpdateMCOVRList ( const ViewPort &vpt )
{
	if ( m_pcompchart )
	{
		//    In single chart mode, there is but one cm93chart (i.e. one "scale value") shown at any one time
		cm93chart *pchart = m_pcompchart->GetCurrentSingleScaleChart();

		if ( pchart )
		{
			m_selected_chart_scale_char = pchart->GetScaleChar();

			m_pcovr_array.Clear();

			//    Get an array of cell indicies at the current viewport
			std::vector<int> cell_array = pchart->GetVPCellArray(vpt);

			ViewPort vp_positive;
			vp_positive = vpt;
			vp_positive.set_positive();

			//    Get the cover set for the cm93chart
			//    and walk the set looking for matches to the viewport referenced cell array
			//    This will give us the covr descriptors of interest
			covr_set *pcover = pchart->GetCoverSet();

			for ( unsigned int im=0 ; im < pcover->GetCoverCount() ; im++ )
			{
				M_COVR_Desc *mcd = pcover->GetCover ( im );

				for ( unsigned int icell=0 ; icell < cell_array.size(); icell++ )
				{
					if ( cell_array[icell] == mcd->m_cell_index )
					{
						wxPoint *pwp = pchart->GetDrawBuffer ( mcd->m_nvertices );
						OCPNRegion rgn = mcd->GetRegion ( vp_positive, pwp );

						//                                    if(_OUT != vp_positive.GetBBox().Intersect(mcd->m_covr_bbox))
						if ( rgn.Contains ( 0, 0, vpt.pix_width, vpt.pix_height ) != wxOutRegion )
							m_pcovr_array.Add ( mcd );
					}
				}
			}

			//    Try to find and maintain the correct list selection, even though the list contents may have changed
			int sel_index = -1;
			for ( unsigned int im=0 ; im < m_pcovr_array.GetCount() ; im++ )
			{
				M_COVR_Desc *mcd = m_pcovr_array.Item ( im );
				if ( ( m_selected_cell_index == mcd->m_cell_index ) &&
						( m_selected_object_id == mcd->m_object_id ) &&
						( m_selected_subcell == mcd->m_subcell ) )
				{
					sel_index = im;
					break;
				}
			}

			m_pListCtrlMCOVRs->SetItemCount ( m_pcovr_array.GetCount() );
			if ( -1 != sel_index )
				m_pListCtrlMCOVRs->SetItemState ( sel_index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
			else
				m_pListCtrlMCOVRs->SetItemState ( sel_index, 0, wxLIST_STATE_SELECTED );   // deselect all

			m_pListCtrlMCOVRs->Refresh ( true );


		}
#ifdef __WXMSW__
		m_pListCtrlMCOVRs->Refresh ( false );
#endif
	}
}

