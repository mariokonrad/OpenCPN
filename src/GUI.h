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

#ifndef __GUI__H__
#define __GUI__H__

#include <wx/gdicmn.h>

class GUI
{
	public:

		struct Toolbar
		{
			wxPoint position;
			long orientation;
		};

		virtual const Toolbar & get_toolbar() const = 0;
		virtual void set_toolbar_position(const wxPoint &) = 0;
		virtual void set_toolbar_orientation(long) = 0;

	public:

		struct AISAlertDialog
		{
			wxPoint position;
			wxSize size;
		};

		virtual const AISAlertDialog & get_ais_alert_dialog() const = 0;
		virtual void set_ais_alert_dialog_position(const wxPoint &) = 0;
		virtual void set_ais_alert_dialog_size(const wxSize &) = 0;

	public:

		struct AISQueryDialog
		{
			wxPoint position;
		};

		virtual const AISQueryDialog & get_ais_query_dialog() const = 0;
		virtual void set_ais_query_dialog_position(const wxPoint &) = 0;
};

#endif
