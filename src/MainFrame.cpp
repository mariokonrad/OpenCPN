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

#ifdef OCPN_USE_CRASHRPT
	#include "CrashRpt.h"
#endif

#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/artprov.h>
#include <wx/stdpaths.h>
#include <wx/intl.h>
#include <wx/listctrl.h>
#include <wx/aui/aui.h>
#include <wx/dialog.h>
#include <wx/progdlg.h>
#include <wx/image.h>
#include <wx/apptrait.h>

#if wxCHECK_VERSION(2, 9, 0)
	#include <wx/dialog.h>
#endif

#ifdef __WXMSW__
	#include <stdlib.h>
	#include <math.h>
	#include <time.h>
	#include <psapi.h>
#endif

#ifndef __WXMSW__
	#include <signal.h>
	#include <setjmp.h>
#endif

#include <algorithm>

#include "MainFrame.h"
#include <version.h>
#include "dychart.h"
#include "ChartCanvas.h"
#include "TimedPopupWin.h"
#include "MessageBox.h"
#include "WayPointman.h"
#include "Track.h"
#include "StyleManager.h"
#include "Style.h"
#include "Routeman.h"
#include "StatWin.h"
#include "PianoWin.h"
#include "ConsoleCanvas.h"
#include "OptionDialog.h"
#include "AboutDialog.h"
#include "ThumbWin.h"
#include "RouteProp.h"
#include "MarkInfo.h"
#include "ToolBarSimple.h"
#include "FloatingCompassWindow.h"
#include "DataStream.h"
#include "OCPN_DataStreamEvent.h"
#include "Multiplexer.h"
#include "Select.h"
#include "FontMgr.h"
#include "NMEALogWindow.h"
#include "Layer.h"
#include "NavObjectCollection.h"
#include "OCP_DataStreamInput_Thread.h"
#include "TrackPropDlg.h"
#include "RouteManagerDialog.h"
#include "MyPrintout.h"
#include "OCPNFloatingToolbarDialog.h"
#include "NavObjectChanges.h"
#include "MicrosoftCompatibility.h"
#include "StatusBar.h"

#include <GUI_IDs.h>
#include <AnchorDist.h>
#include <MemoryStatus.h>
#include <Config.h>
#include <UserColors.h>
#include <PositionParser.h>
#include <Units.h>

#include <plugin/PlugInManager.h>
#include <plugin/OCPN_MsgEvent.h>

#include <tide/TCMgr.h>

#include <global/OCPN.h>
#include <global/GUI.h>
#include <global/Navigation.h>
#include <global/WatchDog.h>

#include <ais/ais.h>
#include <ais/AISTargetListDialog.h>
#include <ais/AISTargetAlertDialog.h>
#include <ais/AIS_Decoder.h>

#include <chart/ChartDB.h>
#include <chart/ChartStack.h>
#include <chart/CacheEntry.h>
#include <chart/ChartBaseBSB.h>
#include <chart/ChartDummy.h>
#include <chart/gshhs/GSHHSChart.h>

#include <wxJSON/jsonreader.h>

#ifdef USE_S57
	#include "S57RegistrarMgr.h"
	#include "S57QueryDialog.h"
	#include <chart/s52plib.h>
	#include <chart/CM93OffsetDialog.h>
	#include <chart/S57Chart.h>
	#include "cpl_csv.h"
#endif

#ifdef __WXMSW__
//#define __MSVC__LEAK
#ifdef __MSVC__LEAK
#include "Stackwalker.h"
#endif
#endif

#ifdef OCPN_USE_PORTAUDIO
	#include "portaudio.h"
#endif

#ifdef __WXMSW__
void RedirectIOToConsole();
#endif


// Define a timer value for Tide/Current updates
// Note that the underlying data algorithms produce fresh data only every 15 minutes
// So maybe 5 minute updates should provide sufficient oversampling
static const unsigned int TIMER_TC_VALUE_SECONDS = 300;

static char nmea_tick_chars[] = { '|', '/', '-', '\\', '|', '/', '-', '\\' };
static int tick_idx;

extern wxString str_version_major;
extern wxString str_version_minor;
extern wxString str_version_patch;
extern wxString OpenCPNVersion;

FILE *flog;
wxLog *logger;
wxLog *Oldlogger;
bool g_bFirstRun;
wxString glog_file;
wxString gConfig_File;
int g_unit_test_1;
MainFrame * gFrame;
ChartCanvas *cc1;
ConsoleCanvas *console;
StatWin *stats;
wxWindowList AppActivateList;
Config *pConfig;
ChartBase *Current_Vector_Ch;
ChartBase *Current_Ch;
ChartDB *ChartData;
ChartStack *pCurrentStack;
wxString *pdir_list[20];
int g_restore_stackindex;
int g_restore_dbindex;
double g_ChartNotRenderScaleFactor;
RouteList *pRouteList;
LayerList *pLayerList;
bool g_bIsNewLayer;
int g_LayerIdx;
bool g_bLayerViz;
Select *pSelect;
Select *pSelectTC;
Select *pSelectAIS;
Routeman *g_pRouteMan;
WayPointman *pWayPointMan;
MarkInfoImpl *pMarkPropDialog;
RouteProp *pRoutePropDialog;
TrackPropDlg *pTrackPropDialog;
MarkInfoImpl *pMarkInfoDialog;
RouteManagerDialog *pRouteManagerDialog;
double gLat;
double gLon;
double vLat;
double vLon;
double initial_scale_ppm;
bool bDBUpdateInProgress;
ThumbWin *pthumbwin;
TCMgr *ptcmgr;
bool bDrawCurrentValues;
wxString g_SData_Locn;
wxString *pChartListFileName;
wxString *pWorldMapLocation;
wxString *pInit_Chart_Dir;
wxString g_csv_locn;
wxString g_SENCPrefix;
wxString g_UserPresLibData;
wxString g_Plugin_Dir;
wxString g_VisibleLayers;
wxString g_InvisibleLayers;
wxString g_uploadConnection;
int user_user_id;
int file_user_id;
int quitflag;
int g_tick;
bool s_bSetSystemTime;
wxArrayOfConnPrm *g_pConnectionParams;
wxDateTime g_start_time;
wxDateTime g_loglast_time;
OCPN_Sound bells_sound[8];
OCPN_Sound g_anchorwatch_sound;
RoutePoint *pAnchorWatchPoint1;
RoutePoint *pAnchorWatchPoint2;
double AnchorPointMinDist;
bool AnchorAlertOn1;
bool AnchorAlertOn2;
bool g_bCruising;
ChartDummy *pDummyChart;
ToolBarSimple* g_toolbar;
ocpnStyle::StyleManager * g_StyleManager;
wxPrintData *g_printData = (wxPrintData*) NULL ;
wxPageSetupData* g_pageSetupData = (wxPageSetupData*) NULL;
bool g_bDisplayGrid; // Flag indicating weather the lat/lon grid should be displayed
bool g_bShowActiveRouteHighway;
int g_nNMEADebug;
int g_nAWDefault;
int g_nAWMax;
bool g_bPlayShipsBells;
bool g_bShowLayers;
bool g_bTransparentToolbar;
bool g_bPermanentMOBIcon;
int g_iSDMMFormat;
int g_iDistanceFormat;
int g_iSpeedFormat;
int g_iNavAidRadarRingsNumberVisible;
float g_fNavAidRadarRingsStep;
int g_pNavAidRadarRingsStepUnits;
bool g_bWayPointPreventDragging;
bool g_bConfirmObjectDelete;
ColorScheme global_color_scheme;
bool bGPSValid;
bool g_bHDT_Rx;
bool g_bVAR_Rx;
int g_SatsInView;
bool g_bSatValid;
bool g_bDebugCM93;
bool g_bDebugS57;
bool g_bfilter_cogsog;
int g_COGFilterSec;
int g_SOGFilterSec;
int g_ChartUpdatePeriod;
int g_SkewCompUpdatePeriod;
wxRect g_last_tb_rect;

#ifdef USE_S57
	s52plib                   *ps52plib;
	S57ClassRegistrar         *g_poRegistrar;
	S57RegistrarMgr           *m_pRegistrarMan;
	extern S57QueryDialog     *g_pObjectQueryDialog;
	CM93OffsetDialog          *g_pCM93OffsetDialog;
#endif

// begin rms
#if defined( USE_S57) || defined ( __WXOSX__ )
#ifdef __WXMSW__
#ifdef USE_GLU_TESS
#ifdef USE_GLU_DLL
// end rms
extern bool               s_glu_dll_ready;
extern HINSTANCE          s_hGLU_DLL; // Handle to DLL
#endif
#endif
#endif
#endif

double g_ownship_predictor_minutes;
int g_current_arrow_scale;
Multiplexer *g_pMUX;
AIS_Decoder *g_pAIS;
bool g_bAIS_CPA_Alert;
bool g_bAIS_CPA_Alert_Audio;
AISTargetAlertDialog *g_pais_alert_dialog_active;
AISTargetQueryDialog *g_pais_query_dialog_active;
int g_S57_dialog_sx;
int g_S57_dialog_sy;
bool g_bAutoAnchorMark;
wxRect g_blink_rect;
double g_PlanSpeed;
wxDateTime g_StartTime;
int g_StartTimeTZ;
IDX_entry *gpIDX;
int gpIDXn;
long gStart_LMT_Offset;
FILE *s_fpdebug;
bool bAutoOpen;
bool bFirstAuto;
bool g_bUseGLL;
int g_nCacheLimit;
int g_memCacheLimit;
bool g_bGDAL_Debug;
double g_VPRotate; // Viewport rotation angle, used on "Course Up" mode
bool g_bCourseUp;
int g_COGAvgSec; // COG average period (sec.) for Course Up Mode
double g_COGAvg;
bool g_bskew_comp;
bool g_bopengl;
bool g_bsmoothpanzoom;
int g_nCOMPortCheck;
bool g_bbigred;
PlugInManager *g_pi_manager;
bool g_bAISRolloverShowClass;
bool g_bAISRolloverShowCOG;
bool g_bAISRolloverShowCPA;
bool g_bDebugGPSD;
bool g_bFullScreenQuilt;
bool g_bQuiltEnable;
bool g_bQuiltStart;
bool g_bportable;
bool g_bdisable_opengl;
ChartGroupArray *g_pGroupArray;
int g_GroupIndex;
wxString g_GPS_Ident;
wxProgressDialog *s_ProgDialog;
wxArrayString TideCurrentDataSet;
wxString g_TCData_Dir;

#ifndef __WXMSW__
struct sigaction sa_all;
struct sigaction sa_all_old;
#endif

bool g_boptionsactive;

#ifdef __WXMSW__
// System color control support

typedef DWORD (WINAPI *SetSysColors_t)(DWORD, DWORD *, DWORD *);
typedef DWORD (WINAPI *GetSysColor_t)(DWORD);

SetSysColors_t pSetSysColors;
GetSysColor_t pGetSysColor;

void SaveSystemColors(void);
void RestoreSystemColors(void);

DWORD color_3dface;
DWORD color_3dhilite;
DWORD color_3dshadow;
DWORD color_3ddkshadow;
DWORD color_3dlight;
DWORD color_activecaption;
DWORD color_gradientactivecaption;
DWORD color_captiontext;
DWORD color_windowframe;
DWORD color_inactiveborder;

#endif

bool g_bShowAIS;
bool g_bCPAMax;
double g_CPAMax_NM;
bool g_bCPAWarn;
double g_CPAWarn_NM;
bool g_bTCPA_Max;
double g_TCPA_Max;
bool g_bMarkLost;
double g_MarkLost_Mins;
bool g_bRemoveLost;
double g_RemoveLost_Mins;
bool g_bShowCOG;
double g_ShowCOG_Mins;
bool g_bAISShowTracks;
double g_AISShowTracks_Mins;
bool g_bShowMoored;
double g_ShowMoored_Kts;
wxString g_sAIS_Alert_Sound_File;
bool g_bAIS_CPA_Alert_Suppress_Moored;
bool g_bAIS_ACK_Timeout;
double g_AckTimeout_Mins;
bool g_bShowAreaNotices;
bool g_bDrawAISSize;
bool g_bShowAISName;
int g_Show_Target_Name_Scale;
bool g_bWplIsAprsPosition;
wxToolBarToolBase *m_pAISTool;
int g_nAIS_activity_timer;
bool g_bEnableZoomToCursor;
bool g_bTrackActive;
bool g_bTrackCarryOver;
bool g_bTrackDaily;
bool g_bHighliteTracks;
int g_route_line_width;
int g_track_line_width;
wxString g_default_wp_icon;
Track *g_pActiveTrack;
double g_TrackIntervalSeconds;
double g_TrackDeltaDistance;
int g_nTrackPrecision;
int g_total_NMEAerror_messages;
int g_cm93_zoom_factor;
CM93DSlide *pCM93DetailSlider;
bool g_bShowCM93DetailSlider;
int g_cm93detail_dialog_x;
int g_cm93detail_dialog_y;
bool g_bUseGreenShip;
wxString g_AW1GUID;
wxString g_AW2GUID;
bool g_b_overzoom_x; // Allow high overzoom
bool g_bshow_overzoom_emboss;
int g_OwnShipIconType;
double g_n_ownship_length_meters;
double g_n_ownship_beam_meters;
double g_n_gps_antenna_offset_y;
double g_n_gps_antenna_offset_x;
int g_n_ownship_min_mm;
double g_n_arrival_circle_radius;
int g_nautosave_interval_seconds;
bool g_bPreserveScaleOnX;
AboutDialog * g_pAboutDlg;
wxPlatformInfo *g_pPlatform;
wxLocale *plocale_def_lang;
wxString g_locale;
bool g_b_assume_azerty;
bool g_bUseRaster;
bool g_bUseVector;
bool g_bUseCM93;
int g_click_stop;
int g_MemFootSec;
int g_MemFootMB;
std::vector<int> g_quilt_noshow_index_array;
wxStaticBitmap *g_pStatBoxTool;
bool g_bquiting;
int g_BSBImgDebug;
AISTargetListDialog *g_pAISTargetList;
wxString g_AisTargetList_perspective;
int g_AisTargetList_range;
int g_AisTargetList_sortColumn;
bool g_bAisTargetList_sortReverse;
wxString g_AisTargetList_column_spec;
int g_AisTargetList_count;
bool g_bGarminHostUpload;
wxAuiManager *g_pauimgr;
wxAuiDefaultDockArt *g_pauidockart;
bool g_blocale_changed;
wxMenu *g_FloatingToolbarConfigMenu;
wxString g_toolbarConfig = _T("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
OCPNFloatingToolbarDialog *g_FloatingToolbarDialog;
FloatingCompassWindow * g_FloatingCompassDialog;
bool g_bMagneticAPB;
int g_GPU_MemSize;
bool g_b_useStencil;

bool g_bShowMag;
double g_UserVar;
int portaudio_initialized;
int g_sticky_chart;
double g_GLMinLineWidth;

char bells_sound_file_name[8][12] =    // pjotrc 2010.02.09
{
	"1bells.wav",
	"2bells.wav",
	"3bells.wav",
	"4bells.wav",
	"5bells.wav",
	"6bells.wav",
	"7bells.wav",
	"8bells.wav"
};

#ifndef __WXMSW__
sigjmp_buf env;                    // the context saved by sigsetjmp();
#endif

#ifdef __MSVC__
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

void appendOSDirSlash(wxString &);
void SetSystemColors( ColorScheme cs );

DEFINE_EVENT_TYPE(EVT_THREADMSG)

//------------------------------------------------------------------------------
//    PNG Icon resources
//------------------------------------------------------------------------------

#ifdef __WXGTK__
#include "bitmaps/opencpn.xpm"
#endif

enum {
    ID_PIANO_DISABLE_QUILT_CHART = 32000,
	ID_PIANO_ENABLE_QUILT_CHART
};

//------------------------------------------------------------------------------
//      Signal Handlers
//-----------------------------------------------------------------------
#ifndef __WXMSW__

//These are the signals possibly expected
//      SIGUSR1
//      Raised externally to cause orderly termination of application
//      Intended to act just like pushing the "EXIT" button

//      SIGSEGV
//      Some undefined segfault......

void catch_signals(int signo)
{
    switch(signo)
    {
        case SIGUSR1:
        quitflag++;                             // signal to the timer loop
        break;

        case SIGSEGV:
        siglongjmp(env, 1);// jump back to the setjmp() point
        break;

        default:
        break;
    }

}
#endif

#ifdef OCPN_USE_CRASHRPT

// Define the crash callback
int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo)
{
  //  Flush log file
    if( logger)
        logger->Flush();

    return CR_CB_DODEFAULT;
}

#endif


//------------------------------------------------------------------------------
// MainFrame
//------------------------------------------------------------------------------
//      Frame implementation
BEGIN_EVENT_TABLE(MainFrame, wxFrame) EVT_CLOSE(MainFrame::OnCloseWindow)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_SIZE(MainFrame::OnSize)
EVT_MOVE(MainFrame::OnMove)
EVT_MENU(-1, MainFrame::OnToolLeftClick)
EVT_TIMER(FRAME_TIMER_1, MainFrame::OnFrameTimer1)
EVT_TIMER(FRAME_TC_TIMER, MainFrame::OnFrameTCTimer)
EVT_TIMER(FRAME_COG_TIMER, MainFrame::OnFrameCOGTimer)
EVT_TIMER(MEMORY_FOOTPRINT_TIMER, MainFrame::OnMemFootTimer)
EVT_ACTIVATE(MainFrame::OnActivate)
EVT_MAXIMIZE(MainFrame::OnMaximize)
EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_TOOL_RCLICKED, MainFrame::RequestNewToolbarArgEvent)
EVT_ERASE_BACKGROUND(MainFrame::OnEraseBackground)
END_EVENT_TABLE()

// My frame constructor
MainFrame::MainFrame( wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size,
        long style ) :
        wxFrame( frame, -1, title, pos, size, style ) //wxSIMPLE_BORDER | wxCLIP_CHILDREN | wxRESIZE_BORDER)
//wxCAPTION | wxSYSTEM_MENU | wxRESIZE_BORDER
{
    m_ulLastNEMATicktime = 0;
    m_pStatusBar = NULL;

    g_toolbar = NULL;
    m_toolbar_scale_tools_shown = false;

    //      Redirect the global heartbeat timer to this frame
    FrameTimer1.SetOwner( this, FRAME_TIMER_1 );

    //      Redirect the Tide/Current update timer to this frame
    FrameTCTimer.SetOwner( this, FRAME_TC_TIMER );

    //      Redirect the COG Averager timer to this frame
    FrameCOGTimer.SetOwner( this, FRAME_COG_TIMER );

    //      Redirect the Memory Footprint Management timer to this frame
    MemFootTimer.SetOwner( this, MEMORY_FOOTPRINT_TIMER );

    //      Set up some assorted member variables
    nRoute_State = 0;
    m_bTimeIsSet = false;
    m_bdefer_resize = false;

    //    Clear the NMEA Filter tables
    for( int i = 0; i < MAX_COGSOG_FILTER_SECONDS; i++ ) {
        COGFilterTable[i] = 0.;
        SOGFilterTable[i] = 0.;
    }
    m_COGFilterLast = 0.;
    m_last_bGPSValid = false;

	global::Navigation & nav = global::OCPN::get().nav();

    nav.set_heading_true(NAN);
    nav.set_heading_magn(NAN);
    nav.set_magn_var(NAN);
	nav.set_speed_over_ground(NAN);
    nav.set_course_over_ground(NAN);
    m_fixtime = 0;

    m_bpersistent_quilt = false;

    m_ChartUpdatePeriod = 1;                  // set the default (1 sec.) period

//    Establish my children
    g_pMUX = new Multiplexer();

    g_pAIS = new AIS_Decoder( this );


    for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
    {
        ConnectionParams *cp = g_pConnectionParams->Item(i);
        if( cp->bEnabled ) {
            dsPortType port_type;
            if (cp->Output)
                port_type = DS_TYPE_INPUT_OUTPUT;
            else
                port_type = DS_TYPE_INPUT;
            DataStream *dstr = new DataStream( g_pMUX,
                                           cp->GetDSPort(),
                                           wxString::Format(wxT("%i"),cp->Baudrate),
                                           port_type,
                                           cp->Priority,
                                           cp->Garmin
                                         );
            dstr->SetInputFilter(cp->InputSentenceList);
            dstr->SetInputFilterType(cp->InputSentenceListType);
            dstr->SetOutputFilter(cp->OutputSentenceList);
            dstr->SetOutputFilterType(cp->OutputSentenceListType);
            dstr->SetChecksumCheck(cp->ChecksumCheck);
            g_pMUX->AddStream(dstr);
        }
    }

    g_pMUX->SetAISHandler(g_pAIS);
    g_pMUX->SetGPSHandler(this);
    //  Create/connect a dynamic event handler slot
    Connect( wxEVT_OCPN_DATASTREAM, (wxObjectEventFunction) (wxEventFunction) &MainFrame::OnEvtOCPN_NMEA );

    bFirstAuto = true;

    //  Create/connect a dynamic event handler slot for OCPN_MsgEvent(s) coming from PlugIn system
    Connect( wxEVT_OCPN_MSG, (wxObjectEventFunction) (wxEventFunction) &MainFrame::OnEvtPlugInMessage );

    Connect( EVT_THREADMSG, (wxObjectEventFunction) (wxEventFunction) &MainFrame::OnEvtTHREADMSG );

    //        Establish the system icons for the frame.

#ifdef __WXMSW__
    SetIcon( wxICON(0) );           // this grabs the first icon in the integrated MSW resource file
#endif

#ifdef __WXGTK__
    wxIcon app_icon(opencpn);          // This comes from opencpn.xpm inclusion above
    SetIcon(app_icon);
#endif

#ifdef __WXMSW__

//    Establish the entry points in USER32.DLL for system color control

    wxDynamicLibrary dllUser32( _T("user32.dll") );

    pSetSysColors = (SetSysColors_t) dllUser32.GetSymbol( wxT("SetSysColors") );
    pGetSysColor = (GetSysColor_t) dllUser32.GetSymbol( wxT("GetSysColor") );

    SaveSystemColors();
#endif

    g_FloatingToolbarConfigMenu = new wxMenu();

    m_next_available_plugin_tool_id = ID_PLUGIN_BASE;

    m_COGFilterLast = 0.;

    g_sticky_chart = -1;
}

MainFrame::~MainFrame()
{
	FrameTimer1.Stop();
	delete ChartData;
	delete pCurrentStack;

	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		delete *i;
	}
	delete pRouteList;
	delete g_FloatingToolbarConfigMenu;
}

void MainFrame::performUniChromeOpenGLResizeHack()
{
	//  This little hack fixes a problem seen with some UniChrome OpenGL drivers
	//  We need a deferred resize to get glDrawPixels() to work right.
	//  So we set a trigger to generate a resize after 5 seconds....
	//  See the "UniChrome" hack elsewhere
	if ( !g_bdisable_opengl )
	{
		glChartCanvas *pgl = (glChartCanvas *) cc1->GetglCanvas();
		if( pgl && ( pgl->GetRendererString().Find( _T("UniChrome") ) != wxNOT_FOUND ) )
		{
			m_defer_size = gFrame->GetSize();
			SetSize(m_defer_size.x - 10, m_defer_size.y);
			g_pauimgr->Update();
			m_bdefer_resize = true;
		}
	}
}

ChartCanvas * MainFrame::GetCanvasWindow()
{
	return m_pchart_canvas;
}

void  MainFrame::SetCanvasWindow(ChartCanvas * pcanv)
{
	m_pchart_canvas = pcanv;
}

int MainFrame::GetNextToolbarToolId()
{
	return m_next_available_plugin_tool_id;
}

void MainFrame::RequestNewToolbarArgEvent(wxCommandEvent &)
{
	return RequestNewToolbar();
}

void MainFrame::OnEraseBackground(wxEraseEvent &)
{
}

void MainFrame::OnMaximize(wxMaximizeEvent &)
{
    g_click_stop = 0;
}

bool MainFrame::hasStatusBar() const
{
	return m_pStatusBar != NULL;
}

void MainFrame::OnActivate( wxActivateEvent& event )
{
//    Code carefully in this method.
//    It is called in some unexpected places,
//    such as on closure of dialogs, etc.

    if( cc1 ) cc1->SetFocus();       // This seems to be needed for MSW, to get key and wheel events
                                     // after minimize/maximize.

#ifdef __WXOSX__
    if(event.GetActive())
    {
        SurfaceToolbar();

        wxWindowListNode *node = AppActivateList.GetFirst();
        while (node) {
            wxWindow *win = node->GetData();
            win->Show();

            node = node->GetNext();
        }

        gFrame->Raise();
    }
#endif

    event.Skip();
}

ColorScheme MainFrame::GetColorScheme()
{
    return global_color_scheme;
}

