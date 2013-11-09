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

#include "Config.h"

#include <dychart.h>
#include <ChartCanvas.h>
#include <MainFrame.h>
#include <Layer.h>
#include <ConnectionParams.h>
#include <WayPointman.h>
#include <Route.h>
#include <RoutePoint.h>
#include <NavObjectCollection.h>
#include <NavObjectChanges.h>
#include <StyleManager.h>
#include <NMEALogWindow.h>
#include <FontMgr.h>
#include <MessageBox.h>
#include <MicrosoftCompatibility.h>

#ifdef USE_S57
	#include <chart/s52plib.h>
#endif

#include <chart/ChartDatabase.h>
#include <chart/s52utils.h>

#include <global/OCPN.h>
#include <global/GUI.h>
#include <global/System.h>
#include <global/Navigation.h>
#include <global/WatchDog.h>

#include <wx/window.h>
#include <wx/dir.h>
#include <wx/tokenzr.h>
#include <wx/progdlg.h>

extern Config* pConfig;
extern ChartCanvas* cc1;
extern MainFrame* gFrame;
extern double g_ChartNotRenderScaleFactor;
extern int g_restore_stackindex;
extern int g_restore_dbindex;
extern RouteList* pRouteList;
extern LayerList* pLayerList;
extern int g_LayerIdx;
extern double vLat, vLon;
extern double initial_scale_ppm;
extern ColorScheme global_color_scheme;
extern bool g_bShowMag;
extern double g_UserVar;
extern wxArrayOfConnPrm* g_pConnectionParams;
extern wxString g_SENCPrefix;
extern wxString g_UserPresLibData;
extern WayPointman* pWayPointMan;
extern bool s_bSetSystemTime;
extern bool g_bDisplayGrid; // Flag indicating if grid is to be displayed
extern bool g_bPlayShipsBells;
extern bool g_bShowLayers;
extern bool g_bPermanentMOBIcon;
extern bool g_bAutoAnchorMark;
extern bool g_bskew_comp;
extern bool g_bopengl;
extern bool g_bdisable_opengl;
extern bool g_bsmoothpanzoom;
extern bool g_bShowActiveRouteHighway;
extern int g_nNMEADebug;
extern int g_nAWDefault;
extern int g_nAWMax;
extern int g_nTrackPrecision;
extern int g_iSDMMFormat;
extern int g_iDistanceFormat;
extern int g_iSpeedFormat;
extern double g_PlanSpeed;
extern wxRect g_blink_rect;

//    AIS Global configuration
extern bool             g_bCPAMax;
extern double           g_CPAMax_NM;
extern bool             g_bCPAWarn;
extern double           g_CPAWarn_NM;
extern bool             g_bTCPA_Max;
extern double           g_TCPA_Max;
extern bool             g_bMarkLost;
extern double           g_MarkLost_Mins;
extern bool             g_bRemoveLost;
extern double           g_RemoveLost_Mins;
extern bool             g_bShowCOG;
extern double           g_ShowCOG_Mins;
extern bool             g_bAISShowTracks;
extern bool             g_bTrackCarryOver;
extern bool             g_bTrackDaily;
extern double           g_AISShowTracks_Mins;
extern bool             g_bShowMoored;
extern double           g_ShowMoored_Kts;
extern bool             g_bAIS_CPA_Alert;
extern bool             g_bAIS_CPA_Alert_Audio;
extern wxString         g_sAIS_Alert_Sound_File;
extern bool             g_bAIS_CPA_Alert_Suppress_Moored;
extern bool             g_bAIS_ACK_Timeout;
extern double           g_AckTimeout_Mins;
extern wxString         g_AisTargetList_perspective;
extern int              g_AisTargetList_range;
extern int              g_AisTargetList_sortColumn;
extern bool             g_bAisTargetList_sortReverse;
extern wxString         g_AisTargetList_column_spec;
extern bool             g_bShowAreaNotices;
extern bool             g_bDrawAISSize;
extern bool             g_bShowAISName;
extern int              g_Show_Target_Name_Scale;
extern bool             g_bWplIsAprsPosition;

extern int              g_S57_dialog_sx, g_S57_dialog_sy;

extern int              g_iNavAidRadarRingsNumberVisible;
extern float            g_fNavAidRadarRingsStep;
extern int              g_pNavAidRadarRingsStepUnits;
extern bool             g_bWayPointPreventDragging;
extern bool             g_bConfirmObjectDelete;

extern bool             g_bEnableZoomToCursor;
extern wxString         g_toolbarConfig;
extern double           g_TrackIntervalSeconds;
extern double           g_TrackDeltaDistance;

extern int              g_nCacheLimit;
extern int              g_memCacheLimit;

extern bool             g_bGDAL_Debug;
extern bool             g_bDebugCM93;
extern bool             g_bDebugS57;

extern double           g_ownship_predictor_minutes;

#ifdef USE_S57
extern s52plib          *ps52plib;
#endif

extern bool             g_bUseGreenShip;

extern bool             g_bshow_overzoom_emboss;
extern int              g_nautosave_interval_seconds;
extern int              g_OwnShipIconType;
extern double           g_n_ownship_length_meters;
extern double           g_n_ownship_beam_meters;
extern double           g_n_gps_antenna_offset_y;
extern double           g_n_gps_antenna_offset_x;
extern int              g_n_ownship_min_mm;
extern double           g_n_arrival_circle_radius;

extern bool             g_bPreserveScaleOnX;

extern bool             g_bUseGLL;

extern wxString         g_locale;

extern bool             g_bUseRaster;
extern bool             g_bUseVector;
extern bool             g_bUseCM93;

extern bool             g_bCourseUp;
extern int              g_COGAvgSec;
extern bool             g_bMagneticAPB;

extern int              g_MemFootSec;
extern int              g_MemFootMB;

extern int              g_nCOMPortCheck;

extern bool             g_bbigred;

extern wxString         g_AW1GUID;
extern wxString         g_AW2GUID;
extern int              g_BSBImgDebug;

extern bool             g_bAISRolloverShowClass;
extern bool             g_bAISRolloverShowCOG;
extern bool             g_bAISRolloverShowCPA;

extern bool             g_blocale_changed;

extern bool             g_bfilter_cogsog;
extern int              g_COGFilterSec;
extern int              g_SOGFilterSec;

int                     g_navobjbackups;

extern bool             g_bQuiltEnable;
extern bool             g_bFullScreenQuilt;
extern bool             g_bQuiltStart;

extern int              g_SkewCompUpdatePeriod;

extern int              g_GPU_MemSize;

extern bool             g_bHighliteTracks;
extern int              g_cog_predictor_width;
extern int              g_ais_cog_predictor_width;

extern int              g_route_line_width;
extern int              g_track_line_width;
extern wxString         g_default_wp_icon;

extern ChartGroupArray  *g_pGroupArray;
extern int              g_GroupIndex;

extern bool             g_bDebugOGL;
extern int              g_current_arrow_scale;
extern wxString         g_GPS_Ident;
extern bool             g_bGarminHostUpload;
extern wxString         g_uploadConnection;

extern ocpnStyle::StyleManager * g_StyleManager;
extern wxArrayString    TideCurrentDataSet;

Config::Config(
		const wxString & appName,
		const wxString & vendorName,
		const wxString & LocalFileName)
	: wxFileConfig(appName, vendorName, LocalFileName, wxString(_T("")))
{
	// Create the default nav object collection FileName
	wxFileName config_file(LocalFileName);
	m_sNavObjSetFile = config_file.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	m_sNavObjSetFile += _T("navobj.xml");
	m_sNavObjSetChangesFile = m_sNavObjSetFile + _T(".changes");

	m_pNavObjectInputSet = NULL;
	m_pNavObjectChangesSet = new NavObjectChanges();

	m_bSkipChangeSetUpdate = false;

	g_pConnectionParams = new wxArrayOfConnPrm();
}

void Config::CreateRotatingNavObjBackup()
{
	//Rotate navobj backups
	if (g_navobjbackups > 0) {
		for (int i = g_navobjbackups - 1; i >= 1; --i) {
			if (wxFile::Exists(wxString::Format( _T("%s.%d"), m_sNavObjSetFile.c_str(), i)))
				wxCopyFile(
					wxString::Format(_T("%s.%d"), m_sNavObjSetFile.c_str(), i),
					wxString::Format(_T("%s.%d"), m_sNavObjSetFile.c_str(), i + 1));
		}
		if (wxFile::Exists(m_sNavObjSetFile))
			wxCopyFile(
				m_sNavObjSetFile,
				wxString::Format(_T("%s.1"), m_sNavObjSetFile.c_str()));
	}

	// try to clean the backups the user doesn't want - breaks if he deleted some by
	// hand as it tries to be effective...
	for (int i = g_navobjbackups + 1; i <= 99; ++i)
		if (wxFile::Exists(wxString::Format(_T("%s.%d"), m_sNavObjSetFile.c_str(), i)))
			wxRemoveFile(wxString::Format(_T("%s.%d"), m_sNavObjSetFile.c_str(), i));
		else
			break;
}

void Config::load_toolbar()
{
	global::GUI & gui = global::OCPN::get().gui();

	int x = 0;
	int y = 0;
	long orientation;
	long transparent = 1;
	long full_screen = 1;

	Read(_T("ToolbarX"), &x, 0);
	Read(_T("ToolbarY"), &y, 0);
	Read(_T("ToolbarOrient"), &orientation, wxTB_HORIZONTAL);
	Read(_T("TransparentToolbar"), &transparent, 1);
	Read(_T("FullscreenToolbar"), &full_screen, 1);

	gui.set_toolbar_position(wxPoint(x, y));
	gui.set_toolbar_orientation(orientation);
	gui.set_toolbar_transparent(transparent);
	gui.set_toolbar_full_screen(full_screen);
}

void Config::load_ais_alert_dialog()
{
	global::GUI & gui = global::OCPN::get().gui();

	long size_x = 200;
	long size_y = 200;
	long pos_x = 0;
	long pos_y = 0;

	Read(_T("AlertDialogSizeX"), &size_x);
	Read(_T("AlertDialogSizeY"), &size_y);
	Read(_T("AlertDialogPosX"), &pos_x);
	Read(_T("AlertDialogPosY"), &pos_y);

	gui.set_ais_alert_dialog_position(wxPoint(pos_x, pos_y));
	gui.set_ais_alert_dialog_size(wxSize(size_x, size_y));
}

void Config::load_ais_query_dialog()
{
	global::GUI & gui = global::OCPN::get().gui();

	long x = 200;
	long y = 200;

	Read(_T("QueryDialogPosX"), &x);
	Read(_T("QueryDialogPosY"), &y);

	gui.set_ais_query_dialog_position(wxPoint(x, y));
}

