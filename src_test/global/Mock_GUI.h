#include <gmock/gmock.h>
#include <global/GUI.h>

namespace test {
namespace global {

class Mock_GUI : public ::global::GUI
{
public: // view
	MOCK_CONST_METHOD0(view, const ::global::GUI::View&());
	MOCK_METHOD1(set_view_screen_brightness, void(int));
	MOCK_METHOD1(set_view_show_outlines, void(bool));
	MOCK_METHOD1(set_view_show_depth_units, void(bool));
	MOCK_METHOD1(set_view_lookahead_mode, void(bool));
	MOCK_METHOD1(set_view_allow_overzoom_x, void(bool));
	MOCK_METHOD1(set_view_show_overzoom_emboss, void(bool));
	MOCK_METHOD1(set_route_line_width, void(int));
	MOCK_METHOD1(set_track_line_width, void(int));
	MOCK_METHOD1(set_enable_zoom_to_cursor, void(bool));
	MOCK_METHOD1(set_DrawAISSize, void(bool));
	MOCK_METHOD1(set_ShowAISName, void(bool));
	MOCK_METHOD1(set_Show_Target_Name_Scale, void(int));
	MOCK_METHOD1(set_WayPointPreventDragging, void(bool));
	MOCK_METHOD1(set_ConfirmObjectDelete, void(bool));
	MOCK_METHOD1(set_color_scheme, void(::global::ColorScheme));
	MOCK_METHOD1(set_initial_scale_ppm, void(double));
	MOCK_METHOD1(set_smooth_pan_zoom, void(bool));
	MOCK_METHOD1(set_view_display_grid, void(bool));
	MOCK_METHOD1(set_view_show_layers, void(bool));
	MOCK_METHOD1(set_view_permanent_mob_icon, void(bool));
	MOCK_METHOD1(set_view_show_active_route_highway, void(bool));
	MOCK_METHOD1(set_auto_anchor_mark, void(bool));
	MOCK_METHOD1(set_view_preserve_scale_on_x, void(bool));
	MOCK_METHOD1(set_view_quilt_enable, void(bool));
	MOCK_METHOD1(set_view_fullscreen_quilt, void(bool));
	MOCK_METHOD1(set_NavAidRadarRingsNumberVisible, void(int));
	MOCK_METHOD1(set_NavAidRadarRingsStep, void(double));
	MOCK_METHOD1(set_NavAidRadarRingsStepUnits, void(int));
	MOCK_METHOD1(set_ShowMag, void(bool));
	MOCK_METHOD1(set_current_arrow_scale, void(int));
	MOCK_METHOD1(set_skew_comp, void(bool));
	MOCK_METHOD1(set_opengl, void(bool));
	MOCK_METHOD1(set_disable_opengl, void(bool));

public: // frame
	MOCK_CONST_METHOD0(frame, const ::global::GUI::Frame&());
	MOCK_METHOD1(set_frame_position, void(const wxPoint&));
	MOCK_METHOD1(set_frame_size, void(const wxSize&));
	MOCK_METHOD1(set_frame_maximized, void(bool));
	MOCK_METHOD1(set_frame_last_position, void(const wxPoint&));
	MOCK_METHOD1(set_frame_last_size, void(const wxSize&));

public: // toolbar
	MOCK_CONST_METHOD0(toolbar, const ::global::GUI::Toolbar&());
	MOCK_METHOD1(set_toolbar_position, void(const wxPoint&));
	MOCK_METHOD1(set_toolbar_orientation, void(long));
	MOCK_METHOD1(set_toolbar_transparent, void(bool));
	MOCK_METHOD1(set_toolbar_full_screen, void(bool));
	MOCK_METHOD1(set_toolbar_config, void(const wxString&));
	MOCK_METHOD2(set_toolbar_config_at, void(int, wxChar));

public: // ais alert dialog
	MOCK_CONST_METHOD0(ais_alert_dialog, const ::global::GUI::AISAlertDialog&());
	MOCK_METHOD1(set_ais_alert_dialog_position, void(const wxPoint&));
	MOCK_METHOD1(set_ais_alert_dialog_size, void(const wxSize&));

public: // ais query dialog
	MOCK_CONST_METHOD0(ais_query_dialog, const ::global::GUI::AISQueryDialog&());
	MOCK_METHOD1(set_ais_query_dialog_position, void(const wxPoint&));

public: // ais target list
	MOCK_CONST_METHOD0(ais_target_list, const ::global::GUI::AISTargetList&());
	MOCK_METHOD1(set_ais_target_list_perspective, void(const wxString&));
	MOCK_METHOD1(set_ais_target_list_range, void(int));
	MOCK_METHOD1(set_ais_target_list_sortColumn, void(int));
	MOCK_METHOD1(set_ais_target_list_sortReverse, void(bool));
	MOCK_METHOD1(set_ais_target_list_column_spec, void(const wxString&));

public: // cm93
	MOCK_CONST_METHOD0(cm93, const ::global::GUI::CM93&());
	MOCK_METHOD1(set_cm93_zoom_factor, void(int));
	MOCK_METHOD1(set_cm93_detail_dialog_position, void(const wxPoint&));
	MOCK_METHOD1(set_cm93_show_detail_slider, void(bool));

public: // s57 dialog
	MOCK_CONST_METHOD0(s57dialog, const ::global::GUI::S57Dialog&());
	MOCK_METHOD1(set_S57_dialog_size, void(const wxSize&));

public: // ownship
	MOCK_CONST_METHOD0(ownship, const ::global::GUI::OwnShip&());
	MOCK_METHOD1(set_ownship_cog_predictor_width, void(int));
	MOCK_METHOD1(set_ownship_length_meters, void(double));
	MOCK_METHOD1(set_ownship_beam_meters, void(double));
	MOCK_METHOD1(set_ownship_min_mm, void(int));
	MOCK_METHOD1(set_gps_antenna_offset_x, void(double));
	MOCK_METHOD1(set_gps_antenna_offset_y, void(double));
	MOCK_METHOD1(set_ownship_predictor_minutes, void(double));
	MOCK_METHOD1(set_ownship_icon_type, void(int));
};

}}

