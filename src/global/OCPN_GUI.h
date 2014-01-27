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
	AISTargetList data_ais_target_list;
	CM93 data_cm93;
	S57Dialog data_s57dialog;
	OwnShip data_ownship;

public: // view
	virtual const View& view() const;
	virtual void set_view_screen_brightness(int);
	virtual void set_view_show_outlines(bool);
	virtual void set_view_show_depth_units(bool);
	virtual void set_view_lookahead_mode(bool);
	virtual void set_view_allow_overzoom_x(bool);
	virtual void set_view_show_overzoom_emboss(bool);
	virtual void set_route_line_width(int);
	virtual void set_track_line_width(int);
	virtual void set_enable_zoom_to_cursor(bool);
	virtual void set_DrawAISSize(bool);
	virtual void set_ShowAISName(bool);
	virtual void set_Show_Target_Name_Scale(int);
	virtual void set_WayPointPreventDragging(bool);
	virtual void set_ConfirmObjectDelete(bool);
	virtual void set_color_scheme(ColorScheme);
	virtual void set_initial_scale_ppm(double);
	virtual void set_smooth_pan_zoom(bool);
	virtual void set_view_display_grid(bool);
	virtual void set_view_show_layers(bool);
	virtual void set_view_permanent_mob_icon(bool);
	virtual void set_view_show_active_route_highway(bool);
	virtual void set_auto_anchor_mark(bool);
	virtual void set_view_preserve_scale_on_x(bool);
	virtual void set_view_quilt_enable(bool);
	virtual void set_view_fullscreen_quilt(bool);

public: // frame
	virtual const Frame& frame() const;
	virtual void set_frame_position(const wxPoint&);
	virtual void set_frame_size(const wxSize&);
	virtual void set_frame_maximized(bool);
	virtual void set_frame_last_position(const wxPoint&);
	virtual void set_frame_last_size(const wxSize&);

public: // toolbar
	virtual const Toolbar& toolbar() const;
	virtual void set_toolbar_position(const wxPoint&);
	virtual void set_toolbar_orientation(long);
	virtual void set_toolbar_transparent(bool);
	virtual void set_toolbar_full_screen(bool);
	void ensure_toolbar_position_range(wxPoint, wxPoint);

public: // ais alert dialog
	virtual const AISAlertDialog& ais_alert_dialog() const;
	virtual void set_ais_alert_dialog_position(const wxPoint&);
	virtual void set_ais_alert_dialog_size(const wxSize&);
	void ensure_ais_alert_dialog_position_range(wxPoint, wxPoint);
	void ensure_ais_alert_dialog_position_range(wxPoint, wxSize);

public: // ais query dialog
	virtual const AISQueryDialog& ais_query_dialog() const;
	virtual void set_ais_query_dialog_position(const wxPoint&);

public: // ais target list
	virtual const AISTargetList& ais_target_list() const;
	virtual void set_ais_target_list_perspective(const wxString&);
	virtual void set_ais_target_list_range(int);
	virtual void set_ais_target_list_sortColumn(int);
	virtual void set_ais_target_list_sortReverse(bool);
	virtual void set_ais_target_list_column_spec(const wxString&);

public: // cm93
	virtual const CM93& cm93() const;
	virtual void set_cm93_zoom_factor(int);
	virtual void set_cm93_detail_dialog_position(const wxPoint&);
	virtual void set_cm93_show_detail_slider(bool);

public: // S57 dialog
	virtual const S57Dialog& s57dialog() const;
	virtual void set_S57_dialog_size(const wxSize&);

public: // own ship
	virtual const OwnShip& ownship() const;
	virtual void set_ownship_cog_predictor_width(int);
	virtual void set_ownship_length_meters(double);
	virtual void set_ownship_beam_meters(double);
	virtual void set_ownship_min_mm(int);
};

}

#endif