void Config::load_frame()
{
	global::GUI & gui = global::OCPN::get().gui();

	long size_x = 0;
	long size_y = 0;
	long pos_x = 0;
	long pos_y = 0;
	bool maximized = false;

	Read(_T("FrameWinX"), &size_x);
	Read(_T("FrameWinY"), &size_y);
	Read(_T("FrameWinPosX"), &pos_x, 0);
	Read(_T("FrameWinPosY"), &pos_y, 0);
	gui.set_frame_position(wxPoint(pos_x, pos_y));
	gui.set_frame_size(wxSize(size_x, size_y));

	Read(_T("FrameMax"), &maximized);
	gui.set_frame_maximized(maximized);

	Read(_T("ClientPosX"), &pos_x, 0);
	Read(_T("ClientPosY"), &pos_y, 0);
	Read(_T("ClientSzX"), &size_x, 0);
	Read(_T("ClientSzY"), &size_y, 0);
	gui.set_frame_last_position(wxPoint(pos_x, pos_y));
	gui.set_frame_last_size(wxSize(size_x, size_y));
}

void Config::load_view()
{
	global::GUI & gui = global::OCPN::get().gui();

	long brightness = 100;
	long show_outlines = 0;
	long show_depth_units = 1;
	long lookahead_mode = 0;

	Read(_T("ScreenBrightness"), &brightness, 100);
	Read(_T("ShowChartOutlines"), &show_outlines, 0);
	Read(_T("ShowDepthUnits"), &show_depth_units, 1);
	Read(_T("LookAheadMode"), &lookahead_mode, 0);

	gui.set_view_screen_brightness(brightness);
	gui.set_view_show_outlines(show_outlines);
	gui.set_view_show_depth_units(show_depth_units);
	gui.set_view_lookahead_mode(lookahead_mode);
}

void Config::load_system_config(int iteration) // FIXME: get rid of this 'iteration'
{
	global::System & sys = global::OCPN::get().sys();

	if (iteration == 0) {
		wxString version_string = _T("");
		long nav_message_shown = 0;

		Read(_T("ConfigVersionString"), &version_string, _T(""));
		Read(_T("NavMessageShown"), &nav_message_shown, 0);

		sys.set_config_version_string(version_string);
		sys.set_config_nav_message_shown(nav_message_shown);
	}
}

void Config::load_watchdog()
{
	global::WatchDog & wdt = global::OCPN::get().wdt();

	long gps_watchdog_timeout_ticks;

	Read(_T("GPSDogTimeout"), &gps_watchdog_timeout_ticks, GPS_TIMEOUT_SECONDS);

	wdt.set_gps_timeout_ticks(gps_watchdog_timeout_ticks);
}

void Config::load_cm93(int display_width, int display_height)
{
#ifdef USE_S57
	global::GUI & gui = global::OCPN::get().gui();

#define CM93_ZOOM_FACTOR_MAX_RANGE 5 // FIXME: better solution (maybe over global infrastructure)

	int zoom_factor = 0;
	long pos_x = 200;
	long pos_y = 200;
	long show_detail_slider = 0;

	Read(_T("CM93DetailFactor"), &zoom_factor, 0);
	zoom_factor = wxMin(zoom_factor, CM93_ZOOM_FACTOR_MAX_RANGE);
	zoom_factor = wxMax(zoom_factor, -CM93_ZOOM_FACTOR_MAX_RANGE);

	Read(_T("CM93DetailZoomPosX"), &pos_x, 200L);
	Read(_T("CM93DetailZoomPosY"), &pos_y, 200L);
	if ((pos_x < 0) || (pos_x > display_width))
		pos_x = 5;
	if ((pos_y < 0) || (pos_y > display_height))
		pos_y = 5;

	Read(_T("ShowCM93DetailSlider"), &show_detail_slider, 0);

	gui.set_cm93_zoom_factor(zoom_factor);
	gui.set_cm93_detail_dialog_position(wxPoint(pos_x, pos_y));
	gui.set_cm93_show_detail_slider(show_detail_slider);
#endif
}

