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

#include "PositionParser.h"
#include <PositionConvert.h>
#include <wx/tokenzr.h>
#include <wx/regex.h>
#include <wx/math.h>

PositionParser::PositionParser(const wxString& src)
	: parsedOk(false)
{
	if (FindSeparator(src)) {
		latitude = PositionConvert::lat(latitudeString);
		longitude = PositionConvert::lon(longitudeString);
		if ((latitude != 0.0) && (longitude != 0.0))
			parsedOk = true;
	}
}

geo::Position PositionParser::get() const
{
	return geo::Position(latitude, longitude);
}

bool PositionParser::FindSeparator(const wxString& src)
{
	wxStringTokenizer t;

	// Used when format is similar to "12 34.56 N 12 34.56 E"
	wxString posPartOfSeparator = _T("");

	// First the XML case:
	// Generalized XML tag format, accepts anything like <XXX yyy="<lat>" zzz="<lon>" >
	// GPX format <wpt lat="<lat>" lon="<lon>" /> tag among others.

	wxRegEx regex;

	int re_compile_flags = wxRE_ICASE;
#ifdef wxHAS_REGEX_ADVANCED
	re_compile_flags |= wxRE_ADVANCED;
#endif

	regex.Compile(
		_T( "<[a-z,A-Z]*\\s*[a-z,A-Z]*=\"([0-9,.]*)\"\\s*[a-z,A-Z]*=\"([-,0-9,.]*)\"\\s*/*>" ),
		re_compile_flags);

	if (regex.IsValid()) {
		if (regex.Matches(src)) {
			latitudeString = regex.GetMatch(src, 1);
			longitudeString = regex.GetMatch(src, 2);
			latitudeString.Trim(true);
			latitudeString.Trim(false);
			longitudeString.Trim(true);
			longitudeString.Trim(false);
			return true;
		}
	}

	// Now try various separators.

	separator = _T(", ");
	t = wxStringTokenizer(src, separator);
	if (t.CountTokens() == 2)
		goto found;

	separator = _T(",");
	t = wxStringTokenizer(src, separator);
	if (t.CountTokens() == 2)
		goto found;

	separator = _T(" ");
	t = wxStringTokenizer(src, separator);
	if (t.CountTokens() == 2)
		goto found;

	separator = _T("\t");
	t = wxStringTokenizer(src, separator);
	if (t.CountTokens() == 2)
		goto found;

	separator = _T("\n");
	t = wxStringTokenizer(src, separator);
	if (t.CountTokens() == 2)
		goto found;

	separator = _T("N");
	t = wxStringTokenizer(src, separator);
	posPartOfSeparator = _T("N");
	if (t.CountTokens() == 2)
		goto found;

	separator = _T("S");
	t = wxStringTokenizer(src, separator);
	posPartOfSeparator = _T("S");
	if (t.CountTokens() == 2)
		goto found;

	// Give up.
	return false;

found:
	latitudeString = t.GetNextToken() << posPartOfSeparator;
	latitudeString.Trim(true);
	latitudeString.Trim(false);
	longitudeString = t.GetNextToken();
	longitudeString.Trim(true);
	longitudeString.Trim(false);

	return true;
}

const wxString & PositionParser::GetSeparator() const
{
	return separator;
}

const wxString & PositionParser::GetLatitudeString() const
{
	return latitudeString;
}

const wxString & PositionParser::GetLongitudeString() const
{
	return longitudeString;
}

double PositionParser::GetLatitude() const
{
	return latitude;
}

double PositionParser::GetLongitude() const
{
	return longitude;
}

bool PositionParser::IsOk() const
{
	return parsedOk;
}

