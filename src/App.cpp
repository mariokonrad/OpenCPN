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

#include "App.h"
#include <global/OCPN.h>
#include <global/OCPN_GUI.h>
#include <global/OCPN_Navigation.h>
#include "dychart.h"
#include "Select.h"
#include "OCPNFloatingToolbarDialog.h"
#include "MessageBox.h"
#include "Multiplexer.h"
#include "StyleManager.h"
#include "Style.h"
#include "AboutDialog.h"
#include "FloatingCompassWindow.h"
#include "Routeman.h"
#include "WayPointman.h"
#include "ThumbWin.h"
#include "tcmgr.h"
#include "chart/ChartStack.h"
#include "chart/ChartDB.h"
#include "Layer.h"
#include "StatWin.h"
#include "PianoWin.h"
#include "ConsoleCanvas.h"
#include "ais/AIS_Decoder.h"
#include "ais/AISTargetAlertDialog.h"
#include "navutil.h"
#include "MicrosoftCompatibility.h"
#include "chart/ChartDummy.h"
#include "plugin/PlugInManager.h"

#include <wx/cmdline.h>
#include <wx/datetime.h>
#include <wx/progdlg.h>
#include <wx/stdpaths.h>
#include <wx/apptrait.h>

#ifdef USE_S57
#include "s52plib.h"
#include "S57ClassRegistrar.h"
#include "S57QueryDialog.h"
#include "S57RegistrarMgr.h"
#include "cm93.h"
#include "cpl_csv.h"
#endif

static unsigned int malloc_max;

// FIXME: this global data CLUSTERFUCK, people... seriously... this was bad even in the 60ties
// this list of 'extern's has doubles, will sort them out later

bool GetMemoryStatus(int *mem_total, int *mem_used);
void appendOSDirSlash(wxString * pString);
int ShowNavWarning();
void InitializeUserColors(void);
void DeInitializeUserColors(void);
void RestoreSystemColors();

#ifndef __WXMSW__
extern struct sigaction sa_all;
extern struct sigaction sa_all_old;
void catch_signals(int signo);
#endif

#ifdef USE_S57
extern s52plib *ps52plib;
extern S57ClassRegistrar *g_poRegistrar;
extern S57RegistrarMgr *m_pRegistrarMan;
extern S57QueryDialog *g_pObjectQueryDialog;
extern CM93OffsetDialog *g_pCM93OffsetDialog;
#endif