int Config::LoadConfig(int iteration) // FIXME: get rid of this 'iteration'
{
	int read_int;

	int display_width;
	int display_height;
	wxDisplaySize(&display_width, &display_height);

	//    Global options and settings
	SetPath(_T("/Settings"));

	load_system_config(iteration);

	wxString uiStyle;
	Read( _T ( "UIStyle" ), &uiStyle, wxT("") );
	g_StyleManager->SetStyle( uiStyle );

	if( iteration == 0 ) {
		Read( _T ( "NCacheLimit" ), &g_nCacheLimit, CACHE_N_LIMIT_DEFAULT );

		int mem_limit;
		Read( _T ( "MEMCacheLimit" ), &mem_limit, 0 );

		if(mem_limit > 0)
			g_memCacheLimit = mem_limit * 1024;       // convert from MBytes to kBytes
	}

	Read( _T ( "DebugGDAL" ), &g_bGDAL_Debug, 0 );
	Read( _T ( "DebugNMEA" ), &g_nNMEADebug, 0 );
	Read( _T ( "DebugOpenGL" ), &g_bDebugOGL, 0 );
	Read( _T ( "AnchorWatchDefault" ), &g_nAWDefault, 50 );
	Read( _T ( "AnchorWatchMax" ), &g_nAWMax, 1852 );
	Read( _T ( "DebugCM93" ), &g_bDebugCM93, 0 );
	Read( _T ( "DebugS57" ), &g_bDebugS57, 0 );         // Show LUP and Feature info in object query
	Read( _T ( "DebugBSBImg" ), &g_BSBImgDebug, 0 );

	load_watchdog();

	Read( _T ( "UseGreenShipIcon" ), &g_bUseGreenShip, 0 );

	// overzoom
	long allow_overzoom_x = 1;
	Read(_T("AllowExtremeOverzoom"), &allow_overzoom_x, 1);
	global::OCPN::get().gui().set_view_allow_overzoom_x(allow_overzoom_x);

	Read( _T ( "ShowOverzoomEmbossWarning" ), &g_bshow_overzoom_emboss, 1 );
	Read( _T ( "AutosaveIntervalSeconds" ), &g_nautosave_interval_seconds, 300 );

	Read( _T ( "GPSIdent" ), &g_GPS_Ident, wxT("Generic") );
	Read( _T ( "UseGarminHostUpload" ),  &g_bGarminHostUpload, 0 );

	Read( _T ( "UseNMEA_GLL" ), &g_bUseGLL, 1 );
	Read( _T ( "UseBigRedX" ), &g_bbigred, 0 );

	Read( _T ( "FilterNMEA_Avg" ), &g_bfilter_cogsog, 0 );
	Read( _T ( "FilterNMEA_Sec" ), &g_COGFilterSec, 1 );
	g_COGFilterSec = wxMin(g_COGFilterSec, MAX_COGSOG_FILTER_SECONDS);
	g_COGFilterSec = wxMax(g_COGFilterSec, 1);
	g_SOGFilterSec = g_COGFilterSec;

	Read( _T ( "ShowMag" ), &g_bShowMag, 0 );
	g_UserVar = 0.0;
	wxString umv;
	Read( _T ( "UserMagVariation" ), &umv );
	if(umv.Len())
		umv.ToDouble( &g_UserVar );

	Read( _T ( "UseMagAPB" ), &g_bMagneticAPB, 0 );

	load_view();

	Read( _T ( "MemFootprintMgrTimeSec" ), &g_MemFootSec, 60 );
	Read( _T ( "MemFootprintTargetMB" ), &g_MemFootMB, 200 );

	Read( _T ( "WindowsComPortMax" ), &g_nCOMPortCheck, 32 );

	Read( _T ( "ChartQuilting" ), &g_bQuiltEnable, 0 );
	Read( _T ( "ChartQuiltingInitial" ), &g_bQuiltStart, 0 );

	Read( _T ( "UseRasterCharts" ), &g_bUseRaster, 1 );             // default is true......
	Read( _T ( "UseVectorCharts" ), &g_bUseVector, 0 );
	Read( _T ( "UseCM93Charts" ), &g_bUseCM93, 0 );

	Read( _T ( "CourseUpMode" ), &g_bCourseUp, 0 );
	Read( _T ( "COGUPAvgSeconds" ), &g_COGAvgSec, 15 );
	g_COGAvgSec = wxMin(g_COGAvgSec, MAX_COG_AVERAGE_SECONDS);        // Bound the array size
	Read( _T ( "SkewToNorthUp" ), &g_bskew_comp, 0 );
	Read( _T ( "OpenGL" ), &g_bopengl, 0 );
	if (g_bdisable_opengl)
		g_bopengl = false;

	Read( _T ( "ActiveChartGroup" ), &g_GroupIndex, 0 );

	Read( _T ( "GPUMemorySize" ), &g_GPU_MemSize, 256 );

	Read( _T ( "SmoothPanZoom" ), &g_bsmoothpanzoom, 0 );

	load_toolbar();
	Read( _T ( "ToolbarConfig" ), &g_toolbarConfig );

	Read( _T ( "AnchorWatch1GUID" ), &g_AW1GUID, _T("") );
	Read( _T ( "AnchorWatch2GUID" ), &g_AW2GUID, _T("") );

	Read( _T ( "InitialStackIndex" ), &g_restore_stackindex, 0 );
	Read( _T ( "InitialdBIndex" ), &g_restore_dbindex, -1 );

	Read( _T ( "ChartNotRenderScaleFactor" ), &g_ChartNotRenderScaleFactor, 1.5 );

	load_cm93(display_width, display_height);

	Read( _T ( "SkewCompUpdatePeriod" ), &g_SkewCompUpdatePeriod, 10 );

	Read( _T ( "SetSystemTime" ), &s_bSetSystemTime, 0 );
	Read( _T ( "ShowDebugWindows" ), &m_bShowDebugWindows, 1 );
	Read( _T ( "ShowGrid" ), &g_bDisplayGrid, 0 );
	Read( _T ( "PlayShipsBells" ), &g_bPlayShipsBells, 0 );
	Read( _T ( "PermanentMOBIcon" ), &g_bPermanentMOBIcon, 0 );
	Read( _T ( "ShowLayers" ), &g_bShowLayers, 1 );
	Read( _T ( "AutoAnchorDrop" ), &g_bAutoAnchorMark, 0 );
	Read( _T ( "ShowActiveRouteHighway" ), &g_bShowActiveRouteHighway, 1 );
	Read( _T ( "MostRecentGPSUploadConnection" ), &g_uploadConnection, _T("") );

	Read( _T ( "SDMMFormat" ), &g_iSDMMFormat, 0 ); //0 = "Degrees, Decimal minutes"), 1 = "Decimal degrees", 2 = "Degrees,Minutes, Seconds"
	Read( _T ( "DistanceFormat" ), &g_iDistanceFormat, 0 ); //0 = "Nautical miles"), 1 = "Statute miles", 2 = "Kilometers", 3 = "Meters"
	Read( _T ( "SpeedFormat" ), &g_iSpeedFormat, 0 ); //0 = "kts"), 1 = "mph", 2 = "km/h", 3 = "m/s"

	Read( _T ( "OwnshipCOGPredictorMinutes" ), &g_ownship_predictor_minutes, 5 );
	Read( _T ( "OwnshipCOGPredictorWidth" ), &g_cog_predictor_width, 3 );
	Read( _T ( "OwnShipIconType" ), &g_OwnShipIconType, 0 );
	Read( _T ( "OwnShipLength" ), &g_n_ownship_length_meters, 0 );
	Read( _T ( "OwnShipWidth" ), &g_n_ownship_beam_meters, 0 );
	Read( _T ( "OwnShipGPSOffsetX" ), &g_n_gps_antenna_offset_x, 0 );
	Read( _T ( "OwnShipGPSOffsetY" ), &g_n_gps_antenna_offset_y, 0 );
	Read( _T ( "OwnShipMinSize" ), &g_n_ownship_min_mm, 1 );
	g_n_ownship_min_mm = wxMax(g_n_ownship_min_mm, 1);

	g_n_arrival_circle_radius = .050;           // default
	wxString racr;
	Read( _T ( "RouteArrivalCircleRadius" ), &racr );
	if(racr.Len())
		racr.ToDouble( &g_n_arrival_circle_radius);
	g_n_arrival_circle_radius = wxMax(g_n_arrival_circle_radius, .001);

	Read( _T ( "FullScreenQuilt" ), &g_bFullScreenQuilt, 1 );

	Read( _T ( "StartWithTrackActive" ), &g_bTrackCarryOver, 0 );
	Read( _T ( "AutomaticDailyTracks" ), &g_bTrackDaily, 0 );
	Read( _T ( "HighlightTracks" ), &g_bHighliteTracks, 1 );

	wxString stps;
	Read( _T ( "PlanSpeed" ), &stps );
	stps.ToDouble( &g_PlanSpeed );

	Read( _T ( "VisibleLayers" ), &visibleLayers );
	Read( _T ( "InvisibleLayers" ), &invisibleLayers );

	Read( _T ( "PreserveScaleOnX" ), &g_bPreserveScaleOnX, 0 );

	if( iteration == 0 ) {
		g_locale = _T("en_US");
		Read( _T ( "Locale" ), &g_locale );
	}

	//We allow 0-99 backups ov navobj.xml
	Read( _T ( "KeepNavobjBackups" ), &g_navobjbackups, 5 );
	if( g_navobjbackups > 99 ) g_navobjbackups = 99;
	if( g_navobjbackups < 0 ) g_navobjbackups = 0;

	NMEALogWindow::Get().SetSize(Read(_T("NMEALogWindowSizeX"), 600L), Read(_T("NMEALogWindowSizeY"), 400L));
	NMEALogWindow::Get().SetPos(Read(_T("NMEALogWindowPosX"), 10L), Read(_T("NMEALogWindowPosY"), 10L));
	NMEALogWindow::Get().CheckPos(display_width, display_height);

	SetPath( _T ( "/Settings/GlobalState" ) );
	Read( _T ( "bFollow" ), &st_bFollow );

	load_frame();

	//    AIS
	wxString s;
	SetPath( _T ( "/Settings/AIS" ) );

	Read( _T ( "bNoCPAMax" ), &g_bCPAMax );

	Read( _T ( "NoCPAMaxNMi" ), &s );
	s.ToDouble( &g_CPAMax_NM );

	Read( _T ( "bCPAWarn" ), &g_bCPAWarn );

	Read( _T ( "CPAWarnNMi" ), &s );
	s.ToDouble( &g_CPAWarn_NM );

	Read( _T ( "bTCPAMax" ), &g_bTCPA_Max );

	Read( _T ( "TCPAMaxMinutes" ), &s );
	s.ToDouble( &g_TCPA_Max );

	Read( _T ( "bMarkLostTargets" ), &g_bMarkLost );

	Read( _T ( "MarkLost_Minutes" ), &s );
	s.ToDouble( &g_MarkLost_Mins );

	Read( _T ( "bRemoveLostTargets" ), &g_bRemoveLost );

	Read( _T ( "RemoveLost_Minutes" ), &s );
	s.ToDouble( &g_RemoveLost_Mins );

	Read( _T ( "bShowCOGArrows" ), &g_bShowCOG );

	Read( _T ( "CogArrowMinutes" ), &s );
	s.ToDouble( &g_ShowCOG_Mins );

	Read( _T ( "bShowTargetTracks" ), &g_bAISShowTracks, 0 );

	if( Read( _T ( "TargetTracksMinutes" ), &s ) ) {
		s.ToDouble( &g_AISShowTracks_Mins );
		g_AISShowTracks_Mins = wxMax(1.0, g_AISShowTracks_Mins);
		g_AISShowTracks_Mins = wxMin(60.0, g_AISShowTracks_Mins);
	} else
		g_AISShowTracks_Mins = 20;

	Read( _T ( "bShowMooredTargets" ), &g_bShowMoored );

	Read( _T ( "MooredTargetMaxSpeedKnots" ), &s );
	s.ToDouble( &g_ShowMoored_Kts );

	Read( _T ( "bShowAreaNotices" ), &g_bShowAreaNotices );
	Read( _T ( "bDrawAISSize" ), &g_bDrawAISSize );
	Read( _T ( "bShowAISName" ), &g_bShowAISName );
	Read( _T ( "bAISAlertDialog" ), &g_bAIS_CPA_Alert );
	g_Show_Target_Name_Scale = Read( _T ( "ShowAISTargetNameScale" ), 250000L );
	g_Show_Target_Name_Scale = wxMax( 5000, g_Show_Target_Name_Scale );
	Read( _T ( "bWplIsAprsPositionReport" ), &g_bWplIsAprsPosition, 1 );
	Read( _T ( "AISCOGPredictorWidth" ), &g_ais_cog_predictor_width, 3 );

	Read( _T ( "bAISAlertAudio" ), &g_bAIS_CPA_Alert_Audio );
	Read( _T ( "AISAlertAudioFile" ), &g_sAIS_Alert_Sound_File );
	Read( _T ( "bAISAlertSuppressMoored" ), &g_bAIS_CPA_Alert_Suppress_Moored );

	Read( _T ( "bAISAlertAckTimeout" ), &g_bAIS_ACK_Timeout, 0 );
	Read( _T ( "AlertAckTimeoutMinutes" ), &s );
	s.ToDouble( &g_AckTimeout_Mins );

	load_ais_alert_dialog();
	load_ais_query_dialog();

	Read( _T ( "AISTargetListPerspective" ), &g_AisTargetList_perspective );
	g_AisTargetList_range = Read( _T ( "AISTargetListRange" ), 40L );
	g_AisTargetList_sortColumn = Read( _T ( "AISTargetListSortColumn" ), 2L ); // Column #2 is MMSI
	Read( _T ( "bAISTargetListSortReverse" ), &g_bAisTargetList_sortReverse, false );
	Read( _T ( "AISTargetListColumnSpec" ), &g_AisTargetList_column_spec );

	Read( _T ( "bAISRolloverShowClass" ), &g_bAISRolloverShowClass );
	Read( _T ( "bAISRolloverShowCOG" ), &g_bAISRolloverShowCOG );
	Read( _T ( "bAISRolloverShowCPA" ), &g_bAISRolloverShowCPA );

	g_S57_dialog_sx = Read( _T ( "S57QueryDialogSizeX" ), 400L );
	g_S57_dialog_sy = Read( _T ( "S57QueryDialogSizeY" ), 400L );

#ifdef USE_S57
	if( NULL != ps52plib ) {
		double dval;
		SetPath( _T ( "/Settings/GlobalState" ) );

		Read( _T ( "bShowS57Text" ), &read_int, 0 );
		ps52plib->SetShowS57Text( !( read_int == 0 ) );

		Read( _T ( "bShowS57ImportantTextOnly" ), &read_int, 0 );
		ps52plib->SetShowS57ImportantTextOnly( !( read_int == 0 ) );

		Read( _T ( "bShowLightDescription" ), &read_int, 0 );
		ps52plib->SetShowLdisText( !( read_int == 0 ) );

		Read( _T ( "bExtendLightSectors" ), &read_int, 0 );
		ps52plib->SetExtendLightSectors( !( read_int == 0 ) );

		Read( _T ( "nDisplayCategory" ), &read_int, (enum DisCat) STANDARD );
		ps52plib->m_nDisplayCategory = (enum DisCat) read_int;

		Read( _T ( "nSymbolStyle" ), &read_int, (enum LUPname) PAPER_CHART );
		ps52plib->m_nSymbolStyle = (LUPname) read_int;

		Read( _T ( "nBoundaryStyle" ), &read_int, PLAIN_BOUNDARIES );
		ps52plib->m_nBoundaryStyle = (LUPname) read_int;

		Read( _T ( "bShowSoundg" ), &read_int, 0 );
		ps52plib->m_bShowSoundg = !( read_int == 0 );

		Read( _T ( "bShowMeta" ), &read_int, 0 );
		ps52plib->m_bShowMeta = !( read_int == 0 );

		Read( _T ( "bUseSCAMIN" ), &read_int, 0 );
		ps52plib->m_bUseSCAMIN = !( read_int == 0 );

		Read( _T ( "bShowAtonText" ), &read_int, 0 );
		ps52plib->m_bShowAtonText = !( read_int == 0 );

		Read( _T ( "bDeClutterText" ), &read_int, 0 );
		ps52plib->m_bDeClutterText = !( read_int == 0 );

		Read( _T ( "bShowNationalText" ), &read_int, 0 );
		ps52plib->m_bShowNationalTexts = !( read_int == 0 );

		if( Read( _T ( "S52_MAR_SAFETY_CONTOUR" ), &dval, 5.0 ) ) {
			S52_setMarinerParam( S52_MAR_SAFETY_CONTOUR, dval );
			S52_setMarinerParam( S52_MAR_SAFETY_DEPTH, dval ); // Set safety_contour and safety_depth the same
		}

		if( Read( _T ( "S52_MAR_SHALLOW_CONTOUR" ), &dval, 3.0 ) ) S52_setMarinerParam(
				S52_MAR_SHALLOW_CONTOUR, dval );

		if( Read( _T ( "S52_MAR_DEEP_CONTOUR" ), &dval, 10.0 ) ) S52_setMarinerParam(
				S52_MAR_DEEP_CONTOUR, dval );

		if( Read( _T ( "S52_MAR_TWO_SHADES" ), &dval, 0.0 ) ) S52_setMarinerParam(
				S52_MAR_TWO_SHADES, dval );

		ps52plib->UpdateMarinerParams();

		SetPath( _T ( "/Settings/GlobalState" ) );
		Read( _T ( "S52_DEPTH_UNIT_SHOW" ), &read_int, 1 );   // default is metres
		ps52plib->m_nDepthUnitDisplay = read_int;
	}

	wxString strpres( _T ( "PresentationLibraryData" ) );
	wxString valpres;
	SetPath( _T ( "/Directories" ) );
	Read( strpres, &valpres );              // Get the File name
	if( iteration == 0 )
		g_UserPresLibData = valpres;

	wxString strs( _T ( "SENCFileLocation" ) );
	SetPath( _T ( "/Directories" ) );
	wxString vals;
	Read( strs, &vals );              // Get the Directory name

	if( iteration == 0 )
		g_SENCPrefix = vals;

#endif

	SetPath(_T("/Directories"));

	wxString init_chart_dir;
	Read(_T("InitChartDir"), &init_chart_dir); // Get the Directory name
	if (!init_chart_dir.IsEmpty()) {
		// don't overwrite on second pass
		if (global::OCPN::get().sys().data().init_chart_dir.IsEmpty()) {
			global::OCPN::get().sys().set_init_chart_dir(init_chart_dir);
		}
	}

	Read( _T ( "GPXIODir" ), &m_gpx_path );           // Get the Directory name

	wxString tc_data_directory;
	Read(_T("TCDataDir"), &tc_data_directory);
	global::OCPN::get().sys().set_tc_data_dir(tc_data_directory);

	SetPath( _T ( "/Settings/GlobalState" ) );
	Read( _T ( "nColorScheme" ), &read_int, 0 );
	global_color_scheme = (ColorScheme) read_int;

	SetPath( _T ( "/Settings/NMEADataSource" ) );

	wxString connectionconfigs;
	Read ( _T( "DataConnections" ),  &connectionconfigs, wxEmptyString );
	wxArrayString confs = wxStringTokenize(connectionconfigs, _T("|"));
	g_pConnectionParams->Clear();
	for (size_t i = 0; i < confs.Count(); i++)
	{
		ConnectionParams * prm = new ConnectionParams(confs[i]);
		g_pConnectionParams->Add(prm);
	}

	//  Automatically handle the upgrade to DataSources architecture...
	//  Capture Garmin host configuration
	SetPath( _T ( "/Settings" ) );
	int b_garmin_host;
	Read ( _T ( "UseGarminHost" ), &b_garmin_host );

	//  Is there an existing NMEADataSource definition?
	SetPath( _T ( "/Settings/NMEADataSource" ) );
	wxString xSource;
	wxString xRate;
	Read ( _T ( "Source" ), &xSource );
	Read ( _T ( "BaudRate" ), &xRate );
	if(xSource.Len()) {
		wxString port;
		if(xSource.Mid(0, 6) == _T("Serial"))
			port = xSource.Mid(7);
		else
			port = _T("");

		if( port.Len() && (port != _T("None")) && (port != _T("AIS Port (Shared)")) ) {
			//  Look in the ConnectionParams array to see if this port has been defined in the newer style
			bool bfound = false;
			for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
			{
				ConnectionParams *cp = g_pConnectionParams->Item(i);
				if(cp->GetAddressStr() == port) {
					bfound = true;
					break;
				}
			}

			if(!bfound) {
				ConnectionParams * prm = new ConnectionParams();
				prm->Baudrate = wxAtoi(xRate);
				prm->Port = port;
				prm->Garmin = (b_garmin_host == 1);

				g_pConnectionParams->Add(prm);

				g_bGarminHostUpload = (b_garmin_host == 1);
			}
		}
		if( iteration == 1 ) {
			Write ( _T ( "Source" ), _T("") );          // clear the old tag
			Write ( _T ( "BaudRate" ), _T("") );
		}
	}

	//  Is there an existing AISPort definition?
	SetPath( _T ( "/Settings/AISPort" ) );
	wxString aSource;
	wxString aRate;
	Read ( _T ( "Port" ), &aSource );
	Read ( _T ( "BaudRate" ), &aRate );
	if(aSource.Len()) {
		wxString port;
		if(aSource.Mid(0, 6) == _T("Serial"))
			port = aSource.Mid(7);
		else
			port = _T("");

		if(port.Len() && port != _T("None") ) {
			//  Look in the ConnectionParams array to see if this port has been defined in the newer style
			bool bfound = false;
			for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
			{
				ConnectionParams *cp = g_pConnectionParams->Item(i);
				if(cp->GetAddressStr() == port) {
					bfound = true;
					break;
				}
			}

			if(!bfound) {
				ConnectionParams * prm = new ConnectionParams();
				if( aRate.Len() )
					prm->Baudrate = wxAtoi(aRate);
				else
					prm->Baudrate = 38400;              // default for most AIS receivers
				prm->Port = port;

				g_pConnectionParams->Add(prm);
			}
		}

		if( iteration == 1 ) {
			Write ( _T ( "Port" ), _T("") );          // clear the old tag
			Write ( _T ( "BaudRate" ), _T("") );
		}
	}

	//  Is there an existing NMEAAutoPilotPort definition?
	SetPath( _T ( "/Settings/NMEAAutoPilotPort" ) );
	Read ( _T ( "Port" ), &xSource );
	if(xSource.Len()) {
		wxString port;
		if(xSource.Mid(0, 6) == _T("Serial"))
			port = xSource.Mid(7);
		else
			port = _T("");

		if(port.Len() && port != _T("None") ) {
			//  Look in the ConnectionParams array to see if this port has been defined in the newer style
			bool bfound = false;
			ConnectionParams *cp;
			for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
			{
				cp = g_pConnectionParams->Item(i);
				if(cp->GetAddressStr() == port) {
					bfound = true;
					break;
				}
			}

			if(!bfound) {
				ConnectionParams * prm = new ConnectionParams();
				prm->Port = port;
				prm->OutputSentenceListType = ConnectionParams::WHITELIST;
				prm->OutputSentenceList.Add(_T("RMB"));
				prm->Output = true;

				g_pConnectionParams->Add(prm);
			}
			else {                                  // port was found, so make sure it is set for output
				cp->Output = true;
				cp->OutputSentenceListType = ConnectionParams::WHITELIST;
				cp->OutputSentenceList.Add(_T("RMB"));
			}
		}

		if( iteration == 1 )
			Write ( _T ( "Port" ), _T("") );          // clear the old tag
	}

	//    Reasonable starting point
	vLat = START_LAT;                   // display viewpoint
	vLon = START_LON;

	global::Navigation & nav = global::OCPN::get().nav();

	// GPS position, as default
	nav.set_latitude(START_LAT);
	nav.set_longitude(START_LON);

	initial_scale_ppm = .0003;        // decent initial value

	SetPath( _T ( "/Settings/GlobalState" ) );
	wxString st;

	if( Read( _T ( "VPLatLon" ), &st ) ) {
		sscanf( st.mb_str( wxConvUTF8 ), "%lf,%lf", &st_lat, &st_lon );

		//    Sanity check the lat/lon...both have to be reasonable.
		if( fabs( st_lon ) < 360. ) {
			while( st_lon < -180. )
				st_lon += 360.;

			while( st_lon > 180. )
				st_lon -= 360.;

			vLon = st_lon;
		}

		if( fabs( st_lat ) < 90.0 ) vLat = st_lat;
	}
	s.Printf( _T ( "Setting Viewpoint Lat/Lon %g, %g" ), vLat, vLon );
	wxLogMessage( s );

	if( Read( wxString( _T ( "VPScale" ) ), &st ) ) {
		sscanf( st.mb_str( wxConvUTF8 ), "%lf", &st_view_scale );
		//    Sanity check the scale
		st_view_scale = fmax(st_view_scale, .001/32);
		st_view_scale = fmin(st_view_scale, 4);
		initial_scale_ppm = st_view_scale;
	}

	wxString sll;
	double lat, lon;
	if( Read( _T ( "OwnShipLatLon" ), &sll ) ) {
		sscanf( sll.mb_str( wxConvUTF8 ), "%lf,%lf", &lat, &lon );

		//    Sanity check the lat/lon...both have to be reasonable.
		if (fabs( lon ) < 360.0) {
			while (lon < -180.0)
				lon += 360.0;

			while (lon > 180.0)
				lon -= 360.0;

			nav.set_longitude(lon);
		}

		if (fabs(lat) < 90.0)
			nav.set_latitude(lat);
	}
	s.Printf(_T("Setting Ownship Lat/Lon %g, %g" ), nav.get_data().lat, nav.get_data().lon);
	wxLogMessage(s);

#ifdef USE_S57
	//    S57 Object Class Visibility

	OBJLElement *pOLE;

	SetPath( _T ( "/Settings/ObjectFilter" ) );

	if( ps52plib ) {
		int iOBJMax = GetNumberOfEntries();
		if( iOBJMax ) {

			wxString str;
			long val;
			long dummy;

			wxString sObj;

			bool bCont = pConfig->GetFirstEntry( str, dummy ); // FIXME: this is basically 'this'
			while( bCont ) {
				pConfig->Read( str, &val ); // Get an Object Viz

				bool bNeedNew = true;

				if( str.StartsWith( _T ( "viz" ), &sObj ) ) {
					for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->size(); iPtr++ ) {
						pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
						if( !strncmp( pOLE->OBJLName, sObj.mb_str(), 6 ) ) {
							pOLE->nViz = val;
							bNeedNew = false;
							break;
						}
					}

					if( bNeedNew ) {
						pOLE = (OBJLElement *) calloc( sizeof(OBJLElement), 1 );
						strncpy( pOLE->OBJLName, sObj.mb_str(), 6 );
						pOLE->nViz = 1;

						ps52plib->pOBJLArray->Add( (void *) pOLE );
					}
				}
				bCont = pConfig->GetNextEntry( str, dummy );
			}
		}
	}