void MainFrame::SetAndApplyColorScheme( ColorScheme cs )
{
    global_color_scheme = cs;

    wxString SchemeName;
    switch( cs ){
        case GLOBAL_COLOR_SCHEME_DAY:
            SchemeName = _T("DAY");
            break;
        case GLOBAL_COLOR_SCHEME_DUSK:
            SchemeName = _T("DUSK");
            break;
        case GLOBAL_COLOR_SCHEME_NIGHT:
            SchemeName = _T("NIGHT");
            break;
        default:
            SchemeName = _T("DAY");
            break;
    }

    g_StyleManager->GetCurrentStyle()->SetColorScheme(cs);
    cc1->GetWorldBackgroundChart()->SetColorScheme(cs);

#ifdef USE_S57
    if (ps52plib)
		ps52plib->SetPLIBColorScheme(SchemeName);
#endif

	setup_current_user_color(SchemeName);
    SetSystemColors(cs);

    if (cc1)
		cc1->SetColorScheme(cs);

    if (pWayPointMan)
		pWayPointMan->SetColorScheme(cs);

    if (ChartData)
		ChartData->ApplyColorSchemeToCachedCharts(cs);

    if( stats ) stats->SetColorScheme( cs );

    if( console ) console->SetColorScheme( cs );

    if( g_pRouteMan ) g_pRouteMan->SetColorScheme( cs );

    if( pMarkPropDialog ) pMarkPropDialog->SetColorScheme( cs );

    if( pMarkInfoDialog ) pMarkInfoDialog->SetColorScheme( cs );

    //    For the AIS target query dialog, we must rebuild it to incorporate the style desired for the colorscheme selected
    if( g_pais_query_dialog_active ) {
        bool b_isshown = g_pais_query_dialog_active->IsShown();
        int n_mmsi = g_pais_query_dialog_active->GetMMSI();
        if( b_isshown ) g_pais_query_dialog_active->Show( false );              // dismiss it

        g_pais_query_dialog_active->Close();

        g_pais_query_dialog_active = new AISTargetQueryDialog();
        g_pais_query_dialog_active->Create(
			this,
			-1,
			_( "AIS Target Query" ),
			global::OCPN::get().gui().ais_query_dialog().position);
        g_pais_query_dialog_active->SetMMSI( n_mmsi );
        g_pais_query_dialog_active->UpdateText();
        if( b_isshown ) g_pais_query_dialog_active->Show();
    }

    if( pRouteManagerDialog ) pRouteManagerDialog->SetColorScheme();

    if( g_pAISTargetList ) g_pAISTargetList->SetColorScheme();

    if( g_pObjectQueryDialog ) g_pObjectQueryDialog->SetColorScheme();

    ApplyGlobalColorSchemetoStatusBar();

    UpdateToolbar( cs );

    if( g_pi_manager ) g_pi_manager->SetColorSchemeForAllPlugIns( cs );
}

void MainFrame::ApplyGlobalColorSchemetoStatusBar( void )
{
    if( m_pStatusBar != NULL ) {
        m_pStatusBar->SetBackgroundColour(GetGlobalColor(_T("UIBDR")));    //UINFF
        m_pStatusBar->ClearBackground();

        int styles[] = { wxSB_FLAT, wxSB_FLAT, wxSB_FLAT, wxSB_FLAT, wxSB_FLAT, wxSB_FLAT };
        m_pStatusBar->SetStatusStyles( m_StatusBarFieldCount, styles );
        int widths[] = { -6, -5, -5, -3, -4 };
        m_pStatusBar->SetStatusWidths( m_StatusBarFieldCount, widths );
    }
}

void MainFrame::DestroyMyToolbar()
{
    if( g_FloatingToolbarDialog ) {
        g_FloatingToolbarDialog->DestroyToolBar();
        g_toolbar = NULL;
    }
}

bool _toolbarConfigMenuUtil( int toolid, wxString tipString )
{
    wxMenuItem* menuitem;

    if( toolid == ID_MOB && g_bPermanentMOBIcon ) return true;

    // Item ID trickery is needed because the wxCommandEvents for menu item clicked and toolbar button
    // clicked are 100% identical, so if we use same id's we can't tell the events apart.

    int idOffset = ID_PLUGIN_BASE - ID_ZOOMIN + 100;  // Hopefully no more than 100 plugins loaded...
    int menuItemId = toolid + idOffset;

    menuitem = g_FloatingToolbarConfigMenu->FindItem( menuItemId );

    if( menuitem ) {
        return menuitem->IsChecked();
    }

    menuitem = g_FloatingToolbarConfigMenu->AppendCheckItem( menuItemId, tipString );
    menuitem->Check( g_toolbarConfig.GetChar( toolid - ID_ZOOMIN ) == _T('X') );
    return menuitem->IsChecked();
}

