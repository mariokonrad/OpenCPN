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

#include "StyleManager.h"
#include <Style.h>
#include <Icon.h>
#include <Tool.h>
#include <tinyxml/tinyxml.h>
#include <global/OCPN.h>
#include <global/System.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/log.h>
#include <wx/image.h>

namespace ocpnStyle {

bool StyleManager::IsOK() const
{
	return isOK;
}

void StyleManager::SetStyleNextInvocation(const wxString & name)
{
	nextInvocationStyle = name;
}

const wxString & StyleManager::GetStyleNextInvocation() const
{
	return nextInvocationStyle;
}

StyleManager::StyleNames StyleManager::GetStyleNames() const
{
	StyleNames names;

	for (Styles::const_iterator i = styles.begin(); i != styles.end(); ++i)
		names.push_back((*i)->name);

	return names;
}

StyleManager::StyleManager(void)
	: isOK(false)
	, currentStyle(NULL)
{
	const global::System::Data & sys = global::OCPN::get().sys().data();

	Init(sys.sound_data_location + _T("uidata") + wxFileName::GetPathSeparator());
	Init(sys.home_location);
	Init(sys.home_location + _T(".opencpn") + wxFileName::GetPathSeparator());
	SetStyle(_T(""));
}

StyleManager::StyleManager(const wxString & configDir)
	: isOK(false)
	, currentStyle(NULL)
{
	Init(configDir);
	SetStyle(_T(""));
}

StyleManager::~StyleManager(void)
{
	for (Styles::const_iterator i = styles.begin(); i != styles.end(); ++i)
		delete *i;
	styles.clear();
}

Style * StyleManager::GetCurrentStyle()
{
	return currentStyle;
}

void StyleManager::Init(const wxString & fromPath)
{
	TiXmlDocument doc;

	if (!wxDir::Exists(fromPath)) {
		wxLogMessage(_T("No styles found at: ") + fromPath);
		return;
	}

	wxDir dir(fromPath);
	if (!dir.IsOpened())
		return;

	wxString filename;

	// We allow any number of styles to load from files called style<something>.xml

	bool more = dir.GetFirst(&filename, _T("style*.xml"), wxDIR_FILES);

	if (!more) {
		wxLogMessage(_T("No styles found at: ") + fromPath);
		return;
	}

	bool firstFile = true;
	while (more) {
		wxString name;
		wxString extension;

		if (!firstFile)
			more = dir.GetNext(&filename);
		if (!more)
			break;
		firstFile = false;

		wxString fullFilePath = fromPath + filename;

		if (!doc.LoadFile((const char*) fullFilePath.mb_str())) {
			wxLogMessage(_T("Attempt to load styles from this file failed: ") + fullFilePath);
			continue;
		}

		wxLogMessage(_T("Styles loading from ") + fullFilePath);

		TiXmlHandle hRoot(doc.RootElement());

		wxString root = wxString( doc.RootElement()->Value(), wxConvUTF8 );
		if( root != _T("styles" ) ) {
			wxLogMessage(_T("    StyleManager: Expected XML Root <styles> not found."));
			continue;
		}

		TiXmlElement* styleElem = hRoot.FirstChild().Element();

		for( ; styleElem; styleElem = styleElem->NextSiblingElement() ) {

			if( wxString( styleElem->Value(), wxConvUTF8 ) == _T("style") ) {

				Style * style = new Style();
				styles.push_back(style);

				style->name = wxString( styleElem->Attribute( "name" ), wxConvUTF8 );
				style->myConfigFileDir = fromPath;

				TiXmlElement* subNode = styleElem->FirstChild()->ToElement();

				for( ; subNode; subNode = subNode->NextSiblingElement() ) {
					wxString nodeType( subNode->Value(), wxConvUTF8 );

					if( nodeType == _T("description") ) {
						style->description = wxString( subNode->GetText(), wxConvUTF8 );
						continue;
					}
					if( nodeType == _T("chart-status-icon") ) {
						int w = 0;
						subNode->QueryIntAttribute( "width", &w );
						style->chartStatusIconWidth = w;
						continue;
					}
					if( nodeType == _T("chart-status-window") ) {
						style->chartStatusWindowTransparent = wxString(
								subNode->Attribute( "transparent" ), wxConvUTF8 ).Lower().IsSameAs(
								_T("true") );
						continue;
					}
					if( nodeType == _T("embossed-indicators") ) {
						style->embossFont = wxString( subNode->Attribute( "font" ), wxConvUTF8 );
						subNode->QueryIntAttribute( "size", &(style->embossHeight) );
						continue;
					}
					if( nodeType == _T("graphics-file") ) {
						style->graphicsFile = wxString( subNode->Attribute( "name" ), wxConvUTF8 );
						isOK = true; // If we got this far we are at least partially OK...
						continue;
					}
					if( nodeType == _T("active-route") ) {
						TiXmlHandle handle( subNode );
						TiXmlElement* tag = handle.Child( "font-color", 0 ).ToElement();
						if( tag ) {
							int r, g, b;
							tag->QueryIntAttribute( "r", &r );
							tag->QueryIntAttribute( "g", &g );
							tag->QueryIntAttribute( "b", &b );
							style->consoleFontColor = wxColour( r, g, b );
						}
						tag = handle.Child( "text-background-location", 0 ).ToElement();
						if( tag ) {
							int x, y, w, h;
							tag->QueryIntAttribute( "x", &x );
							tag->QueryIntAttribute( "y", &y );
							tag->QueryIntAttribute( "width", &w );
							tag->QueryIntAttribute( "height", &h );
							style->consoleTextBackgroundLoc = wxPoint( x, y );
							style->consoleTextBackgroundSize = wxSize( w, h );
						}
						continue;
					}
					if( nodeType == _T("icons") ) {
						TiXmlElement* iconNode = subNode->FirstChild()->ToElement();

						for( ; iconNode; iconNode = iconNode->NextSiblingElement() ) {
							wxString nodeType( iconNode->Value(), wxConvUTF8 );
							if( nodeType == _T("icon") ) {
								Icon* icon = new Icon();
								style->icons.Add( icon );
								icon->name = wxString( iconNode->Attribute( "name" ), wxConvUTF8 );
								style->iconIndex[icon->name] = style->icons.Count() - 1;
								TiXmlHandle handle( iconNode );
								TiXmlElement* tag = handle.Child( "icon-location", 0 ).ToElement();
								if( tag ) {
									int x, y;
									tag->QueryIntAttribute( "x", &x );
									tag->QueryIntAttribute( "y", &y );
									icon->iconLoc = wxPoint( x, y );
								}
								tag = handle.Child( "size", 0 ).ToElement();
								if( tag ) {
									int x, y;
									tag->QueryIntAttribute( "x", &x );
									tag->QueryIntAttribute( "y", &y );
									icon->size = wxSize( x, y );
								}
							}
						}
					}
					if( nodeType == _T("tools") ) {
						TiXmlElement* toolNode = subNode->FirstChild()->ToElement();

						for( ; toolNode; toolNode = toolNode->NextSiblingElement() ) {
							wxString nodeType( toolNode->Value(), wxConvUTF8 );

							if( nodeType == _T("horizontal") || nodeType == _T("vertical") ) {
								int orientation = 0;
								if( nodeType == _T("vertical") ) orientation = 1;

								TiXmlElement* attrNode = toolNode->FirstChild()->ToElement();
								for( ; attrNode; attrNode = attrNode->NextSiblingElement() ) {
									wxString nodeType( attrNode->Value(), wxConvUTF8 );
									if( nodeType == _T("separation") ) {
										attrNode->QueryIntAttribute( "distance",
												&style->toolSeparation[orientation] );
										continue;
									}
									if( nodeType == _T("margin") ) {
										attrNode->QueryIntAttribute( "top",
												&style->toolMarginTop[orientation] );
										attrNode->QueryIntAttribute( "right",
												&style->toolMarginRight[orientation] );
										attrNode->QueryIntAttribute( "bottom",
												&style->toolMarginBottom[orientation] );
										attrNode->QueryIntAttribute( "left",
												&style->toolMarginLeft[orientation] );
										wxString invis = wxString(
												attrNode->Attribute( "invisible" ), wxConvUTF8 );
										style->marginsInvisible = ( invis.Lower() == _T("true") );
										continue;;
									}
									if( nodeType == _T("toggled-location") ) {
										int x, y;
										attrNode->QueryIntAttribute( "x", &x );
										attrNode->QueryIntAttribute( "y", &y );
										style->toggledBGlocation[orientation] = wxPoint( x, y );
										x = 0;
										y = 0;
										attrNode->QueryIntAttribute( "width", &x );
										attrNode->QueryIntAttribute( "height", &y );
										style->toggledBGSize[orientation] = wxSize( x, y );
										continue;
									}
									if( nodeType == _T("toolbar-start") ) {
										int x, y;
										attrNode->QueryIntAttribute( "x", &x );
										attrNode->QueryIntAttribute( "y", &y );
										style->toolbarStartLoc[orientation] = wxPoint( x, y );
										x = 0;
										y = 0;
										attrNode->QueryIntAttribute( "width", &x );
										attrNode->QueryIntAttribute( "height", &y );
										style->toolbarStartSize[orientation] = wxSize( x, y );
										continue;
									}
									if( nodeType == _T("toolbar-end") ) {
										int x, y;
										attrNode->QueryIntAttribute( "x", &x );
										attrNode->QueryIntAttribute( "y", &y );
										style->toolbarEndLoc[orientation] = wxPoint( x, y );
										x = 0;
										y = 0;
										attrNode->QueryIntAttribute( "width", &x );
										attrNode->QueryIntAttribute( "height", &y );
										style->toolbarEndSize[orientation] = wxSize( x, y );
										continue;
									}
									if( nodeType == _T("toolbar-corners") ) {
										int r;
										attrNode->QueryIntAttribute( "radius", &r );
										style->cornerRadius[orientation] = r;
										continue;
									}
									if( nodeType == _T("background-location") ) {
										int x, y;
										attrNode->QueryIntAttribute( "x", &x );
										attrNode->QueryIntAttribute( "y", &y );
										style->normalBGlocation[orientation] = wxPoint( x, y );
										style->HasBackground( true );
										continue;
									}
									if( nodeType == _T("active-location") ) {
										int x, y;
										attrNode->QueryIntAttribute( "x", &x );
										attrNode->QueryIntAttribute( "y", &y );
										style->activeBGlocation[orientation] = wxPoint( x, y );
										continue;
									}
									if( nodeType == _T("size") ) {
										int x, y;
										attrNode->QueryIntAttribute( "x", &x );
										attrNode->QueryIntAttribute( "y", &y );
										style->toolSize[orientation] = wxSize( x, y );
										continue;
									}
									if( nodeType == _T("icon-offset") ) {
										int x, y;
										attrNode->QueryIntAttribute( "x", &x );
										attrNode->QueryIntAttribute( "y", &y );
										style->verticalIconOffset = wxSize( x, y );
										continue;
									}
								}
								continue;
							}
							if( nodeType == _T("compass") ) {

								TiXmlElement* attrNode = toolNode->FirstChild()->ToElement();
								for( ; attrNode; attrNode = attrNode->NextSiblingElement() ) {
									wxString nodeType( attrNode->Value(), wxConvUTF8 );
									if( nodeType == _T("margin") ) {
										attrNode->QueryIntAttribute("top", &style->compassMarginTop);
										attrNode->QueryIntAttribute("right", &style->compassMarginRight);
										attrNode->QueryIntAttribute("bottom", &style->compassMarginBottom);
										attrNode->QueryIntAttribute("left", &style->compassMarginLeft);
										continue;
									}
									if( nodeType == _T("compass-corners") ) {
										int r;
										attrNode->QueryIntAttribute( "radius", &r );
										style->compasscornerRadius = r;
										continue;
									}
									if( nodeType == _T("offset") ) {
										attrNode->QueryIntAttribute("x", &style->compassXoffset);
										attrNode->QueryIntAttribute("y", &style->compassYoffset);
										continue;
									}
								}
							}

							if( nodeType == _T("tool") ) {
								Tool* tool = new Tool();
								style->tools.Add( tool );
								tool->name = wxString( toolNode->Attribute( "name" ), wxConvUTF8 );
								style->toolIndex[tool->name] = style->tools.Count() - 1;
								TiXmlHandle toolHandle( toolNode );
								TiXmlElement* toolTag = toolHandle.Child( "icon-location", 0 ).ToElement();
								if( toolTag ) {
									int x, y;
									toolTag->QueryIntAttribute( "x", &x );
									toolTag->QueryIntAttribute( "y", &y );
									tool->iconLoc = wxPoint( x, y );
								}
								toolTag = toolHandle.Child( "rollover-location", 0 ).ToElement();
								if( toolTag ) {
									int x, y;
									toolTag->QueryIntAttribute( "x", &x );
									toolTag->QueryIntAttribute( "y", &y );
									tool->rolloverLoc = wxPoint( x, y );
								}
								toolTag = toolHandle.Child( "disabled-location", 0 ).ToElement();
								if( toolTag ) {
									int x, y;
									toolTag->QueryIntAttribute( "x", &x );
									toolTag->QueryIntAttribute( "y", &y );
									tool->disabledLoc = wxPoint( x, y );
								}
								toolTag = toolHandle.Child( "size", 0 ).ToElement();
								if( toolTag ) {
									int x, y;
									toolTag->QueryIntAttribute( "x", &x );
									toolTag->QueryIntAttribute( "y", &y );
									tool->customSize = wxSize( x, y );
								}
								continue;
							}
						}
						continue;
					}
				}
			}
		}
	}
}

void StyleManager::SetStyle(wxString name)
{
	Style * style = NULL;
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
		if( style->name == name || selectFirst ) {
			if( style->graphics ) {
				currentStyle = style;
				ok = true;
				break;
			}

			wxString fullFilePath = style->myConfigFileDir + wxFileName::GetPathSeparator() + style->graphicsFile;

			if( !wxFileName::FileExists( fullFilePath ) ) {
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

	return;
}

}