#endif

	//    Fonts

#ifdef __WXX11__
	SetPath ( _T ( "/Settings/X11Fonts" ) );
#endif

#ifdef __WXGTK__
	SetPath ( _T ( "/Settings/GTKFonts" ) );
#endif

#ifdef __WXMSW__
	SetPath( _T ( "/Settings/MSWFonts" ) );
#endif

#ifdef __WXMAC__
	SetPath ( _T ( "/Settings/MacFonts" ) );
#endif

	if( 0 == iteration ) {
		wxString str;
		long dummy;
		wxString *pval = new wxString;
		wxArrayString deleteList;

		bool bCont = GetFirstEntry( str, dummy );
		while( bCont ) {
			Read( str, pval );

			if( str.StartsWith( _T("Font") ) ) {
				// Convert pre 3.1 setting. Can't delete old entries from inside the
				// GetNextEntry() loop, so we need to save those and delete outside.
				deleteList.Add( str );
				wxString oldKey = pval->BeforeFirst( _T(':') );
				str = FontMgr::GetFontConfigKey( oldKey );
			}

			FontMgr::Get().LoadFontNative(str, *pval);

			bCont = GetNextEntry( str, dummy );
		}

		for( unsigned int i=0; i<deleteList.Count(); i++ ) {
			DeleteEntry( deleteList[i] );
		}
		deleteList.Clear();
		delete pval;
	}

	//  Tide/Current Data Sources
	SetPath( _T ( "/TideCurrentDataSources" ) );
	TideCurrentDataSet.Clear();
	if( GetNumberOfEntries() ) {
		wxString str, val;
		long dummy;
		bool bCont = GetFirstEntry( str, dummy );
		while( bCont ) {
			Read( str, &val );              // Get a file name
			TideCurrentDataSet.Add(val);
			bCont = GetNextEntry( str, dummy );
		}
	}


	//    Layers
	if( 0 == iteration )
		pLayerList = new LayerList;

	//  Routes
	if( 0 == iteration )
		pRouteList = new RouteList;

	//    Groups
	if( 0 == iteration )
		LoadConfigGroups( g_pGroupArray );


	//      next thing to do is read tracks, etc from the NavObject XML file,
	if( 0 == iteration ) {
		CreateRotatingNavObjBackup();

		if( NULL == m_pNavObjectInputSet )
			m_pNavObjectInputSet = new NavObjectCollection();

		if( ::wxFileExists( m_sNavObjSetFile ) ) {
			if( m_pNavObjectInputSet->load_file( m_sNavObjSetFile.fn_str() ) )
				m_pNavObjectInputSet->LoadAllGPXObjects();
		}

		delete m_pNavObjectInputSet;


		if( ::wxFileExists( m_sNavObjSetChangesFile ) ) {
			//We crashed last time :(
			//That's why this file still exists...
			//Let's reconstruct the unsaved changes
			NavObjectChanges *pNavObjectChangesSet = new NavObjectChanges();
			pNavObjectChangesSet->load_file( m_sNavObjSetChangesFile.fn_str() );

			//  Remove the file before applying the changes,
			//  just in case the changes file itself causes a fault.
			//  If it does fault, at least the next restart will proceed without fault.
			if( ::wxFileExists( m_sNavObjSetChangesFile ) )
				::wxRemoveFile( m_sNavObjSetChangesFile );

			wxLogMessage( _T("Applying NavObjChanges") );
			pNavObjectChangesSet->ApplyChanges();
			delete pNavObjectChangesSet;

			UpdateNavObj();
		}
	}

	SetPath( _T ( "/Settings/Others" ) );

	// Radar rings
	wxString val;
	g_iNavAidRadarRingsNumberVisible = 0;
	Read( _T ( "RadarRingsNumberVisible" ), &val );
	if( val.Length() > 0 ) g_iNavAidRadarRingsNumberVisible = atoi( val.mb_str() );

	g_fNavAidRadarRingsStep = 1.0;
	Read( _T ( "RadarRingsStep" ), &val );
	if( val.Length() > 0 ) g_fNavAidRadarRingsStep = atof( val.mb_str() );

	g_pNavAidRadarRingsStepUnits = 0;
	Read( _T ( "RadarRingsStepUnits" ), &g_pNavAidRadarRingsStepUnits );

	//  Support Version 3.0 and prior config setting for Radar Rings
	bool b300RadarRings= true;
	Read ( _T ( "ShowRadarRings" ), &b300RadarRings );
	if(!b300RadarRings)
		g_iNavAidRadarRingsNumberVisible = 0;

	Read( _T ( "ConfirmObjectDeletion" ), &g_bConfirmObjectDelete, true );

	// Waypoint dragging with mouse
	g_bWayPointPreventDragging = false;
	Read( _T ( "WaypointPreventDragging" ), &g_bWayPointPreventDragging );

	g_bEnableZoomToCursor = false;
	Read( _T ( "EnableZoomToCursor" ), &g_bEnableZoomToCursor );

	g_TrackIntervalSeconds = 60.0;
	val.Clear();
	Read( _T ( "TrackIntervalSeconds" ), &val );
	if( val.Length() > 0 ) {
		double tval = atof( val.mb_str() );
		if( tval >= 2. ) g_TrackIntervalSeconds = tval;
	}

	g_TrackDeltaDistance = 0.10;
	val.Clear();
	Read( _T ( "TrackDeltaDistance" ), &val );
	if( val.Length() > 0 ) {
		double tval = atof( val.mb_str() );
		if( tval >= 0.05 ) g_TrackDeltaDistance = tval;
	}

	Read( _T ( "TrackPrecision" ), &g_nTrackPrecision, 0 );

	Read( _T ( "NavObjectFileName" ), m_sNavObjSetFile );

	Read( _T ( "RouteLineWidth" ), &g_route_line_width, 2 );
	Read( _T ( "TrackLineWidth" ), &g_track_line_width, 3 );
	Read( _T ( "CurrentArrowScale" ), &g_current_arrow_scale, 100 );
	Read( _T ( "DefaultWPIcon" ), &g_default_wp_icon, _T("triangle") );

	return ( 0 );
}

