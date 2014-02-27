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

#ifndef __GUI__STYLEREADER__H__
#define __GUI__STYLEREADER__H__

#include <vector>

class wxString;

namespace gui {

class Style;

/// Interface for reading styles.
class StyleReader
{
public:
	typedef std::vector<Style*> Styles;

	virtual ~StyleReader()
	{
	}

	/// Reads the specified file and appends found styles to the container.
	///
	/// @retval true Success
	/// @retval false Failure
	virtual bool read_file(const wxString& path, Styles& styles) = 0;

	/// Reads the specified data and appends found styles to the container.
	///
	/// @retval true Success
	/// @retval false Failure
	virtual bool read_data(const char* data, Styles& styles) = 0;
};

}

#endif