extern int sat_watchdog_timeout_ticks;
extern int gSAT_Watchdog;
extern StatWin *stats;
extern ConsoleCanvas *console;
extern double initial_scale_ppm;
extern double vLat;
extern double vLon;
extern ChartCanvas * cc1;
extern wxString str_version_start;
extern wxString str_version_major;
extern wxString str_version_minor;
extern wxString str_version_patch;
extern wxString str_version_date;
extern ColorScheme global_color_scheme;
extern bool g_bFirstRun;
extern wxString gConfig_File;
extern wxString OpenCPNVersion;
extern FILE *flog;
extern int g_mem_used;
extern int g_mem_initial;
extern bool s_bSetSystemTime;
extern wxString *phost_name;
extern wxArrayOfConnPrm *g_pConnectionParams;
extern wxDateTime g_start_time;
extern wxDateTime g_loglast_time;
extern OCPN_Sound bells_sound[8];
extern OCPN_Sound g_anchorwatch_sound;
extern RoutePoint * pAnchorWatchPoint1;
extern RoutePoint * pAnchorWatchPoint2;
extern double AnchorPointMinDist;
extern bool AnchorAlertOn1;
extern bool AnchorAlertOn2;
extern bool g_bCruising;
extern ChartDummy *pDummyChart;
extern ToolBarSimple* g_toolbar;
extern ocpnStyle::StyleManager * g_StyleManager;
extern wxPrintData * g_printData;
extern wxPageSetupData* g_pageSetupData;
extern int portaudio_initialized;
extern int g_sticky_chart;
extern double g_GLMinLineWidth;
extern int n_NavMessageShown;
extern wxString g_config_version_string;
extern int g_nbrightness;
extern bool bDBUpdateInProgress;
extern ThumbWin * pthumbwin;
extern TCMgr * ptcmgr;
extern bool bDrawCurrentValues;
extern wxString g_PrivateDataDir;
extern wxString g_SData_Locn;
extern wxString *pChartListFileName;
extern wxString *pHome_Locn;
extern wxString *pWorldMapLocation;
extern wxString *pInit_Chart_Dir;
extern wxString g_csv_locn;
extern wxString g_SENCPrefix;
extern wxString g_UserPresLibData;
extern wxString g_Plugin_Dir;
extern wxString g_VisibleLayers;
extern wxString g_InvisibleLayers;
extern wxString g_uploadConnection;
extern int user_user_id;
extern int file_user_id;
extern ChartDB *ChartData;
extern double g_ownship_predictor_minutes;
extern int g_current_arrow_scale;
extern Multiplexer *g_pMUX;
extern AIS_Decoder *g_pAIS;
extern bool g_bAIS_CPA_Alert;
extern bool g_bAIS_CPA_Alert_Audio;
extern int g_S57_dialog_sx;
extern int g_S57_dialog_sy;
extern int g_nframewin_x;
extern int g_nframewin_y;
extern int g_nframewin_posx;
extern int g_nframewin_posy;
extern bool g_bframemax;
extern bool g_bAutoAnchorMark;
extern wxRect g_blink_rect;
extern double g_PlanSpeed;
extern wxDateTime g_StartTime;
extern AISTargetAlertDialog * g_pais_alert_dialog_active;
extern AISTargetQueryDialog * g_pais_query_dialog_active;
extern int g_StartTimeTZ;
extern IDX_entry * gpIDX;
extern int gpIDXn;
extern wxArrayString * pMessageOnceArray;
extern FILE *s_fpdebug;
extern bool bAutoOpen;
extern bool bFirstAuto;
extern bool g_bUseRMC;
extern bool g_bUseGLL;
extern int g_nCacheLimit;
extern int g_memCacheLimit;
extern bool g_bGDAL_Debug;
extern double g_VPRotate; // Viewport rotation angle, used on "Course Up" mode
extern bool g_bCourseUp;
extern int g_COGAvgSec; // COG average period (sec.) for Course Up Mode
extern double g_COGAvg;
extern bool g_bLookAhead;
extern bool g_bskew_comp;
extern bool g_bopengl;
extern bool g_bsmoothpanzoom;
extern int g_nCOMPortCheck;
extern bool g_bbigred;
extern PlugInManager *g_pi_manager;
extern bool g_bAISRolloverShowClass;
extern bool g_bAISRolloverShowCOG;
extern bool g_bAISRolloverShowCPA;
extern bool g_bDebugGPSD;
extern bool g_bFullScreenQuilt;
extern bool g_bQuiltEnable;
extern bool g_bQuiltStart;
extern bool g_bportable;
extern bool g_bdisable_opengl;
extern ChartGroupArray *g_pGroupArray;
extern int g_GroupIndex;
extern wxString g_GPS_Ident;
extern wxProgressDialog *s_ProgDialog;
extern wxArrayString TideCurrentDataSet;
extern wxString g_TCData_Dir;
extern bool g_bShowMoored;
extern double g_ShowMoored_Kts;
extern wxString g_sAIS_Alert_Sound_File;
extern bool g_bAIS_CPA_Alert_Suppress_Moored;
extern bool g_bAIS_ACK_Timeout;
extern double g_AckTimeout_Mins;
extern bool g_bShowAreaNotices;
extern bool g_bDrawAISSize;
extern bool g_bShowAISName;
extern int g_Show_Target_Name_Scale;
extern bool g_bWplIsAprsPosition;
extern wxToolBarToolBase *m_pAISTool;
extern int g_nAIS_activity_timer;
extern bool g_bEnableZoomToCursor;
extern bool g_bTrackActive;
extern bool g_bTrackCarryOver;
extern bool g_bTrackDaily;
extern bool g_bHighliteTracks;
extern int g_route_line_width;
extern int g_track_line_width;
extern wxString g_default_wp_icon;
extern Track *g_pActiveTrack;
extern double g_TrackIntervalSeconds;
extern double g_TrackDeltaDistance;
extern int g_nTrackPrecision;
extern int g_total_NMEAerror_messages;
extern int g_cm93_zoom_factor;
extern CM93DSlide *pCM93DetailSlider;
extern bool g_bShowCM93DetailSlider;
extern int g_cm93detail_dialog_x;
extern int g_cm93detail_dialog_y;
extern bool g_bUseGreenShip;
extern wxString g_AW1GUID;
extern wxString g_AW2GUID;
extern bool g_b_overzoom_x;
extern bool g_bshow_overzoom_emboss;
extern int g_OwnShipIconType;
extern double g_n_ownship_length_meters;
extern double g_n_ownship_beam_meters;
extern double g_n_gps_antenna_offset_y;
extern double g_n_gps_antenna_offset_x;
extern int g_n_ownship_min_mm;
extern int g_nautosave_interval_seconds;
extern bool g_bPreserveScaleOnX;
extern AboutDialog * g_pAboutDlg;
extern wxPlatformInfo *g_pPlatform;
extern wxLocale *plocale_def_lang;
extern wxString g_locale;
extern bool g_b_assume_azerty;
extern bool g_bUseRaster;
extern bool g_bUseVector;
extern bool g_bUseCM93;
extern int g_click_stop;
extern int g_MemFootSec;
extern int g_MemFootMB;
extern wxStaticBitmap *g_pStatBoxTool;
extern bool g_bquiting;
extern int g_BSBImgDebug;
extern AISTargetListDialog *g_pAISTargetList;
extern wxString g_AisTargetList_perspective;
extern int g_AisTargetList_range;
extern int g_AisTargetList_sortColumn;
extern bool g_bAisTargetList_sortReverse;
extern wxString g_AisTargetList_column_spec;
extern int g_AisTargetList_count;
extern bool g_bGarminHostUpload;
extern wxAuiManager *g_pauimgr;
extern wxAuiDefaultDockArt *g_pauidockart;
extern bool g_blocale_changed;
extern wxMenu *g_FloatingToolbarConfigMenu;
extern bool g_bShowAIS;
extern bool g_bCPAMax;
extern double g_CPAMax_NM;
extern bool g_bCPAWarn;
extern double g_CPAWarn_NM;
extern bool g_bTCPA_Max;
extern double g_TCPA_Max;
extern bool g_bMarkLost;
extern double g_MarkLost_Mins;
extern bool g_bRemoveLost;
extern double g_RemoveLost_Mins;
extern bool g_bShowCOG;
extern double g_ShowCOG_Mins;
extern bool g_bAISShowTracks;
extern double g_AISShowTracks_Mins;
extern bool g_bShowMoored;
extern double g_ShowMoored_Kts;
extern wxString g_sAIS_Alert_Sound_File;
extern bool g_bAIS_CPA_Alert_Suppress_Moored;
extern bool g_bAIS_ACK_Timeout;
extern double g_AckTimeout_Mins;
extern bool g_bShowAreaNotices;
extern bool g_bDrawAISSize;
extern bool g_bShowAISName;
extern int g_Show_Target_Name_Scale;
extern bool g_bWplIsAprsPosition;
extern bool g_bShowOutlines;
extern bool g_bShowDepthUnits;
extern bool g_bDisplayGrid;
extern bool g_bShowActiveRouteHighway;
extern int g_nNMEADebug;
extern int g_nAWDefault;
extern int g_nAWMax;
extern bool g_bPlayShipsBells;
extern bool g_bFullscreenToolbar;
extern bool g_bShowLayers;
extern bool g_bTransparentToolbar;
extern bool g_bPermanentMOBIcon;
extern int g_iSDMMFormat;
extern int g_iDistanceFormat;
extern int g_iSpeedFormat;
extern int g_iNavAidRadarRingsNumberVisible;
extern float g_fNavAidRadarRingsStep;
extern int g_pNavAidRadarRingsStepUnits;
extern bool g_bWayPointPreventDragging;
extern bool g_bConfirmObjectDelete;
extern wxLog *Oldlogger;
extern ChartDummy *pDummyChart;
extern LayerList *pLayerList;
extern ChartGroupArray *g_pGroupArray;
extern FloatingCompassWindow * g_FloatingCompassDialog;
extern Routeman *g_pRouteMan;
extern wxLog *logger;
extern bool g_bTrackCarryOver;
extern RoutePoint *pAnchorWatchPoint1;
extern RoutePoint *pAnchorWatchPoint2;
extern int g_lastClientRectx;
extern int g_lastClientRecty;
extern int g_lastClientRectw;
extern int g_lastClientRecth;
extern WayPointman *pWayPointMan;
extern wxString g_AW1GUID;
extern wxString g_AW2GUID;
extern wxString *pChartListFileName;
extern wxString *pHome_Locn;
extern wxString *pWorldMapLocation;
extern wxString *pInit_Chart_Dir;
extern bool g_bHDT_Rx;
extern bool g_bVAR_Rx;
extern int gps_watchdog_timeout_ticks;
extern ChartStack *pCurrentStack;
extern int g_unit_test_1;
extern bool g_bportable;
extern bool g_bdisable_opengl;
extern OCPNFloatingToolbarDialog * g_FloatingToolbarDialog;
extern wxDateTime g_start_time;
extern int g_mem_total;
extern int g_mem_used;
extern int g_mem_initial;
extern MyConfig * pConfig;
extern Select * pSelect;
extern Select * pSelectTC;
extern Select * pSelectAIS;
extern wxPlatformInfo * g_pPlatform;
extern wxDateTime g_loglast_time;
extern wxString g_SData_Locn;
extern bool bGPSValid;
extern double gLat;
extern double gLon;
extern double gCog;
extern double gSog;
extern double gHdt;
extern double gHdm;
extern wxString glog_file;
extern int gGPS_Watchdog;
extern int gHDx_Watchdog;
extern int gHDT_Watchdog;
extern int gVAR_Watchdog;
extern int g_GroupIndex;
extern ocpnStyle::StyleManager * g_StyleManager;
extern wxPlatformInfo * g_pPlatform;
extern wxAuiManager * g_pauimgr;
extern wxLocale * plocale_def_lang;
extern MyFrame * gFrame;


IMPLEMENT_APP(App)

BEGIN_EVENT_TABLE(App, wxApp)
	EVT_ACTIVATE_APP(App::OnActivateApp)
END_EVENT_TABLE()

#include "wx/dynlib.h"

App::App()
	: gui_instance(NULL)
	, nav_instance(NULL)
{}

void App::OnInitCmdLine( wxCmdLineParser& parser )
{
	//    Add some OpenCPN specific command line options
	parser.AddSwitch( _T("unit_test_1") );
	parser.AddSwitch( _T("p") );
	parser.AddSwitch( _T("no_opengl") );
}

bool App::OnCmdLineParsed( wxCmdLineParser& parser )
{
	g_unit_test_1 = parser.Found( _T("unit_test_1") );
	g_bportable = parser.Found( _T("p") );
	g_bdisable_opengl = parser.Found( _T("no_opengl") );

	return true;
}

void App::OnActivateApp( wxActivateEvent& event )
{
	//    Code carefully in this method.
	//    It is called in some unexpected places,
	//    such as on closure of dialogs, etc.

	//      Activating?

#ifdef __WXOSX__

	//      On the Mac, this method gets hit when...
	//      a) switching between apps by clicking title bars, coming and going
	//      b) un-iconizing, activeate only/
	//      It does NOT get hit on iconizing the app
	if(!event.GetActive())
	{
		if(g_FloatingToolbarDialog) {
			if(g_FloatingToolbarDialog->IsShown())
				g_FloatingToolbarDialog->Submerge();
		}

		if(console && console->IsShown()) {
			console->Hide();
		}

		if(g_FloatingCompassDialog && g_FloatingCompassDialog->IsShown()) {
			g_FloatingCompassDialog->Hide();
		}

		if(stats && stats->IsShown()) {
			stats->Hide();
		}

	}
	else
	{
		gFrame->SubmergeToolbar();              // This is needed to reset internal wxWidgets logic
		// Also required for other TopLevelWindows here
		// reportedly not required for wx 2.9
		gFrame->SurfaceToolbar();

		if(g_FloatingCompassDialog){
			g_FloatingCompassDialog->Hide();
			g_FloatingCompassDialog->Show();
		}

		if(stats){
			stats->Hide();
			stats->Show();
		}

		if(console) {
			if( g_pRouteMan->IsAnyRouteActive() ){
				console->Hide();
				console->Show();
			}
		}

		gFrame->Raise();

	}
#endif

	if( !event.GetActive() ) {
		if( g_FloatingToolbarDialog )
			g_FloatingToolbarDialog->HideTooltip(); // Hide any existing tip
	}

	event.Skip();
}