bool Config::LoadLayers(wxString &path)
{
	wxArrayString file_array;
	wxDir dir;
	dir.Open( path );
	if( dir.IsOpened() ) {
		wxString filename;
		bool cont = dir.GetFirst( &filename );
		while( cont ) {
			file_array.Clear();
			filename.Prepend( wxFileName::GetPathSeparator() );
			filename.Prepend( path );
			wxFileName f( filename );
			if( f.GetExt().IsSameAs( wxT("gpx") ) )
				file_array.Add( filename); // single-gpx-file layer
			else
				wxDir::GetAllFiles( filename, &file_array, wxT("*.gpx") );      // layers subdirectory set

			if (file_array.size()){
				++g_LayerIdx;
				Layer * layer = new Layer(g_LayerIdx, file_array[0], g_bShowLayers);
				wxString layerName;
				if (file_array.size() <= 1)
					wxFileName::SplitPath( file_array[0], NULL, NULL, &layerName, NULL, NULL );
				else
					wxFileName::SplitPath( filename, NULL, NULL, &layerName, NULL, NULL );
				layer->setName(layerName);

				bool bLayerViz = g_bShowLayers;

				if (visibleLayers.Contains(layer->getName()))
					bLayerViz = true;
				if (invisibleLayers.Contains(layer->getName()))
					bLayerViz = false;

				layer->SetVisibleOnChart(bLayerViz);

				wxString laymsg;
				laymsg.Printf( wxT("New layer %d: %s"), layer->getID(), layer->getName().c_str());
				wxLogMessage( laymsg );

				pLayerList->push_back(layer);

				//  Load the entire file array as a single layer

				for( unsigned int i = 0; i < file_array.size(); i++ ) {
					wxString file_path = file_array[i];

					if( ::wxFileExists( file_path ) ) {
						NavObjectCollection *pSet = new NavObjectCollection;
						pSet->load_file(file_path.fn_str());
						layer->setNoOfItems(pSet->LoadAllGPXObjectsAsLayer(layer->getID(), bLayerViz));

						delete pSet;
					}
				}
			}

			cont = dir.GetNext( &filename );
		}
	}

	return true;
}

bool Config::LoadChartDirArray(ArrayOfCDI & ChartDirArray)
{
	//    Chart Directories
	SetPath(_T("/ChartDirectories"));
	int iDirMax = GetNumberOfEntries();
	if( iDirMax ) {
		ChartDirArray.clear();
		wxString str;
		wxString val;
		long dummy;
		int nAdjustChartDirs = 0;
		bool bCont = pConfig->GetFirstEntry( str, dummy ); // FIXME: this is basically 'this'
		while( bCont ) {
			pConfig->Read( str, &val );              // Get a Directory name

			wxString dirname( val );
			if( !dirname.IsEmpty() ) {

				/*     Special case for first time run after Windows install with sample chart data...
					   We desire that the sample configuration file opencpn.ini should not contain any
					   installation dependencies, so...
					   Detect and update the sample [ChartDirectories] entries to point to the Shared Data directory
					   For instance, if the (sample) opencpn.ini file should contain shortcut coded entries like:

					   [ChartDirectories]
					   ChartDir1=SampleCharts\\MaptechRegion7

					   then this entry will be updated to be something like:
					   ChartDir1=c:\Program Files\opencpn\SampleCharts\\MaptechRegion7

				 */
				if( dirname.Find( _T ( "SampleCharts" ) ) == 0 ) // only update entries starting with "SampleCharts"
				{
					nAdjustChartDirs++;

					pConfig->DeleteEntry( str );
					wxString new_dir = dirname.Mid( dirname.Find( _T ( "SampleCharts" ) ) );
					new_dir.Prepend( global::OCPN::get().sys().data().sound_data_location);
					dirname = new_dir;
				}
				ChartDirInfo cdi;
				cdi.fullpath = dirname.BeforeFirst( '^' );
				cdi.magic_number = dirname.AfterFirst( '^' );

				ChartDirArray.push_back(cdi);
			}

			bCont = pConfig->GetNextEntry( str, dummy );
		}

		if (nAdjustChartDirs)
			pConfig->UpdateChartDirs(ChartDirArray);
	}

	return true;
}

bool Config::AddNewRoute(Route *pr, int) // FIXME: does this really belong to config?
{
	if (pr->m_bIsInLayer)
		return true;

	if (!m_bSkipChangeSetUpdate) {
		m_pNavObjectChangesSet->AddRoute( pr, "add" );
		StoreNavObjChanges();
	}

	return true;
}

bool Config::UpdateRoute(Route * pr) // FIXME: does this really belong to config?
{
	if( pr->m_bIsInLayer )
		return true;

	if( !m_bSkipChangeSetUpdate ) {
		if( pr->m_bIsTrack )
			m_pNavObjectChangesSet->AddTrack( (Track *)pr, "update" );
		else
			m_pNavObjectChangesSet->AddRoute( pr, "update" );

		StoreNavObjChanges();
	}

	return true;
}

bool Config::DeleteConfigRoute( Route *pr ) // FIXME: does this really belong to config?
{
	if( pr->m_bIsInLayer )
		return true;

	if( !m_bSkipChangeSetUpdate ) {
		if( !pr->m_bIsTrack )
			m_pNavObjectChangesSet->AddRoute( (Track *)pr, "delete" );
		else
			m_pNavObjectChangesSet->AddTrack( (Track *)pr, "delete" );

		StoreNavObjChanges();
	}
	return true;
}

bool Config::AddNewWayPoint(RoutePoint * pWP, int) // FIXME: does this really belong to config?
{
	if( pWP->m_bIsInLayer )
		return true;

	if( !m_bSkipChangeSetUpdate ) {
		m_pNavObjectChangesSet->AddWP( pWP, "add" );
		StoreNavObjChanges();
	}

	return true;
}

