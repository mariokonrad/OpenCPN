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

#ifndef __PLUGIN__PLUGINPANEL__H__
#define __PLUGIN__PLUGINPANEL__H__

#include <wx/panel.h>

class wxButton;
class wxStaticText;
class wxFlexGridSizer;
class PluginListPanel;
class PlugInContainer;

class PluginPanel : public wxPanel
{
	public:
		PluginPanel(
				PluginListPanel * parent,
				wxWindowID id,
				const wxPoint & pos,
				const wxSize & size,
				PlugInContainer * p_plugin);
		virtual ~PluginPanel();

		void OnPluginSelected(wxMouseEvent & event);
		void SetSelected(bool selected);
		void OnPluginPreferences(wxCommandEvent & event);
		void OnPluginEnable(wxCommandEvent & event);
		void SetEnabled(bool enabled);
		bool GetSelected();

	private:
		PluginListPanel * m_PluginListPanel;
		bool m_bSelected;
		PlugInContainer * m_pPlugin;
		wxStaticText * m_pName;
		wxStaticText * m_pVersion;
		wxStaticText * m_pDescription;
		wxFlexGridSizer * m_pButtons;
		wxButton * m_pButtonEnable;
		wxButton * m_pButtonPreferences;
};

#endif
