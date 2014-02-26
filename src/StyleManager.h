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

namespace ocpnStyle {

class Style;

/// Interface for style managers.
class StyleManager
{
public:
	typedef std::vector<wxString> StyleNames;

	virtual ~StyleManager()
	{
	}

	virtual bool IsOK() const = 0;
	virtual void SetStyle(const wxString& name) = 0;
	virtual void SetStyleNextInvocation(const wxString& name) = 0;
	virtual const wxString& GetStyleNextInvocation() const = 0;
	virtual StyleNames GetStyleNames() const = 0;
	virtual const Style& current() const = 0;
	virtual Style& current() = 0; // FIXME: as soon as Style has a reasonable behaviour (icon handling), this method has to go
};

}

#endif