bool Config::UpdateWayPoint( RoutePoint *pWP ) // FIXME: does this really belong to config?
{
	if( pWP->m_bIsInLayer )
		return true;

	if( !m_bSkipChangeSetUpdate ) {
		m_pNavObjectChangesSet->AddWP( pWP, "update" );
		StoreNavObjChanges();
	}

	return true;
}

bool Config::DeleteWayPoint( RoutePoint *pWP ) // FIXME: does this really belong to config?
{
	if( pWP->m_bIsInLayer )
		return true;

	if( !m_bSkipChangeSetUpdate ) {
		m_pNavObjectChangesSet->AddWP( pWP, "delete" );
		StoreNavObjChanges();
	}

	return true;
}

bool Config::UpdateChartDirs(ArrayOfCDI& dir_array)
{
	wxString key;
	wxString dir;
	wxString str_buf;

	SetPath(_T("/ChartDirectories"));
	int iDirMax = GetNumberOfEntries();
	if (iDirMax) {
		long dummy;
		for (int i = 0; i < iDirMax; ++i) {
			GetFirstEntry(key, dummy);
			DeleteEntry(key, false);
		}
	}

	for (ArrayOfCDI::iterator i = dir_array.begin(); i != dir_array.end(); ++i) {
		ChartDirInfo cdi = *i;

		wxString dirn = cdi.fullpath;
		dirn += _T("^");
		dirn += cdi.magic_number;
		str_buf.Printf(_T("ChartDir%d"), i - dir_array.begin() + 1);
		Write(str_buf, dirn);
	}

	Flush();
	return true;
}

void Config::CreateConfigGroups(ChartGroupArray* pGroupArray)
{
	if (!pGroupArray)
		return;

	SetPath(_T ( "/Groups" ));
	Write(_T ( "GroupCount" ), (int)pGroupArray->size());

	for (unsigned int i = 0; i < pGroupArray->size(); i++) {
		ChartGroup* pGroup = pGroupArray->at(i);
		wxString s;
		s.Printf(_T("Group%d"), i + 1);
		s.Prepend(_T ( "/Groups/" ));
		SetPath(s);

		Write(_T ( "GroupName" ), pGroup->m_group_name);
		Write(_T ( "GroupItemCount" ), (int)pGroup->m_element_array.size());

		for (unsigned int j = 0; j < pGroup->m_element_array.size(); j++) {
			wxString sg;
			sg.Printf(_T("Group%d/Item%d"), i + 1, j);
			sg.Prepend(_T ( "/Groups/" ));
			SetPath(sg);
			Write(_T ( "IncludeItem" ), pGroup->m_element_array.Item(j)->m_element_name);

			wxString t;
			wxArrayString u = pGroup->m_element_array.Item(j)->m_missing_name_array;
			if (u.size()) {
				for (unsigned int k = 0; k < u.size(); k++) {
					t += u.Item(k);
					t += _T(";");
				}
				Write(_T ( "ExcludeItems" ), t);
			}
		}
	}
}

void Config::DestroyConfigGroups(void)
{
	DeleteGroup(_T ( "/Groups" )); // zap
}

void Config::LoadConfigGroups(ChartGroupArray* pGroupArray)
{
	SetPath(_T ( "/Groups" ));
	unsigned int group_count;
	Read(_T ( "GroupCount" ), (int*)&group_count, 0);

	for (unsigned int i = 0; i < group_count; i++) {
		ChartGroup* pGroup = new ChartGroup;
		wxString s;
		s.Printf(_T("Group%d"), i + 1);
		s.Prepend(_T ( "/Groups/" ));
		SetPath(s);

		wxString t;
		Read(_T ( "GroupName" ), &t);
		pGroup->m_group_name = t;

		unsigned int item_count;
		Read(_T ( "GroupItemCount" ), (int*)&item_count);
		for (unsigned int j = 0; j < item_count; j++) {
			wxString sg;
			sg.Printf(_T("Group%d/Item%d"), i + 1, j);
			sg.Prepend(_T ( "/Groups/" ));
			SetPath(sg);

			wxString v;
			Read(_T ( "IncludeItem" ), &v);
			ChartGroupElement* pelement = new ChartGroupElement;
			pelement->m_element_name = v;
			pGroup->m_element_array.Add(pelement);

			wxString u;
			if (Read(_T ( "ExcludeItems" ), &u)) {
				if (!u.IsEmpty()) {
					wxStringTokenizer tk(u, _T(";"));
					while (tk.HasMoreTokens()) {
						wxString token = tk.GetNextToken();
						pelement->m_missing_name_array.Add(token);
					}
				}
			}
		}
		pGroupArray->push_back(pGroup);
	}
}

void Config::write_toolbar()
{
	const global::GUI::Toolbar & config = global::OCPN::get().gui().toolbar();

	Write(_T("ToolbarX"), config.position.x);
	Write(_T("ToolbarY"), config.position.y);
	Write(_T("ToolbarOrient"), config.orientation);
	Write(_T("TransparentToolbar"), config.transparent);
	Write(_T("FullscreenToolbar"), config.full_screen);
}

void Config::write_ais_alert_dialog()
{
	const global::GUI::AISAlertDialog & config = global::OCPN::get().gui().ais_alert_dialog();

	Write(_T("AlertDialogSizeX"), config.size.GetWidth());
	Write(_T("AlertDialogSizeY"), config.size.GetHeight());
	Write(_T("AlertDialogPosX"),  config.position.x);
	Write(_T("AlertDialogPosY"),  config.position.y);
}

void Config::write_ais_query_dialog()
{
	const global::GUI::AISQueryDialog & config = global::OCPN::get().gui().ais_query_dialog();

	Write(_T("QueryDialogPosX"), config.position.x);
	Write(_T("QueryDialogPosY"), config.position.y);
}

void Config::write_frame()
{
	const global::GUI::Frame & config = global::OCPN::get().gui().frame();

	Write(_T("FrameWinX"), config.size.GetWidth());
	Write(_T("FrameWinY"), config.size.GetHeight());
	Write(_T("FrameWinPosX"), config.position.x);
	Write(_T("FrameWinPosY"), config.position.y);
	Write(_T("FrameMax"), config.maximized);
	Write(_T("ClientPosX"), config.last_position.x);
	Write(_T("ClientPosY"), config.last_position.y);
	Write(_T("ClientSzX"), config.last_size.GetWidth());
	Write(_T("ClientSzY"), config.last_size.GetHeight());
}

void Config::write_view()
{
	const global::GUI::View & config = global::OCPN::get().gui().view();

	Write(_T("ShowChartOutlines"), config.show_outlines);
	Write(_T("ShowDepthUnits"), config.show_depth_units);
	Write(_T("LookAheadMode"), config.lookahead_mode);
	Write(_T("AllowExtremeOverzoom"), config.allow_overzoom_x);
}

void Config::write_system_config()
{
	const global::System::Config & config = global::OCPN::get().sys().config();

	Write(_T("ConfigVersionString"), config.version_string);
	Write(_T("NavMessageShown"), config.nav_message_shown);
}

void Config::write_cm93()
{
	const global::GUI::CM93 & config = global::OCPN::get().gui().cm93();

	Write(_T("CM93DetailFactor"), config.zoom_factor);
	Write(_T("CM93DetailZoomPosX"), config.detail_dialog_position.x);
	Write(_T("CM93DetailZoomPosY"), config.detail_dialog_position.y);

	Write(_T("ShowCM93DetailSlider"), config.show_detail_slider);
}

void Config::UpdateSettings()
{
	//    Global options and settings
	SetPath( _T ( "/Settings" ) );

	write_view();
	write_system_config();

	Write( _T ( "UIStyle" ), g_StyleManager->GetStyleNextInvocation() );
	Write( _T ( "ChartNotRenderScaleFactor" ), g_ChartNotRenderScaleFactor );

	Write( _T ( "ShowDebugWindows" ), m_bShowDebugWindows );
	Write( _T ( "SetSystemTime" ), s_bSetSystemTime );
	Write( _T ( "ShowGrid" ), g_bDisplayGrid );
	Write( _T ( "PlayShipsBells" ), g_bPlayShipsBells );
	Write( _T ( "PermanentMOBIcon" ), g_bPermanentMOBIcon );
	Write( _T ( "ShowLayers" ), g_bShowLayers );
	Write( _T ( "AutoAnchorDrop" ), g_bAutoAnchorMark );
	Write( _T ( "ShowActiveRouteHighway" ), g_bShowActiveRouteHighway );
	Write( _T ( "SDMMFormat" ), g_iSDMMFormat );
	Write( _T ( "DistanceFormat" ), g_iDistanceFormat );
	Write( _T ( "SpeedFormat" ), g_iSpeedFormat );
	Write( _T ( "MostRecentGPSUploadConnection" ), g_uploadConnection );

	Write( _T ( "FilterNMEA_Avg" ), g_bfilter_cogsog );
	Write( _T ( "FilterNMEA_Sec" ), g_COGFilterSec );

	Write( _T ( "ShowMag" ), g_bShowMag );
	Write( _T ( "UserMagVariation" ), wxString::Format( _T("%.2f"), g_UserVar ) );

	write_cm93();

	Write( _T ( "SkewToNorthUp" ), g_bskew_comp );
	Write( _T ( "OpenGL" ), g_bopengl );
	Write( _T ( "SmoothPanZoom" ), g_bsmoothpanzoom );

	Write( _T ( "UseRasterCharts" ), g_bUseRaster );
	Write( _T ( "UseVectorCharts" ), g_bUseVector );
	Write( _T ( "UseCM93Charts" ), g_bUseCM93 );

	Write( _T ( "CourseUpMode" ), g_bCourseUp );
	Write( _T ( "COGUPAvgSeconds" ), g_COGAvgSec );
	Write( _T ( "ShowMag" ), g_bMagneticAPB );

	Write( _T ( "OwnshipCOGPredictorMinutes" ), g_ownship_predictor_minutes );
	Write( _T ( "OwnshipCOGPredictorWidth" ), g_cog_predictor_width );
	Write( _T ( "OwnShipIconType" ), g_OwnShipIconType );
	Write( _T ( "OwnShipLength" ), g_n_ownship_length_meters );
	Write( _T ( "OwnShipWidth" ), g_n_ownship_beam_meters );
	Write( _T ( "OwnShipGPSOffsetX" ), g_n_gps_antenna_offset_x );
	Write( _T ( "OwnShipGPSOffsetY" ), g_n_gps_antenna_offset_y );
	Write( _T ( "OwnShipMinSize" ), g_n_ownship_min_mm );

	Write( _T ( "RouteArrivalCircleRadius" ), wxString::Format( _T("%.2f"), g_n_arrival_circle_radius ));

	Write( _T ( "ChartQuilting" ), g_bQuiltEnable );
	Write( _T ( "FullScreenQuilt" ), g_bFullScreenQuilt );

	if( cc1 ) Write( _T ( "ChartQuiltingInitial" ), cc1->GetQuiltMode() );

	Write( _T ( "NMEALogWindowSizeX" ), NMEALogWindow::Get().GetSizeW());
	Write( _T ( "NMEALogWindowSizeY" ), NMEALogWindow::Get().GetSizeH());
	Write( _T ( "NMEALogWindowPosX" ), NMEALogWindow::Get().GetPosX());
	Write( _T ( "NMEALogWindowPosY" ), NMEALogWindow::Get().GetPosY());

	Write( _T ( "PreserveScaleOnX" ), g_bPreserveScaleOnX );

	Write( _T ( "StartWithTrackActive" ), g_bTrackCarryOver );
	Write( _T ( "AutomaticDailyTracks" ), g_bTrackDaily );
	Write( _T ( "HighlightTracks" ), g_bHighliteTracks );

	Write( _T ( "InitialStackIndex" ), g_restore_stackindex );
	Write( _T ( "InitialdBIndex" ), g_restore_dbindex );
	Write( _T ( "ActiveChartGroup" ), g_GroupIndex );

	Write( _T ( "AnchorWatch1GUID" ), g_AW1GUID );
	Write( _T ( "AnchorWatch2GUID" ), g_AW2GUID );

	write_toolbar();
	Write(_T("ToolbarConfig"), g_toolbarConfig);

	Write( _T ( "GPSIdent" ), g_GPS_Ident );
	Write( _T ( "UseGarminHostUpload" ), g_bGarminHostUpload );

	wxString st0;
	st0.Printf( _T ( "%g" ), g_PlanSpeed );
	Write( _T ( "PlanSpeed" ), st0 );

	wxString vis, invis;
	int index = 0;
	for (LayerList::iterator it = pLayerList->begin(); it != pLayerList->end(); ++it, ++index) {
		Layer *lay = (Layer *) ( *it );
		if (lay->IsVisibleOnChart())
			vis += lay->getName() + _T(";");
		else
			invis += lay->getName() + _T(";");
	}
	Write( _T ( "VisibleLayers" ), vis );
	Write( _T ( "InvisibleLayers" ), invis );

	Write( _T ( "Locale" ), g_locale );

	Write( _T ( "KeepNavobjBackups" ), g_navobjbackups );

	//    S57 Object Filter Settings

	SetPath( _T ( "/Settings/ObjectFilter" ) );

#ifdef USE_S57
	if( ps52plib ) {
		for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->size(); iPtr++ ) {
			OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );

			wxString st1( _T ( "viz" ) );
			char name[7];
			strncpy( name, pOLE->OBJLName, 6 );
			name[6] = 0;
			st1 += wxString(name, wxConvUTF8);
			Write(st1, pOLE->nViz);
		}
	}
