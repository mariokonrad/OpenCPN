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

#ifndef __GLOBAL__OCPN_GUI__H__
#define __GLOBAL__OCPN_GUI__H__

#include <global/GUI.h>

namespace global {

class OCPN_GUI : public GUI
{
	private:
		View data_view;
		Frame data_frame;
		Toolbar data_toolbar;
		AISAlertDialog data_ais_alert_dialog;
		AISQueryDialog data_ais_query_dialog;

	public: // view
		virtual const View & view() const;
		virtual void set_view_screen_brightness(int);
		virtual void set_view_show_outlines(bool);
		virtual void set_view_show_depth_units(bool);
		virtual void set_view_lookahead_mode(bool);

	public: // frame
		virtual const Frame & frame() const;
		virtual void set_frame_position(const wxPoint &);
		virtual void set_frame_size(const wxSize &);
		virtual void set_frame_maximized(bool);
		virtual void set_frame_last_position(const wxPoint &);
		virtual void set_frame_last_size(const wxSize &);

	public: // toolbar
		virtual const Toolbar & toolbar() const;
		virtual void set_toolbar_position(const wxPoint &);
		virtual void set_toolbar_orientation(long);
		virtual void set_toolbar_transparent(bool);
		virtual void set_toolbar_full_screen(bool);
		void ensure_toolbar_position_range(wxPoint, wxPoint);

	public: // ais alert dialog
		virtual const AISAlertDialog & ais_alert_dialog() const;
		virtual void set_ais_alert_dialog_position(const wxPoint &);
		virtual void set_ais_alert_dialog_size(const wxSize &);
		void ensure_ais_alert_dialog_position_range(wxPoint, wxPoint);
		void ensure_ais_alert_dialog_position_range(wxPoint, wxSize);

	public: // ais query dialog
		virtual const AISQueryDialog & ais_query_dialog() const;
		virtual void set_ais_query_dialog_position(const wxPoint &);
};

}

#endif