bool App::OnInit()
{
	if( !wxApp::OnInit() )
		return false;

	gui_instance = new global::OCPN_GUI;
	global::OCPN::get().inject(gui_instance);

	nav_instance = new global::OCPN_Navigation;
	global::OCPN::get().inject(nav_instance);

	//  On Windows
	//  We allow only one instance unless the portable option is used
#ifdef __WXMSW__
	m_checker = new wxSingleInstanceChecker(_T("OpenCPN"));
	if(!g_bportable) {
		if ( m_checker->IsAnotherRunning() )
			return false;               // exit quietly
	}
#endif

#ifdef OCPN_USE_CRASHRPT
#ifndef _DEBUG
	// Install Windows crash reporting

	CR_INSTALL_INFO info;
	memset(&info, 0, sizeof(CR_INSTALL_INFO));
	info.cb = sizeof(CR_INSTALL_INFO);
	info.pszAppName = _T("OpenCPN");

	wxString version_crash = str_version_major + _T(".") + str_version_minor + _T(".") + str_version_patch;
	info.pszAppVersion = version_crash.c_str();

	// URL for sending error reports over HTTP.
	info.pszEmailTo = _T("opencpn@bigdumboat.com");
	info.pszSmtpProxy = _T("mail.bigdumboat.com:587");
	info.pszUrl = _T("http://bigdumboat.com/crashrpt/ocpn_crashrpt.php");
	info.uPriorities[CR_HTTP] = 3;  // First try send report over HTTP
	info.uPriorities[CR_SMTP] = CR_NEGATIVE_PRIORITY;  // Second try send report over SMTP
	info.uPriorities[CR_SMAPI] = CR_NEGATIVE_PRIORITY; //1; // Third try send report over Simple MAPI

	// Install all available exception handlers.
	info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;

	// Use binary encoding for HTTP uploads (recommended).
	info.dwFlags |= CR_INST_HTTP_BINARY_ENCODING;

	// Provide privacy policy URL
	wxStandardPathsBase& std_path_crash = wxApp::GetTraits()->GetStandardPaths();
	std_path_crash.Get();
	wxFileName exec_path_crash( std_path_crash.GetExecutablePath() );
	wxString policy_file =  exec_path_crash.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR );
	policy_file += _T("PrivacyPolicy.txt");
	policy_file.Prepend(_T("file:"));

	info.pszPrivacyPolicyURL = policy_file.c_str();;

	int nResult = crInstall(&info);
	if(nResult!=0) {
		TCHAR buff[256];
		crGetLastErrorMsg(buff, 256);
		MessageBox(NULL, buff, _T("crInstall error, Crash Reporting disabled."), MB_OK);
	}

	// Establish the crash callback function
	crSetCrashCallback( CrashCallback, NULL );

	// Take screenshot of the app window at the moment of crash
	crAddScreenshot2(CR_AS_PROCESS_WINDOWS|CR_AS_USE_JPEG_FORMAT, 95);

	//  Mark some files to add to the crash report
	wxString home_data_crash = std_path_crash.GetConfigDir();
	if( g_bportable ) {
		wxFileName f( std_path_crash.GetExecutablePath() );
		home_data_crash = f.GetPath();
	}
	appendOSDirSlash( &home_data_crash );

	wxString config_crash = _T("opencpn.ini");
	config_crash.Prepend( home_data_crash );
	crAddFile2( config_crash.c_str(), NULL, NULL, CR_AF_MISSING_FILE_OK | CR_AF_ALLOW_DELETE );

	wxString log_crash = _T("opencpn.log");
	log_crash.Prepend( home_data_crash );
	crAddFile2( log_crash.c_str(), NULL, NULL, CR_AF_MISSING_FILE_OK | CR_AF_ALLOW_DELETE );

#endif
#endif

	//  Seed the random number generator
	wxDateTime x = wxDateTime::UNow();
	long seed = x.GetMillisecond();
	seed *= x.GetTicks();
	srand(seed);

	g_pPlatform = new wxPlatformInfo;

	//    On MSW, force the entire process to run on one CPU core only
	//    This resolves some difficulty with wxThread syncronization
#ifdef __WXMSW__
	//Gets the current process handle
	HANDLE hProc = GetCurrentProcess();
	DWORD procMask;
	DWORD sysMask;
	HANDLE hDup;
	DuplicateHandle( hProc, hProc, hProc, &hDup, 0, FALSE, DUPLICATE_SAME_ACCESS );

	//Gets the current process affinity mask
	GetProcessAffinityMask( hDup, &procMask, &sysMask );

	// Take a simple approach, and assume up to 4 processors
	DWORD newMask;
	if( ( procMask & 1 ) == 1 ) newMask = 1;
	else
		if( ( procMask & 2 ) == 2 ) newMask = 2;
		else
			if( ( procMask & 4 ) == 4 ) newMask = 4;
			else
				if( ( procMask & 8 ) == 8 ) newMask = 8;

	//Set te affinity mask for the process
	BOOL res = SetProcessAffinityMask( hDup, (DWORD_PTR) newMask );

	if( res == 0 ) {
		//Error setting affinity mask!!
	}
#endif

	//Fulup: force floating point to use dot as separation.
	// This needs to be set early to catch numerics in config file.
	//#ifdef __POSIX__
	setlocale( LC_NUMERIC, "C" );
	//#endif

	//      CALLGRIND_STOP_INSTRUMENTATION

	g_start_time = wxDateTime::Now();

	g_loglast_time = g_start_time;   // pjotrc 2010.02.09
	g_loglast_time.MakeGMT();        // pjotrc 2010.02.09
	g_loglast_time.Subtract( wxTimeSpan( 0, 29, 0, 0 ) ); // give 1 minute for GPS to get a fix   // pjotrc 2010.02.09

	AnchorPointMinDist = 5.0;

#ifdef __WXMSW__

	//    Handle any Floating Point Exceptions which may leak thru from other
	//    processes.  The exception filter is in cutil.c
	//    Seems to only happen for W98

	if( g_pPlatform->GetOperatingSystemId() == wxOS_WINDOWS_9X ) SetUnhandledExceptionFilter (&MyUnhandledExceptionFilter);
#endif

#ifdef __WXMSW__
	//     _CrtSetBreakAlloc(141542);
#endif

#ifndef __WXMSW__
	//      Setup Linux SIGNAL handling, for external program control

	//      Build the sigaction structure
	sa_all.sa_handler = catch_signals;// point to my handler
	sigemptyset(&sa_all.sa_mask);// make the blocking set
	// empty, so that all
	// other signals will be
	// unblocked during my handler
	sa_all.sa_flags = 0;

	sigaction(SIGUSR1, NULL, &sa_all_old);// save existing action for this signal

	//      Register my request for some signals
	sigaction(SIGUSR1, &sa_all, NULL);

	sigaction(SIGUSR1, NULL, &sa_all_old);// inspect existing action for this signal
#endif


	//      Init the private memory manager
	malloc_max = 0;

	//      Record initial memory status
	GetMemoryStatus( &g_mem_total, &g_mem_initial );

	// Set up default FONT encoding, which should have been done by wxWidgets some time before this......
	wxFont temp_font( 10, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, wxString( _T("") ),
			wxFONTENCODING_SYSTEM );
	temp_font.SetDefaultEncoding( wxFONTENCODING_SYSTEM );

	//      Establish a "home" location
	wxStandardPathsBase& std_path = wxApp::GetTraits()->GetStandardPaths();
	std_path.Get();

	pHome_Locn = new wxString;
#ifdef __WXMSW__
	pHome_Locn->Append( std_path.GetConfigDir() );   // on w98, produces "/windows/Application Data"
#else
	pHome_Locn->Append(std_path.GetUserConfigDir());
#endif

	if( g_bportable ) {
		pHome_Locn->Clear();
		wxFileName f( std_path.GetExecutablePath() );
		pHome_Locn->Append( f.GetPath() );
	}

	appendOSDirSlash( pHome_Locn );

	//      Establish Log File location
	glog_file = *pHome_Locn;