#endif

	//    Global State

	SetPath( _T ( "/Settings/GlobalState" ) );

	wxString st1;

	if( cc1 ) {
		ViewPort vp = cc1->GetVP();

		if( vp.IsValid() ) {
			st1.Printf( _T ( "%10.4f,%10.4f" ), vp.clat, vp.clon );
			Write( _T ( "VPLatLon" ), st1 );
			st1.Printf( _T ( "%g" ), vp.view_scale_ppm );
			Write( _T ( "VPScale" ), st1 );
		}
	}

	const global::Navigation::Data & nav = global::OCPN::get().nav().get_data();
	st1.Printf( _T ( "%10.4f, %10.4f" ), nav.lat, nav.lon);
	Write( _T ( "OwnShipLatLon" ), st1 );

	//    Various Options
	SetPath( _T ( "/Settings/GlobalState" ) );
	if( cc1 ) Write( _T ( "bFollow" ), cc1->m_bFollow );
	Write( _T ( "nColorScheme" ), (int) gFrame->GetColorScheme() );

	write_frame();

	//    AIS
	SetPath( _T ( "/Settings/AIS" ) );

	Write( _T ( "bNoCPAMax" ), g_bCPAMax );
	Write( _T ( "NoCPAMaxNMi" ), g_CPAMax_NM );
	Write( _T ( "bCPAWarn" ), g_bCPAWarn );
	Write( _T ( "CPAWarnNMi" ), g_CPAWarn_NM );
	Write( _T ( "bTCPAMax" ), g_bTCPA_Max );
	Write( _T ( "TCPAMaxMinutes" ), g_TCPA_Max );
	Write( _T ( "bMarkLostTargets" ), g_bMarkLost );
	Write( _T ( "MarkLost_Minutes" ), g_MarkLost_Mins );
	Write( _T ( "bRemoveLostTargets" ), g_bRemoveLost );
	Write( _T ( "RemoveLost_Minutes" ), g_RemoveLost_Mins );
	Write( _T ( "bShowCOGArrows" ), g_bShowCOG );
	Write( _T ( "CogArrowMinutes" ), g_ShowCOG_Mins );
	Write( _T ( "bShowTargetTracks" ), g_bAISShowTracks );
	Write( _T ( "TargetTracksMinutes" ), g_AISShowTracks_Mins );
	Write( _T ( "bShowMooredTargets" ), g_bShowMoored );
	Write( _T ( "MooredTargetMaxSpeedKnots" ), g_ShowMoored_Kts );
	Write( _T ( "bAISAlertDialog" ), g_bAIS_CPA_Alert );
	Write( _T ( "bAISAlertAudio" ), g_bAIS_CPA_Alert_Audio );
	Write( _T ( "AISAlertAudioFile" ), g_sAIS_Alert_Sound_File );
	Write( _T ( "bAISAlertSuppressMoored" ), g_bAIS_CPA_Alert_Suppress_Moored );
	Write( _T ( "bShowAreaNotices" ), g_bShowAreaNotices );
	Write( _T ( "bDrawAISSize" ), g_bDrawAISSize );
	Write( _T ( "bShowAISName" ), g_bShowAISName );
	Write( _T ( "ShowAISTargetNameScale" ), g_Show_Target_Name_Scale );
	Write( _T ( "bWplIsAprsPositionReport" ), g_bWplIsAprsPosition );
	Write( _T ( "AISCOGPredictorWidth" ), g_ais_cog_predictor_width );

	write_ais_alert_dialog();
	write_ais_query_dialog();

	Write( _T ( "AISTargetListPerspective" ), g_AisTargetList_perspective );
	Write( _T ( "AISTargetListRange" ), g_AisTargetList_range );
	Write( _T ( "AISTargetListSortColumn" ), g_AisTargetList_sortColumn );
	Write( _T ( "bAISTargetListSortReverse" ), g_bAisTargetList_sortReverse );
	Write( _T ( "AISTargetListColumnSpec" ), g_AisTargetList_column_spec );

	Write( _T ( "S57QueryDialogSizeX" ), g_S57_dialog_sx );
	Write( _T ( "S57QueryDialogSizeY" ), g_S57_dialog_sy );

	Write( _T ( "bAISRolloverShowClass" ), g_bAISRolloverShowClass );
	Write( _T ( "bAISRolloverShowCOG" ), g_bAISRolloverShowCOG );
	Write( _T ( "bAISRolloverShowCPA" ), g_bAISRolloverShowCPA );

	Write( _T ( "bAISAlertAckTimeout" ), g_bAIS_ACK_Timeout );
	Write( _T ( "AlertAckTimeoutMinutes" ), g_AckTimeout_Mins );

#ifdef USE_S57
	SetPath( _T ( "/Settings/GlobalState" ) );
	if( ps52plib ) {
		Write( _T ( "bShowS57Text" ), ps52plib->GetShowS57Text() );
		Write( _T ( "bShowS57ImportantTextOnly" ), ps52plib->GetShowS57ImportantTextOnly() );
		Write( _T ( "nDisplayCategory" ), (long) ps52plib->m_nDisplayCategory );
		Write( _T ( "nSymbolStyle" ), (int) ps52plib->m_nSymbolStyle );
		Write( _T ( "nBoundaryStyle" ), (int) ps52plib->m_nBoundaryStyle );

		Write( _T ( "bShowSoundg" ), ps52plib->m_bShowSoundg );
		Write( _T ( "bShowMeta" ), ps52plib->m_bShowMeta );
		Write( _T ( "bUseSCAMIN" ), ps52plib->m_bUseSCAMIN );
		Write( _T ( "bShowAtonText" ), ps52plib->m_bShowAtonText );
		Write( _T ( "bShowLightDescription" ), ps52plib->m_bShowLdisText );
		Write( _T ( "bExtendLightSectors" ), ps52plib->m_bExtendLightSectors );
		Write( _T ( "bDeClutterText" ), ps52plib->m_bDeClutterText );
		Write( _T ( "bShowNationalText" ), ps52plib->m_bShowNationalTexts );

		Write( _T ( "S52_MAR_SAFETY_CONTOUR" ), S52_getMarinerParam( S52_MAR_SAFETY_CONTOUR ) );
		Write( _T ( "S52_MAR_SHALLOW_CONTOUR" ), S52_getMarinerParam( S52_MAR_SHALLOW_CONTOUR ) );
		Write( _T ( "S52_MAR_DEEP_CONTOUR" ), S52_getMarinerParam( S52_MAR_DEEP_CONTOUR ) );
		Write( _T ( "S52_MAR_TWO_SHADES" ), S52_getMarinerParam( S52_MAR_TWO_SHADES ) );
		Write( _T ( "S52_DEPTH_UNIT_SHOW" ), ps52plib->m_nDepthUnitDisplay );
	}
	SetPath( _T ( "/Directories" ) );
	Write( _T ( "S57DataLocation" ), _T("") );
	Write( _T ( "SENCFileLocation" ), _T("") );

#endif

	SetPath(_T("/Directories"));
	Write(_T("InitChartDir" ), global::OCPN::get().sys().data().init_chart_dir);
	Write(_T("GPXIODir"), m_gpx_path);
	Write(_T("TCDataDir"), global::OCPN::get().sys().data().tc_data_dir);

	SetPath( _T ( "/Settings/NMEADataSource" ) );
	wxString connectionconfigs;
	for (size_t i = 0; i < g_pConnectionParams->Count(); i++) {
		if (i > 0)
			connectionconfigs += _T("|");
		connectionconfigs += g_pConnectionParams->Item(i)->Serialize();
	}
	Write ( _T ( "DataConnections" ), connectionconfigs );

	//    Fonts
	wxString font_path;
#ifdef __WXX11__
	font_path = ( _T ( "/Settings/X11Fonts" ) );
#endif

#ifdef __WXGTK__
	font_path = ( _T ( "/Settings/GTKFonts" ) );
#endif

#ifdef __WXMSW__
	font_path = ( _T ( "/Settings/MSWFonts" ) );
#endif

#ifdef __WXMAC__
	font_path = ( _T ( "/Settings/MacFonts" ) );
