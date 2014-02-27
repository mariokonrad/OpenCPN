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

#ifndef __GUI__XMLSTYLEREADER__H__
#define __GUI__XMLSTYLEREADER__H__

#include <gui/StyleReader.h>

class TiXmlElement;
class TiXmlDocument;

namespace gui {

/// Reads styles from XML, using the TinyXML library.
class XMLStyleReader : public StyleReader
{
public:
	virtual ~XMLStyleReader();

	virtual bool read_file(const wxString& path, Styles& styles);
	virtual bool read_data(const char* data, Styles& styles);

private:
	void read_doc(TiXmlDocument& doc, const wxString& path, Styles& styles) const;
	bool read_style(Style* style, TiXmlElement* node) const;
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
	void read_tool_attr_background_location(Style* style, TiXmlElement* node,
											int orientation) const;
	void read_tool_attr_active_location(Style* style, TiXmlElement* node, int orientation) const;
	void read_tool_attr_size(Style* style, TiXmlElement* node, int orientation) const;
	void read_tool_attr_icon_offset(Style* style, TiXmlElement* node, int orientation) const;

	void query_intattr_xy(TiXmlElement* element, int& x, int& y) const;
};

}

#endif
