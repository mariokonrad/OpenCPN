/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#include "Hyperlink.h"

Hyperlink::Finder::Finder(const wxString& url, const wxString& label)
	: url(url)
	, label(label)
{
}

bool Hyperlink::Finder::operator()(const Hyperlink& link) const
{
	return (link.url() == url)
		   && ((link.desc() == label) || (link.url() == label && link.desc() == wxEmptyString));
}

Hyperlink::Hyperlink(const wxString& desc, const wxString& url, const wxString& type)
	: DescrText(desc)
	, Link(url)
	, LType(type)
{
}

Hyperlink::Hyperlink(const Hyperlink& other)
	: DescrText(other.DescrText)
	, Link(other.Link)
	, LType(other.LType)
{
}

const wxString& Hyperlink::desc() const
{
	return DescrText;
}

const wxString& Hyperlink::url() const
{
	return Link;
}

const wxString& Hyperlink::type() const
{
	return LType;
}

