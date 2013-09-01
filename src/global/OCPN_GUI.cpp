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

#include "OCPN_GUI.h"
#include <algorithm>

namespace global {

const GUI::View & OCPN_GUI::view() const
{
	return data_view;
}

void OCPN_GUI::set_view_screen_brightness(int value)
{
	data_view.screen_brightness = value;
}

const GUI::Frame & OCPN_GUI::frame() const
{
	return data_frame;
}

void OCPN_GUI::set_frame_position(const wxPoint & position)
{
	data_frame.position = position;
}

void OCPN_GUI::set_frame_size(const wxSize & size)
{
	data_frame.size = size;
}

void OCPN_GUI::set_frame_maximized(bool flag)
{
	data_frame.maximized = flag;
}

void OCPN_GUI::set_frame_last_position(const wxPoint & position)
{
	data_frame.last_position = position;
}

void OCPN_GUI::set_frame_last_size(const wxSize & size)
{
	data_frame.last_size = size;
}

const GUI::Toolbar & OCPN_GUI::toolbar() const
{
	return data_toolbar;
}

void OCPN_GUI::set_toolbar_position(const wxPoint & position)
{
	data_toolbar.position = position;
}

void OCPN_GUI::set_toolbar_orientation(long orientation)
{
	data_toolbar.orientation = orientation;
}

void OCPN_GUI::set_toolbar_transparent(bool flag)
{
	data_toolbar.transparent = flag;
}

void OCPN_GUI::ensure_toolbar_position_range(wxPoint p0, wxPoint p1)
{
	data_toolbar.position.x = std::max(data_toolbar.position.x, p0.x);
	data_toolbar.position.y = std::max(data_toolbar.position.y, p0.y);
	data_toolbar.position.x = std::min(data_toolbar.position.x, p1.x);
	data_toolbar.position.y = std::min(data_toolbar.position.y, p1.y);
}

const GUI::AISAlertDialog & OCPN_GUI::ais_alert_dialog() const
{
	return data_ais_alert_dialog;
}

void OCPN_GUI::set_ais_alert_dialog_position(const wxPoint & position)
{
	data_ais_alert_dialog.position = position;
}

void OCPN_GUI::set_ais_alert_dialog_size(const wxSize & size)
{
	data_ais_alert_dialog.size = size;
}

void OCPN_GUI::ensure_ais_alert_dialog_position_range(wxPoint p0, wxPoint p1)
{
	data_ais_alert_dialog.position.x = std::max(data_ais_alert_dialog.position.x, p0.x);
	data_ais_alert_dialog.position.y = std::max(data_ais_alert_dialog.position.y, p0.y);
	data_ais_alert_dialog.position.x = std::min(data_ais_alert_dialog.position.x, p1.x);
	data_ais_alert_dialog.position.y = std::min(data_ais_alert_dialog.position.y, p1.y);
}

void OCPN_GUI::ensure_ais_alert_dialog_position_range(wxPoint p0, wxSize p1)
{
	ensure_ais_alert_dialog_position_range(p0, wxPoint(0, 0) + p1);
}

const GUI::AISQueryDialog & OCPN_GUI::ais_query_dialog() const
{
	return data_ais_query_dialog;
}

void OCPN_GUI::set_ais_query_dialog_position(const wxPoint & position)
{
	data_ais_query_dialog.position = position;
}

}

