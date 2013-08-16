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

#include "ToolBarTool.h"
#include "pluginmanager.h"

extern PlugInManager * g_pi_manager;

ToolBarTool::ToolBarTool(
		ToolBarSimple * tbar,
		int id,
		const wxString & label,
		const wxBitmap & bmpNormal,
		const wxBitmap & bmpRollover,
		wxItemKind kind,
		wxObject * clientData,
		const wxString & shortHelp,
		const wxString & longHelp)
	: wxToolBarToolBase((wxToolBarBase*)tbar, id, label, bmpNormal, bmpRollover, kind, clientData, shortHelp, longHelp)
{
	m_enabled = true;
	m_toggled = false;
	rollover = false;
	bitmapOK = false;

	toolname = g_pi_manager->GetToolOwnerCommonName( id );
	if( toolname == _T("") ) {
		isPluginTool = false;
		toolname = label;
		iconName = label;
	} else {
		isPluginTool = true;
		pluginNormalIcon = &bmpNormal;
		pluginRolloverIcon = &bmpRollover;
	}
}

void ToolBarTool::SetSize(const wxSize& size)
{
	m_width = size.x;
	m_height = size.y;
}

wxCoord ToolBarTool::GetWidth() const
{
	return m_width;
}

wxCoord ToolBarTool::GetHeight() const
{
	return m_height;
}

wxString ToolBarTool::GetToolname() const
{
	return toolname;
}

void ToolBarTool::SetIconName(const wxString &  name)
{
	iconName = name;
}

wxString ToolBarTool::GetIconName() const
{
	return iconName;
}