ToolBarSimple *MainFrame::CreateAToolbar()
{
    ToolBarSimple *tb = NULL;
    wxToolBarToolBase* newtool;

    if( g_FloatingToolbarDialog ) tb = g_FloatingToolbarDialog->GetToolbar();
    if( !tb ) return 0;

    ocpnStyle::Style * style = g_StyleManager->GetCurrentStyle();

    wxString tipString;

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Zoom In") ) << _T(" (+)");
    if( _toolbarConfigMenuUtil( ID_ZOOMIN, tipString ) )
        tb->AddTool( ID_ZOOMIN, _T("zoomin"),
            style->GetToolIcon( _T("zoomin"), ocpnStyle::TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Zoom Out") ) << _T(" (-)");
    if( _toolbarConfigMenuUtil( ID_ZOOMOUT, tipString ) )
        tb->AddTool( ID_ZOOMOUT, _T("zoomout"),
            style->GetToolIcon( _T("zoomout"), ocpnStyle::TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    m_toolbar_scale_tools_shown = pCurrentStack && pCurrentStack->b_valid
            && ( pCurrentStack->nEntry > 1 );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Shift to Larger Scale Chart") ) << _T(" (F7)");
    if( _toolbarConfigMenuUtil( ID_STKDN, tipString ) ) {
        newtool = tb->AddTool( ID_STKDN, _T("scin"),
                style->GetToolIcon( _T("scin"), ocpnStyle::TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );
        newtool->Enable( m_toolbar_scale_tools_shown );
    }

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Shift to Smaller Scale Chart") ) << _T(" (F8)");
    if( _toolbarConfigMenuUtil( ID_STKUP, tipString ) ) {
        newtool = tb->AddTool( ID_STKUP, _T("scout"),
                style->GetToolIcon( _T("scout"), ocpnStyle::TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );
        newtool->Enable( m_toolbar_scale_tools_shown );
    }

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Create Route") ) << _T(" (Ctrl-R)");
    if( _toolbarConfigMenuUtil( ID_ROUTE, tipString ) )
        tb->AddTool( ID_ROUTE, _T("route"),
            style->GetToolIcon( _T("route"), ocpnStyle::TOOLICON_NORMAL ),
            style->GetToolIcon( _T("route"), ocpnStyle::TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Auto Follow") ) << _T(" (F2)");
    if( _toolbarConfigMenuUtil( ID_FOLLOW, tipString ) )
        tb->AddTool( ID_FOLLOW, _T("follow"),
            style->GetToolIcon( _T("follow"), ocpnStyle::TOOLICON_NORMAL ),
            style->GetToolIcon( _T("follow"), ocpnStyle::TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    CheckAndAddPlugInTool( tb );
    tipString = _("Options");
    if( _toolbarConfigMenuUtil( ID_SETTINGS, tipString ) )
        tb->AddTool( ID_SETTINGS, _T("settings"),
            style->GetToolIcon( _T("settings"), ocpnStyle::TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Show ENC Text") ) << _T(" (T)");
    if( _toolbarConfigMenuUtil( ID_TEXT, tipString ) )
        tb->AddTool( ID_TEXT, _T("text"),
            style->GetToolIcon( _T("text"), ocpnStyle::TOOLICON_NORMAL ),
            style->GetToolIcon( _T("text"), ocpnStyle::TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    m_pAISTool = NULL;
    CheckAndAddPlugInTool( tb );
    tipString = _("Hide AIS Targets");          // inital state is on
    if( _toolbarConfigMenuUtil( ID_AIS, tipString ) )
        m_pAISTool = tb->AddTool( ID_AIS, _T("AIS"), style->GetToolIcon( _T("AIS"), ocpnStyle::TOOLICON_NORMAL ),
                                  style->GetToolIcon( _T("AIS"), ocpnStyle::TOOLICON_DISABLED ),
                                  wxITEM_NORMAL, tipString );

    CheckAndAddPlugInTool( tb );
    tipString = _("Show Currents");
    if( _toolbarConfigMenuUtil( ID_CURRENT, tipString ) )
        tb->AddTool( ID_CURRENT, _T("current"),
            style->GetToolIcon( _T("current"), ocpnStyle::TOOLICON_NORMAL ), tipString, wxITEM_CHECK );

    CheckAndAddPlugInTool( tb );
    tipString = _("Show Tides");
    if( _toolbarConfigMenuUtil( ID_TIDE, tipString ) )
        tb->AddTool( ID_TIDE, _T("tide"),
            style->GetToolIcon( _T("tide"), ocpnStyle::TOOLICON_NORMAL ), tipString, wxITEM_CHECK );

    CheckAndAddPlugInTool( tb );
    tipString = _("Print Chart");
    if( _toolbarConfigMenuUtil( ID_PRINT, tipString ) )
        tb->AddTool( ID_PRINT, _T("print"),
            style->GetToolIcon( _T("print"), ocpnStyle::TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = _("Route Manager");
    if( _toolbarConfigMenuUtil( ID_ROUTEMANAGER, tipString ) )
        tb->AddTool( ID_ROUTEMANAGER,
            _T("route_manager"), style->GetToolIcon( _T("route_manager"), ocpnStyle::TOOLICON_NORMAL ),
            tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = _("Toggle Tracking");
    if( _toolbarConfigMenuUtil( ID_TRACK, tipString ) )
        tb->AddTool( ID_TRACK, _T("track"),
            style->GetToolIcon( _T("track"), ocpnStyle::TOOLICON_NORMAL ),
            style->GetToolIcon( _T("track"), ocpnStyle::TOOLICON_TOGGLED ), wxITEM_CHECK, tipString );

    CheckAndAddPlugInTool( tb );
    tipString = wxString( _("Change Color Scheme") ) << _T(" (F5)");
    if( _toolbarConfigMenuUtil( ID_COLSCHEME, tipString ) )
        tb->AddTool( ID_COLSCHEME,
            _T("colorscheme"), style->GetToolIcon( _T("colorscheme"), ocpnStyle::TOOLICON_NORMAL ),
            tipString, wxITEM_NORMAL );

    CheckAndAddPlugInTool( tb );
    tipString = _("About OpenCPN");
    if( _toolbarConfigMenuUtil( ID_HELP, tipString ) )
        tb->AddTool( ID_HELP, _T("help"),
            style->GetToolIcon( _T("help"), ocpnStyle::TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

    //      Add any PlugIn toolbar tools that request default positioning
    AddDefaultPositionPlugInTools( tb );

    //  And finally add the MOB tool
    tipString = wxString( _("Drop MOB Marker") ) << _(" (Ctrl-Space)");
    if( _toolbarConfigMenuUtil( ID_MOB, tipString ) )
        tb->AddTool( ID_MOB, _T("mob_btn"),
                     style->GetToolIcon( _T("mob_btn"), ocpnStyle::TOOLICON_NORMAL ), tipString, wxITEM_NORMAL );

// Realize() the toolbar
    g_FloatingToolbarDialog->Realize();

//      Set up the toggle states

    if( cc1 ) {
        //  Re-establish toggle states
        tb->ToggleTool( ID_CURRENT, cc1->GetbShowCurrent() );
        tb->ToggleTool( ID_TIDE, cc1->GetbShowTide() );
    }

    if (pConfig)
		tb->ToggleTool( ID_FOLLOW, cc1->m_bFollow);

#ifdef USE_S57
    if (pConfig && ps52plib)
		if (ps52plib->m_bOK)
			tb->ToggleTool(ID_TEXT, ps52plib->GetShowS57Text());
#endif

    wxString initiconName;
    if (g_bShowAIS) {
        tb->SetToolShortHelp( ID_AIS, _("Hide AIS Targets") );
        initiconName = _T("AIS");
    } else {
        tb->SetToolShortHelp( ID_AIS, _("Show AIS Targets") );
        initiconName = _T("AIS_Disabled");
    }
    tb->SetToolNormalBitmapEx( m_pAISTool, initiconName );
    m_lastAISiconName = initiconName;

    tb->ToggleTool( ID_TRACK, g_bTrackActive );

    SetStatusBarPane(-1); // don't show help on status bar

    return tb;
}

bool MainFrame::CheckAndAddPlugInTool( ToolBarSimple *tb )
{
    if( !g_pi_manager ) return false;

    bool bret = false;
    int n_tools = tb->GetToolsCount();

    //    Walk the PlugIn tool spec array, checking the requested position
    //    If a tool has been requested by a plugin at this position, add it
    ArrayOfPlugInToolbarTools tool_array = g_pi_manager->GetPluginToolbarToolArray();

    for( unsigned int i = 0; i < tool_array.GetCount(); i++ ) {
        PlugInToolbarToolContainer *pttc = tool_array.Item( i );
        if( pttc->position == n_tools ) {
            wxBitmap *ptool_bmp;

            switch( global_color_scheme ){
                case GLOBAL_COLOR_SCHEME_DAY:
                    ptool_bmp = pttc->bitmap_day;
                    ;
                    break;
                case GLOBAL_COLOR_SCHEME_DUSK:
                    ptool_bmp = pttc->bitmap_dusk;
                    break;
                case GLOBAL_COLOR_SCHEME_NIGHT:
                    ptool_bmp = pttc->bitmap_night;
                    break;
                default:
                    ptool_bmp = pttc->bitmap_day;
                    ;
                    break;
            }

            tb->AddTool( pttc->id, wxString( pttc->label ), *( ptool_bmp ),
                    wxString( pttc->shortHelp ), pttc->kind );
            if( pttc->kind == wxITEM_CHECK ) tb->ToggleTool( pttc->id, pttc->b_toggle );
            bret = true;
        }
    }

    //    If we added a tool, call again (recursively) to allow for adding adjacent tools
    if( bret ) while( CheckAndAddPlugInTool( tb ) ) { /* nothing to do */
    }

    return bret;
}

bool MainFrame::AddDefaultPositionPlugInTools( ToolBarSimple *tb )
{
    if( !g_pi_manager ) return false;

    bool bret = false;

    //    Walk the PlugIn tool spec array, checking the requested position
    //    If a tool has been requested by a plugin at this position, add it
    ArrayOfPlugInToolbarTools tool_array = g_pi_manager->GetPluginToolbarToolArray();

    for( unsigned int i = 0; i < tool_array.GetCount(); i++ ) {
        PlugInToolbarToolContainer *pttc = tool_array.Item( i );
        if( pttc->position == -1 )                  // PlugIn has requested default positioning
                {
            wxBitmap *ptool_bmp;

            switch( global_color_scheme ){
                case GLOBAL_COLOR_SCHEME_DAY:
                    ptool_bmp = pttc->bitmap_day;
                    ;
                    break;
                case GLOBAL_COLOR_SCHEME_DUSK:
                    ptool_bmp = pttc->bitmap_dusk;
                    break;
                case GLOBAL_COLOR_SCHEME_NIGHT:
                    ptool_bmp = pttc->bitmap_night;
                    break;
                default:
                    ptool_bmp = pttc->bitmap_day;
                    ;
                    break;
            }

            tb->AddTool( pttc->id, wxString( pttc->label ), *( ptool_bmp ),
                    wxString( pttc->shortHelp ), pttc->kind );
            if( pttc->kind == wxITEM_CHECK ) tb->ToggleTool( pttc->id, pttc->b_toggle );
            bret = true;
        }
    }
    return bret;
}

void MainFrame::RequestNewToolbar()
{
    if( g_FloatingToolbarDialog ) {
        bool b_reshow = g_FloatingToolbarDialog->IsShown();
        if( g_FloatingToolbarDialog->IsToolbarShown() ) DestroyMyToolbar();

        g_toolbar = CreateAToolbar();
        g_FloatingToolbarDialog->RePosition();
        g_FloatingToolbarDialog->Show( b_reshow );
    }
}

//      Update inplace the current toolbar with bitmaps corresponding to the current color scheme
void MainFrame::UpdateToolbar( ColorScheme cs )
{
    if( g_FloatingToolbarDialog ) {
        g_FloatingToolbarDialog->SetColorScheme( cs );

        if( g_FloatingToolbarDialog->IsToolbarShown() ) {
            DestroyMyToolbar();
            g_toolbar = CreateAToolbar();
        }
    }

    if( g_FloatingCompassDialog ) g_FloatingCompassDialog->SetColorScheme( cs );

    if( g_toolbar ) {
        //  Re-establish toggle states
        g_toolbar->ToggleTool( ID_FOLLOW, cc1->m_bFollow );
        g_toolbar->ToggleTool( ID_CURRENT, cc1->GetbShowCurrent() );
        g_toolbar->ToggleTool( ID_TIDE, cc1->GetbShowTide() );
    }

    return;
}

void MainFrame::EnableToolbar( bool newstate )
{
    if( g_toolbar ) {
        g_toolbar->EnableTool( ID_ZOOMIN, newstate );
        g_toolbar->EnableTool( ID_ZOOMOUT, newstate );
        g_toolbar->EnableTool( ID_STKUP, newstate );
        g_toolbar->EnableTool( ID_STKDN, newstate );
        g_toolbar->EnableTool( ID_ROUTE, newstate );
        g_toolbar->EnableTool( ID_FOLLOW, newstate );
        g_toolbar->EnableTool( ID_SETTINGS, newstate );
        g_toolbar->EnableTool( ID_TEXT, newstate );
        g_toolbar->EnableTool( ID_CURRENT, newstate );
        g_toolbar->EnableTool( ID_TIDE, newstate );
        g_toolbar->EnableTool( ID_HELP, newstate );
        g_toolbar->EnableTool( ID_TBEXIT, newstate );
        g_toolbar->EnableTool( ID_TBSTAT, newstate );
        g_toolbar->EnableTool( ID_PRINT, newstate );
        g_toolbar->EnableTool( ID_COLSCHEME, newstate );
        g_toolbar->EnableTool( ID_ROUTEMANAGER, newstate );
        g_toolbar->EnableTool( ID_TRACK, newstate );
        g_toolbar->EnableTool( ID_AIS, newstate );
    }
}

// Intercept menu commands
void MainFrame::OnExit(wxCommandEvent &)
{
    quitflag++;                             // signal to the timer loop

}

static bool b_inCloseWindow;

void MainFrame::OnCloseWindow(wxCloseEvent &)
{
    //    It is possible that double clicks on application exit box could cause re-entrance here
    //    Not good, and don't need it anyway, so simply return.
    if( b_inCloseWindow ) {
//            wxLogMessage(_T("opencpn::MainFrame re-entering OnCloseWindow"));
        return;
    }

    b_inCloseWindow = true;

    ::wxSetCursor( wxCURSOR_WAIT );

    // If we happen to have the measure tool open on Ctrl-Q quit
    cc1->CancelMeasureRoute();

    // We save perspective before closing to restore position next time
    // Pane is not closed so the child is not notified (OnPaneClose)
    if( g_pAISTargetList ) {
        wxAuiPaneInfo &pane = g_pauimgr->GetPane( g_pAISTargetList );
        g_AisTargetList_perspective = g_pauimgr->SavePaneInfo( pane );
        g_pauimgr->DetachPane( g_pAISTargetList );
    }

    pConfig->SetPath( _T ( "/AUI" ) );
    pConfig->Write( _T ( "AUIPerspective" ), g_pauimgr->SavePerspective() );

    g_bquiting = true;
    if( cc1 ) {
        cc1->SetCursor( wxCURSOR_WAIT );

        cc1->Refresh( false );
        cc1->Update();
    }

    //   Save the saved Screen Brightness
    RestoreScreenBrightness();

    //    Deactivate the PlugIns
    if( g_pi_manager ) {
        g_pi_manager->DeactivateAllPlugIns();
    }

    wxLogMessage( _T("opencpn::MainFrame exiting cleanly.") );

    quitflag++;

    FrameTimer1.Stop();

    g_pMUX->ClearStreams();

    /*
     Automatically drop an anchorage waypoint, if enabled
     On following conditions:
     1.  In "Cruising" mode, meaning that speed has at some point exceeded 3.0 kts.
     2.  Current speed is less than 0.5 kts.
     3.  Opencpn has been up at least 30 minutes
     4.  And, of course, opencpn is going down now.
     5.  And if there is no anchor watch set on "anchor..." icon mark           // pjotrc 2010.02.15
     */
    if( g_bAutoAnchorMark ) {
        bool watching_anchor = false;                                           // pjotrc 2010.02.15
        if( pAnchorWatchPoint1 )                                               // pjotrc 2010.02.15
        watching_anchor = ( pAnchorWatchPoint1->m_IconName.StartsWith( _T("anchor") ) ); // pjotrc 2010.02.15
        if( pAnchorWatchPoint2 )                                               // pjotrc 2010.02.15
        watching_anchor |= ( pAnchorWatchPoint2->m_IconName.StartsWith( _T("anchor") ) ); // pjotrc 2010.02.15

        wxDateTime now = wxDateTime::Now();
        wxTimeSpan uptime = now.Subtract( g_start_time );

		const global::Navigation::Data & nav = global::OCPN::get().nav().get_data();

        if (!watching_anchor && ( g_bCruising ) && (nav.sog < 0.5)
                && uptime.IsLongerThan(wxTimeSpan(0, 30, 0, 0)))     // pjotrc 2010.02.15
                {
            //    First, delete any single anchorage waypoint closer than 0.25 NM from this point
            //    This will prevent clutter and database congestion....

			for (RoutePointList::iterator i = pWayPointMan->m_pWayPointList->begin(); i != pWayPointMan->m_pWayPointList->end(); ++i) {
                RoutePoint * pr = *i;
                if (pr->GetName().StartsWith(_T("Anchorage"))) {
                    double a = gLat - pr->m_lat;
                    double b = gLon - pr->m_lon;
                    double l = sqrt((a * a) + (b * b));

                    // caveat: this is accurate only on the Equator
                    if ((l * 60.0 * 1852.0) < (0.25 * 1852.0)) {
                        pConfig->DeleteWayPoint(pr);
                        pSelect->DeleteSelectablePoint(pr, Select::TYPE_ROUTEPOINT);
                        delete pr;
                        break;
                    }
                }
            }

            wxString name = now.Format();
            name.Prepend( _("Anchorage created ") );
            RoutePoint *pWP = new RoutePoint( gLat, gLon, _T("anchorage"), name);
            pWP->m_bShowName = false;
            pWP->m_bIsolatedMark = true;

            pConfig->AddNewWayPoint( pWP, -1 );       // use auto next num
        }
    }

    FrameTimer1.Stop();

	global::OCPN::get().gui().set_frame_maximized(IsMaximized());

    //    Record the current state of tracking
    g_bTrackCarryOver = g_bTrackActive;

    TrackOff();

    if( pCurrentStack ) {
        g_restore_stackindex = pCurrentStack->CurrentStackEntry;
        g_restore_dbindex = pCurrentStack->GetCurrentEntrydbIndex();
    }

    if (g_FloatingToolbarDialog) {
        wxPoint tbp = g_FloatingToolbarDialog->GetPosition();
		global::OCPN::get().gui().set_toolbar_position(cc1->ScreenToClient(tbp));
		global::OCPN::get().gui().set_toolbar_orientation(g_FloatingToolbarDialog->GetOrient());
    }

    pConfig->UpdateSettings();
    pConfig->UpdateNavObj();

//    pConfig->m_pNavObjectChangesSet->Clear();
    delete pConfig->m_pNavObjectChangesSet;

    //Remove any leftover Routes and Waypoints from config file as they were saved to navobj before
    pConfig->DeleteGroup( _T ( "/Routes" ) );
    pConfig->DeleteGroup( _T ( "/Marks" ) );
    pConfig->Flush();

    delete g_printData;
    delete g_pageSetupData;

    if( g_pAboutDlg ) g_pAboutDlg->Destroy();

//      Explicitely Close some children, especially the ones with event handlers
//      or that call GUI methods

#ifdef USE_S57
    if( g_pCM93OffsetDialog ) g_pCM93OffsetDialog->Destroy();
#endif

    g_FloatingToolbarDialog->Destroy();

    if( g_pAISTargetList ) {
        g_pAISTargetList->Disconnect_decoder();
        g_pAISTargetList->Destroy();
    }

    g_FloatingCompassDialog->Destroy();
    g_FloatingCompassDialog = NULL;

    //      Delete all open charts in the cache
    if( ChartData ) ChartData->PurgeCache();

    SetStatusBar( NULL );
    stats = NULL;

    cc1->Destroy();
    cc1 = NULL;


    //    Unload the PlugIns
    //      Note that we are waiting until after the canvas is destroyed,
    //      since some PlugIns may have created children of canvas.
    //      Such a PlugIn must stay intact for the canvas dtor to call DestoryChildren()
    if( g_pi_manager ) {
        g_pi_manager->UnLoadAllPlugIns();
        delete g_pi_manager;
        g_pi_manager = NULL;
    }

    if( g_pAIS ) {
        if(g_pMUX)
            g_pMUX->SetAISHandler(NULL);
        delete g_pAIS;
        g_pAIS = NULL;
    }

    delete g_pMUX;

    pthumbwin = NULL;

    g_FloatingToolbarDialog = NULL;

    g_pauimgr->UnInit();


    this->Destroy();

}

void MainFrame::OnMove(wxMoveEvent &)
{
    if (g_FloatingToolbarDialog)
		g_FloatingToolbarDialog->RePosition();

    if (stats)
		stats->RePosition();

    UpdateGPSCompassStatusBox( true );

    if (console && console->IsShown())
		PositionConsole();

	global::OCPN::get().gui().set_frame_position(GetPosition());
}

void MainFrame::ProcessCanvasResize(void)
{
    if (stats) {
        stats->ReSize();
        stats->RePosition();
    }

    if (g_FloatingToolbarDialog) {
        g_FloatingToolbarDialog->RePosition();
        g_FloatingToolbarDialog->SetGeometry();
        g_FloatingToolbarDialog->Realize();
        g_FloatingToolbarDialog->RePosition();
    }

    UpdateGPSCompassStatusBox( true );

    if (console->IsShown())
		PositionConsole();
}

void MainFrame::OnSize(wxSizeEvent &)
{
    ODoSetSize();
}

void MainFrame::ODoSetSize( void )
{
    int x, y;
    GetClientSize( &x, &y );

//      Resize the children

    if( m_pStatusBar ) {
        //  Maybe resize the font
        wxRect stat_box;
        m_pStatusBar->GetFieldRect( 0, stat_box );
        int font_size = stat_box.width / 28;                // 30 for linux

#ifdef __WXMAC__
        font_size = wxMax(10, font_size);             // beats me...
#endif

        wxFont* templateFont = FontMgr::Get().GetFont( _("StatusBar"), 12 );
        font_size += templateFont->GetPointSize() - 10;

        font_size = wxMin( font_size, 12 );
        font_size = wxMax( font_size, 5 );

        wxFont *pstat_font = wxTheFontList->FindOrCreateFont( font_size,
              wxFONTFAMILY_SWISS, templateFont->GetStyle(), templateFont->GetWeight(), false,
              templateFont->GetFaceName() );

        m_pStatusBar->SetFont( *pstat_font );
    }

    int cccw = x;
    int ccch = y;

    if( cc1 ) {
        cccw = x * 10 / 10;               // constrain to mod 4
        int wr = cccw / 4;
        cccw = wr * 4;
        cccw += 2;                              // account for simple border

        int cur_width, cur_height;
        cc1->GetSize( &cur_width, &cur_height );
        if( ( cur_width != cccw ) || ( cur_height != ccch ) ) {
            if( g_pauimgr->GetPane( cc1 ).IsOk() )
                g_pauimgr->GetPane( cc1 ).BestSize( cccw, ccch );
            else
                cc1->SetSize( 0, 0, cccw, ccch );
        }
    }

    if( g_FloatingToolbarDialog ) {
        wxSize oldSize = g_FloatingToolbarDialog->GetSize();
        g_FloatingToolbarDialog->RePosition();
        g_FloatingToolbarDialog->SetGeometry();
        g_FloatingToolbarDialog->Realize();

        if( oldSize != g_FloatingToolbarDialog->GetSize() )
            g_FloatingToolbarDialog->Refresh( false );

        g_FloatingToolbarDialog->RePosition();

    }

    UpdateGPSCompassStatusBox( true );

    if( console ) PositionConsole();

    if( stats ) {
        stats->ReSize();
        stats->FormatStat();
        stats->RePosition();
    }

//  Update the stored window size
    GetSize(&x, &y);
	global::OCPN::get().gui().set_frame_size(wxSize(x, y));

//  Inform the PlugIns
    if (g_pi_manager)
		g_pi_manager->SendResizeEventToAllPlugIns(x, y);

//  Force redraw if in lookahead mode
    if (global::OCPN::get().gui().view().lookahead_mode) {
        if (g_bCourseUp)
			DoCOGSet();
        else
            DoChartUpdate();
    }

    if (pthumbwin)
		pthumbwin->SetMaxSize(cc1->GetParent()->GetSize());
}

void MainFrame::PositionConsole( void )
{
    if( NULL == cc1 ) return;
    //    Reposition console based on its size and chartcanvas size
    int ccx, ccy, ccsx, ccsy, consx, consy;
    cc1->GetSize( &ccsx, &ccsy );
    cc1->GetPosition( &ccx, &ccy );

    console->GetSize( &consx, &consy );

    wxPoint screen_pos = ClientToScreen( wxPoint( ccx + ccsx - consx - 2, ccy + 45 ) );
    console->Move( screen_pos );
}

void MainFrame::UpdateAllFonts()
{
    if( console ) {
        console->UpdateFonts();
        //    Reposition console
        PositionConsole();
    }

    if( g_pais_query_dialog_active ) {
        g_pais_query_dialog_active->Destroy();
        g_pais_query_dialog_active = NULL;
    }

    if( pWayPointMan ) pWayPointMan->ClearRoutePointFonts();

    cc1->Refresh();
}

void MainFrame::SetGroupIndex( int index )
{
    int new_index = index;
    if( index > (int) g_pGroupArray->GetCount() ) new_index = 0;

    bool bgroup_override = false;
    int old_group_index = new_index;

    if( !CheckGroup( new_index ) ) {
        new_index = 0;
        bgroup_override = true;
    }

    //    Get the currently displayed chart native scale, and the current ViewPort
    int current_chart_native_scale = cc1->GetCanvasChartNativeScale();
    ViewPort vp = cc1->GetVP();

    g_GroupIndex = new_index;

    //  Invalidate the "sticky" chart on group change, since it might not be in the new group
    g_sticky_chart = -1;

    //    We need a new chartstack and quilt to figure out which chart to open in the new group
    cc1->UpdateCanvasOnGroupChange();

    int dbi_hint = cc1->FindClosestCanvasChartdbIndex( current_chart_native_scale );

    //    Refresh the canvas, selecting the "best" chart,
    //    applying the prior ViewPort exactly
    ChartsRefresh( dbi_hint, vp, false );

    //    Message box is deferred so that canvas refresh occurs properly before dialog
    if( bgroup_override ) {
        wxString msg( _("Group \"") );
        msg += GetGroupName( old_group_index );
        msg += _("\" is empty, switching to \"All Active Charts\" group.");

        OCPNMessageBox( this, msg, _("OpenCPN Group Notice"), wxOK );
    }
}

void MainFrame::OnToolLeftClick( wxCommandEvent& event )
{
    if( s_ProgDialog ) return;

    switch( event.GetId() ){
        case ID_STKUP:
            DoStackUp();
            DoChartUpdate();
            break;

        case ID_STKDN:
            DoStackDown();
            DoChartUpdate();
            break;

        case ID_ZOOMIN: {
            cc1->ZoomCanvasIn( 2.0 );
            DoChartUpdate();
            break;
        }

        case ID_ZOOMOUT: {
            cc1->ZoomCanvasOut( 2.0 );
            DoChartUpdate();
            break;
        }

        case ID_ROUTE: {
            nRoute_State = 1;
            cc1->SetCursor( *cc1->pCursorPencil );
            break;
        }

        case ID_FOLLOW: {
            TogglebFollow();
            break;
        }

#ifdef USE_S57
        case ID_TEXT: {
            ps52plib->SetShowS57Text( !ps52plib->GetShowS57Text() );
            if( g_toolbar ) g_toolbar->ToggleTool( ID_TEXT, ps52plib->GetShowS57Text() );
            cc1->ReloadVP();
            break;
        }
#endif

        case ID_AIS: {
            g_bShowAIS = !g_bShowAIS;

            if( g_toolbar ) {
                if( g_bShowAIS )
                    g_toolbar->SetToolShortHelp( ID_AIS, _("Hide AIS Targets") );
                else
                    g_toolbar->SetToolShortHelp( ID_AIS, _("Show AIS Targets") );
            }

            wxString iconName;
            if( g_bShowAIS )
                iconName = _T("AIS");
            else
                iconName = _T("AIS_Disabled");

            if( m_pAISTool && g_toolbar) {
                g_toolbar->SetToolNormalBitmapEx( m_pAISTool, iconName );
                g_toolbar->Refresh();
                m_lastAISiconName = iconName;
            }

            cc1->ReloadVP();

            break;
        }

        case ID_SETTINGS: {

            bool bnewtoolbar = !( DoOptionsDialog() == 0 );

//              Apply various system settings
            ApplyGlobalSettings( true, bnewtoolbar );                 // flying update

            if( g_FloatingToolbarDialog ) g_FloatingToolbarDialog->RefreshFadeTimer();

            if( cc1->GetbShowCurrent() || cc1->GetbShowTide() ) LoadHarmonics();
//  The chart display options may have changed, especially on S57 ENC,
//  So, flush the cache and redraw
            cc1->ReloadVP();
            break;
        }

        case ID_CURRENT: {
            LoadHarmonics();

            if( ptcmgr->IsReady() ) {
                cc1->SetbShowCurrent( !cc1->GetbShowCurrent() );
                if( g_toolbar ) g_toolbar->ToggleTool( ID_CURRENT, cc1->GetbShowCurrent() );
                cc1->ReloadVP();
            } else {
                wxLogMessage( _T("Chart1::Event...TCMgr Not Available") );
                cc1->SetbShowCurrent( false );
                if( g_toolbar ) g_toolbar->ToggleTool( ID_CURRENT, false );
            }

            if( cc1->GetbShowCurrent() ) {
                FrameTCTimer.Start( TIMER_TC_VALUE_SECONDS * 1000, wxTIMER_CONTINUOUS );
                cc1->SetbTCUpdate( true );                        // force immediate update
            } else
                FrameTCTimer.Stop();

            cc1->Refresh( false );

            break;

        }

        case ID_TIDE: {
            LoadHarmonics();

            if( ptcmgr->IsReady() ) {
                cc1->SetbShowTide( !cc1->GetbShowTide() );
                if( g_toolbar ) g_toolbar->ToggleTool( ID_TIDE, cc1->GetbShowTide() );
                cc1->ReloadVP();
            } else {
                wxLogMessage( _("Chart1::Event...TCMgr Not Available") );
                cc1->SetbShowTide( false );
                if( g_toolbar ) g_toolbar->ToggleTool( ID_TIDE, false );
            }

            if( cc1->GetbShowTide() ) {
                FrameTCTimer.Start( TIMER_TC_VALUE_SECONDS * 1000, wxTIMER_CONTINUOUS );
                cc1->SetbTCUpdate( true );                        // force immediate update
            } else
                FrameTCTimer.Stop();

            cc1->Refresh( false );

            break;
        }

        case ID_HELP: {
            if( !g_pAboutDlg ) g_pAboutDlg = new AboutDialog( this, &g_SData_Locn );

            g_pAboutDlg->Update();
            g_pAboutDlg->Show();

            break;
        }

        case ID_PRINT: {
            DoPrint();
            break;
        }

        case ID_COLSCHEME: {
            ToggleColorScheme();
            break;
        }

        case ID_TBEXIT: {
            Close();
            break;
        }

        case ID_ROUTEMANAGER: {
            if( NULL == pRouteManagerDialog )         // There is one global instance of the Dialog
            pRouteManagerDialog = new RouteManagerDialog( cc1 );

            pRouteManagerDialog->UpdateRouteListCtrl();
            pRouteManagerDialog->UpdateTrkListCtrl();
            pRouteManagerDialog->UpdateWptListCtrl();
            pRouteManagerDialog->UpdateLayListCtrl();
            pRouteManagerDialog->Show();

            //    Required if RMDialog is not STAY_ON_TOP
#ifdef __WXOSX__
            pRouteManagerDialog->Centre();
            pRouteManagerDialog->Raise();
#endif
            break;
        }

        case ID_TRACK: {
            if( !g_bTrackActive ) TrackOn();
            else
                TrackOff( true );
            break;
        }

        case ID_TBSTATBOX: {
            ToggleCourseUp();
            break;
        }

        case ID_MOB: {
            ActivateMOB();
            break;
        }

        default: {
            //        Look for PlugIn tools
            //        If found, make the callback.
            //        TODO Modify this to allow multiple tools per plugin
            if( g_pi_manager ) {
                ArrayOfPlugInToolbarTools tool_array = g_pi_manager->GetPluginToolbarToolArray();
                for( unsigned int i = 0; i < tool_array.GetCount(); i++ ) {
                    PlugInToolbarToolContainer *pttc = tool_array.Item( i );
                    if( event.GetId() == pttc->id ) {
                        if( pttc->m_pplugin ) pttc->m_pplugin->OnToolbarToolCallback( pttc->id );
                    }
                }
            }
            break;
        }

    }         // switch
}

void MainFrame::ToggleColorScheme()
{
    ColorScheme s = GetColorScheme();
    int is = (int) s;
    is++;
    s = (ColorScheme) is;
    if( s == N_COLOR_SCHEMES ) s = GLOBAL_COLOR_SCHEME_RGB;

    SetAndApplyColorScheme( s );
}

void MainFrame::ToggleFullScreen()
{
	bool to = !IsFullScreen();
	long style = wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION | wxFULLSCREEN_NOMENUBAR;

	if (g_FloatingToolbarDialog)
		g_FloatingToolbarDialog->Show(global::OCPN::get().gui().toolbar().full_screen | !to);

	ShowFullScreen(to, style);
	UpdateToolbar(global_color_scheme);
    Layout();
}

void MainFrame::ActivateMOB( void )
{
    //    The MOB point
    wxDateTime mob_time = wxDateTime::Now();
    wxString mob_label( _( "MAN OVERBOARD" ) );
    mob_label += _T(" at ");
    mob_label += mob_time.FormatTime();

    RoutePoint *pWP_MOB = new RoutePoint( gLat, gLon, _T ( "mob" ), mob_label);
    pWP_MOB->m_bKeepXRoute = true;
    pWP_MOB->m_bIsolatedMark = true;
    pSelect->AddSelectableRoutePoint( gLat, gLon, pWP_MOB );
    pConfig->AddNewWayPoint( pWP_MOB, -1 );       // use auto next num


	const global::Navigation::Data & nav = global::OCPN::get().nav().get_data();
    if( bGPSValid && !wxIsNaN(nav.cog) && !wxIsNaN(nav.sog) ) {
        //    Create a point that is one mile along the present course
        double zlat, zlon;
        ll_gc_ll( gLat, gLon, nav.cog, 1.0, &zlat, &zlon );

        RoutePoint *pWP_src = new RoutePoint( zlat, zlon, g_default_wp_icon, wxString( _( "1.0 NM along COG" )));
        pSelect->AddSelectableRoutePoint( zlat, zlon, pWP_src );

        Route *temp_route = new Route();
        pRouteList->push_back(temp_route);

        temp_route->AddPoint( pWP_src );
        temp_route->AddPoint( pWP_MOB );

        pSelect->AddSelectableRouteSegment( gLat, gLon, zlat, zlon, pWP_src, pWP_MOB, temp_route );

        temp_route->m_RouteNameString = _("Temporary MOB Route");
        temp_route->m_RouteStartString = _("Assumed 1 Mile Point");
        ;
        temp_route->m_RouteEndString = mob_label;

        temp_route->m_bDeleteOnArrival = false;

        temp_route->SetRouteArrivalRadius( -1.0 );                    // never arrives

        temp_route->RebuildGUIDList();         // ensure the GUID list is intact and good

        if( g_pRouteMan->GetpActiveRoute() ) g_pRouteMan->DeactivateRoute();
        g_pRouteMan->ActivateRoute( temp_route, pWP_MOB );

        wxJSONValue v;
        v[_T("GUID")] = temp_route->m_GUID;
        wxString msg_id( _T("OCPN_MAN_OVERBOARD") );
        g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
    }

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) {
        pRouteManagerDialog->UpdateRouteListCtrl();
        pRouteManagerDialog->UpdateWptListCtrl();
    }

    cc1->Refresh( false );

    wxString mob_message( _( "MAN OVERBOARD" ) );
    mob_message += _T(" Time: ");
    mob_message += mob_time.Format();
    mob_message += _T("  Position: ");
    mob_message += toSDMM( 1, gLat );
    mob_message += _T("   ");
    mob_message += toSDMM( 2, gLon );
    wxLogMessage( mob_message );

}
void MainFrame::TrackOn( void )
{
    g_bTrackActive = true;
    g_pActiveTrack = new Track();

    pRouteList->push_back(g_pActiveTrack);
    g_pActiveTrack->Start();

    if( g_toolbar )
        g_toolbar->ToggleTool( ID_TRACK, g_bTrackActive );

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
    {
        pRouteManagerDialog->UpdateTrkListCtrl();
        pRouteManagerDialog->UpdateRouteListCtrl();
    }

    wxJSONValue v;
    wxDateTime now;
    now = now.Now().ToUTC();
    wxString name = g_pActiveTrack->m_RouteNameString;
    if(name.IsEmpty())
    {
        RoutePoint *rp = g_pActiveTrack->GetPoint( 1 );
        if( rp && rp->GetCreateTime().IsValid() )
            name = rp->GetCreateTime().FormatISODate() + _T(" ") + rp->GetCreateTime().FormatISOTime();
        else
            name = _("(Unnamed Track)");
    }
    v[_T("Name")] = name;
    v[_T("GUID")] = g_pActiveTrack->m_GUID;
    wxString msg_id( _T("OCPN_TRK_ACTIVATED") );
    g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
}

Track *MainFrame::TrackOff( bool do_add_point )
{
    Track *return_val = g_pActiveTrack;

    if( g_pActiveTrack )
    {
        wxJSONValue v;
        wxString msg_id( _T("OCPN_TRK_DEACTIVATED") );
        v[_T("GUID")] = g_pActiveTrack->m_GUID;
        g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );

        g_pActiveTrack->Stop( do_add_point );

        if( g_pActiveTrack->GetnPoints() < 2 ) {
            g_pRouteMan->DeleteRoute( g_pActiveTrack );
            return_val = NULL;
        }
        else {
            if( g_bTrackDaily ) {
                Track *pExtendTrack = g_pActiveTrack->DoExtendDaily();
                if(pExtendTrack) {
                    g_pRouteMan->DeleteRoute( g_pActiveTrack );
                    return_val = pExtendTrack;
                }
            }
        }
    }

    g_pActiveTrack = NULL;

    g_bTrackActive = false;

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
    {
        pRouteManagerDialog->UpdateTrkListCtrl();
        pRouteManagerDialog->UpdateRouteListCtrl();
    }

    if( g_toolbar )
        g_toolbar->ToggleTool( ID_TRACK, g_bTrackActive );

    return return_val;
}

void MainFrame::TrackMidnightRestart( void )
{
    if( !g_pActiveTrack )
        return;

    Track *pPreviousTrack = TrackOff( true );
    TrackOn();

    //  Set the restarted track's current state such that the current track point's attributes match the
    //  attributes of the last point of the track that was just stopped at midnight.

    if( pPreviousTrack ) {
        RoutePoint *pMidnightPoint = pPreviousTrack->GetLastPoint();
        g_pActiveTrack->AdjustCurrentTrackPoint(pMidnightPoint);
    }

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) {
        pRouteManagerDialog->UpdateTrkListCtrl();
        pRouteManagerDialog->UpdateRouteListCtrl();
    }
}

void MainFrame::ToggleCourseUp(void)
{
    g_bCourseUp = !g_bCourseUp;

    if( g_bCourseUp ) {
		const global::Navigation::Data & nav = global::OCPN::get().nav().get_data();

        //    Stuff the COGAvg table in case COGUp is selected
        double stuff = 0.;
        if (!wxIsNaN(nav.cog))
			stuff = nav.cog;

        if( g_COGAvgSec > 0) {
            for( int i = 0; i < g_COGAvgSec; i++ )
                COGTable[i] = stuff;
        }
        g_COGAvg = stuff;
    }

    DoCOGSet();
    UpdateGPSCompassStatusBox( true );
    DoChartUpdate();
    cc1->ReloadVP();
}

void MainFrame::ToggleENCText( void )
{
#ifdef USE_S57
    if( ps52plib ) {
        ps52plib->SetShowS57Text( !ps52plib->GetShowS57Text() );
        if( g_toolbar ) g_toolbar->ToggleTool( ID_TEXT, ps52plib->GetShowS57Text() );
        cc1->ReloadVP();
    }

#endif
}

void MainFrame::ToggleSoundings( void )
{
#ifdef USE_S57
    if( ps52plib ) {
        ps52plib->SetShowSoundings( !ps52plib->GetShowSoundings() );
        cc1->ReloadVP();
    }
#endif
}

bool MainFrame::ToggleLights( bool doToggle, bool temporary )
{
    bool oldstate = true;
#ifdef USE_S57
    if( ps52plib ) {
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "LIGHTS", 6 ) ) {
                oldstate = pOLE->nViz != 0;
                if( doToggle ) pOLE->nViz = !pOLE->nViz;
                break;
            }
        }
        if( doToggle ) {
            if( ! temporary ) {
                ps52plib->GenerateStateHash();
                cc1->ReloadVP();
            }
        }
    }
#endif
    return oldstate;
}

void MainFrame::ToggleRocks( void )
{
#ifdef USE_S57
    if( ps52plib ) {
        int vis =  0;
        // Need to loop once for UWTROC, which is our "master", then for
        // other categories, since order is unknown?
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "UWTROC", 6 ) ) {
                pOLE->nViz = !pOLE->nViz;
                vis = pOLE->nViz;
            }
        }
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "OBSTRN", 6 ) ) {
                pOLE->nViz = vis;
            }
            if( !strncmp( pOLE->OBJLName, "WRECKS", 6 ) ) {
                pOLE->nViz = vis;
            }
        }
        ps52plib->GenerateStateHash();
        cc1->ReloadVP();
    }
#endif
}

void MainFrame::ToggleAnchor( void )
{
#ifdef USE_S57
    if( ps52plib ) {
        int vis =  0;
        // Need to loop once for SBDARE, which is our "master", then for
        // other categories, since order is unknown?
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "SBDARE", 6 ) ) {
                pOLE->nViz = !pOLE->nViz;
                vis = pOLE->nViz;
                break;
            }
        }
        const char * categories[] = { "ACHBRT", "ACHARE", "CBLSUB", "PIPARE", "PIPSOL", "TUNNEL" };
        unsigned int num = sizeof(categories) / sizeof(categories[0]);
        unsigned int cnt = 0;
        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
            for( unsigned int c = 0; c < num; c++ ) {
                if( !strncmp( pOLE->OBJLName, categories[c], 6 ) ) {
                    pOLE->nViz = vis;
                    cnt++;
                    break;
                }
            }
            if( cnt == num ) break;
        }
        ps52plib->GenerateStateHash();
        cc1->ReloadVP();
    }
#endif
}

void MainFrame::TogglebFollow( void )
{
    if( !cc1->m_bFollow ) SetbFollow();
    else
        ClearbFollow();
}

void MainFrame::SetbFollow(void)
{
    cc1->m_bFollow = true;
	SetToolbarItemState(ID_FOLLOW, cc1->m_bFollow);
    DoChartUpdate();
    cc1->ReloadVP();
}

void MainFrame::ClearbFollow(void)
{
    //    Center the screen on the GPS position, for lack of a better place
    vLat = gLat;
    vLon = gLon;
    cc1->m_bFollow = false;
	SetToolbarItemState(ID_FOLLOW, cc1->m_bFollow);
    DoChartUpdate();
    cc1->ReloadVP();
}

void MainFrame::ToggleChartOutlines(void)
{
	global::GUI & gui = global::OCPN::get().gui();

	gui.set_view_show_outlines(!gui.view().show_outlines);
	cc1->Refresh(false);
}

void MainFrame::SetToolbarItemState(int tool_id, bool state)
{
	if (g_toolbar)
		g_toolbar->ToggleTool(tool_id, state);
}

void MainFrame::SetToolbarItemBitmaps(int tool_id, wxBitmap *bmp, wxBitmap *bmpRollover)
{
    if( g_toolbar ) {
        g_toolbar->SetToolBitmaps( tool_id, bmp, bmpRollover );
        wxRect rect = g_toolbar->GetToolRect( tool_id );
        g_toolbar->RefreshRect( rect );
    }
}

void MainFrame::ApplyGlobalSettings(bool, bool bnewtoolbar)
{
    //             ShowDebugWindow as a wxStatusBar
    m_StatusBarFieldCount = 5;

#ifdef __WXMSW__
    UseNativeStatusBar( false );              // better for MSW, undocumented in frame.cpp
#endif

    if( pConfig->m_bShowDebugWindows ) {
        if( !m_pStatusBar ) {
            m_pStatusBar = CreateStatusBar( m_StatusBarFieldCount, 0 );   // No wxST_SIZEGRIP needed
            ApplyGlobalColorSchemetoStatusBar();
            SendSizeEvent();                        // seem only needed for MSW...
        }

    } else {
        if( m_pStatusBar ) {
            m_pStatusBar->Destroy();
            m_pStatusBar = NULL;
            SetStatusBar( NULL );

            SendSizeEvent();                        // seem only needed for MSW...
            Refresh( false );
        }
    }

    if (bnewtoolbar)
		UpdateToolbar(global_color_scheme);
}

void MainFrame::SubmergeToolbarIfOverlap( int x, int y, int margin )
{
    if (g_FloatingToolbarDialog) {
        wxRect rect = g_FloatingToolbarDialog->GetScreenRect();
        rect.Inflate(margin);
        if (rect.Contains(x, y))
			g_FloatingToolbarDialog->Submerge();
    }
}

void MainFrame::SubmergeToolbar( void )
{
    if (g_FloatingToolbarDialog)
		g_FloatingToolbarDialog->Submerge();
}

void MainFrame::SurfaceToolbar(void)
{
	if (g_FloatingToolbarDialog && g_FloatingToolbarDialog->IsToolbarShown()) {
		if (IsFullScreen()) {
			if (global::OCPN::get().gui().toolbar().full_screen) {
				g_FloatingToolbarDialog->Surface();
			}
		} else
			g_FloatingToolbarDialog->Surface();
	}
	gFrame->Raise();
}

void MainFrame::JumpToPosition( double lat, double lon, double scale )
{
    vLat = lat;
    vLon = lon;
    cc1->m_bFollow = false;
    DoChartUpdate();

    cc1->SetViewPoint( lat, lon, scale, 0, cc1->GetVPRotation() );
    cc1->ReloadVP();

    SetToolbarItemState( ID_FOLLOW, false );

    if( g_pi_manager ) {
        g_pi_manager->SendViewPortToRequestingPlugIns( cc1->GetVP() );
    }
}

int MainFrame::DoOptionsDialog()
{
    static int lastPage = -1;
    static wxPoint lastWindowPos( 0,0 );
    static wxSize lastWindowSize( 0,0 );

    g_boptionsactive = true;

    ::wxBeginBusyCursor();
    options optionsDlg( this, -1, _("Options") );
    ::wxEndBusyCursor();

//    Set initial Chart Dir
    optionsDlg.SetInitChartDir( *pInit_Chart_Dir );

//      Pass two working pointers for Chart Dir Dialog
    optionsDlg.SetCurrentDirList( ChartData->GetChartDirArray() );
    ArrayOfCDI * pWorkDirArray = new ArrayOfCDI; // FIXME: dynamic allocation, to be used in dialog, deletion later... in this method
    optionsDlg.SetWorkDirListPtr(pWorkDirArray);

//      Pass a ptr to MyConfig, for updates
    optionsDlg.SetConfigPtr(pConfig);

    optionsDlg.SetInitialSettings();

    bDBUpdateInProgress = true;

    bPrevQuilt = g_bQuiltEnable;
    bPrevFullScreenQuilt = g_bFullScreenQuilt;
    bPrevOGL = g_bopengl;

    prev_locale = g_locale;

    bool b_sub = false;
    if( g_FloatingToolbarDialog && g_FloatingToolbarDialog->IsShown() ) {
        wxRect bx_rect = optionsDlg.GetScreenRect();
        wxRect tb_rect = g_FloatingToolbarDialog->GetScreenRect();
        if( tb_rect.Intersects( bx_rect ) ) b_sub = true;

        if( b_sub ) g_FloatingToolbarDialog->Submerge();
    }

#ifdef __WXOSX__
    if(stats) stats->Hide();
#endif

    if( lastPage >= 0 ) optionsDlg.m_pListbook->SetSelection( lastPage );
    optionsDlg.lastWindowPos = lastWindowPos;
    if( lastWindowPos != wxPoint(0,0) ) {
        optionsDlg.Move( lastWindowPos );
        optionsDlg.SetSize( lastWindowSize );
    } else {
        optionsDlg.Center();
    }

    if( g_FloatingToolbarDialog) g_FloatingToolbarDialog->DisableTooltips();

    int rr = optionsDlg.ShowModal();

    if( g_FloatingToolbarDialog) g_FloatingToolbarDialog->EnableTooltips();

    lastPage = optionsDlg.lastPage;
    lastWindowPos = optionsDlg.lastWindowPos;
    lastWindowSize = optionsDlg.lastWindowSize;

    if( b_sub ) {
        SurfaceToolbar();
        cc1->SetFocus();
    }

#ifdef __WXGTK__
    Raise();                      // I dunno why...
#endif

    bool ret_val = false;
    if( rr ) {
        ProcessOptionsDialog( rr, &optionsDlg );
        ret_val = true;
    }

    delete pWorkDirArray;

    bDBUpdateInProgress = false;
    if (g_FloatingToolbarDialog) {
        if (IsFullScreen() && !global::OCPN::get().gui().toolbar().full_screen)
			g_FloatingToolbarDialog->Submerge();
    }

#ifdef __WXMAC__
    if(stats) stats->Show();
#endif

    Refresh( false );

    g_boptionsactive = false;

    return ret_val;
}

int MainFrame::ProcessOptionsDialog( int rr, options* dialog )
{
    ArrayOfCDI * pWorkDirArray = dialog->GetWorkDirListPtr();
    if( ( rr & VISIT_CHARTS )
            && ( ( rr & CHANGE_CHARTS ) || ( rr & FORCE_UPDATE ) || ( rr & SCAN_UPDATE ) ) ) {

        //    Capture the currently open chart
        wxString chart_file_name;
        if( cc1->GetQuiltMode() ) {
            int dbi = cc1->GetQuiltRefChartdbIndex();
            chart_file_name = ChartData->GetDBChartFileName( dbi );
        } else
            if( Current_Ch ) chart_file_name = Current_Ch->GetFullPath();

        UpdateChartDatabaseInplace( *pWorkDirArray, ( ( rr & FORCE_UPDATE ) == FORCE_UPDATE ),
                true, *pChartListFileName );

        //    Re-open the last open chart
        int dbii = ChartData->FinddbIndex( chart_file_name );
        ChartsRefresh( dbii, cc1->GetVP() );
    }

    if( ( rr & LOCALE_CHANGED ) || ( rr & STYLE_CHANGED ) ) {
        if( ( prev_locale != g_locale ) || ( rr & STYLE_CHANGED ) ) {
            OCPNMessageBox(NULL, _("Please restart OpenCPN to activate language or style changes."),
                    _("OpenCPN Info"), wxOK | wxICON_INFORMATION );
            if( rr & LOCALE_CHANGED ) g_blocale_changed = true;;
        }
    }

    if( ( ( rr & VISIT_CHARTS )
            && ( ( rr & CHANGE_CHARTS ) || ( rr & FORCE_UPDATE ) || ( rr & SCAN_UPDATE ) ) )
            || ( rr & GROUPS_CHANGED ) ) {
        ScrubGroupArray();
        ChartData->ApplyGroupArray( g_pGroupArray );
        SetGroupIndex( g_GroupIndex );
    }

    if( rr & GROUPS_CHANGED ) {
        pConfig->DestroyConfigGroups();
        pConfig->CreateConfigGroups( g_pGroupArray );
    }

    if( rr & TIDES_CHANGED ) {
        LoadHarmonics();
    }

    pConfig->UpdateSettings();

    if( g_pActiveTrack ) {
        g_pActiveTrack->SetPrecision( g_nTrackPrecision );
    }

    if( ( bPrevQuilt != g_bQuiltEnable ) || ( bPrevFullScreenQuilt != g_bFullScreenQuilt ) ) {
        cc1->SetQuiltMode( g_bQuiltEnable );
        SetupQuiltMode();
    }

	const global::Navigation::Data & nav = global::OCPN::get().nav().get_data();

    if( g_bCourseUp ) {
        //    Stuff the COGAvg table in case COGUp is selected
        double stuff = 0.;
        if (!wxIsNaN(nav.cog))
			stuff = nav.cog;
        if (g_COGAvgSec > 0) {
            for( int i = 0; i < g_COGAvgSec; i++ )
                COGTable[i] = stuff;
        }

        g_COGAvg = stuff;

        //    Short circuit the COG timer to force immediate refresh of canvas in case COGUp is selected
        FrameCOGTimer.Stop();
        FrameCOGTimer.Start( 100, wxTIMER_CONTINUOUS );
    }

    //    Stuff the Filter tables
    double stuffcog = 0.;
    double stuffsog = 0.;
    if (!wxIsNaN(nav.cog))
		stuffcog = nav.cog;
    if (!wxIsNaN(nav.sog))
		stuffsog = nav.sog;

    for( int i = 0; i < MAX_COGSOG_FILTER_SECONDS; i++ ) {
        COGFilterTable[i] = stuffcog;
        SOGFilterTable[i] = stuffsog;
    }
    m_COGFilterLast = stuffcog;

    SetChartUpdatePeriod( cc1->GetVP() );              // Pick up changes to skew compensator

    return 0;
}

wxString MainFrame::GetGroupName( int igroup )
{
    ChartGroup *pGroup = g_pGroupArray->Item( igroup - 1 );
    return pGroup->m_group_name;
}

bool MainFrame::CheckGroup( int igroup )
{
    if( igroup == 0 ) return true;              // "all charts" is always OK

    ChartGroup *pGroup = g_pGroupArray->Item( igroup - 1 );
    bool b_chart_in_group = false;

    for( unsigned int j = 0; j < pGroup->m_element_array.GetCount(); j++ ) {
        wxString element_root = pGroup->m_element_array.Item( j )->m_element_name;

        for( unsigned int ic = 0; ic < (unsigned int) ChartData->GetChartTableEntries(); ic++ ) {
            const ChartTableEntry & cte = ChartData->GetChartTableEntry(ic);
            wxString chart_full_path(cte.GetpFullPath(), wxConvUTF8);

            if( chart_full_path.StartsWith( element_root ) ) {
                b_chart_in_group = true;
                break;
            }
        }

        if( b_chart_in_group ) break;
    }

    return b_chart_in_group;                           // this group is empty

}

void MainFrame::ScrubGroupArray()
{
    //    For each group,
    //    make sure that each group element (dir or chart) references at least oneitem in the database.
    //    If not, remove the element.

    unsigned int igroup = 0;
    while( igroup < g_pGroupArray->GetCount() ) {
        bool b_chart_in_element = false;
        ChartGroup *pGroup = g_pGroupArray->Item( igroup );

        for( unsigned int j = 0; j < pGroup->m_element_array.GetCount(); j++ ) {
            wxString element_root = pGroup->m_element_array.Item( j )->m_element_name;

            for (unsigned int ic = 0; ic < (unsigned int) ChartData->GetChartTableEntries(); ic++) {
                const ChartTableEntry & cte = ChartData->GetChartTableEntry(ic);
                wxString chart_full_path(cte.GetpFullPath(), wxConvUTF8);

                if (chart_full_path.StartsWith(element_root)) {
                    b_chart_in_element = true;
                    break;
                }
            }

            if( !b_chart_in_element )             // delete the element
            {
                ChartGroupElement *pelement = pGroup->m_element_array.Item( j );
                pGroup->m_element_array.RemoveAt( j );
                delete pelement;
            }
        }

        igroup++;                                 // next group
    }
}

// Flav: This method reloads all charts for convenience
void MainFrame::ChartsRefresh( int dbi_hint, ViewPort &vp, bool b_purge )
{
    if( !ChartData ) return;

    ::wxBeginBusyCursor();

    bool b_run = FrameTimer1.IsRunning();

    FrameTimer1.Stop();                  // stop other asynchronous activity

    cc1->InvalidateQuilt();
    cc1->SetQuiltRefChart( -1 );

    Current_Ch = NULL;

    delete pCurrentStack;
    pCurrentStack = NULL;

    if( b_purge ) ChartData->PurgeCache();

    //    Build a new ChartStack
    pCurrentStack = new ChartStack;
    ChartData->BuildChartStack( pCurrentStack, vLat, vLon );

    if( -1 != dbi_hint ) {
        if( cc1->GetQuiltMode() ) {
            pCurrentStack->SetCurrentEntryFromdbIndex( dbi_hint );
            cc1->SetQuiltRefChart( dbi_hint );
        } else {
            //      Open the saved chart
            ChartBase *pTentative_Chart;
            pTentative_Chart = ChartData->OpenChartFromDB( dbi_hint, FULL_INIT );

            if( pTentative_Chart ) {
                if( Current_Ch ) Current_Ch->Deactivate();

                Current_Ch = pTentative_Chart;
                Current_Ch->Activate();

                pCurrentStack->CurrentStackEntry = ChartData->GetStackEntry( pCurrentStack,
                        Current_Ch->GetFullPath() );
            } else
                SetChartThumbnail( dbi_hint );       // need to reset thumbnail on failed chart open
        }

        //          Refresh the Piano Bar
        if( stats ) {
            std::vector<int> piano_active_chart_index_array;
            piano_active_chart_index_array.push_back(pCurrentStack->GetCurrentEntrydbIndex());
            stats->pPiano->SetActiveKeyArray(piano_active_chart_index_array);

            stats->Refresh( true );
        }

    } else {
        //    Select reference chart from the stack, as though clicked by user
        //    Make it the smallest scale chart on the stack
        pCurrentStack->CurrentStackEntry = pCurrentStack->nEntry - 1;
        int selected_index = pCurrentStack->GetCurrentEntrydbIndex();
        cc1->SetQuiltRefChart( selected_index );
    }

    //    Validate the correct single chart, or set the quilt mode as appropriate
    SetupQuiltMode();

    if( vp.IsValid() ) cc1->LoadVP( vp );
    else
        cc1->ReloadVP();

    UpdateControlBar();

    UpdateGPSCompassStatusBox( true );

    cc1->SetCursor( wxCURSOR_ARROW );

    if( b_run ) FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

    ::wxEndBusyCursor();

}

bool MainFrame::UpdateChartDatabaseInplace(
		ArrayOfCDI & DirArray,
		bool b_force,
		bool b_prog,
        const wxString & ChartListFileName)
{
    bool b_run = FrameTimer1.IsRunning();
    FrameTimer1.Stop();                  // stop other asynchronous activity

    cc1->InvalidateQuilt();
    cc1->SetQuiltRefChart( -1 );

    Current_Ch = NULL;

    delete pCurrentStack;
    pCurrentStack = NULL;

    ::wxBeginBusyCursor();

    wxProgressDialog *pprog = NULL;
    if( b_prog ) {
        pprog = new wxProgressDialog( _("OpenCPN Chart Update"), _T(""), 100, this,
                wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME );

        //    Make sure the dialog is big enough to be readable
        pprog->Hide();
        wxSize sz = pprog->GetSize();
        wxSize csz = GetClientSize();
        sz.x = csz.x * 7 / 10;
        pprog->SetSize( sz );
        pprog->Centre();
        pprog->Update( 1, _T("") );
        pprog->Show();
        pprog->Raise();
    }

    wxLogMessage(_T("   "));
    wxLogMessage(_T("Starting chart database Update..."));
    ChartData->Update(DirArray, b_force, pprog);
    ChartData->SaveBinary(ChartListFileName);
    wxLogMessage(_T("Finished chart database Update"));
    wxLogMessage(_T("   "));

    delete pprog;

    ::wxEndBusyCursor();

    pConfig->UpdateChartDirs(DirArray);

    if (b_run)
		FrameTimer1.Start( TIMER_GFRAME_1, wxTIMER_CONTINUOUS );

    return true;
}

void MainFrame::ToggleQuiltMode( void )
{
    if( cc1 ) {
        bool cur_mode = cc1->GetQuiltMode();

        if( !cc1->GetQuiltMode() && g_bQuiltEnable ) cc1->SetQuiltMode( true );
        else
            if( cc1->GetQuiltMode() ) {
                cc1->SetQuiltMode( false );
                g_sticky_chart = cc1->GetQuiltReferenceChartIndex();
            }


        if( cur_mode != cc1->GetQuiltMode() ) SetupQuiltMode();
    }
}

void MainFrame::SetQuiltMode( bool bquilt )
{
    if( cc1 ) cc1->SetQuiltMode( bquilt );
}

bool MainFrame::GetQuiltMode( void )
{
    if( cc1 ) return cc1->GetQuiltMode();
    else
        return false;
}

void MainFrame::SetupQuiltMode( void )
{

    if( cc1->GetQuiltMode() )                               // going to quilt mode
    {
        ChartData->LockCache();

        stats->pPiano->SetNoshowIndexArray(g_quilt_noshow_index_array);

        ocpnStyle::Style * style = g_StyleManager->GetCurrentStyle();

        stats->pPiano->SetVizIcon( new wxBitmap( style->GetIcon( _T("viz") ) ) );
        stats->pPiano->SetInVizIcon( new wxBitmap( style->GetIcon( _T("redX") ) ) );
        stats->pPiano->SetTMercIcon( new wxBitmap( style->GetIcon( _T("tmercprj") ) ) );
        stats->pPiano->SetSkewIcon( new wxBitmap( style->GetIcon( _T("skewprj") ) ) );

        stats->pPiano->SetRoundedRectangles( true );

        //    Select the proper Ref chart
        int target_new_dbindex = -1;
        if( pCurrentStack ) {
            target_new_dbindex = pCurrentStack->GetCurrentEntrydbIndex();

#ifdef QUILT_ONLY_MERC
            if(-1 != target_new_dbindex)
            {
                //    Check to see if the target new chart is Merc
                int proj = ChartData->GetDBChartProj(target_new_dbindex);
                int type = ChartData->GetDBChartType(target_new_dbindex);

                if(PROJECTION_MERCATOR != proj)
                {
                    // If it is not Merc, cannot use it for quilting
                    // walk the stack up looking for a satisfactory chart
                    int stack_index = pCurrentStack->CurrentStackEntry;

                    while((stack_index < pCurrentStack->nEntry-1) && (stack_index >= 0))
                    {
                        int proj_tent = ChartData->GetDBChartProj( pCurrentStack->GetDBIndex(stack_index));
                        int type_tent = ChartData->GetDBChartType( pCurrentStack->GetDBIndex(stack_index));

                        if((PROJECTION_MERCATOR ==proj_tent) && (type_tent == type))
                        {
                            target_new_dbindex = pCurrentStack->GetDBIndex(stack_index);
                            break;
                        }
                        stack_index++;
                    }
                }

            }
#endif
        }

        if( cc1->IsChartQuiltableRef( target_new_dbindex ) ) SelectQuiltRefdbChart(
                target_new_dbindex );
        else
            SelectQuiltRefdbChart( -1 );

        Current_Ch = NULL;                  // Bye....
        cc1->ReloadVP();

    } else                                                  // going to SC Mode
    {
        stats->pPiano->SetActiveKeyArray(std::vector<int>());
        stats->pPiano->SetNoshowIndexArray(std::vector<int>());
        stats->pPiano->SetSubliteIndexArray(std::vector<int>());
        stats->pPiano->SetVizIcon( NULL );
        stats->pPiano->SetInVizIcon( NULL );

        ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

        stats->pPiano->SetTMercIcon( new wxBitmap( style->GetIcon( _T("tmercprj") ) ) );
        stats->pPiano->SetSkewIcon( new wxBitmap( style->GetIcon( _T("skewprj") ) ) );

        stats->pPiano->SetRoundedRectangles( false );

    }

    //    When shifting from quilt to single chart mode, select the "best" single chart to show
    if( !cc1->GetQuiltMode() ) {
        if( ChartData && ChartData->IsValid() ) {
            ChartData->UnLockCache();

            double tLat, tLon;
            if( cc1->m_bFollow == true ) {
                tLat = gLat;
                tLon = gLon;
            } else {
                tLat = vLat;
                tLon = vLon;
            }

            if( !Current_Ch ) {

                // Build a temporary chart stack based on tLat, tLon
                ChartStack TempStack;
                ChartData->BuildChartStack( &TempStack, tLat, tLon, g_sticky_chart );

                //    Iterate over the quilt charts actually shown, looking for the largest scale chart that will be in the new chartstack....
                //    This will (almost?) always be the reference chart....

                ChartBase *Candidate_Chart = NULL;
                int cur_max_scale = (int) 1e8;

                ChartBase *pChart = cc1->GetFirstQuiltChart();
                while( pChart ) {
                    //  Is this pChart in new stack?
                    int tEntry = ChartData->GetStackEntry( &TempStack, pChart->GetFullPath() );
                    if( tEntry != -1 ) {
                        if( pChart->GetNativeScale() < cur_max_scale ) {
                            Candidate_Chart = pChart;
                            cur_max_scale = pChart->GetNativeScale();
                        }
                    }
                    pChart = cc1->GetNextQuiltChart();
                }

                Current_Ch = Candidate_Chart;

                //    If the quilt is empty, there is no "best" chart.
                //    So, open the smallest scale chart in the current stack
                if( NULL == Current_Ch ) {
                    Current_Ch = ChartData->OpenStackChartConditional( &TempStack,
                            TempStack.nEntry - 1, true, CHART_TYPE_DONTCARE,
                            CHART_FAMILY_DONTCARE );
                }
            }

            //  Invalidate all the charts in the quilt,
            // as any cached data may be region based and not have fullscreen coverage
            cc1->InvalidateAllQuiltPatchs();

            if( Current_Ch ) {
                int dbi = ChartData->FinddbIndex( Current_Ch->GetFullPath() );
                std::vector<int> one_array;
                one_array.push_back(dbi);
                stats->pPiano->SetActiveKeyArray(one_array);
            }

        }
        //    Invalidate the current stack so that it will be rebuilt on next tick
        if( pCurrentStack ) pCurrentStack->b_valid = false;
    }

}

void MainFrame::ClearRouteTool()
{
    if( g_toolbar ) g_toolbar->ToggleTool( ID_ROUTE, false );
}

void MainFrame::DoStackDown( void )
{
    int current_stack_index = pCurrentStack->CurrentStackEntry;

    if( 0 == current_stack_index ) return;

    if( !cc1->GetQuiltMode() ) SelectChartFromStack( current_stack_index - 1 );
    else {
        int new_dbIndex = pCurrentStack->GetDBIndex( current_stack_index - 1 );

        if( !cc1->IsChartQuiltableRef( new_dbIndex ) ) {
            ToggleQuiltMode();
            SelectChartFromStack( current_stack_index - 1 );
        } else
            SelectQuiltRefChart( current_stack_index - 1 );

    }

    cc1->SetQuiltChartHiLiteIndex( -1 );

    cc1->ReloadVP();
}

void MainFrame::DoStackUp( void )
{
    int current_stack_index = pCurrentStack->CurrentStackEntry;

    if( current_stack_index >= pCurrentStack->nEntry - 1 ) return;

    if( !cc1->GetQuiltMode() ) {
        SelectChartFromStack( current_stack_index + 1 );
    } else {
        int new_dbIndex = pCurrentStack->GetDBIndex( current_stack_index + 1 );

        if( !cc1->IsChartQuiltableRef( new_dbIndex ) ) {
            ToggleQuiltMode();
            SelectChartFromStack( current_stack_index + 1 );
        } else
            SelectQuiltRefChart( current_stack_index + 1 );
    }

    cc1->SetQuiltChartHiLiteIndex( -1 );

    cc1->ReloadVP();
}

//    Manage the application memory footprint on a periodic schedule
void MainFrame::OnMemFootTimer(wxTimerEvent &)
{
    MemFootTimer.Stop();

    int memsize = GetApplicationMemoryUse();

    g_MemFootMB = 100;
    // The application memory usage has exceeded the target, so try to manage it down....
    if (memsize > (g_MemFootMB * 1000)) {
        if (ChartData && cc1) {
            // Get a local copy of the cache info
            wxArrayPtrVoid * pCache = ChartData->GetChartCache();
            const unsigned int nCache = pCache->GetCount();
			std::vector<CacheEntry> cache;
			cache.reserve(nCache);
            for (unsigned int i = 0; i < nCache; ++i) {
                cache.push_back(*static_cast<CacheEntry *>(pCache->Item(i)));
            }

            if (nCache > 1) {
                // Bubble Sort the local cache entry array
                bool b_cont = true;
                while (b_cont) {
                    b_cont = false;
                    for (unsigned int i = 0; i < nCache - 1; ++i) {
                        if (cache[i].RecentTime > cache[i + 1].RecentTime) {
							std::swap(cache[i], cache[i+1]);
                            b_cont = true;
                            break;
                        }
                    }
                }

                // Free up some chart cache entries until the memory footprint target is realized

                unsigned int idelete = 0; // starting at top. which is oldest
                unsigned int idelete_max = pCache->GetCount();

                // How many can be deleted?
                unsigned int minimum_cache = 1;
                if (cc1->GetQuiltMode())
					minimum_cache = cc1->GetQuiltChartCount();

                while ((memsize > (g_MemFootMB * 1000)) && (pCache->GetCount() > minimum_cache) && (idelete < idelete_max)) {
                    ChartData->DeleteCacheChart(static_cast<ChartBase *>(cache[idelete].pChart));
                    idelete++;
                    memsize = GetApplicationMemoryUse();
                }
            }
        }
    }

    MemFootTimer.Start(9000, wxTIMER_CONTINUOUS);
}

wxString MainFrame::get_cog()
{
	const global::Navigation::Data & nav = global::OCPN::get().nav().get_data();

	if (wxIsNaN(nav.cog))
		return _T("COG ----- ");

	return wxString::Format(_T("COG %10.5f "), nav.cog);
}

wxString MainFrame::get_sog()
{
	const global::Navigation::Data & nav = global::OCPN::get().nav().get_data();

	if (wxIsNaN(nav.sog))
		return _T("SOG -----  ");

	return wxString::Format(_T("SOG %6.2f ") + getUsrSpeedUnit(), toUsrSpeed(nav.sog));
}

wxString MainFrame::prepare_logbook_message(const wxDateTime & lognow)
{
	wxString navmsg = _T("LOGBOOK:  ");
	navmsg += lognow.FormatISODate();
	navmsg += _T(" ");
	navmsg += lognow.FormatISOTime();
	navmsg += _T(" UTC ");

	if( bGPSValid ) {
		navmsg += wxString::Format( _T(" GPS Lat %10.5f Lon %10.5f "), gLat, gLon);
		navmsg += get_cog();
		navmsg += get_sog();
	} else {
		navmsg += wxString::Format( _T(" DR Lat %10.5f Lon %10.5f"), gLat, gLon );
	}
	return navmsg;
}

int ut_index;

void MainFrame::update_gps_watchdog()
{
	global::WatchDog & wdt = global::OCPN::get().wdt();

	// Update and check watchdog timer for GPS data source
	wdt.decrement_gps_watchdog();
	if (wdt.get_data().gps_watchdog <= 0) {
		bGPSValid = false;
		if (g_nNMEADebug && (wdt.get_data().gps_watchdog == 0))
			wxLogMessage(_T("   ***GPS Watchdog timeout..."));

		global::Navigation & nav = global::OCPN::get().nav();
		nav.set_speed_over_ground(NAN);
		nav.set_course_over_ground(NAN);
	}
}

void MainFrame::update_hdx_watchdog()
{
	global::WatchDog & wdt = global::OCPN::get().wdt();

	// Update and check watchdog timer for Mag Heading data source
	wdt.decrement_hdx_watchdog();
	if (wdt.get_data().hdx_watchdog <= 0) {
		global::Navigation & nav = global::OCPN::get().nav();
		nav.set_heading_magn(NAN);
		if (g_nNMEADebug && (wdt.get_data().hdx_watchdog == 0))
			wxLogMessage(_T("   ***HDx Watchdog timeout..."));
	}
}

void MainFrame::update_hdt_watchdog()
{
	global::WatchDog & wdt = global::OCPN::get().wdt();

	// Update and check watchdog timer for True Heading data source
	wdt.decrement_hdt_watchdog();
	if (wdt.get_data().hdt_watchdog <= 0) {
		g_bHDT_Rx = false;
		global::Navigation & nav = global::OCPN::get().nav();
		nav.set_heading_true(NAN);
		if (g_nNMEADebug && (wdt.get_data().hdt_watchdog == 0))
			wxLogMessage(_T("   ***HDT Watchdog timeout..."));
	}
}

void MainFrame::update_var_watchdog()
{
	global::WatchDog & wdt = global::OCPN::get().wdt();

	// Update and check watchdog timer for Magnetic Variation data source
	wdt.decrement_var_watchdog();
	if (wdt.get_data().var_watchdog <= 0) {
		g_bVAR_Rx = false;
		if (g_nNMEADebug && (wdt.get_data().var_watchdog == 0))
			wxLogMessage(_T("   ***VAR Watchdog timeout..."));
	}
}

void MainFrame::update_sat_watchdog()
{
	global::WatchDog & wdt = global::OCPN::get().wdt();

	// Update and check watchdog timer for GSV (Satellite data)
	wdt.decrement_sat_watchdog();
	if (wdt.get_data().sat_watchdog <= 0) {
		g_bSatValid = false;
		g_SatsInView = 0;
		if (g_nNMEADebug && (wdt.get_data().sat_watchdog == 0))
			wxLogMessage(_T("   ***SAT Watchdog timeout..."));
	}
}

void MainFrame::check_anchorwatch_1()
{
	if (pAnchorWatchPoint1) {
		double dist;
		double brg;
		DistanceBearingMercator(pAnchorWatchPoint1->m_lat, pAnchorWatchPoint1->m_lon, gLat, gLon, &brg, &dist);
		double d = g_nAWMax;
		( pAnchorWatchPoint1->GetName() ).ToDouble( &d );
		d = AnchorDistFix( d, AnchorPointMinDist, g_nAWMax );
		bool toofar = false;
		bool tooclose = false;
		if (d >= 0.0)
			toofar = (dist * 1852. > d);
		if (d < 0.0)
			tooclose = (dist * 1852 < -d);

		if (tooclose || toofar)
			AnchorAlertOn1 = true;
		else
			AnchorAlertOn1 = false;
	} else
		AnchorAlertOn1 = false;
}

void MainFrame::check_anchorwatch_2()
{
	if (pAnchorWatchPoint2) {
		double dist;
		double brg;
		DistanceBearingMercator(pAnchorWatchPoint2->m_lat, pAnchorWatchPoint2->m_lon, gLat, gLon, &brg, &dist);

		double d = g_nAWMax;
		pAnchorWatchPoint2->GetName().ToDouble(&d);
		d = AnchorDistFix(d, AnchorPointMinDist, g_nAWMax);
		bool toofar = false;
		bool tooclose = false;
		if (d >= 0)
			toofar = (dist * 1852. > d);
		if (d < 0)
			tooclose = (dist * 1852 < -d);

		if (tooclose || toofar)
			AnchorAlertOn2 = true;
		else
			AnchorAlertOn2 = false;
	} else
		AnchorAlertOn2 = false;
}

void MainFrame::send_gps_to_plugins() const
{
	// Build and send a Position Fix event to PlugIns
	if (!g_pi_manager)
		return;

	const global::Navigation::Data & nav_data = global::OCPN::get().nav().get_data();

	GenericPosDatEx GPSData;
	GPSData.kLat = gLat;
	GPSData.kLon = gLon;
	GPSData.kCog = nav_data.cog;
	GPSData.kSog = nav_data.sog;
	GPSData.kVar = nav_data.var;
	GPSData.kHdm = nav_data.hdm;
	GPSData.kHdt = nav_data.hdt;
	GPSData.nSats = g_SatsInView;

	GPSData.FixTime = m_fixtime;

	if (g_pi_manager)
		g_pi_manager->SendPositionFixToAllPlugIns(&GPSData);
}

void MainFrame::OnFrameTimer1(wxTimerEvent &)
{
	if (s_ProgDialog) {
		return;
	}

	if (g_unit_test_1) {

		cc1->m_bFollow = false;
		if (g_toolbar)
			g_toolbar->ToggleTool(ID_FOLLOW, cc1->m_bFollow);

		if (ChartData) {
			if (ut_index < ChartData->GetChartTableEntries()) {
				const ChartTableEntry *cte = &ChartData->GetChartTableEntry(ut_index);
				double lat = (cte->GetLatMax() + cte->GetLatMin()) / 2;
				double lon = (cte->GetLonMax() + cte->GetLonMin()) / 2;

				vLat = lat;
				vLon = lon;

				cc1->SetViewPoint(lat, lon);

				if (cc1->GetQuiltMode()) {
					if (cc1->IsChartQuiltableRef(ut_index))
						SelectQuiltRefdbChart(ut_index);
				} else
					SelectdbChart(ut_index);

				double ppm = cc1->GetCanvasScaleFactor() / cte->GetScale();
				ppm /= 2;
				cc1->SetVPScale(ppm);
				cc1->ReloadVP();
				ut_index++;
			}
		}
	}
	g_tick++;

#ifdef __WXOSX__
	// To fix an ugly bug ?? in wxWidgets for Carbon.....
	// Or, maybe this is the way Macs work....
	// Hide some non-UI Dialogs if the application is minimized....
	// They will be re-Show()-n in MainFrame::OnActivate()
	if(IsIconized()) {
		if (g_FloatingToolbarDialog) {
			if (g_FloatingToolbarDialog->IsShown())
				g_FloatingToolbarDialog->Submerge();
		}

        AppActivateList.Clear();
        if(cc1){
            for ( wxWindowList::iterator it = cc1->GetChildren().begin(); it != cc1->GetChildren().end(); ++it ) {
                if( (*it)->IsShown() ) {
                    (*it)->Hide();
                    AppActivateList.Append(*it);
                }
            }
        }

        if(gFrame){
            for ( wxWindowList::iterator it = gFrame->GetChildren().begin(); it != gFrame->GetChildren().end(); ++it ) {
                if( (*it)->IsShown() ) {
                    if( !(*it)->IsKindOf( CLASSINFO(ChartCanvas) ) ) {
                        (*it)->Hide();
                        AppActivateList.Append(*it);
                    }
                }
            }
        }
	}
#endif

	// Listen for quitflag to be set, requesting application close
	if (quitflag) {
		wxLogMessage(_T("Got quitflag from SIGUSR1"));
		FrameTimer1.Stop();
		Close();
		return;
	}

	if (bDBUpdateInProgress)
		return;

	FrameTimer1.Stop();

	update_gps_watchdog();
	update_hdx_watchdog();
	update_hdt_watchdog();
	update_var_watchdog();
	update_sat_watchdog();
	send_gps_to_plugins();
	check_anchorwatch_1();
	check_anchorwatch_2();

	if ((pAnchorWatchPoint1 || pAnchorWatchPoint2) && !bGPSValid)
		AnchorAlertOn1 = true;

	// Send current nav status data to log file on every half hour   // pjotrc 2010.02.09

	wxDateTime lognow = wxDateTime::Now().MakeGMT();   // pjotrc 2010.02.09
	wxTimeSpan logspan = lognow.Subtract(g_loglast_time);
	if ((logspan.IsLongerThan(wxTimeSpan(0, 30, 0, 0))) || (lognow.GetMinute() == 0) || (lognow.GetMinute() == 30)) {
		if (logspan.IsLongerThan(wxTimeSpan(0, 1, 0, 0))) {
			wxLogMessage(prepare_logbook_message(lognow));
			g_loglast_time = lognow;

			if (lognow.GetHour() == 0 && lognow.GetMinute() == 0 && g_bTrackDaily)
				TrackMidnightRestart();

			int bells = (lognow.GetHour() % 4) * 2; // 2 bells each hour
			if (lognow.GetMinute() != 0)
				bells++; // + 1 bell on 30 minutes, FIXME
			if (!bells)
				bells = 8; // 0 is 8 bells, FIXME

			if (g_bPlayShipsBells && ((lognow.GetMinute() == 0) || (lognow.GetMinute() == 30))) {
				if (!bells_sound[bells - 1].IsOk()) { // load the bells sound
					wxString soundfile = _T("sounds");
					appendOSDirSlash(soundfile);
					soundfile += wxString( bells_sound_file_name[bells - 1], wxConvUTF8 );
					soundfile.Prepend( g_SData_Locn );
					bells_sound[bells - 1].Create( soundfile );
					wxLogMessage( _T("Using bells sound file: ") + soundfile );

				}

				if (bells_sound[bells - 1].IsOk())
					bells_sound[bells - 1].Play();
			}
		}
	}

	// Update the Toolbar Status windows and lower status bar the first time watchdog times out
	const global::WatchDog::Data wdt = global::OCPN::get().wdt().get_data();
	if ((wdt.gps_watchdog == 0) || (wdt.sat_watchdog == 0)) {
		wxString sogcog( _T("SOG --- ") + getUsrSpeedUnit() + _T(" COG ---\u00B0") );
		if (GetStatusBar())
			SetStatusText(sogcog, STAT_FIELD_SOGCOG);

		global::OCPN::get().nav().set_course_over_ground(0.0); // say speed is zero to kill ownship predictor
	}

	if (cc1) {
		double cursor_lat, cursor_lon;
		cc1->GetCursorLatLon(&cursor_lat, &cursor_lon);

		wxString s1;
		s1 += _T(" ");
		s1 += toSDMM( 1, cursor_lat );
		s1 += _T("   ");
		s1 += toSDMM( 2, cursor_lon );
		if (GetStatusBar())
			SetStatusText(s1, STAT_FIELD_CURSOR_LL);

		double brg;
		double dist;
		DistanceBearingMercator(cursor_lat, cursor_lon, gLat, gLon, &brg, &dist);
		wxString s;
		if (g_bShowMag)
			s.Printf( wxString("%03d°(M)  ", wxConvUTF8), (int)GetTrueOrMag(brg));
		else
			s.Printf( wxString("%03d°  ", wxConvUTF8), (int)GetTrueOrMag(brg));
		s << cc1->FormatDistanceAdaptive( dist );
		if (GetStatusBar())
			SetStatusText(s, STAT_FIELD_CURSOR_BRGRNG);
	}

	// Update the chart database and displayed chart
	bool bnew_view = false;

	// Do the chart update based on the global update period currently set
	// If in COG UP mode, the chart update is handled by COG Update timer
	if (!g_bCourseUp && (0 == m_ChartUpdatePeriod--)) {
		bnew_view = DoChartUpdate();
		m_ChartUpdatePeriod = g_ChartUpdatePeriod;
	}

	// Update the active route, if any
	if (g_pRouteMan->UpdateProgress()) {
		nBlinkerTick++;
		//    This RefreshRect will cause any active routepoint to blink
		if( g_pRouteMan->GetpActiveRoute() ) cc1->RefreshRect( g_blink_rect, false );
	}

	// Possibly save the current configuration
	if (0 == (g_tick % (g_nautosave_interval_seconds))) {
		pConfig->UpdateSettings();
		pConfig->UpdateNavObj();
	}

	// Force own-ship drawing parameters
	cc1->SetOwnShipState(SHIP_NORMAL);

	if (cc1->GetQuiltMode()) {
		double erf = cc1->GetQuiltMaxErrorFactor();
		if (erf > 0.02)
			cc1->SetOwnShipState(SHIP_LOWACCURACY);
	} else {
		if (Current_Ch) {
			if (Current_Ch->GetChart_Error_Factor() > 0.02)
				cc1->SetOwnShipState(SHIP_LOWACCURACY);
		}
	}

	if (!bGPSValid) {
		cc1->SetOwnShipState(SHIP_INVALID);
		if (cc1->m_bFollow)
			cc1->UpdateShips();
	}

	if (bGPSValid != m_last_bGPSValid) {
		cc1->UpdateShips();
		bnew_view = true;                  // force a full Refresh()
		m_last_bGPSValid = bGPSValid;
	}

	// If any PlugIn requested dynamic overlay callbacks, force a full canvas refresh
	// thus, ensuring at least 1 Hz. callback.
	bool brq_dynamic = false;
	if (g_pi_manager) {
		ArrayOfPlugIns *pplugin_array = g_pi_manager->GetPlugInArray();
		for( unsigned int i = 0; i < pplugin_array->GetCount(); i++ ) {
			PlugInContainer *pic = pplugin_array->Item( i );
			if( pic->m_bEnabled && pic->m_bInitState ) {
				if( pic->m_cap_flag & WANTS_DYNAMIC_OPENGL_OVERLAY_CALLBACK ) {
					brq_dynamic = true;
					break;
				}
			}
		}
	}

	if (brq_dynamic) {
		cc1->Refresh();
		bnew_view = true;
	}

	FrameTimer1.Start(TIMER_GFRAME_1, wxTIMER_CONTINUOUS);

	// Invalidate the ChartCanvas window appropriately
	// In non-follow mode, invalidate the rectangles containing the AIS targets and the ownship, etc...
	// In follow mode, if there has already been a full screen refresh, there is no need to check ownship or AIS,
	// since they will be always drawn on the full screen paint.
	if ((!cc1->m_bFollow) || g_bCourseUp) {
		cc1->UpdateShips();
		cc1->UpdateAIS();
		cc1->UpdateAlerts();
	} else {
		if (!bnew_view) { // There has not been a Refresh() yet.....
			cc1->UpdateAIS();
			cc1->UpdateAlerts();
		}
	}

	if (g_pais_query_dialog_active && g_pais_query_dialog_active->IsShown())
		g_pais_query_dialog_active->UpdateText();

	// Refresh AIS target list every 5 seconds to avoid blinking
	if (g_pAISTargetList && (0 == (g_tick % 5)))
		g_pAISTargetList->UpdateAISTargetList();

	//  Pick up any change Toolbar status displays
	UpdateGPSCompassStatusBox();
	UpdateAISTool();

	if (console && console->IsShown()) {
		console->RefreshConsoleData();
	}

	//  This little hack fixes a problem seen with some UniChrome OpenGL drivers
	//  We need a deferred resize to get glDrawPixels() to work right.
	//  So we set a trigger to generate a resize after 5 seconds....
	//  See the "UniChrome" hack elsewhere
	if (m_bdefer_resize) {
		if (0 == (g_tick % 5)) {
			printf("___RESIZE\n");
			SetSize(m_defer_size);
			g_pauimgr->Update();
			m_bdefer_resize = false;
		}
	}
}

double MainFrame::GetTrueOrMag(double a)
{
	if (g_bShowMag) {
		const global::Navigation::Data & nav = global::OCPN::get().nav().get_data();
		if (!wxIsNaN(nav.var))
			return ((a + nav.var) >= 0.0) ? (a + nav.var) : (a + nav.var + 360.0);
		else
			return ((a + g_UserVar) >= 0.0) ? (a + g_UserVar) : (a + g_UserVar + 360.0);
	} else
		return a;
}

void MainFrame::TouchAISActive( void )
{
	if (m_pAISTool) {
		if ((!g_pAIS->IsAISSuppressed()) && (!g_pAIS->IsAISAlertGeneral())) {
			g_nAIS_activity_timer = 5;                // seconds

            wxString iconName = _T("AIS_Normal_Active");
            if( g_pAIS->IsAISAlertGeneral() ) iconName = _T("AIS_AlertGeneral_Active");
            if( g_pAIS->IsAISSuppressed() ) iconName = _T("AIS_Suppressed_Active");
            if( !g_bShowAIS ) iconName = _T("AIS_Disabled");

            if (m_lastAISiconName != iconName) {
                if (g_toolbar) {
                    g_toolbar->SetToolNormalBitmapEx(m_pAISTool, iconName);
                    g_toolbar->Refresh();
                    m_lastAISiconName = iconName;
                }
            }
        }
    }
}

void MainFrame::UpdateAISTool( void )
{
    if(!g_pAIS) return;

    wxString iconName;

    if( m_pAISTool ) {
        bool b_update = false;

        iconName = _T("AIS");
        if( g_pAIS->IsAISSuppressed() )
            iconName = _T("AIS_Suppressed");
        if( g_pAIS->IsAISAlertGeneral() )
            iconName = _T("AIS_AlertGeneral");
        if( !g_bShowAIS )
            iconName = _T("AIS_Disabled");

        //  Manage timeout for AIS activity indicator
        if( g_nAIS_activity_timer ) {
            g_nAIS_activity_timer--;

            if( 0 == g_nAIS_activity_timer ) b_update = true;
            else {
                iconName = _T("AIS_Normal_Active");
                if( g_pAIS->IsAISSuppressed() )
                    iconName = _T("AIS_Suppressed_Active");
                if( g_pAIS->IsAISAlertGeneral() )
                    iconName = _T("AIS_AlertGeneral_Active");
                if( !g_bShowAIS )
                    iconName = _T("AIS_Disabled");
            }
        }

        if( ( m_lastAISiconName != iconName ) ) b_update = true;

        if( b_update && g_toolbar) {
            g_toolbar->SetToolNormalBitmapEx( m_pAISTool, iconName );
            g_toolbar->Refresh();
            m_lastAISiconName = iconName;
        }

    }
}

//    Cause refresh of active Tide/Current data, if displayed
void MainFrame::OnFrameTCTimer(wxTimerEvent &)
{
    if( cc1 ) {
        cc1->SetbTCUpdate( true );
        cc1->Refresh( false );
    }
}

//    Keep and update the Viewport rotation angle according to average COG for COGUP mode
void MainFrame::OnFrameCOGTimer(wxTimerEvent &)
{
//      return;
    FrameCOGTimer.Stop();

    DoCOGSet();

    //    Restart the timer, max frequency is 10 hz.
    if( g_COGAvgSec > 0 )
        FrameCOGTimer.Start( g_COGAvgSec * 1000, wxTIMER_CONTINUOUS );
    else
        FrameCOGTimer.Start( 100, wxTIMER_CONTINUOUS );

}

void MainFrame::DoCOGSet( void )
{
    double old_VPRotate = g_VPRotate;

    if( g_bCourseUp ) g_VPRotate = -g_COGAvg * M_PI / 180.;
    else
        g_VPRotate = 0.;

    if( cc1 ) cc1->SetVPRotation( g_VPRotate );

    if( g_bCourseUp ) {
        bool bnew_chart = DoChartUpdate();

        if( ( bnew_chart ) || ( old_VPRotate != g_VPRotate ) ) if( cc1 ) cc1->ReloadVP();
    }
}

void RenderShadowText( wxDC *pdc, wxFont *pFont, wxString& str, int x, int y )
{
#ifdef DrawText
#undef DrawText
#define FIXIT
#endif

    wxFont oldfont = pdc->GetFont(); // save current font

    pdc->SetFont( *pFont );
    pdc->SetTextForeground( GetGlobalColor( _T("CHGRF") ) );
    pdc->SetBackgroundMode( wxTRANSPARENT );

    pdc->DrawText( str, x, y + 1 );
    pdc->DrawText( str, x, y - 1 );
    pdc->DrawText( str, x + 1, y );
    pdc->DrawText( str, x - 1, y );

    pdc->SetTextForeground( GetGlobalColor( _T("CHBLK") ) );

    pdc->DrawText( str, x, y );

    pdc->SetFont( oldfont );                  // restore last font

}


void MainFrame::UpdateGPSCompassStatusBox( bool b_force_new )
{
    if( !g_FloatingCompassDialog ) return;

    //    Look for change in overlap or positions
    bool b_update = false;
    wxRect tentative_rect;
    wxPoint tentative_pt_in_screen;
    int x_offset;
    int y_offset;
    int size_x, size_y;
    int cc1_edge_comp = 2;

    if( g_FloatingToolbarDialog ) {
        x_offset = g_FloatingCompassDialog->GetXOffset();
        y_offset = g_FloatingCompassDialog->GetYOffset();
        g_FloatingCompassDialog->GetSize(&size_x, &size_y);
        wxSize parent_size = g_FloatingCompassDialog->GetParent()->GetSize();

        // check to see if it would overlap if it was in its home position (upper right)
         tentative_pt_in_screen = g_FloatingCompassDialog->GetParent()->ClientToScreen(
                wxPoint( parent_size.x - size_x - x_offset - cc1_edge_comp, y_offset ) );

        tentative_rect = wxRect( tentative_pt_in_screen.x, tentative_pt_in_screen.y, size_x, size_y );

    }

    //  If the toolbar location has changed, or the proposed compassDialog location has changed
	if( (g_FloatingToolbarDialog->GetScreenRect() != g_last_tb_rect) ||
			(tentative_rect != g_FloatingCompassDialog->GetScreenRect()) ) {

		wxRect tb_rect = g_FloatingToolbarDialog->GetScreenRect();

		//    if they would not intersect, go ahead and move it to the upper right
		//      Else it has to be on lower right
		if( !tb_rect.Intersects( tentative_rect ) ) {
			g_FloatingCompassDialog->Move( tentative_pt_in_screen );
		}
		else {
			wxPoint posn_in_canvas = wxPoint(
					cc1->GetSize().x - size_x - x_offset - cc1_edge_comp,
					cc1->GetSize().y - ( size_y + y_offset + cc1_edge_comp));
			g_FloatingCompassDialog->Move( cc1->ClientToScreen( posn_in_canvas ) );
		}

		b_update = true;

		g_last_tb_rect = tb_rect;

	}

	if( g_FloatingCompassDialog && g_FloatingCompassDialog->IsShown()) {
		g_FloatingCompassDialog->UpdateStatus( b_force_new | b_update );
		g_FloatingCompassDialog->Update();
	}
}

int MainFrame::GetnChartStack( void )
{
	if (pCurrentStack)
		return pCurrentStack->nEntry;
	else
		return 0;
}

//    Application memory footprint management

int MainFrame::GetApplicationMemoryUse( void )
{
    int memsize = -1;
#ifdef __LINUX__

    //    Use a contrived ps command to get the virtual memory size associated with this process
    wxWindow *fWin = wxWindow::FindFocus();

    wxArrayString outputArray;
    wxString cmd(_T("ps --no-headers -o vsize "));
    unsigned long pid = wxGetProcessId();
    wxString cmd1;
    cmd1.Printf(_T("%ld"), pid);
    cmd += cmd1;
    wxExecute(cmd, outputArray);

    if(outputArray.GetCount())
    {
        wxString s = outputArray.Item(0);
        long vtmp;
        if(s.ToLong(&vtmp))
        memsize = vtmp;
    }

    if(fWin)
    fWin->SetFocus();

#endif

#ifdef __WXMSW__
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    unsigned long processID = wxGetProcessId();

    hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );
    if( NULL == hProcess ) return 0;

    if( GetProcessMemoryInfo( hProcess, &pmc, sizeof( pmc ) ) ) {
        memsize = pmc.WorkingSetSize / 1024;
    }

    CloseHandle( hProcess );

#endif

    return memsize;
}

void MainFrame::HandlePianoClick( int selected_index, int selected_dbIndex )
{
    if( !pCurrentStack ) return;
    if( s_ProgDialog ) return;

    if( !cc1->GetQuiltMode() ) {
        if( m_bpersistent_quilt && g_bQuiltEnable ) {
            if( cc1->IsChartQuiltableRef( selected_dbIndex ) ) {
                ToggleQuiltMode();
                SelectQuiltRefdbChart( selected_dbIndex );
                m_bpersistent_quilt = false;
            } else {
                SelectChartFromStack( selected_index );
            }
        } else {
            SelectChartFromStack( selected_index );
            g_sticky_chart = selected_dbIndex;
        }
    } else {
        if( cc1->IsChartQuiltableRef( selected_dbIndex ) ) SelectQuiltRefdbChart(
                selected_dbIndex );
        else {
            ToggleQuiltMode();
            SelectdbChart( selected_dbIndex );
            m_bpersistent_quilt = true;
        }
    }

    cc1->SetQuiltChartHiLiteIndex( -1 );
    cc1->HideChartInfoWindow();
    DoChartUpdate();
    cc1->ReloadVP();                  // Pick up the new selections
}

void MainFrame::HandlePianoRClick( int x, int y, int selected_index, int selected_dbIndex )
{
    if( !pCurrentStack ) return;
    if( s_ProgDialog ) return;

    PianoPopupMenu( x, y, selected_index, selected_dbIndex );
    UpdateControlBar();

    cc1->SetQuiltChartHiLiteIndex( -1 );

}

void MainFrame::HandlePianoRollover( int selected_index, int selected_dbIndex )
{
    if( !cc1 ) return;
    if( !pCurrentStack ) return;
    if( s_ProgDialog ) return;

    int sx, sy;
    stats->GetPosition( &sx, &sy );
    wxPoint key_location = stats->pPiano->GetKeyOrigin( selected_index );
    wxPoint rolloverPos = stats->GetParent()->ScreenToClient( wxPoint( sx, sy ) );
    rolloverPos.y -= 3;
    rolloverPos.x += key_location.x;

    if( !cc1->GetQuiltMode() ) {
        SetChartThumbnail( selected_index );
        cc1->ShowChartInfoWindow( key_location.x, sy + key_location.y, selected_dbIndex );
    } else {
        std::vector<int> piano_chart_index_array = cc1->GetQuiltExtendedStackdbIndexArray();

        if( ( pCurrentStack->nEntry > 1 ) || ( piano_chart_index_array.size() > 1 ) ) {
            cc1->ShowChartInfoWindow( rolloverPos.x, rolloverPos.y, selected_dbIndex );
            cc1->SetQuiltChartHiLiteIndex( selected_dbIndex );

            cc1->ReloadVP( false );         // no VP adjustment allowed
        } else if( pCurrentStack->nEntry == 1 ) {
            const ChartTableEntry &cte = ChartData->GetChartTableEntry(
                    pCurrentStack->GetDBIndex( 0 ) );
            if( CHART_TYPE_CM93COMP != cte.GetChartType() ) {
                cc1->ShowChartInfoWindow( rolloverPos.x, rolloverPos.y, selected_dbIndex );
                cc1->ReloadVP( false );
            } else if( ( -1 == selected_index ) && ( -1 == selected_dbIndex ) ) {
                cc1->ShowChartInfoWindow( rolloverPos.x, rolloverPos.y, selected_dbIndex );
            }
        }
        SetChartThumbnail( -1 );        // hide all thumbs in quilt mode
    }
}

void MainFrame::HandlePianoRolloverIcon( int selected_index, int selected_dbIndex )
{
    if( !cc1 ) return;

    if( !cc1->GetQuiltMode() ) {
        SetChartThumbnail( selected_index );
    } else {
        cc1->SetQuiltChartHiLiteIndex( selected_dbIndex );
    }
}

double MainFrame::GetBestVPScale( ChartBase *pchart )
{
    if( pchart ) {
        double proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / cc1->GetVPScale();

        if( ( g_bPreserveScaleOnX ) || ( CHART_TYPE_CM93COMP == pchart->GetChartType() ) ) {
            double new_scale_ppm = pchart->GetNearestPreferredScalePPM( cc1->GetVPScale() );
            proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / new_scale_ppm;
        } else {
            //  This logic will bring the new chart onscreen at roughly twice the true paper scale equivalent.
            proposed_scale_onscreen = pchart->GetNativeScale() / 2;
            double equivalent_vp_scale = cc1->GetCanvasScaleFactor() / proposed_scale_onscreen;
            double new_scale_ppm = pchart->GetNearestPreferredScalePPM( equivalent_vp_scale );
            proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / new_scale_ppm;
        }

        proposed_scale_onscreen =
                wxMin(proposed_scale_onscreen, pchart->GetNormalScaleMax(cc1->GetCanvasScaleFactor(), cc1->GetCanvasWidth()));
        proposed_scale_onscreen =
                wxMax(proposed_scale_onscreen, pchart->GetNormalScaleMin(cc1->GetCanvasScaleFactor(), g_b_overzoom_x));

        return cc1->GetCanvasScaleFactor() / proposed_scale_onscreen;
    } else
        return 1.0;
}

void MainFrame::SelectQuiltRefChart( int selected_index )
{
    std::vector<int> piano_chart_index_array = cc1->GetQuiltExtendedStackdbIndexArray();
    int current_db_index = piano_chart_index_array[selected_index];

    SelectQuiltRefdbChart( current_db_index );
}

void MainFrame::SelectQuiltRefdbChart( int db_index )
{
    if( pCurrentStack ) pCurrentStack->SetCurrentEntryFromdbIndex( db_index );

    cc1->SetQuiltRefChart( db_index );

    ChartBase *pc = ChartData->OpenChartFromDB( db_index, FULL_INIT );
    if( pc ) {
        double best_scale = GetBestVPScale( pc );
        cc1->SetVPScale( best_scale );
    }

}

void MainFrame::SelectChartFromStack( int index, bool bDir, ChartTypeEnum New_Type,
        ChartFamilyEnum New_Family )
{
    if( !pCurrentStack )
        return;

    if( index < pCurrentStack->nEntry ) {
//      Open the new chart
        ChartBase *pTentative_Chart;
        pTentative_Chart = ChartData->OpenStackChartConditional( pCurrentStack, index, bDir,
                New_Type, New_Family );

        if( pTentative_Chart ) {
            if( Current_Ch ) Current_Ch->Deactivate();

            Current_Ch = pTentative_Chart;
            Current_Ch->Activate();

            pCurrentStack->CurrentStackEntry = ChartData->GetStackEntry( pCurrentStack,
                    Current_Ch->GetFullPath() );
        } else
            SetChartThumbnail( -1 );   // need to reset thumbnail on failed chart open

//      Setup the view
        double zLat, zLon;
        if( cc1->m_bFollow ) {
            zLat = gLat;
            zLon = gLon;
        } else {
            zLat = vLat;
            zLon = vLon;
        }

        double best_scale = GetBestVPScale( Current_Ch );

        cc1->SetViewPoint( zLat, zLon, best_scale, Current_Ch->GetChartSkew() * M_PI / 180.,
                cc1->GetVPRotation() );

        SetChartUpdatePeriod( cc1->GetVP() );

        UpdateGPSCompassStatusBox();           // Pick up the rotation

    }

    //          Refresh the Piano Bar
    if( stats ) {
        std::vector<int> piano_active_chart_index_array;
        piano_active_chart_index_array.push_back( pCurrentStack->GetCurrentEntrydbIndex());
        stats->pPiano->SetActiveKeyArray(piano_active_chart_index_array);

        stats->Refresh( true );
    }
}

void MainFrame::SelectdbChart( int dbindex )
{
    if( !pCurrentStack )
        return;

    if( dbindex >= 0 ) {
//      Open the new chart
        ChartBase *pTentative_Chart;
        pTentative_Chart = ChartData->OpenChartFromDB( dbindex, FULL_INIT );

        if( pTentative_Chart ) {
            if( Current_Ch ) Current_Ch->Deactivate();

            Current_Ch = pTentative_Chart;
            Current_Ch->Activate();

            pCurrentStack->CurrentStackEntry = ChartData->GetStackEntry( pCurrentStack,
                    Current_Ch->GetFullPath() );
        } else
            SetChartThumbnail( -1 );       // need to reset thumbnail on failed chart open

//      Setup the view
        double zLat, zLon;
        if( cc1->m_bFollow ) {
            zLat = gLat;
            zLon = gLon;
        } else {
            zLat = vLat;
            zLon = vLon;
        }

        double best_scale = GetBestVPScale( Current_Ch );

        cc1->SetViewPoint( zLat, zLon, best_scale, Current_Ch->GetChartSkew() * M_PI / 180.,
                cc1->GetVPRotation() );

        SetChartUpdatePeriod( cc1->GetVP() );

        UpdateGPSCompassStatusBox();           // Pick up the rotation

    }

    //          Refresh the Piano Bar
    if( stats ) {
        std::vector<int> piano_active_chart_index_array;
        piano_active_chart_index_array.push_back(pCurrentStack->GetCurrentEntrydbIndex());
        stats->pPiano->SetActiveKeyArray(piano_active_chart_index_array);

        stats->Refresh( true );
    }
}

void MainFrame::SetChartUpdatePeriod( ViewPort &vp )
{
	//    Set the chart update period based upon chart skew and skew compensator

	g_ChartUpdatePeriod = 1;            // General default

	if (!vp.b_quilt) {
		if (g_bskew_comp && (fabs(vp.skew)) > 0.01)
			g_ChartUpdatePeriod = g_SkewCompUpdatePeriod;
	}

    m_ChartUpdatePeriod = g_ChartUpdatePeriod;
}

void MainFrame::SetChartThumbnail(int index)
{
	if (bDBUpdateInProgress)
		return;
	if (NULL == pCurrentStack)
		return;
	if (NULL == pthumbwin)
		return;
	if (NULL == cc1)
		return;

	if (index == -1) {
		wxRect thumb_rect_in_parent = pthumbwin->GetRect();
		pthumbwin->pThumbChart = NULL;
		pthumbwin->Show( false );
		cc1->RefreshRect( thumb_rect_in_parent, FALSE );
		return;
	}

	if (index >= pCurrentStack->nEntry)
		return;

	if ((ChartData->GetCSChartType( pCurrentStack, index) == CHART_TYPE_KAP)
			|| (ChartData->GetCSChartType( pCurrentStack, index) == CHART_TYPE_GEO)
			|| (ChartData->GetCSChartType( pCurrentStack, index) == CHART_TYPE_PLUGIN)) {
		ChartBase *new_pThumbChart = ChartData->OpenChartFromStack( pCurrentStack, index );
		if (new_pThumbChart) { // chart opened ok
			ThumbData *pTD = new_pThumbChart->GetThumbData( 150, 150, gLat, gLon );
			if (pTD) {
				pthumbwin->pThumbChart = new_pThumbChart;

				pthumbwin->Resize();
				pthumbwin->Show(true);
				pthumbwin->Refresh(false);
				pthumbwin->Move(wxPoint(4, 4));

				// Simplistic overlap avoidance works only when toolbar is at top of screen.
				if (g_FloatingToolbarDialog) {
					if (g_FloatingToolbarDialog->GetScreenRect().Intersects( pthumbwin->GetScreenRect())) {
						pthumbwin->Move( wxPoint( 4, g_FloatingToolbarDialog->GetSize().y + 4 ) );
					}
				}
			} else {
				wxLogMessage(_T("    chart1.cpp:SetChartThumbnail...Could not create thumbnail"));
				pthumbwin->pThumbChart = NULL;
				pthumbwin->Show( false );
				cc1->Refresh( FALSE );
			}
		} else {                          // some problem opening chart
			wxString fp = ChartData->GetFullPath( pCurrentStack, index );
			fp.Prepend(_T("    chart1.cpp:SetChartThumbnail...Could not open chart "));
			wxLogMessage(fp);
			pthumbwin->pThumbChart = NULL;
			pthumbwin->Show(false);
			cc1->Refresh(false);
		}
	} else {
		ChartBase * new_pThumbChart = ChartData->OpenChartFromStack(pCurrentStack, index, THUMB_ONLY);
		pthumbwin->pThumbChart = new_pThumbChart;
		if (new_pThumbChart) {
			ThumbData * pTD = new_pThumbChart->GetThumbData(200, 200, gLat, gLon);
			if (pTD) {
				pthumbwin->Resize();
				pthumbwin->Show(true);
				pthumbwin->Refresh(true);
			} else {
				pthumbwin->Show(false);
			}
			cc1->Refresh(false);
		}
	}
}

void MainFrame::UpdateControlBar( void )
{
	if (!cc1)
		return;

	if (!stats)
		return;

	if (!pCurrentStack)
		return;

	std::vector<int> piano_chart_index_array;
	std::vector<int> empty_piano_chart_index_array;

	if( cc1->GetQuiltMode() ) {
		piano_chart_index_array = cc1->GetQuiltExtendedStackdbIndexArray();
		stats->pPiano->SetKeyArray(piano_chart_index_array);

		std::vector<int> piano_active_chart_index_array = cc1->GetQuiltCandidatedbIndexArray();
		stats->pPiano->SetActiveKeyArray(piano_active_chart_index_array);

		std::vector<int> piano_eclipsed_chart_index_array = cc1->GetQuiltEclipsedStackdbIndexArray();
		stats->pPiano->SetSubliteIndexArray(piano_eclipsed_chart_index_array);

		stats->pPiano->SetNoshowIndexArray(g_quilt_noshow_index_array);

	} else {
		piano_chart_index_array = ChartData->GetCSArray( pCurrentStack );
		stats->pPiano->SetKeyArray(piano_chart_index_array);

		std::vector<int> piano_active_chart_index_array;
		piano_active_chart_index_array.push_back(pCurrentStack->GetCurrentEntrydbIndex() );
		stats->pPiano->SetActiveKeyArray( piano_active_chart_index_array );
	}

	//    Set up the TMerc and Skew arrays
    std::vector<int> piano_skew_chart_index_array;
    std::vector<int> piano_tmerc_chart_index_array;
    std::vector<int> piano_poly_chart_index_array;

    for( unsigned int ino = 0; ino < piano_chart_index_array.size(); ino++ ) {
        const ChartTableEntry &ctei = ChartData->GetChartTableEntry(piano_chart_index_array[ino]);
        double skew_norm = ctei.GetChartSkew();
        if (skew_norm > 180.0)
			skew_norm -= 360.0;

        if (ctei.GetChartProjectionType() == PROJECTION_TRANSVERSE_MERCATOR)
			piano_tmerc_chart_index_array.push_back(piano_chart_index_array[ino]);
        else // Polyconic skewed charts should show as skewed
            if( ctei.GetChartProjectionType() == PROJECTION_POLYCONIC ) {
                if( fabs( skew_norm ) > 1.0)
					piano_skew_chart_index_array.push_back(piano_chart_index_array[ino]);
                else
                    piano_poly_chart_index_array.push_back(piano_chart_index_array[ino]);
            } else
                if( fabs( skew_norm ) > 1.0)
					piano_skew_chart_index_array.push_back(piano_chart_index_array[ino]);

    }
    stats->pPiano->SetSkewIndexArray(piano_skew_chart_index_array);
    stats->pPiano->SetTmercIndexArray(piano_tmerc_chart_index_array);
    stats->pPiano->SetPolyIndexArray(piano_poly_chart_index_array);

    stats->FormatStat();
    stats->Refresh( true );
}

//----------------------------------------------------------------------------------
//      DoChartUpdate
//      Create a chartstack based on current lat/lon.
//      Update Current_Ch, using either current chart, if still in stack, or
//      smallest scale new chart in stack if not.
//      Return true if a Refresh(false) was called within.
//----------------------------------------------------------------------------------
bool MainFrame::DoChartUpdate( void )
{

    double tLat, tLon;           // Chart Stack location
    double vpLat, vpLon;         // ViewPort location

    bool bNewChart = false;
    bool bNewView = false;

    bool bNewPiano = false;
    bool bOpenSpecified;
    ChartStack LastStack;
    ChartBase *pLast_Ch;

    ChartStack WorkStack;

    if( !cc1 ) return false;
    if( bDBUpdateInProgress ) return false;
    if( !ChartData ) return false;

    int last_nEntry = -1;
    if( pCurrentStack ) last_nEntry = pCurrentStack->nEntry;

    //    Startup case:
    //    Quilting is enabled, but the last chart seen was not quiltable
    //    In this case, drop to single chart mode, set persistence flag,
    //    And open the specified chart
    if( bFirstAuto && ( g_restore_dbindex >= 0 ) ) {
        if( cc1->GetQuiltMode() ) {
            if( !cc1->IsChartQuiltableRef( g_restore_dbindex ) ) {
                ToggleQuiltMode();
                m_bpersistent_quilt = true;
                Current_Ch = NULL;
            }
        }
    }

    //      If in auto-follow mode, use the current glat,glon to build chart stack.
    //      Otherwise, use vLat, vLon gotten from click on chart canvas, or other means

    if( cc1->m_bFollow == true ) {
        tLat = gLat;
        tLon = gLon;
        vpLat = gLat;
        vpLon = gLon;

        // on lookahead mode, adjust the vp center point
        if (cc1 && global::OCPN::get().gui().view().lookahead_mode) {
            double angle = g_COGAvg + ( cc1->GetVPRotation() * 180.0  / M_PI);
            double pixel_deltay = fabs(cos(angle * M_PI / 180.0)) * cc1->GetCanvasHeight() / 4;
            double pixel_deltax = fabs(sin(angle * M_PI / 180.0)) * cc1->GetCanvasWidth() / 4;
            double pixel_delta_tent = sqrt((pixel_deltay * pixel_deltay) + (pixel_deltax * pixel_deltax));
            double pixel_delta = 0;

            //    The idea here is to cancel the effect of LookAhead for slow speed ove ground, to avoid
            //    jumping of the vp center point during slow maneuvering, or at anchor....
			const global::Navigation::Data & nav = global::OCPN::get().nav().get_data();
            if (!wxIsNaN(nav.sog)) {
                if (nav.sog < 1.0)
					pixel_delta = 0.0;
                else
                    if (nav.sog >= 3.0)
						pixel_delta = pixel_delta_tent;
                    else
                        pixel_delta = pixel_delta_tent * (nav.sog - 1.0) / 2.0;
            }

            double meters_to_shift = cos(gLat * M_PI / 180.0) * pixel_delta / cc1->GetVPScale();
            double dir_to_shift = g_COGAvg;
            ll_gc_ll( gLat, gLon, dir_to_shift, meters_to_shift / 1852.0, &vpLat, &vpLon );
        }
    } else {
        tLat = vLat;
        tLon = vLon;
        vpLat = vLat;
        vpLon = vLon;
    }

    if( cc1->GetQuiltMode() ) {
        int current_db_index = -1;
        if( pCurrentStack ) current_db_index = pCurrentStack->GetCurrentEntrydbIndex(); // capture the currently selected Ref chart dbIndex
        else
            pCurrentStack = new ChartStack;

        ChartData->BuildChartStack( pCurrentStack, tLat, tLon );
        pCurrentStack->SetCurrentEntryFromdbIndex( current_db_index );

        if( bFirstAuto ) {
            double proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / cc1->GetVPScale(); // as set from config load

            int initial_db_index = g_restore_dbindex;
            if( initial_db_index < 0 ) {
                if( pCurrentStack->nEntry ) {
                    if( ( g_restore_stackindex < pCurrentStack->nEntry )
                            && ( g_restore_stackindex >= 0 ) ) initial_db_index =
                            pCurrentStack->GetDBIndex( g_restore_stackindex );
                    else
                        initial_db_index = pCurrentStack->GetDBIndex( pCurrentStack->nEntry - 1 );
                } else
                    initial_db_index = 0;
            }

            if( pCurrentStack->nEntry ) {

                int initial_type = ChartData->GetDBChartType( initial_db_index );

                //    Check to see if the target new chart is quiltable as a reference chart

                if( !cc1->IsChartQuiltableRef( initial_db_index ) ) {
                    // If it is not quiltable, then walk the stack up looking for a satisfactory chart
                    // i.e. one that is quiltable and of the same type
                    int stack_index = g_restore_stackindex;

                    while( ( stack_index < pCurrentStack->nEntry - 1 ) && ( stack_index >= 0 ) ) {
                        int test_db_index = pCurrentStack->GetDBIndex( stack_index );
                        if( cc1->IsChartQuiltableRef( test_db_index )
                                && ( initial_type == ChartData->GetDBChartType( initial_db_index ) ) ) {
                            initial_db_index = test_db_index;
                            break;
                        }
                        stack_index++;
                    }
                }

                cc1->SetQuiltRefChart( initial_db_index );
                pCurrentStack->SetCurrentEntryFromdbIndex( initial_db_index );

                // Try to bound the inital Viewport scale to something reasonable for the selected reference chart
                if( ChartData ) {
                    ChartBase *pc = ChartData->OpenChartFromDB( initial_db_index, FULL_INIT );
                    if( pc ) {
                        proposed_scale_onscreen =
                                wxMin(proposed_scale_onscreen, pc->GetNormalScaleMax(cc1->GetCanvasScaleFactor(), cc1->GetCanvasWidth()));
                        proposed_scale_onscreen =
                                wxMax(proposed_scale_onscreen, pc->GetNormalScaleMin(cc1->GetCanvasScaleFactor(), g_b_overzoom_x));
                    }
                }

            }

            bNewView |= cc1->SetViewPoint( vpLat, vpLon,
                    cc1->GetCanvasScaleFactor() / proposed_scale_onscreen, 0,
                    cc1->GetVPRotation() );

        }

        bNewView |= cc1->SetViewPoint( vpLat, vpLon, cc1->GetVPScale(), 0, cc1->GetVPRotation() );

        goto update_finish;

    }

    //  Single Chart Mode from here....
    pLast_Ch = Current_Ch;
    ChartTypeEnum new_open_type;
    ChartFamilyEnum new_open_family;
    if( pLast_Ch ) {
        new_open_type = pLast_Ch->GetChartType();
        new_open_family = pLast_Ch->GetChartFamily();
    } else {
        new_open_type = CHART_TYPE_KAP;
        new_open_family = CHART_FAMILY_RASTER;
    }

    bOpenSpecified = bFirstAuto;
    bAutoOpen = true;                             // debugging

    //  Make sure the target stack is valid
    if( NULL == pCurrentStack ) pCurrentStack = new ChartStack;

    // Build a chart stack based on tLat, tLon
    if( 0 == ChartData->BuildChartStack( &WorkStack, tLat, tLon, g_sticky_chart ) )       // Bogus Lat, Lon?
            {
        if( NULL == pDummyChart ) {
            pDummyChart = new ChartDummy;
            bNewChart = true;
        }

        if( Current_Ch ) if( Current_Ch->GetChartType() != CHART_TYPE_DUMMY ) bNewChart = true;

        Current_Ch = pDummyChart;

//    If the current viewpoint is invalid, set the default scale to something reasonable.
        double set_scale = cc1->GetVPScale();
        if( !cc1->GetVP().IsValid() ) set_scale = 1. / 200000.;

        bNewView |= cc1->SetViewPoint( tLat, tLon, set_scale, 0, cc1->GetVPRotation() );

        //      If the chart stack has just changed, there is new status
        if( !ChartData->EqualStacks( &WorkStack, pCurrentStack ) ) {
            bNewPiano = true;
            bNewChart = true;
        }

        //      Copy the new (by definition empty) stack into the target stack
        ChartData->CopyStack( pCurrentStack, &WorkStack );

        goto update_finish;
    }

    //              Check to see if Chart Stack has changed
    if( !ChartData->EqualStacks( &WorkStack, pCurrentStack ) ) {
        //      New chart stack, so...
        bNewPiano = true;

        //      Save a copy of the current stack
        ChartData->CopyStack( &LastStack, pCurrentStack );

        //      Copy the new stack into the target stack
        ChartData->CopyStack( pCurrentStack, &WorkStack );

        //  Is Current Chart in new stack?

        int tEntry = -1;
        if( NULL != Current_Ch )                                  // this handles startup case
            tEntry = ChartData->GetStackEntry( pCurrentStack, Current_Ch->GetFullPath() );

        if( tEntry != -1 ) {                // Current_Ch is in the new stack
            pCurrentStack->CurrentStackEntry = tEntry;
            bNewChart = false;
        }

        else                           // Current_Ch is NOT in new stack
        {                                       // So, need to open a new chart
                                                //      Find the largest scale raster chart that opens OK

            ChartBase *pProposed = NULL;

            if( bAutoOpen ) {
                bool search_direction = false;        // default is to search from lowest to highest
                int start_index = 0;

                //    A special case:  If panning at high scale, open largest scale chart first
                if( ( LastStack.CurrentStackEntry == LastStack.nEntry - 1 )
                        || ( LastStack.nEntry == 0 ) ) {
                    search_direction = true;
                    start_index = pCurrentStack->nEntry - 1;
                }

                //    Another special case, open specified index on program start
                if( bOpenSpecified ) {
                    search_direction = false;
                    start_index = g_restore_stackindex;
                    if( ( start_index < 0 ) | ( start_index >= pCurrentStack->nEntry ) ) start_index =
                            0;
                    new_open_type = CHART_TYPE_DONTCARE;
                }

                pProposed = ChartData->OpenStackChartConditional( pCurrentStack, start_index,
                        search_direction, new_open_type, new_open_family );

//    Try to open other types/families of chart in some priority
                if( NULL == pProposed ) pProposed = ChartData->OpenStackChartConditional(
                        pCurrentStack, start_index, search_direction, CHART_TYPE_CM93COMP,
                        CHART_FAMILY_VECTOR );

                if( NULL == pProposed ) pProposed = ChartData->OpenStackChartConditional(
                        pCurrentStack, start_index, search_direction, CHART_TYPE_CM93COMP,
                        CHART_FAMILY_RASTER );

                bNewChart = true;

            }     // bAutoOpen

            else
                pProposed = NULL;

//  If no go, then
//  Open a Dummy Chart
            if( NULL == pProposed ) {
                if( NULL == pDummyChart ) {
                    pDummyChart = new ChartDummy;
                    bNewChart = true;
                }

                if( pLast_Ch ) if( pLast_Ch->GetChartType() != CHART_TYPE_DUMMY ) bNewChart = true;

                pProposed = pDummyChart;
            }

// Arriving here, pProposed points to an opened chart, or NULL.
            if( Current_Ch ) Current_Ch->Deactivate();
            Current_Ch = pProposed;

            if( Current_Ch ) {
                Current_Ch->Activate();
                pCurrentStack->CurrentStackEntry = ChartData->GetStackEntry( pCurrentStack,
                        Current_Ch->GetFullPath() );
            }
        }   // need new chart

// Arriving here, Current_Ch is opened and OK, or NULL
        if( NULL != Current_Ch ) {

//      Setup the view using the current scale
            double set_scale = cc1->GetVPScale();

//    If the current viewpoint is invalid, set the default scale to something reasonable.
            if( !cc1->GetVP().IsValid() )
                set_scale = 1. / 200000.;
            else {                                    // otherwise, match scale if elected.
                double proposed_scale_onscreen;

                if( cc1->m_bFollow ) {          // autoset the scale only if in autofollow
                    double new_scale_ppm = Current_Ch->GetNearestPreferredScalePPM( cc1->GetVPScale() );
                    proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / new_scale_ppm;
                }
                else
                    proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / set_scale;


                //  This logic will bring a new chart onscreen at roughly twice the true paper scale equivalent.
                //  Note that first chart opened on application startup (bOpenSpecified = true) will open at the config saved scale
                if( bNewChart && !g_bPreserveScaleOnX && !bOpenSpecified ) {
                    proposed_scale_onscreen = Current_Ch->GetNativeScale() / 2;
                    double equivalent_vp_scale = cc1->GetCanvasScaleFactor()
                            / proposed_scale_onscreen;
                    double new_scale_ppm = Current_Ch->GetNearestPreferredScalePPM(
                            equivalent_vp_scale );
                    proposed_scale_onscreen = cc1->GetCanvasScaleFactor() / new_scale_ppm;
                }

                if( cc1->m_bFollow ) {     // bounds-check the scale only if in autofollow
                    proposed_scale_onscreen =
                        wxMin(proposed_scale_onscreen, Current_Ch->GetNormalScaleMax(cc1->GetCanvasScaleFactor(), cc1->GetCanvasWidth()));
                    proposed_scale_onscreen =
                        wxMax(proposed_scale_onscreen, Current_Ch->GetNormalScaleMin(cc1->GetCanvasScaleFactor(), g_b_overzoom_x));
                }

                set_scale = cc1->GetCanvasScaleFactor() / proposed_scale_onscreen;
            }

            bNewView |= cc1->SetViewPoint( vpLat, vpLon, set_scale,
                    Current_Ch->GetChartSkew() * M_PI / 180., cc1->GetVPRotation() );

        }
    }         // new stack

    else                                                                 // No change in Chart Stack
    {
        if( ( cc1->m_bFollow ) && Current_Ch ) bNewView |= cc1->SetViewPoint( vpLat, vpLon,
                cc1->GetVPScale(), Current_Ch->GetChartSkew() * M_PI / 180., cc1->GetVPRotation() );
    }

    update_finish:

    //    Ask for a new tool bar if the stack is going to or coming from only one entry.
    if( pCurrentStack
            && ( ( ( pCurrentStack->nEntry <= 1 ) && m_toolbar_scale_tools_shown )
                    || ( ( pCurrentStack->nEntry > 1 ) && !m_toolbar_scale_tools_shown ) ) ) if( !bFirstAuto ) RequestNewToolbar();

    if( bNewPiano ) UpdateControlBar();

    //  Update the ownship position on thumbnail chart, if shown
    if( pthumbwin && pthumbwin->IsShown() ) {
        if( pthumbwin->pThumbChart ){
            if( pthumbwin->pThumbChart->UpdateThumbData( gLat, gLon ) )
                pthumbwin->Refresh( TRUE );
        }
    }

    bFirstAuto = false;                           // Auto open on program start

    //  If we need a Refresh(), do it here...
    //  But don't duplicate a Refresh() done by SetViewPoint()
    if( bNewChart && !bNewView ) cc1->Refresh( false );

    return bNewChart | bNewView;
}

void MainFrame::MouseEvent( wxMouseEvent& event )
{
	// FIXME: has no effect?
    int x;
    int y;
    event.GetPosition( &x, &y );
}

void MainFrame::RemoveChartFromQuilt(int dbIndex)
{
	// Remove the item from the list (if it appears) to avoid multiple addition
	// FIXME: why remove if the same index will be added to the list anyway? order of insertion?

	std::vector<int>::iterator i = find(
			g_quilt_noshow_index_array.begin(),
			g_quilt_noshow_index_array.end(),
			dbIndex);

	if (i != g_quilt_noshow_index_array.end())
		g_quilt_noshow_index_array.erase(i);

	g_quilt_noshow_index_array.push_back(dbIndex);
}

//          Piano window Popup Menu Handlers and friends

static int menu_selected_dbIndex;
static int menu_selected_index;

void MainFrame::PianoPopupMenu(int, int, int selected_index, int selected_dbIndex)
{
    if( !pCurrentStack )
        return;

    //    No context menu if quilting is disabled
    if( !cc1->GetQuiltMode() ) return;

    menu_selected_dbIndex = selected_dbIndex;
    menu_selected_index = selected_index;

    wxMenu *pctx_menu = new wxMenu();

    //    Search the no-show array
    bool b_is_in_noshow = g_quilt_noshow_index_array.end() != find(
			g_quilt_noshow_index_array.begin(),
			g_quilt_noshow_index_array.end(),
			selected_dbIndex);

    if( b_is_in_noshow ) {
        pctx_menu->Append( ID_PIANO_ENABLE_QUILT_CHART, _("Show This Chart") );
        Connect( ID_PIANO_ENABLE_QUILT_CHART, wxEVT_COMMAND_MENU_SELECTED,
                wxCommandEventHandler(MainFrame::OnPianoMenuEnableChart) );
    } else
        if( pCurrentStack->nEntry > 1 ) {
            pctx_menu->Append( ID_PIANO_DISABLE_QUILT_CHART, _("Hide This Chart") );
            Connect( ID_PIANO_DISABLE_QUILT_CHART, wxEVT_COMMAND_MENU_SELECTED,
                    wxCommandEventHandler(MainFrame::OnPianoMenuDisableChart) );
        }

    int sx, sy;
    stats->GetPosition( &sx, &sy );
    wxPoint pos = stats->GetParent()->ScreenToClient( wxPoint( sx, sy ) );
    wxPoint key_location = stats->pPiano->GetKeyOrigin( selected_index );
    pos.x += key_location.x;
    pos.y -= 30;

	// Invoke the drop-down menu
    if( pctx_menu->GetMenuItems().GetCount() )
		PopupMenu( pctx_menu, pos );

    cc1->HideChartInfoWindow();
    stats->pPiano->ResetRollover();

    cc1->SetQuiltChartHiLiteIndex( -1 );

    cc1->ReloadVP();

    delete pctx_menu;
}

void MainFrame::OnPianoMenuEnableChart(wxCommandEvent &)
{
	std::vector<int>::iterator i = find(
			g_quilt_noshow_index_array.begin(),
			g_quilt_noshow_index_array.end(),
			menu_selected_dbIndex);

	if (i != g_quilt_noshow_index_array.end())
		g_quilt_noshow_index_array.erase(i);
}

void MainFrame::OnPianoMenuDisableChart(wxCommandEvent &)
{
    if( !pCurrentStack )
        return;

    RemoveChartFromQuilt( menu_selected_dbIndex );

//      It could happen that the chart being disabled is the reference chart....
    if( menu_selected_dbIndex == cc1->GetQuiltRefChartdbIndex() ) {
        int type = ChartData->GetDBChartType( menu_selected_dbIndex );

        int i = menu_selected_index + 1;          // select next smaller scale chart
        bool b_success = false;
        while( i < pCurrentStack->nEntry - 1 ) {
            int dbIndex = pCurrentStack->GetDBIndex( i );
            if( type == ChartData->GetDBChartType( dbIndex ) ) {
                SelectQuiltRefChart( i );
                b_success = true;
                break;
            }
            i++;
        }

        //    If that did not work, try to select the next larger scale compatible chart
        if( !b_success ) {
            i = menu_selected_index - 1;
            while( i > 0 ) {
                int dbIndex = pCurrentStack->GetDBIndex( i );
                if( type == ChartData->GetDBChartType( dbIndex ) ) {
                    SelectQuiltRefChart( i );
                    b_success = true;
                    break;
                }
                i--;
            }
        }
    }
}

void MainFrame::DoPrint( void )
{
    if( NULL == g_printData ) {
        g_printData = new wxPrintData;
        g_printData->SetOrientation( wxLANDSCAPE );
        g_pageSetupData = new wxPageSetupDialogData;
    }

    wxPrintDialogData printDialogData( *g_printData );
    printDialogData.EnablePageNumbers( false );

    wxPrinter printer( &printDialogData );

    MyPrintout printout( _("Chart Print") );
    if( !printer.Print( this, &printout, true ) ) {
        if( wxPrinter::GetLastError() == wxPRINTER_ERROR ) OCPNMessageBox(NULL,
                _("There was a problem printing.\nPerhaps your current printer is not set correctly?"),
                _T("OpenCPN"), wxOK );
    } else {
        ( *g_printData ) = printer.GetPrintDialogData().GetPrintData();
    }
}

void MainFrame::OnEvtPlugInMessage( OCPN_MsgEvent & event )
{
    wxString message_ID = event.GetID();
    wxString message_JSONText = event.GetJSONText();

    //  We are free to use or ignore any or all of the PlugIn messages flying thru this pipe tee.

    //  We can possibly use the estimated magnetic variation if WMM_pi is present and active
    //  and we have no other source of Variation
    if(!g_bVAR_Rx)
    {
        if(message_ID == _T("WMM_VARIATION_BOAT"))
        {

        // construct the JSON root object
            wxJSONValue  root;
        // construct a JSON parser
            wxJSONReader reader;

        // now read the JSON text and store it in the 'root' structure
        // check for errors before retreiving values...
            int numErrors = reader.Parse( message_JSONText, &root );
            if ( numErrors > 0 )  {
                return;
            }

            // get the DECL value from the JSON message
            wxString decl = root[_T("Decl")].AsString();
            double decl_val;
            decl.ToDouble(&decl_val);

			global::OCPN::get().nav().set_magn_var(decl_val);
        }
    }

    if(message_ID == _T("OCPN_TRACK_REQUEST"))
    {
        wxJSONValue  root;
        wxJSONReader reader;
        wxString trk_id = wxEmptyString;

        int numErrors = reader.Parse( message_JSONText, &root );
        if ( numErrors > 0 )
            return;

        if(root.HasMember(_T("Track_ID")))
            trk_id = root[_T("Track_ID")].AsString();

        for (RouteList::iterator it = pRouteList->begin(); it != pRouteList->end(); ++it) {
            wxString name = wxEmptyString; // FIXME: why? 'name' is never used
            if ((*it)->IsTrack() && (*it)->m_GUID == trk_id) {
                name = (*it)->m_RouteNameString;
                if (name.IsEmpty()) {
                    RoutePoint *rp = (*it)->GetPoint( 1 );
                    if (rp && rp->GetCreateTime().IsValid())
                        name = rp->GetCreateTime().FormatISODate() + _T(" ") + rp->GetCreateTime().FormatISOTime();
                    else
                        name = _("(Unnamed Track)");
                }

				// To avoid memory problems send a single trackpoint. It's up to the plugin to collect the data.
                int i = 1;
				wxJSONValue v;
                for(RoutePointList::iterator itp = (*it)->pRoutePointList->begin(); itp != (*it)->pRoutePointList->end(); itp++) {
                    v[_T("lat")] = (*itp)->m_lat;
                    v[_T("lon")] = (*itp)->m_lon;
                    v[_T("TotalNodes")] = (*it)->pRoutePointList->GetCount();
                    v[_T("NodeNr")] = i;
                    v[_T("error")] = false;
                    i++;
                    wxString msg_id( _T("OCPN_TRACKPOINTS_COORDS") );
                    g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
                }
            } else {
                wxJSONValue v;
                v[_T("error")] = true;

                wxString msg_id( _T("OCPN_TRACKPOINTS_COORDS") );
                g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
            }
        }
    } else if(message_ID == _T("OCPN_ROUTE_REQUEST")) {
        wxJSONValue  root;
        wxJSONReader reader;
        wxString route_id = wxEmptyString;

        int numErrors = reader.Parse( message_JSONText, &root );
        if ( numErrors > 0 )  {
            return;
        }

        if(root.HasMember(_T("Route_ID")))
            route_id = root[_T("Route_ID")].AsString();

        for (RouteList::iterator it = pRouteList->begin(); it != pRouteList->end(); ++it) {
            wxString name = wxEmptyString;
            wxJSONValue v;

            if(!(*it)->IsTrack() && (*it)->m_GUID == route_id)
            {
                name = (*it)->m_RouteNameString;
                if(name.IsEmpty())
                    name = _("(Unnamed Route)");

                v[_T("Name")] = name;

                wxJSONValue v; int i = 0;
                for(RoutePointList::iterator itp = (*it)->pRoutePointList->begin(); itp != (*it)->pRoutePointList->end(); itp++)
                {
                    v[i][_T("error")] = false;
                    v[i][_T("lat")] = (*itp)->m_lat;
                    v[i][_T("lon")] = (*itp)->m_lon;
                    v[i][_T("WPName")] = (*itp)->GetName();
                    v[i][_T("WPDescription")] = (*itp)->GetDescription();
                    wxHyperlinkListNode *node = (*itp)->m_HyperlinkList->GetFirst();
                    if(node)
                    {
                        int n = 1;
                        while(node)
                        {
                            Hyperlink *httpLink = node->GetData();
                            v[i][_T("WPLink")+wxString::Format(_T("%d"),n)] = httpLink->Link;
                            v[i][_T("WPLinkDesciption")+wxString::Format(_T("%d"),n++)] = httpLink->DescrText;
                            node = node->GetNext();
                        }
                    }
                    i++;
                }
                wxString msg_id( _T("OCPN_ROUTE_RESPONSE") );
                g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
            }
            else
            {
                wxJSONValue v;
                v[0][_T("error")] = true;

                wxString msg_id( _T("OCPN_ROUTE_RESPONSE") );
                g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
            }
        }
    }
    else if(message_ID == _T("OCPN_ROUTELIST_REQUEST"))
    {
        wxJSONValue  root;
        wxJSONReader reader;
        bool mode = true;

        int numErrors = reader.Parse( message_JSONText, &root );
        if ( numErrors > 0 )
            return;

        if(root.HasMember(_T("mode")))
        {
            wxString str = root[_T("mode")].AsString();
            if( str == _T("Track")) mode = false;

            wxJSONValue v; int i = 1;
            for (RouteList::iterator it = pRouteList->begin(); it != pRouteList->end(); ++it) {
                if((*it)->IsTrack())
                    if(mode == true) continue;
                if(!(*it)->IsTrack())
                    if(mode == false) continue;
                v[0][_T("isTrack")] = !mode;

                wxString name = (*it)->m_RouteNameString;
                if(name.IsEmpty() && !mode)
                {
                    RoutePoint *rp = (*it)->GetPoint( 1 );
                    if( rp && rp->GetCreateTime().IsValid() ) name = rp->GetCreateTime().FormatISODate() + _T(" ")
                        + rp->GetCreateTime().FormatISOTime();
                    else
                        name = _("(Unnamed Track)");
                }
                else if(name.IsEmpty() && mode)
                    name = _("(Unnamed Route)");


                v[i][_T("error")] = false;
                v[i][_T("name")] = name;
                v[i][_T("GUID")] = (*it)->m_GUID;
                if(g_pActiveTrack == (*it) && !mode)
                    v[i][_T("active")] = true;
                else
                    v[i][_T("active")] = (*it)->IsActive();
                i++;
            }
            wxString msg_id( _T("OCPN_ROUTELIST_RESPONSE") );
            g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
        }
        else
        {
            wxJSONValue v;
            v[0][_T("error")] = true;
            wxString msg_id( _T("OCPN_ROUTELIST_RESPONSE") );
            g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
        }
    }
}

void MainFrame::OnEvtTHREADMSG( wxCommandEvent & event )
{
    wxLogMessage( event.GetString() );
}


bool MainFrame::EvalPriority(const wxString & message, DataStream *pDS )
{
    bool bret = true;
    wxString msg_type = message.Mid(1, 5);

    wxString stream_name;
    int stream_priority = 0;
    if( pDS ){
        stream_priority = pDS->GetPriority();
        stream_name = pDS->GetPort();
    }

    //  If the message type has never been seen before...
    if( NMEA_Msg_Hash.find( msg_type ) == NMEA_Msg_Hash.end() ) {
        NMEA_Msg_Container *pcontainer = new NMEA_Msg_Container;
        pcontainer-> current_priority = -1;     //  guarantee to execute the next clause
        pcontainer->stream_name = stream_name;
        pcontainer->receipt_time = wxDateTime::Now();

        NMEA_Msg_Hash[msg_type] = pcontainer;
    }

    NMEA_Msg_Container *pcontainer = NMEA_Msg_Hash[msg_type];
    wxString old_port = pcontainer->stream_name;

    int old_priority = pcontainer->current_priority;

    //  If the message has been seen before, and the priority is greater than or equal to current priority,
    //  then simply update the record
    if( stream_priority >= pcontainer->current_priority )
    {
        pcontainer->receipt_time = wxDateTime::Now();
        pcontainer-> current_priority = stream_priority;
        pcontainer->stream_name = stream_name;

        bret = true;
    }

    //  If the message has been seen before, and the priority is less than the current priority,
    //  then if the time since the last recorded message is greater than GPS_TIMEOUT_SECONDS
    //  then update the record with the new priority and stream.
    //  Otherwise, ignore the message as too low a priority
    else
    {
        if( (wxDateTime::Now().GetTicks() - pcontainer->receipt_time.GetTicks()) > GPS_TIMEOUT_SECONDS )
        {
            pcontainer->receipt_time = wxDateTime::Now();
            pcontainer-> current_priority = stream_priority;
            pcontainer->stream_name = stream_name;

            bret = true;
        }
        else
            bret = false;
    }

    wxString new_port = pcontainer->stream_name;

    //  If the data source or priority has changed for this message type, emit a log entry
    if (pcontainer->current_priority != old_priority || new_port != old_port )
    {
         wxString logmsg = wxString::Format(_T("Changing NMEA Datasource for %s to %s (Priority: %i)"),
                                            msg_type.c_str(),
                                            new_port.c_str(),
                                            pcontainer->current_priority);
         wxLogMessage(logmsg );

         if (NMEALogWindow::Get().Active())
         {
             wxDateTime now = wxDateTime::Now();
             wxString ss = now.FormatISOTime();
             ss.Append( _T(" ") );
             ss.Append( logmsg );
             ss.Prepend( _T("<RED>") );

             NMEALogWindow::Get().Add(ss);
             NMEALogWindow::Get().Refresh(false);
         }
    }
    return bret;
}

void MainFrame::gps_debug(const NMEA0183 & nmea, const wxString & str_buf) const
{
	wxString msg(_T("   "));
	msg.Append(nmea.ErrorMessage);
	msg.Append(_T(" : "));
	msg.Append(str_buf);
	wxLogMessage(msg);
}

void MainFrame::OnEvtOCPN_NMEA(OCPN_DataStreamEvent & event) // FIXME: this method is way to long
{
	wxString sfixtime;
	bool pos_valid = false;
	bool bis_recognized_sentence = true;
	bool ll_valid = true;

	const global::WatchDog::Data & wdt = global::OCPN::get().wdt().get_data();

	wxString str_buf = wxString(event.GetNMEAString().c_str(), wxConvUTF8);

	if( g_nNMEADebug && ( g_total_NMEAerror_messages < g_nNMEADebug ) )
	{
		g_total_NMEAerror_messages++;
		wxString msg( _T("MEH.NMEA Sentence received...") );
		msg.Append( str_buf );
		wxLogMessage( msg );
	}

	//  The message must be at least reasonably formed...
	if( (str_buf[0] != '$')  &&  (str_buf[0] != '!') )
		return;

	if (event.GetStream()) {
		if (!event.GetStream()->ChecksumOK(str_buf)) {
			if (g_nNMEADebug && (g_total_NMEAerror_messages < g_nNMEADebug)) {
				g_total_NMEAerror_messages++;
				wxString msg(_T(">>>>>>NMEA Sentence Checksum Bad..."));
				msg.Append(str_buf);
				wxLogMessage(msg);
			}
			return;
		}
	}

	bool b_accept = EvalPriority(str_buf, event.GetStream());
	if (!b_accept)
		return;

	m_NMEA0183 << str_buf;
	if (m_NMEA0183.PreParse()) {
		if (m_NMEA0183.LastSentenceIDReceived == _T("RMC")) {
			if (m_NMEA0183.Parse()) {
				if (m_NMEA0183.Rmc.IsDataValid == NTrue) {
					if (!wxIsNaN(m_NMEA0183.Rmc.Position.Latitude.Latitude)) {
						double llt = m_NMEA0183.Rmc.Position.Latitude.Latitude;
						int lat_deg_int = (int) ( llt / 100 );
						double lat_deg = lat_deg_int;
						double lat_min = llt - ( lat_deg * 100 );
						gLat = lat_deg + ( lat_min / 60. );
						if( m_NMEA0183.Rmc.Position.Latitude.Northing == South ) gLat = -gLat;
					} else
						ll_valid = false;

					if (!wxIsNaN(m_NMEA0183.Rmc.Position.Longitude.Longitude)) {
						double lln = m_NMEA0183.Rmc.Position.Longitude.Longitude;
						int lon_deg_int = (int) ( lln / 100 );
						double lon_deg = lon_deg_int;
						double lon_min = lln - ( lon_deg * 100 );
						gLon = lon_deg + ( lon_min / 60. );
						if (m_NMEA0183.Rmc.Position.Longitude.Easting == West)
							gLon = -gLon;
					} else
						ll_valid = false;

					global::Navigation & nav = global::OCPN::get().nav();
					nav.set_speed_over_ground(m_NMEA0183.Rmc.SpeedOverGroundKnots);
					nav.set_course_over_ground(m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue);

					if(!wxIsNaN(m_NMEA0183.Rmc.MagneticVariation)) {
						if (m_NMEA0183.Rmc.MagneticVariationDirection == East)
							global::OCPN::get().nav().set_magn_var(m_NMEA0183.Rmc.MagneticVariation);
						else
							if (m_NMEA0183.Rmc.MagneticVariationDirection == West)
								global::OCPN::get().nav().set_magn_var(-m_NMEA0183.Rmc.MagneticVariation);

						g_bVAR_Rx = true;
						global::OCPN::get().wdt().set_var_watchdog(wdt.gps_watchdog_timeout_ticks);
					}

					sfixtime = m_NMEA0183.Rmc.UTCTime;

					if (ll_valid) {
						global::OCPN::get().wdt().set_gps_watchdog(wdt.gps_watchdog_timeout_ticks);
						m_fixtime = wxDateTime::Now().GetTicks();
					}
					pos_valid = ll_valid;
				}
			} else if (g_nNMEADebug) {
				gps_debug(m_NMEA0183, str_buf);
			}
		} else if (m_NMEA0183.LastSentenceIDReceived == _T("HDT")) {
			if (m_NMEA0183.Parse()) {
				global::OCPN::get().nav().set_heading_true(m_NMEA0183.Hdt.DegreesTrue);
				if (!wxIsNaN(m_NMEA0183.Hdt.DegreesTrue)) {
					g_bHDT_Rx = true;
					global::OCPN::get().wdt().set_hdt_watchdog(wdt.gps_watchdog_timeout_ticks);
				}
			} else if (g_nNMEADebug) {
				gps_debug(m_NMEA0183, str_buf);
			}
		} else if (m_NMEA0183.LastSentenceIDReceived == _T("HDG")) {
			if (m_NMEA0183.Parse()) {
				global::Navigation & nav = global::OCPN::get().nav();
				nav.set_heading_magn(m_NMEA0183.Hdg.MagneticSensorHeadingDegrees);
				if (!wxIsNaN(m_NMEA0183.Hdg.MagneticSensorHeadingDegrees))
					global::OCPN::get().wdt().set_hdx_watchdog(wdt.gps_watchdog_timeout_ticks);

				if (m_NMEA0183.Hdg.MagneticVariationDirection == East)
					nav.set_magn_var(m_NMEA0183.Hdg.MagneticVariationDegrees);
				else if( m_NMEA0183.Hdg.MagneticVariationDirection == West )
					nav.set_magn_var(-m_NMEA0183.Hdg.MagneticVariationDegrees);

				if (!wxIsNaN(m_NMEA0183.Hdg.MagneticVariationDegrees)) {
					g_bVAR_Rx = true;
					global::OCPN::get().wdt().set_var_watchdog(wdt.gps_watchdog_timeout_ticks);
				}
			} else if (g_nNMEADebug) {
				gps_debug(m_NMEA0183, str_buf);
			}
		} else if (m_NMEA0183.LastSentenceIDReceived == _T("HDM")) {
			if (m_NMEA0183.Parse()) {
				global::OCPN::get().nav().set_heading_magn(m_NMEA0183.Hdm.DegreesMagnetic);
				if (!wxIsNaN(m_NMEA0183.Hdm.DegreesMagnetic))
					global::OCPN::get().wdt().set_hdx_watchdog(wdt.gps_watchdog_timeout_ticks);
			} else if (g_nNMEADebug) {
				gps_debug(m_NMEA0183, str_buf);
			}
		} else if (m_NMEA0183.LastSentenceIDReceived == _T("VTG")) {
			if (m_NMEA0183.Parse()) {
				global::Navigation & nav = global::OCPN::get().nav();
				nav.set_speed_over_ground(m_NMEA0183.Vtg.SpeedKnots);
				nav.set_course_over_ground(m_NMEA0183.Vtg.TrackDegreesTrue);
				if (!wxIsNaN(m_NMEA0183.Vtg.SpeedKnots) && !wxIsNaN(m_NMEA0183.Vtg.TrackDegreesTrue))
					global::OCPN::get().wdt().set_gps_watchdog(wdt.gps_watchdog_timeout_ticks);
			} else if (g_nNMEADebug) {
				gps_debug(m_NMEA0183, str_buf);
			}
		} else if (m_NMEA0183.LastSentenceIDReceived == _T("GSV")) {
			if (m_NMEA0183.Parse()) {
				g_SatsInView = m_NMEA0183.Gsv.SatsInView;
				global::OCPN::get().wdt().set_sat_watchdog(wdt.sat_watchdog_timeout_ticks);
				g_bSatValid = true;
			} else if (g_nNMEADebug) {
				gps_debug(m_NMEA0183, str_buf);
			}
		} else if (g_bUseGLL && m_NMEA0183.LastSentenceIDReceived == _T("GLL")) {
			if (m_NMEA0183.Parse()) {
				if( m_NMEA0183.Gll.IsDataValid == NTrue ) {
					if( !wxIsNaN(m_NMEA0183.Gll.Position.Latitude.Latitude) ) {
						double llt = m_NMEA0183.Gll.Position.Latitude.Latitude;
						int lat_deg_int = (int) ( llt / 100 );
						double lat_deg = lat_deg_int;
						double lat_min = llt - ( lat_deg * 100 );
						gLat = lat_deg + ( lat_min / 60. );
						if (m_NMEA0183.Gll.Position.Latitude.Northing == South)
							gLat = -gLat;
					} else
						ll_valid = false;

					if (!wxIsNaN(m_NMEA0183.Gll.Position.Longitude.Longitude)) {
						double lln = m_NMEA0183.Gll.Position.Longitude.Longitude;
						int lon_deg_int = (int) ( lln / 100 );
						double lon_deg = lon_deg_int;
						double lon_min = lln - ( lon_deg * 100 );
						gLon = lon_deg + ( lon_min / 60. );
						if (m_NMEA0183.Gll.Position.Longitude.Easting == West)
							gLon = -gLon;
					} else
						ll_valid = false;

					sfixtime = m_NMEA0183.Gll.UTCTime;

					if(ll_valid) {
						global::OCPN::get().wdt().set_gps_watchdog(wdt.gps_watchdog_timeout_ticks);
						m_fixtime = wxDateTime::Now().GetTicks();
					}
					pos_valid = ll_valid;
				}
			} else if (g_nNMEADebug) {
				gps_debug(m_NMEA0183, str_buf);
			}
		} else if (m_NMEA0183.LastSentenceIDReceived == _T("GGA")) {
			if (m_NMEA0183.Parse()) {
				if (m_NMEA0183.Gga.GPSQuality > 0) {
					if (!wxIsNaN(m_NMEA0183.Gll.Position.Latitude.Latitude)) {
						double llt = m_NMEA0183.Gga.Position.Latitude.Latitude;
						int lat_deg_int = (int) ( llt / 100 );
						double lat_deg = lat_deg_int;
						double lat_min = llt - ( lat_deg * 100 );
						gLat = lat_deg + ( lat_min / 60. );
						if (m_NMEA0183.Gga.Position.Latitude.Northing == South)
							gLat = -gLat;
					} else
						ll_valid = false;

					if (!wxIsNaN(m_NMEA0183.Gga.Position.Longitude.Longitude)) {
						double lln = m_NMEA0183.Gga.Position.Longitude.Longitude;
						int lon_deg_int = (int) ( lln / 100 );
						double lon_deg = lon_deg_int;
						double lon_min = lln - ( lon_deg * 100 );
						gLon = lon_deg + ( lon_min / 60. );
						if (m_NMEA0183.Gga.Position.Longitude.Easting == West)
							gLon = -gLon;
					} else
						ll_valid = false;

					sfixtime = m_NMEA0183.Gga.UTCTime;

					if (ll_valid) {
						global::OCPN::get().wdt().set_gps_watchdog(wdt.gps_watchdog_timeout_ticks);
						m_fixtime = wxDateTime::Now().GetTicks();
					}
					pos_valid = ll_valid;

					g_SatsInView = m_NMEA0183.Gga.NumberOfSatellitesInUse;
					global::OCPN::get().wdt().set_sat_watchdog(wdt.sat_watchdog_timeout_ticks);
					g_bSatValid = true;
				}
			} else if (g_nNMEADebug) {
				gps_debug(m_NMEA0183, str_buf);
			}
		}
	}
	// Process ownship (AIVDO) messages from any source
	else if (str_buf.Mid( 1, 5 ).IsSameAs(_T("AIVDO")))
	{
		GenericPosDatEx gpd;
		AIS_Error nerr = AIS_GENERIC_ERROR;
		if(g_pAIS)
			nerr = g_pAIS->DecodeSingleVDO(str_buf, &gpd, &m_VDO_accumulator);

		if (nerr == AIS_NoError) {
			if (!wxIsNaN(gpd.kLat))
				gLat = gpd.kLat;
			if (!wxIsNaN(gpd.kLon))
				gLon = gpd.kLon;

			global::Navigation & nav = global::OCPN::get().nav();
			nav.set_speed_over_ground(gpd.kSog);
			nav.set_course_over_ground(gpd.kCog);

			global::OCPN::get().nav().set_heading_true(gpd.kHdt);
			if( !wxIsNaN(gpd.kHdt) ) {
				g_bHDT_Rx = true;
				global::OCPN::get().wdt().set_hdt_watchdog(wdt.gps_watchdog_timeout_ticks);
			}

			if (!wxIsNaN(gpd.kLat) && !wxIsNaN(gpd.kLon) ) {
				global::OCPN::get().wdt().set_gps_watchdog(wdt.gps_watchdog_timeout_ticks);
				m_fixtime = wxDateTime::Now().GetTicks();
				pos_valid = true;
			}
		} else {
			if (g_nNMEADebug && (g_total_NMEAerror_messages < g_nNMEADebug)) {
				g_total_NMEAerror_messages++;
				wxString msg( _T("   Invalid AIVDO Sentence...") );
				msg.Append( str_buf );
				wxLogMessage( msg );
			}
		}
	} else {
		bis_recognized_sentence = false;
		if (g_nNMEADebug && (g_total_NMEAerror_messages < g_nNMEADebug)) {
			g_total_NMEAerror_messages++;
			wxString msg( _T("   Unrecognized NMEA Sentence...") );
			msg.Append( str_buf );
			wxLogMessage( msg );
		}
	}

	if (bis_recognized_sentence)
		PostProcessNNEA(pos_valid, sfixtime);
}

void MainFrame::PostProcessNNEA(bool pos_valid, const wxString &sfixtime)
{
	const global::Navigation::Data & nav = global::OCPN::get().nav().get_data();
	const global::WatchDog::Data & wdt = global::OCPN::get().wdt().get_data();

	FilterCogSog();

	//    If speed over ground is greater than some threshold, we determine that we are "cruising"
	if (nav.sog > 3.0)
		g_bCruising = true;

	//    Here is the one place we try to create Hdt from Hdm and Var,
	//    but only if NMEA HDT sentence is not being received

	if (!g_bHDT_Rx) {
		if( !wxIsNaN(nav.var) && !wxIsNaN(nav.hdm)) {
			global::OCPN::get().nav().set_heading_true(nav.hdm + nav.var);
			global::OCPN::get().wdt().set_hdt_watchdog(wdt.gps_watchdog_timeout_ticks);
		}
	}

	if( pos_valid ) {
		if( g_nNMEADebug ) {
			wxString msg( _T("PostProcess NMEA with valid position") );
			wxLogMessage( msg );
		}

		//      Maintain the validity flags
		bool last_bGPSValid = bGPSValid;
		bGPSValid = true;
		if( !last_bGPSValid ) UpdateGPSCompassStatusBox();

		// Show a little heartbeat tick in StatusWindow0 on NMEA events
		// But no faster than 10 hz.
		unsigned long uiCurrentTickCount;
		m_MMEAeventTime.SetToCurrent();
		uiCurrentTickCount = m_MMEAeventTime.GetMillisecond() / 100;           // tenths of a second
		uiCurrentTickCount += m_MMEAeventTime.GetTicks() * 10;
		if( uiCurrentTickCount > m_ulLastNEMATicktime + 1 ) {
			m_ulLastNEMATicktime = uiCurrentTickCount;

			if( tick_idx++ > 6 ) tick_idx = 0;
		}
	}

	// Show gLat/gLon in StatusWindow0

	if (NULL != GetStatusBar()) {
		char tick_buf[2];
		tick_buf[0] = nmea_tick_chars[tick_idx];
		tick_buf[1] = 0;

		wxString s1( tick_buf, wxConvUTF8 );
		s1 += _(" Ship ");
		s1 += toSDMM( 1, gLat );
		s1 += _T("   ");
		s1 += toSDMM( 2, gLon );
		SetStatusText( s1, STAT_FIELD_TICK );

		wxString over_ground;
		if (wxIsNaN(nav.sog))
			over_ground += wxString::Format(_T("SOG --- ") + getUsrSpeedUnit() + _T("  "));
		else
			over_ground += wxString::Format(_T("SOG %2.2f ") + getUsrSpeedUnit() + _T("  "), toUsrSpeed(nav.sog));

		if (wxIsNaN(nav.cog)) {
			over_ground += wxString("COG ---\u00B0", wxConvUTF8);
		} else {
			const global::Navigation::Data & nav = global::OCPN::get().nav().get_data();
			if (g_bShowMag) {
				over_ground += wxString::Format(wxString("COG %03d°(M)  ", wxConvUTF8), (int)gFrame->GetTrueOrMag(nav.cog));
			} else {
				over_ground += wxString::Format(wxString("COG %03d°  ", wxConvUTF8), (int)gFrame->GetTrueOrMag(nav.cog));
			}
		}

		SetStatusText(over_ground, STAT_FIELD_SOGCOG);
	}

	//    Maintain average COG for Course Up Mode

	if( !wxIsNaN(nav.cog) ) {
		if( g_COGAvgSec > 0 ) {
			//    Make a hole
			for( int i = g_COGAvgSec - 1; i > 0; i-- )
				COGTable[i] = COGTable[i - 1];
			COGTable[0] = nav.cog;

			double sum = 0.;
			for( int i = 0; i < g_COGAvgSec; i++ ) {
				double adder = COGTable[i];

				if (fabs(adder - g_COGAvg) > 180.0) {
					if ((adder - g_COGAvg) > 0.0)
						adder -= 360.0;
					else
						adder += 360.0;
				}

				sum += adder;
			}
			sum /= g_COGAvgSec;

			if (sum < 0.0)
				sum += 360.0;
			else if (sum >= 360.0)
				sum -= 360.0;

			g_COGAvg = sum;
		} else
			g_COGAvg = nav.cog;
	}

#ifdef ocpnUPDATE_SYSTEM_TIME
	//      Use the fix time to update the local system clock, only once per session
	if( ( sfixtime.Len() ) && s_bSetSystemTime && ( m_bTimeIsSet == false ) ) {
        wxDateTime Fix_Time;

        if( 6 == sfixtime.Len() )                   // perfectly recognised format?
                {
            wxString a;
            long b;
            int hr = 0;
            int min = 0;
            int sec = 0;

            a = sfixtime.Mid( 0, 2 );
            if( a.ToLong( &b ) ) hr = b;
            a = sfixtime.Mid( 2, 2 );
            if( a.ToLong( &b ) ) min = b;
            a = sfixtime.Mid( 4, 2 );
            if( a.ToLong( &b ) ) sec = b;

            Fix_Time.Set( hr, min, sec );
        }
        wxString fix_time_format = Fix_Time.Format( _T("%Y-%m-%dT%H:%M:%S") ); // this should show as LOCAL

        //          Compare the server (fix) time to the current system time
        wxDateTime sdt;
        sdt.SetToCurrent();
        wxDateTime cwxft = Fix_Time;                  // take a copy
        wxTimeSpan ts;
        ts = cwxft.Subtract( sdt );

        int b = ( ts.GetSeconds() ).ToLong();

        //          Correct system time if necessary
        //      Only set the time if wrong by more than 1 minute, and less than 2 hours
        //      This should eliminate bogus times which may come from faulty GPS units

        if( ( abs( b ) > 60 ) && ( abs( b ) < ( 2 * 60 * 60 ) ) ) {

#ifdef __WXMSW__
            //      Fix up the fix_time to convert to GMT
            Fix_Time = Fix_Time.ToGMT();

            //    Code snippet following borrowed from wxDateCtrl, MSW

            const wxDateTime::Tm tm( Fix_Time.GetTm() );

            SYSTEMTIME stm;
            stm.wYear = (WXWORD) tm.year;
            stm.wMonth = (WXWORD) ( tm.mon - wxDateTime::Jan + 1 );
            stm.wDay = tm.mday;

            stm.wDayOfWeek = 0;
            stm.wHour = Fix_Time.GetHour();
            stm.wMinute = tm.min;
            stm.wSecond = tm.sec;
            stm.wMilliseconds = 0;

            ::SetSystemTime( &stm );            // in GMT

#else

            //      This contortion sets the system date/time on POSIX host
            //      Requires the following line in /etc/sudoers
            //          nav ALL=NOPASSWD:/bin/date -s *

            wxString msg;
            msg.Printf(_T("Setting system time, delta t is %d seconds"), b);
            wxLogMessage(msg);

            wxString sdate(Fix_Time.Format(_T("%D")));
            sdate.Prepend(_T("sudo /bin/date -s \""));

            wxString stime(Fix_Time.Format(_T("%T")));
            stime.Prepend(_T(" "));
            sdate.Append(stime);
            sdate.Append(_T("\""));

            msg.Printf(_T("Linux command is:"));
            msg += sdate;
            wxLogMessage(msg);
            wxExecute(sdate, wxEXEC_ASYNC);

#endif      //__WXMSW__
            m_bTimeIsSet = true;

        }           // if needs correction
    }               // if valid time

#endif            //ocpnUPDATE_SYSTEM_TIME
}

void MainFrame::FilterCogSog( void )
{
	global::Navigation & nav = global::OCPN::get().nav();

    if( g_bfilter_cogsog ) {
        //    If the data are undefined, leave the array intact
        if( !wxIsNaN(nav.get_data().cog) ) {
            //    Simple averaging filter for COG
            double cog_last = nav.get_data().cog;       // most recent reported value

            //    Make a hole in array
            for( int i = g_COGFilterSec - 1; i > 0; i-- )
                COGFilterTable[i] = COGFilterTable[i - 1];
            COGFilterTable[0] = cog_last;

            //
            double sum = 0.;
            for( int i = 0; i < g_COGFilterSec; i++ ) {
                double adder = COGFilterTable[i];

                if( fabs( adder - m_COGFilterLast ) > 180. ) {
                    if( ( adder - m_COGFilterLast ) > 0. )
						adder -= 360.;
                    else
                        adder += 360.;
                }

                sum += adder;
            }
            sum /= g_COGFilterSec;

            if (sum < 0.0)
				sum += 360.0;
            else
                if (sum >= 360.0)
					sum -= 360.0;

            nav.set_course_over_ground(sum);
            m_COGFilterLast = sum;
        }

		global::Navigation & nav = global::OCPN::get().nav();

        //    If the data are undefined, leave the array intact
		// FIXME: perfect opportunity to move such a filter to a separate class, dare I mention "unit test"
        if (!wxIsNaN(nav.get_data().sog)) {
            //    Simple averaging filter for SOG
            double sog_last = nav.get_data().sog; // most recent reported value

            //    Make a hole in array
            for( int i = g_SOGFilterSec - 1; i > 0; i-- )
                SOGFilterTable[i] = SOGFilterTable[i - 1];
            SOGFilterTable[0] = sog_last;

            double sum = 0.0;
            for (int i = 0; i < g_SOGFilterSec; ++i) {
                sum += SOGFilterTable[i];
            }
            sum /= g_SOGFilterSec;

			nav.set_speed_over_ground(sum);
        }
    }
}

void MainFrame::StopSockets( void )
{
//TODO: Can be removed?
}

void MainFrame::ResumeSockets( void )
{
//TODO: Can be removed?
}

void MainFrame::LoadHarmonics()
{
	if(!ptcmgr) {
		ptcmgr = new TCMgr;
		ptcmgr->LoadDataSources(TideCurrentDataSet);
		return ;
	}

	bool b_newdataset = false;

	//      Test both ways
	wxArrayString test = ptcmgr->GetDataSet();
	for(unsigned int i=0 ; i < test.GetCount() ; i++) {
		bool b_foundi = false;
		for(unsigned int j=0 ; j < TideCurrentDataSet.GetCount() ; j++) {
			if(TideCurrentDataSet.Item(j) == test.Item(i)) {
				b_foundi = true;
				break;              // j loop
			}
		}
		if(!b_foundi) {
			b_newdataset = true;
			break;                  //  i loop
		}
	}

	test = TideCurrentDataSet;
	for(unsigned int i=0 ; i < test.GetCount() ; i++) {
		bool b_foundi = false;
		for(unsigned int j=0 ; j < ptcmgr->GetDataSet().GetCount() ; j++) {
			if(ptcmgr->GetDataSet().Item(j) == test.Item(i)) {
				b_foundi = true;
				break;              // j loop
			}
		}
		if(!b_foundi) {
			b_newdataset = true;
			break;                  //  i loop
		}
	}

	if(b_newdataset)
		ptcmgr->LoadDataSources(TideCurrentDataSet);
}

void appendOSDirSlash(wxString & s)
{
    wxChar sep = wxFileName::GetPathSeparator();
    if (s.Last() != sep)
		s.Append(sep);
}

/*************************************************************************
 * Global color management routines
 *
 *************************************************************************/

#ifdef __WXMSW__

#define NCOLORS 40

typedef struct _MSW_COLOR_SPEC {
    int COLOR_NAME;
    wxString S52_RGB_COLOR;
    int SysRGB_COLOR;
} MSW_COLOR_SPEC;

MSW_COLOR_SPEC color_spec[] = { { COLOR_MENU, _T("UIBCK"), 0 }, { COLOR_MENUTEXT, _T("UITX1"), 0 },
        { COLOR_BTNSHADOW, _T("UIBCK"), 0 },                        // Menu Frame
        { -1, _T(""), 0 } };

void SaveSystemColors()
{
    /*
     color_3dface = pGetSysColor(COLOR_3DFACE);
     color_3dhilite = pGetSysColor(COLOR_3DHILIGHT);
     color_3dshadow = pGetSysColor(COLOR_3DSHADOW);
     color_3ddkshadow = pGetSysColor(COLOR_3DDKSHADOW);
     color_3dlight = pGetSysColor(COLOR_3DLIGHT);
     color_activecaption = pGetSysColor(COLOR_ACTIVECAPTION);
     color_gradientactivecaption = pGetSysColor(27); //COLOR_3DLIGHT);
     color_captiontext = pGetSysColor(COLOR_CAPTIONTEXT);
     color_windowframe = pGetSysColor(COLOR_WINDOWFRAME);
     color_inactiveborder = pGetSysColor(COLOR_INACTIVEBORDER);
     */
    //    Record the default system color in my substitution structure
    MSW_COLOR_SPEC *pcspec = &color_spec[0];
    while( pcspec->COLOR_NAME != -1 ) {
        pcspec->SysRGB_COLOR = pGetSysColor( pcspec->COLOR_NAME );
        pcspec++;
    }
}

void RestoreSystemColors()
{
    int element[NCOLORS];
    int rgbcolor[NCOLORS];
    int i = 0;

    MSW_COLOR_SPEC *pcspec = &color_spec[0];
    while( pcspec->COLOR_NAME != -1 ) {
        element[i] = pcspec->COLOR_NAME;
        rgbcolor[i] = pcspec->SysRGB_COLOR;

        pcspec++;
        i++;
    }

    pSetSysColors( i, (unsigned long *) &element[0], (unsigned long *) &rgbcolor[0] );

}

#endif

void SetSystemColors(ColorScheme cs)
{
//---------------
#if 0
    //    This is the list of Color Types from winuser.h
    /*
     * Color Types
     */
#define CTLCOLOR_MSGBOX         0
#define CTLCOLOR_EDIT           1
#define CTLCOLOR_LISTBOX        2
#define CTLCOLOR_BTN            3
#define CTLCOLOR_DLG            4
#define CTLCOLOR_SCROLLBAR      5
#define CTLCOLOR_STATIC         6
#define CTLCOLOR_MAX            7

#define COLOR_SCROLLBAR         0         //??
#define COLOR_BACKGROUND        1         //??
#define COLOR_ACTIVECAPTION     2       //??
#define COLOR_INACTIVECAPTION   3         //??
#define COLOR_MENU              4         // Menu background
#define COLOR_WINDOW            5         // default window background
#define COLOR_WINDOWFRAME       6         // Sub-Window frames, like status bar, etc..
#define COLOR_MENUTEXT          7         // Menu text
#define COLOR_WINDOWTEXT        8         //??
#define COLOR_CAPTIONTEXT       9         //??
#define COLOR_ACTIVEBORDER      10        //??
#define COLOR_INACTIVEBORDER    11       //??
#define COLOR_APPWORKSPACE      12       //??
#define COLOR_HIGHLIGHT         13       //Highlited text background  in query box tree
#define COLOR_HIGHLIGHTTEXT     14        //??
#define COLOR_BTNFACE           15        //??
#define COLOR_BTNSHADOW         16        // Menu Frame
#define COLOR_GRAYTEXT          17        // Greyed out text in menu
#define COLOR_BTNTEXT           18        //??
#define COLOR_INACTIVECAPTIONTEXT 19      //??
#define COLOR_BTNHIGHLIGHT      20        //??
#if(WINVER >= 0x0400)
#define COLOR_3DDKSHADOW        21        //??
#define COLOR_3DLIGHT           22        // Grid rule lines in list control
#define COLOR_INFOTEXT          23        //??
#define COLOR_INFOBK            24
#endif /* WINVER >= 0x0400 */

#if(WINVER >= 0x0500)
#define COLOR_HOTLIGHT          26              //??
#define COLOR_GRADIENTACTIVECAPTION 27        //??
#define COLOR_GRADIENTINACTIVECAPTION 28        //??
#if(WINVER >= 0x0501)
#define COLOR_MENUHILIGHT       29              // Selected item in menu, maybe needs reset on popup menu?
#define COLOR_MENUBAR           30              //??
#endif /* WINVER >= 0x0501 */
#endif /* WINVER >= 0x0500 */

#if(WINVER >= 0x0400)
#define COLOR_DESKTOP           COLOR_BACKGROUND
#define COLOR_3DFACE            COLOR_BTNFACE
#define COLOR_3DSHADOW          COLOR_BTNSHADOW
#define COLOR_3DHIGHLIGHT       COLOR_BTNHIGHLIGHT
#define COLOR_3DHILIGHT         COLOR_BTNHIGHLIGHT
#define COLOR_BTNHILIGHT        COLOR_BTNHIGHLIGHT
#endif /* WINVER >= 0x0400 */
#endif

#ifdef __WXMSW__
    int element[NCOLORS];
    int rgbcolor[NCOLORS];
    int i = 0;
    if( ( GLOBAL_COLOR_SCHEME_DUSK == cs ) || ( GLOBAL_COLOR_SCHEME_NIGHT == cs ) ) {
        MSW_COLOR_SPEC *pcspec = &color_spec[0];
        while( pcspec->COLOR_NAME != -1 ) {
            wxColour color = GetGlobalColor( pcspec->S52_RGB_COLOR );
            rgbcolor[i] = ( color.Red() << 16 ) + ( color.Green() << 8 ) + color.Blue();
            element[i] = pcspec->COLOR_NAME;

            i++;
            pcspec++;
        }

        pSetSysColors( i, (unsigned long *) &element[0], (unsigned long *) &rgbcolor[0] );

    } else {         // for daylight colors, use default windows colors as saved....

        RestoreSystemColors();
    }
#else
	WXUNUSED(cs);
#endif
}

