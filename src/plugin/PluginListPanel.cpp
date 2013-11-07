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

#include "PluginListPanel.h"
#include <wx/sizer.h>
#include <wx/statline.h>

PluginListPanel::PluginListPanel(
		wxWindow * parent,
		wxWindowID id,
		const wxPoint & pos,
		const wxSize & size,
		ArrayOfPlugIns * pPluginArray)
	: wxScrolledWindow( parent, id, pos, size, wxTAB_TRAVERSAL | wxVSCROLL)
{
	m_pPluginArray = pPluginArray;
	m_PluginSelected = NULL;

	wxBoxSizer* itemBoxSizer01 = new wxBoxSizer( wxVERTICAL );
	SetSizer( itemBoxSizer01 );

	int max_dy = 0;

	for( unsigned int i=0 ; i < pPluginArray->size() ; i++ )
	{
		PluginPanel *pPluginPanel = new PluginPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, pPluginArray->Item(i) );
		itemBoxSizer01->Add( pPluginPanel, 0, wxEXPAND|wxALL, 0 );
		m_PluginItems.Add( pPluginPanel );

		wxStaticLine* itemStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
		itemBoxSizer01->Add( itemStaticLine, 0, wxEXPAND|wxALL, 0 );

		//    When a child Panel is selected, its size grows to include "Preferences" and Enable" buttons.
		//    As a consequence, the vertical size of the ListPanel grows as well.
		//    Calculate and add a spacer to bottom of ListPanel so that initial ListPanel
		//    minimum size calculations account for selected Panel size growth.

		pPluginPanel->SetSelected( false );       // start unselected
		itemBoxSizer01->Layout();
		wxSize nsel_size = pPluginPanel->GetSize();

		pPluginPanel->SetSelected( true );        // switch to selected, a bit bigger
		itemBoxSizer01->Layout();
		wxSize sel_size = pPluginPanel->GetSize();

		pPluginPanel->SetSelected( false );       // reset to unselected
		itemBoxSizer01->Layout();

		int dy = sel_size.y - nsel_size.y;
		dy += 10;                                 // fluff
		max_dy = wxMax(dy, max_dy);
	}

	itemBoxSizer01->AddSpacer(max_dy);
}

PluginListPanel::~PluginListPanel()
{
}

void PluginListPanel::UpdateSelections()
{
	for(unsigned int i=0 ; i < m_PluginItems.size() ; i++) {
		PluginPanel *pPluginPanel = m_PluginItems.Item(i);
		if( pPluginPanel ){
			pPluginPanel->SetSelected( pPluginPanel->GetSelected() );
		}
	}
}

void PluginListPanel::SelectPlugin(PluginPanel * pi)
{
	if (m_PluginSelected == pi)
		return;

	if (m_PluginSelected)
		m_PluginSelected->SetSelected(false);

	m_PluginSelected = pi;
	Layout();
	Refresh(false);
}

