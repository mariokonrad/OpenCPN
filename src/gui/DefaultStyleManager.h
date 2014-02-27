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

#ifndef __GUI__DEFAULTSTYLEMANAGER__H__
#define __GUI__DEFAULTSTYLEMANAGER__H__

#include <gui/StyleManager.h>

namespace gui {

/// The default implementation of style management.
class DefaultStyleManager : public StyleManager
{
public:
	DefaultStyleManager();
	virtual ~DefaultStyleManager();

	virtual bool initialize();
	virtual void SetStyle(const wxString& name);

	virtual void SetStyleNextInvocation(const wxString& name);
	virtual const wxString& GetStyleNextInvocation() const;

	virtual StyleNames GetStyleNames() const;

	virtual const Style& current() const;
	virtual Style& current();

private:
	typedef std::vector<Style*> Styles;

	void Init(const wxString& path);
	std::vector<wxString> enumerate_style_files(const wxString& path) const;

	Styles styles;
	Style* currentStyle;
	wxString nextInvocationStyle;
};

}

#endif
