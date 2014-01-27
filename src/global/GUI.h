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

#ifndef __GLOBAL__GUI__H__
#define __GLOBAL__GUI__H__

#include <wx/gdicmn.h>

#include <global/ColorScheme.h>

namespace global {

class GUI
{
public:
	virtual ~GUI()
	{
	}

public:
	struct View
	{
		int screen_brightness;
		bool show_outlines;
		bool show_depth_units;
		bool lookahead_mode;
		bool allow_overzoom_x; // Allow high overzoom
		bool show_overzoom_emboss;
		int route_line_width;
		int track_line_width;
		bool enable_zoom_to_cursor;
		bool DrawAISSize;
		bool ShowAISName;
		int Show_Target_Name_Scale;
		bool WayPointPreventDragging;
		bool ConfirmObjectDelete;
		ColorScheme color_scheme;
		double initial_scale_ppm;
		bool smooth_pan_zoom;
		bool display_grid; // Flag indicating weather the lat/lon grid should be displayed
		bool show_layers;
		bool permanent_mob_icon;
		bool show_active_route_highway;
		bool auto_anchor_mark;
		bool preserve_scale_on_x;
		bool quilt_enable;
		bool fullscreen_quilt;
	};

	virtual const View& view() const = 0;
	virtual void set_view_screen_brightness(int) = 0;
	virtual void set_view_show_outlines(bool) = 0;
	virtual void set_view_show_depth_units(bool) = 0;
	virtual void set_view_lookahead_mode(bool) = 0;
	virtual void set_view_allow_overzoom_x(bool) = 0;
	virtual void set_view_show_overzoom_emboss(bool) = 0;
	virtual void set_route_line_width(int) = 0;
	virtual void set_track_line_width(int) = 0;
	virtual void set_enable_zoom_to_cursor(bool) = 0;
	virtual void set_DrawAISSize(bool) = 0;
	virtual void set_ShowAISName(bool) = 0;
	virtual void set_Show_Target_Name_Scale(int) = 0;
	virtual void set_WayPointPreventDragging(bool) = 0;
	virtual void set_ConfirmObjectDelete(bool) = 0;
	virtual void set_color_scheme(ColorScheme) = 0;
	virtual void set_initial_scale_ppm(double) = 0;
	virtual void set_smooth_pan_zoom(bool) = 0;
	virtual void set_view_display_grid(bool) = 0;
	virtual void set_view_show_layers(bool) = 0;
	virtual void set_view_permanent_mob_icon(bool) = 0;
	virtual void set_view_show_active_route_highway(bool) = 0;
	virtual void set_auto_anchor_mark(bool) = 0;
	virtual void set_view_preserve_scale_on_x(bool) = 0;
	virtual void set_view_quilt_enable(bool) = 0;
	virtual void set_view_fullscreen_quilt(bool) = 0;

public:
	struct Frame
	{
		wxPoint position;
		wxSize size;
		bool maximized;
		wxPoint last_position;
		wxSize last_size;
	};

	virtual const Frame& frame() const = 0;
	virtual void set_frame_position(const wxPoint&) = 0;
	virtual void set_frame_size(const wxSize&) = 0;
	virtual void set_frame_maximized(bool) = 0;
	virtual void set_frame_last_position(const wxPoint&) = 0;
	virtual void set_frame_last_size(const wxSize&) = 0;

public:
	struct Toolbar
	{
		wxPoint position;
		long orientation;
		bool transparent;
		bool full_screen;
	};

	virtual const Toolbar& toolbar() const = 0;
	virtual void set_toolbar_position(const wxPoint&) = 0;
	virtual void set_toolbar_orientation(long) = 0;
	virtual void set_toolbar_transparent(bool) = 0;
	virtual void set_toolbar_full_screen(bool) = 0;

public:
	struct AISAlertDialog
	{
		wxPoint position;
		wxSize size;
	};

	virtual const AISAlertDialog& ais_alert_dialog() const = 0;
	virtual void set_ais_alert_dialog_position(const wxPoint&) = 0;
	virtual void set_ais_alert_dialog_size(const wxSize&) = 0;

public:
	struct AISQueryDialog
	{
		wxPoint position;
	};

	virtual const AISQueryDialog& ais_query_dialog() const = 0;
	virtual void set_ais_query_dialog_position(const wxPoint&) = 0;

public:
	struct AISTargetList
	{
		wxString perspective;
		int range;
		int sortColumn;
		bool sortReverse;
		wxString column_spec;
	};

	virtual const AISTargetList& ais_target_list() const = 0;
	virtual void set_ais_target_list_perspective(const wxString&) = 0;
	virtual void set_ais_target_list_range(int) = 0;
	virtual void set_ais_target_list_sortColumn(int) = 0;
	virtual void set_ais_target_list_sortReverse(bool) = 0;
	virtual void set_ais_target_list_column_spec(const wxString&) = 0;

public:
	struct CM93
	{
		int zoom_factor;
		wxPoint detail_dialog_position;
		bool show_detail_slider;
	};

	virtual const CM93& cm93() const = 0;
	virtual void set_cm93_zoom_factor(int) = 0;
	virtual void set_cm93_detail_dialog_position(const wxPoint&) = 0;
	virtual void set_cm93_show_detail_slider(bool) = 0;

public:
	struct S57Dialog
	{
		wxSize size;
	};

	virtual const S57Dialog& s57dialog() const = 0;
	virtual void set_S57_dialog_size(const wxSize&) = 0;

public:
	struct OwnShip
	{
		int cog_predictor_width;
		double length_meters;
		double beam_meters;
	};

	virtual const OwnShip& ownship() const = 0;
	virtual void set_ownship_cog_predictor_width(int) = 0;
	virtual void set_ownship_length_meters(double) = 0;
	virtual void set_ownship_beam_meters(double) = 0;
};

}

#endif
