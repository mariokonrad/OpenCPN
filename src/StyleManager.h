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

#ifndef __OCPNSTYLE__STYLEMANAGER__H__
#define __OCPNSTYLE__STYLEMANAGER__H__

#include <wx/string.h>

#include <vector>

class TiXmlElement;

namespace ocpnStyle {

class Style;

class StyleManager // FIXME: provide interface for global access
{
public:
	typedef std::vector<wxString> StyleNames;

	StyleManager();
	~StyleManager();

	bool IsOK() const;
	void Init(const wxString& fromPath);
	void SetStyle(wxString name);
	void SetStyleNextInvocation(const wxString& name);
	const wxString& GetStyleNextInvocation() const;

	const Style& current() const;
	Style& current(); // FIXME: as soon as Style has a reasonable behaviour (icon handling), this method has to go

	StyleNames GetStyleNames() const;

private:
	typedef std::vector<Style*> Styles;

	// FIXME: move style reading from XML into separate class, 'StyleFactory' perhaps

	void read_description(Style* style, TiXmlElement* node) const;
	void read_chart_status_icon(Style* style, TiXmlElement* node) const;
	void read_chart_status_window(Style* style, TiXmlElement* node) const;
	void read_embossed_indicators(Style* style, TiXmlElement* node) const;
	void read_graphics_file(Style* style, TiXmlElement* node) const;
	void read_active_route(Style* style, TiXmlElement* node) const;
	void read_icons(Style* style, TiXmlElement* node) const;
	void read_tools(Style* style, TiXmlElement* node) const;
	void read_tool_compass(Style* style, TiXmlElement* node) const;
	void read_tool_attr_margin(Style* style, TiXmlElement* node, int orientation) const;
	void read_tool_attr_toggled_location(Style* style, TiXmlElement* node, int orientation) const;
	void read_tool_attr_toolbar_start(Style* style, TiXmlElement* node, int orientation) const;
	void read_tool_attr_toolbar_end(Style* style, TiXmlElement* node, int orientation) const;
	void read_tool_attr_toolbar_corners(Style* style, TiXmlElement* node, int orientation) const;
	void read_tool_attr_background_location(Style* style, TiXmlElement* node, int orientation) const;
	void read_tool_attr_active_location(Style* style, TiXmlElement* node, int orientation) const;
	void read_tool_attr_size(Style* style, TiXmlElement* node, int orientation) const;
	void read_tool_attr_icon_offset(Style* style, TiXmlElement* node, int orientation) const;

	bool isOK;
	Styles styles;
	Style* currentStyle;
	wxString nextInvocationStyle;
};

}

#endif