#endif

	SetPath( font_path );

	int nFonts = FontMgr::Get().GetNumFonts();

	for( int i = 0; i < nFonts; i++ ) {
		wxString cfstring(FontMgr::Get().GetConfigString(i));
		wxString valstring = FontMgr::Get().GetFullConfigDesc( i );
		Write( cfstring, valstring );
	}

	//  Tide/Current Data Sources
	DeleteGroup( _T ( "/TideCurrentDataSources" ) );
	SetPath( _T ( "/TideCurrentDataSources" ) );
	unsigned int iDirMax = TideCurrentDataSet.Count();
	for( unsigned int id = 0 ; id < iDirMax ; id++ ) {
		wxString key;
		key.Printf(_T("tcds%d"), id);
		Write( key, TideCurrentDataSet.Item(id) );
	}

	SetPath( _T ( "/Settings/Others" ) );

	// Radar rings
	Write( _T ( "ShowRadarRings" ), (bool)(g_iNavAidRadarRingsNumberVisible > 0) );  //3.0.0 config support
	Write( _T ( "RadarRingsNumberVisible" ), g_iNavAidRadarRingsNumberVisible );
	Write( _T ( "RadarRingsStep" ), g_fNavAidRadarRingsStep );
	Write( _T ( "RadarRingsStepUnits" ), g_pNavAidRadarRingsStepUnits );

	Write( _T ( "ConfirmObjectDeletion" ), g_bConfirmObjectDelete );

	// Waypoint dragging with mouse; toh, 2009.02.24
	Write( _T ( "WaypointPreventDragging" ), g_bWayPointPreventDragging );

	Write( _T ( "EnableZoomToCursor" ), g_bEnableZoomToCursor );

	Write( _T ( "TrackIntervalSeconds" ), g_TrackIntervalSeconds );
	Write( _T ( "TrackDeltaDistance" ), g_TrackDeltaDistance );
	Write( _T ( "TrackPrecision" ), g_nTrackPrecision );

	Write( _T ( "RouteLineWidth" ), g_route_line_width );
	Write( _T ( "TrackLineWidth" ), g_track_line_width );
	Write( _T ( "CurrentArrowScale" ), g_current_arrow_scale );
	Write( _T ( "DefaultWPIcon" ), g_default_wp_icon );

	Flush();
}

void Config::UpdateNavObj( void )
{
	//   Create the nav object collection, and save to specified file
	NavObjectCollection *pNavObjectSet = new NavObjectCollection();

	pNavObjectSet->CreateAllGPXObjects();
	pNavObjectSet->SaveFile( m_sNavObjSetFile );

	delete pNavObjectSet;

	wxRemoveFile( m_sNavObjSetChangesFile );
	delete m_pNavObjectChangesSet;
	m_pNavObjectChangesSet = new NavObjectChanges();
}

void Config::StoreNavObjChanges( void )
{
	m_pNavObjectChangesSet->SaveFile( m_sNavObjSetChangesFile );
}

bool Config::ExportGPXRoutes( wxWindow* parent, RouteList *pRoutes, const wxString suggestedName )
{
	wxFileDialog saveDialog( parent, _( "Export GPX file" ), m_gpx_path, suggestedName,
			wxT ( "GPX files (*.gpx)|*.gpx" ), wxFD_SAVE );

	int response = saveDialog.ShowModal();

	wxString path = saveDialog.GetPath();
	wxFileName fn( path );
	m_gpx_path = fn.GetPath();

	if( response == wxID_OK ) {
		fn.SetExt( _T ( "gpx" ) );

		if( wxFileExists( fn.GetFullPath() ) ) {
			int answer = OCPNMessageBox( NULL, _("Overwrite existing file?"), _T("Confirm"),
					wxICON_QUESTION | wxYES_NO | wxCANCEL );
			if( answer != wxID_YES ) return false;
		}

		NavObjectCollection *pgpx = new NavObjectCollection;
		pgpx->AddGPXRoutesList( pRoutes );
		pgpx->SaveFile(fn.GetFullPath());
		delete pgpx;

		return true;
	} else
		return false;
}

bool Config::ExportGPXWaypoints( wxWindow* parent, RoutePointList * pRoutePoints, const wxString suggestedName )
{
	wxFileDialog saveDialog( parent, _( "Export GPX file" ), m_gpx_path, suggestedName,
			wxT ( "GPX files (*.gpx)|*.gpx" ), wxFD_SAVE );

	int response = saveDialog.ShowModal();

	wxString path = saveDialog.GetPath();
	wxFileName fn( path );
	m_gpx_path = fn.GetPath();

	if( response == wxID_OK ) {
		fn.SetExt( _T ( "gpx" ) );

		if( wxFileExists( fn.GetFullPath() ) ) {
			int answer = OCPNMessageBox(NULL,  _("Overwrite existing file?"), _T("Confirm"),
					wxICON_QUESTION | wxYES_NO | wxCANCEL );
			if( answer != wxID_YES ) return false;
		}

		NavObjectCollection * pgpx = new NavObjectCollection;
		pgpx->AddGPXPointsList(pRoutePoints);
		pgpx->SaveFile(fn.GetFullPath());
		delete pgpx;

		return true;
	} else
		return false;
}

void Config::ExportGPX( wxWindow* parent, bool bviz_only, bool blayer )
{
	wxFileDialog saveDialog( parent, _( "Export GPX file" ), m_gpx_path, wxT ( "" ),
			wxT ( "GPX files (*.gpx)|*.gpx" ), wxFD_SAVE );

	int response = saveDialog.ShowModal();

	wxString path = saveDialog.GetPath();
	wxFileName fn( path );
	m_gpx_path = fn.GetPath();

	if( response == wxID_OK ) {
		fn.SetExt( _T ( "gpx" ) );

		if( wxFileExists( fn.GetFullPath() ) ) {
			int answer = OCPNMessageBox( NULL, _("Overwrite existing file?"), _T("Confirm"),
					wxICON_QUESTION | wxYES_NO | wxCANCEL );
			if( answer != wxID_YES ) return;
		}

		::wxBeginBusyCursor();

		NavObjectCollection *pgpx = new NavObjectCollection;

		wxProgressDialog *pprog = NULL;
		int count = pWayPointMan->m_pWayPointList->size();
		if (count > 200) {
			pprog = new wxProgressDialog(_("Export GPX file"), _T("0/0"), count, NULL,
					wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);
			pprog->SetSize(400, wxDefaultCoord);
			pprog->Centre();
		}

		//WPTs
		int ic = 0;

		for (RoutePointList::iterator i = pWayPointMan->m_pWayPointList->begin(); i != pWayPointMan->m_pWayPointList->end(); ++i) {
			if (pprog) {
				wxString msg;
				msg.Printf(_T("%d/%d"), ic, count);
				pprog->Update(ic, msg);
				ic++;
			}

			RoutePoint * pr = *i;

			bool b_add = true;

			if (bviz_only && !pr->m_bIsVisible)
				b_add = false;

			if (pr->m_bIsInLayer && !blayer)
				b_add = false;

			if (b_add) {
				if (pr->m_bKeepXRoute || !WptIsInRouteList(pr))
					pgpx->AddGPXWaypoint(pr);
			}
		}

		//RTEs and TRKs
		for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
			Route * route = *i;

			bool b_add = true;

			if (bviz_only && !route->IsVisible())
				b_add = false;

			if (route->m_bIsInLayer && !blayer)
				b_add = false;

			if (b_add) {
				if (!route->m_bIsTrack)
					pgpx->AddGPXRoute(route);
				else
					pgpx->AddGPXTrack((Track *)route);
			}
		}

		pgpx->SaveFile( fn.GetFullPath() );
		delete pgpx;
		::wxEndBusyCursor();

		if( pprog)
			delete pprog;
	}
}

void Config::UI_ImportGPX(wxWindow* parent, bool islayer, wxString dirpath, bool isdirectory)
{
	int response = wxID_CANCEL;
	wxArrayString file_array;

	if( !islayer || dirpath.IsSameAs( _T("") ) ) {
		wxFileDialog openDialog( parent, _( "Import GPX file" ), m_gpx_path, wxT ( "" ),
				wxT ( "GPX files (*.gpx)|*.gpx|All files (*.*)|*.*" ),
				wxFD_OPEN | wxFD_MULTIPLE );
		response = openDialog.ShowModal();
		if( response == wxID_OK ) {
			openDialog.GetPaths( file_array );

			//    Record the currently selected directory for later use
			if( file_array.size() ) {
				wxFileName fn( file_array[0] );
				m_gpx_path = fn.GetPath();
			}
		}

	} else {
		if( isdirectory ) {
			if( wxDir::GetAllFiles( dirpath, &file_array, wxT("*.gpx") ) )
				response = wxID_OK;
		} else {
			file_array.Add( dirpath );
			response = wxID_OK;
		}
	}

	if( response == wxID_OK ) {
		Layer * layer = NULL;

		if (islayer) {
			++g_LayerIdx;
			layer = new Layer(g_LayerIdx, file_array[0], g_bShowLayers);
			wxString layerName;
			if (file_array.size() <= 1) {
					wxFileName::SplitPath(file_array[0], NULL, NULL, &layerName, NULL, NULL);
			} else {
				if (dirpath.IsSameAs(_T("")))
					wxFileName::SplitPath(m_gpx_path, NULL, NULL, &layerName, NULL, NULL);
				else
					wxFileName::SplitPath(dirpath, NULL, NULL, &layerName, NULL, NULL);
			}
			layer->setName(layerName);

			bool bLayerViz = g_bShowLayers;
			if (visibleLayers.Contains(layer->getName()))
				bLayerViz = true;
			if (invisibleLayers.Contains(layer->getName()))
				bLayerViz = false;
			layer->SetVisibleOnChart(bLayerViz);

			wxString laymsg;
			laymsg.Printf( wxT("New layer %d: %s"), layer->getID(), layer->getName().c_str() );
			wxLogMessage(laymsg);

			pLayerList->push_back(layer);
		}

		for (unsigned int i = 0; i < file_array.size(); ++i) {
			wxString path = file_array[i];
			if (::wxFileExists(path)) {
				NavObjectCollection * pSet = new NavObjectCollection;
				pSet->load_file(path.fn_str());

				if (islayer) {
					layer->setNoOfItems(pSet->LoadAllGPXObjectsAsLayer(layer->getID(), layer->IsVisibleOnChart()));
				} else {
					pSet->LoadAllGPXObjects();
				}

				delete pSet;
			}
		}
	}
}

bool Config::WptIsInRouteList(RoutePoint * pr)
{
	for (RouteList::iterator j = pRouteList->begin(); j != pRouteList->end(); ++j) {
		RoutePointList * pRoutePointList = (*j)->pRoutePointList;
		for (RoutePointList::iterator i = pRoutePointList->begin(); i != pRoutePointList->end(); ++i) {
			if (pr->IsSame(*i)) {
				return true;
			}
		}
	}
	return false;
}