#ifdef  __WXOSX__
	pHome_Locn->Append(_T("opencpn"));
	appendOSDirSlash(pHome_Locn);

	wxFileName LibPref(glog_file);          // starts like "~/Library/Preferences"
	LibPref.RemoveLastDir();// takes off "Preferences"

	glog_file = LibPref.GetFullPath();
	appendOSDirSlash(&glog_file);

	glog_file.Append(_T("Logs/"));// so, on OS X, opencpn.log ends up in ~/Library/Logs
	// which makes it accessible to Applications/Utilities/Console....
#endif

	// create the opencpn "home" directory if we need to
	wxFileName wxHomeFiledir( *pHome_Locn );
	if( true != wxHomeFiledir.DirExists( wxHomeFiledir.GetPath() ) ) if( !wxHomeFiledir.Mkdir(
				wxHomeFiledir.GetPath() ) ) {
		wxASSERT_MSG(false,_T("Cannot create opencpn home directory"));
		return false;
	}

	// create the opencpn "log" directory if we need to
	wxFileName wxLogFiledir( glog_file );
	if( true != wxLogFiledir.DirExists( wxLogFiledir.GetPath() ) ) {
		if( !wxLogFiledir.Mkdir( wxLogFiledir.GetPath() ) ) {
			wxASSERT_MSG(false,_T("Cannot create opencpn log directory"));
			return false;
		}
	}
	glog_file.Append( _T("opencpn.log") );

	//  Constrain the size of the log file
	if( ::wxFileExists( glog_file ) ) {
		if( wxFileName::GetSize( glog_file ) > 1000000 ) {
			wxString oldlog = glog_file;                      // pjotrc 2010.02.09
			oldlog.Append( _T(".log") );
			wxString msg1( _("Old log will be moved to opencpn.log.log") );
			OCPNMessageBox ( NULL, msg1, wxString( _("OpenCPN Info") ),
					wxICON_INFORMATION | wxOK );
			//            int dlg_ret;
			//            dlg_ret = mdlg.ShowModal();
			::wxRenameFile( glog_file, oldlog );
		}
	}

	flog = fopen( glog_file.mb_str(), "a" );
	logger = new wxLogStderr( flog );

	Oldlogger = wxLog::SetActiveTarget( logger );

#ifdef __WXMSW__

	//  Un-comment the following to establish a separate console window as a target for printf() in Windows
	//     RedirectIOToConsole();

#endif

	//        wxLog::AddTraceMask("timer");               // verbose message traces to log output

#ifndef __WXMSW__
	logger->SetTimestamp(_T("%H:%M:%S %Z"));
#endif

	//      Send init message
	wxLogMessage( _T("\n\n________\n") );

	wxDateTime date_now = wxDateTime::Now();

	wxString imsg = date_now.FormatISODate();
	wxLogMessage( imsg );

	imsg = _T(" ------- Starting OpenCPN -------");
	wxLogMessage( imsg );

	wxString version = OpenCPNVersion;
	wxString vs = version.Trim( true );
	vs = vs.Trim( false );
	wxLogMessage( vs );

	wxString wxver(wxVERSION_STRING);
	wxver.Prepend( _T("wxWidgets version: ") );
	wxLogMessage( wxver );

	wxLogMessage( _T("MemoryStatus:  mem_total: %d mb,  mem_initial: %d mb"), g_mem_total / 1024,
			g_mem_initial / 1024 );

	//    Initialize embedded PNG icon graphics
	::wxInitAllImageHandlers();

	//      Establish a "shared data" location
	/*  From the wxWidgets documentation...

		wxStandardPaths::GetDataDir
		wxString GetDataDir() const
		Return the location of the applications global, i.e. not user-specific, data files.
	 * Unix: prefix/share/appname
	 * Windows: the directory where the executable file is located
	 * Mac: appname.app/Contents/SharedSupport bundle subdirectory
	 */
	g_SData_Locn = std_path.GetDataDir();
	appendOSDirSlash( &g_SData_Locn );

	if( g_bportable ) g_SData_Locn = *pHome_Locn;

	imsg = _T("SData_Locn is ");
	imsg += g_SData_Locn;
	wxLogMessage( imsg );

	//      Create some static strings
	pInit_Chart_Dir = new wxString();

	//  Establish an empty ChartCroupArray
	g_pGroupArray = new ChartGroupArray;

	//      Establish the prefix of the location of user specific data files
#ifdef __WXMSW__
	g_PrivateDataDir = *pHome_Locn;                     // should be {Documents and Settings}\......
#elif defined __WXOSX__
	g_PrivateDataDir = std_path.GetUserConfigDir();     // should be ~/Library/Preferences
#else
	g_PrivateDataDir = std_path.GetUserDataDir();       // should be ~/.opencpn
#endif

	if( g_bportable ) g_PrivateDataDir = *pHome_Locn;

	//  Get the PlugIns directory location
	g_Plugin_Dir = std_path.GetPluginsDir();   // linux:   {prefix}/lib/opencpn
	// Mac:     appname.app/Contents/PlugIns
#ifdef __WXMSW__
	g_Plugin_Dir += _T("\\plugins");             // Windows: {exe dir}/plugins
#endif

	if( g_bportable ) {
		g_Plugin_Dir = *pHome_Locn;
		g_Plugin_Dir += _T("plugins");
	}

	//      Create an array string to hold repeating messages, so they don't
	//      overwhelm the log
	pMessageOnceArray = new wxArrayString;

	//      Init the Route Manager
	g_pRouteMan = new Routeman( this );

	//      Init the Selectable Route Items List
	pSelect = new Select();

	//      Init the Selectable Tide/Current Items List
	pSelectTC = new Select();
	//  Increase the select radius for tide/current stations
	pSelectTC->SetSelectPixelRadius(25);

	//      Init the Selectable AIS Target List
	pSelectAIS = new Select();

	//      Initially AIS display is always on
	g_bShowAIS = true;
	g_pais_query_dialog_active = NULL;

	//      Who am I?
	phost_name = new wxString( ::wxGetHostName() );

	//      (Optionally) Capture the user and file(effective) ids
	//  Some build environments may need root privileges for hardware
	//  port I/O, as in the NMEA data input class.  Set that up here.

#ifndef __WXMSW__
#ifdef PROBE_PORTS__WITH_HELPER
	user_user_id = getuid ();
	file_user_id = geteuid ();
#endif
#endif

	//      Establish the location of the config file
#ifdef __WXMSW__
	gConfig_File = _T("opencpn.ini");
	gConfig_File.Prepend( *pHome_Locn );

#elif defined __WXOSX__
	gConfig_File = std_path.GetUserConfigDir(); // should be ~/Library/Preferences
	appendOSDirSlash(&gConfig_File);
	gConfig_File.Append(_T("opencpn.ini"));
#else
	gConfig_File = std_path.GetUserDataDir(); // should be ~/.opencpn
	appendOSDirSlash(&gConfig_File);
	gConfig_File.Append(_T("opencpn.conf"));
#endif

	if( g_bportable ) {
		gConfig_File = *pHome_Locn;
#ifdef __WXMSW__
		gConfig_File += _T("opencpn.ini");
#elif defined __WXOSX__
		gConfig_File +=_T("opencpn.ini");
#else
		gConfig_File += _T("opencpn.conf");
#endif

	}

	bool b_novicemode = false;

	wxFileName config_test_file_name( gConfig_File );
	if( config_test_file_name.FileExists() ) wxLogMessage(
			_T("Using existing Config_File: ") + gConfig_File );
	else {
		{
			wxLogMessage( _T("Creating new Config_File: ") + gConfig_File );

			//    Flag to preset some options for initial config file creation
			b_novicemode = true;

			if( true != config_test_file_name.DirExists( config_test_file_name.GetPath() ) ) if( !config_test_file_name.Mkdir(
						config_test_file_name.GetPath() ) ) wxLogMessage(
					_T("Cannot create config file directory for ") + gConfig_File );
		}
	}

	// Now initialize UI Style.
	g_StyleManager = new ocpnStyle::StyleManager();

	if( !g_StyleManager->IsOK() ) {
		wxString msg = _("Failed to initialize the user interface. ");
		msg << _("OpenCPN cannot start. ");
		msg << _("The necessary configuration files were not found. ");
		msg << _("See the log file at ") << glog_file << _(" for details.");
		wxMessageDialog w( NULL, msg, _("Failed to initialize the user interface. "),
				wxCANCEL | wxICON_ERROR );
		w.ShowModal();
		exit( EXIT_FAILURE );
	}

	//      Init the WayPoint Manager (Must be after UI Style init).
	pWayPointMan = new WayPointman();
	pWayPointMan->ProcessIcons( g_StyleManager->GetCurrentStyle() );

	//      Open/Create the Config Object (Must be after UI Style init).
	MyConfig *pCF = new MyConfig( wxString( _T("") ), wxString( _T("") ), gConfig_File );
	pConfig = (MyConfig *) pCF;
	pConfig->LoadMyConfig(0);

	//        Is this the first run after a clean install?
	if( !n_NavMessageShown )
		g_bFirstRun = true;

	//  Now we can set the locale

	//    Manage internationalization of embedded messages
	//    using wxWidgets/gettext methodology....

	// Add a new prefix for search order.
