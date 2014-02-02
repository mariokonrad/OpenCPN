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

#include "OCPN_Version.h"
#include <version.h>

namespace ocpn {

Version::Version()
{
	version_major = wxString::Format(wxT("%i"), VERSION_MAJOR);
	version_minor = wxString::Format(wxT("%i"), VERSION_MINOR);
	version_patch = wxString::Format(wxT("%i"), VERSION_PATCH);
	version_date = wxString(VERSION_DATE, wxConvUTF8);
}

wxString Version::get_version() const
{
	return wxT("\n      Version ") + version_major + wxT(".") + version_minor + wxT(".")
		   + version_patch + wxT(" Build ") + version_date;
}

wxString Version::get_short() const
{
	return version_major + _T(".") + version_minor + _T(".") + version_patch;
}

wxString Version::get_git_info() const
{
	return wxString::Format(wxT("\n\n(git: %s / %s)\n"), wxT(GIT_BRANCH), wxT(GIT_COMMIT_HASH));
}

}

