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

#ifndef __PLUGIN__PLUGINLISTPANEL__H__
#define __PLUGIN__PLUGINLISTPANEL__H__

#include <wx/scrolwin.h>
#include "plugin/PlugInContainer.h"
#include "plugin/PluginPanel.h"

WX_DEFINE_ARRAY_PTR(PluginPanel *, ArrayOfPluginPanel);

class PluginListPanel : public wxScrolledWindow
{
	public:
		PluginListPanel(
				wxWindow * parent,
				wxWindowID id,
				const wxPoint & pos,
				const wxSize & size,
				ArrayOfPlugIns * pPluginArray);
		virtual ~PluginListPanel();

		void SelectPlugin(PluginPanel *pi);
		void UpdateSelections();

	private:
		ArrayOfPlugIns * m_pPluginArray;
		ArrayOfPluginPanel m_PluginItems;
		PluginPanel * m_PluginSelected;
};

#endif
