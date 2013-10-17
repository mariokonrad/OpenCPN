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

#include "PluginPanel.h"

#include <plugin/PlugInManager.h>
#include <plugin/PluginListPanel.h>
#include <plugin/PluginListPanel.h>

#include <UserColors.h>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>
#include <wx/button.h>

extern PlugInManager * s_ppim;

void NotifySetupOptionsPlugin(PlugInContainer * pic);

PluginPanel::PluginPanel(
		PluginListPanel * parent,
		wxWindowID id,
		const wxPoint & pos,
		const wxSize & size,
		PlugInContainer * p_plugin)
	: wxPanel(parent, id, pos, size, wxBORDER_NONE)
{
	m_PluginListPanel = parent;
	m_pPlugin = p_plugin;
	m_bSelected = false;

	wxBoxSizer* itemBoxSizer01 = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(itemBoxSizer01);
	Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(PluginPanel::OnPluginSelected), NULL, this);

	wxStaticBitmap *itemStaticBitmap = new wxStaticBitmap( this, wxID_ANY, *m_pPlugin->m_bitmap);
	itemBoxSizer01->Add(itemStaticBitmap, 0, wxEXPAND|wxALL, 5);
	itemStaticBitmap->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler( PluginPanel::OnPluginSelected ), NULL, this);
	wxBoxSizer* itemBoxSizer02 = new wxBoxSizer(wxVERTICAL);
	itemBoxSizer01->Add(itemBoxSizer02, 1, wxEXPAND|wxALL, 0);
	wxBoxSizer* itemBoxSizer03 = new wxBoxSizer(wxHORIZONTAL);
	itemBoxSizer02->Add(itemBoxSizer03);
	m_pName = new wxStaticText( this, wxID_ANY, m_pPlugin->m_common_name );
	m_pName->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler( PluginPanel::OnPluginSelected ), NULL, this);
	wxFont font = *wxNORMAL_FONT;
	font.SetWeight(wxFONTWEIGHT_BOLD);
	m_pName->SetFont(font);
	itemBoxSizer03->Add(m_pName, 0, wxEXPAND|wxALL, 5);
	m_pVersion = new wxStaticText( this, wxID_ANY,
			wxString::Format(_T("%d.%d"), m_pPlugin->m_version_major, m_pPlugin->m_version_minor) );
	itemBoxSizer03->Add(m_pVersion, 0, wxEXPAND|wxALL, 5);
	m_pVersion->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler( PluginPanel::OnPluginSelected ), NULL, this);
	m_pDescription = new wxStaticText( this, wxID_ANY, m_pPlugin->m_short_description );
	itemBoxSizer02->Add( m_pDescription, 0, wxEXPAND|wxALL, 5 );
	m_pDescription->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler( PluginPanel::OnPluginSelected ), NULL, this);

	m_pButtons = new wxFlexGridSizer(2);
	m_pButtons->AddGrowableCol(1);

	//      m_pButtons = new wxBoxSizer(wxHORIZONTAL);
	itemBoxSizer02->Add( m_pButtons, 1, wxEXPAND|wxALL, 0 );
	m_pButtonPreferences = new wxButton( this, wxID_ANY, _("Preferences"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pButtons->Add( m_pButtonPreferences, 0, wxALIGN_LEFT|wxALL, 2);
	m_pButtonEnable = new wxButton( this, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
	m_pButtons->Add(m_pButtonEnable, 0, wxALIGN_RIGHT|wxALL, 2);
	m_pButtonPreferences->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PluginPanel::OnPluginPreferences), NULL, this);
	m_pButtonEnable->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PluginPanel::OnPluginEnable), NULL, this);

	SetSelected( m_bSelected );
}

PluginPanel::~PluginPanel()
{}

void PluginPanel::OnPluginSelected(wxMouseEvent &)
{
	SetSelected(true);
	m_PluginListPanel->SelectPlugin(this);
}

void PluginPanel::SetSelected(bool selected)
{
	m_bSelected = selected;
	if (selected) {
		SetBackgroundColour(GetGlobalColor(_T("DILG1")));
		m_pDescription->SetLabel( m_pPlugin->m_long_description );
		m_pButtons->Show(true);
		Layout();
	} else {
		SetBackgroundColour(GetGlobalColor(_T("DILG0")));
		m_pDescription->SetLabel( m_pPlugin->m_short_description );
		m_pButtons->Show(false);
		Layout();
	}
	// StaticText color change upon selection
	SetEnabled( m_pPlugin->m_bEnabled );
}

void PluginPanel::OnPluginPreferences(wxCommandEvent &)
{
	if (m_pPlugin->m_bEnabled && m_pPlugin->m_bInitState && (m_pPlugin->m_cap_flag & WANTS_PREFERENCES)) {
		m_pPlugin->m_pplugin->ShowPreferencesDialog(this);
	}
}

void PluginPanel::OnPluginEnable(wxCommandEvent &)
{
	SetEnabled(!m_pPlugin->m_bEnabled);
}

void PluginPanel::SetEnabled(bool enabled)
{
	if (m_pPlugin->m_bEnabled != enabled) {
		m_pPlugin->m_bEnabled = enabled;
		if(s_ppim)
			s_ppim->UpdatePlugIns();
		NotifySetupOptionsPlugin( m_pPlugin );
	}
	if (!enabled && !m_bSelected) {
		m_pName->SetForegroundColour(*wxLIGHT_GREY);
		m_pVersion->SetForegroundColour(*wxLIGHT_GREY);
		m_pDescription->SetForegroundColour(*wxLIGHT_GREY);
		m_pButtonEnable->SetLabel(_("Enable"));
	} else {
		m_pName->SetForegroundColour(*wxBLACK);
		m_pVersion->SetForegroundColour(*wxBLACK);
		m_pDescription->SetForegroundColour(*wxBLACK);
		if ( enabled )
			m_pButtonEnable->SetLabel(_("Disable"));
		else
			m_pButtonEnable->SetLabel(_("Enable"));
	}
	m_pButtonPreferences->Enable( enabled && (m_pPlugin->m_cap_flag & WANTS_PREFERENCES) );
}

bool PluginPanel::GetSelected()
{
	return m_bSelected;
}

