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

#include <gui/Style.h>
#include <gui/XMLStyleReader.h>

#include <global/OCPN.h>
#include <global/System.h>

#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/log.h>
#include <wx/image.h>

namespace gui {

DefaultStyleManager::DefaultStyleManager()
	: currentStyle(NULL)
{
}

DefaultStyleManager::~DefaultStyleManager()
{
	for (Styles::const_iterator i = styles.begin(); i != styles.end(); ++i)
		delete *i;
	styles.clear();
}

bool DefaultStyleManager::initialize()
{
	const global::System::Data& sys = global::OCPN::get().sys().data();

	Init(sys.sound_data_location + _T("uidata") + wxFileName::GetPathSeparator());
	Init(sys.home_location);
	Init(sys.home_location + _T(".opencpn") + wxFileName::GetPathSeparator());
	SetStyle(_T(""));

	return !styles.empty();
}

const Style& DefaultStyleManager::current() const
{
	return *currentStyle;
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
		names.push_back((*i)->getName());

	return names;
}

Style& DefaultStyleManager::current()
{
	return *currentStyle;
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

void DefaultStyleManager::Init(const wxString& path)
{
	std::vector<wxString> filenames = enumerate_style_files(path);
	if (filenames.empty()) {
		wxLogMessage(_T("No styles found at: ") + path);
		return;
	}

	XMLStyleReader reader;
	for (std::vector<wxString>::const_iterator fn = filenames.begin(); fn != filenames.end(); ++fn) {
		if (reader.read_file(path + *fn, styles)) {
			wxLogMessage(_T("Styles loaded from ") + path + *fn);
		} else {
			wxLogMessage(_T("Attempt to load styles from this file failed: ") + path + *fn);
		}
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
		if (style->getName() == name || selectFirst) {
			if (style->getGraphics()) {
				currentStyle = style;
				ok = true;
				break;
			}

			// FIXME: move the stuff to style

			wxString fullFilePath = style->config_path() + wxFileName::GetPathSeparator()
									+ style->graphics_filename();

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
			style->setGraphics(new wxBitmap(img)); // FIXME: not to be done here, stop this lazy init already
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
		if ((style->get_consoleTextBackgroundSize().x)
			&& (style->get_consoleTextBackgroundSize().y)) {
			style->set_consoleTextBackground(style->getGraphics()->GetSubBitmap(wxRect(
				style->get_consoleTextBackgroundLoc(), style->get_consoleTextBackgroundSize())));
		}
	}

	if (style)
		nextInvocationStyle = style->getName();
}

}