#ifdef __WXMSW__
	wxString locale_location = g_SData_Locn;
	locale_location += _T("share/locale");
	wxLocale::AddCatalogLookupPathPrefix( locale_location );
#endif

	//  Get the default language info
	wxString def_lang_canonical;
	const wxLanguageInfo* languageInfo = wxLocale::GetLanguageInfo(wxLANGUAGE_DEFAULT);
	if( languageInfo ) {
		def_lang_canonical = languageInfo->CanonicalName;
		imsg = _T("System default Language:  ");
		imsg += def_lang_canonical;
		wxLogMessage( imsg );
	}

	//  For windows, installer may have left information in the registry defining the
	//  user's selected install language.
	//  If so, override the config file value and use this selection for opencpn...
#ifdef __WXMSW__
	if( g_bFirstRun ) {
		wxRegKey RegKey( wxString( _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\OpenCPN") ) );
		if( RegKey.Exists() ) {
			wxLogMessage( _("Retrieving initial language selection from Windows Registry") );
			RegKey.QueryValue( wxString( _T("InstallerLanguage") ), g_locale );
		}
	}
#endif
	//  Find the language specified by the config file
	const wxLanguageInfo *pli = wxLocale::FindLanguageInfo( g_locale );
	wxString loc_lang_canonical;
	bool b_initok;
	plocale_def_lang = new wxLocale;

	if( pli ) {
		b_initok = plocale_def_lang->Init( pli->Language, 0 );
		loc_lang_canonical = pli->CanonicalName;
	}

	if( !pli || !b_initok ) {
		delete plocale_def_lang;
		plocale_def_lang = new wxLocale;
		plocale_def_lang->Init( wxLANGUAGE_ENGLISH_US, 0 );
		loc_lang_canonical = wxLocale::GetLanguageInfo( wxLANGUAGE_ENGLISH_US )->CanonicalName;
	}

	imsg = _T("Opencpn language set to:  ");
	imsg += loc_lang_canonical;
	wxLogMessage( imsg );

	// Set filename without extension (example : opencpn_fr_FR)
	// i.e. : Set-up the filename needed for translation
	//        wxString loc_lang_filename = _T("opencpn_") + loc_lang_canonical;
	wxString loc_lang_filename = _T("opencpn");

	// Get translation file (example : opencpn_fr_FR.mo)
	// No problem if the file doesn't exist
	// as this case is handled by wxWidgets
	if( plocale_def_lang ) plocale_def_lang->AddCatalog( loc_lang_filename );

	//    Always use dot as decimal
	setlocale( LC_NUMERIC, "C" );

	wxLog::SetVerbose( false );           // log no verbose messages

	//  French language locale is assumed to include the AZERTY keyboard
	//  This applies to either the system language, or to OpenCPN language selection
	if( loc_lang_canonical == _T("fr_FR") ) g_b_assume_azerty = true;
	if( def_lang_canonical == _T("fr_FR") ) g_b_assume_azerty = true;

	//  Send the Welcome/warning message if it has never been sent before,
	//  or if the version string has changed at all
	//  We defer until here to allow for localization of the message
	if( !n_NavMessageShown || ( vs != g_config_version_string ) ) {
		if( wxID_CANCEL == ShowNavWarning() ) return false;
		n_NavMessageShown = 1;
	}

	g_config_version_string = vs;

#ifdef USE_S57

	//      Set up a useable CPL library error handler for S57 stuff
	CPLSetErrorHandler( MyCPLErrorHandler );

	//      Init the s57 chart object, specifying the location of the required csv files
	g_csv_locn = g_SData_Locn;
	g_csv_locn.Append( _T("s57data") );

	if( g_bportable ) {
		g_csv_locn = _T(".");
		appendOSDirSlash( &g_csv_locn );
		g_csv_locn.Append( _T("s57data") );
	}

	//      If the config file contains an entry for SENC file prefix, use it.
	//      Otherwise, default to PrivateDataDir
	if( g_SENCPrefix.IsEmpty() ) {
		g_SENCPrefix = g_PrivateDataDir;
		appendOSDirSlash( &g_SENCPrefix );
		g_SENCPrefix.Append( _T("SENC") );
	}

	if( g_bportable ) {
		wxFileName f( g_SENCPrefix );
		if( f.MakeRelativeTo( g_PrivateDataDir ) ) g_SENCPrefix = f.GetFullPath();
		else
			g_SENCPrefix = _T("SENC");
	}

	//      If the config file contains an entry for PresentationLibraryData, use it.
	//      Otherwise, default to conditionally set spot under g_pcsv_locn
	wxString plib_data;
	bool b_force_legacy = false;

	if( g_UserPresLibData.IsEmpty() ) {
		plib_data = g_csv_locn;
		appendOSDirSlash( &plib_data );
		plib_data.Append( _T("S52RAZDS.RLE") );
	} else {
		plib_data = g_UserPresLibData;
		b_force_legacy = true;
	}

	ps52plib = new s52plib( plib_data, b_force_legacy );

	//  If the library load failed, try looking for the s57 data elsewhere

	//  First, look in UserDataDir
	/*    From wxWidgets documentation

		  wxStandardPaths::GetUserDataDir
		  wxString GetUserDataDir() const
		  Return the directory for the user-dependent application data files:
	 * Unix: ~/.appname
	 * Windows: C:\Documents and Settings\username\Application Data\appname
	 * Mac: ~/Library/Application Support/appname
	 */

	if( !ps52plib->m_bOK ) {
		delete ps52plib;

		wxString look_data_dir;
		look_data_dir.Append( std_path.GetUserDataDir() );
		appendOSDirSlash( &look_data_dir );
		wxString tentative_SData_Locn = look_data_dir;
		look_data_dir.Append( _T("s57data") );

		plib_data = look_data_dir;
		appendOSDirSlash( &plib_data );
		plib_data.Append( _T("S52RAZDS.RLE") );

		wxLogMessage( _T("Looking for s57data in ") + look_data_dir );
		ps52plib = new s52plib( plib_data );

		if( ps52plib->m_bOK ) {
			g_csv_locn = look_data_dir;
			g_SData_Locn = tentative_SData_Locn;
		}
	}

	//  And if that doesn't work, look again in the original SData Location
	//  This will cover the case in which the .ini file entry is corrupted or moved

	if( !ps52plib->m_bOK ) {
		delete ps52plib;

		wxString look_data_dir;
		look_data_dir = g_SData_Locn;
		look_data_dir.Append( _T("s57data") );

		plib_data = look_data_dir;
		appendOSDirSlash( &plib_data );
		plib_data.Append( _T("S52RAZDS.RLE") );

		wxLogMessage( _T("Looking for s57data in ") + look_data_dir );
		ps52plib = new s52plib( plib_data );

		if( ps52plib->m_bOK ) g_csv_locn = look_data_dir;
	}

	if( ps52plib->m_bOK ) wxLogMessage( _T("Using s57data in ") + g_csv_locn );
	else
		wxLogMessage( _T("   S52PLIB Initialization failed, disabling Vector charts.") );

	// Todo Maybe initialize only when an s57 chart is actually opened???
	if( ps52plib->m_bOK ) m_pRegistrarMan = new S57RegistrarMgr( g_csv_locn, flog );

	if( !ps52plib->m_bOK ) {
		delete ps52plib;
		ps52plib = NULL;
	}

#endif  // S57
	// Set default color scheme
	global_color_scheme = GLOBAL_COLOR_SCHEME_DAY;

	// On Windows platforms, establish a default cache managment policy
	// as allowing OpenCPN a percentage of available physical memory,
	// not to exceed 1 GB
	// Note that this logic implies that Windows platforms always use
	// the memCacheLimit policy, and never use the fallback nCacheLimit policy
#ifdef __WXMSW__
	if( 0 == g_memCacheLimit )
		g_memCacheLimit = (int) ( g_mem_total * 0.5 );
	g_memCacheLimit = wxMin(g_memCacheLimit, 1024 * 1024); // math in kBytes
#endif

	//      Establish location and name of chart database
#ifdef __WXMSW__
	pChartListFileName = new wxString( _T("CHRTLIST.DAT") );
	pChartListFileName->Prepend( *pHome_Locn );

#else
	pChartListFileName = new wxString(_T(""));
	pChartListFileName->Append(std_path.GetUserDataDir());
	appendOSDirSlash(pChartListFileName);
	pChartListFileName->Append(_T("chartlist.dat"));
#endif

	if( g_bportable ) {
		pChartListFileName->Clear();
#ifdef __WXMSW__
		pChartListFileName->Append( _T("CHRTLIST.DAT") );
#else
		pChartListFileName->Append(_T("chartlist.dat"));
#endif
		pChartListFileName->Prepend( *pHome_Locn );
	}

	//      Establish guessed location of chart tree
	if( pInit_Chart_Dir->IsEmpty() ) {
		if( !g_bportable )
			pInit_Chart_Dir->Append( std_path.GetDocumentsDir() );
	}

	//      Establish the GSHHS Dataset location
	pWorldMapLocation = new wxString( _T("gshhs") );
	pWorldMapLocation->Prepend( g_SData_Locn );
	pWorldMapLocation->Append( wxFileName::GetPathSeparator() );

	//      Reload the config data, to pick up any missing data class configuration info
	//      e.g. s52plib, which could not be created until first config load completes
	//      Think catch-22
	pConfig->LoadMyConfig(1);

	//  Override some config options for initial user startup with empty config file
	if( b_novicemode ) {
		g_bShowOutlines = true;
		g_CPAMax_NM = 20.;
		g_CPAWarn_NM = 2.;
		g_TCPA_Max = 30.;
		g_bMarkLost = true;
		g_MarkLost_Mins = 8;
		g_bRemoveLost = true;
		g_RemoveLost_Mins = 10;
		g_bShowCOG = true;
		g_ShowCOG_Mins = 6;
		g_bShowMoored = true;
		g_ShowMoored_Kts = 0.2;
		g_bTrackDaily = false;
		g_PlanSpeed = 6.;
		g_bFullScreenQuilt = true;
		g_bQuiltEnable = true;
		g_bskew_comp = false;
		g_bShowAreaNotices = false;
		g_bDrawAISSize = false;
		g_bShowAISName = false;

#ifdef USE_S57
		if( ps52plib && ps52plib->m_bOK ) {
			ps52plib->m_bShowSoundg = true;
			ps52plib->m_nDisplayCategory = (enum _DisCat) STANDARD;
			ps52plib->m_nSymbolStyle = (LUPname) PAPER_CHART;
			ps52plib->m_nBoundaryStyle = (LUPname) PLAIN_BOUNDARIES;
			ps52plib->m_bUseSCAMIN = true;
			ps52plib->m_bShowAtonText = true;

			//    Preset some object class visibilites for "Mariner's Standard" disply category
			for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
				OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
				if( !strncmp( pOLE->OBJLName, "DEPARE", 6 ) ) pOLE->nViz = 1;
				if( !strncmp( pOLE->OBJLName, "LNDARE", 6 ) ) pOLE->nViz = 1;
				if( !strncmp( pOLE->OBJLName, "COALNE", 6 ) ) pOLE->nViz = 1;
			}

		}
#endif

	}

	//  Check the global Tide/Current data source array
	//  If empty, preset one default (US) Ascii data source
	if(!TideCurrentDataSet.GetCount()) {
		wxString default_tcdata =  ( g_SData_Locn + _T("tcdata") +
				wxFileName::GetPathSeparator() +
				_T("HARMONIC.IDX"));

		if( g_bportable ) {
			wxFileName f( default_tcdata );
			f.MakeRelativeTo( g_PrivateDataDir );
			TideCurrentDataSet.Add( f.GetFullPath() );
		}
		else
			TideCurrentDataSet.Add( default_tcdata );
	}


	//  Check the global AIS alarm sound file
	//  If empty, preset default
	if(g_sAIS_Alert_Sound_File.IsEmpty()) {
		wxString default_sound =  ( g_SData_Locn + _T("sounds") +
				wxFileName::GetPathSeparator() +
				_T("2bells.wav"));

		if( g_bportable ) {
			wxFileName f( default_sound );
			f.MakeRelativeTo( g_PrivateDataDir );
			g_sAIS_Alert_Sound_File = f.GetFullPath();
		}
		else
			g_sAIS_Alert_Sound_File = default_sound ;
	}


	g_StartTime = wxInvalidDateTime;
	g_StartTimeTZ = 1;                // start with local times
	gpIDX = NULL;
	gpIDXn = 0;

	//  Set up the frame initial visual parameters
	//      Default size, resized later
	wxSize new_frame_size( -1, -1 );
	int cx, cy, cw, ch;
	::wxClientDisplayRect( &cx, &cy, &cw, &ch );

	InitializeUserColors();

	if( ( g_nframewin_x > 100 ) && ( g_nframewin_y > 100 ) && ( g_nframewin_x <= cw )
			&& ( g_nframewin_y <= ch ) ) new_frame_size.Set( g_nframewin_x, g_nframewin_y );
	else
		new_frame_size.Set( cw * 7 / 10, ch * 7 / 10 );

	//  Try to detect any change in physical screen configuration
	//  This can happen when drivers are changed, for instance....
	//  and can confuse the WUI layout perspective stored in the config file.
	//  If detected, force a nominal window size and position....
	if( ( g_lastClientRectx != cx ) || ( g_lastClientRecty != cy ) || ( g_lastClientRectw != cw )
			|| ( g_lastClientRecth != ch ) ) {
		new_frame_size.Set( cw * 7 / 10, ch * 7 / 10 );
		g_bframemax = false;
	}

	g_lastClientRectx = cx;
	g_lastClientRecty = cy;
	g_lastClientRectw = cw;
	g_lastClientRecth = ch;

	//  Validate config file position
	wxPoint position( 0, 0 );
	wxSize dsize = wxGetDisplaySize();

#ifdef __WXMAC__
	g_nframewin_posy = wxMax(g_nframewin_posy, 22);
#endif

	if( ( g_nframewin_posx < dsize.x ) && ( g_nframewin_posy < dsize.y ) ) position = wxPoint(
			g_nframewin_posx, g_nframewin_posy );

#ifdef __WXMSW__
	//  Support MultiMonitor setups which an allow negative window positions.
	RECT frame_rect;
	frame_rect.left = position.x;
	frame_rect.top = position.y;
	frame_rect.right = position.x + new_frame_size.x;
	frame_rect.bottom = position.y + new_frame_size.y;

	//  If the requested frame window does not intersect any installed monitor,
	//  then default to simple primary monitor positioning.
	if( NULL == MonitorFromRect( &frame_rect, MONITOR_DEFAULTTONULL ) ) position = wxPoint( 10,
			10 );
#endif

	//  For Windows and GTK, provide the expected application Minimize/Close bar
	long app_style = wxDEFAULT_FRAME_STYLE;
	app_style |= wxWANTS_CHARS;

	// Create the main frame window
	wxString myframe_window_title = wxT("OpenCPN ") + str_version_major + wxT(".")
		+ str_version_minor + wxT(".") + str_version_patch; //Gunther

	if( g_bportable ) {
		myframe_window_title += _(" -- [Portable(-p) executing from ");
		myframe_window_title += *pHome_Locn;
		myframe_window_title += _T("]");
	}

	gFrame = new MyFrame( NULL, myframe_window_title, position, new_frame_size, app_style ); //Gunther

	g_pauimgr = new wxAuiManager;
	//        g_pauidockart= new wxAuiDefaultDockArt;
	//        g_pauimgr->SetArtProvider(g_pauidockart);

	// tell wxAuiManager to manage the frame
	g_pauimgr->SetManagedWindow( gFrame );

	//      Create Children of Frame
	//              n.b.  if only one child exists, wxWindows expands the child
	//                        to the parent client area automatically, (as a favor?)
	//                        Here, we'll do explicit sizing on SIZE events

	cc1 = new ChartCanvas( gFrame );                         // the chart display canvas
	gFrame->SetCanvasWindow( cc1 );

	cc1->SetQuiltMode( g_bQuiltEnable );                     // set initial quilt mode
	cc1->m_bFollow = pConfig->st_bFollow;               // set initial state
	cc1->SetViewPoint( vLat, vLon, initial_scale_ppm, 0., 0. );

	gFrame->Enable();

	cc1->SetFocus();

	console = new ConsoleCanvas( gFrame );                    // the console

	pthumbwin = new ThumbWin( cc1 );

	gFrame->ApplyGlobalSettings( 1, false );               // done once on init with resize

	gui_instance->ensure_toolbar_position_range(wxPoint(0, 0), wxPoint(cw, ch));
	gui_instance->ensure_ais_alert_dialog_position_range(wxPoint(0, 0), wxGetDisplaySize());

	g_FloatingToolbarDialog = new OCPNFloatingToolbarDialog(
		cc1,
		gui_instance->get_toolbar().position,
		gui_instance->get_toolbar().orientation);
	g_FloatingToolbarDialog->LockPosition(true);

	gFrame->SetAndApplyColorScheme( global_color_scheme );

	//  The position and size of the static frame children (i.e. the canvas, and the status bar) are now set
	//  So now we can establish the AUI panes for them.
	//  It is important to have set the chartcanvas and status bar sizes before this point,
	//  so that the pane.BestSize values are correctly captured by the AuiManager.

	g_pauimgr->AddPane(cc1);
	g_pauimgr->GetPane(cc1).Name( _T("ChartCanvas") );
	g_pauimgr->GetPane(cc1).Fixed();
	g_pauimgr->GetPane(cc1).CaptionVisible( false );
	g_pauimgr->GetPane(cc1).CenterPane();
	g_pauimgr->GetPane(cc1).BestSize( cc1->GetSize() );

	//      Load and initialize any PlugIns
	g_pi_manager = new PlugInManager( gFrame );
	g_pi_manager->LoadAllPlugIns( g_Plugin_Dir );

	// Show the frame

	gFrame->ClearBackground();
	gFrame->Show( TRUE );

	if( g_bframemax ) gFrame->Maximize( true );

	stats = new StatWin( cc1 );
	stats->SetColorScheme( global_color_scheme );

	ocpnStyle::Style * style = g_StyleManager->GetCurrentStyle();

	if( cc1->GetQuiltMode() ) {
		stats->pPiano->SetVizIcon( new wxBitmap( style->GetIcon( _T("viz") ) ) );
		stats->pPiano->SetInVizIcon( new wxBitmap( style->GetIcon( _T("redX") ) ) );

		stats->pPiano->SetRoundedRectangles( true );
	}
	stats->pPiano->SetTMercIcon( new wxBitmap( style->GetIcon( _T("tmercprj") ) ) );
	stats->pPiano->SetPolyIcon( new wxBitmap( style->GetIcon( _T("polyprj") ) ) );
	stats->pPiano->SetSkewIcon( new wxBitmap( style->GetIcon( _T("skewprj") ) ) );

	//  Yield to pick up the OnSize() calls that result from Maximize()
	Yield();

	wxString perspective;
	pConfig->SetPath( _T ( "/AUI" ) );
	pConfig->Read( _T ( "AUIPerspective" ), &perspective );

	// Make sure the perspective saved in the config file is "reasonable"
	// In particular, the perspective should have an entry for every
	// windows added to the AUI manager so far.
	// If any are not found, then use the default layout

	bool bno_load = false;
	wxAuiPaneInfoArray pane_array_val = g_pauimgr->GetAllPanes();

	for( unsigned int i = 0; i < pane_array_val.GetCount(); i++ ) {
		wxAuiPaneInfo pane = pane_array_val.Item( i );
		if( perspective.Find( pane.name ) == wxNOT_FOUND ) {
			bno_load = true;
			break;
		}
	}

	if( !bno_load ) g_pauimgr->LoadPerspective( perspective, false );


	g_pauimgr->Update();

	//   Notify all the AUI PlugIns so that they may syncronize with the Perspective
	g_pi_manager->NotifyAuiPlugIns();

	//   Initialize and Save the existing Screen Brightness
	//       InitScreenBrightness();

	bool b_SetInitialPoint = false;

	//   Build the initial chart dir array
	ArrayOfCDI ChartDirArray;
	pConfig->LoadChartDirArray( ChartDirArray );

	//  Windows installer may have left hints regarding the initial chart dir selection
#ifdef __WXMSW__
	if( g_bFirstRun ) {
		int ndirs = 0;

		wxRegKey RegKey( wxString( _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\OpenCPN") ) );
		if( RegKey.Exists() ) {
			wxLogMessage( _("Retrieving initial Chart Directory set from Windows Registry") );
			wxString dirs;
			RegKey.QueryValue( wxString( _T("ChartDirs") ), dirs );

			wxStringTokenizer tkz( dirs, _T(";") );
			while( tkz.HasMoreTokens() ) {
				wxString token = tkz.GetNextToken();

				ChartDirInfo cdi;
				cdi.fullpath = token.Trim();
				cdi.magic_number = _T("");

				ChartDirArray.Add( cdi );
				ndirs++;
			}

		}

		if( ndirs ) pConfig->UpdateChartDirs( ChartDirArray );

		//    As a favor to new users, poll the database and
		//    move the initial viewport so that a chart will come up.
		if( ndirs ) b_SetInitialPoint = true;

	}
#endif

	//    If the ChartDirArray is empty at this point, any existing chart database file must be declared invalid,
	//    So it is best to simply delete it if present.
	//    TODO  There is a possibility of recreating the dir list from the database itself......

	if( !ChartDirArray.GetCount() ) ::wxRemoveFile( *pChartListFileName );

	//      Try to load the current chart list Data file
	ChartData = new ChartDB( gFrame );
	if (!ChartData->LoadBinary(*pChartListFileName, ChartDirArray)) {
		bDBUpdateInProgress = true;

		if( ChartDirArray.GetCount() ) {
			//              Create and Save a new Chart Database based on the hints given in the config file

			/*
			   wxString msg1(_("OpenCPN needs to update the chart database from config file entries...."));

			   OCPNMessageDialog mdlg(gFrame, msg1, wxString(_("OpenCPN Info")),wxICON_INFORMATION | wxOK );
			   int dlg_ret;
			   dlg_ret = mdlg.ShowModal();
			 */
			delete ChartData;
			ChartData = new ChartDB( gFrame );

			wxString line( _("Rebuilding chart database from configuration file entries...") );
			/* The following 3 strings are embeded in wxProgressDialog but must be included by xgettext
			 * to be localized properly. See {wxWidgets}src/generic/progdlgg.cpp:190 */
			wxString dummy1 = _("Elapsed time : ");
			wxString dummy2 = _("Estimated time : ");
			wxString dummy3 = _("Remaining time : ");
			wxProgressDialog *pprog = new wxProgressDialog( _("OpenCPN Chart Update"), line, 100,
					NULL,
					wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME );

			ChartData->Create( ChartDirArray, pprog );
			ChartData->SaveBinary(*pChartListFileName);

			delete pprog;
		}

		else            // No chart database, no config hints, so bail to Options....
		{
			wxLogMessage(
					_T("Chartlist file not found, config chart dir array is empty.  Chartlist target file is:")
					+ *pChartListFileName );

			wxString msg1(
					_("No Charts Installed.\nPlease select chart folders in Options > Charts.") );

			OCPNMessageBox(gFrame, msg1, wxString( _("OpenCPN Info") ), wxICON_INFORMATION | wxOK );

			gFrame->DoOptionsDialog();

			b_SetInitialPoint = true;

		}

		bDBUpdateInProgress = false;

		//    As a favor to new users, poll the database and
		//    move the initial viewport so that a chart will come up.

		if( b_SetInitialPoint ) {
			double clat, clon;
			if( ChartData->GetCentroidOfLargestScaleChart( &clat, &clon, CHART_FAMILY_RASTER ) ) {
				gLat = clat;
				gLon = clon;
				gFrame->ClearbFollow();
			} else {
				if( ChartData->GetCentroidOfLargestScaleChart( &clat, &clon,
							CHART_FAMILY_VECTOR ) ) {
					gLat = clat;
					gLon = clon;
					gFrame->ClearbFollow();
				}
			}
		}

	}

	//  Apply the inital Group Array structure to the chart data base
	ChartData->ApplyGroupArray( g_pGroupArray );

	//  Make sure that the Selected Group is sensible...
	if( g_GroupIndex > (int) g_pGroupArray->GetCount() )
		g_GroupIndex = 0;
	if( !gFrame->CheckGroup( g_GroupIndex ) )
		g_GroupIndex = 0;

	//  Delete any stack built by no-chart startup case
	if( pCurrentStack )
		delete pCurrentStack;

	pCurrentStack = new ChartStack;

	//  A useability enhancement....
	//  if the chart database is truly empty on startup, switch to SCMode
	//  so that the WVS chart will at least be shown
	if( ChartData && ( 0 == ChartData->GetChartTableEntries() ) ) {
		cc1->SetQuiltMode( false );
		gFrame->SetupQuiltMode();
	}

	//      All set to go.....

	//      establish GPS timeout value as multiple of frame timer
	//      This will override any nonsense or unset value from the config file
	if( ( gps_watchdog_timeout_ticks > 60 ) || ( gps_watchdog_timeout_ticks <= 0 ) )
		gps_watchdog_timeout_ticks = ( GPS_TIMEOUT_SECONDS * 1000 ) / TIMER_GFRAME_1;

	wxString dogmsg;
	dogmsg.Printf( _T("GPS Watchdog Timeout is: %d sec."), gps_watchdog_timeout_ticks );
	wxLogMessage( dogmsg );

	sat_watchdog_timeout_ticks = 12;

	gGPS_Watchdog = 2;
	gHDx_Watchdog = 2;
	gHDT_Watchdog = 2;
	gSAT_Watchdog = 2;
	gVAR_Watchdog = 2;

	//  Most likely installations have no ownship heading information
	g_bHDT_Rx = false;
	g_bVAR_Rx = false;

	//  Start up a new track if enabled in config file
	//        test this
	if( g_bTrackCarryOver )
		gFrame->TrackOn();

	//    Re-enable anchor watches if set in config file
	if( !g_AW1GUID.IsEmpty() ) {
		pAnchorWatchPoint1 = pWayPointMan->FindRoutePointByGUID( g_AW1GUID );
	}
	if( !g_AW2GUID.IsEmpty() ) {
		pAnchorWatchPoint2 = pWayPointMan->FindRoutePointByGUID( g_AW2GUID );
	}

	stats->Show( true );

	gFrame->DoChartUpdate();

	g_FloatingToolbarDialog->LockPosition(false);

	gFrame->RequestNewToolbar();

	//      Start up the ticker....
	gFrame->FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

	//      Start up the ViewPort Rotation angle Averaging Timer....
	gFrame->FrameCOGTimer.Start( 10, wxTIMER_CONTINUOUS );

	//        gFrame->MemFootTimer.Start(wxMax(g_MemFootSec * 1000, 60 * 1000), wxTIMER_CONTINUOUS);
	//        gFrame->MemFootTimer.Start(1000, wxTIMER_CONTINUOUS);

	// Import Layer-wise any .gpx files from /Layers directory
	wxString layerdir = g_PrivateDataDir;
	appendOSDirSlash( &layerdir );
	layerdir.Append( _T("layers") );

	if( wxDir::Exists( layerdir ) ) {
		wxString laymsg;
		laymsg.Printf( wxT("Getting .gpx layer files from: %s"), layerdir.c_str() );
		wxLogMessage( laymsg );

		pConfig->LoadLayers(layerdir);
	}

	cc1->ReloadVP();                  // once more, and good to go

	g_FloatingCompassDialog = new FloatingCompassWindow(cc1);
	if( g_FloatingCompassDialog )
		g_FloatingCompassDialog->UpdateStatus( true );

	g_FloatingToolbarDialog->Raise();
	g_FloatingToolbarDialog->Show();

	gFrame->Refresh( false );
	gFrame->Raise();

	cc1->Enable();
	cc1->SetFocus();

	//  This little hack fixes a problem seen with some UniChrome OpenGL drivers
	//  We need a deferred resize to get glDrawPixels() to work right.
	//  So we set a trigger to generate a resize after 5 seconds....
	//  See the "UniChrome" hack elsewhere
	if ( !g_bdisable_opengl )
	{
		glChartCanvas *pgl = (glChartCanvas *) cc1->GetglCanvas();
		if( pgl && ( pgl->GetRendererString().Find( _T("UniChrome") ) != wxNOT_FOUND ) )
		{
			gFrame->m_defer_size = gFrame->GetSize();
			gFrame->SetSize( gFrame->m_defer_size.x - 10, gFrame->m_defer_size.y );
			g_pauimgr->Update();
			gFrame->m_bdefer_resize = true;
		}
	}

	g_pi_manager->CallLateInit();

	return TRUE;
}

int App::OnExit()
{
	//  Send current nav status data to log file   // pjotrc 2010.02.09

	wxDateTime lognow = wxDateTime::Now();
	lognow.MakeGMT();
	wxString day = lognow.FormatISODate();
	wxString utc = lognow.FormatISOTime();
	wxString navmsg = _T("LOGBOOK:  ");
	navmsg += day;
	navmsg += _T(" ");
	navmsg += utc;
	navmsg += _T(" UTC ");

	if( bGPSValid ) {
		wxString data;
		data.Printf( _T("OFF: Lat %10.5f Lon %10.5f "), gLat, gLon );
		navmsg += data;

		wxString cog;
		if( wxIsNaN(gCog) ) cog.Printf( _T("COG ----- ") );
		else
			cog.Printf( _T("COG %10.5f "), gCog );

		wxString sog;
		if( wxIsNaN(gSog) ) sog.Printf( _T("SOG -----  ") );
		else
			sog.Printf( _T("SOG %6.2f ") + getUsrSpeedUnit(), toUsrSpeed( gSog ) );

		navmsg += cog;
		navmsg += sog;

	} else {
		wxString data;
		data.Printf( _T("OFF: Lat %10.5f Lon %10.5f"), gLat, gLon );
		navmsg += data;
	}
	wxLogMessage( navmsg );
	g_loglast_time = lognow;

	if( ptcmgr ) delete ptcmgr;

	wxLogMessage( _T("opencpn::App exiting cleanly...\n") );
	delete pConfig;
	delete pSelect;
	delete pSelectTC;
	delete pSelectAIS;

#ifdef USE_S57
	delete ps52plib;
#endif

	delete g_pGroupArray;
	delete pDummyChart;

	if( logger ) {
		wxLog::SetActiveTarget( Oldlogger );
		delete logger;
	}

	delete pChartListFileName;
	delete pHome_Locn;
	delete phost_name;
	delete pInit_Chart_Dir;
	delete pWorldMapLocation;

	delete g_pRouteMan;
	delete pWayPointMan;

	delete pMessageOnceArray;

	DeInitializeUserColors();

	delete pLayerList;

#ifdef USE_S57
	delete m_pRegistrarMan;
	CSVDeaccess( NULL );
#endif

	delete g_StyleManager;

#ifdef USE_S57
#ifdef __WXMSW__
#ifdef USE_GLU_TESS
#ifdef USE_GLU_DLL
	if(s_glu_dll_ready)
		FreeLibrary(s_hGLU_DLL);           // free the glu32.dll
#endif
#endif
#endif
#endif

#ifdef OCPN_USE_PORTAUDIO
	if(portaudio_initialized)
		Pa_Terminate();
#endif


	//      Restore any changed system colors
#ifdef __WXMSW__
	RestoreSystemColors();
#endif

#ifdef __MSVC__LEAK
	DeInitAllocCheck();
#endif

	delete g_pPlatform;
	delete g_pauimgr;

	delete plocale_def_lang;

#ifdef __WXMSW__
	delete m_checker;
#endif

#ifdef OCPN_USE_CRASHRPT
#ifndef _DEBUG
	// Uninstall Windows crash reporting
	crUninstall();
#endif
#endif

	return TRUE;
}

void App::TrackOff(void)
{
	if (gFrame)
		gFrame->TrackOff();
}

