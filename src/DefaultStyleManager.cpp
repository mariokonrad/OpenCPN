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

#include "DefaultStyleManager.h"
#include <Style.h>
#include <Icon.h>
#include <Tool.h>

#include <global/OCPN.h>
#include <global/System.h>

#include <tinyxml/tinyxml.h>

#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/log.h>
#include <wx/image.h>

namespace gui {

bool DefaultStyleManager::IsOK() const
{
	return isOK;
}

void DefaultStyleManager::SetStyleNextInvocation(const wxString& name)
{
	nextInvocationStyle = name;
}

const wxString& DefaultStyleManager::GetStyleNextInvocation() const
{
	return nextInvocationStyle;
}

StyleManager::StyleNames DefaultStyleManager::GetStyleNames() const
{
	StyleNames names;

	for (Styles::const_iterator i = styles.begin(); i != styles.end(); ++i)
		names.push_back((*i)->name);

	return names;
}

DefaultStyleManager::DefaultStyleManager()
	: isOK(false)
	, currentStyle(NULL)
{
	const global::System::Data& sys = global::OCPN::get().sys().data();

	Init(sys.sound_data_location + _T("uidata") + wxFileName::GetPathSeparator());
	Init(sys.home_location);
	Init(sys.home_location + _T(".opencpn") + wxFileName::GetPathSeparator());
	SetStyle(_T(""));
}

DefaultStyleManager::~DefaultStyleManager()
{
	for (Styles::const_iterator i = styles.begin(); i != styles.end(); ++i)
		delete *i;
	styles.clear();
}

const Style& DefaultStyleManager::current() const
{
	return *currentStyle;
}

Style& DefaultStyleManager::current()
{
	return *currentStyle;
}

void DefaultStyleManager::read_description(Style* style, TiXmlElement* node) const
{
	style->description = wxString(node->GetText(), wxConvUTF8);
}

void DefaultStyleManager::read_chart_status_icon(Style* style, TiXmlElement* node) const
{
	int w = 0;
	node->QueryIntAttribute("width", &w);
	style->chartStatusIconWidth = w;
}

void DefaultStyleManager::read_chart_status_window(Style* style, TiXmlElement* node) const
{
	style->chartStatusWindowTransparent
		= wxString(node->Attribute("transparent"), wxConvUTF8).Lower().IsSameAs(_T("true"));
}

void DefaultStyleManager::read_embossed_indicators(Style* style, TiXmlElement* node) const
{
	style->embossFont = wxString(node->Attribute("font"), wxConvUTF8);
	node->QueryIntAttribute("size", &(style->embossHeight));
}

void DefaultStyleManager::read_graphics_file(Style* style, TiXmlElement* node) const
{
	style->graphicsFile = wxString(node->Attribute("name"), wxConvUTF8);
}

void DefaultStyleManager::read_active_route(Style* style, TiXmlElement* node) const
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

void DefaultStyleManager::read_icons(Style* style, TiXmlElement* node) const
{
	TiXmlElement* iconNode = node->FirstChild()->ToElement();

	for (; iconNode; iconNode = iconNode->NextSiblingElement()) {
		wxString nodeType(iconNode->Value(), wxConvUTF8);
		if (nodeType == _T("icon")) {
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

void DefaultStyleManager::read_tool_compass(Style* style, TiXmlElement* node) const
{
	TiXmlElement* attrNode = node->FirstChild()->ToElement();
	for (; attrNode; attrNode = attrNode->NextSiblingElement()) {
		wxString nodeType(attrNode->Value(), wxConvUTF8);
		if (nodeType == _T("margin")) {
			attrNode->QueryIntAttribute("top", &style->compassMarginTop);
			attrNode->QueryIntAttribute("right", &style->compassMarginRight);
			attrNode->QueryIntAttribute("bottom", &style->compassMarginBottom);
			attrNode->QueryIntAttribute("left", &style->compassMarginLeft);
			continue;
		}
		if (nodeType == _T("compass-corners")) {
			int r;
			attrNode->QueryIntAttribute("radius", &r);
			style->compasscornerRadius = r;
			continue;
		}
		if (nodeType == _T("offset")) {
			attrNode->QueryIntAttribute("x", &style->compassXoffset);
			attrNode->QueryIntAttribute("y", &style->compassYoffset);
			continue;
		}
	}
}

void DefaultStyleManager::read_tool_attr_margin(Style* style, TiXmlElement* node, int orientation) const
{
	node->QueryIntAttribute("top", &style->toolMarginTop[orientation]);
	node->QueryIntAttribute("right", &style->toolMarginRight[orientation]);
	node->QueryIntAttribute("bottom", &style->toolMarginBottom[orientation]);
	node->QueryIntAttribute("left", &style->toolMarginLeft[orientation]);
	wxString invis = wxString(node->Attribute("invisible"), wxConvUTF8);
	style->marginsInvisible = (invis.Lower() == _T("true"));
}

void DefaultStyleManager::read_tool_attr_toggled_location(Style* style, TiXmlElement* node,
												   int orientation) const
{
	int x, y;
	node->QueryIntAttribute("x", &x);
	node->QueryIntAttribute("y", &y);
	style->toggledBGlocation[orientation] = wxPoint(x, y);
	x = 0;
	y = 0;
	node->QueryIntAttribute("width", &x);
	node->QueryIntAttribute("height", &y);
	style->toggledBGSize[orientation] = wxSize(x, y);
}

void DefaultStyleManager::read_tool_attr_toolbar_start(Style* style, TiXmlElement* node,
												int orientation) const
{
	int x;
	int y;
	node->QueryIntAttribute("x", &x);
	node->QueryIntAttribute("y", &y);
	style->toolbarStartLoc[orientation] = wxPoint(x, y);
	x = 0;
	y = 0;
	node->QueryIntAttribute("width", &x);
	node->QueryIntAttribute("height", &y);
	style->toolbarStartSize[orientation] = wxSize(x, y);
}

void DefaultStyleManager::read_tool_attr_toolbar_end(Style* style, TiXmlElement* node,
											  int orientation) const
{
	int x;
	int y;
	node->QueryIntAttribute("x", &x);
	node->QueryIntAttribute("y", &y);
	style->toolbarEndLoc[orientation] = wxPoint(x, y);
	x = 0;
	y = 0;
	node->QueryIntAttribute("width", &x);
	node->QueryIntAttribute("height", &y);
	style->toolbarEndSize[orientation] = wxSize(x, y);
}

void DefaultStyleManager::read_tool_attr_toolbar_corners(Style* style, TiXmlElement* node,
												  int orientation) const
{
	int r;
	node->QueryIntAttribute("radius", &r);
	style->cornerRadius[orientation] = r;
}

void DefaultStyleManager::read_tool_attr_background_location(Style* style, TiXmlElement* node,
													  int orientation) const
{
	int x, y;
	node->QueryIntAttribute("x", &x);
	node->QueryIntAttribute("y", &y);
	style->normalBGlocation[orientation] = wxPoint(x, y);
	style->hasBackground = true;
}

void DefaultStyleManager::read_tool_attr_active_location(Style* style, TiXmlElement* node,
												  int orientation) const
{
	int x, y;
	node->QueryIntAttribute("x", &x);
	node->QueryIntAttribute("y", &y);
	style->activeBGlocation[orientation] = wxPoint(x, y);
}

void DefaultStyleManager::read_tool_attr_size(Style* style, TiXmlElement* node, int orientation) const
{
	int x, y;
	node->QueryIntAttribute("x", &x);
	node->QueryIntAttribute("y", &y);
	style->toolSize[orientation] = wxSize(x, y);
}

void DefaultStyleManager::read_tool_attr_icon_offset(Style* style, TiXmlElement* node,
											  int orientation) const
{
	int x, y;
	node->QueryIntAttribute("x", &x);
	node->QueryIntAttribute("y", &y);
	style->verticalIconOffset = wxSize(x, y);
}

void DefaultStyleManager::read_tools(Style* style, TiXmlElement* node) const
{
	TiXmlElement* toolNode = node->FirstChild()->ToElement();
	for (; toolNode; toolNode = toolNode->NextSiblingElement()) {
		wxString nodeType(toolNode->Value(), wxConvUTF8);

		if (nodeType == _T("horizontal") || nodeType == _T("vertical")) {
			int orientation = 0;
			if (nodeType == _T("vertical"))
				orientation = 1;

			TiXmlElement* attrNode = toolNode->FirstChild()->ToElement();
			for (; attrNode; attrNode = attrNode->NextSiblingElement()) {
				wxString nodeType(attrNode->Value(), wxConvUTF8);
				if (nodeType == _T("separation")) {
					attrNode->QueryIntAttribute("distance", &style->toolSeparation[orientation]);
					continue;
				}
				if (nodeType == _T("margin")) {
					read_tool_attr_margin(style, attrNode, orientation);
					continue;
				}
				if (nodeType == _T("toggled-location")) {
					read_tool_attr_toggled_location(style, attrNode, orientation);
					continue;
				}
				if (nodeType == _T("toolbar-start")) {
					read_tool_attr_toolbar_start(style, attrNode, orientation);
					continue;
				}
				if (nodeType == _T("toolbar-end")) {
					read_tool_attr_toolbar_end(style, attrNode, orientation);
					continue;
				}
				if (nodeType == _T("toolbar-corners")) {
					read_tool_attr_toolbar_corners(style, attrNode, orientation);
					continue;
				}
				if (nodeType == _T("background-location")) {
					read_tool_attr_background_location(style, attrNode, orientation);
					continue;
				}
				if (nodeType == _T("active-location")) {
					read_tool_attr_active_location(style, attrNode, orientation);
					continue;
				}
				if (nodeType == _T("size")) {
					read_tool_attr_size(style, attrNode, orientation);
					continue;
				}
				if (nodeType == _T("icon-offset")) {
					read_tool_attr_icon_offset(style, attrNode, orientation);
					continue;
				}
			}
			continue;
		}
		if (nodeType == _T("compass")) {
			read_tool_compass(style, toolNode);
		}

		if (nodeType == _T("tool")) {
			Tool* tool = new Tool();
			style->tools.push_back(tool);
			tool->name = wxString(toolNode->Attribute("name"), wxConvUTF8);
			style->toolIndex[tool->name] = style->tools.size() - 1;
			TiXmlHandle toolHandle(toolNode);
			TiXmlElement* toolTag = toolHandle.Child("icon-location", 0).ToElement();
			if (toolTag) {
				int x, y;
				toolTag->QueryIntAttribute("x", &x);
				toolTag->QueryIntAttribute("y", &y);
				tool->iconLoc = wxPoint(x, y);
			}
			toolTag = toolHandle.Child("rollover-location", 0).ToElement();
			if (toolTag) {
				int x, y;
				toolTag->QueryIntAttribute("x", &x);
				toolTag->QueryIntAttribute("y", &y);
				tool->rolloverLoc = wxPoint(x, y);
			}
			toolTag = toolHandle.Child("disabled-location", 0).ToElement();
			if (toolTag) {
				int x, y;
				toolTag->QueryIntAttribute("x", &x);
				toolTag->QueryIntAttribute("y", &y);
				tool->disabledLoc = wxPoint(x, y);
			}
			toolTag = toolHandle.Child("size", 0).ToElement();
			if (toolTag) {
				int x, y;
				toolTag->QueryIntAttribute("x", &x);
				toolTag->QueryIntAttribute("y", &y);
				tool->customSize = wxSize(x, y);
			}
			continue;
		}
	}
}

/// Collects a list of files with the pattern path + '/style*.xml'
/// and returns the list of found entries.
/// If none were found, the container is empty.
///
/// @param[in] path The path to search for the files.
/// @return The container with found filenames.
std::vector<wxString> DefaultStyleManager::enumerate_style_files(const wxString& path) const
{
	std::vector<wxString> files;

	if (!wxDir::Exists(path)) {
		return files;
	}

	wxDir dir(path);
	if (!dir.IsOpened())
		return files;

	wxString filename;
	bool more = dir.GetFirst(&filename, _T("style*.xml"), wxDIR_FILES);
	while (more) {
		files.push_back(filename);
		more = dir.GetNext(&filename);
	}

	return files;
}

void DefaultStyleManager::read_style(Style* style, TiXmlElement* node)
{
	TiXmlElement* subNode = node->FirstChild()->ToElement();
	for (; subNode; subNode = subNode->NextSiblingElement()) {
		wxString nodeType(subNode->Value(), wxConvUTF8);

		if (nodeType == _T("description")) {
			read_description(style, subNode);
			continue;
		}
		if (nodeType == _T("chart-status-icon")) {
			read_chart_status_icon(style, subNode);
			continue;
		}
		if (nodeType == _T("chart-status-window")) {
			read_chart_status_window(style, subNode);
			continue;
		}
		if (nodeType == _T("embossed-indicators")) {
			read_embossed_indicators(style, subNode);
			continue;
		}
		if (nodeType == _T("graphics-file")) {
			read_graphics_file(style, subNode);
			isOK = true; // If we got this far we are at least partially OK... FIXME: this is just silly
			continue;
		}
		if (nodeType == _T("active-route")) {
			read_active_route(style, subNode);
			continue;
		}
		if (nodeType == _T("icons")) {
			read_icons(style, subNode);
		}
		if (nodeType == _T("tools")) {
			read_tools(style, subNode);
			continue;
		}
	}
}

/// Reads style information from the specified document.
///
/// This method creates new style objects, depending on wheather
/// or not enough style information could be obtained from the
/// specified document or not.
void DefaultStyleManager::read_doc(TiXmlDocument& doc, const wxString& path)
{
	TiXmlHandle hRoot(doc.RootElement());

	wxString root = wxString(doc.RootElement()->Value(), wxConvUTF8);
	if (root != _T("styles")) {
		wxLogMessage(_T("    DefaultStyleManager: Expected XML Root <styles> not found."));
		return;
	}

	// FIXME: remove the whole UTF8 conversion for finding nodes, it is useless.
	//        TinyXML returns either 'const char*' or 'const std::string&' anyways
	//        The 'styles.xml' also should contain an XML-header in general, example:
	//          <?xml version="1.0" encoding="US-ASCII" ?>

	TiXmlElement* styleElem = hRoot.FirstChild().Element();
	for (; styleElem; styleElem = styleElem->NextSiblingElement()) {

		if (wxString(styleElem->Value(), wxConvUTF8) != _T("style"))
			continue;

		Style* style = new Style;
		styles.push_back(style);

		style->name = wxString(styleElem->Attribute("name"), wxConvUTF8);
		style->myConfigFileDir = path;

		read_style(style, styleElem);
	}
}

void DefaultStyleManager::Init(const wxString& path)
{
	std::vector<wxString> filenames = enumerate_style_files(path);
	if (filenames.empty()) {
		wxLogMessage(_T("No styles found at: ") + path);
		return;
	}

	for (std::vector<wxString>::const_iterator fn = filenames.begin(); fn != filenames.end(); ++fn) {
		wxString fullFilePath = path + *fn;
		TiXmlDocument doc;

		if (!doc.LoadFile((const char*)fullFilePath.mb_str())) {
			wxLogMessage(_T("Attempt to load styles from this file failed: ") + fullFilePath);
			continue;
		}

		wxLogMessage(_T("Styles loading from ") + fullFilePath);

		read_doc(doc, path);
	}
}

void DefaultStyleManager::SetStyle(const wxString& name)
{
	Style* style = NULL;
	bool ok = true;

	if (currentStyle)
		currentStyle->Unload();
	else
		ok = false;

	bool selectFirst = false;

	if (name.Length() == 0)
		selectFirst = true;

	for (Styles::iterator i = styles.begin(); i != styles.end(); ++i) {
		style = *i;
		if (style->name == name || selectFirst) {
			if (style->graphics) {
				currentStyle = style;
				ok = true;
				break;
			}

			wxString fullFilePath = style->myConfigFileDir + wxFileName::GetPathSeparator()
									+ style->graphicsFile;

			if (!wxFileName::FileExists(fullFilePath)) {
				wxLogMessage(_T("Styles Graphics File not found: ") + fullFilePath);
				ok = false;
				if (selectFirst)
					continue;
				break;
			}

			wxImage img; // Only image does PNG LoadFile properly on GTK.

			if (!img.LoadFile(fullFilePath, wxBITMAP_TYPE_PNG)) {
				wxLogMessage(_T("Styles Graphics File failed to load: ") + fullFilePath);
				ok = false;
				break;
			}
			style->graphics = new wxBitmap(img);
			currentStyle = style;
			ok = true;
			break;
		}
	}

	if (!ok) {
		wxLogMessage(_T("The requested style was not found: ") + name);
		return;
	}

	if (style) {
		if ((style->consoleTextBackgroundSize.x) && (style->consoleTextBackgroundSize.y)) {
			style->consoleTextBackground = style->graphics->GetSubBitmap(
				wxRect(style->consoleTextBackgroundLoc, style->consoleTextBackgroundSize));
		}
	}

	if (style)
		nextInvocationStyle = style->name;
}

}

