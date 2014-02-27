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

#include "XMLStyleReader.h"

#include <gui/Style.h>
#include <gui/Icon.h>
#include <gui/Tool.h>

#include <tinyxml/tinyxml.h>

#include <wx/filename.h>

namespace gui {

XMLStyleReader::~XMLStyleReader()
{
}

bool XMLStyleReader::read_file(const wxString& path, Styles& styles)
{
	TiXmlDocument doc;

	if (!doc.LoadFile(static_cast<const char*>(path.mb_str()))) { // FIXME: string conversion
		return false;
	}

	read_doc(doc, wxFileName(path).GetPath(), styles);
	return true;
}

bool XMLStyleReader::read_data(const char* data, Styles& styles)
{
	TiXmlDocument doc;

	const char* p = doc.Parse(data);
	if (!p && doc.Error()) {
		return false;
	}

	read_doc(doc, wxString(), styles);
	return true;
}

void XMLStyleReader::read_description(Style* style, TiXmlElement* node) const
{
	style->description = wxString(node->GetText(), wxConvUTF8);
}

void XMLStyleReader::read_chart_status_icon(Style* style, TiXmlElement* node) const
{
	int w = 0;
	node->QueryIntAttribute("width", &w);
	style->chartStatusIconWidth = w;
}

void XMLStyleReader::read_chart_status_window(Style* style, TiXmlElement* node) const
{
	style->chartStatusWindowTransparent
		= wxString(node->Attribute("transparent"), wxConvUTF8).Lower().IsSameAs(_T("true"));
}

void XMLStyleReader::read_embossed_indicators(Style* style, TiXmlElement* node) const
{
	style->embossFont = wxString(node->Attribute("font"), wxConvUTF8);
	node->QueryIntAttribute("size", &(style->embossHeight));
}

void XMLStyleReader::read_graphics_file(Style* style, TiXmlElement* node) const
{
	style->graphicsFile = wxString(node->Attribute("name"), wxConvUTF8);
}

void XMLStyleReader::read_active_route(Style* style, TiXmlElement* node) const
{
	TiXmlHandle handle(node);
	TiXmlElement* tag = handle.Child("font-color", 0).ToElement();
	if (tag) {
		int r, g, b;
		tag->QueryIntAttribute("r", &r);
		tag->QueryIntAttribute("g", &g);
		tag->QueryIntAttribute("b", &b);
		style->consoleFontColor = wxColour(r, g, b);
	}
	tag = handle.Child("text-background-location", 0).ToElement();
	if (tag) {
		int x, y, w, h;
		tag->QueryIntAttribute("x", &x);
		tag->QueryIntAttribute("y", &y);
		tag->QueryIntAttribute("width", &w);
		tag->QueryIntAttribute("height", &h);
		style->consoleTextBackgroundLoc = wxPoint(x, y);
		style->consoleTextBackgroundSize = wxSize(w, h);
	}
}

void XMLStyleReader::read_icons(Style* style, TiXmlElement* node) const
{
	TiXmlElement* iconNode = node->FirstChild()->ToElement();

	for (; iconNode; iconNode = iconNode->NextSiblingElement()) {
		std::string nodeType = iconNode->ValueStr();
		if (nodeType == "icon") {
			Icon* icon = new Icon;
			style->icons.push_back(icon);
			icon->name = wxString(iconNode->Attribute("name"), wxConvUTF8);
			style->iconIndex[icon->name] = style->icons.size() - 1;
			TiXmlHandle handle(iconNode);
			TiXmlElement* tag = handle.Child("icon-location", 0).ToElement();
			if (tag) {
				int x;
				int y;
				tag->QueryIntAttribute("x", &x);
				tag->QueryIntAttribute("y", &y);
				icon->iconLoc = wxPoint(x, y);
			}
			tag = handle.Child("size", 0).ToElement();
			if (tag) {
				int x;
				int y;
				tag->QueryIntAttribute("x", &x);
				tag->QueryIntAttribute("y", &y);
				icon->size = wxSize(x, y);
			}
		}
	}
}

void XMLStyleReader::read_tool_compass(Style* style, TiXmlElement* node) const
{
	TiXmlElement* attrNode = node->FirstChild()->ToElement();
	for (; attrNode; attrNode = attrNode->NextSiblingElement()) {
		std::string nodeType = attrNode->ValueStr();
		if (nodeType == "margin") {
			attrNode->QueryIntAttribute("top", &style->compassMarginTop);
			attrNode->QueryIntAttribute("right", &style->compassMarginRight);
			attrNode->QueryIntAttribute("bottom", &style->compassMarginBottom);
			attrNode->QueryIntAttribute("left", &style->compassMarginLeft);
			continue;
		}
		if (nodeType == "compass-corners") {
			int r;
			attrNode->QueryIntAttribute("radius", &r);
			style->compasscornerRadius = r;
			continue;
		}
		if (nodeType == "offset") {
			attrNode->QueryIntAttribute("x", &style->compassXoffset);
			attrNode->QueryIntAttribute("y", &style->compassYoffset);
			continue;
		}
	}
}

void XMLStyleReader::read_tool_attr_margin(Style* style, TiXmlElement* node, int orientation) const
{
	node->QueryIntAttribute("top", &style->toolMarginTop[orientation]);
	node->QueryIntAttribute("right", &style->toolMarginRight[orientation]);
	node->QueryIntAttribute("bottom", &style->toolMarginBottom[orientation]);
	node->QueryIntAttribute("left", &style->toolMarginLeft[orientation]);
	wxString invis = wxString(node->Attribute("invisible"), wxConvUTF8);
	style->marginsInvisible = (invis.Lower() == _T("true"));
}

void XMLStyleReader::read_tool_attr_toggled_location(Style* style, TiXmlElement* node,
												   int orientation) const
{
	int x;
	int y;
	query_intattr_xy(node, x, y);
	style->toggledBGlocation[orientation] = wxPoint(x, y);
	int w;
	int h;
	node->QueryIntAttribute("width", &w);
	node->QueryIntAttribute("height", &h);
	style->toggledBGSize[orientation] = wxSize(w, h);
}

void XMLStyleReader::read_tool_attr_toolbar_start(Style* style, TiXmlElement* node,
												int orientation) const
{
	int x;
	int y;
	query_intattr_xy(node, x, y);
	style->toolbarStartLoc[orientation] = wxPoint(x, y);
	int w;
	int h;
	node->QueryIntAttribute("width", &w);
	node->QueryIntAttribute("height", &h);
	style->toolbarStartSize[orientation] = wxSize(w, h);
}

void XMLStyleReader::read_tool_attr_toolbar_end(Style* style, TiXmlElement* node,
											  int orientation) const
{
	int x;
	int y;
	query_intattr_xy(node, x, y);
	style->toolbarEndLoc[orientation] = wxPoint(x, y);
	int w;
	int h;
	node->QueryIntAttribute("width", &w);
	node->QueryIntAttribute("height", &h);
	style->toolbarEndSize[orientation] = wxSize(w, h);
}

void XMLStyleReader::read_tool_attr_toolbar_corners(Style* style, TiXmlElement* node,
												  int orientation) const
{
	int r;
	node->QueryIntAttribute("radius", &r);
	style->cornerRadius[orientation] = r;
}

void XMLStyleReader::read_tool_attr_background_location(Style* style, TiXmlElement* node,
													  int orientation) const
{
	int x;
	int y;
	query_intattr_xy(node, x, y);
	style->normalBGlocation[orientation] = wxPoint(x, y);
	style->hasBackground = true;
}

void XMLStyleReader::read_tool_attr_active_location(Style* style, TiXmlElement* node,
												  int orientation) const
{
	int x;
	int y;
	query_intattr_xy(node, x, y);
	style->activeBGlocation[orientation] = wxPoint(x, y);
}

void XMLStyleReader::read_tool_attr_size(Style* style, TiXmlElement* node,
											  int orientation) const
{
	int x;
	int y;
	query_intattr_xy(node, x, y);
	style->toolSize[orientation] = wxSize(x, y);
}

void XMLStyleReader::read_tool_attr_icon_offset(Style* style, TiXmlElement* node, int) const
{
	int x;
	int y;
	query_intattr_xy(node, x, y);
	style->verticalIconOffset = wxSize(x, y);
}

void XMLStyleReader::read_tools(Style* style, TiXmlElement* node) const
{
	TiXmlElement* toolNode = node->FirstChild()->ToElement();
	for (; toolNode; toolNode = toolNode->NextSiblingElement()) {
		std::string nodeType = toolNode->ValueStr();

		if (nodeType == "horizontal" || nodeType == "vertical") {
			int orientation = 0;
			if (nodeType == "vertical")
				orientation = 1;

			TiXmlElement* attrNode = toolNode->FirstChild()->ToElement();
			for (; attrNode; attrNode = attrNode->NextSiblingElement()) {
				std::string type = attrNode->ValueStr();
				if (type == "separation") {
					attrNode->QueryIntAttribute("distance", &style->toolSeparation[orientation]);
					continue;
				}
				if (type == "margin") {
					read_tool_attr_margin(style, attrNode, orientation);
					continue;
				}
				if (type == "toggled-location") {
					read_tool_attr_toggled_location(style, attrNode, orientation);
					continue;
				}
				if (type == "toolbar-start") {
					read_tool_attr_toolbar_start(style, attrNode, orientation);
					continue;
				}
				if (type == "toolbar-end") {
					read_tool_attr_toolbar_end(style, attrNode, orientation);
					continue;
				}
				if (type == "toolbar-corners") {
					read_tool_attr_toolbar_corners(style, attrNode, orientation);
					continue;
				}
				if (type == "background-location") {
					read_tool_attr_background_location(style, attrNode, orientation);
					continue;
				}
				if (type == "active-location") {
					read_tool_attr_active_location(style, attrNode, orientation);
					continue;
				}
				if (type == "size") {
					read_tool_attr_size(style, attrNode, orientation);
					continue;
				}
				if (type == "icon-offset") {
					read_tool_attr_icon_offset(style, attrNode, orientation);
					continue;
				}
			}
			continue;
		}

		if (nodeType == "compass") {
			read_tool_compass(style, toolNode);
		}

		if (nodeType == "tool") {
			Tool* tool = new Tool();
			style->tools.push_back(tool);
			tool->name = wxString(toolNode->Attribute("name"), wxConvUTF8);
			style->toolIndex[tool->name] = style->tools.size() - 1;
			TiXmlHandle toolHandle(toolNode);
			TiXmlElement* toolTag = toolHandle.Child("icon-location", 0).ToElement();
			if (toolTag) {
				int x;
				int y;
				query_intattr_xy(toolTag, x, y);
				tool->iconLoc = wxPoint(x, y);
			}
			toolTag = toolHandle.Child("rollover-location", 0).ToElement();
			if (toolTag) {
				int x;
				int y;
				query_intattr_xy(toolTag, x, y);
				tool->rolloverLoc = wxPoint(x, y);
			}
			toolTag = toolHandle.Child("disabled-location", 0).ToElement();
			if (toolTag) {
				int x;
				int y;
				query_intattr_xy(toolTag, x, y);
				tool->disabledLoc = wxPoint(x, y);
			}
			toolTag = toolHandle.Child("size", 0).ToElement();
			if (toolTag) {
				int x;
				int y;
				query_intattr_xy(toolTag, x, y);
				tool->customSize = wxSize(x, y);
			}
			continue;
		}
	}
}

void XMLStyleReader::query_intattr_xy(TiXmlElement* element, int& x, int& y) const
{
	element->QueryIntAttribute("x", &x);
	element->QueryIntAttribute("y", &y);
}

bool XMLStyleReader::read_style(Style* style, TiXmlElement* node) const
{
	if (node->NoChildren())
		return false;

	bool enough_data = false;

	style->name = wxString(node->Attribute("name"), wxConvUTF8);

	TiXmlElement* subNode = node->FirstChild()->ToElement();
	for (; subNode; subNode = subNode->NextSiblingElement()) {
		std::string nodeType = subNode->ValueStr();

		if (nodeType == "description") {
			read_description(style, subNode);
			continue;
		}
		if (nodeType == "chart-status-icon") {
			read_chart_status_icon(style, subNode);
			continue;
		}
		if (nodeType == "chart-status-window") {
			read_chart_status_window(style, subNode);
			continue;
		}
		if (nodeType == "embossed-indicators") {
			read_embossed_indicators(style, subNode);
			continue;
		}
		if (nodeType == "graphics-file") {
			read_graphics_file(style, subNode);
			enough_data = true;
			continue;
		}
		if (nodeType == "active-route") {
			read_active_route(style, subNode);
			continue;
		}
		if (nodeType == "icons") {
			read_icons(style, subNode);
		}
		if (nodeType == "tools") {
			read_tools(style, subNode);
			continue;
		}
	}

	return enough_data;
}

/// Reads style information from the specified document.
///
/// This method creates new style objects, depending on wheather
/// or not enough style information could be obtained from the
/// specified document or not.
void XMLStyleReader::read_doc(TiXmlDocument& doc, const wxString& path, Styles& styles) const
{
	TiXmlHandle hRoot(doc.RootElement());

	std::string root = doc.RootElement()->ValueStr();
	if (root != "styles") {
		return;
	}

	TiXmlElement* styleElem = hRoot.FirstChild().Element();
	for (; styleElem; styleElem = styleElem->NextSiblingElement()) {

		if (styleElem->ValueStr() != "style")
			continue;

		Style* style = new Style(path);
		if (read_style(style, styleElem)) {
			styles.push_back(style);
		} else {
			delete style;
		}
	}
}

}

