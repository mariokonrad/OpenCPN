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

#include <wx/image.h>
#include <wx/graphics.h>
#include <wx/listbook.h>
#include <wx/clipbrd.h>
#include <wx/aui/aui.h>
#include <wx/listimpl.cpp>

#include "ChartCanvas.h"
#include <ocpnDC.h>
#include <RouteManagerDialog.h>
#include <GoToPositionDialog.h>
#include <TimedPopupWin.h>
#include <MessageBox.h>
#include <dychart.h>
#include <StyleManager.h>
#include <Style.h>
#include <StatWin.h>
#include <Track.h>
#include <Kml.h>
#include <ConsoleCanvas.h>
#include <ThumbWin.h>
#include <WayPointman.h>
#include <MainFrame.h>
#include <RouteProp.h>
#include <MarkInfo.h>
#include <TrackPropDlg.h>
#include <ocpn_pixel.h>
#include <Undo.h>
#include <Multiplexer.h>
#include <timers.h>
#include <ChInfoWin.h>
#include <Quilt.h>
#include <SelectItem.h>
#include <Select.h>
#include <FontMgr.h>
#include <SendToGpsDlg.h>
#include <OCPNRegion.h>
#include <FloatingCompassWindow.h>
#include <S57QueryDialog.h>
#include <OCPNFloatingToolbarDialog.h>
#include <OCPNMemDC.h>
#include <EmbossData.h>
#include <TCWin.h>
#include <StatusBar.h>
#include <GUI_IDs.h>
#include <Config.h>
#include <Layer.h>
#include <PositionConvert.h>
#include <Units.h>

#include <windows/compatibility.h>

#include <util/math.h>

#include <plugin/PlugInManager.h>

#include <navigation/AnchorDist.h>
#include <navigation/MagneticVariation.h>
#include <navigation/RouteTracker.h>
#include <navigation/RouteManager.h>

#include <geo/LineClip.h>
#include <geo/Geodesic.h>

#include <chart/gshhs/GSHHSChart.h>
#include <chart/ChartDB.h>
#include <chart/ChartStack.h>
#include <chart/ChartBaseBSB.h>

#include <tide/IDX_entry.h>
#include <tide/tide_time.h>
#include <tide/TCMgr.h>

#include <global/OCPN.h>
#include <global/System.h>
#include <global/GUI.h>
#include <global/AIS.h>
#include <global/Navigation.h>
#include <global/ColorManager.h>

#include <ais/ais.h>
#include <ais/AIS_Decoder.h>
#include <ais/AIS_Target_Data.h>
#include <ais/AISTargetAlertDialog.h>

#ifdef USE_S57
	#include <chart/S57Chart.h>
	#include <chart/s52plib.h>
	#include <chart/CM93compchart.h>
	#include <chart/CM93OffsetDialog.h>
#endif

#include <vector>

#ifdef __MSVC__
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif


static int SetScreenBrightness(int brightness);
static int InitScreenBrightness(void);

namespace chart {
	extern bool G_FloatPtInPolygon(MyFlPoint* rgpts, int wnumpts, float x, float y);
}

extern void catch_signals(int signo);

extern chart::ChartBase* Current_Vector_Ch;
extern chart::ChartBase* Current_Ch;
extern chart::ChartDB* ChartData;
extern ConsoleCanvas* console;
extern FloatingCompassWindow* g_FloatingCompassDialog;
extern RouteList* pRouteList;
extern Config* pConfig;
extern Select* pSelect;
extern ThumbWin* pthumbwin;
extern tide::TCMgr* ptcmgr;
extern Select* pSelectTC;
extern Select* pSelectAIS;
extern WayPointman* pWayPointMan;
extern MarkInfoImpl* pMarkPropDialog;
extern RouteProp* pRoutePropDialog;
extern TrackPropDlg* pTrackPropDialog;
extern MarkInfoImpl* pMarkInfoDialog;

extern chart::ChartGroupArray* g_pGroupArray;
extern RoutePoint* pAnchorWatchPoint1;
extern RoutePoint* pAnchorWatchPoint2;

extern OCPNFloatingToolbarDialog* g_FloatingToolbarDialog;
extern RouteManagerDialog* pRouteManagerDialog;
GoToPositionDialog* pGoToPositionDialog;

#ifdef USE_S57
extern chart::s52plib* ps52plib;
extern chart::CM93OffsetDialog* g_pCM93OffsetDialog;
#endif

extern ais::AIS_Decoder* g_pAIS;

extern MainFrame* gFrame;
extern StatWin* stats;

extern ais::AISTargetAlertDialog* g_pais_alert_dialog_active;
extern ais::AISTargetQueryDialog* g_pais_query_dialog_active;

extern CM93DSlide* pCM93DetailSlider;

extern ChartCanvas* cc1;

extern chart::ChartStack* pCurrentStack;
extern bool g_bquiting;
extern ais::AISTargetListDialog* g_pAISTargetList;

extern PlugInManager* g_pi_manager;

extern wxAuiManager* g_pauimgr;

extern wxProgressDialog* s_ProgDialog;

extern bool g_b_assume_azerty;

S57QueryDialog* g_pObjectQueryDialog = NULL;
extern ocpnStyle::StyleManager* g_StyleManager;
extern ArrayOfConnPrm* g_pConnectionParams;

extern sound::OCPN_Sound g_anchorwatch_sound;

// TODO why are these static?
static int mouse_x;
static int mouse_y;
static bool mouse_leftisdown;

// FIXME: (semi-)globals?
static int r_gamma_mult = 1;
static int g_gamma_mult = 1;
static int b_gamma_mult = 1;
static int gamma_state = 0;
static bool g_brightness_init = false;
static int last_brightness = 0;

// "Curtain" mode parameters
wxDialog* g_pcurtain;

#define MIN_BRIGHT 10
#define MAX_BRIGHT 100

using ais::AIS_Target_Data;
using ais::AIS_Target_Hash;
using ais::AIS_Area_Notice_Hash;
using ais::Ais8_001_22;
using ais::Ais8_001_22_SubAreaList;

// Constants for right click menus
enum {
	ID_DEF_MENU_MAX_DETAIL = 1,
	ID_DEF_MENU_SCALE_IN,
	ID_DEF_MENU_SCALE_OUT,
	ID_DEF_MENU_DROP_WP,
	ID_DEF_MENU_QUERY,
	ID_DEF_MENU_MOVE_BOAT_HERE,
	ID_DEF_MENU_GOTO_HERE,
	ID_DEF_MENU_GOTOPOSITION,
	ID_WP_MENU_GOTO,
	ID_WP_MENU_DELPOINT,
	ID_WP_MENU_PROPERTIES,
	ID_RT_MENU_ACTIVATE,
	ID_RT_MENU_DEACTIVATE,
	ID_RT_MENU_INSERT,
	ID_RT_MENU_APPEND,
	ID_RT_MENU_COPY,
	ID_TK_MENU_COPY,
	ID_WPT_MENU_COPY,
	ID_WPT_MENU_SENDTOGPS,
	ID_PASTE_WAYPOINT,
	ID_PASTE_ROUTE,
	ID_PASTE_TRACK,
	ID_RT_MENU_DELETE,
	ID_RT_MENU_REVERSE,
	ID_RT_MENU_DELPOINT,
	ID_RT_MENU_ACTPOINT,
	ID_RT_MENU_DEACTPOINT,
	ID_RT_MENU_ACTNXTPOINT,
	ID_RT_MENU_REMPOINT,
	ID_RT_MENU_PROPERTIES,
	ID_RT_MENU_SENDTOGPS,
	ID_WP_MENU_SET_ANCHORWATCH,
	ID_WP_MENU_CLEAR_ANCHORWATCH,
	ID_DEF_MENU_AISTARGETLIST,
	ID_RC_MENU_SCALE_IN,
	ID_RC_MENU_SCALE_OUT,
	ID_RC_MENU_ZOOM_IN,
	ID_RC_MENU_ZOOM_OUT,
	ID_RC_MENU_FINISH,
	ID_DEF_MENU_AIS_QUERY,
	ID_DEF_MENU_AIS_CPA,
	ID_DEF_MENU_AISSHOWTRACK,
	ID_DEF_MENU_ACTIVATE_MEASURE,
	ID_DEF_MENU_DEACTIVATE_MEASURE,
	ID_UNDO,
	ID_REDO,
	ID_DEF_MENU_CM93OFFSET_DIALOG,
	ID_TK_MENU_PROPERTIES,
	ID_TK_MENU_DELETE,
	ID_WP_MENU_ADDITIONAL_INFO,
	ID_DEF_MENU_QUILTREMOVE,
	ID_DEF_MENU_COGUP,
	ID_DEF_MENU_NORTHUP,
	ID_DEF_MENU_TIDEINFO,
	ID_DEF_MENU_CURRENTINFO,
	ID_DEF_MENU_GROUPBASE,
	ID_DEF_MENU_LAST
};

PFNGLGENFRAMEBUFFERSEXTPROC         s_glGenFramebuffersEXT;
PFNGLGENRENDERBUFFERSEXTPROC        s_glGenRenderbuffersEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    s_glFramebufferTexture2DEXT;
PFNGLBINDFRAMEBUFFEREXTPROC         s_glBindFramebufferEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC s_glFramebufferRenderbufferEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC     s_glRenderbufferStorageEXT;
PFNGLBINDRENDERBUFFEREXTPROC        s_glBindRenderbufferEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  s_glCheckFramebufferStatusEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC      s_glDeleteFramebuffersEXT;
PFNGLDELETERENDERBUFFERSEXTPROC     s_glDeleteRenderbuffersEXT;

BEGIN_EVENT_TABLE(ChartCanvas, wxWindow)
	EVT_PAINT(ChartCanvas::OnPaint)
	EVT_ACTIVATE(ChartCanvas::OnActivate)
	EVT_SIZE(ChartCanvas::OnSize)
	EVT_MOUSE_EVENTS(ChartCanvas::MouseEvent)
	EVT_TIMER(DBLCLICK_TIMER, ChartCanvas::MouseTimedEvent)
	EVT_TIMER(PAN_TIMER, ChartCanvas::PanTimerEvent)
	EVT_TIMER(CURTRACK_TIMER, ChartCanvas::OnCursorTrackTimerEvent)
	EVT_TIMER(ROT_TIMER, ChartCanvas::RotateTimerEvent)
	EVT_TIMER(ROPOPUP_TIMER, ChartCanvas::OnRolloverPopupTimerEvent)
	EVT_KEY_DOWN(ChartCanvas::OnKeyDown)
	EVT_KEY_UP(ChartCanvas::OnKeyUp)
	EVT_TIMER(PANKEY_TIMER, ChartCanvas::Do_Pankeys)
	EVT_MOUSE_CAPTURE_LOST(ChartCanvas::LostMouseCapture)
	EVT_TIMER(ZOOM_TIMER, ChartCanvas::OnZoomTimerEvent)

	EVT_MENU(ID_DEF_MENU_MAX_DETAIL, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_SCALE_IN, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_SCALE_OUT, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_QUERY, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_DROP_WP, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_MOVE_BOAT_HERE, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_GOTO_HERE, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_GOTOPOSITION, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_COGUP, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_NORTHUP, ChartCanvas::PopupMenuHandler)

	EVT_MENU(ID_RT_MENU_ACTIVATE, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RT_MENU_DEACTIVATE, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RT_MENU_INSERT, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RT_MENU_APPEND, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RT_MENU_COPY, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_TK_MENU_COPY, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RT_MENU_SENDTOGPS, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_WPT_MENU_COPY, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_WPT_MENU_SENDTOGPS, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_PASTE_WAYPOINT, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_PASTE_ROUTE, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_PASTE_TRACK, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RT_MENU_DELETE, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RT_MENU_REVERSE, ChartCanvas::PopupMenuHandler)

	EVT_MENU(ID_RT_MENU_DELPOINT, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RT_MENU_REMPOINT, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RT_MENU_ACTPOINT, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RT_MENU_DEACTPOINT, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RT_MENU_ACTNXTPOINT, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RT_MENU_PROPERTIES, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_WP_MENU_SET_ANCHORWATCH, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_WP_MENU_CLEAR_ANCHORWATCH, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_AISTARGETLIST, ChartCanvas::PopupMenuHandler)

	EVT_MENU(ID_RC_MENU_SCALE_IN, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RC_MENU_SCALE_OUT, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RC_MENU_ZOOM_IN, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RC_MENU_ZOOM_OUT, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_RC_MENU_FINISH, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_AIS_QUERY, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_AIS_CPA, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_AISSHOWTRACK, ChartCanvas::PopupMenuHandler)

	EVT_MENU(ID_UNDO, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_REDO, ChartCanvas::PopupMenuHandler)

	EVT_MENU(ID_DEF_MENU_ACTIVATE_MEASURE, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_DEACTIVATE_MEASURE, ChartCanvas::PopupMenuHandler)

	EVT_MENU(ID_DEF_MENU_CM93OFFSET_DIALOG, ChartCanvas::PopupMenuHandler)

	EVT_MENU(ID_WP_MENU_GOTO, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_WP_MENU_DELPOINT, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_WP_MENU_PROPERTIES, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_WP_MENU_ADDITIONAL_INFO, ChartCanvas::PopupMenuHandler)

	EVT_MENU(ID_TK_MENU_PROPERTIES, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_TK_MENU_DELETE, ChartCanvas::PopupMenuHandler)

	EVT_MENU(ID_DEF_MENU_QUILTREMOVE, ChartCanvas::PopupMenuHandler)

	EVT_MENU(ID_DEF_MENU_TIDEINFO, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_CURRENTINFO, ChartCanvas::PopupMenuHandler)
	EVT_MENU(ID_DEF_MENU_GROUPBASE, ChartCanvas::PopupMenuHandler)
END_EVENT_TABLE()

// Define a constructor for my canvas
ChartCanvas::ChartCanvas(wxFrame* frame)
	: wxWindow(frame, wxID_ANY, wxPoint(20, 20), wxSize(5, 5), wxSIMPLE_BORDER)
	, m_bFollow(false)
	, pCwin(NULL)
	, m_pCIWin(NULL)
	, m_prev_pMousePoint(NULL)
	, m_bShowCurrent(false)
	, m_bShowTide(false)
	, m_bTCupdate(false)
	, m_bDrawingRoute(false)
	, m_bRouteEditing(false)
	, m_bMarkEditing(false)
	, m_pRoutePointEditTarget(NULL)
	, m_pFoundPoint(NULL)
	, m_bChartDragging(false)
	, m_pMouseRoute(NULL)
	, m_pSelectedRoute(NULL)
	, m_pSelectedTrack(NULL)
	, m_pFoundRoutePoint(NULL)
	, m_pFoundRoutePointSecond(NULL)
	, m_bAppendingRoute(false) // was true in MSW, why??
	, pThumbDIBShow(NULL)
	, bShowingCurrent(false)
	, m_bmouse_key_mod(false)
	, parent_frame(NULL)
	, warp_flag(false)
	, pss_overlay_bmp(NULL)
	, pss_overlay_mask(NULL)
	, m_bMeasure_Active(false)
	, m_pMeasureRoute(NULL)
	, m_pRouteRolloverWin(NULL)
	, m_pAISRolloverWin(NULL)
	, m_pos_image_user_day(NULL)
	, m_pos_image_user_dusk(NULL)
	, m_pos_image_user_night(NULL)
	, m_pos_image_user_grey_day(NULL)
	, m_pos_image_user_grey_dusk(NULL)
	, m_pos_image_user_grey_night(NULL)
	, m_pos_image_user_yellow_day(NULL)
	, m_pos_image_user_yellow_dusk(NULL)
	, m_pos_image_user_yellow_night(NULL)
	, m_pRolloverRouteSeg(NULL)
	, m_bbrightdir(false)
	, m_bzooming(false)
	, m_glcc(NULL)
	, m_pGLcontext(NULL)
	, m_bzooming_in(false)
	, m_bzooming_out(false)
	, m_b_paint_enable(true)
	, click_stop(0)
{
	parent_frame = (MainFrame*)frame; // save a pointer to parent

	SetBackgroundColour(global::OCPN::get().color().get_color(_T("NODTA")));
	SetBackgroundStyle(
		wxBG_STYLE_CUSTOM); // on WXMSW, this prevents flashing on color scheme change

	m_zoom_timer.SetOwner(this, ZOOM_TIMER);

	EnableAutoPan(true);

	undo = new Undo;

	VPoint.Invalidate();

#ifdef ocpnUSE_GL
	if (!global::OCPN::get().gui().view().disable_opengl) {
		wxLogMessage(_T("Creating glChartCanvas"));
		m_glcc = new glChartCanvas(this);

#if wxCHECK_VERSION(2, 9, 0)
		m_pGLcontext = new wxGLContext(m_glcc);
		m_glcc->SetContext(m_pGLcontext);
#else
		m_pGLcontext = m_glcc->GetContext();
#endif
	}
#endif

	singleClickEventIsValid = false;

	// Build the cursors

	ocpnStyle::Style& style = g_StyleManager->current();

#if defined(__WXGTK__) || defined(__WXOSX__)
	wxImage ICursorLeft = style.GetIcon(_T("left")).ConvertToImage();
	wxImage ICursorRight = style.GetIcon(_T("right")).ConvertToImage();
	wxImage ICursorUp = style.GetIcon(_T("up")).ConvertToImage();
	wxImage ICursorDown = style.GetIcon(_T("down")).ConvertToImage();
	wxImage ICursorPencil = style.GetIcon(_T("pencil")).ConvertToImage();
	wxImage ICursorCross = style.GetIcon(_T("cross")).ConvertToImage();

	ICursorLeft.ConvertAlphaToMask(128);
	ICursorRight.ConvertAlphaToMask(128);
	ICursorUp.ConvertAlphaToMask(128);
	ICursorDown.ConvertAlphaToMask(128);
	ICursorPencil.ConvertAlphaToMask(10);
	ICursorCross.ConvertAlphaToMask(10);

	if (ICursorLeft.Ok()) {
		ICursorLeft.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
		ICursorLeft.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15);
		pCursorLeft = new wxCursor(ICursorLeft);
	} else
		pCursorLeft = new wxCursor(wxCURSOR_ARROW);

	if (ICursorRight.Ok()) {
		ICursorRight.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 31);
		ICursorRight.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15);
		pCursorRight = new wxCursor(ICursorRight);
	} else
		pCursorRight = new wxCursor(wxCURSOR_ARROW);

	if (ICursorUp.Ok()) {
		ICursorUp.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 15);
		ICursorUp.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
		pCursorUp = new wxCursor(ICursorUp);
	} else
		pCursorUp = new wxCursor(wxCURSOR_ARROW);

	if (ICursorDown.Ok()) {
		ICursorDown.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 15);
		ICursorDown.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 31);
		pCursorDown = new wxCursor(ICursorDown);
	} else
		pCursorDown = new wxCursor(wxCURSOR_ARROW);

	if (ICursorPencil.Ok()) {
		ICursorPencil.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
		ICursorPencil.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
		pCursorPencil = new wxCursor(ICursorPencil);
	} else
		pCursorPencil = new wxCursor(wxCURSOR_ARROW);

	if (ICursorCross.Ok()) {
		ICursorCross.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 13);
		ICursorCross.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 12);
		pCursorCross = new wxCursor(ICursorCross);
	} else
		pCursorCross = new wxCursor(wxCURSOR_ARROW);
#else
	wxImage ICursorLeft = style.GetIcon(_T("left")).ConvertToImage();
	wxImage ICursorRight = style.GetIcon(_T("right")).ConvertToImage();
	wxImage ICursorUp = style.GetIcon(_T("up")).ConvertToImage();
	wxImage ICursorDown = style.GetIcon(_T("down")).ConvertToImage();
	wxImage ICursorPencil = style.GetIcon(_T("pencil")).ConvertToImage();
	wxImage ICursorCross = style.GetIcon(_T("cross")).ConvertToImage();

	if (ICursorLeft.Ok()) {
		ICursorLeft.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
		ICursorLeft.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15);
		pCursorLeft = new wxCursor(ICursorLeft);
	} else
		pCursorLeft = new wxCursor(wxCURSOR_ARROW);

	if (ICursorRight.Ok()) {
		ICursorRight.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 31);
		ICursorRight.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15);
		pCursorRight = new wxCursor(ICursorRight);
	} else
		pCursorRight = new wxCursor(wxCURSOR_ARROW);

	if (ICursorUp.Ok()) {
		ICursorUp.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 15);
		ICursorUp.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 0);
		pCursorUp = new wxCursor(ICursorUp);
	} else
		pCursorUp = new wxCursor(wxCURSOR_ARROW);

	if (ICursorDown.Ok()) {
		ICursorDown.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 15);
		ICursorDown.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 31);
		pCursorDown = new wxCursor(ICursorDown);
	} else
		pCursorDown = new wxCursor(wxCURSOR_ARROW);

	if (ICursorPencil.Ok()) {
		ICursorPencil.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 0);
		ICursorPencil.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 15);
		pCursorPencil = new wxCursor(ICursorPencil);
	} else
		pCursorPencil = new wxCursor(wxCURSOR_ARROW);

	if (ICursorCross.Ok()) {
		ICursorCross.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 13);
		ICursorCross.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 12);
		pCursorCross = new wxCursor(ICursorCross);
	} else
		pCursorCross = new wxCursor(wxCURSOR_ARROW);
#endif // MSW, X11

	pCursorArrow = new wxCursor(wxCURSOR_ARROW);

	SetCursor(*pCursorArrow);

	pPanTimer = new wxTimer(this, PAN_TIMER);
	pPanTimer->Stop();

	pRotDefTimer = new wxTimer(this, ROT_TIMER);
	pRotDefTimer->Stop();

	m_DoubleClickTimer = new wxTimer(this, DBLCLICK_TIMER);
	m_DoubleClickTimer->Stop();

	pPanKeyTimer = new wxTimer(this, PANKEY_TIMER);
	pPanKeyTimer->Stop();
	m_panx = m_pany = 0;
	m_panspeed = 0;

	pCurTrackTimer = new wxTimer(this, CURTRACK_TIMER);
	pCurTrackTimer->Stop();
	m_curtrack_timer_msec = 10;

	m_MouseWheelTimer.SetOwner(this);

	m_RolloverPopupTimer.SetOwner(this, ROPOPUP_TIMER);

	m_rollover_popup_timer_msec = 20;

	m_b_rot_hidef = true;

	// Set up current arrow drawing factors
	int mmx;
	int mmy;
	wxDisplaySizeMM(&mmx, &mmy);

	int sx;
	int sy;
	wxDisplaySize(&sx, &sy);

	m_pix_per_mm = (static_cast<double>(sx)) / (static_cast<double>(mmx));

	int mm_per_knot = 10;
	current_draw_scaler = mm_per_knot * m_pix_per_mm
						  * global::OCPN::get().gui().view().current_arrow_scale / 100.0;
	pscratch_bm = NULL;
	proute_bm = NULL;

	m_prot_bm = NULL;

	// Set some benign initial values

	VPoint.set_position(geo::Position(0, 0));
	VPoint.set_view_scale(1.0);
	VPoint.Invalidate();

	m_canvas_scale_factor = 1.0;

	m_canvas_width = 1000;

	// Create the default world chart
	pWorldBackgroundChart = new GSHHSChart;

	// Create the default depth unit emboss maps
	m_pEM_Feet = NULL;
	m_pEM_Meters = NULL;
	m_pEM_Fathoms = NULL;

	CreateDepthUnitEmbossMaps(global::GLOBAL_COLOR_SCHEME_DAY);

	m_pEM_OverZoom = NULL;
	CreateOZEmbossMapData(global::GLOBAL_COLOR_SCHEME_DAY);

	// Build icons for tide/current points
	m_bmTideDay = style.GetIcon(_T("tidesml"));

	// Dusk
	m_bmTideDusk = CreateDimBitmap(m_bmTideDay, 0.50);

	// Night
	m_bmTideNight = CreateDimBitmap(m_bmTideDay, 0.20);

	// Build Dusk/Night  ownship icons
	double factor_dusk = 0.5;
	double factor_night = 0.25;

	// Red
	m_os_image_red_day = style.GetIcon(_T("ship-red")).ConvertToImage();

	int rimg_width = m_os_image_red_day.GetWidth();
	int rimg_height = m_os_image_red_day.GetHeight();

	m_os_image_red_dusk = m_os_image_red_day.Copy();
	m_os_image_red_night = m_os_image_red_day.Copy();

	for (int iy = 0; iy < rimg_height; iy++) {
		for (int ix = 0; ix < rimg_width; ix++) {
			if (!m_os_image_red_day.IsTransparent(ix, iy)) {
				wxImage::RGBValue rgb(m_os_image_red_day.GetRed(ix, iy),
									  m_os_image_red_day.GetGreen(ix, iy),
									  m_os_image_red_day.GetBlue(ix, iy));
				wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
				hsv.value = hsv.value * factor_dusk;
				wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
				m_os_image_red_dusk.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);

				hsv = wxImage::RGBtoHSV(rgb);
				hsv.value = hsv.value * factor_night;
				nrgb = wxImage::HSVtoRGB(hsv);
				m_os_image_red_night.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);
			}
		}
	}

	// Grey
	m_os_image_grey_day = style.GetIcon(_T("ship-red")).ConvertToImage().ConvertToGreyscale();

	int gimg_width = m_os_image_grey_day.GetWidth();
	int gimg_height = m_os_image_grey_day.GetHeight();

	m_os_image_grey_dusk = m_os_image_grey_day.Copy();
	m_os_image_grey_night = m_os_image_grey_day.Copy();

	for (int iy = 0; iy < gimg_height; iy++) {
		for (int ix = 0; ix < gimg_width; ix++) {
			if (!m_os_image_grey_day.IsTransparent(ix, iy)) {
				wxImage::RGBValue rgb(m_os_image_grey_day.GetRed(ix, iy),
									  m_os_image_grey_day.GetGreen(ix, iy),
									  m_os_image_grey_day.GetBlue(ix, iy));
				wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
				hsv.value = hsv.value * factor_dusk;
				wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
				m_os_image_grey_dusk.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);

				hsv = wxImage::RGBtoHSV(rgb);
				hsv.value = hsv.value * factor_night;
				nrgb = wxImage::HSVtoRGB(hsv);
				m_os_image_grey_night.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);
			}
		}
	}

	// Yellow
	m_os_image_yellow_day = m_os_image_red_day.Copy();

	gimg_width = m_os_image_yellow_day.GetWidth();
	gimg_height = m_os_image_yellow_day.GetHeight();

	m_os_image_yellow_dusk = m_os_image_red_day.Copy();
	m_os_image_yellow_night = m_os_image_red_day.Copy();

	for (int iy = 0; iy < gimg_height; iy++) {
		for (int ix = 0; ix < gimg_width; ix++) {
			if (!m_os_image_yellow_day.IsTransparent(ix, iy)) {
				wxImage::RGBValue rgb(m_os_image_yellow_day.GetRed(ix, iy),
									  m_os_image_yellow_day.GetGreen(ix, iy),
									  m_os_image_yellow_day.GetBlue(ix, iy));
				wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
				hsv.hue += 60. / 360.; // shift to yellow
				wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
				m_os_image_yellow_day.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);

				hsv = wxImage::RGBtoHSV(rgb);
				hsv.value = hsv.value * factor_dusk;
				hsv.hue += 60. / 360.; // shift to yellow
				nrgb = wxImage::HSVtoRGB(hsv);
				m_os_image_yellow_dusk.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);

				hsv = wxImage::RGBtoHSV(rgb);
				hsv.hue += 60. / 360.; // shift to yellow
				hsv.value = hsv.value * factor_night;
				nrgb = wxImage::HSVtoRGB(hsv);
				m_os_image_yellow_night.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);
			}
		}
	}

	// Set initial pointers to ownship images
	m_pos_image_red = &m_os_image_red_day;
	m_pos_image_yellow = &m_os_image_yellow_day;

	// Look for user defined ownship image
	// This may be found in the shared data location along with other user defined icons.
	// and will be called "ownship.xpm" or "ownship.png"
	if (pWayPointMan && pWayPointMan->DoesIconExist(_T("ownship"))) {
		m_pos_image_user_day = new wxImage;
		*m_pos_image_user_day = pWayPointMan->GetIconBitmap(_T("ownship"))->ConvertToImage();
		m_pos_image_user_day->InitAlpha();

		int gimg_width = m_pos_image_user_day->GetWidth();
		int gimg_height = m_pos_image_user_day->GetHeight();

		// Make dusk and night images
		m_pos_image_user_dusk = new wxImage;
		m_pos_image_user_night = new wxImage;

		*m_pos_image_user_dusk = m_pos_image_user_day->Copy();
		*m_pos_image_user_night = m_pos_image_user_day->Copy();

		for (int iy = 0; iy < gimg_height; iy++) {
			for (int ix = 0; ix < gimg_width; ix++) {
				if (!m_pos_image_user_day->IsTransparent(ix, iy)) {
					wxImage::RGBValue rgb(m_pos_image_user_day->GetRed(ix, iy),
										  m_pos_image_user_day->GetGreen(ix, iy),
										  m_pos_image_user_day->GetBlue(ix, iy));
					wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
					hsv.value = hsv.value * factor_dusk;
					wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
					m_pos_image_user_dusk->SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);

					hsv = wxImage::RGBtoHSV(rgb);
					hsv.value = hsv.value * factor_night;
					nrgb = wxImage::HSVtoRGB(hsv);
					m_pos_image_user_night->SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);
				}
			}
		}

		//  Make some alternate greyed out day/dusk/night images
		m_pos_image_user_grey_day = new wxImage;
		*m_pos_image_user_grey_day = m_pos_image_user_day->ConvertToGreyscale();

		m_pos_image_user_grey_dusk = new wxImage;
		m_pos_image_user_grey_night = new wxImage;

		*m_pos_image_user_grey_dusk = m_pos_image_user_grey_day->Copy();
		*m_pos_image_user_grey_night = m_pos_image_user_grey_day->Copy();

		for (int iy = 0; iy < gimg_height; iy++) {
			for (int ix = 0; ix < gimg_width; ix++) {
				if (!m_pos_image_user_grey_day->IsTransparent(ix, iy)) {
					wxImage::RGBValue rgb(m_pos_image_user_grey_day->GetRed(ix, iy),
										  m_pos_image_user_grey_day->GetGreen(ix, iy),
										  m_pos_image_user_grey_day->GetBlue(ix, iy));
					wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
					hsv.value = hsv.value * factor_dusk;
					wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
					m_pos_image_user_grey_dusk->SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);

					hsv = wxImage::RGBtoHSV(rgb);
					hsv.value = hsv.value * factor_night;
					nrgb = wxImage::HSVtoRGB(hsv);
					m_pos_image_user_grey_night->SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);
				}
			}
		}

		//  Make a yellow image for rendering under low accuracy chart conditions
		m_pos_image_user_yellow_day = new wxImage;
		m_pos_image_user_yellow_dusk = new wxImage;
		m_pos_image_user_yellow_night = new wxImage;

		*m_pos_image_user_yellow_day = m_pos_image_user_grey_day->Copy();
		*m_pos_image_user_yellow_dusk = m_pos_image_user_grey_day->Copy();
		*m_pos_image_user_yellow_night = m_pos_image_user_grey_day->Copy();

		for (int iy = 0; iy < gimg_height; iy++) {
			for (int ix = 0; ix < gimg_width; ix++) {
				if (!m_pos_image_user_grey_day->IsTransparent(ix, iy)) {
					wxImage::RGBValue rgb(m_pos_image_user_grey_day->GetRed(ix, iy),
										  m_pos_image_user_grey_day->GetGreen(ix, iy),
										  m_pos_image_user_grey_day->GetBlue(ix, iy));

					//  Simply remove all "blue" from the greyscaled image...
					//  so, what is not black becomes yellow.
					wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
					wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
					m_pos_image_user_yellow_day->SetRGB(ix, iy, nrgb.red, nrgb.green, 0);

					hsv = wxImage::RGBtoHSV(rgb);
					hsv.value = hsv.value * factor_dusk;
					nrgb = wxImage::HSVtoRGB(hsv);
					m_pos_image_user_yellow_dusk->SetRGB(ix, iy, nrgb.red, nrgb.green, 0);

					hsv = wxImage::RGBtoHSV(rgb);
					hsv.value = hsv.value * factor_night;
					nrgb = wxImage::HSVtoRGB(hsv);
					m_pos_image_user_yellow_night->SetRGB(ix, iy, nrgb.red, nrgb.green, 0);
				}
			}
		}
	}

	m_pBrightPopup = NULL;
	m_pQuilt = new Quilt;
}

ChartCanvas::~ChartCanvas()
{
	delete pThumbDIBShow;

	// Delete Cursors
	delete pCursorLeft;
	delete pCursorRight;
	delete pCursorUp;
	delete pCursorDown;
	delete pCursorArrow;
	delete pCursorPencil;
	delete pCursorCross;

	delete pPanTimer;
	delete pCurTrackTimer;
	delete pRotDefTimer;
	delete pPanKeyTimer;
	delete m_DoubleClickTimer;

	delete m_pRouteRolloverWin;
	delete m_pAISRolloverWin;
	delete m_pBrightPopup;

	delete m_pCIWin;

	delete pscratch_bm;

	m_dc_route.SelectObject(wxNullBitmap);
	delete proute_bm;

	delete pWorldBackgroundChart;
	delete pss_overlay_bmp;

	delete m_pEM_Feet;
	delete m_pEM_Meters;
	delete m_pEM_Fathoms;

	delete m_pEM_OverZoom;

	delete m_pQuilt;

	delete m_prot_bm;

	delete m_pos_image_user_day;
	delete m_pos_image_user_dusk;
	delete m_pos_image_user_night;
	delete m_pos_image_user_grey_day;
	delete m_pos_image_user_grey_dusk;
	delete m_pos_image_user_grey_night;
	delete m_pos_image_user_yellow_day;
	delete m_pos_image_user_yellow_dusk;
	delete m_pos_image_user_yellow_night;
	delete undo;
#ifdef ocpnUSE_GL
	if (!global::OCPN::get().gui().view().disable_opengl)
		delete m_glcc;
#endif
}

void ChartCanvas::reset_click_stop()
{
	click_stop = 0;
}

bool ChartCanvas::follow() const
{
	return m_bFollow;
}

void ChartCanvas::set_follow(bool value)
{
	m_bFollow = value;
}

void ChartCanvas::reset_tide_window()
{
	pCwin = NULL;
}

int ChartCanvas::GetCanvasChartNativeScale()
{
	int ret = 1;
	if (!VPoint.is_quilt()) {
		if (Current_Ch)
			ret = Current_Ch->GetNativeScale();
	} else
		ret = (int)m_pQuilt->GetRefNativeScale();

	return ret;
}

chart::ChartBase* ChartCanvas::GetChartAtCursor()
{
	chart::ChartBase* target_chart;
	if (Current_Ch && (Current_Ch->GetChartFamily() == chart::CHART_FAMILY_VECTOR))
		target_chart = Current_Ch;
	else if (VPoint.is_quilt())
		target_chart = cc1->m_pQuilt->GetChartAtPix(wxPoint(mouse_x, mouse_y));
	else
		target_chart = NULL;
	return target_chart;
}

chart::ChartBase* ChartCanvas::GetOverlayChartAtCursor()
{
	chart::ChartBase* target_chart;
	if (VPoint.is_quilt())
		target_chart = cc1->m_pQuilt->GetOverlayChartAtPix(wxPoint(mouse_x, mouse_y));
	else
		target_chart = NULL;
	return target_chart;
}

int ChartCanvas::FindClosestCanvasChartdbIndex(int scale)
{
	int new_dbIndex = -1;
	if (!VPoint.is_quilt()) {
		if (pCurrentStack) {
			for (int i = 0; i < pCurrentStack->nEntry; i++) {
				int sc = ChartData->GetStackChartScale(pCurrentStack, i, NULL, 0);
				if (sc >= scale) {
					new_dbIndex = pCurrentStack->GetDBIndex(i);
					break;
				}
			}
		}
	} else {
		// Using the current quilt, select a useable reference chart
		// Said chart will be in the extended (possibly full-screen) stack,
		// And will have a scale equal to or just greater than the stipulated value
		unsigned int im = m_pQuilt->GetExtendedStackIndexArray().size();
		if (im > 0) {
			using namespace chart;
			for (unsigned int is = 0; is < im; is++) {
				const ChartTableEntry& m
					= ChartData->GetChartTableEntry(m_pQuilt->GetExtendedStackIndexArray().at(is));
				if ((m.GetScale() >= scale) /* && (m_reference_family == m.GetChartFamily())*/) {
					new_dbIndex = m_pQuilt->GetExtendedStackIndexArray().at(is);
					break;
				}
			}
		}
	}

	return new_dbIndex;
}

void ChartCanvas::SetVPRotation(double angle)
{
	VPoint.rotation = angle;
}

double ChartCanvas::GetVPRotation(void) const
{
	return GetVP().rotation;
}

double ChartCanvas::GetVPSkew(void) const
{
	return GetVP().skew;
}

#ifdef ocpnUSE_GL
glChartCanvas* ChartCanvas::GetglCanvas()
{
	return m_glcc;
}
#endif

GSHHSChart* ChartCanvas::GetWorldBackgroundChart()
{
	return pWorldBackgroundChart;
}

void ChartCanvas::SetbTCUpdate(bool f)
{
	m_bTCupdate = f;
}

bool ChartCanvas::GetbTCUpdate() const
{
	return m_bTCupdate;
}

void ChartCanvas::SetbShowCurrent(bool f)
{
	m_bShowCurrent = f;
}

bool ChartCanvas::GetbShowCurrent() const
{
	return m_bShowCurrent;
}

void ChartCanvas::SetbShowTide(bool f)
{
	m_bShowTide = f;
}

bool ChartCanvas::GetbShowTide() const
{
	return m_bShowTide;
}

double ChartCanvas::GetPixPerMM() const
{
	return m_pix_per_mm;
}

void ChartCanvas::SetOwnShipState(ownship_state_t state)
{
	m_ownship_state = state;
}

int ChartCanvas::GetCanvasWidth() const
{
	return m_canvas_width;
}

int ChartCanvas::GetCanvasHeight() const
{
	return m_canvas_height;
}

float ChartCanvas::GetVPScale() const
{
	return GetVP().view_scale();
}

float ChartCanvas::GetVPChartScale() const
{
	return GetVP().chart_scale;
}

double ChartCanvas::GetCanvasScaleFactor() const
{
	return m_canvas_scale_factor;
}

double ChartCanvas::GetCanvasTrueScale() const
{
	return m_true_scale_ppm;
}

double ChartCanvas::GetAbsoluteMinScalePpm() const
{
	return m_absolute_min_scale_ppm;
}

void ChartCanvas::EnablePaint(bool b_enable)
{
	m_b_paint_enable = b_enable;
#ifdef ocpnUSE_GL
	if (m_glcc)
		m_glcc->EnablePaint(b_enable);
#endif
}

bool ChartCanvas::IsQuiltDelta()
{
	return m_pQuilt->IsQuiltDelta(VPoint);
}

std::vector<int> ChartCanvas::GetQuiltIndexArray(void)
{
	return m_pQuilt->GetQuiltIndexArray();
}

void ChartCanvas::SetQuiltMode(bool quilt)
{
	VPoint.set_quilt(quilt, global::OCPN::get().gui().view().fullscreen_quilt);
}

bool ChartCanvas::GetQuiltMode(void) const
{
	return VPoint.is_quilt();
}

int ChartCanvas::GetQuiltReferenceChartIndex(void)
{
	return m_pQuilt->GetRefChartdbIndex();
}

void ChartCanvas::InvalidateAllQuiltPatchs(void)
{
	m_pQuilt->InvalidateAllQuiltPatchs();
}

chart::ChartBase* ChartCanvas::GetLargestScaleQuiltChart()
{
	return m_pQuilt->GetLargestScaleChart();
}

chart::ChartBase* ChartCanvas::GetFirstQuiltChart()
{
	return m_pQuilt->GetFirstChart();
}

chart::ChartBase* ChartCanvas::GetNextQuiltChart()
{
	return m_pQuilt->GetNextChart();
}

int ChartCanvas::GetQuiltChartCount()
{
	return m_pQuilt->GetnCharts();
}

void ChartCanvas::SetQuiltChartHiLiteIndex(int dbIndex)
{
	m_pQuilt->SetHiliteIndex(dbIndex);
}

std::vector<int> ChartCanvas::GetQuiltCandidatedbIndexArray(bool flag1, bool flag2)
{
	return m_pQuilt->GetCandidatedbIndexArray(flag1, flag2);
}

int ChartCanvas::GetQuiltRefChartdbIndex(void)
{
	return m_pQuilt->GetRefChartdbIndex();
}

std::vector<int> ChartCanvas::GetQuiltExtendedStackdbIndexArray()
{
	return m_pQuilt->GetExtendedStackIndexArray();
}

std::vector<int> ChartCanvas::GetQuiltEclipsedStackdbIndexArray()
{
	return m_pQuilt->GetEclipsedStackIndexArray();
}

void ChartCanvas::InvalidateQuilt(void)
{
	return m_pQuilt->Invalidate();
}

double ChartCanvas::GetQuiltMaxErrorFactor()
{
	return m_pQuilt->GetMaxErrorFactor();
}

bool ChartCanvas::IsChartQuiltableRef(int db_index) const
{
	return m_pQuilt->IsChartQuiltableRef(db_index);
}

bool ChartCanvas::IsChartLargeEnoughToRender(chart::ChartBase* chart, const ViewPort& vp) const
{
	const double chart_not_render_scale_factor = global::OCPN::get().gui().view().ChartNotRenderScaleFactor;
	const double chartMaxScale = chart->GetNormalScaleMax(GetCanvasScaleFactor(), GetCanvasWidth());
	return chartMaxScale * chart_not_render_scale_factor > vp.chart_scale;
}

void ChartCanvas::CancelMeasureRoute()
{
	m_bMeasure_Active = false;
	m_nMeasureState = 0;
	global::OCPN::get().routeman().DeleteRoute(m_pMeasureRoute);
	m_pMeasureRoute = NULL;
}

const ViewPort& ChartCanvas::GetVP() const
{
	return VPoint;
}

void ChartCanvas::set_prev_mouse_point(RoutePoint* route_point)
{
	m_prev_pMousePoint = route_point;
	m_prev_route = m_prev_pMousePoint->get_position();
}

void ChartCanvas::invalidate_undo()
{
	undo->InvalidateUndo();
}

void ChartCanvas::invalidate_redo()
{
	undo->InvalidateRedo();
}

Undo& ChartCanvas::get_undo()
{
	return *undo;
}

const wxCursor& ChartCanvas::get_cursor_pencil() const
{
	return *pCursorPencil;
}

const wxCursor& ChartCanvas::get_cursor_arrow() const
{
	return *pCursorArrow;
}

const wxCursor& ChartCanvas::get_cursor_cross() const
{
	return *pCursorCross;
}

void ChartCanvas::OnKeyDown(wxKeyEvent& event)
{
	m_panx = 0; //  Stop any autopanning
	m_pany = 0;

	m_modkeys = event.GetModifiers();

	if (event.GetKeyCode() == WXK_CONTROL)
		m_bmouse_key_mod = true;

	// HOTKEYS
	switch (event.GetKeyCode()) {
		case WXK_LEFT:
			if (m_modkeys == wxMOD_CONTROL)
				parent_frame->DoStackDown();
			else
				m_panx = -1;
			break;

		case WXK_UP:
			m_pany = -1;
			break;

		case WXK_RIGHT:
			if (m_modkeys == wxMOD_CONTROL)
				parent_frame->DoStackUp();
			else
				m_panx = 1;
			break;

		case WXK_DOWN:
			m_pany = 1;
			break;

		case WXK_F2:
			parent_frame->TogglebFollow();
			break;

		case WXK_F3: {
			parent_frame->ToggleENCText();
			break;
		}
		case WXK_F4:
			if (!parent_frame->nRoute_State) // no measure tool if currently creating route
			{
				if (m_bMeasure_Active) {
					global::OCPN::get().routeman().DeleteRoute(m_pMeasureRoute);
					m_pMeasureRoute = NULL;
				}

				m_bMeasure_Active = true;
				m_nMeasureState = 1;
				SetCursor(*pCursorPencil);
				Refresh();
			}
			break;

		case WXK_F5:
			parent_frame->ToggleColorScheme();
			break;

		case WXK_F6: {
			int mod = m_modkeys & wxMOD_SHIFT;
			if (mod != m_brightmod) {
				m_brightmod = mod;
				m_bbrightdir = !m_bbrightdir;
			}

			global::GUI& gui = global::OCPN::get().gui();
			int brightness = gui.view().screen_brightness;

			if (!m_bbrightdir) {
				brightness -= 10;
				if (brightness <= MIN_BRIGHT) {
					brightness = MIN_BRIGHT;
					m_bbrightdir = true;
				}
			} else {
				brightness += 10;
				if (brightness >= MAX_BRIGHT) {
					brightness = MAX_BRIGHT;
					m_bbrightdir = false;
				}
			}
			gui.set_view_screen_brightness(brightness);

			SetScreenBrightness(brightness);
			ShowBrightnessLevelTimedPopup(brightness / 10, 1, 10);

			SetFocus(); // just in case the external program steals it....
			gFrame->Raise(); // And reactivate the application main

			break;
		}

		case WXK_F7:
			parent_frame->DoStackDown();
			break;

		case WXK_F8:
			parent_frame->DoStackUp();
			break;

		case WXK_F9: {
			parent_frame->ToggleQuiltMode();
			ReloadVP();
			break;
		}

		case WXK_F11:
			parent_frame->ToggleFullScreen();
			break;

		case WXK_F12:
			parent_frame->ToggleChartOutlines();
			break;

		// NUMERIC PAD
		case WXK_NUMPAD_ADD: // '+' on NUM PAD
			if (m_modkeys == wxMOD_CONTROL)
				ZoomCanvasIn(1.1);
			else
				ZoomCanvasIn(2.0);
			break;

		case WXK_NUMPAD_SUBTRACT: // '-' on NUM PAD
			if (m_modkeys == wxMOD_CONTROL)
				ZoomCanvasOut(1.1);
			else
				ZoomCanvasOut(2.0);
			break;

		default:
			break;
	}

	if (event.GetKeyCode() < 128) // ascii
	{
		char key_char = (char)event.GetKeyCode();
		if (m_modkeys == wxMOD_CONTROL)
			key_char -= 64;

		// Handle both QWERTY and AZERTY keyboard separately for a few control codes
		if (!g_b_assume_azerty) {
			switch (key_char) {
				case '+':
				case '+' - 64:
				case '=':
				case '=' - 64: // Ctrl =
					if (m_modkeys == wxMOD_CONTROL)
						ZoomCanvasIn(1.1);
					else
						ZoomCanvasIn(2.0);
					break;

				case '-':
				case '-' - 64: // Ctrl -
				case '_':
					if (m_modkeys == wxMOD_CONTROL)
						ZoomCanvasOut(1.1);
					else
						ZoomCanvasOut(2.0);
					break;
			}
		} else {
			switch (key_char) {
				case 43:
				case -21:
					if (m_modkeys == wxMOD_CONTROL)
						ZoomCanvasIn(1.1);
					else
						ZoomCanvasIn(2.0);
					break;

				case 54: // '-'  alpha/num pad
				case 56: // '_'  alpha/num pad
				case -10: // Ctrl '-'  alpha/num pad
				case -8: // Ctrl '_' alpha/num pad
					if (m_modkeys == wxMOD_CONTROL)
						ZoomCanvasOut(1.1);
					else
						ZoomCanvasOut(2.0);
					break;
			}
		}

		using namespace chart;
		switch (key_char) {
			case 'A':
				parent_frame->ToggleAnchor();
				break;

			case 'D': {
				int x, y;
				event.GetPosition(&x, &y);
				bool cm93IsAvailable
					= (Current_Ch && (Current_Ch->GetChartType() == CHART_TYPE_CM93COMP));
				if (VPoint.is_quilt()) {
					ChartBase* pChartTest = m_pQuilt->GetChartAtPix(wxPoint(x, y));
					if (pChartTest) {
						if (pChartTest->GetChartType() == CHART_TYPE_CM93)
							cm93IsAvailable = true;
						if (pChartTest->GetChartType() == CHART_TYPE_CM93COMP)
							cm93IsAvailable = true;
					}
				}

				if (cm93IsAvailable) {
					if (!pCM93DetailSlider) {
#define CM93_ZOOM_FACTOR_MAX_RANGE 5 // FIXME: better solution (maybe over global infrastructure)
						pCM93DetailSlider = new CM93DSlide(
							this, -1, 0, -CM93_ZOOM_FACTOR_MAX_RANGE, CM93_ZOOM_FACTOR_MAX_RANGE,
							global::OCPN::get().gui().cm93().detail_dialog_position, wxDefaultSize,
							wxSIMPLE_BORDER, _("CM93 Detail Level"));
					}
					pCM93DetailSlider->Show(!pCM93DetailSlider->IsShown());
				}
				break;
			}

			case 'L':
				parent_frame->ToggleLights();
				break;

			case 'O':
				parent_frame->ToggleChartOutlines();
				break;

			case 'S':
				parent_frame->ToggleSoundings();
				break;

			case 'T':
				parent_frame->ToggleENCText();
				break;

			case 1: // Ctrl A
				parent_frame->TogglebFollow();
				break;

			case 2: // Ctrl B
				if (stats) {
					if (stats->IsShown())
						stats->Hide();
					else {
						stats->Move(0, 0);
						stats->RePosition();
						stats->Show();
						gFrame->Raise();
					}
					Refresh();
				}
				break;

			case 13: // Ctrl M // Drop Marker at cursor
			{
				const global::GUI::View& view = global::OCPN::get().gui().view();
				RoutePoint* pWP = new RoutePoint(m_cursor_pos, view.default_wp_icon, wxEmptyString);
				pWP->m_bIsolatedMark = true; // This is an isolated mark
				pSelect->AddSelectableRoutePoint(m_cursor_pos, pWP);
				pConfig->AddNewWayPoint(pWP, -1); // use auto next num

				if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
					pRouteManagerDialog->UpdateWptListCtrl();
				undo->BeforeUndoableAction(UndoAction::Undo_CreateWaypoint, pWP,
										   UndoAction::Undo_HasParent, NULL);
				undo->AfterUndoableAction(NULL);
				Refresh(false);
				break;
			}

			case 14: // Ctrl N - Activate next waypoint in a route
			{
				if (Route* r = global::OCPN::get().routeman().GetpActiveRoute()) {
					int indexActive = r->GetIndexOf(r->m_pRouteActivePoint);
					if ((indexActive + 1) <= r->GetnPoints()) {
						global::OCPN::get().routeman().ActivateNextPoint(r, true);
						Refresh(false);
					}
				}
				break;
			}

			case 15: // Ctrl O - Drop Marker at boat's position
			{
				const global::GUI::View& view = global::OCPN::get().gui().view();
				const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
				RoutePoint* pWP = new RoutePoint(nav.pos, view.default_wp_icon, wxEmptyString);
				pWP->m_bIsolatedMark = true; // This is an isolated mark
				pSelect->AddSelectableRoutePoint(nav.pos, pWP);
				pConfig->AddNewWayPoint(pWP, -1); // use auto next num

				if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
					pRouteManagerDialog->UpdateWptListCtrl();
				undo->BeforeUndoableAction(UndoAction::Undo_CreateWaypoint, pWP,
										   UndoAction::Undo_HasParent, NULL);
				undo->AfterUndoableAction(NULL);
				Refresh(false);
				break;
			}

			case -32: // Ctrl Space: Drop MOB
				if (m_modkeys == wxMOD_CONTROL)
					parent_frame->ActivateMOB();
				break;

			case 17: // Ctrl Q
				parent_frame->Close();
				return;

			case 18: // Ctrl R
				gFrame->nRoute_State = 1;
				cc1->SetCursor(*cc1->pCursorPencil);
				return;

			case 20: // Ctrl T
				if (NULL == pGoToPositionDialog) // There is one global instance of the GoToPositionDialog
					pGoToPositionDialog = new GoToPositionDialog(this);
				pGoToPositionDialog->Show();
				break;

			case 25: // Ctrl Y
				if (undo->AnythingToRedo()) {
					undo->RedoNextAction();
					Refresh(false);
				}
				break;

			case 26: // Ctrl Z
				if (undo->AnythingToUndo()) {
					undo->UndoLastAction();
					Refresh(false);
				}
				break;

			case 27:
				// Generic break
				if (m_bMeasure_Active) {
					m_bMeasure_Active = false;
					m_nMeasureState = 0;
					global::OCPN::get().routeman().DeleteRoute(m_pMeasureRoute);
					m_pMeasureRoute = NULL;
					gFrame->SurfaceToolbar();
					Refresh(false);
				}

				if (parent_frame->nRoute_State) { // creating route?
					FinishRoute();
					gFrame->SurfaceToolbar();
					Refresh(false);
				}

				break;

			case 7: // Ctrl G
				switch (gamma_state) {
					case 0:
						r_gamma_mult = 0;
						g_gamma_mult = 1;
						b_gamma_mult = 0;
						gamma_state = 1;
						break;
					case 1:
						r_gamma_mult = 1;
						g_gamma_mult = 0;
						b_gamma_mult = 0;
						gamma_state = 2;
						break;
					case 2:
						r_gamma_mult = 1;
						g_gamma_mult = 1;
						b_gamma_mult = 1;
						gamma_state = 0;
						break;
				}
				SetScreenBrightness(global::OCPN::get().gui().view().screen_brightness);

				break;

			case 9: // Ctrl I
				if (g_FloatingCompassDialog) {
					if (g_FloatingCompassDialog->IsShown()) {
						g_FloatingCompassDialog->Hide();
					} else {
						g_FloatingCompassDialog->Show();
					}
					gFrame->Raise();
					Refresh();
				}
				break;

			default:
				return;

		} // switch
	}

	if (m_benable_autopan && !pPanKeyTimer->IsRunning() && (m_panx || m_pany))
		pPanKeyTimer->Start(1, wxTIMER_ONE_SHOT);

#ifndef __WXMAC__
	event.Skip();
#endif
}

void ChartCanvas::OnKeyUp(wxKeyEvent& event)
{
	switch (event.GetKeyCode()) {
		case WXK_LEFT:
		case WXK_RIGHT:
			m_panx = 0;
			m_panspeed = 0;
			break;

		case WXK_UP:
		case WXK_DOWN:
			m_pany = 0;
			m_panspeed = 0;
			break;

		case WXK_CONTROL:
			m_modkeys = wxMOD_NONE; // Clear Ctrl key
			m_bmouse_key_mod = false;

			break;
	}
	event.Skip();
}

void ChartCanvas::Do_Pankeys(wxTimerEvent&)
{
	if (!(m_panx || m_pany))
		return;

	if (!m_benable_autopan)
		return;

	const int slowpan = 2, maxpan = 100;
	int repeat = 100;

	if (m_modkeys == wxMOD_ALT)
		m_panspeed = slowpan;
	else if (global::OCPN::get().gui().view().smooth_pan_zoom) {
		// accelerate panning
		m_panspeed += 2;
		if (m_panspeed > maxpan)
			m_panspeed = maxpan;

		repeat = 5;
	} else
		m_panspeed = maxpan;

	PanCanvas(m_panspeed * m_panx, m_panspeed * m_pany);
	pPanKeyTimer->Start(repeat, wxTIMER_ONE_SHOT);
}

void ChartCanvas::SetColorScheme(global::ColorScheme cs)
{
	// Setup ownship image pointers
	switch (cs) {
		case global::GLOBAL_COLOR_SCHEME_DAY:
			m_pos_image_red = &m_os_image_red_day;
			m_pos_image_grey = &m_os_image_grey_day;
			m_pos_image_yellow = &m_os_image_yellow_day;
			m_pos_image_user = m_pos_image_user_day;
			m_pos_image_user_grey = m_pos_image_user_grey_day;
			m_pos_image_user_yellow = m_pos_image_user_yellow_day;
			break;
		case global::GLOBAL_COLOR_SCHEME_DUSK:
			m_pos_image_red = &m_os_image_red_dusk;
			m_pos_image_grey = &m_os_image_grey_dusk;
			m_pos_image_yellow = &m_os_image_yellow_dusk;
			m_pos_image_user = m_pos_image_user_dusk;
			m_pos_image_user_grey = m_pos_image_user_grey_dusk;
			m_pos_image_user_yellow = m_pos_image_user_yellow_dusk;
			break;
		case global::GLOBAL_COLOR_SCHEME_NIGHT:
			m_pos_image_red = &m_os_image_red_night;
			m_pos_image_grey = &m_os_image_grey_night;
			m_pos_image_yellow = &m_os_image_yellow_night;
			m_pos_image_user = m_pos_image_user_night;
			m_pos_image_user_grey = m_pos_image_user_grey_night;
			m_pos_image_user_yellow = m_pos_image_user_yellow_night;
			break;
		default:
			m_pos_image_red = &m_os_image_red_day;
			m_pos_image_grey = &m_os_image_grey_day;
			m_pos_image_yellow = &m_os_image_yellow_day;
			m_pos_image_user = m_pos_image_user_day;
			m_pos_image_user_grey = m_pos_image_user_grey_day;
			m_pos_image_user_yellow = m_pos_image_user_yellow_day;
			break;
	}

	CreateDepthUnitEmbossMaps(cs);
	CreateOZEmbossMapData(cs);

#ifdef ocpnUSE_GL
	if (global::OCPN::get().gui().view().opengl && m_glcc)
		m_glcc->ClearAllRasterTextures();
#endif

	SetbTCUpdate(true); // force re-render of tide/current locators
	ReloadVP();
}

wxBitmap ChartCanvas::CreateDimBitmap(wxBitmap& Bitmap, double factor)
{
	wxImage img = Bitmap.ConvertToImage();
	const int sx = img.GetWidth();
	const int sy = img.GetHeight();

	wxImage new_img(img);

	for (int i = 0; i < sx; i++) {
		for (int j = 0; j < sy; j++) {
			if (!img.IsTransparent(i, j)) {
				new_img.SetRGB(i, j, (unsigned char)(img.GetRed(i, j) * factor),
							   (unsigned char)(img.GetGreen(i, j) * factor),
							   (unsigned char)(img.GetBlue(i, j) * factor));
			}
		}
	}

	return wxBitmap(new_img);
}

void ChartCanvas::ShowBrightnessLevelTimedPopup(int brightness, int min, int max)
{
	wxFont* pfont = wxTheFontList->FindOrCreateFont(40, wxDEFAULT, wxNORMAL, wxBOLD);

	if (!m_pBrightPopup) {
		// Calculate size
		int x;
		int y;
		GetTextExtent(_T("MAX"), &x, &y, NULL, NULL, pfont);

		m_pBrightPopup = new TimedPopupWin(this, 3);

		m_pBrightPopup->SetSize(x, y);
		m_pBrightPopup->Move(120, 120);
	}

	int bmpsx = m_pBrightPopup->GetSize().x;
	int bmpsy = m_pBrightPopup->GetSize().y;

	wxBitmap bmp(bmpsx, bmpsx);
	wxMemoryDC mdc(bmp);

	const global::ColorManager& colors = global::OCPN::get().color();

	mdc.SetTextForeground(colors.get_color(_T("GREEN4")));
	mdc.SetBackground(wxBrush(colors.get_color(_T("UINFD"))));
	mdc.SetPen(wxPen(wxColour(0, 0, 0)));
	mdc.SetBrush(wxBrush(colors.get_color(_T("UINFD"))));
	mdc.Clear();

	mdc.DrawRectangle(0, 0, bmpsx, bmpsy);

	mdc.SetFont(*pfont);
	wxString val;

	if (brightness == max)
		val = _T("MAX");
	else if (brightness == min)
		val = _T("MIN");
	else
		val.Printf(_T("%3d"), brightness);

	mdc.DrawText(val, 0, 0);

	mdc.SelectObject(wxNullBitmap);

	m_pBrightPopup->SetBitmap(bmp);
	m_pBrightPopup->Show();
	m_pBrightPopup->Refresh();
}

void ChartCanvas::RotateTimerEvent(wxTimerEvent&)
{
	m_b_rot_hidef = true;
	ReloadVP();
}

void ChartCanvas::OnRolloverPopupTimerEvent(wxTimerEvent&)
{
	const global::GUI::View& view = global::OCPN::get().gui().view();

	bool b_need_refresh = false;

	// Handle the AIS Rollover Window first
	bool showAISRollover = false;
	if (g_pAIS && g_pAIS->GetNumTargets() && view.ShowAIS) {
		SelectItem* pFind
			= pSelectAIS->FindSelection(m_cursor_pos, SelectItem::TYPE_AISTARGET);
		if (pFind) {
			int FoundAIS_MMSI
				= (long)pFind->m_pData1; // cast to long avoids problems with 64bit compilers
			AIS_Target_Data* ptarget = g_pAIS->Get_Target_Data_From_MMSI(FoundAIS_MMSI);

			if (ptarget) {
				showAISRollover = true;

				if (NULL == m_pAISRolloverWin) {
					m_pAISRolloverWin = new RolloverWin(this, 10);
					m_pAISRolloverWin->IsActive(false);
					b_need_refresh = true;
				}

				// Sometimes the mouse moves fast enough to get over a new AIS target before
				// the one-shot has fired to remove the old target.
				// Result:  wrong target data is shown.
				// Detect this case,close the existing rollover ASAP, and restart the timer.
				if (m_pAISRolloverWin && m_pAISRolloverWin->IsActive() && m_AISRollover_MMSI
					&& (m_AISRollover_MMSI != FoundAIS_MMSI)) {
					m_RolloverPopupTimer.Start(50, wxTIMER_ONE_SHOT);
					m_pAISRolloverWin->IsActive(false);
					m_AISRollover_MMSI = 0;
					Refresh();
					return;
				}

				m_AISRollover_MMSI = FoundAIS_MMSI;

				if (!m_pAISRolloverWin->IsActive()) {

					wxString s = ptarget->GetRolloverString();
					m_pAISRolloverWin->SetString(s);

					wxSize win_size = GetSize();
					if (console->IsShown())
						win_size.x -= console->GetSize().x;
					m_pAISRolloverWin->SetBestPosition(mouse_x, mouse_y, 16, 16,
													   RolloverWin::AIS_ROLLOVER, win_size);

					m_pAISRolloverWin->SetBitmap(RolloverWin::AIS_ROLLOVER);
					m_pAISRolloverWin->IsActive(true);
					b_need_refresh = true;
				}
			}
		} else {
			m_AISRollover_MMSI = 0;
			showAISRollover = false;
		}
	}

	//  Maybe turn the rollover off
	if (m_pAISRolloverWin && m_pAISRolloverWin->IsActive() && !showAISRollover) {
		m_pAISRolloverWin->IsActive(false);
		m_AISRollover_MMSI = 0;
		b_need_refresh = true;
	}

	// Now the Route info rollover
	// Show the route segment info
	bool showRollover = false;

	if (NULL == m_pRolloverRouteSeg) {
		// Get a list of all selectable sgements, and search for the first visible segment as the
		// rollover target.

		SelectableItemList SelList
			= pSelect->FindSelectionList(m_cursor_pos, SelectItem::TYPE_ROUTESEGMENT);
		SelectableItemList::iterator index = SelList.begin();
		while (index != SelList.end()) {
			SelectItem* pFindSel = *index;

			Route* pr = pFindSel->route; // candidate

			if (pr && pr->IsVisible()) {
				m_pRolloverRouteSeg = pFindSel;
				showRollover = true;

				if (NULL == m_pRouteRolloverWin) {
					m_pRouteRolloverWin = new RolloverWin(this);
					m_pRouteRolloverWin->IsActive(false);
				}

				if (!m_pRouteRolloverWin->IsActive()) {
					wxString s;
					RoutePoint* segShow_point_a = (RoutePoint*)m_pRolloverRouteSeg->m_pData1;
					RoutePoint* segShow_point_b = m_pRolloverRouteSeg->route_point;

					double brg;
					double dist;
					geo::DistanceBearingMercator(segShow_point_b->get_position(),
												 segShow_point_a->get_position(), &brg, &dist);

					if (!pr->m_bIsInLayer)
						s.Append(_("Route: "));
					else
						s.Append(_("Layer Route: "));

					if (pr->get_name().IsEmpty())
						s.Append(_("(unnamed)"));
					else
						s.Append(pr->get_name());

					s << _T("\n") << _("Total Length: ")
					  << FormatDistanceAdaptive(pr->m_route_length) << _T("\n") << _("Leg: from ")
					  << segShow_point_a->GetName() << _(" to ") << segShow_point_b->GetName()
					  << _T("\n");

					if (global::OCPN::get().gui().view().ShowMag) // FIXME: code duplication
						s << wxString::Format(wxString("%03d°(M)  ", wxConvUTF8),
											  (int)navigation::GetTrueOrMag(brg));
					else
						s << wxString::Format(wxString("%03d°  ", wxConvUTF8),
											  (int)navigation::GetTrueOrMag(brg));

					s << FormatDistanceAdaptive(dist);

					// Compute and display cumulative distance from route start point to current
					// leg end point.

					if (segShow_point_a != pr->routepoints().front()) {
						float dist_to_endleg = 0;
						RoutePointList::iterator node = pr->routepoints().begin();
						++node;
						for (; node != pr->routepoints().end(); ++node) {
							RoutePoint* prp = *node;
							dist_to_endleg += prp->m_seg_len;
							if (prp->IsSame(segShow_point_a))
								break;
						}
						s << _T(" (+") << FormatDistanceAdaptive(dist_to_endleg) << _T(")");
					}

					m_pRouteRolloverWin->SetString(s);

					wxSize win_size = GetSize();
					if (console->IsShown())
						win_size.x -= console->GetSize().x;
					m_pRouteRolloverWin->SetBestPosition(mouse_x, mouse_y, 16, 16,
														 RolloverWin::LEG_ROLLOVER, win_size);
					m_pRouteRolloverWin->SetBitmap(RolloverWin::LEG_ROLLOVER);
					m_pRouteRolloverWin->IsActive(true);
					b_need_refresh = true;
					showRollover = true;
					break;
				}
			} else {
				++index;
			}
		}
	} else {
		// Is the cursor still in select radius?
		if (!pSelect->IsSelectableSegmentSelected(m_cursor_pos, m_pRolloverRouteSeg))
			showRollover = false;
		else
			showRollover = true;
	}

	// If currently creating a route, do not show this rollover window
	if (parent_frame->nRoute_State)
		showRollover = false;

	// Similar for AIS target rollover window
	if (m_pAISRolloverWin && m_pAISRolloverWin->IsActive())
		showRollover = false;

	if (m_pRouteRolloverWin && m_pRouteRolloverWin->IsActive() && !showRollover) {
		m_pRouteRolloverWin->IsActive(false);
		m_pRolloverRouteSeg = NULL;
		m_pRouteRolloverWin->Destroy();
		m_pRouteRolloverWin = NULL;
		b_need_refresh = true;
	} else if (m_pRouteRolloverWin && showRollover) {
		m_pRouteRolloverWin->IsActive(true);
		b_need_refresh = true;
	}

	if (b_need_refresh)
		Refresh();
}

void ChartCanvas::OnCursorTrackTimerEvent(wxTimerEvent&)
{
#ifdef USE_S57
	if (s57_CheckExtendedLightSectors(mouse_x, mouse_y, VPoint, extendedSectorLegs))
		ReloadVP(false);
#endif

#ifdef __WXGTK__
	// This is here because GTK status window update is expensive.. Why??
	// Anyway, only update the status bar when this timer expires

	// Check the absolute range of the cursor position
	// There could be a window wherein the chart geoereferencing is not valid....
	geo::Position cursor = cc1->GetCanvasPixPoint(mouse_x, mouse_y);

	if ((fabs(cursor.lat()) < 90.0) && (fabs(cursor.lon()) < 360.0)) {
		cursor.normalize_lon();

		if (parent_frame->hasStatusBar()) {
			wxString s1;
			s1 += _T(" ");
			s1 += toSDMM(1, cursor.lat());
			s1 += _T("   ");
			s1 += toSDMM(2, cursor.lon());
			parent_frame->SetStatusText(s1, STAT_FIELD_CURSOR_LL);

			double brg;
			double dist;
			wxString s;
			const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
			geo::DistanceBearingMercator(cursor, nav.pos, &brg, &dist);
			if (global::OCPN::get().gui().view().ShowMag) // FIXME: code duplication
				s.Printf(wxString("%03d°(M)  ", wxConvUTF8), (int)navigation::GetTrueOrMag(brg));
			else
				s.Printf(wxString("%03d°  ", wxConvUTF8), (int)navigation::GetTrueOrMag(brg));

			s << FormatDistanceAdaptive(dist);
			parent_frame->SetStatusText(s, STAT_FIELD_CURSOR_BRGRNG);
		}
	}

#endif
}

geo::Position ChartCanvas::GetCursorLatLon()
{
	return GetCanvasPixPoint(mouse_x, mouse_y);
}

wxPoint ChartCanvas::GetCanvasPointPix(const geo::Position& pos)
{
	wxPoint r;

	// If the Current Chart is a raster chart, and the
	// requested lat/long is within the boundaries of the chart,
	// and the VP is not rotated,
	// then use the embedded BSB chart georeferencing algorithm
	// for greater accuracy
	// Additionally, use chart embedded georef if the projection is TMERC
	//  i.e. NOT MERCATOR and NOT POLYCONIC

	// If for some reason the chart rejects the request by returning an error,
	// then fall back to Viewport Projection estimate from canvas parameters
	bool bUseVP = true;

	if (Current_Ch && (Current_Ch->GetChartFamily() == chart::CHART_FAMILY_RASTER)
		&& (((fabs(GetVP().rotation) < 0.01) && !global::OCPN::get().gui().view().skew_comp)
			|| ((Current_Ch->GetChartProjectionType() != PROJECTION_MERCATOR)
				&& (Current_Ch->GetChartProjectionType() != PROJECTION_POLYCONIC)))) {
		chart::ChartBaseBSB* Cur_BSB_Ch = dynamic_cast<chart::ChartBaseBSB*>(Current_Ch);
		if (Cur_BSB_Ch) {
			// This is a Raster chart....
			// If the VP is changing, the raster chart parameters may not yet be setup
			// So do that before accessing the chart's embedded georeferencing
			Cur_BSB_Ch->SetVPRasterParms(GetVP());
			int rpixxd, rpixyd;
			if (0 == Cur_BSB_Ch->latlong_to_pix_vp(pos.lat(), pos.lon(), rpixxd, rpixyd, GetVP())) {
				r.x = rpixxd;
				r.y = rpixyd;
				bUseVP = false;
			}
		}
	}

	// if needed, use the VPoint scaling estimator,
	if (bUseVP) {
		r = GetVP().GetPixFromLL(pos);
	}

	return r;
}

geo::Position ChartCanvas::GetCanvasPixPoint(int x, int y)
{
	double lat;
	double lon;

	// If the Current Chart is a raster chart, and the
	// requested x,y is within the boundaries of the chart,
	// and the VP is not rotated,
	// then use the embedded BSB chart georeferencing algorithm
	// for greater accuracy
	// Additionally, use chart embedded georef if the projection is TMERC
	//  i.e. NOT MERCATOR and NOT POLYCONIC

	// If for some reason the chart rejects the request by returning an error,
	// then fall back to Viewport Projection  estimate from canvas parameters
	bool bUseVP = true;

	if (Current_Ch && (Current_Ch->GetChartFamily() == chart::CHART_FAMILY_RASTER)
		&& (((fabs(GetVP().rotation) < 0.01) && !global::OCPN::get().gui().view().skew_comp)
			|| ((Current_Ch->GetChartProjectionType() != PROJECTION_MERCATOR)
				&& (Current_Ch->GetChartProjectionType() != PROJECTION_POLYCONIC)))) {
		chart::ChartBaseBSB* Cur_BSB_Ch = dynamic_cast<chart::ChartBaseBSB*>(Current_Ch);

		// TODO     maybe need iterative process to validate bInside
		//          first pass is mercator, then check chart boundaries

		if (Cur_BSB_Ch) {
			// This is a Raster chart....
			// If the VP is changing, the raster chart parameters may not yet be setup
			// So do that before accessing the chart's embedded georeferencing
			Cur_BSB_Ch->SetVPRasterParms(GetVP());

			double slat;
			double slon;
			if (0 == Cur_BSB_Ch->vp_pix_to_latlong(GetVP(), x, y, &slat, &slon)) {
				lat = slat;

				if (slon < -180.0)
					slon += 360.0;
				else if (slon > 180.0)
					slon -= 360.0;

				lon = slon;
				bUseVP = false;
			}
		}
	}

	// if needed, use the VPoint scaling estimator
	if (bUseVP) {
		return GetVP().GetLLFromPix(wxPoint(x, y));
	}

	return geo::Position(lat, lon);
}

bool ChartCanvas::do_smooth_scrolling() const
{
	const global::GUI::View& view = global::OCPN::get().gui().view();

	bool smooth = true
		&& view.smooth_pan_zoom
		&& view.opengl
		&& !view.enable_zoom_to_cursor
		;

	if (!VPoint.is_quilt()) {
		if (Current_Ch->GetChartFamily() == chart::CHART_FAMILY_VECTOR)
			smooth = false;
	} else {
		smooth = true
			&& view.smooth_pan_zoom
			&& !m_pQuilt->IsQuiltVector()
			&& !view.enable_zoom_to_cursor;
	}

	return smooth;
}

bool ChartCanvas::ZoomCanvasIn(double zoom_factor)
{
	if (!VPoint.is_quilt()) {
		if (!Current_Ch) {
			return false;
		}
	}

	if (do_smooth_scrolling()) {
		if (m_bzooming_out) {
			// Interrupt?
			m_zoom_timer.Stop();
			m_bzooming_in = false;
			m_bzooming_out = false;
		}

		if (!m_bzooming_in) {
			// Set up some parameters
			m_zoomt = 5;
			m_zoom_target_factor = zoom_factor;
			m_zoom_current_factor = 1.0;
			m_zoom_timer.Start(m_zoomt);
			m_bzooming_in = true;
			m_bzooming_out = false;
		} else {
			// Make sure timer is running, to recover from lost events
			if (!m_zoom_timer.IsRunning())
				m_zoom_timer.Start(m_zoomt);
		}
	} else {
		DoZoomCanvasIn(zoom_factor);
	}

	extendedSectorLegs.clear();
	return true;
}

bool ChartCanvas::ZoomCanvasOut(double zoom_factor)
{
	if (!VPoint.is_quilt()) {
		if (!Current_Ch) {
			return false;
		}
	}

	if (do_smooth_scrolling()) {
		if (m_bzooming_in) {
			// Interrupt?
			m_zoom_timer.Stop();
			m_bzooming_in = false;
			m_bzooming_out = false;
		}

		if (!m_bzooming_out) {
			// Set up some parameters
			m_zoomt = 5;
			m_zoom_target_factor = zoom_factor;
			m_zoom_current_factor = 1.0;
			m_zoom_timer.Start(m_zoomt);
			m_bzooming_in = false;
			m_bzooming_out = true;
		} else {
			// Make sure timer is running, to recover from lost events
			if (!m_zoom_timer.IsRunning())
				m_zoom_timer.Start(m_zoomt);
		}

	} else {
		DoZoomCanvasOut(zoom_factor);
	}

	extendedSectorLegs.clear();
	return true;
}

void ChartCanvas::OnZoomTimerEvent(wxTimerEvent&)
{
	if (m_bzooming_in && !m_bzooming_out) {
		if (m_zoom_current_factor < m_zoom_target_factor) {
			DoZoomCanvasIn(1.05);
			m_zoom_current_factor *= 1.05;
			m_zoom_timer.Start(m_zoomt);
		} else {
			m_bzooming_in = false;
		}
	} else if (m_bzooming_out && !m_bzooming_in) {
		if (m_zoom_current_factor < m_zoom_target_factor) {
			DoZoomCanvasOut(1.05);
			m_zoom_current_factor *= 1.05;
			m_zoom_timer.Start(m_zoomt);
		} else {
			m_bzooming_out = false;
		}

		if (m_zoom_current_factor >= m_zoom_target_factor) {
			m_bzooming_out = false;
		}
	} else if (m_bzooming_out && m_bzooming_in) {
		// incoherent, should never happen
		m_zoom_timer.Stop();
		m_bzooming_out = false;
		m_bzooming_in = false;
	}
}

bool ChartCanvas::DoZoomCanvasIn(double zoom_factor)
{
	// Cannot allow Yield() re-entrancy here
	if (m_bzooming)
		return false;
	m_bzooming = true;

	bool b_do_zoom = true;

	using namespace chart;

	double proposed_scale_onscreen = GetCanvasScaleFactor() / (GetVPScale() * zoom_factor);
	const ChartBase* pc = NULL;

	if (!VPoint.is_quilt()) {
		pc = Current_Ch;
	} else {
		int new_db_index = m_pQuilt->AdjustRefOnZoomIn(proposed_scale_onscreen);
		if (new_db_index >= 0)
			pc = ChartData->OpenChartFromDB(new_db_index, FULL_INIT);

		if (pCurrentStack) {
			// highlite the correct bar entry
			pCurrentStack->SetCurrentEntryFromdbIndex(new_db_index);
		}
	}

	if (pc) {
		double min_allowed_scale = 50.0; // meters per meter
		min_allowed_scale = pc->GetNormalScaleMin(
			GetCanvasScaleFactor(), global::OCPN::get().gui().view().allow_overzoom_x);

		double target_scale_ppm = GetVPScale() * zoom_factor;
		double new_scale_ppm = target_scale_ppm;

		proposed_scale_onscreen = GetCanvasScaleFactor() / new_scale_ppm;

		// Query the chart to determine the appropriate zoom range
		if (proposed_scale_onscreen < min_allowed_scale) {
			if (min_allowed_scale == GetCanvasScaleFactor() / (GetVPScale()))
				b_do_zoom = false;
			else
				proposed_scale_onscreen = min_allowed_scale;
		}
	}

	if (b_do_zoom) {
		SetVPScale(GetCanvasScaleFactor() / proposed_scale_onscreen);
		Refresh(false);
	}

	m_bzooming = false;
	return true;
}

bool ChartCanvas::DoZoomCanvasOut(double zoom_factor)
{
	// Cannot allow Yield() re-entrancy here
	if (m_bzooming)
		return false;
	m_bzooming = true;

	bool b_do_zoom = true;

	using namespace chart;

	double proposed_scale_onscreen = GetCanvasScaleFactor() / (GetVPScale() / zoom_factor);
	const ChartBase* pc = NULL;

	bool b_smallest = false;

	if (!VPoint.is_quilt()) {
		// not quilted
		pc = Current_Ch;
		double target_scale_ppm = GetVPScale() / zoom_factor;
		double new_scale_ppm = target_scale_ppm;
		proposed_scale_onscreen = GetCanvasScaleFactor() / new_scale_ppm;

		if (ChartData && pc) {
			// If Current_Ch is not on the screen, unbound the zoomout
			const geo::LatLonBoundingBox& viewbox = VPoint.GetBBox();
			geo::BoundingBox chart_box;
			int current_index = ChartData->FinddbIndex(pc->GetFullPath());
			ChartData->GetDBBoundingBox(current_index, &chart_box);
			if ((viewbox.Intersect(chart_box) == geo::BoundingBox::_OUT)) {
				proposed_scale_onscreen = wxMin(proposed_scale_onscreen,
												GetCanvasScaleFactor() / m_absolute_min_scale_ppm);
			} else {
				// Clamp the minimum scale zoom-out to the value specified by the chart
				double max_allowed_scale
					= 4.0 * (pc->GetNormalScaleMax(GetCanvasScaleFactor(), GetCanvasWidth()));
				proposed_scale_onscreen = wxMin(proposed_scale_onscreen, max_allowed_scale);
			}
		}
	} else {
		int new_db_index = m_pQuilt->AdjustRefOnZoomOut(proposed_scale_onscreen);
		if (new_db_index >= 0)
			pc = ChartData->OpenChartFromDB(new_db_index, FULL_INIT);

		if (pCurrentStack)
			pCurrentStack->SetCurrentEntryFromdbIndex(
				new_db_index); // highlite the correct bar entry

		b_smallest = m_pQuilt->IsChartSmallestScale(new_db_index);

		double target_scale_ppm = GetVPScale() / zoom_factor;
		proposed_scale_onscreen = GetCanvasScaleFactor() / target_scale_ppm;

		if (b_smallest || (0 == m_pQuilt->GetExtendedStackCount()))
			proposed_scale_onscreen
				= wxMin(proposed_scale_onscreen, GetCanvasScaleFactor() / m_absolute_min_scale_ppm);
	}

	if (!pc) {
		// no chart, so set a minimum scale
		if ((GetCanvasScaleFactor() / proposed_scale_onscreen) < m_absolute_min_scale_ppm)
			b_do_zoom = false;
	}

	if (b_do_zoom) {
		SetVPScale(GetCanvasScaleFactor() / proposed_scale_onscreen);
		Refresh(false);
	}

	m_bzooming = false;
	return true;
}

void ChartCanvas::ClearbFollow(void)
{
	m_bFollow = false; // update the follow flag
	parent_frame->SetToolbarItemState(ID_FOLLOW, false);
}

bool ChartCanvas::PanCanvas(int dx, int dy)
{
	extendedSectorLegs.clear();

	wxPoint p = GetCanvasPointPix(GetVP().get_position());
	geo::Position dpos = GetCanvasPixPoint(p.x + dx, p.y + dy);
	dpos.normalize_lon();

	// This should not really be necessary, but round-trip georef on some charts is not perfect,
	// So we can get creep on repeated unidimensional pans, and corrupt chart cacheing.......

	// But this only works on north-up projections
	if (((fabs(GetVP().skew) < 0.001)) && (fabs(GetVP().rotation) < 0.001)) {

		if (dx == 0)
			dpos = geo::Position(dpos.lat(), GetVP().longitude());
		if (dy == 0)
			dpos = geo::Position(GetVP().latitude(), dpos.lon());
	}

	int cur_ref_dbIndex = m_pQuilt->GetRefChartdbIndex();
	SetViewPoint(dpos, VPoint.view_scale(), VPoint.skew, VPoint.rotation);

	if (VPoint.is_quilt()) {
		int new_ref_dbIndex = m_pQuilt->GetRefChartdbIndex();
		if ((new_ref_dbIndex != cur_ref_dbIndex) && (new_ref_dbIndex != -1)) {
			using namespace chart;
			// Tweak the scale slightly for a new ref chart
			ChartBase* pc = ChartData->OpenChartFromDB(new_ref_dbIndex, FULL_INIT);
			if (pc) {
				double tweak_scale_ppm = pc->GetNearestPreferredScalePPM(VPoint.view_scale());
				SetVPScale(tweak_scale_ppm);
			}
		}
	}

	ClearbFollow(); // update the follow flag
	Refresh(false);

	// Force an immediate screen update to be sure screen stays in sync with (fast) smooth
	// panning on truly asynchronous opengl renderers.
	Update();

	return true;
}

void ChartCanvas::ReloadVP(bool b_adjust)
{
	if (g_brightness_init)
		SetScreenBrightness(global::OCPN::get().gui().view().screen_brightness);
	LoadVP(VPoint, b_adjust);
}

void ChartCanvas::LoadVP(const ViewPort& vp, bool b_adjust)
{
#ifdef ocpnUSE_GL
	if (global::OCPN::get().gui().view().opengl) {
		m_glcc->Invalidate();
		if (m_glcc->GetSize().x != VPoint.pix_width || m_glcc->GetSize().y != VPoint.pix_height)
			m_glcc->SetSize(VPoint.pix_width, VPoint.pix_height);
	} else
#endif
	{
		m_cache_vp.Invalidate();
		m_bm_cache_vp.Invalidate();
	}

	VPoint.Invalidate();

	if (m_pQuilt)
		m_pQuilt->Invalidate();

	SetViewPoint(vp.get_position(), vp.view_scale(), vp.skew, vp.rotation, b_adjust);
}

void ChartCanvas::SetQuiltRefChart(int dbIndex)
{
	m_pQuilt->SetReferenceChart(dbIndex);
	VPoint.Invalidate();
	m_pQuilt->Invalidate();
}

void ChartCanvas::UpdateCanvasOnGroupChange(void)
{
	delete pCurrentStack;
	pCurrentStack = NULL;
	pCurrentStack = new chart::ChartStack;
	ChartData->BuildChartStack(pCurrentStack, VPoint.latitude(), VPoint.longitude());

	if (m_pQuilt) {
		m_pQuilt->Compose(VPoint);
	}
}

bool ChartCanvas::SetVPScale(double scale)
{
	return SetViewPoint(VPoint.get_position(), scale, VPoint.skew, VPoint.rotation);
}

bool ChartCanvas::SetViewPoint(const geo::Position& pos)
{
	return SetViewPoint(pos, VPoint.view_scale(), VPoint.skew, VPoint.rotation);
}

bool ChartCanvas::SetViewPoint(const geo::Position& pos, double scale_ppm, double skew,
							   double rotation, bool b_adjust)
{
	bool b_ret = false;

	// Any sensible change?
	if ((fabs(VPoint.view_scale() - scale_ppm) < 1e-9) && (fabs(VPoint.skew - skew) < 1e-9)
		&& (fabs(VPoint.rotation - rotation) < 1e-9) && (fabs(VPoint.latitude() - pos.lat()) < 1e-9)
		&& (fabs(VPoint.longitude() - pos.lon()) < 1e-9) && VPoint.IsValid())
		return false;

	VPoint.SetProjectionType(PROJECTION_MERCATOR); // default

	VPoint.Validate(); // Mark this ViewPoint as OK

	// Take a local copy of the last viewport
	ViewPort last_vp = VPoint;

	VPoint.skew = skew;
	VPoint.set_position(pos);
	VPoint.set_view_scale(scale_ppm);
	VPoint.rotation = rotation;

	if ((VPoint.pix_width <= 0) || (VPoint.pix_height <= 0)) // Canvas parameters not yet set
		return false;

	// Has the Viewport scale changed?  If so, invalidate the vp describing the cached bitmap
	if (last_vp.view_scale() != scale_ppm) {
		m_cache_vp.Invalidate();

#ifdef ocpnUSE_GL
		if (global::OCPN::get().gui().view().opengl)
			m_glcc->Invalidate();
#endif
	}

	// A preliminary value, may be tweaked below
	VPoint.chart_scale = m_canvas_scale_factor / (scale_ppm);

	if (!VPoint.is_quilt() && Current_Ch) {

		VPoint.SetProjectionType(Current_Ch->GetChartProjectionType());
		VPoint.SetBoxes();

		//  Allow the chart to adjust the new ViewPort for performance optimization
		//  This will normally be only a fractional (i.e.sub-pixel) adjustment...
		if (b_adjust)
			Current_Ch->AdjustVP(last_vp, VPoint);

		// If there is a sensible change in the chart render, refresh the whole screen
		if ((!m_cache_vp.IsValid()) || (m_cache_vp.view_scale() != VPoint.view_scale())) {
			Refresh(false);
			b_ret = true;
		} else {
			wxPoint cp_last = GetCanvasPointPix(m_cache_vp.get_position());
			wxPoint cp_this = GetCanvasPointPix(VPoint.get_position());

			if (cp_last != cp_this) {
				Refresh(false);
				b_ret = true;
			}
		}
	}

	// Handle the quilted case
	if (VPoint.is_quilt()) {

		if (last_vp.view_scale() != scale_ppm)
			m_pQuilt->InvalidateAllQuiltPatchs();

		// Create the quilt
		if (ChartData && ChartData->IsValid()) {
			if (!pCurrentStack)
				return false;

			int current_db_index = -1;
			current_db_index = pCurrentStack->GetCurrentEntrydbIndex(); // capture the current

			ChartData->BuildChartStack(pCurrentStack, pos.lat(), pos.lon());
			pCurrentStack->SetCurrentEntryFromdbIndex(current_db_index);

			// Check to see if the current quilt reference chart is in the new stack
			int current_ref_stack_index = -1;
			for (int i = 0; i < pCurrentStack->nEntry; i++) {
				if (m_pQuilt->GetRefChartdbIndex() == pCurrentStack->GetDBIndex(i))
					current_ref_stack_index = i;
			}

			if (global::OCPN::get().gui().view().fullscreen_quilt) {
				current_ref_stack_index = m_pQuilt->GetRefChartdbIndex();
			}

			// If the new stack does not contain the current ref chart....
			if ((-1 == current_ref_stack_index) && (m_pQuilt->GetRefChartdbIndex() >= 0)) {
				using namespace chart;
				const ChartTableEntry& cte_ref
					= ChartData->GetChartTableEntry(m_pQuilt->GetRefChartdbIndex());
				int target_scale = cte_ref.GetScale();
				int target_type = cte_ref.GetChartType();
				int candidate_stack_index;

				// reset the ref chart in a way that does not lead to excessive underzoom, for
				// performance reasons
				// Try to find a chart that is the same type, and has a scale of just smaller
				// than the current ref chart

				candidate_stack_index = 0;
				while (candidate_stack_index <= pCurrentStack->nEntry - 1) {
					const ChartTableEntry& cte_candidate = ChartData->GetChartTableEntry(
						pCurrentStack->GetDBIndex(candidate_stack_index));
					int candidate_scale = cte_candidate.GetScale();
					int candidate_type = cte_candidate.GetChartType();

					if ((candidate_scale >= target_scale) && (candidate_type == target_type))
						break;

					candidate_stack_index++;
				}

				//    If that did not work, look for a chart of just larger scale and same type
				if (candidate_stack_index >= pCurrentStack->nEntry) {
					candidate_stack_index = pCurrentStack->nEntry - 1;
					while (candidate_stack_index >= 0) {
						const ChartTableEntry& cte_candidate = ChartData->GetChartTableEntry(
							pCurrentStack->GetDBIndex(candidate_stack_index));
						int candidate_scale = cte_candidate.GetScale();
						int candidate_type = cte_candidate.GetChartType();

						if ((candidate_scale <= target_scale) && (candidate_type == target_type))
							break;

						candidate_stack_index--;
					}
				}

				// and if that did not work, chose stack entry 0
				if ((candidate_stack_index >= pCurrentStack->nEntry) || (candidate_stack_index < 0))
					candidate_stack_index = 0;

				int new_ref_index = pCurrentStack->GetDBIndex(candidate_stack_index);

				m_pQuilt->SetReferenceChart(new_ref_index); // maybe???
			}

			// Preset the VPoint projection type to match what the quilt projection type will be
			int ref_db_index = m_pQuilt->GetRefChartdbIndex();
			int proj = ChartData->GetDBChartProj(ref_db_index);

			// Always keep the default Mercator projection if the reference chart is
			// not in the patch list or the scale is too small for it to render.

			using namespace chart;

			bool renderable = true;
			ChartBase* referenceChart = ChartData->OpenChartFromDB(ref_db_index, FULL_INIT);
			if (referenceChart) {
				double chartMaxScale = referenceChart->GetNormalScaleMax(
					cc1->GetCanvasScaleFactor(), cc1->GetCanvasWidth());
				renderable = chartMaxScale * 1.5 > VPoint.chart_scale;
			}

			VPoint.b_MercatorProjectionOverride = (m_pQuilt->GetnCharts() == 0 || !renderable);

			if (!VPoint.b_MercatorProjectionOverride)
				VPoint.SetProjectionType(proj);

			VPoint.SetBoxes();

			// If this quilt will be a perceptible delta from the existing quilt, then refresh
			// the entire screen
			if (m_pQuilt->IsQuiltDelta(VPoint)) {
				//  Allow the quilt to adjust the new ViewPort for performance optimization
				//  This will normally be only a fractional (i.e. sub-pixel) adjustment...
				if (b_adjust)
					m_pQuilt->AdjustQuiltVP(last_vp, VPoint);

				ChartData->ClearCacheInUseFlags();
				unsigned long hash1 = m_pQuilt->GetXStackHash();
				m_pQuilt->Compose(VPoint);

				// If the extended chart stack has changed, invalidate any cached render bitmap

				if (m_pQuilt->GetXStackHash() != hash1) {
					m_bm_cache_vp.Invalidate();
#ifdef ocpnUSE_GL
					if (global::OCPN::get().gui().view().opengl)
						m_glcc->Invalidate();
#endif
				}

				ChartData->UnLockCache();
				ChartData->PurgeCacheUnusedCharts(false);
				ChartData->LockCache();

				Refresh(false);
				b_ret = true;
			}
			parent_frame->UpdateControlBar();
		}

		VPoint.skew = 0.; // Quilting supports 0 Skew
	}

	if (!VPoint.GetBBox().GetValid())
		VPoint.SetBoxes();

	if (VPoint.GetBBox().GetValid()) {

		// Calculate the on-screen displayed actual scale
		// by a simple traverse northward from the center point
		// of roughly 10 % of the Viewport extent

		// roughly 10 % of lat range, in NM
		double delta_y = (VPoint.GetBBox().GetMaxY() - VPoint.GetBBox().GetMinY()) * 60.0 * 0.10;

		// Make sure the two points are in phase longitudinally
		double lon_norm = VPoint.longitude();
		if (lon_norm > 180.0)
			lon_norm -= 360.0;
		else if (lon_norm < -180.0)
			lon_norm += 360.0;

		geo::Position t = geo::ll_gc_ll(geo::Position(VPoint.latitude(), lon_norm), 0, delta_y);

		wxPoint r1 = GetCanvasPointPix(t); // TODO: cleanup
		wxPoint r = GetCanvasPointPix(geo::Position(VPoint.latitude(), lon_norm));

		m_true_scale_ppm = sqrt(util::sqr((double)(r.y - r1.y)) + util::sqr((double)(r.x - r1.x)))
						   / (delta_y * 1852.0);

		// A fall back in case of very high zoom-out, giving delta_y == 0
		// which can probably only happen with vector charts
		if (0.0 == m_true_scale_ppm)
			m_true_scale_ppm = scale_ppm;

		// Another fallback, for highly zoomed out charts
		// This adjustment makes the displayed TrueScale correspond to the
		// same algorithm used to calculate the chart zoom-out limit for ChartDummy.
		if (scale_ppm < 1e-4)
			m_true_scale_ppm = scale_ppm;

		if (m_true_scale_ppm)
			VPoint.chart_scale = m_canvas_scale_factor / (m_true_scale_ppm);
		else
			VPoint.chart_scale = 1.0;

		if (parent_frame->hasStatusBar()) {
			double true_scale_display = floor(VPoint.chart_scale / 100.0) * 100.0;
			wxString text;

			if (Current_Ch) {
				double chart_native_ppm = m_canvas_scale_factor / Current_Ch->GetNativeScale();
				double scale_factor = scale_ppm / chart_native_ppm;
				if (scale_factor > 1.0)
					text.Printf(_("Scale %4.0f (%1.1fx)"), true_scale_display, scale_factor);
				else
					text.Printf(_("Scale %4.0f (%1.2fx)"), true_scale_display, scale_factor);
			} else
				text.Printf(_("Scale %4.0f"), true_scale_display);

			parent_frame->SetStatusText(text, STAT_FIELD_SCALE);
		}
	}

	//  Maintain global view point position
	global::OCPN::get().nav().set_view_point(VPoint.get_position());

	return b_ret;
}

// Static Icon definitions for some symbols requiring scaling/rotation/translation
// Very specific wxDC draw commands are necessary to properly render these icons...See the code in
// ShipDraw()

// This icon was adapted and scaled from the S52 Presentation Library version 3_03.
// Symbol VECGND02

static const int s_png_pred_icon[] = { -10, -10, -10, 10, 10, 10, 10, -10 };

// This ownship icon was adapted and scaled from the S52 Presentation Library version 3_03
// Symbol OWNSHP05
static const int s_ownship_icon[]
	= { 5, -42, 11, -28, 11, 42, -11, 42, -11, -28, -5, -42, -11, 0, 11, 0, 0, 42, 0, -42 };

static wxPoint transrot(wxPoint pt, double theta, wxPoint offset)
{
	wxPoint ret;
	double px = (double)(pt.x * sin(theta)) + (double)(pt.y * cos(theta));
	double py = (double)(pt.y * sin(theta)) - (double)(pt.x * cos(theta));
	ret.x = (int)wxRound(px);
	ret.y = (int)wxRound(py);
	ret.x += offset.x;
	ret.y += offset.y;

	return ret;
}

void ChartCanvas::ShipDraw(ocpnDC& dc)
{
	if (!GetVP().IsValid())
		return;
	int drawit = 0;
	wxPoint GPSOffsetPixels(0, 0);

	const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();

	// Is ship in Vpoint?
	if (GetVP().GetBBox().PointInBox(nav.pos.lon(), nav.pos.lat(), 0)) // FIXME: alter interface: use position
		drawit++;

	// COG/SOG may be undefined in NMEA data stream
	double pCog = nav.cog;
	if (wxIsNaN(pCog))
		pCog = 0.0;
	double pSog = nav.sog;
	if (wxIsNaN(pSog))
		pSog = 0.0;

	const double ownship_predictor_minutes = global::OCPN::get().gui().ownship().predictor_minutes;

	// Calculate ownship position Predictor
	geo::Position pred = geo::ll_gc_ll(nav.pos, pCog, pSog * ownship_predictor_minutes / 60.0);

	wxPoint lGPSPoint = GetCanvasPointPix(nav.pos);
	wxPoint lShipMidPoint = lGPSPoint;
	wxPoint lPredPoint = GetCanvasPointPix(pred);

	double cog_rad
		= atan2((double)(lPredPoint.y - lShipMidPoint.y), (double)(lPredPoint.x - lShipMidPoint.x));
	cog_rad += M_PI;

	double lpp = sqrt(util::sqr((double)(lPredPoint.x - lShipMidPoint.x))
					  + util::sqr((double)(lPredPoint.y - lShipMidPoint.y)));

	// Is predicted point in the VPoint?
	if (GetVP().GetBBox().PointInBox(pred.lon(), pred.lat(), 0))
		drawit++; // yep

	// Draw the icon rotated to the COG
	// or to the Hdt if available
	double icon_hdt = pCog;
	if (!wxIsNaN(global::OCPN::get().nav().get_data().hdt))
		icon_hdt = global::OCPN::get().nav().get_data().hdt;

	// COG may be undefined in NMEA data stream
	if (wxIsNaN(icon_hdt))
		icon_hdt = 0.0;

	// Calculate the ownship drawing angle icon_rad using an assumed 10 minute predictor
	geo::Position osd_head = geo::ll_gc_ll(nav.pos, icon_hdt, pSog * 10.0 / 60.0);
	lShipMidPoint = GetCanvasPointPix(nav.pos);
	wxPoint osd_head_point = GetCanvasPointPix(osd_head);

	double icon_rad = atan2((double)(osd_head_point.y - lShipMidPoint.y),
							(double)(osd_head_point.x - lShipMidPoint.x));
	icon_rad += M_PI;

	if (pSog < 0.2)
		icon_rad = ((icon_hdt + 90.0) * M_PI / 180.0) + GetVP().rotation;

	// Calculate ownship Heading pointer as a predictor
	geo::Position hdg_pred = geo::ll_gc_ll(nav.pos, icon_hdt, pSog * ownship_predictor_minutes / 60.0);
	lShipMidPoint = GetCanvasPointPix(nav.pos);
	wxPoint lHeadPoint = GetCanvasPointPix(hdg_pred);

	// Should we draw the Head vector?
	// Compare the points lHeadPoint and lPredPoint
	// If they differ by more than n pixels, and the head vector is valid, then render the head
	// vector

	double ndelta_pix = 10.0;
	bool b_render_hdt = false;
	if (!wxIsNaN(global::OCPN::get().nav().get_data().hdt)) {
		double dist = sqrt(util::sqr((double)(lHeadPoint.x - lPredPoint.x))
						   + util::sqr((double)(lHeadPoint.y - lPredPoint.y)));
		if (dist > ndelta_pix && !wxIsNaN(nav.sog))
			b_render_hdt = true;
	}

	// Another draw test ,based on pixels, assuming the ship icon is a fixed nominal size
	// and is just barely outside the viewport        ....
	geo::BoundingBox bb_screen(0, 0, GetVP().pix_width, GetVP().pix_height);
	if (bb_screen.PointInBox(lShipMidPoint.x, lShipMidPoint.y, 20))
		drawit++;

	// And one more test to catch the case where COG line crosses the screen,
	// but ownship and pred point are both off

	if (GetVP().GetBBox().LineIntersect(nav.pos.lon(), nav.pos.lat(), pred.lon(), pred.lat()))
		drawit++;

	const global::ColorManager& colors = global::OCPN::get().color();

	// Do the draw if either the ship or prediction is within the current VPoint
	if (drawit) {
		int img_height;

		wxColour pred_colour = colors.get_color(_T("URED"));
		if (SHIP_NORMAL != m_ownship_state)
			pred_colour = colors.get_color(_T("GREY1"));

		// Establish ship color
		// It changes color based on GPS and Chart accuracy/availability
		wxColour ship_color(colors.get_color(_T("URED"))); // default is OK

		if (SHIP_NORMAL != m_ownship_state)
			ship_color = colors.get_color(_T("GREY1"));

		if (SHIP_LOWACCURACY == m_ownship_state)
			ship_color = colors.get_color(_T("YELO1"));

		if (GetVP().chart_scale > 300000) // According to S52, this should be 50,000
		{
			dc.SetPen(wxPen(pred_colour, 2));

			if (SHIP_NORMAL == m_ownship_state)
				dc.SetBrush(wxBrush(ship_color, wxTRANSPARENT));
			else
				dc.SetBrush(wxBrush(colors.get_color(_T("YELO1"))));

			dc.DrawEllipse(lShipMidPoint.x - 10, lShipMidPoint.y - 10, 20, 20);
			dc.DrawEllipse(lShipMidPoint.x - 6, lShipMidPoint.y - 6, 12, 12);

			dc.DrawLine(lShipMidPoint.x - 12, lShipMidPoint.y, lShipMidPoint.x + 12,
						lShipMidPoint.y);
			dc.DrawLine(lShipMidPoint.x, lShipMidPoint.y - 12, lShipMidPoint.x,
						lShipMidPoint.y + 12);
			img_height = 20;
		} else {
			double screenResolution = (double)::wxGetDisplaySize().y / ::wxGetDisplaySizeMM().y;

			wxImage pos_image;
			pos_image = m_pos_image_red->Copy();

			if (SHIP_LOWACCURACY == m_ownship_state)
				pos_image = m_pos_image_yellow->Copy();
			else if (SHIP_NORMAL != m_ownship_state)
				pos_image = m_pos_image_grey->Copy();

			// Substitute user ownship image if found
			if (m_pos_image_user) {
				pos_image = m_pos_image_user->Copy();
				if (SHIP_LOWACCURACY == m_ownship_state)
					pos_image = m_pos_image_user_yellow->Copy();
				else if (SHIP_NORMAL != m_ownship_state)
					pos_image = m_pos_image_user_grey->Copy();
			}

			const global::GUI::OwnShip& ownship = global::OCPN::get().gui().ownship();

			if (ownship.beam_meters > 0.0 && ownship.length_meters > 0.0 && ownship.icon_type > 0) {
				// use large ship

				int ownShipWidth = 22; // Default values from s_ownship_icon
				int ownShipLength = 84;

				if (ownship.icon_type == 1) { // FIXME: use enum
					ownShipWidth = pos_image.GetWidth();
					ownShipLength = pos_image.GetHeight();
				}

				// Calculate the true ship length in exact pixels
				geo::Position ship_bow = geo::ll_gc_ll(nav.pos, icon_hdt, ownship.length_meters / 1852.0);
				wxPoint lShipBowPoint;
				wxPoint2DDouble b_point = GetVP().GetDoublePixFromLL(ship_bow);
				wxPoint2DDouble a_point = GetVP().GetDoublePixFromLL(nav.pos);

				double shipLength_px = sqrt(util::sqr((double)(b_point.m_x - a_point.m_x))
											+ util::sqr((double)(b_point.m_y - a_point.m_y)));

				// And in mm
				double shipLength_mm = shipLength_px / screenResolution;

				// Set minimum ownship drawing size
				double ownship_min_mm = ownship.min_mm;
				ownship_min_mm = wxMax(ownship_min_mm, 1.0);

				// Calculate Nautical Miles distance from midships to gps antenna
				double hdt_ant = icon_hdt + 180.0;
				double dy = (ownship.length_meters / 2 - ownship.gps_antenna_offset_y) / 1852.0;
				double dx = ownship.gps_antenna_offset_x / 1852.0;
				if (ownship.gps_antenna_offset_y > ownship.length_meters / 2) { // reverse?
					hdt_ant = icon_hdt;
					dy = -dy;
				}

				// If the drawn ship size is going to be clamped, adjust the gps antenna offsets
				if (shipLength_mm < ownship_min_mm) {
					dy /= shipLength_mm / ownship_min_mm;
					dx /= shipLength_mm / ownship_min_mm;
				}

				geo::Position ship_mid = geo::ll_gc_ll(nav.pos, hdt_ant, dy);
				geo::Position ship_mid1 = geo::ll_gc_ll(ship_mid, icon_hdt - 90.0, dx);

				lShipMidPoint = GetCanvasPointPix(ship_mid1);
				GPSOffsetPixels.x = lShipMidPoint.x - lGPSPoint.x;
				GPSOffsetPixels.y = lShipMidPoint.y - lGPSPoint.y;

				double scale_factor = shipLength_px / ownShipLength;

				// Calculate a scale factor that would produce a reasonably sized icon
				double scale_factor_min = ownship_min_mm / (ownShipLength / screenResolution);

				// And choose the correct one
				scale_factor = wxMax(scale_factor, scale_factor_min);

				double scale_factor_y = scale_factor;
				double scale_factor_x
					= scale_factor_y * ((double)ownShipLength / ownShipWidth)
					  / (ownship.length_meters / ownship.beam_meters);

				if (ownship.icon_type == 1) { // Scaled bitmap (FIXME:use enum)
					pos_image.Rescale(ownShipWidth * scale_factor_x, ownShipLength * scale_factor_y,
									  wxIMAGE_QUALITY_HIGH);
					wxPoint rot_ctr(pos_image.GetWidth() / 2, pos_image.GetHeight() / 2);
					wxImage rot_image = pos_image.Rotate(-(icon_rad - (M_PI / 2.0)), rot_ctr, true);

					// Simple sharpening algorithm.....
					for (int ip = 0; ip < rot_image.GetWidth(); ip++)
						for (int jp = 0; jp < rot_image.GetHeight(); jp++)
							if (rot_image.GetAlpha(ip, jp) > 64)
								rot_image.SetAlpha(ip, jp, 255);

					wxBitmap os_bm(rot_image);

					int w = os_bm.GetWidth();
					int h = os_bm.GetHeight();
					img_height = h;

					dc.DrawBitmap(os_bm, lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2, true);

					// Maintain dirty box,, missing in __WXMSW__ library
					dc.CalcBoundingBox(lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2);
					dc.CalcBoundingBox(lShipMidPoint.x - w / 2 + w, lShipMidPoint.y - h / 2 + h);
				} else if (ownship.icon_type == 2) { // Scaled Vector (FIXME:use enum)
					wxPoint ownship_icon[10];

					for (int i = 0; i < 10; i++) {
						int j = i * 2;
						double pxa = (double)(s_ownship_icon[j]);
						double pya = (double)(s_ownship_icon[j + 1]);
						pya *= scale_factor_y;
						pxa *= scale_factor_x;

						double px = (pxa * sin(icon_rad)) + (pya * cos(icon_rad));
						double py = (pya * sin(icon_rad)) - (pxa * cos(icon_rad));

						ownship_icon[i].x = (int)(px) + lShipMidPoint.x;
						ownship_icon[i].y = (int)(py) + lShipMidPoint.y;
					}

					wxPen ppPen1(colors.get_color(_T("UBLCK")), 1, wxSOLID);
					dc.SetPen(ppPen1);
					dc.SetBrush(wxBrush(ship_color));

					dc.StrokePolygon(6, &ownship_icon[0], 0, 0);

					// draw reference point (midships) cross
					dc.StrokeLine(ownship_icon[6].x, ownship_icon[6].y, ownship_icon[7].x,
								  ownship_icon[7].y);
					dc.StrokeLine(ownship_icon[8].x, ownship_icon[8].y, ownship_icon[9].x,
								  ownship_icon[9].y);
				}

				img_height = ownShipLength * scale_factor_y;

				// Reference point, where the GPS antenna is
				int circle_rad = 3;
				if (m_pos_image_user)
					circle_rad = 1;

				dc.SetPen(wxPen(colors.get_color(_T("UBLCK")), 1));
				dc.SetBrush(wxBrush(colors.get_color(_T("UIBCK"))));
				dc.StrokeCircle(lGPSPoint.x, lGPSPoint.y, circle_rad);
			} else { // Fixed bitmap icon.
				wxPoint rot_ctr(pos_image.GetWidth() / 2, pos_image.GetHeight() / 2);
				wxImage rot_image = pos_image.Rotate(-(icon_rad - (M_PI / 2.0)), rot_ctr, true);

				// Simple sharpening algorithm.....
				for (int ip = 0; ip < rot_image.GetWidth(); ip++)
					for (int jp = 0; jp < rot_image.GetHeight(); jp++)
						if (rot_image.GetAlpha(ip, jp) > 64)
							rot_image.SetAlpha(ip, jp, 255);

				wxBitmap os_bm(rot_image);

				int w = os_bm.GetWidth();
				int h = os_bm.GetHeight();
				img_height = h;

				dc.DrawBitmap(os_bm, lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2, true);

				//  Reference point, where the GPS antenna is
				int circle_rad = 3;
				if (m_pos_image_user)
					circle_rad = 1;

				dc.SetPen(wxPen(colors.get_color(_T("UBLCK")), 1));
				dc.SetBrush(wxBrush(colors.get_color(_T("UIBCK"))));
				dc.StrokeCircle(lShipMidPoint.x, lShipMidPoint.y, circle_rad);

				// Maintain dirty box,, missing in __WXMSW__ library
				dc.CalcBoundingBox(lShipMidPoint.x - w / 2, lShipMidPoint.y - h / 2);
				dc.CalcBoundingBox(lShipMidPoint.x - w / 2 + w, lShipMidPoint.y - h / 2 + h);
			}
		} // ownship draw

		// draw course over ground if they are longer than the ship
		if (!wxIsNaN(nav.cog) && !wxIsNaN(nav.sog)) {
			if (lpp >= img_height / 2) {
				const double png_pred_icon_scale_factor = 0.4;
				wxPoint icon[4];

				for (int i = 0; i < 4; i++) {
					int j = i * 2;
					double pxa = (double)(s_png_pred_icon[j]);
					double pya = (double)(s_png_pred_icon[j + 1]);

					pya *= png_pred_icon_scale_factor;
					pxa *= png_pred_icon_scale_factor;

					double px = (pxa * sin(cog_rad)) + (pya * cos(cog_rad));
					double py = (pya * sin(cog_rad)) - (pxa * cos(cog_rad));

					icon[i].x = (int)wxRound(px) + lPredPoint.x + GPSOffsetPixels.x;
					icon[i].y = (int)wxRound(py) + lPredPoint.y + GPSOffsetPixels.y;
				}

				// COG Predictor
				wxDash dash_long[2];
				dash_long[0] = (int)(3.0 * m_pix_per_mm); // 8// Long dash  <---------+
				dash_long[1] = (int)(1.5 * m_pix_per_mm); // 2// Short gap            |

				const int cog_predictor_width = global::OCPN::get().gui().ownship().cog_predictor_width;

				wxPen ppPen2(pred_colour, cog_predictor_width, wxUSER_DASH);
				ppPen2.SetDashes(2, dash_long);
				dc.SetPen(ppPen2);
				dc.StrokeLine(lGPSPoint.x + GPSOffsetPixels.x, lGPSPoint.y + GPSOffsetPixels.y,
							  lPredPoint.x + GPSOffsetPixels.x, lPredPoint.y + GPSOffsetPixels.y);

				wxDash dash_long3[2];
				dash_long3[0] = cog_predictor_width * dash_long[0];
				dash_long3[1] = cog_predictor_width * dash_long[1];

				if (cog_predictor_width > 1) {
					wxPen ppPen3(colors.get_color(_T("UBLCK")), 1, wxUSER_DASH);
					ppPen3.SetDashes(2, dash_long3);
					dc.SetPen(ppPen3);
					dc.StrokeLine(lGPSPoint.x + GPSOffsetPixels.x, lGPSPoint.y + GPSOffsetPixels.y,
								  lPredPoint.x + GPSOffsetPixels.x,
								  lPredPoint.y + GPSOffsetPixels.y);
				}
				wxPen ppPen1(colors.get_color(_T("UBLCK")), 1, wxSOLID);
				dc.SetPen(ppPen1);
				dc.SetBrush(wxBrush(pred_colour));

				dc.StrokePolygon(4, icon);
			}
		}

		// HDT Predictor
		if (b_render_hdt) {
			wxDash dash_short[2];
			dash_short[0] = (int)(1.5 * m_pix_per_mm); // Short dash  <---------+
			dash_short[1] = (int)(1.8 * m_pix_per_mm); // Short gap            |

			wxPen ppPen2(pred_colour, 1, wxUSER_DASH);
			ppPen2.SetDashes(2, dash_short);

			dc.SetPen(ppPen2);
			dc.StrokeLine(lGPSPoint.x + GPSOffsetPixels.x, lGPSPoint.y + GPSOffsetPixels.y,
						  lHeadPoint.x + GPSOffsetPixels.x, lHeadPoint.y + GPSOffsetPixels.y);

			wxPen ppPen1(pred_colour, 2, wxSOLID);
			dc.SetPen(ppPen1);
			dc.SetBrush(wxBrush(colors.get_color(_T("GREY2"))));

			dc.StrokeCircle(lHeadPoint.x + GPSOffsetPixels.x, lHeadPoint.y + GPSOffsetPixels.y, 4);
		}

		// Draw radar rings if activated
		const global::GUI::View& view = global::OCPN::get().gui().view();
		if (view.NavAidRadarRingsNumberVisible > 0) {
			double factor = 1.00;
			if (view.NavAidRadarRingsStepUnits == 1) // nautical miles
				factor = 1 / 1.852;

			factor *= view.NavAidRadarRingsStep;

			geo::Position t = geo::ll_gc_ll(nav.pos, 0, factor);
			wxPoint r = GetCanvasPointPix(t);

			double lpp = sqrt(util::sqr((double)(lShipMidPoint.x - r.x))
							  + util::sqr((double)(lShipMidPoint.y - r.y)));
			int pix_radius = (int)lpp;

			wxPen ppPen1(colors.get_color(_T("URED")), 2);
			dc.SetPen(ppPen1);
			dc.SetBrush(wxBrush(colors.get_color(_T("URED")), wxTRANSPARENT));

			for (int i = 1; i <= view.NavAidRadarRingsNumberVisible; i++)
				dc.StrokeCircle(lShipMidPoint.x, lShipMidPoint.y, i * pix_radius);
		}
	}
}

/// @ChartCanvas::CalcGridSpacing
///
/// Calculate the major and minor spacing between the lat/lon grid
///
/// @param [r] WindowDegrees [double] displayed number of lat or lan in the window
/// @param [w] MajorSpacing [double &] Major distance between grid lines
/// @param [w] MinorSpacing [double &] Minor distance between grid lines
/// @return [void]
void CalcGridSpacing(double WindowDegrees, double& MajorSpacing, double& MinorSpacing)
{
	int tabi; // iterator for lltab

	// table for calculating the distance between the grids
	// [0] width or height of the displayed chart in degrees
	// [1] spacing between major grid liones in degrees
	// [2] spacing between minor grid lines in degrees
	const double lltab[][3] = { { 180.0, 90.0, 30.0 },
								{ 90.0, 45.0, 15.0 },
								{ 60.0, 30.0, 10.0 },
								{ 20.0, 10.0, 2.0 },
								{ 10.0, 5.0, 1.0 },
								{ 4.0, 2.0, 30.0 / 60.0 },
								{ 2.0, 1.0, 20.0 / 60.0 },
								{ 1.0, 0.5, 10.0 / 60.0 },
								{ 30.0 / 60.0, 15.0 / 60.0, 5.0 / 60.0 },
								{ 20.0 / 60.0, 10.0 / 60.0, 2.0 / 60.0 },
								{ 10.0 / 60.0, 5.0 / 60.0, 1.0 / 60.0 },
								{ 4.0 / 60.0, 2.0 / 60.0, 0.5 / 60.0 },
								{ 2.0 / 60.0, 1.0 / 60.0, 0.2 / 60.0 },
								{ 1.0 / 60.0, 0.5 / 60.0, 0.1 / 60.0 },
								{ 0.4 / 60.0, 0.2 / 60.0, 0.05 / 60.0 },
								{ 0.0, 0.1 / 60.0, 0.02 / 60.0 } // indicates last entry
	};

	for (tabi = 0; lltab[tabi][0] != 0.0; tabi++) {
		if (WindowDegrees > lltab[tabi][0]) {
			break;
		}
	}
	MajorSpacing = lltab[tabi][1]; // major latitude distance
	MinorSpacing = lltab[tabi][2]; // minor latitude distance
	return;
}

/// @ChartCanvas::CalcGridText *************************************
///
/// Calculates text to display at the major grid lines
///
/// @param [r] latlon [double] latitude or longitude of grid line
/// @param [r] spacing [double] distance between two major grid lines
/// @param [r] bPostfix [bool] true for latitudes, false for longitudes
/// @param [w] text [char*] textbuffer for result, minimum of 12 chars in length
///
/// @return [void]
void CalcGridText(double latlon, double spacing, bool bPostfix, char* text)
{
	int deg = (int)fabs(latlon); // degrees
	double min = fabs((fabs(latlon) - deg) * 60.0); // Minutes
	char postfix;
	const unsigned int BufLen = 12;

	// calculate postfix letter (NSEW)
	if (latlon > 0.0) {
		if (bPostfix) {
			postfix = 'N';
		} else {
			postfix = 'E';
		}
	} else if (latlon < 0.0) {
		if (bPostfix) {
			postfix = 'S';
		} else {
			postfix = 'W';
		}
	} else {
		postfix = ' '; // no postfix for equator and greenwich
	}
	// calculate text, display minutes only if spacing is smaller than one degree

	if (spacing >= 1.0) {
		snprintf(text, BufLen, "%3d° %c", deg, postfix);
	} else if (spacing >= (1.0 / 60.0)) {
		snprintf(text, BufLen, "%3d°%02.0f %c", deg, min, postfix);
	} else {
		snprintf(text, BufLen, "%3d°%02.2f %c", deg, min, postfix);
	}
	text[BufLen - 1] = '\0';
	return;
}

/// Draws major and minor Lat/Lon Grid on the chart
/// - distance between Grid-lm ines are calculated automatic
/// - major grid lines will be across the whole chart window
/// - minor grid lines will be 10 pixel at each edge of the chart window.
///
/// @param [w] dc [wxDC&] the wx drawing context
///
/// @return [void]
void ChartCanvas::GridDraw(ocpnDC& dc)
{
	const global::GUI::View& view = global::OCPN::get().gui().view();
	const global::ColorManager& colors = global::OCPN::get().color();

	if (!(view.display_grid && (fabs(GetVP().rotation) < 1e-5)
		  && ((fabs(GetVP().skew) < 1e-9) || view.skew_comp)))
		return;

	double lat;
	double lon;
	double dlat;
	double dlon;
	double gridlatMajor;
	double gridlatMinor;
	double gridlonMajor;
	double gridlonMinor;
	wxCoord w;
	wxCoord h;
	wxPen GridPen(colors.get_color(_T("SNDG1")), 1, wxSOLID);
	wxFont* font = wxTheFontList->FindOrCreateFont(
		8, wxFONTFAMILY_SWISS, wxNORMAL, wxFONTWEIGHT_NORMAL, FALSE, wxString(_T("Arial")));
	dc.SetPen(GridPen);
	dc.SetFont(*font);
	dc.SetTextForeground(colors.get_color(_T("SNDG1")));

	w = m_canvas_width;
	h = m_canvas_height;

	geo::Position p0 = GetCanvasPixPoint(0, 0); // get lat/lon of upper left point of the window
	geo::Position p1 = GetCanvasPixPoint(w, h); // get lat/lon of lower right point of the window
	dlat = p0.lat() - p1.lat(); // calculate how many degrees of latitude are shown in the window
	dlon = p1.lon() - p0.lon(); // calculate how many degrees of longitude are shown in the window
	if (dlon < 0.0) {
		// consider datum border at 180 degrees longitude
		dlon = dlon + 360.0;
	}
	// calculate distance between latitude grid lines
	CalcGridSpacing(dlat, gridlatMajor, gridlatMinor);

	// calculate position of first major latitude grid line
	lat = ceil(p1.lat() / gridlatMajor) * gridlatMajor;

	// Draw Major latitude grid lines and text
	while (lat < p0.lat()) {
		char sbuf[12];
		CalcGridText(lat, gridlatMajor, true, sbuf); // get text for grid line
		wxPoint r = GetCanvasPointPix(geo::Position(lat, (p1.lon() + p0.lon()) / 2));
		dc.DrawLine(0, r.y, w, r.y, false); // draw grid line
		dc.DrawText(wxString(sbuf, wxConvUTF8), 0, r.y); // draw text
		lat = lat + gridlatMajor;

		if (fabs(lat - wxRound(lat)) < 1e-5)
			lat = wxRound(lat);
	}

	// calculate position of first minor latitude grid line
	lat = ceil(p1.lat() / gridlatMinor) * gridlatMinor;

	// Draw minor latitude grid lines
	while (lat < p0.lat()) {
		wxPoint r = GetCanvasPointPix(geo::Position(lat, (p1.lon() + p0.lon()) / 2));
		dc.DrawLine(0, r.y, 10, r.y, false);
		dc.DrawLine(w - 10, r.y, w, r.y, false);
		lat = lat + gridlatMinor;
	}

	// calculate distance between grid lines
	CalcGridSpacing(dlon, gridlonMajor, gridlonMinor);

	// calculate position of first major latitude grid line
	lon = ceil(p0.lon() / gridlonMajor) * gridlonMajor;

	// draw major longitude grid lines
	for (int i = 0, itermax = (int)(dlon / gridlonMajor); i <= itermax; i++) {
		char sbuf[12];
		CalcGridText(lon, gridlonMajor, false, sbuf);
		wxPoint r = GetCanvasPointPix(geo::Position((p0.lat() + p1.lat()) / 2, lon));
		dc.DrawLine(r.x, 0, r.x, h, false);
		dc.DrawText(wxString(sbuf, wxConvUTF8), r.x, 0);
		lon = lon + gridlonMajor;
		if (lon > 180.0) {
			lon = lon - 360.0;
		}

		if (fabs(lon - wxRound(lon)) < 1e-5)
			lon = wxRound(lon);
	}

	// calculate position of first minor longitude grid line
	lon = ceil(p0.lon() / gridlonMinor) * gridlonMinor;
	// draw minor longitude grid lines
	for (int i = 0, itermax = (int)(dlon / gridlonMinor); i <= itermax; i++) {
		wxPoint r = GetCanvasPointPix(geo::Position((p0.lat() + p1.lat()) / 2, lon));
		dc.DrawLine(r.x, 0, r.x, 10, false);
		dc.DrawLine(r.x, h - 10, r.x, h, false);
		lon = lon + gridlonMinor;
		if (lon > 180.0) {
			lon = lon - 360.0;
		}
	}
}

void ChartCanvas::ScaleBarDraw(ocpnDC& dc)
{
	const global::ColorManager& colors = global::OCPN::get().color();

	int x_origin = global::OCPN::get().gui().view().display_grid ? 60 : 20;
	int y_origin = m_canvas_height - 50;

	if (GetVP().chart_scale > 80000) {
		// Draw 10 mile scale as SCALEB11
		geo::Position pos = GetCanvasPixPoint(x_origin, y_origin);

		geo::Position t = geo::ll_gc_ll(pos, 0, 10.0);
		wxPoint r = GetCanvasPointPix(t);

		int l1 = (y_origin - r.y) / 5;

		wxPen pen1(colors.get_color(_T("SNDG2")), 3, wxSOLID);
		wxPen pen2(colors.get_color(_T("SNDG1")), 3, wxSOLID);

		for (int i = 0; i < 5; i++) {
			int y = l1 * i;
			if (i & 1)
				dc.SetPen(pen1);
			else
				dc.SetPen(pen2);

			dc.DrawLine(x_origin, y_origin - y, x_origin, y_origin - (y + l1));
		}
	} else {
		// Draw 1 mile scale as SCALEB10
		geo::Position pos = GetCanvasPixPoint(x_origin, y_origin);

		geo::Position t= geo::ll_gc_ll(pos, 0, 1.0);
		wxPoint r = GetCanvasPointPix(t);

		int l1 = (y_origin - r.y) / 10;

		wxPen pen1(colors.get_color(_T("SCLBR")), 3, wxSOLID);
		wxPen pen2(colors.get_color(_T("CHDRD")), 3, wxSOLID);

		for (int i = 0; i < 10; i++) {
			int y = l1 * i;
			if (i & 1)
				dc.SetPen(pen1);
			else
				dc.SetPen(pen2);

			dc.DrawLine(x_origin, y_origin - y, x_origin, y_origin - (y + l1));
		}
	}
}

void ChartCanvas::AISDrawAreaNotices(ocpnDC& dc)
{
	const global::AIS::Data& ais = global::OCPN::get().ais().get_data();
	const global::ColorManager& colors = global::OCPN::get().color();
	const global::GUI::View& view = global::OCPN::get().gui().view();

	if (!g_pAIS || !view.ShowAIS || !ais.ShowAreaNotices)
		return;

	wxDateTime now = wxDateTime::Now();
	now.MakeGMT();

	bool b_pens_set = false;
	wxPen pen_save;
	wxBrush brush_save;
	wxColour yellow;
	wxColour green;
	wxPen pen;
	wxBrush* yellow_brush = wxTheBrushList->FindOrCreateBrush(wxColour(0, 0, 0), wxTRANSPARENT);
	wxBrush* green_brush = wxTheBrushList->FindOrCreateBrush(wxColour(0, 0, 0), wxTRANSPARENT);
	wxBrush* brush;

	AIS_Target_Hash* current_targets = g_pAIS->GetAreaNoticeSourcesList();

	float vp_scale = GetVPScale();

	for (AIS_Target_Hash::iterator target = current_targets->begin();
		 target != current_targets->end(); ++target) {
		AIS_Target_Data* target_data = target->second;
		if (!target_data->area_notices.empty()) {
			if (!b_pens_set) {
				pen_save = dc.GetPen();
				brush_save = dc.GetBrush();

				yellow = colors.get_color(_T("YELO1"));
				yellow.Set(yellow.Red(), yellow.Green(), yellow.Blue(), 64);

				green = colors.get_color(_T("GREEN4"));
				green.Set(green.Red(), green.Green(), green.Blue(), 64);

				pen.SetColour(yellow);
				pen.SetWidth(2);

				yellow_brush = wxTheBrushList->FindOrCreateBrush(yellow, wxCROSSDIAG_HATCH);
				green_brush = wxTheBrushList->FindOrCreateBrush(green, wxTRANSPARENT);
				brush = yellow_brush;

				b_pens_set = true;
			}

			for (AIS_Area_Notice_Hash::iterator ani = target_data->area_notices.begin();
				 ani != target_data->area_notices.end(); ++ani) {
				Ais8_001_22& area_notice = ani->second;

				if (area_notice.expiry_time > now) {
					std::vector<wxPoint> points;
					bool draw_polygon = false;

					switch (area_notice.notice_type) {
						case 0:
							pen.SetColour(green);
							brush = green_brush;
							break;
						case 1:
							pen.SetColour(yellow);
							brush = yellow_brush;
							break;
						default:
							pen.SetColour(yellow);
							brush = yellow_brush;
					}
					dc.SetPen(pen);
					dc.SetBrush(*brush);

					geo::Position pos;
					for (Ais8_001_22_SubAreaList::iterator sa = area_notice.sub_areas.begin();
						 sa != area_notice.sub_areas.end(); ++sa) {
						using namespace ais;
						switch (sa->shape) {
							case AIS8_001_22_SHAPE_CIRCLE: {
								pos = geo::Position(sa->latitude, sa->longitude);

								wxPoint target_point = GetCanvasPointPix(geo::Position(sa->latitude, sa->longitude));
								points.push_back(target_point);
								if (sa->radius_m > 0.0)
									dc.DrawCircle(target_point, sa->radius_m * vp_scale);
								break;
							}
							case AIS8_001_22_SHAPE_POLYGON:
								draw_polygon = true;
							case AIS8_001_22_SHAPE_POLYLINE: {
								for (int i = 0; i < 4; ++i) {
									pos = geo::ll_gc_ll(pos, sa->angles[i], sa->dists_m[i] / 1852.0);
									wxPoint target_point = GetCanvasPointPix(pos);
									points.push_back(target_point);
								}
							}
						}
					}
					if (draw_polygon)
						dc.DrawPolygon(points.size(), &points.front());
				}
			}
		}
	}

	if (b_pens_set) {
		dc.SetPen(pen_save);
		dc.SetBrush(brush_save);
	}
}

void ChartCanvas::AISDraw(ocpnDC& dc)
{
	using namespace ais;

	if (!g_pAIS)
		return;

	// Toggling AIS display on and off
	const global::GUI::View& view = global::OCPN::get().gui().view();
	if (!view.ShowAIS)
		return;

	const global::AIS::Data& ais = global::OCPN::get().ais().get_data();

	// Iterate over the AIS Target Hashmap
	AIS_Target_Hash::iterator it;

	AIS_Target_Hash* current_targets = g_pAIS->GetTargetList();

	// Draw all targets in three pass loop, sorted on SOG, GPSGate & DSC on top
	// This way, fast targets are not obscured by slow/stationary targets
	for (it = (*current_targets).begin(); it != (*current_targets).end(); ++it) {
		AIS_Target_Data* td = it->second;
		if ((td->SOG < ais.ShowMoored_Kts)
			&& !((td->Class == AIS_GPSG_BUDDY) || (td->Class == AIS_DSC)))
			AISDrawTarget(td, dc);
	}

	for (it = (*current_targets).begin(); it != (*current_targets).end(); ++it) {
		AIS_Target_Data* td = it->second;
		if ((td->SOG >= ais.ShowMoored_Kts)
			&& !((td->Class == AIS_GPSG_BUDDY) || (td->Class == AIS_DSC)))
			AISDrawTarget(td, dc);
	}

	for (it = (*current_targets).begin(); it != (*current_targets).end(); ++it) {
		AIS_Target_Data* td = it->second;
		if ((td->Class == AIS_GPSG_BUDDY) || (td->Class == AIS_DSC))
			AISDrawTarget(td, dc);
	}
}

void ChartCanvas::AISDrawTarget(ais::AIS_Target_Data* td, ocpnDC& dc)
{
	using namespace ais;

	const global::AIS::Data& ais = global::OCPN::get().ais().get_data();

	// Target data must be valid
	if (NULL == td)
		return;

	// Target is lost due to position report time-out, but still in Target List
	if (td->b_lost)
		return;

	// Skip anchored/moored (interpreted as low speed) targets if requested
	// unless the target is NUC or AtoN, in which case it is always displayed.
	if ((!ais.ShowMoored) && (td->SOG <= ais.ShowMoored_Kts) && (td->NavStatus != NOT_UNDER_COMMAND)
		&& ((td->Class == AIS_CLASS_A) || (td->Class == AIS_CLASS_B)))
		return;

	// Target data position must have been valid once
	if (!td->b_positionOnceValid)
		return;

	// And we never draw ownship
	if (td->b_OwnShip)
		return;

	// If target's speed is unavailable, use zero for further calculations
	double target_sog = td->SOG;
	if ((td->SOG > 102.2) && !td->b_SarAircraftPosnReport)
		target_sog = 0.0;

	int drawit = 0;
	wxPoint TargetPoint, PredPoint;

	// Is target in Vpoint?
	if (GetVP().GetBBox().PointInBox(td->Lon, td->Lat, 0))
		drawit++; // yep

	// Always draw alert targets, even if they are off the screen
	if (td->n_alarm_state == AIS_ALARM_SET)
		drawit++;

	// If AIS tracks are shown, is the first point of the track on-screen?
	if (td->b_show_track) {
		ais::AISTargetTrackList::const_iterator i = td->m_ptrack.begin();
		if (i != td->m_ptrack.end()) {
			if (GetVP().GetBBox().PointInBox(i->m_lon, i->m_lat, 0))
				drawit++;
		}
	}

	// Calculate AIS target position Predictor, using global static variable for length of vector

	geo::Position pred = geo::ll_gc_ll(geo::Position(td->Lat, td->Lon), td->COG,
									   target_sog * ais.ShowCOG_Mins / 60.0);

	// Is predicted point in the VPoint?
	if (GetVP().GetBBox().PointInBox(pred.lon(), pred.lat(), 0))
		drawit++;

	// And one more test to catch the case where target COG line crosses the screen,
	// but the target itself and its pred point are both off-screen
	if (GetVP().GetBBox().LineIntersect(td->Lon, td->Lat, pred.lon(), pred.lat()))
		drawit++;

	const global::GUI::View& view = global::OCPN::get().gui().view();
	const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
	const global::ColorManager& colors = global::OCPN::get().color();

	// Do the draw if conditions indicate
	if (drawit) {
		TargetPoint = GetCanvasPointPix(geo::Position(td->Lat, td->Lon));
		PredPoint = GetCanvasPointPix(pred);

		// Calculate the relative angle for this chart orientation
		// Use a 100 pixel vector to calculate angle
		double angle_distance_nm = (100.0 / GetVP().view_scale()) / 1852.0;
		geo::Position angle
			= geo::ll_gc_ll(geo::Position(td->Lat, td->Lon), td->COG, angle_distance_nm);
		wxPoint AnglePoint = GetCanvasPointPix(angle);

		double theta;

		if (abs(AnglePoint.x - TargetPoint.x) > 0) {
			if (target_sog > ais.ShowMoored_Kts)
				theta = atan2((double)(AnglePoint.y - TargetPoint.y),
							  (double)(AnglePoint.x - TargetPoint.x));
			else
				theta = -M_PI / 2;
		} else {
			if (AnglePoint.y > TargetPoint.y)
				theta = M_PI / 2.0; // valid COG 180
			else
				theta = -M_PI / 2.0; // valid COG 000 or speed is too low to resolve course
		}

		// Of course, if the target reported a valid HDG, then use it for icon
		if ((int)(td->HDG) != 511) {
			theta = ((td->HDG - 90) * M_PI / 180.0) + GetVP().rotation;
			if (!view.skew_comp && !nav.CourseUp)
				theta += GetVP().skew;
		}

		wxDash dash_long[2];
		dash_long[0] = (int)(1.0 * m_pix_per_mm); // Long dash  <---------+
		dash_long[1] = (int)(0.5 * m_pix_per_mm); // Short gap            |

		// Draw the icon rotated to the COG
		wxPoint ais_quad_icon[4];
		ais_quad_icon[0].x = -8;
		ais_quad_icon[0].y = -6;
		ais_quad_icon[1].x = 0;
		ais_quad_icon[1].y = 24;
		ais_quad_icon[2].x = 8;
		ais_quad_icon[2].y = -6;
		ais_quad_icon[3].x = 0;
		ais_quad_icon[3].y = -6;

		wxPoint ais_real_size[6];
		bool bcan_draw_size = true;
		if (view.DrawAISSize) {
			if (td->DimA + td->DimB == 0 || td->DimC + td->DimD == 0) {
				bcan_draw_size = false;
			} else {
				geo::Position ref
					= geo::ll_gc_ll(geo::Position(td->Lat, td->Lon), 0, 100.0 / 1852.0);
				wxPoint2DDouble b_point = GetVP().GetDoublePixFromLL(geo::Position(td->Lat, td->Lon));
				wxPoint2DDouble r_point = GetVP().GetDoublePixFromLL(ref);
				double ppm = r_point.GetDistance(b_point) / 100.0;
				double offwid = (td->DimC + td->DimD) * ppm * 0.25;
				double offlen = (td->DimA + td->DimB) * ppm * 0.15;
				ais_real_size[0].x = -td->DimD * ppm;
				ais_real_size[0].y = -td->DimB * ppm;
				ais_real_size[1].x = -td->DimD * ppm;
				ais_real_size[1].y = td->DimA * ppm - offlen;
				ais_real_size[2].x = -td->DimD * ppm + offwid;
				ais_real_size[2].y = td->DimA * ppm;
				ais_real_size[3].x = td->DimC * ppm - offwid;
				ais_real_size[3].y = td->DimA * ppm;
				ais_real_size[4].x = td->DimC * ppm;
				ais_real_size[4].y = td->DimA * ppm - offlen;
				ais_real_size[5].x = td->DimC * ppm;
				ais_real_size[5].y = -td->DimB * ppm;
				if (ais_real_size[4].x - ais_real_size[0].x < 16 || ais_real_size[2].y
																	- ais_real_size[0].y < 30)
					bcan_draw_size = false; // drawing too small does not make sense
				else
					bcan_draw_size = true;
			}
		}

		// If this is an AIS Class B target, so symbolize it differently
		if (td->Class == AIS_CLASS_B)
			ais_quad_icon[3].y = 0;
		else if (td->Class == AIS_GPSG_BUDDY) {
			ais_quad_icon[0].x = -5;
			ais_quad_icon[0].y = -12;
			ais_quad_icon[1].x = -3;
			ais_quad_icon[1].y = 12;
			ais_quad_icon[2].x = 3;
			ais_quad_icon[2].y = 12;
			ais_quad_icon[3].x = 5;
			ais_quad_icon[3].y = -12;
		} else if (td->Class == AIS_DSC) {
			ais_quad_icon[0].y = 0;
			ais_quad_icon[1].y = 8;
			ais_quad_icon[2].y = 0;
			ais_quad_icon[3].y = -8;
		} else if (td->Class == AIS_APRS) {
			ais_quad_icon[0].x = -8;
			ais_quad_icon[0].y = -8;
			ais_quad_icon[1].x = -8;
			ais_quad_icon[1].y = 8;
			ais_quad_icon[2].x = 8;
			ais_quad_icon[2].y = 8;
			ais_quad_icon[3].x = 8;
			ais_quad_icon[3].y = -8;
		}

		for (int i = 0; i < 4; i++) {
			double px = ((double)ais_quad_icon[i].x) * sin(theta) + ((double)ais_quad_icon[i].y)
																	* cos(theta);
			double py = ((double)ais_quad_icon[i].y) * sin(theta) - ((double)ais_quad_icon[i].x)
																	* cos(theta);
			ais_quad_icon[i].x = (int)round(px);
			ais_quad_icon[i].y = (int)round(py);
		}

		if (view.DrawAISSize && bcan_draw_size)
			for (int i = 0; i < 6; i++) {
				double px = ((double)ais_real_size[i].x) * sin(theta) + ((double)ais_real_size[i].y)
																		* cos(theta);
				double py = ((double)ais_real_size[i].y) * sin(theta) - ((double)ais_real_size[i].x)
																		* cos(theta);
				ais_real_size[i].x = (int)round(px);
				ais_real_size[i].y = (int)round(py);
			}

		dc.SetPen(wxPen(colors.get_color(_T("UBLCK"))));

		// Default color is green
		wxBrush target_brush = wxBrush(colors.get_color(_T("UINFG")));

		// Euro Inland targets render slightly differently
		if (td->b_isEuroInland)
			target_brush = wxBrush(colors.get_color(_T("TEAL1")));

		// and....
		if (!td->b_nameValid)
			target_brush = wxBrush(colors.get_color(_T("CHYLW")));
		if ((td->Class == AIS_DSC) && (td->ShipType == 12)) // distress
			target_brush = wxBrush(colors.get_color(_T("URED")));
		if (td->b_SarAircraftPosnReport)
			target_brush = wxBrush(colors.get_color(_T("UINFG")));

		if ((td->n_alarm_state == AIS_ALARM_SET) && (td->bCPA_Valid))
			target_brush = wxBrush(colors.get_color(_T("URED")));

		if (td->b_positionDoubtful)
			target_brush = wxBrush(colors.get_color(_T("UINFF")));

		// Check for alarms here, maintained by AIS class timer tick
		if (((td->n_alarm_state == AIS_ALARM_SET) && (td->bCPA_Valid))
			|| (td->b_show_AIS_CPA && (td->bCPA_Valid))) {
			// Calculate the point of CPA for target
			geo::Position tcpa = geo::ll_gc_ll(geo::Position(td->Lat, td->Lon), td->COG,
											   target_sog * td->TCPA / 60.0);
			wxPoint TPoint = TargetPoint;
			wxPoint tCPAPoint = GetCanvasPointPix(tcpa);

			// Draw the intercept line from target
			geo::ClipResult res = geo::cohen_sutherland_line_clip_i(
				&TPoint.x, &TPoint.y, &tCPAPoint.x, &tCPAPoint.y, 0, GetVP().pix_width, 0,
				GetVP().pix_height);

			if (res != geo::Invisible) {
				wxPen ppPen2(colors.get_color(_T("URED")), 2, wxUSER_DASH);
				ppPen2.SetDashes(2, dash_long);
				dc.SetPen(ppPen2);

				dc.StrokeLine(TPoint.x, TPoint.y, tCPAPoint.x, tCPAPoint.y);
			}

			// Calculate the point of CPA for ownship
			geo::Position ocpa;

			// Detect and handle the case where ownship COG is undefined....
			const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
			if (wxIsNaN(nav.cog) || wxIsNaN(nav.sog)) {
				ocpa = nav.pos;
			} else {
				ocpa = geo::ll_gc_ll(nav.pos, nav.cog, nav.sog * td->TCPA / 60.0);
			}

			wxPoint oCPAPoint = GetCanvasPointPix(ocpa);
			tCPAPoint = GetCanvasPointPix(tcpa);

			// Save a copy of these unclipped points
			wxPoint oCPAPoint_unclipped = oCPAPoint;
			wxPoint tCPAPoint_unclipped = tCPAPoint;

			// Draw a line from target CPA point to ownship CPA point
			geo::ClipResult ores = geo::cohen_sutherland_line_clip_i(
				&tCPAPoint.x, &tCPAPoint.y, &oCPAPoint.x, &oCPAPoint.y, 0, GetVP().pix_width, 0,
				GetVP().pix_height);

			if (ores != geo::Invisible) {
				wxColour yellow = colors.get_color(_T("YELO1"));
				dc.SetPen(wxPen(yellow, 4));
				dc.StrokeLine(tCPAPoint.x, tCPAPoint.y, oCPAPoint.x, oCPAPoint.y);

				wxPen ppPen2(colors.get_color(_T("URED")), 2, wxUSER_DASH);
				ppPen2.SetDashes(2, dash_long);
				dc.SetPen(ppPen2);
				dc.StrokeLine(tCPAPoint.x, tCPAPoint.y, oCPAPoint.x, oCPAPoint.y);

				// Draw little circles at the ends of the CPA alert line
				wxBrush br(colors.get_color(_T("BLUE3")));
				dc.SetBrush(br);
				dc.SetPen(wxPen(colors.get_color(_T("UBLK"))));

				// Using the true ends, not the clipped ends
				dc.StrokeCircle(tCPAPoint_unclipped.x, tCPAPoint_unclipped.y, 5);
				dc.StrokeCircle(oCPAPoint_unclipped.x, oCPAPoint_unclipped.y, 5);
			}

			// Draw the intercept line from ownship
			wxPoint oShipPoint = GetCanvasPointPix(nav.pos);
			oCPAPoint = oCPAPoint_unclipped; // recover the unclipped point

			geo::ClipResult ownres = geo::cohen_sutherland_line_clip_i(
				&oShipPoint.x, &oShipPoint.y, &oCPAPoint.x, &oCPAPoint.y, 0, GetVP().pix_width, 0,
				GetVP().pix_height);

			if (ownres != geo::Invisible) {
				wxPen ppPen2(colors.get_color(_T("URED")), 2, wxUSER_DASH);
				ppPen2.SetDashes(2, dash_long);
				dc.SetPen(ppPen2);

				dc.StrokeLine(oShipPoint.x, oShipPoint.y, oCPAPoint.x, oCPAPoint.y);
			} // TR : till here

			dc.SetPen(wxPen(colors.get_color(_T("UBLCK"))));
			dc.SetBrush(wxBrush(colors.get_color(_T("URED"))));
		}

		// Highlight the AIS target symbol if an alert dialog is currently open for it
		if (g_pais_alert_dialog_active && g_pais_alert_dialog_active->IsShown()) {
			if (g_pais_alert_dialog_active->Get_Dialog_MMSI() == td->MMSI)
				JaggyCircle(dc, wxPen(colors.get_color(_T("URED")), 2), TargetPoint.x,
							TargetPoint.y, 100);
		}

		// Highlight the AIS target symbol if a query dialog is currently open for it
		if (g_pais_query_dialog_active && g_pais_query_dialog_active->IsShown()) {
			if (g_pais_query_dialog_active->GetMMSI() == td->MMSI)
				TargetFrame(dc, wxPen(colors.get_color(_T("UBLCK")), 2), TargetPoint.x,
							TargetPoint.y, 25);
		}

		// Render the COG line if the speed is greater than moored speed defined by ais
		// options dialog
		if (ais.ShowCOG && (target_sog > ais.ShowMoored_Kts)) {
			int pixx = TargetPoint.x;
			int pixy = TargetPoint.y;
			int pixx1 = PredPoint.x;
			int pixy1 = PredPoint.y;

			// Don't draw the COG line  and predictor point if zoomed far out.... or if target
			// lost/inactive
			double l = sqrt(util::sqr((double)(PredPoint.x - TargetPoint.x))
							+ util::sqr((double)(PredPoint.y - TargetPoint.y)));

			if (l > 24) {
				geo::ClipResult res = geo::cohen_sutherland_line_clip_i(
					&pixx, &pixy, &pixx1, &pixy1, 0, GetVP().pix_width, 0, GetVP().pix_height);

				if ((res != geo::Invisible) && (td->b_active)) {
					const global::GUI::View& view = global::OCPN::get().gui().view();
					// Draw a wider coloured line
					wxPen wide_pen(target_brush.GetColour(), view.ais_cog_predictor_width);
					dc.SetPen(wide_pen);
					dc.StrokeLine(pixx, pixy, pixx1, pixy1);

					if (view.ais_cog_predictor_width > 1) {
						// Draw a 1 pixel wide black line
						wxPen narrow_pen(colors.get_color(_T("UBLCK")), 1);
						dc.SetPen(narrow_pen);
						dc.StrokeLine(pixx, pixy, pixx1, pixy1);
					}

					dc.SetBrush(target_brush);
					dc.StrokeCircle(PredPoint.x, PredPoint.y, 5);
				}

				// Draw RateOfTurn Vector
				if ((td->ROTAIS != 0) && (td->ROTAIS != -128) && td->b_active) {
					double nv = 10;
					double theta2 = theta;
					if (td->ROTAIS > 0)
						theta2 += M_PI / 2.0;
					else
						theta2 -= M_PI / 2.0;

					int xrot = (int)round(pixx1 + (nv * cos(theta2)));
					int yrot = (int)round(pixy1 + (nv * sin(theta2)));
					dc.StrokeLine(pixx1, pixy1, xrot, yrot);
					dc.CalcBoundingBox(xrot, yrot);
				}
			}
		}

		// Actually Draw the target
		if (td->Class == AIS_ARPA) {
			draw_ais_ARPA(dc, TargetPoint, target_brush, td);
		} else if (td->Class == AIS_ATON) { // Aid to Navigation
			draw_ais_ATON(dc, TargetPoint, target_brush, td);
		} else if (td->Class == AIS_BASE) { // Base Station
			draw_ais_BASE(dc, TargetPoint, target_brush, td);
		} else if (td->Class == AIS_SART) { // SART Target
			draw_ais_SART(dc, TargetPoint, target_brush, td);
		} else if (td->b_SarAircraftPosnReport) {
			// FIXME: move to separate method

			wxPoint SarIcon[10];
			wxPoint SarRot[10];

			SarIcon[0] = wxPoint(0, 12);
			SarIcon[1] = wxPoint(4, 2);
			SarIcon[2] = wxPoint(16, -2);
			SarIcon[3] = wxPoint(16, -8);
			SarIcon[4] = wxPoint(4, -8);
			SarIcon[5] = wxPoint(3, -16);
			SarIcon[6] = wxPoint(10, -18);
			SarIcon[7] = wxPoint(10, -22);
			SarIcon[8] = wxPoint(0, -22);

			// Draw icon as two halves

			//  First half

			for (int i = 0; i < 9; i++) {
				double px = ((double)SarIcon[i].x) * sin(theta) + ((double)SarIcon[i].y)
																  * cos(theta);
				double py = ((double)SarIcon[i].y) * sin(theta) - ((double)SarIcon[i].x)
																  * cos(theta);
				SarRot[i].x = (int)round(px);
				SarRot[i].y = (int)round(py);
			}
			wxPoint ais_tri_icon[3];

			wxPen tri_pen(target_brush.GetColour(), 1);
			dc.SetPen(tri_pen);
			dc.SetBrush(target_brush);

			ais_tri_icon[0] = SarRot[0];
			ais_tri_icon[1] = SarRot[1];
			ais_tri_icon[2] = SarRot[4];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
			ais_tri_icon[0] = SarRot[1];
			ais_tri_icon[1] = SarRot[2];
			ais_tri_icon[2] = SarRot[3];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
			ais_tri_icon[0] = SarRot[1];
			ais_tri_icon[1] = SarRot[3];
			ais_tri_icon[2] = SarRot[4];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
			ais_tri_icon[0] = SarRot[0];
			ais_tri_icon[1] = SarRot[4];
			ais_tri_icon[2] = SarRot[5];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
			ais_tri_icon[0] = SarRot[0];
			ais_tri_icon[1] = SarRot[5];
			ais_tri_icon[2] = SarRot[8];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
			ais_tri_icon[0] = SarRot[5];
			ais_tri_icon[1] = SarRot[6];
			ais_tri_icon[2] = SarRot[7];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
			ais_tri_icon[0] = SarRot[5];
			ais_tri_icon[1] = SarRot[7];
			ais_tri_icon[2] = SarRot[8];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);

			wxPen target_outline_pen(colors.get_color(_T("UBLCK")), 2);
			dc.SetPen(target_outline_pen);
			dc.SetBrush(wxBrush(colors.get_color(_T("UBLCK")), wxTRANSPARENT));
			dc.StrokePolygon(9, SarRot, TargetPoint.x, TargetPoint.y);

			// second half

			for (int i = 0; i < 9; i++) { // mirror the icon (x -> -x)
				double px = (-(double)SarIcon[i].x) * sin(theta) + ((double)SarIcon[i].y)
																   * cos(theta);
				double py = ((double)SarIcon[i].y) * sin(theta) - (-(double)SarIcon[i].x)
																  * cos(theta);
				SarRot[i].x = (int)round(px);
				SarRot[i].y = (int)round(py);
			}

			dc.SetPen(tri_pen);
			dc.SetBrush(target_brush);

			ais_tri_icon[0] = SarRot[0];
			ais_tri_icon[1] = SarRot[1];
			ais_tri_icon[2] = SarRot[4];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
			ais_tri_icon[0] = SarRot[1];
			ais_tri_icon[1] = SarRot[2];
			ais_tri_icon[2] = SarRot[3];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
			ais_tri_icon[0] = SarRot[1];
			ais_tri_icon[1] = SarRot[3];
			ais_tri_icon[2] = SarRot[4];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
			ais_tri_icon[0] = SarRot[0];
			ais_tri_icon[1] = SarRot[4];
			ais_tri_icon[2] = SarRot[5];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
			ais_tri_icon[0] = SarRot[0];
			ais_tri_icon[1] = SarRot[5];
			ais_tri_icon[2] = SarRot[8];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
			ais_tri_icon[0] = SarRot[5];
			ais_tri_icon[1] = SarRot[6];
			ais_tri_icon[2] = SarRot[7];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
			ais_tri_icon[0] = SarRot[5];
			ais_tri_icon[1] = SarRot[7];
			ais_tri_icon[2] = SarRot[8];
			dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);

			dc.SetPen(target_outline_pen);
			dc.SetBrush(wxBrush(colors.get_color(_T("UBLCK")), wxTRANSPARENT));
			dc.StrokePolygon(9, SarRot, TargetPoint.x, TargetPoint.y);

		} else { // ship class A or B or a Buddy or DSC
			wxPen target_pen(colors.get_color(_T("UBLCK")), 1);

			dc.SetPen(target_pen);
			dc.SetBrush(target_brush);

			if (td->Class == AIS_CLASS_B) {
				// decompose to two "convex" polygons and one combined outline to satisfy OpenGL's
				// requirements
				wxPen tri_pen(target_brush.GetColour(), 1);
				dc.SetPen(tri_pen);

				wxPoint ais_tri_icon[3];

				ais_tri_icon[0] = ais_quad_icon[0];
				ais_tri_icon[1] = ais_quad_icon[1];
				ais_tri_icon[2] = ais_quad_icon[3];
				dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);

				ais_tri_icon[0] = ais_quad_icon[1];
				ais_tri_icon[1] = ais_quad_icon[2];
				ais_tri_icon[2] = ais_quad_icon[3];
				dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);

				dc.SetPen(target_pen);
				dc.SetBrush(wxBrush(colors.get_color(_T("UBLCK")), wxTRANSPARENT));
				dc.StrokePolygon(4, ais_quad_icon, TargetPoint.x, TargetPoint.y);

			} else
				dc.StrokePolygon(4, ais_quad_icon, TargetPoint.x, TargetPoint.y);

			if (view.DrawAISSize && bcan_draw_size) {
				dc.SetBrush(wxBrush(colors.get_color(_T("UBLCK")), wxTRANSPARENT));
				dc.StrokePolygon(6, ais_real_size, TargetPoint.x, TargetPoint.y);
			}

			dc.SetBrush(wxBrush(colors.get_color(_T("SHIPS"))));
			int navstatus = td->NavStatus;

			// HSC usually have correct ShipType but navstatus == 0...
			if (((td->ShipType >= 40) && (td->ShipType < 50)) && navstatus == UNDERWAY_USING_ENGINE)
				navstatus = HSC;

			switch (navstatus) {
				case MOORED:
				case AT_ANCHOR: {
					dc.StrokeCircle(TargetPoint.x, TargetPoint.y, 4);
					break;
				}
				case RESTRICTED_MANOEUVRABILITY: {
					wxPoint diamond[4];
					diamond[0] = wxPoint(4, 0);
					diamond[1] = wxPoint(0, -6);
					diamond[2] = wxPoint(-4, 0);
					diamond[3] = wxPoint(0, 6);
					dc.StrokePolygon(4, diamond, TargetPoint.x, TargetPoint.y - 11);
					dc.StrokeCircle(TargetPoint.x, TargetPoint.y, 4);
					dc.StrokeCircle(TargetPoint.x, TargetPoint.y - 22, 4);
					break;
					break;
				}
				case CONSTRAINED_BY_DRAFT: {
					wxPoint can[4];
					can[0] = wxPoint(-3, 0);
					can[1] = wxPoint(3, 0);
					can[2] = wxPoint(3, -16);
					can[3] = wxPoint(-3, -16);
					dc.StrokePolygon(4, can, TargetPoint.x, TargetPoint.y);
					break;
				}
				case NOT_UNDER_COMMAND: {
					dc.StrokeCircle(TargetPoint.x, TargetPoint.y, 4);
					dc.StrokeCircle(TargetPoint.x, TargetPoint.y - 9, 4);
					break;
				}
				case FISHING: {
					wxPoint tri[3];
					tri[0] = wxPoint(-4, 0);
					tri[1] = wxPoint(4, 0);
					tri[2] = wxPoint(0, -9);
					dc.StrokePolygon(3, tri, TargetPoint.x, TargetPoint.y);
					tri[0] = wxPoint(0, -9);
					tri[1] = wxPoint(4, -18);
					tri[2] = wxPoint(-4, -18);
					dc.StrokePolygon(3, tri, TargetPoint.x, TargetPoint.y);
					break;
				}
				case AGROUND: {
					dc.StrokeCircle(TargetPoint.x, TargetPoint.y, 4);
					dc.StrokeCircle(TargetPoint.x, TargetPoint.y - 9, 4);
					dc.StrokeCircle(TargetPoint.x, TargetPoint.y - 18, 4);
					break;
				}
				case HSC:
				case WIG: {
					wxPoint arrow[3];
					arrow[0] = wxPoint(-4, 20);
					arrow[1] = wxPoint(0, 27);
					arrow[2] = wxPoint(4, 20);
					for (int i = 0; i < 3; i++) {
						double px = ((double)arrow[i].x) * sin(theta) + ((double)arrow[i].y)
																		* cos(theta);
						double py = ((double)arrow[i].y) * sin(theta) - ((double)arrow[i].x)
																		* cos(theta);
						arrow[i].x = (int)round(px);
						arrow[i].y = (int)round(py);
					}
					dc.SetBrush(target_brush);
					dc.StrokePolygon(3, arrow, TargetPoint.x, TargetPoint.y);
					arrow[0] = wxPoint(-4, 27);
					arrow[1] = wxPoint(0, 34);
					arrow[2] = wxPoint(4, 27);
					for (int i = 0; i < 3; i++) {
						double px = ((double)arrow[i].x) * sin(theta) + ((double)arrow[i].y)
																		* cos(theta);
						double py = ((double)arrow[i].y) * sin(theta) - ((double)arrow[i].x)
																		* cos(theta);
						arrow[i].x = (int)round(px);
						arrow[i].y = (int)round(py);
					}
					dc.StrokePolygon(3, arrow, TargetPoint.x, TargetPoint.y);
					break;
				}
			}

			// Draw the inactive cross-out line
			if (!td->b_active) {
				dc.SetPen(wxPen(colors.get_color(_T("UBLCK")), 2));

				wxPoint p1 = transrot(wxPoint(-14, 0), theta, TargetPoint);
				wxPoint p2 = transrot(wxPoint(14, 0), theta, TargetPoint);
				dc.StrokeLine(p1.x, p1.y, p2.x, p2.y);
				dc.CalcBoundingBox(p1.x, p1.y);
				dc.CalcBoundingBox(p2.x, p2.y);

				dc.SetPen(wxPen(colors.get_color(_T("UBLCK")), 1));
			}

			// European Inland AIS define a "stbd-stbd" meeting sign, a blue paddle.
			// Symbolize it if set by most recent message
			if (td->b_blue_paddle) {
				wxPoint ais_flag_icon[4];
				ais_flag_icon[0].x = -8;
				ais_flag_icon[0].y = -6;
				ais_flag_icon[1].x = -2;
				ais_flag_icon[1].y = 18;
				ais_flag_icon[2].x = -2;
				ais_flag_icon[2].y = 0;
				ais_flag_icon[3].x = -2;
				ais_flag_icon[3].y = -6;

				for (int i = 0; i < 4; i++) {
					double px = ((double)ais_flag_icon[i].x) * sin(theta)
								+ ((double)ais_flag_icon[i].y) * cos(theta);
					double py = ((double)ais_flag_icon[i].y) * sin(theta)
								- ((double)ais_flag_icon[i].x) * cos(theta);
					ais_flag_icon[i].x = (int)round(px);
					ais_flag_icon[i].y = (int)round(py);
				}

				dc.SetBrush(wxBrush(colors.get_color(_T("UINFB"))));
				dc.SetPen(wxPen(colors.get_color(_T("CHWHT")), 2));
				dc.StrokePolygon(4, ais_flag_icon, TargetPoint.x, TargetPoint.y);
			}
		}

		if (view.ShowAISName) {
			double true_scale_display = floor(VPoint.chart_scale / 100.0) * 100.0;
			if (true_scale_display < view.Show_Target_Name_Scale) { // from which scale to display name

				wxString tgt_name = td->GetFullName();
				tgt_name = tgt_name.substr(0, tgt_name.find(_T("Unknown"), 0));

				if (tgt_name != wxEmptyString) {
					dc.SetFont(*FontMgr::Get().GetFont(_("AIS Target Name"), 12));
					dc.SetTextForeground(FontMgr::Get().GetFontColor(_("AIS Target Name")));

					int w, h;
					dc.GetTextExtent(tgt_name, &w, &h);

					if ((td->COG > 90) && (td->COG < 180))
						dc.DrawText(tgt_name, TargetPoint.x + 10, TargetPoint.y - h);
					else
						dc.DrawText(tgt_name, TargetPoint.x + 10, TargetPoint.y + 0.5 * h);

				}
			}
		}

		// Draw tracks if enabled
		if (td->b_show_track) {
			wxPoint TrackPointA;
			wxPoint TrackPointB;

			dc.SetPen(wxPen(colors.get_color(_T("CHMGD")), 2));

			// First point
			ais::AISTargetTrackList::iterator track_point = td->m_ptrack.begin();
			if (track_point != td->m_ptrack.end()) {
				TrackPointA = GetCanvasPointPix(geo::Position(track_point->m_lat, track_point->m_lon));
				++track_point;
			}
			while (track_point != td->m_ptrack.end()) {
				TrackPointB = GetCanvasPointPix(geo::Position(track_point->m_lat, track_point->m_lon));
				dc.StrokeLine(TrackPointA, TrackPointB);
				++track_point;
				TrackPointA = TrackPointB;
			}
		}
	}
}

void ChartCanvas::draw_ais_ARPA(ocpnDC& dc, const wxPoint& TargetPoint, const wxBrush& target_brush,
								const ais::AIS_Target_Data* td) const
{
	const global::ColorManager& colors = global::OCPN::get().color();

	wxPen target_pen(colors.get_color(_T("UBLCK")), 2);

	dc.SetPen(target_pen);
	dc.SetBrush(target_brush);

	dc.StrokeCircle(TargetPoint.x, TargetPoint.y, 9);
	dc.StrokeCircle(TargetPoint.x, TargetPoint.y, 1);
	// Draw the inactive cross-out line
	if (!td->b_active) {
		dc.SetPen(wxPen(colors.get_color(_T("UBLCK")), 2));
		dc.StrokeLine(TargetPoint.x - 14, TargetPoint.y, TargetPoint.x + 14, TargetPoint.y);
		dc.CalcBoundingBox(TargetPoint.x - 14, TargetPoint.y);
		dc.CalcBoundingBox(TargetPoint.x + 14, TargetPoint.y);
		dc.SetPen(wxPen(colors.get_color(_T("UBLCK")), 1));
	}
}

void ChartCanvas::draw_ais_ATON(ocpnDC& dc, const wxPoint& TargetPoint, const wxBrush&,
								const ais::AIS_Target_Data* td) const
{
	using namespace ais;

	const global::ColorManager& colors = global::OCPN::get().color();

	wxPen aton_pen;
	if ((td->NavStatus == ATON_VIRTUAL_OFFPOSITION) || (td->NavStatus == ATON_REAL_OFFPOSITION))
		aton_pen = wxPen(colors.get_color(_T("URED")), 2);
	else
		aton_pen = wxPen(colors.get_color(_T("UBLCK")), 2);

	bool b_virt = (td->NavStatus == ATON_VIRTUAL) | (td->NavStatus == ATON_VIRTUAL_ONPOSITION)
				  | (td->NavStatus == ATON_VIRTUAL_OFFPOSITION);

	AtoN_Diamond(dc, aton_pen, TargetPoint.x, TargetPoint.y, 12, b_virt);
}

void ChartCanvas::draw_ais_BASE(ocpnDC& dc, const wxPoint& TargetPoint, const wxBrush&,
								const ais::AIS_Target_Data*) const
{
	const global::ColorManager& colors = global::OCPN::get().color();

	Base_Square(dc, wxPen(colors.get_color(_T("UBLCK")), 2), TargetPoint.x, TargetPoint.y, 8);
}

void ChartCanvas::draw_ais_SART(ocpnDC& dc, const wxPoint& TargetPoint, const wxBrush&,
								const ais::AIS_Target_Data* td) const
{
	const global::ColorManager& colors = global::OCPN::get().color();

	if (td->NavStatus == 14) // active
		SART_Render(dc, wxPen(colors.get_color(_T("URED")), 2), TargetPoint.x, TargetPoint.y, 8);
	else
		SART_Render(dc, wxPen(colors.get_color(_T("UGREN")), 2), TargetPoint.x, TargetPoint.y, 8);
}

void ChartCanvas::JaggyCircle(ocpnDC& dc, wxPen pen, int x, int y, int radius)
{
	// Constants?
	double da_min = 2.0;
	double da_max = 6.0;
	double ra_min = 0.0;
	double ra_max = 40.0;

	wxPen pen_save = dc.GetPen();

	dc.SetPen(pen);

	int x0;
	int y0;
	int x1;
	int y1;

	x0 = x + radius; // Start point
	y0 = y;
	double angle = 0.0;
	int i = 0;

	while (angle < 360.0) {
		double da = da_min + (((double)rand() / RAND_MAX) * (da_max - da_min));
		angle += da;

		if (angle > 360.0)
			angle = 360.0;

		double ra = ra_min + (((double)rand() / RAND_MAX) * (ra_max - ra_min));

		double r;
		if (i % 1)
			r = radius + ra;
		else
			r = radius - ra;

		x1 = (int)(x + cos(angle * M_PI / 180.0) * r);
		y1 = (int)(y + sin(angle * M_PI / 180.0) * r);

		dc.DrawLine(x0, y0, x1, y1);

		x0 = x1;
		y0 = y1;

		i++;
	}

	dc.DrawLine(x + radius, y, x1, y1); // closure
	dc.SetPen(pen_save);
}

void ChartCanvas::TargetFrame(ocpnDC& dc, wxPen pen, int x, int y, int radius) const
{
	const int gap2 = 2 * radius / 6;

	wxPen pen_save = dc.GetPen();

	dc.SetPen(pen);

	dc.DrawLine(x - radius, y + gap2, x - radius, y + radius);
	dc.DrawLine(x - radius, y + radius, x - gap2, y + radius);
	dc.DrawLine(x + gap2, y + radius, x + radius, y + radius);
	dc.DrawLine(x + radius, y + radius, x + radius, y + gap2);
	dc.DrawLine(x + radius, y - gap2, x + radius, y - radius);
	dc.DrawLine(x + radius, y - radius, x + gap2, y - radius);
	dc.DrawLine(x - gap2, y - radius, x - radius, y - radius);
	dc.DrawLine(x - radius, y - radius, x - radius, y - gap2);

	dc.SetPen(pen_save);
}

void ChartCanvas::AtoN_Diamond(ocpnDC& dc, wxPen pen, int x, int y, int radius,
							   bool b_virtual) const
{
	const int gap2 = 2 * radius / 8;
	int pen_width = pen.GetWidth();

	wxPen pen_save = dc.GetPen();

	dc.SetPen(pen); // draw diamond

	dc.DrawLine(x - radius, y, x, y + radius);
	dc.DrawLine(x, y + radius, x + radius, y);
	dc.DrawLine(x + radius, y, x, y - radius);
	dc.DrawLine(x, y - radius, x - radius, y);

	// draw cross inside
	if (pen_width > 1) {
		pen_width -= 1;
		pen.SetWidth(pen_width);
		dc.SetPen(pen);
	}

	dc.DrawLine(x - gap2, y, x + gap2, y);
	dc.DrawLine(x, y - gap2, x, y + gap2);

	if (b_virtual) {
		dc.DrawLine(x - gap2 - 3, y - 1, x, y + gap2 + 5);
		dc.DrawLine(x, y + gap2 + 5, x + gap2 + 4, y - 2);
	}

	dc.SetPen(pen_save);
}

void ChartCanvas::Base_Square(ocpnDC& dc, wxPen pen, int x, int y, int radius) const
{
	const int gap2 = 2 * radius / 6;
	int pen_width = pen.GetWidth();

	wxPen pen_save = dc.GetPen();

	dc.SetPen(pen); // draw square

	dc.DrawLine(x - radius, y - radius, x - radius, y + radius);
	dc.DrawLine(x - radius, y + radius, x + radius, y + radius);
	dc.DrawLine(x + radius, y + radius, x + radius, y - radius);
	dc.DrawLine(x + radius, y - radius, x - radius, y - radius);

	// draw cross inside
	if (pen_width > 1) {
		pen_width -= 1;
		pen.SetWidth(pen_width);
	}

	dc.DrawLine(x - gap2, y, x + gap2, y);
	dc.DrawLine(x, y - gap2, x, y + gap2);

	dc.SetPen(pen_save);
}

void ChartCanvas::SART_Render(ocpnDC& dc, wxPen pen, int x, int y, int radius) const
{
	const int gap = (radius * 12) / 10;
	int pen_width = pen.GetWidth();

	wxPen pen_save = dc.GetPen();

	dc.SetPen(pen);

	wxBrush brush_save = dc.GetBrush();
	wxBrush* ppBrush = wxTheBrushList->FindOrCreateBrush(wxColour(0, 0, 0), wxTRANSPARENT);
	dc.SetBrush(*ppBrush);

	dc.DrawCircle(x, y, radius);

	if (pen_width > 1) {
		pen_width -= 1;
		pen.SetWidth(pen_width);
	}

	// draw cross inside
	dc.DrawLine(x - gap, y - gap, x + gap, y + gap);
	dc.DrawLine(x - gap, y + gap, x + gap, y - gap);

	dc.SetBrush(brush_save);
	dc.SetPen(pen_save);
}

void ChartCanvas::AnchorWatchDraw(ocpnDC& dc)
{
	// FIXME: code duplication
	// Just for prototyping, visual alert for anchorwatch goes here

	const global::ColorManager& colors = global::OCPN::get().color();
	global::Navigation& nav = global::OCPN::get().nav();

	bool play_sound = false;
	if (pAnchorWatchPoint1 && nav.anchor().AlertOn1) {
		if (nav.anchor().AlertOn1) {
			wxPoint TargetPoint = GetCanvasPointPix(pAnchorWatchPoint1->get_position());
			JaggyCircle(dc, wxPen(colors.get_color(_T("URED")), 2), TargetPoint.x, TargetPoint.y,
						100);
			play_sound = true;
		}
	} else {
		nav.set_anchor_AlertOn1(false);
	}

	if (pAnchorWatchPoint2 && nav.anchor().AlertOn2) {
		if (nav.anchor().AlertOn2) {
			wxPoint TargetPoint = GetCanvasPointPix(pAnchorWatchPoint2->get_position());
			JaggyCircle(dc, wxPen(colors.get_color(_T("URED")), 2), TargetPoint.x, TargetPoint.y,
						100);
			play_sound = true;
		}
	} else {
		nav.set_anchor_AlertOn2(false);
	}

	if (play_sound) {
		const global::AIS::Data& ais = global::OCPN::get().ais().get_data();

		if (!g_anchorwatch_sound.IsOk())
			g_anchorwatch_sound.Create(ais.AIS_Alert_Sound_File);

#ifndef __WXMSW__
		if (g_anchorwatch_sound.IsOk() && !g_anchorwatch_sound.IsPlaying())
			g_anchorwatch_sound.Play();
#else
		if (g_anchorwatch_sound.IsOk())
			g_anchorwatch_sound.Play();
#endif
	} else {
		if (g_anchorwatch_sound.IsOk())
			g_anchorwatch_sound.Stop();
	}
}

void ChartCanvas::UpdateShips()
{
	// Get the rectangle in the current dc which bounds the "ownship" symbol

	wxClientDC dc(this);
	if (!dc.IsOk())
		return;

	wxBitmap test_bitmap(dc.GetSize().x, dc.GetSize().y);
	wxMemoryDC temp_dc(test_bitmap);

	temp_dc.ResetBoundingBox();
	temp_dc.DestroyClippingRegion();
	temp_dc.SetClippingRegion(0, 0, dc.GetSize().x, dc.GetSize().y);

	// Draw the ownship on the temp_dc
	ocpnDC ocpndc = ocpnDC(temp_dc);
	ShipDraw(ocpndc);

	const navigation::RouteTracker& tracker = global::OCPN::get().tracker();
	if (tracker.has_active_track() && tracker.is_running()) {
		wxPoint px = cc1->GetCanvasPointPix(tracker.get_last_position());
		ocpndc.CalcBoundingBox(px.x, px.y);
	}

	ship_draw_rect = wxRect(temp_dc.MinX(), temp_dc.MinY(), temp_dc.MaxX() - temp_dc.MinX(),
							temp_dc.MaxY() - temp_dc.MinY());

	wxRect own_ship_update_rect = ship_draw_rect;

	if (!own_ship_update_rect.IsEmpty()) {
		// The required invalidate rectangle is the union of the last drawn rectangle
		// and this drawn rectangle
		own_ship_update_rect.Union(ship_draw_last_rect);
		own_ship_update_rect.Inflate(2);
	}

	if (!own_ship_update_rect.IsEmpty())
		RefreshRect(own_ship_update_rect, false);

	ship_draw_last_rect = ship_draw_rect;

	temp_dc.SelectObject(wxNullBitmap);
}

void ChartCanvas::UpdateAlerts()
{
	// Get the rectangle in the current dc which bounds the detected Alert targets

	// Use this dc
	wxClientDC dc(this);

	// Get dc boundary
	int sx, sy;
	dc.GetSize(&sx, &sy);

	//  Need a bitmap
	wxBitmap test_bitmap(sx, sy, -1);

	// Create a memory DC
	wxMemoryDC temp_dc;
	temp_dc.SelectObject(test_bitmap);

	temp_dc.ResetBoundingBox();
	temp_dc.DestroyClippingRegion();
	temp_dc.SetClippingRegion(wxRect(0, 0, sx, sy));

	// Draw the Alert Targets on the temp_dc
	ocpnDC ocpndc = ocpnDC(temp_dc);
	AnchorWatchDraw(ocpndc);

	// Retrieve the drawing extents
	wxRect alert_rect(temp_dc.MinX(), temp_dc.MinY(), temp_dc.MaxX() - temp_dc.MinX(),
					  temp_dc.MaxY() - temp_dc.MinY());

	if (!alert_rect.IsEmpty())
		alert_rect.Inflate(2); // clear all drawing artifacts

	if (!alert_rect.IsEmpty() || !alert_draw_rect.IsEmpty()) {
		//  The required invalidate rectangle is the union of the last drawn rectangle
		//  and this drawn rectangle
		wxRect alert_update_rect = alert_draw_rect;
		alert_update_rect.Union(alert_rect);

		//  Invalidate the rectangular region
		RefreshRect(alert_update_rect, false);
	}

	//  Save this rectangle for next time
	alert_draw_rect = alert_rect;

	temp_dc.SelectObject(wxNullBitmap); // clean up
}

void ChartCanvas::UpdateAIS()
{
	if (!g_pAIS)
		return;

	// Get the rectangle in the current dc which bounds the detected AIS targets

	// Use this dc
	wxClientDC dc(this);

	// Get dc boundary
	int sx;
	int sy;
	dc.GetSize(&sx, &sy);

	wxRect ais_rect;

	// How many targets are there?

	// If more than "some number", it will be cheaper to refresh the entire screen
	// than to build update rectangles for each target.
	AIS_Target_Hash* current_targets = g_pAIS->GetTargetList();
	if (current_targets->size() > 10) {
		ais_rect = wxRect(0, 0, sx, sy); // full screen
	} else {
		//  Need a bitmap
		wxBitmap test_bitmap(sx, sy, -1);

		// Create a memory DC
		wxMemoryDC temp_dc;
		temp_dc.SelectObject(test_bitmap);

		temp_dc.ResetBoundingBox();
		temp_dc.DestroyClippingRegion();
		temp_dc.SetClippingRegion(wxRect(0, 0, sx, sy));

		// Draw the AIS Targets on the temp_dc
		ocpnDC ocpndc = ocpnDC(temp_dc);
		AISDraw(ocpndc);
		AISDrawAreaNotices(ocpndc);

		// Retrieve the drawing extents
		ais_rect = wxRect(temp_dc.MinX(), temp_dc.MinY(), temp_dc.MaxX() - temp_dc.MinX(),
						  temp_dc.MaxY() - temp_dc.MinY());

		if (!ais_rect.IsEmpty())
			ais_rect.Inflate(2); // clear all drawing artifacts

		temp_dc.SelectObject(wxNullBitmap); // clean up
	}

	if (!ais_rect.IsEmpty() || !ais_draw_rect.IsEmpty()) {
		// The required invalidate rectangle is the union of the last drawn rectangle
		// and this drawn rectangle
		wxRect ais_update_rect = ais_draw_rect;
		ais_update_rect.Union(ais_rect);

		// Invalidate the rectangular region
		RefreshRect(ais_update_rect, false);
	}

	// Save this rectangle for next time
	ais_draw_rect = ais_rect;
}

void ChartCanvas::OnActivate(wxActivateEvent&)
{
	ReloadVP();
}

void ChartCanvas::OnSize(wxSizeEvent& event)
{
	GetClientSize(&m_canvas_width, &m_canvas_height);

	// Constrain the active width to be mod 4

	int wr = m_canvas_width / 4;
	m_canvas_width = wr * 4;

	// Get some canvas metrics

	// Rescale to current value, in order to rebuild VPoint data structures
	// for new canvas size
	SetVPScale(GetVPScale());

	// gives screen size(width) in meters
	double display_size_meters = wxGetDisplaySizeMM().GetWidth() / 1000.0;
	m_canvas_scale_factor = wxGetDisplaySize().GetWidth() / display_size_meters;

	// something like 180 degrees
	m_absolute_min_scale_ppm = m_canvas_width / (0.95 * geo::WGS84_semimajor_axis_meters * M_PI);

#ifdef USE_S57
	if (ps52plib)
		ps52plib->SetPPMM(m_canvas_scale_factor / 1000.0);
#endif

	// Inform the parent Frame that I am being resized...
	gFrame->ProcessCanvasResize();

	// Set up the scroll margins
	xr_margin = (m_canvas_width * 95) / 100;
	xl_margin = (m_canvas_width * 5) / 100;
	yt_margin = (m_canvas_height * 5) / 100;
	yb_margin = (m_canvas_height * 95) / 100;

	if (m_pQuilt)
		m_pQuilt->SetQuiltParameters(m_canvas_scale_factor, m_canvas_width);

	// Resize the current viewport

	VPoint.pix_width = m_canvas_width;
	VPoint.pix_height = m_canvas_height;

	// Resize the scratch BM
	delete pscratch_bm;
	pscratch_bm = new wxBitmap(VPoint.pix_width, VPoint.pix_height, -1);

	// Resize the Route Calculation BM
	m_dc_route.SelectObject(wxNullBitmap);
	delete proute_bm;
	proute_bm = new wxBitmap(VPoint.pix_width, VPoint.pix_height, -1);
	m_dc_route.SelectObject(*proute_bm);

	// Resize the saved Bitmap
	m_cached_chart_bm.Create(VPoint.pix_width, VPoint.pix_height, -1);

	// Resize the working Bitmap
	m_working_bm.Create(VPoint.pix_width, VPoint.pix_height, -1);

	// Rescale again, to capture all the changes for new canvas size
	SetVPScale(GetVPScale());

#ifdef ocpnUSE_GL
	if (global::OCPN::get().gui().view().opengl && m_glcc) {
		m_glcc->OnSize(event);
	}
#endif
	// Invalidate the whole window
	ReloadVP();
}

wxBitmap& ChartCanvas::get_scratch_bitmap()
{
	return *pscratch_bm;
}

void ChartCanvas::ShowChartInfoWindow(int x, int, int dbIndex)
{
	if (dbIndex >= 0) {
		if (NULL == m_pCIWin) {
			m_pCIWin = new ChInfoWin(this);
			m_pCIWin->Hide();
		}

		if (!m_pCIWin->IsShown()) {
			using namespace chart;

			ChartBase* pc = NULL;

			if ((ChartData->IsChartInCache(dbIndex)) && ChartData->IsValid())
				pc = ChartData->OpenChartFromDB(dbIndex, FULL_INIT); // this must come from cache

			int char_width, char_height;
			wxString s = ChartData->GetFullChartInfo(pc, dbIndex, &char_width, &char_height);
			m_pCIWin->SetString(s);
			m_pCIWin->FitToChars(char_width, char_height);

			wxPoint p;
			p.x = x;
			if ((p.x + m_pCIWin->GetWinSize().x) > m_canvas_width)
				p.x = m_canvas_width - m_pCIWin->GetWinSize().x;

			int statsW, statsH;
			stats->GetSize(&statsW, &statsH);
			p.y = m_canvas_height - statsH - 4 - m_pCIWin->GetWinSize().y;

			m_pCIWin->SetPosition(p);
			m_pCIWin->SetBitmap();
			m_pCIWin->Refresh();
			m_pCIWin->Show();
		}
	} else {
		HideChartInfoWindow();
	}
}

void ChartCanvas::HideChartInfoWindow(void)
{
	if (m_pCIWin && m_pCIWin->IsShown())
		m_pCIWin->Hide();
}

void ChartCanvas::PanTimerEvent(wxTimerEvent&)
{
	wxMouseEvent ev(wxEVT_MOTION);
	ev.m_x = mouse_x;
	ev.m_y = mouse_y;
	ev.m_leftDown = mouse_leftisdown;

	wxEvtHandler* evthp = GetEventHandler();

	::wxPostEvent(evthp, ev);
}

void ChartCanvas::EnableAutoPan(bool b_enable)
{
	if (b_enable) {
		m_benable_autopan = true;
	} else {
		m_benable_autopan = false;
		pPanKeyTimer->Stop();
		m_panx = 0;
		m_pany = 0;
		m_panspeed = 0;
	}
}

bool ChartCanvas::CheckEdgePan(int x, int y, bool bdragging)
{
	bool bft = false;
	int pan_margin = m_canvas_width * 5 / 100;
	int pan_timer_set = 200;
	double pan_delta = GetVP().pix_width / 50;
	int pan_x = 0;
	int pan_y = 0;

	if (x > m_canvas_width - pan_margin) {
		bft = true;
		pan_x = pan_delta;
	} else if (x < pan_margin) {
		bft = true;
		pan_x = -pan_delta;
	}

	if (y < pan_margin) {
		bft = true;
		pan_y = -pan_delta;
	} else if (y > m_canvas_height - pan_margin) {
		bft = true;
		pan_y = pan_delta;
	}

	// Of course, if dragging, and the mouse left button is not down, we must stop the event
	// injection
	if (bdragging) {
		wxMouseState state = ::wxGetMouseState();
		if (!state.LeftDown())
			bft = false;
	}

	if ((bft) && !pPanTimer->IsRunning()) {
		PanCanvas(pan_x, pan_y);

		pPanTimer->Start(pan_timer_set, wxTIMER_ONE_SHOT);
		return true;
	}

	// This mouse event must not be due to pan timer event injector
	// Mouse is out of the pan zone, so prevent any orphan event injection
	if ((!bft) && pPanTimer->IsRunning()) {
		pPanTimer->Stop();
	}

	return false;
}

// Look for waypoints at the current position.
// Used to determine what a mouse event should act on.

void ChartCanvas::FindRoutePointsAtCursor(float, bool setBeingEdited)
{
	m_pRoutePointEditTarget = NULL;
	m_pFoundPoint = NULL;

	SelectableItemList SelList
		= pSelect->FindSelectionList(m_cursor_pos, SelectItem::TYPE_ROUTEPOINT);
	for (SelectableItemList::iterator i = SelList.begin(); i != SelList.end(); ++i) {
		SelectItem* pFind = *i;
		RoutePoint* frp = (RoutePoint*)pFind->m_pData1;

		// Get an array of all routes using this point
		m_EditRouteArray = global::OCPN::get().routeman().GetRouteArrayContaining(frp);

		// Use route array to determine actual visibility for the point
		bool brp_viz = false;
		if (!m_EditRouteArray.empty()) {
			for (unsigned int ir = 0; ir < m_EditRouteArray.size(); ++ir) {
				Route* pr = m_EditRouteArray.at(ir);
				if (pr->IsVisible()) {
					brp_viz = true;
					break;
				}
			}
		} else {
			brp_viz = frp->IsVisible(); // isolated point
		}

		if (brp_viz) {
			// Use route array to rubberband all affected routes
			if (!m_EditRouteArray.empty()) {
				// Editing Waypoint as part of route
				m_bRouteEditing = setBeingEdited;
			} else {
				// editing Mark
				m_bMarkEditing = setBeingEdited;
			}

			m_pRoutePointEditTarget = frp;
			m_pFoundPoint = pFind;
			break;
		}
	}
}

void ChartCanvas::MouseTimedEvent(wxTimerEvent&)
{
	if (singleClickEventIsValid)
		MouseEvent(singleClickEvent);
	singleClickEventIsValid = false;
	m_DoubleClickTimer->Stop();
}

void ChartCanvas::mouse_left_center(const wxMouseEvent& event)
{
	m_DoubleClickTimer->Start();
	singleClickEventIsValid = false;

	geo::Position zpos = GetCanvasPixPoint(event.GetX(), event.GetY());

	SelectItem* pFindAIS = pSelectAIS->FindSelection(zpos, SelectItem::TYPE_AISTARGET);

	if (pFindAIS) {
		m_FoundAIS_MMSI = pFindAIS->GetUserData();
		if (g_pAIS->Get_Target_Data_From_MMSI(m_FoundAIS_MMSI)) {
			wxWindow* pwin = wxDynamicCast(this, wxWindow);
			ShowAISTargetQueryDialog(pwin, m_FoundAIS_MMSI);
		}
		return;
	}

	if (m_pRoutePointEditTarget) {
		ShowMarkPropertiesDialog(m_pRoutePointEditTarget);
		return;
	}

	SelectItem* cursorItem;
	cursorItem = pSelect->FindSelection(zpos, SelectItem::TYPE_ROUTESEGMENT);

	if (cursorItem) {
		if (cursorItem->route->IsVisible()) {
			ShowRoutePropertiesDialog(_("Route Properties"), cursorItem->route);
			return;
		}
	}

	cursorItem = pSelect->FindSelection(zpos, SelectItem::TYPE_TRACKSEGMENT);

	if (cursorItem) {
		if (cursorItem->route->IsVisible()) {
			ShowTrackPropertiesDialog(cursorItem->route);
			return;
		}
	}

	// Found no object to act on, so show chart info.

	ShowObjectQueryWindow(event.GetX(), event.GetY(), zpos.lat(), zpos.lon());
}

void ChartCanvas::mouse_route_rubber_band(const wxMouseEvent& event)
{
	if (parent_frame->nRoute_State >= 2) {
		r_rband = event.GetPosition();
		m_bDrawingRoute = true;
		CheckEdgePan(event.GetX(), event.GetY(), event.Dragging());
		Refresh(false);
	}
}

void ChartCanvas::mouse_measure_rubber_band(const wxMouseEvent& event)
{
	if (m_bMeasure_Active && (m_nMeasureState >= 2)) {
		r_rband = event.GetPosition();
		m_bDrawingRoute = true;
		CheckEdgePan(event.GetX(), event.GetY(), event.Dragging());
		Refresh(false);
	}
}

void ChartCanvas::MouseEvent(wxMouseEvent & event)
{
	int x;
	int y;
	int mx;
	int my;

	// Protect from leftUp's coming from event handlers in child
	// windows who return focus to the canvas.
	static bool leftIsDown = false;

	// Protect from very small cursor slips during double click, which produce a
	// single Drag event.
	static bool lastEventWasDrag = false;

	if (event.Dragging() && !lastEventWasDrag) {
		lastEventWasDrag = true;
		return;
	}
	lastEventWasDrag = event.Dragging();

	event.GetPosition(&x, &y);

#ifdef __WXMSW__
	// TODO Test carefully in other platforms, remove ifdef....
	if (event.ButtonDown() && !HasCapture())
		CaptureMouse();
	if (event.ButtonUp() && HasCapture())
		ReleaseMouse();
#endif

	// We start with Double Click processing. The first left click just starts a timer and
	// is remembered, then we actually do something if there is a LeftDClick.
	// If there is, the two single clicks are ignored.

	if (event.LeftDClick() && (cursor_region == CENTER)) {
		mouse_left_center(event);
		return;
	}

	// Capture LeftUp's and time them, unless it already came from the timer.
	if (event.LeftUp() && !singleClickEventIsValid) {

		// Ignore the second LeftUp after the DClick.
		if (m_DoubleClickTimer->IsRunning()) {
			m_DoubleClickTimer->Stop();
			return;
		}

		// Save the event for later running if there is no DClick.
		m_DoubleClickTimer->Start(250, wxTIMER_ONE_SHOT);
		singleClickEvent = event;
		singleClickEventIsValid = true;
		return;
	}

#ifdef __WXMSW__
	// This logic is necessary on MSW to handle the case where
	// a context (right-click) menu is dismissed without action
	// by clicking on the chart surface.
	// We need to avoid an unintentional pan by eating some clicks...
	if (event.LeftDown() || event.LeftUp() || event.Dragging()) {
		if (click_stop > 0) {
			click_stop--;
			return;
		}
	}
#endif

	if (s_ProgDialog)
		return;

	if ((m_bMeasure_Active && (m_nMeasureState >= 2)) || (parent_frame->nRoute_State > 1)
		|| (parent_frame->nRoute_State) > 1) {
		wxPoint p = ClientToScreen(wxPoint(x, y));
		gFrame->SubmergeToolbarIfOverlap(p.x, p.y, 20);
	}

	// Kick off the Rotation control timer
	if (global::OCPN::get().nav().get_data().CourseUp) {
		m_b_rot_hidef = false;
		pRotDefTimer->Start(500, wxTIMER_ONE_SHOT);
	}

	mouse_x = x;
	mouse_y = y;
	mouse_leftisdown = event.LeftIsDown();

	// Retrigger the route leg popup timer
	if (m_pRouteRolloverWin && m_pRouteRolloverWin->IsActive()) {
		// faster response while the rollover is turned on
		m_RolloverPopupTimer.Start(10, wxTIMER_ONE_SHOT);
	} else {
		m_RolloverPopupTimer.Start(m_rollover_popup_timer_msec, wxTIMER_ONE_SHOT);
	}

	// Retrigger the cursor tracking timer
	pCurTrackTimer->Start(m_curtrack_timer_msec, wxTIMER_ONE_SHOT);

	mx = x;
	my = y;
	m_cursor_pos = GetCanvasPixPoint(x, y);

	// Calculate meaningful SelectRadius
	float SelectRadius;
	int sel_rad_pix = 8;
	SelectRadius = sel_rad_pix / (m_true_scale_ppm * 1852 * 60); // Degrees, approximately

#ifndef __WXGTK__
	// Show cursor position on Status Bar, if present
	// except for GTK, under which status bar updates are very slow
	// due to Update() call.
	// In this case, as a workaround, update the status window
	// after an interval timer (pCurTrackTimer) pops, which will happen
	// whenever the mouse has stopped moving for specified interval.
	// See the method OnCursorTrackTimerEvent()
	if (parent_frame->hasStatusBar()) {
		double show_cursor_lon = m_cursor_pos.lon();
		double show_cursor_lat = m_cursor_pos.lat();

		// Check the absolute range of the cursor position
		// There could be a window wherein the chart geoereferencing is not valid....
		if ((fabs(show_cursor_lat) < 90.0) && (fabs(show_cursor_lon) < 360.0)) {
			while (show_cursor_lon < -180.0)
				show_cursor_lon += 360.0;

			while (show_cursor_lon > 180.0)
				show_cursor_lon -= 360.0;

			wxString s1 = _T(" ");
			s1 += toSDMM(1, show_cursor_lat);
			s1 += _T("   ");
			s1 += toSDMM(2, show_cursor_lon);
			parent_frame->SetStatusText(s1, STAT_FIELD_CURSOR_LL);

			double brg;
			double dist;
			const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
			geo::DistanceBearingMercator(m_cursor_pos, nav.pos, &brg, &dist);
			wxString s;
			if (global::OCPN::get().gui().view().ShowMag) // FIXME: code duplication
				s.Printf(wxString("%03d°(M)  ", wxConvUTF8), (int)navigation::GetTrueOrMag(brg));
			else
				s.Printf(wxString("%03d°  ", wxConvUTF8), (int)navigation::GetTrueOrMag(brg));

			s << FormatDistanceAdaptive(dist);
			parent_frame->SetStatusText(s, STAT_FIELD_CURSOR_BRGRNG);
		}
	}
#endif

	// Send the current cursor lat/lon to all PlugIns requesting it
	if (g_pi_manager)
		g_pi_manager->SendCursorLatLonToAllPlugIns(m_cursor_pos.lat(), m_cursor_pos.lon());

	// Check for wheel rotation
	m_mouse_wheel_oneshot = 50; // msec
	// ideally, should be just longer than the time between
	// processing accumulated mouse events from the event queue
	// as would happen during screen redraws.
	int wheel_dir = event.GetWheelRotation();

	if (m_MouseWheelTimer.IsRunning()) {
		if (wheel_dir != m_last_wheel_dir)
			m_MouseWheelTimer.Stop();
		else
			m_MouseWheelTimer.Start(m_mouse_wheel_oneshot, true); // restart timer
	}
	m_last_wheel_dir = wheel_dir;
	if (wheel_dir) {
		if (!m_MouseWheelTimer.IsRunning()) {
			double factor = m_bmouse_key_mod ? 1.1 : 2.0;

			if (global::OCPN::get().gui().view().enable_zoom_to_cursor) {
				// Capture current cursor position, as the zooms below may change it.
				if (wheel_dir > 0)
					ZoomCanvasIn(factor);
				else if (wheel_dir < 0)
					ZoomCanvasOut(factor);

				wxPoint r = GetCanvasPointPix(m_cursor_pos);
				PanCanvas(r.x - x, r.y - y);
				ClearbFollow(); // update the follow flag
			} else {
				if (wheel_dir > 0)
					ZoomCanvasIn(factor);
				else if (wheel_dir < 0)
					ZoomCanvasOut(factor);
			}

			m_MouseWheelTimer.Start(m_mouse_wheel_oneshot, true); // start timer
		}
	}

	mouse_route_rubber_band(event); // Route Creation Rubber Banding
	mouse_measure_rubber_band(event); // Measure Tool Rubber Banding

	// Mouse Clicks

	if (event.LeftDown()) {
		// This really should not be needed, but....
		// on Windows, when using wxAUIManager, sometimes the focus is lost
		// when clicking into another pane, e.g.the AIS target list, and then back to this pane.
		// Oddly, some mouse events are not lost, however.  Like this one....
		SetFocus();

		last_drag.x = mx;
		last_drag.y = my;
		leftIsDown = true;

		if (parent_frame->nRoute_State) {
			// creating route?

			SetCursor(*pCursorPencil);

			m_bRouteEditing = true;

			if (parent_frame->nRoute_State == 1) {
				m_pMouseRoute = new Route();
				pRouteList->push_back(m_pMouseRoute);
				r_rband.x = x;
				r_rband.y = y;
			}

			// Check to see if there is a nearby point which may be reused
			RoutePoint* pMousePoint = NULL;

			// Calculate meaningful SelectRadius
			int nearby_sel_rad_pix = 8;
			double nearby_radius_meters = nearby_sel_rad_pix / m_true_scale_ppm;

			RoutePoint* pNearbyPoint
				= pWayPointMan->GetNearbyWaypoint(m_cursor_pos, nearby_radius_meters);
			if (pNearbyPoint && (pNearbyPoint != m_prev_pMousePoint) && !pNearbyPoint->is_in_track()
				&& !pNearbyPoint->m_bIsInLayer) {
				int dlg_return;
#ifndef __WXOSX__
				dlg_return
					= OCPNMessageBox(this, _("Use nearby waypoint?"), _("OpenCPN Route Create"),
									 (long)wxYES_NO | wxCANCEL | wxYES_DEFAULT);
#else
				dlg_return = wxID_YES;
#endif
				if (dlg_return == wxID_YES) {
					pMousePoint = pNearbyPoint;

					// Using existing waypoint, so nothing to delete for undo.
					if (parent_frame->nRoute_State > 1)
						undo->BeforeUndoableAction(UndoAction::Undo_AppendWaypoint, pMousePoint,
												   UndoAction::Undo_HasParent, NULL);

					// check all other routes to see if this point appears in any other route
					// If it appears in NO other route, then it should e considered an isolated mark
					if (!global::OCPN::get().routeman().FindRouteContainingWaypoint(pMousePoint))
						pMousePoint->m_bKeepXRoute = true;
				}
			}

			if (NULL == pMousePoint) { // need a new point
				pMousePoint = new RoutePoint(m_cursor_pos, _T("diamond"), _T(""));
				pMousePoint->SetNameShown(false);

				pConfig->AddNewWayPoint(pMousePoint, -1); // use auto next num
				pSelect->AddSelectableRoutePoint(m_cursor_pos, pMousePoint);

				if (parent_frame->nRoute_State > 1)
					undo->BeforeUndoableAction(UndoAction::Undo_AppendWaypoint, pMousePoint,
											   UndoAction::Undo_IsOrphanded, NULL);
			}

			if (parent_frame->nRoute_State == 1) {
				// First point in the route.
				m_pMouseRoute->AddPoint(pMousePoint);
			} else {
				if (m_pMouseRoute->m_NextLegGreatCircle) {
					double rhumbBearing;
					double rhumbDist;
					double gcBearing;
					double gcDist;
					geo::DistanceBearingMercator(m_cursor_pos, m_prev_route, &rhumbBearing,
												 &rhumbDist);
					geo::Geodesic::GreatCircleDistBear(m_prev_route, m_cursor_pos, &gcDist,
													   &gcBearing, NULL);
					double gcDistNM = gcDist / 1852.0;

					// Empirically found expression to get reasonable route segments.
					int segmentCount = (3.0 + (rhumbDist - gcDistNM))
									   / pow(rhumbDist - gcDistNM - 1, 0.5);

					wxString msg;
					msg << _("For this leg the Great Circle route is ")
						<< FormatDistanceAdaptive(rhumbDist - gcDistNM)
						<< _(" shorter than rhumbline.\n\n")
						<< _("Would you like include the Great Circle")
						<< _(" routing points for this leg?");

#ifndef __WXOSX__
					int answer = OCPNMessageBox(this, msg, _("OpenCPN Route Create"),
												wxYES_NO | wxNO_DEFAULT);
#else
					int answer = wxID_NO;
#endif

					if (answer == wxID_YES) {
						RoutePoint* gcPoint;
						RoutePoint* prevGcPoint = m_prev_pMousePoint;
						wxRealPoint gcCoord;

						for (int i = 1; i <= segmentCount; i++) {
							double fraction = (double)i * (1.0 / (double)segmentCount);
							geo::Geodesic::GreatCircleTravel(m_prev_route, gcDist * fraction,
															 gcBearing, &gcCoord.x, &gcCoord.y,
															 NULL);

							if (i < segmentCount) {
								geo::Position pos(gcCoord.y, gcCoord.x);
								gcPoint
									= new RoutePoint(pos, _T("xmblue"), _T(""));
								gcPoint->SetNameShown(false);
								pConfig->AddNewWayPoint(gcPoint, -1);
								pSelect->AddSelectableRoutePoint(pos, gcPoint);
							} else {
								gcPoint = pMousePoint; // Last point, previously exsisting!
							}

							m_pMouseRoute->AddPoint(gcPoint);
							pSelect->AddSelectableRouteSegment(prevGcPoint->get_position(),
															   gcPoint->get_position(), prevGcPoint,
															   gcPoint, m_pMouseRoute);
							prevGcPoint = gcPoint;
						}

						undo->CancelUndoableAction(true);

					} else {
						m_pMouseRoute->AddPoint(pMousePoint);
						pSelect->AddSelectableRouteSegment(m_prev_route, m_cursor_pos,
														   m_prev_pMousePoint, pMousePoint,
														   m_pMouseRoute);
						undo->AfterUndoableAction(m_pMouseRoute);
					}
				} else {
					// Ordinary rhumblinesegment.
					m_pMouseRoute->AddPoint(pMousePoint);
					pSelect->AddSelectableRouteSegment(m_prev_route, m_cursor_pos,
													   m_prev_pMousePoint, pMousePoint,
													   m_pMouseRoute);
					undo->AfterUndoableAction(m_pMouseRoute);
				}
			}

			m_prev_route = m_cursor_pos;
			m_prev_pMousePoint = pMousePoint;
			m_pMouseRoute->m_lastMousePointIndex = m_pMouseRoute->GetnPoints();

			parent_frame->nRoute_State++;
			Refresh(false);
		} else if (m_bMeasure_Active && m_nMeasureState) {
			// measure tool?
			SetCursor(*pCursorPencil);
			if (m_nMeasureState == 1) {
				m_pMeasureRoute = new Route();
				pRouteList->push_back(m_pMeasureRoute);
				r_rband.x = x;
				r_rband.y = y;
			}

			RoutePoint* pMousePoint
				= new RoutePoint(m_cursor_pos, wxString(_T("circle")), wxEmptyString);
			pMousePoint->m_bShowName = false;

			m_pMeasureRoute->AddPoint(pMousePoint);

			m_prev_route = m_cursor_pos;
			m_prev_pMousePoint = pMousePoint;
			m_pMeasureRoute->m_lastMousePointIndex = m_pMeasureRoute->GetnPoints();

			m_nMeasureState++;

			Refresh(false);
		} else {
			FindRoutePointsAtCursor(SelectRadius, true); // Not creating Route
		}
	}

	if (event.Dragging()) {
		const global::GUI::View& view = global::OCPN::get().gui().view();

		if (m_bRouteEditing && m_pRoutePointEditTarget) {
			bool DraggingAllowed = true;

			if (NULL == pMarkPropDialog) {
				if (view.WayPointPreventDragging)
					DraggingAllowed = false;
			} else if (!pMarkPropDialog->IsShown() && view.WayPointPreventDragging)
				DraggingAllowed = false;

			if (m_pRoutePointEditTarget && (m_pRoutePointEditTarget->icon_name() == _T("mob")))
				DraggingAllowed = false;

			if (m_pRoutePointEditTarget->m_bIsInLayer)
				DraggingAllowed = false;

			if (DraggingAllowed) {

				if (!undo->InUndoableAction()) {
					undo->BeforeUndoableAction(UndoAction::Undo_MoveWaypoint,
											   m_pRoutePointEditTarget, UndoAction::Undo_NeedsCopy,
											   m_pFoundPoint);
				}

				// Get the update rectangle for the union of the un-edited routes
				wxRect pre_rect;

				if (!m_EditRouteArray.empty()) {
					for (unsigned int ir = 0; ir < m_EditRouteArray.size(); ir++) {
						Route* pr = m_EditRouteArray.at(ir);
						// Need to validate route pointer
						// Route may be gone due to drgging close to ownship with
						// "Delete On Arrival" state set, as in the case of
						// navigating to an isolated waypoint on a temporary route
						if (global::OCPN::get().routeman().IsRouteValid(pr)) {
							wxRect route_rect;
							pr->CalculateDCRect(m_dc_route, route_rect, VPoint);
							pre_rect.Union(route_rect);
						}
					}
				}

				// update the RoutePoint entry
				m_pRoutePointEditTarget->set_position(m_cursor_pos);

				// update the SelectList entry
				m_pFoundPoint->pos1 = m_cursor_pos;

				if (CheckEdgePan(x, y, true)) {
					geo::Position newcursorpos = GetCanvasPixPoint(x, y);

					// update the RoutePoint entry
					m_pRoutePointEditTarget->set_position(newcursorpos);

					// update the SelectList entry
					m_pFoundPoint->pos1 = newcursorpos;
				}

				// Update the MarkProperties Dialog, if currently shown
				if ((NULL != pMarkPropDialog) && (pMarkPropDialog->IsShown())) {
					if (m_pRoutePointEditTarget == pMarkPropDialog->GetRoutePoint())
						pMarkPropDialog->UpdateProperties();
				}

				// Get the update rectangle for the edited route
				wxRect post_rect;

				if (!m_EditRouteArray.empty()) {
					for (unsigned int ir = 0; ir < m_EditRouteArray.size(); ir++) {
						Route* pr = m_EditRouteArray.at(ir);
						if (global::OCPN::get().routeman().IsRouteValid(pr)) {
							wxRect route_rect;
							pr->CalculateDCRect(m_dc_route, route_rect, VPoint);
							post_rect.Union(route_rect);
						}
					}
				}

				// Invalidate the union region
				pre_rect.Union(post_rect);
				RefreshRect(pre_rect, false);
			}
		} else if (m_bMarkEditing && m_pRoutePointEditTarget) {

			bool DraggingAllowed = true;

			if (NULL == pMarkPropDialog) {
				if (view.WayPointPreventDragging)
					DraggingAllowed = false;
			} else if (!pMarkPropDialog->IsShown() && view.WayPointPreventDragging) {
				DraggingAllowed = false;
			}

			if (m_pRoutePointEditTarget && (m_pRoutePointEditTarget->icon_name() == _T("mob")))
				DraggingAllowed = false;

			if (m_pRoutePointEditTarget->m_bIsInLayer)
				DraggingAllowed = false;

			if (DraggingAllowed) {
				if (!undo->InUndoableAction()) {
					undo->BeforeUndoableAction(UndoAction::Undo_MoveWaypoint,
											   m_pRoutePointEditTarget, UndoAction::Undo_NeedsCopy,
											   m_pFoundPoint);
				}

				// The mark may be an anchorwatch
				double lpp1 = 0.0;
				double lpp2 = 0.0;
				double lppmax;

				if (pAnchorWatchPoint1 == m_pRoutePointEditTarget) {
					lpp1 = fabs(GetAnchorWatchRadiusPixels(pAnchorWatchPoint1));
				}
				if (pAnchorWatchPoint2 == m_pRoutePointEditTarget) {
					lpp2 = fabs(GetAnchorWatchRadiusPixels(pAnchorWatchPoint2));
				}
				lppmax = wxMax(lpp1 + 10, lpp2 + 10); // allow for cruft

				// Get the update rectangle for the un-edited mark
				wxRect pre_rect;
				m_pRoutePointEditTarget->CalculateDCRect(m_dc_route, pre_rect);
				if ((lppmax > pre_rect.width / 2) || (lppmax > pre_rect.height / 2))
					pre_rect.Inflate((int)(lppmax - (pre_rect.width / 2)),
									 (int)(lppmax - (pre_rect.height / 2)));

				// update the RoutePoint entry
				m_pRoutePointEditTarget->set_position(m_cursor_pos);

				// update the SelectList entry
				m_pFoundPoint->pos1 = m_cursor_pos;

				// Update the MarkProperties Dialog, if currently shown
				if ((NULL != pMarkPropDialog) && (pMarkPropDialog->IsShown())) {
					if (m_pRoutePointEditTarget == pMarkPropDialog->GetRoutePoint())
						pMarkPropDialog->UpdateProperties(true);
				}

				// Get the update rectangle for the edited mark
				wxRect post_rect;
				m_pRoutePointEditTarget->CalculateDCRect(m_dc_route, post_rect);
				if ((lppmax > post_rect.width / 2) || (lppmax > post_rect.height / 2))
					post_rect.Inflate((int)(lppmax - (post_rect.width / 2)),
									  (int)(lppmax - (post_rect.height / 2)));

				// Invalidate the union region
				pre_rect.Union(post_rect);
				RefreshRect(pre_rect, false);
			}
		} else if (leftIsDown) {
			// must be chart dragging...
			if ((last_drag.x != mx) || (last_drag.y != my)) {
				m_bChartDragging = true;
				PanCanvas(last_drag.x - mx, last_drag.y - my);

				last_drag.x = mx;
				last_drag.y = my;

				Refresh(false);
			}
		}
	}

	if (event.LeftUp()) {
		if (m_bRouteEditing) {
			if (m_pRoutePointEditTarget) {
				pSelect->UpdateSelectableRouteSegments(m_pRoutePointEditTarget);

				if (!m_EditRouteArray.empty()) {
					for (unsigned int ir = 0; ir < m_EditRouteArray.size(); ir++) {
						Route* pr = m_EditRouteArray.at(ir);
						if (global::OCPN::get().routeman().IsRouteValid(pr)) {
							pr->CalculateBBox();
							pr->UpdateSegmentDistances();
							pConfig->UpdateRoute(pr);
						}
					}
				}

				// Update the RouteProperties Dialog, if currently shown
				if ((NULL != pRoutePropDialog) && (pRoutePropDialog->IsShown())) {
					if (!m_EditRouteArray.empty()) {
						for (unsigned int ir = 0; ir < m_EditRouteArray.size(); ir++) {
							Route* pr = m_EditRouteArray.at(ir);
							if (global::OCPN::get().routeman().IsRouteValid(pr)) {
								if (!pr->IsTrack() && pRoutePropDialog->getRoute() == pr) {
									pRoutePropDialog->SetRouteAndUpdate(pr);
									pRoutePropDialog->UpdateProperties();
								} else if ((NULL != pTrackPropDialog)
										   && (pTrackPropDialog->IsShown())
										   && pTrackPropDialog->m_pRoute == pr) {
									pTrackPropDialog->SetTrackAndUpdate(pr);
									pTrackPropDialog->UpdateProperties();
								}
							}
						}
					}
				}

				m_pRoutePointEditTarget->m_bPtIsSelected = false;

				m_EditRouteArray.clear();
				undo->AfterUndoableAction(m_pRoutePointEditTarget);
			}

			m_bRouteEditing = false;
			m_pRoutePointEditTarget = NULL;
			if (!g_FloatingToolbarDialog->IsShown())
				gFrame->SurfaceToolbar();
		} else if (m_bMarkEditing) {
			if (m_pRoutePointEditTarget) {
				pConfig->UpdateWayPoint(m_pRoutePointEditTarget);
				undo->AfterUndoableAction(m_pRoutePointEditTarget);
				m_pRoutePointEditTarget->m_bPtIsSelected = false;
			}
			m_pRoutePointEditTarget = NULL;
			m_bMarkEditing = false;
			if (!g_FloatingToolbarDialog->IsShown())
				gFrame->SurfaceToolbar();
		} else if (leftIsDown) { // left click for chart center
			leftIsDown = false;

			if (!m_bChartDragging && !m_bMeasure_Active) {
				switch (cursor_region) {
					case MID_RIGHT:
						PanCanvas(100, 0);
						break;
					case MID_LEFT:
						PanCanvas(-100, 0);
						break;
					case MID_TOP:
						PanCanvas(0, 100);
						break;
					case MID_BOT:
						PanCanvas(0, -100);
						break;
					case CENTER:
						PanCanvas(x - GetVP().pix_width / 2, y - GetVP().pix_height / 2);
						break;
				}
			} else {
				m_bChartDragging = false;
			}
		}
	}

	if (event.RightDown()) {
		last_drag.x = mx;
		last_drag.y = my;

		if (parent_frame->nRoute_State) { // creating route?
			CanvasPopupMenu(x, y, SelectItem::TYPE_ROUTECREATE);
		} else { // General Right Click
			// Look for selectable objects
#ifdef __WXMAC__
			wxScreenDC sdc;
			ocpnDC dc(sdc);
#else
			wxClientDC cdc(GetParent());
			ocpnDC dc(cdc);
#endif

			SelectItem* pFindAIS;
			SelectItem* pFindRP;
			SelectItem* pFindRouteSeg;
			SelectItem* pFindTrackSeg;
			SelectItem* pFindCurrent = NULL;
			SelectItem* pFindTide = NULL;

			//    Deselect any current objects
			if (m_pSelectedRoute) {
				m_pSelectedRoute->m_bRtIsSelected = false; // Only one selection at a time
				m_pSelectedRoute->DeSelectRoute();
				m_pSelectedRoute->Draw(dc, VPoint);
			}

			if (m_pFoundRoutePoint) {
				m_pFoundRoutePoint->m_bPtIsSelected = false;
				m_pFoundRoutePoint->Draw(dc);
				RefreshRect(m_pFoundRoutePoint->CurrentRect_in_DC);
			}

			// Get all the selectable things at the cursor
			pFindAIS = pSelectAIS->FindSelection(m_cursor_pos, SelectItem::TYPE_AISTARGET);
			pFindRP = pSelect->FindSelection(m_cursor_pos, SelectItem::TYPE_ROUTEPOINT);
			pFindRouteSeg = pSelect->FindSelection(m_cursor_pos, SelectItem::TYPE_ROUTESEGMENT);
			pFindTrackSeg = pSelect->FindSelection(m_cursor_pos, SelectItem::TYPE_TRACKSEGMENT);

			if (m_bShowCurrent)
				pFindCurrent = pSelectTC->FindSelection(m_cursor_pos, SelectItem::TYPE_CURRENTPOINT);

			if (m_bShowTide) // look for tide stations
				pFindTide = pSelectTC->FindSelection(m_cursor_pos, SelectItem::TYPE_TIDEPOINT);

			int seltype = 0;

			// Try for AIS targets first
			if (pFindAIS) {
				m_FoundAIS_MMSI = pFindAIS->GetUserData();

				// Make sure the target data is available
				if (g_pAIS->Get_Target_Data_From_MMSI(m_FoundAIS_MMSI))
					seltype |= SelectItem::TYPE_AISTARGET;
			}

			// Now the various Route Parts

			m_pFoundRoutePoint = NULL;
			if (pFindRP) {
				RoutePoint* pFirstVizPoint = NULL;
				RoutePoint* pFoundActiveRoutePoint = NULL;
				RoutePoint* pFoundVizRoutePoint = NULL;
				Route* pSelectedActiveRoute = NULL;
				Route* pSelectedVizRoute = NULL;

				// There is at least one routepoint, so get the whole list
				SelectableItemList SelList
					= pSelect->FindSelectionList(m_cursor_pos, SelectItem::TYPE_ROUTEPOINT);
				for (SelectableItemList::iterator index = SelList.begin(); index != SelList.end(); ++index) {
					SelectItem* item = *index;

					RoutePoint* prp = (RoutePoint*)item->m_pData1; // candidate

					// Get an array of all routes using this point
					navigation::RouteManager::RouteArray route_array
						= global::OCPN::get().routeman().GetRouteArrayContaining(prp);

					// Use route array (if any) to determine actual visibility for this point
					bool brp_viz = false;
					if (!route_array.empty()) {
						for (navigation::RouteManager::RouteArray::iterator i = route_array.begin();
							 i != route_array.end(); ++i) {
							Route* pr = *i;
							if (pr->IsVisible()) {
								brp_viz = true;
								break;
							}
						}
						if (!brp_viz) // is not visible as part of route
							brp_viz = prp->IsVisible(); //  so treat as isolated point

					} else {
						brp_viz = prp->IsVisible(); // isolated point
					}

					if ((NULL == pFirstVizPoint) && brp_viz)
						pFirstVizPoint = prp;

					// Use route array to choose the appropriate route
					// Give preference to any active route, otherwise select the first visible route
					// in the array for this point
					m_pSelectedRoute = NULL;
					if (!route_array.empty()) {
						for (unsigned int ir = 0; ir < route_array.size(); ++ir) {
							Route* pr = route_array.at(ir);
							if (pr->m_bRtIsActive) {
								pSelectedActiveRoute = pr;
								pFoundActiveRoutePoint = prp;
								break;
							}
						}

						if (NULL == pSelectedVizRoute) {
							for (unsigned int ir = 0; ir < route_array.size(); ++ir) {
								Route* pr = route_array.at(ir);
								if (pr->IsVisible()) {
									pSelectedVizRoute = pr;
									pFoundVizRoutePoint = prp;
									break;
								}
							}
						}
					}
				}

				// Now choose the "best" selections
				if (pFoundActiveRoutePoint) {
					m_pFoundRoutePoint = pFoundActiveRoutePoint;
					m_pSelectedRoute = pSelectedActiveRoute;
				} else if (pFoundVizRoutePoint) {
					m_pFoundRoutePoint = pFoundVizRoutePoint;
					m_pSelectedRoute = pSelectedVizRoute;
				} else {
					// default is first visible point in list
					m_pFoundRoutePoint = pFirstVizPoint;
				}

				if (m_pSelectedRoute) {
					if (m_pSelectedRoute->IsVisible())
						seltype |= SelectItem::TYPE_ROUTEPOINT;
				} else if (m_pFoundRoutePoint) {
					seltype |= SelectItem::TYPE_MARKPOINT;
				}
			}

			// Note here that we use SelectItem::TYPE_ROUTESEGMENT to select tracks as well as routes
			// But call the popup handler with identifier appropriate to the type
			if (pFindRouteSeg) { // there is at least one select item
				SelectableItemList SelList
					= pSelect->FindSelectionList(m_cursor_pos, SelectItem::TYPE_ROUTESEGMENT);

				if (NULL == m_pSelectedRoute) {
					// the case where a segment only is selected
					// Choose the first visible route containing segment in the list
					for (SelectableItemList::iterator index = SelList.begin(); index != SelList.end(); ++index) {
						SelectItem* item = *index;
						if (item->route->IsVisible()) {
							m_pSelectedRoute = item->route;
							break;
						}
					}
				}

				if (m_pSelectedRoute) {
					if (NULL == m_pFoundRoutePoint)
						m_pFoundRoutePoint = (RoutePoint*)pFindRouteSeg->m_pData1;
					m_pFoundRoutePointSecond = pFindRouteSeg->route_point;

					m_pSelectedRoute->m_bRtIsSelected = !(seltype & SelectItem::TYPE_ROUTEPOINT);
					if (m_pSelectedRoute->m_bRtIsSelected)
						m_pSelectedRoute->Draw(dc, GetVP());
					seltype |= SelectItem::TYPE_ROUTESEGMENT;
				}
			}

			if (pFindTrackSeg) {
				m_pSelectedTrack = NULL;
				SelectableItemList SelList
					= pSelect->FindSelectionList(m_cursor_pos, SelectItem::TYPE_TRACKSEGMENT);

				// Choose the first visible track containing segment in the list
				for (SelectableItemList::iterator index = SelList.begin(); index != SelList.end(); ++index) {
					SelectItem* item = *index;
					if (item->route->IsVisible()) {
						m_pSelectedTrack = item->route;
						break;
					}
				}

				if (m_pSelectedTrack)
					seltype |= SelectItem::TYPE_TRACKSEGMENT;
			}

			bool bseltc = false;
			if (pFindCurrent) {
				// There may be multiple current entries at the same point.
				// For example, there often is a current substation (with directions specified)
				// co-located with its master.  We want to select the substation, so that
				// the direction will be properly indicated on the graphic.
				// So, we search the select list looking for IDX_type == 'c' (i.e substation)
				tide::IDX_entry* pIDX_best_candidate;

				SelectItem* pFind = NULL;
				SelectableItemList SelList = pSelectTC->FindSelectionList(
					m_cursor_pos, SelectItem::TYPE_CURRENTPOINT);

				// Default is first entry
				SelectableItemList::iterator index = SelList.begin();
				pFind = *index;
				pIDX_best_candidate = (tide::IDX_entry*)(pFind->m_pData1);

				if (SelList.size() > 1) {
					++index;
					while (index != SelList.end()) {
						pFind = *index;
						tide::IDX_entry* pIDX_candidate = (tide::IDX_entry*)(pFind->m_pData1);
						if (pIDX_candidate->IDX_type == 'c') {
							pIDX_best_candidate = pIDX_candidate;
							break;
						}
						++index;
					}
				} else {
					pFind = SelList.front();
					pIDX_best_candidate = (tide::IDX_entry*)(pFind->m_pData1);
				}

				m_pIDXCandidate = pIDX_best_candidate;

				if (0 == seltype) {
					DrawTCWindow(x, y, pIDX_best_candidate);
					Refresh(false);
					bseltc = true;
				} else
					seltype |= SelectItem::TYPE_CURRENTPOINT;
			} else if (pFindTide) {
				m_pIDXCandidate = (tide::IDX_entry*)pFindTide->m_pData1;

				if (0 == seltype) {
					DrawTCWindow(x, y, m_pIDXCandidate);
					Refresh(false);
					bseltc = true;
				} else
					seltype |= SelectItem::TYPE_TIDEPOINT;
			}

			if (0 == seltype)
				seltype |= SelectItem::TYPE_UNKNOWN;

			if (!bseltc)
				CanvasPopupMenu(x, y, seltype);

			// Seth: Is this refresh needed?
			Refresh(false); // needed for MSW, not GTK  Why??
		}
	}

	// Switch to the appropriate cursor on mouse movement

	wxCursor* ptarget_cursor = pCursorArrow;

	if ((!parent_frame->nRoute_State)
		&& (!m_bMeasure_Active) /*&& ( !m_bCM93MeasureOffset_Active )*/) {

		if (x > xr_margin) {
			ptarget_cursor = pCursorRight;
			cursor_region = MID_RIGHT;
		} else if (x < xl_margin) {
			ptarget_cursor = pCursorLeft;
			cursor_region = MID_LEFT;
		} else if (y > yb_margin) {
			ptarget_cursor = pCursorDown;
			cursor_region = MID_TOP;
		} else if (y < yt_margin) {
			ptarget_cursor = pCursorUp;
			cursor_region = MID_BOT;
		} else {
			ptarget_cursor = pCursorArrow;
			cursor_region = CENTER;
		}
	} else if (m_bMeasure_Active || parent_frame->nRoute_State) // If Measure tool use Pencil Cursor
		ptarget_cursor = pCursorPencil;

	SetCursor(*ptarget_cursor);
}

void ChartCanvas::LostMouseCapture(wxMouseCaptureLostEvent&)
{
	SetCursor(*pCursorArrow);
}

wxString _menuText(wxString name, wxString shortcut)
{
	wxString menutext;
	menutext << name << _T("\t") << shortcut;
	return menutext;
}

void ChartCanvas::CanvasPopupMenu(int x, int y, int seltype)
{
	wxMenu* contextMenu = new wxMenu;
	wxMenu* menuWaypoint = new wxMenu(_("Waypoint"));
	wxMenu* menuRoute = new wxMenu(_("Route"));
	wxMenu* menuTrack = new wxMenu(_("Track"));
	wxMenu* menuAIS = new wxMenu(_("AIS"));

	wxMenu* subMenuChart = new wxMenu;

	wxMenu* menuFocus = contextMenu; // This is the one that will be shown

	popx = x;
	popy = y;

	if (seltype == SelectItem::TYPE_ROUTECREATE) {
#ifndef __WXOSX__
		contextMenu->Append(ID_RC_MENU_FINISH, _menuText(_("End Route"), _T("Esc")));
#else
		contextMenu->Append(ID_RC_MENU_FINISH, _("End Route"));
#endif
	}

	if (!m_pMouseRoute) {
		if (m_bMeasure_Active)
#ifndef __WXOSX__
			contextMenu->Prepend(ID_DEF_MENU_DEACTIVATE_MEASURE,
								 _menuText(_("Measure Off"), _T("Esc")));
#else
			contextMenu->Prepend(ID_DEF_MENU_DEACTIVATE_MEASURE, _("Measure Off"));
#endif
		else
			contextMenu->Prepend(ID_DEF_MENU_ACTIVATE_MEASURE, _menuText(_("Measure"), _T("F4")));
	}

	if (undo->AnythingToUndo()) {
		wxString undoItem;
		undoItem << _("Undo") << _T(" ") << undo->GetNextUndoableAction()->Description();
		contextMenu->Prepend(ID_UNDO, _menuText(undoItem, _T("Ctrl-Z")));
	}

	if (undo->AnythingToRedo()) {
		wxString redoItem;
		redoItem << _("Redo") << _T(" ") << undo->GetNextRedoableAction()->Description();
		contextMenu->Prepend(ID_REDO, _menuText(redoItem, _T("Ctrl-Y")));
	}

	const global::AIS::Data& ais = global::OCPN::get().ais().get_data();
	const global::GUI::View& view = global::OCPN::get().gui().view();
	bool ais_areanotice = false;
	if (g_pAIS && view.ShowAIS && ais.ShowAreaNotices) {

		AIS_Target_Hash* an_sources = g_pAIS->GetAreaNoticeSourcesList();

		float vp_scale = GetVPScale();

		for (AIS_Target_Hash::iterator target = an_sources->begin(); target != an_sources->end();
			 ++target) {
			AIS_Target_Data* target_data = target->second;
			if (!target_data->area_notices.empty()) {
				for (AIS_Area_Notice_Hash::iterator ani = target_data->area_notices.begin();
					 ani != target_data->area_notices.end(); ++ani) {
					Ais8_001_22& area_notice = ani->second;

					geo::BoundingBox bbox;
					geo::Position pos;

					for (Ais8_001_22_SubAreaList::iterator sa = area_notice.sub_areas.begin();
						 sa != area_notice.sub_areas.end(); ++sa) {
						using namespace ais;
						switch (sa->shape) {
							case AIS8_001_22_SHAPE_CIRCLE: {
								pos = geo::Position(sa->latitude, sa->longitude);

								wxPoint target_point = GetCanvasPointPix(geo::Position(sa->latitude, sa->longitude));
								bbox.Expand(target_point.x, target_point.y);
								if (sa->radius_m > 0.0)
									bbox.EnLarge(sa->radius_m * vp_scale);
								break;
							}
							case AIS8_001_22_SHAPE_POLYGON:
							case AIS8_001_22_SHAPE_POLYLINE: {
								for (int i = 0; i < 4; ++i) {
									pos = geo::ll_gc_ll(pos, sa->angles[i], sa->dists_m[i] / 1852.0);
									wxPoint target_point = GetCanvasPointPix(pos);
									bbox.Expand(target_point.x, target_point.y);
								}
							}
						}
					}

					if (bbox.PointInBox(x, y)) {
						ais_areanotice = true;
						break;
					}
				}
			}
		}
	}
	if (!VPoint.is_quilt()) {
		if (parent_frame->GetnChartStack() > 1) {
			contextMenu->Append(ID_DEF_MENU_MAX_DETAIL, _("Max Detail Here"));
			contextMenu->Append(ID_DEF_MENU_SCALE_IN, _menuText(_("Scale In"), _T("F7")));
			contextMenu->Append(ID_DEF_MENU_SCALE_OUT, _menuText(_("Scale Out"), _T("F8")));
		}

		if ((Current_Ch && (Current_Ch->GetChartFamily() == chart::CHART_FAMILY_VECTOR))
			|| ais_areanotice) {
			contextMenu->Append(ID_DEF_MENU_QUERY, _("Object Query..."));
		}

	} else {
		using namespace chart;
		ChartBase* pChartTest = m_pQuilt->GetChartAtPix(wxPoint(x, y));
		if ((pChartTest && (pChartTest->GetChartFamily() == chart::CHART_FAMILY_VECTOR))
			|| ais_areanotice) {
			contextMenu->Append(ID_DEF_MENU_QUERY, _("Object Query..."));
		} else {
			if (parent_frame->GetnChartStack() > 1) {
				contextMenu->Append(ID_DEF_MENU_SCALE_IN, _menuText(_("Scale In"), _T("F7")));
				contextMenu->Append(ID_DEF_MENU_SCALE_OUT, _menuText(_("Scale Out"), _T("F8")));
			}
		}
	}

	contextMenu->Append(ID_DEF_MENU_DROP_WP, _menuText(_("Drop Mark"), _T("Ctrl-M")));

	const global::Navigation& nav = global::OCPN::get().nav();

	if (!nav.gps().valid)
		contextMenu->Append(ID_DEF_MENU_MOVE_BOAT_HERE, _("Move Boat Here"));

	if (!(global::OCPN::get().routeman().GetpActiveRoute()
		  || (seltype & SelectItem::TYPE_MARKPOINT)))
		contextMenu->Append(ID_DEF_MENU_GOTO_HERE, _("Navigate To Here"));

	contextMenu->Append(ID_DEF_MENU_GOTOPOSITION, _("Center View..."));

	if (!nav.get_data().CourseUp)
		contextMenu->Append(ID_DEF_MENU_COGUP, _("Course Up Mode"));
	else {
		if (!VPoint.is_quilt() && Current_Ch && (fabs(Current_Ch->GetChartSkew()) > 0.01)
			&& !view.skew_comp)
			contextMenu->Append(ID_DEF_MENU_NORTHUP, _("Chart Up Mode"));
		else
			contextMenu->Append(ID_DEF_MENU_NORTHUP, _("North Up Mode"));
	}

	Kml* kml = new Kml;
	int pasteBuffer = kml->ParsePasteBuffer();
	if (pasteBuffer != KML_PASTE_INVALID) {
		switch (pasteBuffer) {
			case KML_PASTE_WAYPOINT: {
				contextMenu->Append(ID_PASTE_WAYPOINT, _("Paste Waypoint"));
				break;
			}
			case KML_PASTE_ROUTE: {
				contextMenu->Append(ID_PASTE_ROUTE, _("Paste Route"));
				break;
			}
			case KML_PASTE_TRACK: {
				contextMenu->Append(ID_PASTE_TRACK, _("Paste Track"));
				break;
			}
			case KML_PASTE_ROUTE_TRACK: {
				contextMenu->Append(ID_PASTE_ROUTE, _("Paste Route"));
				contextMenu->Append(ID_PASTE_TRACK, _("Paste Track"));
				break;
			}
		}
	}
	delete kml;

	using namespace chart;
	if (!VPoint.is_quilt() && Current_Ch && (Current_Ch->GetChartType() == CHART_TYPE_CM93COMP)) {
		contextMenu->Append(ID_DEF_MENU_CM93OFFSET_DIALOG, _("CM93 Offset Dialog..."));
	}

	if (VPoint.is_quilt() && (pCurrentStack && pCurrentStack->b_valid)) {
		int dbIndex = m_pQuilt->GetChartdbIndexAtPix(wxPoint(popx, popy));
		if (dbIndex != -1)
			contextMenu->Append(ID_DEF_MENU_QUILTREMOVE, _("Hide This Chart"));
	}

#ifdef __WXMSW__
	// If we dismiss the context menu without action, we need to discard some mouse events....
	// Eat the next 2 button events, which happen as down-up on MSW XP
	click_stop = 2;
#endif

	// ChartGroup SubMenu
	wxMenuItem* subItemChart = contextMenu->AppendSubMenu(subMenuChart, _("Chart Groups"));
	if (g_pGroupArray->size()) {
		subMenuChart->AppendRadioItem(ID_DEF_MENU_GROUPBASE, _("All Active Charts"));

		for (unsigned int i = 0; i < g_pGroupArray->size(); i++) {
			subMenuChart->AppendRadioItem(ID_DEF_MENU_GROUPBASE + i + 1,
										  g_pGroupArray->at(i)->m_group_name);
			Connect(ID_DEF_MENU_GROUPBASE + i + 1, wxEVT_COMMAND_MENU_SELECTED,
					(wxObjectEventFunction)(wxEventFunction) & ChartCanvas::PopupMenuHandler);
		}

		subMenuChart->Check(ID_DEF_MENU_GROUPBASE + global::OCPN::get().gui().view().GroupIndex,
							true);
	}

	// Add PlugIn Context Menu items
	ArrayOfPlugInMenuItems item_array = g_pi_manager->GetPluginContextMenuItemArray();

	for (unsigned int i = 0; i < item_array.size(); i++) {
		PlugInMenuItemContainer* pimis = item_array.Item(i);
		{
			if (pimis->b_viz) {
				wxMenuItem* pmi
					= new wxMenuItem(contextMenu, pimis->id, pimis->pmenu_item->GetLabel(),
									 pimis->pmenu_item->GetHelp(), pimis->pmenu_item->GetKind(),
									 pimis->pmenu_item->GetSubMenu());
				contextMenu->Append(pmi);
				contextMenu->Enable(pimis->id, !pimis->b_grey);

				Connect(pimis->id, wxEVT_COMMAND_MENU_SELECTED,
						(wxObjectEventFunction)(wxEventFunction) & ChartCanvas::PopupMenuHandler);
			}
		}
	}

	// This is the default context menu
	menuFocus = contextMenu;

	if (g_pAIS) {
		contextMenu->Append(ID_DEF_MENU_AISTARGETLIST, _("AIS Target List..."));

		if (seltype & SelectItem::TYPE_AISTARGET) {
			using namespace ais;

			menuAIS->Append(ID_DEF_MENU_AIS_QUERY, _("Target Query..."));
			AIS_Target_Data* myptarget = g_pAIS->Get_Target_Data_From_MMSI(m_FoundAIS_MMSI);
			if (myptarget && myptarget->bCPA_Valid && (myptarget->n_alarm_state != AIS_ALARM_SET)) {
				if (myptarget->b_show_AIS_CPA)
					menuAIS->Append(ID_DEF_MENU_AIS_CPA, _("Hide Target CPA"));
				else
					menuAIS->Append(ID_DEF_MENU_AIS_CPA, _("Show Target CPA"));
			}
			menuAIS->Append(ID_DEF_MENU_AISTARGETLIST, _("Target List..."));
			if (myptarget && myptarget->b_show_track)
				menuAIS->Append(ID_DEF_MENU_AISSHOWTRACK, _("Hide Target Track"));
			else
				menuAIS->Append(ID_DEF_MENU_AISSHOWTRACK, _("Show Target Track"));
			menuFocus = menuAIS;
		}
	}

	if (seltype & SelectItem::TYPE_ROUTESEGMENT) {
		bool blay = false;
		if (m_pSelectedRoute && m_pSelectedRoute->m_bIsInLayer)
			blay = true;

		if (blay) {
			delete menuRoute;
			menuRoute = new wxMenu(_("Layer Route"));
			menuRoute->Append(ID_RT_MENU_PROPERTIES, _("Properties..."));
			if (m_pSelectedRoute) {
				if (m_pSelectedRoute->IsActive()) {
					int indexActive
						= m_pSelectedRoute->GetIndexOf(m_pSelectedRoute->m_pRouteActivePoint);
					if ((indexActive + 1) <= m_pSelectedRoute->GetnPoints()) {
						menuRoute->Append(ID_RT_MENU_ACTNXTPOINT, _("Activate Next Waypoint"));
					}
					menuRoute->Append(ID_RT_MENU_DEACTIVATE, _("Deactivate"));
				} else {
					menuRoute->Append(ID_RT_MENU_ACTIVATE, _("Activate"));
				}
			}
		} else {
			menuRoute->Append(ID_RT_MENU_PROPERTIES, _("Properties..."));
			if (m_pSelectedRoute) {
				if (m_pSelectedRoute->IsActive()) {
					int indexActive
						= m_pSelectedRoute->GetIndexOf(m_pSelectedRoute->m_pRouteActivePoint);
					if ((indexActive + 1) <= m_pSelectedRoute->GetnPoints()) {
						menuRoute->Append(ID_RT_MENU_ACTNXTPOINT, _("Activate Next Waypoint"));
					}
					menuRoute->Append(ID_RT_MENU_DEACTIVATE, _("Deactivate"));
				} else {
					menuRoute->Append(ID_RT_MENU_ACTIVATE, _("Activate"));
				}
			}
			menuRoute->Append(ID_RT_MENU_INSERT, _("Insert Waypoint"));
			menuRoute->Append(ID_RT_MENU_APPEND, _("Append Waypoint"));
			menuRoute->Append(ID_RT_MENU_COPY, _("Copy as KML..."));
			menuRoute->Append(ID_RT_MENU_DELETE, _("Delete..."));
			menuRoute->Append(ID_RT_MENU_REVERSE, _("Reverse..."));
			wxString port = FindValidUploadPort();
			m_active_upload_port = port;
			wxString item = _("Send to GPS");
			if (!port.IsEmpty()) {
				item.Append(_T(" ( "));
				item.Append(port);
				item.Append(_T(" )"));
			}
			menuRoute->Append(ID_RT_MENU_SENDTOGPS, item);
		}
		// Set this menu as the "focused context menu"
		menuFocus = menuRoute;
	}

	if (seltype & SelectItem::TYPE_TRACKSEGMENT) {
		bool blay = false;
		if (m_pSelectedTrack && m_pSelectedTrack->m_bIsInLayer)
			blay = true;

		if (blay) {
			delete menuTrack;
			menuTrack = new wxMenu(_("Layer Track"));
			menuTrack->Append(ID_TK_MENU_PROPERTIES, _("Properties..."));
		} else {
			menuTrack->Append(ID_TK_MENU_PROPERTIES, _("Properties..."));
			menuTrack->Append(ID_TK_MENU_COPY, _("Copy As KML"));
			menuTrack->Append(ID_TK_MENU_DELETE, _("Delete..."));
		}

		// Set this menu as the "focused context menu"
		menuFocus = menuTrack;
	}

	if (seltype & SelectItem::TYPE_ROUTEPOINT) {
		bool blay = false;
		if (m_pFoundRoutePoint && m_pFoundRoutePoint->m_bIsInLayer)
			blay = true;

		if (blay) {
			delete menuWaypoint;
			menuWaypoint = new wxMenu(_("Layer Routepoint"));
			menuWaypoint->Append(ID_WP_MENU_PROPERTIES, _("Properties..."));

			if (m_pSelectedRoute && m_pSelectedRoute->IsActive())
				menuWaypoint->Append(ID_RT_MENU_ACTPOINT, _("Activate"));
		} else {
			menuWaypoint->Append(ID_WP_MENU_PROPERTIES, _("Properties..."));
			if (m_pSelectedRoute && m_pSelectedRoute->IsActive()) {
				if (m_pSelectedRoute->m_pRouteActivePoint != m_pFoundRoutePoint)
					menuWaypoint->Append(ID_RT_MENU_ACTPOINT, _("Activate"));
			}

			if (m_pSelectedRoute && m_pSelectedRoute->IsActive()) {
				if (m_pSelectedRoute->m_pRouteActivePoint == m_pFoundRoutePoint) {
					int indexActive
						= m_pSelectedRoute->GetIndexOf(m_pSelectedRoute->m_pRouteActivePoint);
					if ((indexActive + 1) <= m_pSelectedRoute->GetnPoints())
						menuWaypoint->Append(ID_RT_MENU_ACTNXTPOINT, _("Activate Next Waypoint"));
				}
			}
			if (m_pSelectedRoute->GetnPoints() > 2)
				menuWaypoint->Append(ID_RT_MENU_REMPOINT, _("Remove from Route"));

			menuWaypoint->Append(ID_WPT_MENU_COPY, _("Copy as KML"));

			if (m_pFoundRoutePoint->icon_name() != _T("mob"))
				menuWaypoint->Append(ID_RT_MENU_DELPOINT, _("Delete"));

			wxString port = FindValidUploadPort();
			m_active_upload_port = port;
			wxString item = _("Send to GPS");
			if (!port.IsEmpty()) {
				item.Append(_T(" ( "));
				item.Append(port);
				item.Append(_T(" )"));
			}
			menuWaypoint->Append(ID_WPT_MENU_SENDTOGPS, item);
		}
		// Set this menu as the "focused context menu"
		menuFocus = menuWaypoint;
	}

	if (seltype & SelectItem::TYPE_MARKPOINT) {
		bool blay = false;
		if (m_pFoundRoutePoint && m_pFoundRoutePoint->m_bIsInLayer)
			blay = true;

		if (blay) {
			delete menuWaypoint;
			menuWaypoint = new wxMenu(_("Layer Waypoint"));
			menuWaypoint->Append(ID_WP_MENU_PROPERTIES, _("Properties..."));
		} else {
			menuWaypoint->Append(ID_WP_MENU_PROPERTIES, _("Properties..."));

			if (!global::OCPN::get().routeman().GetpActiveRoute())
				menuWaypoint->Append(ID_WP_MENU_GOTO, _("Navigate To This"));

			menuWaypoint->Append(ID_WPT_MENU_COPY, _("Copy as KML"));

			if (m_pFoundRoutePoint->icon_name() != _T("mob"))
				menuWaypoint->Append(ID_WP_MENU_DELPOINT, _("Delete"));

			wxString port = FindValidUploadPort();
			m_active_upload_port = port;
			wxString item = _("Send to GPS");
			if (!port.IsEmpty()) {
				item.Append(_T(" ( "));
				item.Append(port);
				item.Append(_T(" )"));
			}
			menuWaypoint->Append(ID_WPT_MENU_SENDTOGPS, item);

			if ((m_pFoundRoutePoint == pAnchorWatchPoint1)
				|| (m_pFoundRoutePoint == pAnchorWatchPoint2)) {
				menuWaypoint->Append(ID_WP_MENU_CLEAR_ANCHORWATCH, _("Clear Anchor Watch"));
			} else {
				if (!m_pFoundRoutePoint->m_bIsInLayer
					&& ((NULL == pAnchorWatchPoint1) || (NULL == pAnchorWatchPoint2))) {
					const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();

					double dist;
					double brg;
					geo::DistanceBearingMercator(m_pFoundRoutePoint->get_position(), nav.pos, &brg,
												 &dist);
					if (dist * 1852.0 <= global::OCPN::get().nav().anchor().AWMax)
						menuWaypoint->Append(ID_WP_MENU_SET_ANCHORWATCH, _("Set Anchor Watch"));
				}
			}
		}

		// Set this menu as the "focused context menu"
		menuFocus = menuWaypoint;
	}

	if (!subMenuChart->GetMenuItemCount())
		contextMenu->Destroy(subItemChart);

	// Add the Tide/Current selections if the item was not activated by shortcut in right-click
	// handlers
	bool bsep = false;
	if (seltype & SelectItem::TYPE_TIDEPOINT) {
		menuFocus->AppendSeparator();
		bsep = true;
		menuFocus->Append(ID_DEF_MENU_TIDEINFO, _("Show Tide Information"));
	}

	if (seltype & SelectItem::TYPE_CURRENTPOINT) {
		if (!bsep)
			menuFocus->AppendSeparator();
		menuFocus->Append(ID_DEF_MENU_CURRENTINFO, _("Show Current Information"));
	}

	// Invoke the correct focused drop-down menu
	PopupMenu(menuFocus, x, y);

	// Cleanup
	if ((m_pSelectedRoute)) {
		m_pSelectedRoute->m_bRtIsSelected = false;
	}

	m_pSelectedRoute = NULL;

	if (m_pFoundRoutePoint) {
		m_pFoundRoutePoint->m_bPtIsSelected = false;
	}
	m_pFoundRoutePoint = NULL;

	m_pFoundRoutePointSecond = NULL;

	delete contextMenu;
	delete menuAIS;
	delete menuRoute;
	delete menuTrack;
	delete menuWaypoint;
}

void ChartCanvas::ShowObjectQueryWindow(int x, int y, float zlat, float zlon)
{
	using namespace chart;

	ChartPlugInWrapper* target_plugin_chart = NULL;
	s57chart* Chs57 = NULL;

	ChartBase* target_chart = GetChartAtCursor();
	if (target_chart) {
		if (target_chart->GetChartType() == CHART_TYPE_PLUGIN)
			target_plugin_chart = dynamic_cast<ChartPlugInWrapper*>(target_chart);
		else
			Chs57 = dynamic_cast<s57chart*>(target_chart);
	}
	std::vector<Ais8_001_22*> area_notices;

	const global::AIS::Data& ais = global::OCPN::get().ais().get_data();
	const global::GUI::View& view = global::OCPN::get().gui().view();
	if (g_pAIS && view.ShowAIS && ais.ShowAreaNotices) {
		AIS_Target_Hash* an_sources = g_pAIS->GetAreaNoticeSourcesList();

		float vp_scale = GetVPScale();

		for (AIS_Target_Hash::iterator target = an_sources->begin(); target != an_sources->end();
			 ++target) {
			AIS_Target_Data* target_data = target->second;
			if (!target_data->area_notices.empty()) {
				for (AIS_Area_Notice_Hash::iterator ani = target_data->area_notices.begin();
					 ani != target_data->area_notices.end(); ++ani) {
					Ais8_001_22& area_notice = ani->second;

					geo::BoundingBox bbox;
					geo::Position pos;

					for (Ais8_001_22_SubAreaList::iterator sa = area_notice.sub_areas.begin();
						 sa != area_notice.sub_areas.end(); ++sa) {
						using namespace ais;
						switch (sa->shape) {
							case AIS8_001_22_SHAPE_CIRCLE: {
								pos = geo::Position(sa->latitude, sa->longitude);

								wxPoint target_point
									= GetCanvasPointPix(geo::Position(sa->latitude, sa->longitude));
								bbox.Expand(target_point.x, target_point.y);
								if (sa->radius_m > 0.0)
									bbox.EnLarge(sa->radius_m * vp_scale);
								break;
							}
							case AIS8_001_22_SHAPE_POLYGON:
							case AIS8_001_22_SHAPE_POLYLINE: {
								for (int i = 0; i < 4; ++i) {
									pos = geo::ll_gc_ll(pos, sa->angles[i], sa->dists_m[i] / 1852.0);
									wxPoint target_point = GetCanvasPointPix(pos);
									bbox.Expand(target_point.x, target_point.y);
								}
							}
						}
					}

					if (bbox.PointInBox(x, y)) {
						area_notices.push_back(&area_notice);
					}
				}
			}
		}
	}

	if (target_plugin_chart || Chs57 || !area_notices.empty()) {
		// Go get the array of all objects at the cursor lat/lon
		int sel_rad_pix = 5;
		float SelectRadius = sel_rad_pix / (GetVP().view_scale() * 1852.0 * 60.0);

		// Make sure we always get the lights from an object, even if we are currently
		// not displaying lights on the chart.

		SetCursor(wxCURSOR_WAIT);
		bool lightsVis = gFrame->ToggleLights(false);
		if (!lightsVis)
			gFrame->ToggleLights(true, true);
		ListOfObjRazRules* rule_list = NULL;
		ListOfPI_S57Obj* pi_rule_list = NULL;
		if (Chs57)
			rule_list = Chs57->GetObjRuleListAtLatLon(zlat, zlon, SelectRadius, GetVP());
		else if (target_plugin_chart)
			pi_rule_list = g_pi_manager->GetPlugInObjRuleListAtLatLon(target_plugin_chart, zlat,
																	  zlon, SelectRadius, GetVP());

		ChartBase* overlay_chart = GetOverlayChartAtCursor();
		s57chart* CHs57_Overlay = dynamic_cast<s57chart*>(overlay_chart);

		ListOfObjRazRules* overlay_rule_list = NULL;
		if (CHs57_Overlay) {
			overlay_rule_list
				= CHs57_Overlay->GetObjRuleListAtLatLon(zlat, zlon, SelectRadius, GetVP());
		}

		if (!lightsVis)
			gFrame->ToggleLights(true, true);

		wxString objText;
		wxFont* dFont = FontMgr::Get().GetFont(_("ObjectQuery"), 12);
		wxString face = dFont->GetFaceName();

		if (NULL == g_pObjectQueryDialog) {
			g_pObjectQueryDialog = new S57QueryDialog();

			g_pObjectQueryDialog->Create(this, -1, _("Object Query"), wxDefaultPosition,
										 global::OCPN::get().gui().s57dialog().size);
			g_pObjectQueryDialog->Centre();
		}

		wxColor bg = g_pObjectQueryDialog->GetBackgroundColour();
		wxColor fg = FontMgr::Get().GetFontColor(_("ObjectQuery"));

		objText.Printf(_T("<html><body bgcolor=#%02x%02x%02x><font color=#%02x%02x%02x face="),
					   bg.Red(), bg.Blue(), bg.Green(), fg.Red(), fg.Blue(), fg.Green());
		objText += _T("\"");
		objText += face;
		objText += _T("\">");

		if (overlay_rule_list && CHs57_Overlay) {
			objText << CHs57_Overlay->CreateObjDescriptions(overlay_rule_list);
			objText << _T("<hr noshade>");
		}

		for (std::vector<Ais8_001_22*>::iterator an = area_notices.begin();
			 an != area_notices.end(); ++an) {
			using namespace ais;
			objText << _T( "<b>AIS Area Notice:</b> " );
			objText << ais8_001_22_notice_names[(*an)->notice_type];
			for (std::vector<Ais8_001_22_SubArea>::iterator sa = (*an)->sub_areas.begin();
				 sa != (*an)->sub_areas.end(); ++sa)
				if (!sa->text.empty())
					objText << sa->text;
			objText << _T( "<br>expires: " ) << (*an)->expiry_time.Format();
			objText << _T( "<hr noshade>" );
		}

		if (Chs57)
			objText << Chs57->CreateObjDescriptions(rule_list);
		else if (target_plugin_chart)
			objText << g_pi_manager->CreateObjDescriptions(target_plugin_chart, pi_rule_list);

		objText << _T("</font></body></html>");
		g_pObjectQueryDialog->SetHTMLPage(objText);
		g_pObjectQueryDialog->Show();

		delete rule_list;
		delete overlay_rule_list;

		if (pi_rule_list)
			pi_rule_list->Clear();
		delete pi_rule_list;

		SetCursor(wxCURSOR_ARROW);
	}
}

void ChartCanvas::RemovePointFromRoute(RoutePoint* point, Route* route)
{
	// Rebuild the route selectables
	pSelect->DeleteAllSelectableRoutePoints(route);
	pSelect->DeleteAllSelectableRouteSegments(route);

	route->RemovePoint(point);

	// Check for 1 point routes. If we are creating a route, this is an undo, so keep the 1 point.
	if ((route->GetnPoints() <= 1) && (parent_frame->nRoute_State == 0)) {
		pConfig->DeleteConfigRoute(route);
		global::OCPN::get().routeman().DeleteRoute(route);
		route = NULL;
	}
	// Add this point back into the selectables
	pSelect->AddSelectableRoutePoint(point->get_position(), point);

	if (pRoutePropDialog && (pRoutePropDialog->IsShown())) {
		pRoutePropDialog->SetRouteAndUpdate(route);
		pRoutePropDialog->UpdateProperties();
	}
}

void ChartCanvas::ShowMarkPropertiesDialog(RoutePoint* markPoint)
{
	if (NULL == pMarkPropDialog) // There is one global instance of the MarkProp Dialog
		pMarkPropDialog = new MarkInfoImpl(this);

	pMarkPropDialog->SetRoutePoint(markPoint);
	pMarkPropDialog->UpdateProperties();
	if (markPoint->m_bIsInLayer) {
		wxString caption(_("Waypoint Properties, Layer: "));
		caption.Append(GetLayerName(markPoint->get_layer_ID()));
		pMarkPropDialog->SetDialogTitle(caption);
	} else {
		pMarkPropDialog->SetDialogTitle(_("Waypoint Properties"));
	}

	pMarkPropDialog->Show();
	pMarkPropDialog->InitialFocus();
}

void ChartCanvas::ShowRoutePropertiesDialog(wxString title, Route* selected)
{
	if (NULL == pRoutePropDialog) // There is one global instance of the RouteProp Dialog
		pRoutePropDialog = new RouteProp(this);

	pRoutePropDialog->SetRouteAndUpdate(selected);
	pRoutePropDialog->UpdateProperties();
	if (!selected->m_bIsInLayer)
		pRoutePropDialog->SetDialogTitle(title);
	else {
		wxString caption(title << _T(", Layer: "));
		caption.Append(GetLayerName(selected->m_LayerID));
		pRoutePropDialog->SetDialogTitle(caption);
	}

	pRoutePropDialog->Show();

	Refresh(false);
}

void ChartCanvas::ShowTrackPropertiesDialog(Route* selected)
{
	if (NULL == pTrackPropDialog) // There is one global instance of the RouteProp Dialog
		pTrackPropDialog = new TrackPropDlg(this);

	pTrackPropDialog->SetTrackAndUpdate(selected);
	pTrackPropDialog->UpdateProperties();

	pTrackPropDialog->Show();

	Refresh(false);
}

void pupHandler_PasteWaypoint()
{
	Kml* kml = new Kml();
	::wxBeginBusyCursor();

	kml->ParsePasteBuffer();
	RoutePoint* pasted = kml->GetParsedRoutePoint();

	int nearby_sel_rad_pix = 8;
	double nearby_radius_meters = nearby_sel_rad_pix / cc1->GetCanvasTrueScale();

	RoutePoint* nearPoint
		= pWayPointMan->GetNearbyWaypoint(pasted->get_position(), nearby_radius_meters);

	int answer = wxID_NO;
	if (nearPoint && !nearPoint->is_in_track() && !nearPoint->m_bIsInLayer) {
		wxString msg;
		msg << _("There is an existing waypoint at the same location as the one you are pasting. ")
			<< _("Would you like to merge the pasted data with it?\n\n");
		msg << _("Answering 'No' will create a new waypoint at the same location.");
		answer = OCPNMessageBox(cc1, msg, _("Merge waypoint?"),
								(long)wxYES_NO | wxCANCEL | wxNO_DEFAULT);
	}

	if (answer == wxID_YES) {
		nearPoint->SetName(pasted->GetName());
		nearPoint->set_description(pasted->get_description());
		if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
			pRouteManagerDialog->UpdateWptListCtrl();
	}

	if (answer == wxID_NO) {
		RoutePoint* newPoint = new RoutePoint(*pasted);
		newPoint->m_bIsolatedMark = true;
		pSelect->AddSelectableRoutePoint(newPoint->get_position(), newPoint);
		pConfig->AddNewWayPoint(newPoint, -1);
		pWayPointMan->push_back(newPoint);
		if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
			pRouteManagerDialog->UpdateWptListCtrl();
	}

	cc1->Refresh(false);
	delete kml;
	::wxEndBusyCursor();
}

void pupHandler_PasteRoute()
{
	Kml* kml = new Kml();
	::wxBeginBusyCursor();

	kml->ParsePasteBuffer();
	Route* pasted = kml->GetParsedRoute();
	if (!pasted)
		return;

	int nearby_sel_rad_pix = 8;
	double nearby_radius_meters = nearby_sel_rad_pix / cc1->GetCanvasTrueScale();

	RoutePoint* curPoint;
	RoutePoint* nearPoint;
	RoutePoint* prevPoint = NULL;

	bool mergepoints = false;
	bool createNewRoute = true;
	int existingWaypointCounter = 0;

	for (int i = 1; i <= pasted->GetnPoints(); i++) {
		curPoint = pasted->GetPoint(i); // NB! n starts at 1 !
		nearPoint = pWayPointMan->GetNearbyWaypoint(curPoint->get_position(), nearby_radius_meters);
		if (nearPoint) {
			mergepoints = true;
			existingWaypointCounter++;
			// Small hack here to avoid both extending RoutePoint and repeating all the
			// GetNearbyWaypoint
			// calculations. Use existin data field in RoutePoint as temporary storage.
			curPoint->m_bPtIsSelected = true;
		}
	}

	int answer = wxID_NO;
	if (mergepoints) {
		wxString msg;
		msg << _("There are existing waypoints at the same location as some of the ones you are ")
			<< _("pasting. Would you like to just merge the pasted data into them?\n\n");
		msg << _("Answering 'No' will create all new waypoints for this route.");
		answer = OCPNMessageBox(cc1, msg, _("Merge waypoints?"),
								(long)wxYES_NO | wxCANCEL | wxYES_DEFAULT);

		if (answer == wxID_CANCEL) {
			delete kml;
			return;
		}
	}

	// If all waypoints exist since before, and a route with the same name, we don't create a new
	// route.
	if ((mergepoints && answer == wxID_YES) && (existingWaypointCounter == pasted->GetnPoints())) {
		for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
			Route* route = *i;
			if (pasted->get_name() == route->get_name()) {
				createNewRoute = false;
				break;
			}
		}
	}

	Route* newRoute = NULL;
	RoutePoint* newPoint;

	if (createNewRoute) {
		newRoute = new Route();
		newRoute->set_name(pasted->get_name());
	}

	for (int i = 1; i <= pasted->GetnPoints(); i++) {
		curPoint = pasted->GetPoint(i);
		if (answer == wxID_YES && curPoint->m_bPtIsSelected) {
			curPoint->m_bPtIsSelected = false;
			newPoint
				= pWayPointMan->GetNearbyWaypoint(curPoint->get_position(), nearby_radius_meters);
			newPoint->SetName(curPoint->GetName());
			newPoint->set_description(curPoint->get_description());

			if (createNewRoute)
				newRoute->AddPoint(newPoint);
		} else {
			curPoint->m_bPtIsSelected = false;

			newPoint = new RoutePoint(*curPoint);
			newPoint->m_bIsolatedMark = false;
			newPoint->set_icon_name(_T("circle"));
			newPoint->m_bIsVisible = true;
			newPoint->m_bShowName = false;
			newPoint->m_bKeepXRoute = false;

			newRoute->AddPoint(newPoint);
			pSelect->AddSelectableRoutePoint(newPoint->get_position(), newPoint);
			pConfig->AddNewWayPoint(newPoint, -1);
			pWayPointMan->push_back(newPoint);
		}
		if (i > 1 && createNewRoute)
			pSelect->AddSelectableRouteSegment(prevPoint->get_position(), curPoint->get_position(),
											   prevPoint, newPoint, newRoute);
		prevPoint = newPoint;
	}

	if (createNewRoute) {
		pRouteList->push_back(newRoute);
		pConfig->AddNewRoute(newRoute, -1); // use auto next num
		newRoute->RebuildGUIDList(); // ensure the GUID list is intact and good

		if (pRoutePropDialog && (pRoutePropDialog->IsShown())) {
			pRoutePropDialog->SetRouteAndUpdate(newRoute);
			pRoutePropDialog->UpdateProperties();
		}

		if (pRouteManagerDialog && pRouteManagerDialog->IsShown()) {
			pRouteManagerDialog->UpdateRouteListCtrl();
			pRouteManagerDialog->UpdateWptListCtrl();
		}
		cc1->Refresh(false);
	}

	delete kml;
	::wxEndBusyCursor();
}

void pupHandler_PasteTrack()
{
	Kml* kml = new Kml();
	::wxBeginBusyCursor();

	kml->ParsePasteBuffer();
	Track* pasted = kml->GetParsedTrack();
	if (!pasted)
		return;

	Track* newTrack = new Track();
	RoutePoint* newPoint;
	RoutePoint* prevPoint = NULL;

	newTrack->set_name(pasted->get_name());

	for (int i = 1; i <= pasted->GetnPoints(); i++) {
		RoutePoint* curPoint = pasted->GetPoint(i);

		newPoint = new RoutePoint(*curPoint);
		newPoint->m_bShowName = false;
		newPoint->m_bIsVisible = false;
		newPoint->m_GPXTrkSegNo = 1;

		newPoint->SetCreateTime(curPoint->GetCreateTime());

		newTrack->AddPoint(newPoint);

		// This is a hack, need to undo the action of Route::AddPoint
		newPoint->m_bIsInRoute = false;
		newPoint->m_bIsInTrack = true;

		if (prevPoint)
			pSelect->AddSelectableTrackSegment(prevPoint->get_position(), newPoint->get_position(),
											   prevPoint, newPoint, newTrack);

		prevPoint = newPoint;
	}

	pRouteList->push_back(newTrack);
	pConfig->AddNewRoute(newTrack, -1); // use auto next num
	newTrack->RebuildGUIDList(); // ensure the GUID list is intact and good

	cc1->Refresh(false);
	delete kml;
	::wxEndBusyCursor();
}

void ChartCanvas::PopupMenuHandler(wxCommandEvent& event)
{
	RoutePoint* pLast;

	wxPoint r;
	geo::Position zpos = GetCanvasPixPoint(popx, popy);

	using namespace chart;

	switch (event.GetId()) {
		case ID_DEF_MENU_MAX_DETAIL:
			global::OCPN::get().nav().set_view_point(zpos);
			ClearbFollow();
			parent_frame->DoChartUpdate();
			parent_frame->SelectChartFromStack(0, false, CHART_TYPE_DONTCARE, chart::CHART_FAMILY_RASTER);
			break;

		case ID_DEF_MENU_SCALE_IN:
			parent_frame->DoStackDown();
			break;

		case ID_DEF_MENU_SCALE_OUT:
			parent_frame->DoStackUp();
			break;

		case ID_UNDO:
			undo->UndoLastAction();
			Refresh(false);
			break;

		case ID_REDO:
			undo->RedoNextAction();
			Refresh(false);
			break;

		case ID_DEF_MENU_MOVE_BOAT_HERE:
			global::OCPN::get().nav().set_position(zpos);
			break;

		case ID_DEF_MENU_GOTO_HERE: {
			const global::GUI::View& view = global::OCPN::get().gui().view();
			const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
			RoutePoint* pWP_dest = new RoutePoint(zpos, view.default_wp_icon, wxEmptyString);
			pSelect->AddSelectableRoutePoint(zpos, pWP_dest);

			RoutePoint* pWP_src = new RoutePoint(nav.pos, view.default_wp_icon, wxEmptyString);
			pSelect->AddSelectableRoutePoint(nav.pos, pWP_src);

			Route* temp_route = new Route();
			pRouteList->push_back(temp_route);

			temp_route->AddPoint(pWP_src);
			temp_route->AddPoint(pWP_dest);

			pSelect->AddSelectableRouteSegment(nav.pos, zpos, pWP_src, pWP_dest, temp_route);

			temp_route->set_name(_("Temporary GOTO Route"));
			temp_route->set_startString(_("Here"));

			temp_route->set_endString(_("There"));

			temp_route->m_bDeleteOnArrival = true;

			navigation::RouteManager& routemanager = global::OCPN::get().routeman();
			if (routemanager.GetpActiveRoute())
				routemanager.DeactivateRoute();

			routemanager.ActivateRoute(temp_route, pWP_dest);
		} break;

		case ID_DEF_MENU_DROP_WP: {
			const global::GUI::View& view = global::OCPN::get().gui().view();
			RoutePoint* pWP = new RoutePoint(zpos, view.default_wp_icon, wxEmptyString);
			pWP->m_bIsolatedMark = true; // This is an isolated mark
			pSelect->AddSelectableRoutePoint(zpos, pWP);
			pConfig->AddNewWayPoint(pWP, -1); // use auto next num

			if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
				pRouteManagerDialog->UpdateWptListCtrl();
			undo->BeforeUndoableAction(UndoAction::Undo_CreateWaypoint, pWP,
									   UndoAction::Undo_HasParent, NULL);
			undo->AfterUndoableAction(NULL);
			Refresh(false); // Needed for MSW, why not GTK??
		} break;

		case ID_DEF_MENU_AISTARGETLIST:
			ShowAISTargetList();
			break;

		case ID_WP_MENU_GOTO: {
			const global::GUI::View& view = global::OCPN::get().gui().view();
			const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
			RoutePoint* pWP_src = new RoutePoint(nav.pos, view.default_wp_icon, wxEmptyString);
			pSelect->AddSelectableRoutePoint(nav.pos, pWP_src);

			Route* temp_route = new Route();
			pRouteList->push_back(temp_route);

			temp_route->AddPoint(pWP_src);
			temp_route->AddPoint(m_pFoundRoutePoint);
			m_pFoundRoutePoint->m_bKeepXRoute = true;

			pSelect->AddSelectableRouteSegment(nav.pos, m_pFoundRoutePoint->get_position(), pWP_src,
											   m_pFoundRoutePoint, temp_route);

			wxString name = m_pFoundRoutePoint->GetName();
			if (name.IsEmpty())
				name = _("(Unnamed Waypoint)");
			wxString rteName = _("Go to ");
			rteName.Append(name);
			temp_route->set_name(rteName);
			temp_route->set_startString(_("Here"));
			temp_route->set_endString(name);
			temp_route->m_bDeleteOnArrival = true;

			navigation::RouteManager& routemanager = global::OCPN::get().routeman();
			if (routemanager.GetpActiveRoute())
				routemanager.DeactivateRoute();

			routemanager.ActivateRoute(temp_route, m_pFoundRoutePoint);
		} break;

		case ID_DEF_MENU_COGUP:
			gFrame->ToggleCourseUp();
			break;

		case ID_DEF_MENU_NORTHUP:
			gFrame->ToggleCourseUp();
			break;

		case ID_DEF_MENU_GOTOPOSITION:
			if (NULL
				== pGoToPositionDialog) // There is one global instance of the Go To position Dialog
				pGoToPositionDialog = new GoToPositionDialog(this);
			pGoToPositionDialog->CheckPasteBufferForPosition();
			pGoToPositionDialog->Show();
			break;

		case ID_WP_MENU_DELPOINT: {
			if (m_pFoundRoutePoint == pAnchorWatchPoint1) {
				pAnchorWatchPoint1 = NULL;
				global::OCPN::get().nav().set_anchor_AW1GUID(_T(""));
			} else if (m_pFoundRoutePoint == pAnchorWatchPoint2) {
				pAnchorWatchPoint2 = NULL;
				global::OCPN::get().nav().set_anchor_AW2GUID(_T(""));
			}

			if (m_pFoundRoutePoint && !(m_pFoundRoutePoint->m_bIsInLayer)
				&& (m_pFoundRoutePoint->icon_name() != _T("mob"))) {

				// If the WP belongs to an invisible route, we come here instead of to
				// ID_RT_MENU_DELPOINT
				// Check it, and if so then remove the point from its routes
				navigation::RouteManager::RouteArray route_array
					= global::OCPN::get().routeman().GetRouteArrayContaining(m_pFoundRoutePoint);
				if (!route_array.empty()) {
					pWayPointMan->DestroyWaypoint(m_pFoundRoutePoint);
					m_pFoundRoutePoint = NULL;
				} else {
					undo->BeforeUndoableAction(UndoAction::Undo_DeleteWaypoint, m_pFoundRoutePoint,
											   UndoAction::Undo_IsOrphanded, m_pFoundPoint);
					pConfig->DeleteWayPoint(m_pFoundRoutePoint);
					pSelect->DeleteSelectablePoint(m_pFoundRoutePoint, SelectItem::TYPE_ROUTEPOINT);
					if (NULL != pWayPointMan)
						pWayPointMan->remove(m_pFoundRoutePoint);
					m_pFoundRoutePoint = NULL;
					undo->AfterUndoableAction(NULL);
				}

				if (pMarkPropDialog) {
					pMarkPropDialog->SetRoutePoint(NULL);
					pMarkPropDialog->UpdateProperties();
				}

				if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
					pRouteManagerDialog->UpdateWptListCtrl();
			}
		} break;

		case ID_WP_MENU_PROPERTIES:
			ShowMarkPropertiesDialog(m_pFoundRoutePoint);
			break;

		case ID_WP_MENU_CLEAR_ANCHORWATCH:
			if (pAnchorWatchPoint1 == m_pFoundRoutePoint) {
				pAnchorWatchPoint1 = NULL;
				global::OCPN::get().nav().set_anchor_AW1GUID(_T(""));
			} else if (pAnchorWatchPoint2 == m_pFoundRoutePoint) {
				pAnchorWatchPoint2 = NULL;
				global::OCPN::get().nav().set_anchor_AW2GUID(_T(""));
			}
			break;

		case ID_WP_MENU_SET_ANCHORWATCH:
			if (pAnchorWatchPoint1 == NULL) {
				pAnchorWatchPoint1 = m_pFoundRoutePoint;
				global::OCPN::get().nav().set_anchor_AW1GUID(pAnchorWatchPoint1->guid());
				wxString nn = m_pFoundRoutePoint->GetName();
				if (nn.IsNull()) {
					nn.Printf(_T("%d m"), global::OCPN::get().nav().anchor().AWDefault);
					m_pFoundRoutePoint->SetName(nn);
				}
			} else if (pAnchorWatchPoint2 == NULL) {
				pAnchorWatchPoint2 = m_pFoundRoutePoint;
				global::OCPN::get().nav().set_anchor_AW2GUID(pAnchorWatchPoint2->guid());
				wxString nn = m_pFoundRoutePoint->GetName();
				if (nn.IsNull()) {
					nn.Printf(_T("%d m"), global::OCPN::get().nav().anchor().AWDefault);
					m_pFoundRoutePoint->SetName(nn);
				}
			}
			break;

		case ID_WP_MENU_ADDITIONAL_INFO:
			if (NULL == pMarkInfoDialog) // There is one global instance of the MarkInfo Dialog
				pMarkInfoDialog = new MarkInfoImpl(this);

			pMarkInfoDialog->SetRoutePoint(m_pFoundRoutePoint);
			pMarkInfoDialog->UpdateProperties();

			pMarkInfoDialog->Show();
			break;

		case ID_DEF_MENU_ACTIVATE_MEASURE:
			m_bMeasure_Active = true;
			m_nMeasureState = 1;
			break;

		case ID_DEF_MENU_DEACTIVATE_MEASURE:
			m_bMeasure_Active = false;
			m_nMeasureState = 0;
			global::OCPN::get().routeman().DeleteRoute(m_pMeasureRoute);
			m_pMeasureRoute = NULL;
			gFrame->SurfaceToolbar();
			Refresh(false);
			break;

#ifdef USE_S57
		case ID_DEF_MENU_CM93OFFSET_DIALOG:
			if (NULL == g_pCM93OffsetDialog) {
				if (!VPoint.is_quilt() && Current_Ch
					&& (Current_Ch->GetChartType() == CHART_TYPE_CM93COMP)) {
					cm93compchart* pch = (cm93compchart*)Current_Ch;
					g_pCM93OffsetDialog = new CM93OffsetDialog(parent_frame, pch);
				}
			}
			g_pCM93OffsetDialog->Show();
			g_pCM93OffsetDialog->UpdateMCOVRList(GetVP());

			break;

		case ID_DEF_MENU_QUERY: {
			ShowObjectQueryWindow(popx, popy, zpos.lat(), zpos.lon());
			break;
		}
#endif
		case ID_DEF_MENU_AIS_QUERY: {
			wxWindow* pwin = wxDynamicCast(this, wxWindow);
			ShowAISTargetQueryDialog(pwin, m_FoundAIS_MMSI);
			break;
		}

		case ID_DEF_MENU_AIS_CPA: { // TR 2012.06.28: Show AIS-CPA
			AIS_Target_Data* myptarget
				= g_pAIS->Get_Target_Data_From_MMSI(m_FoundAIS_MMSI); // TR 2012.06.28: Show AIS-CPA
			if (myptarget) // TR 2012.06.28: Show AIS-CPA
				myptarget->Toggle_AIS_CPA(); // TR 2012.06.28: Show AIS-CPA
			break; // TR 2012.06.28: Show AIS-CPA
		}

		case ID_DEF_MENU_AISSHOWTRACK: {
			AIS_Target_Data* myptarget = g_pAIS->Get_Target_Data_From_MMSI(m_FoundAIS_MMSI);
			if (myptarget)
				myptarget->ToggleShowTrack();
			break;
		}

		case ID_DEF_MENU_QUILTREMOVE: {
			if (VPoint.is_quilt()) {
				int dbIndex = m_pQuilt->GetChartdbIndexAtPix(wxPoint(popx, popy));
				parent_frame->RemoveChartFromQuilt(dbIndex);

				ReloadVP();
			}
			break;
		}

		case ID_DEF_MENU_CURRENTINFO: {
			DrawTCWindow(popx, popy, m_pIDXCandidate);
			Refresh(false);

			break;
		}

		case ID_DEF_MENU_TIDEINFO: {
			DrawTCWindow(popx, popy, m_pIDXCandidate);
			Refresh(false);

			break;
		}
		case ID_RT_MENU_REVERSE: {
			if (m_pSelectedRoute->m_bIsInLayer)
				break;

			pSelect->DeleteAllSelectableRouteSegments(m_pSelectedRoute);
			int ask_return
				= OCPNMessageBox(this, global::OCPN::get().routeman().GetRouteReverseMessage(),
								 _("Rename Waypoints?"), wxYES_NO);
			m_pSelectedRoute->Reverse(ask_return == wxID_YES);
			pSelect->AddAllSelectableRouteSegments(m_pSelectedRoute);
			pConfig->UpdateRoute(m_pSelectedRoute);
			if (pRoutePropDialog && (pRoutePropDialog->IsShown())) {
				pRoutePropDialog->SetRouteAndUpdate(m_pSelectedRoute);
				pRoutePropDialog->UpdateProperties();
			}
			break;
		}

		case ID_RT_MENU_DELETE: {
			int dlg_return = wxID_YES;
			if (global::OCPN::get().gui().view().ConfirmObjectDelete) {
				dlg_return = OCPNMessageBox(this, _("Are you sure you want to delete this route?"),
											_("OpenCPN Route Delete"),
											(long)wxYES_NO | wxCANCEL | wxYES_DEFAULT);
			}

			if (dlg_return == wxID_YES) {
				navigation::RouteManager& routemanager = global::OCPN::get().routeman();
				if (routemanager.GetpActiveRoute() == m_pSelectedRoute)
					routemanager.DeactivateRoute();

				if (m_pSelectedRoute->m_bIsInLayer)
					break;

				pConfig->DeleteConfigRoute(m_pSelectedRoute);
				routemanager.DeleteRoute(m_pSelectedRoute);
				if (pRoutePropDialog && (pRoutePropDialog->IsShown())
					&& (m_pSelectedRoute == pRoutePropDialog->getRoute())) {
					pRoutePropDialog->Hide();
				}

				m_pSelectedRoute = NULL;
				m_pFoundRoutePoint = NULL;
				m_pFoundRoutePointSecond = NULL;

				if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
					pRouteManagerDialog->UpdateRouteListCtrl();

				if (pMarkPropDialog && pMarkPropDialog->IsShown()) {
					pMarkPropDialog->ValidateMark();
					pMarkPropDialog->UpdateProperties();
				}

				undo->InvalidateUndo();
			}
			break;
		}

		case ID_RT_MENU_ACTIVATE: {
			const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
			navigation::RouteManager& routemanager = global::OCPN::get().routeman();
			if (routemanager.GetpActiveRoute())
				routemanager.DeactivateRoute();

			RoutePoint* best_point = m_pSelectedRoute->FindBestActivatePoint(nav.pos, nav.cog);
			routemanager.ActivateRoute(m_pSelectedRoute, best_point);
			m_pSelectedRoute->m_bRtIsSelected = false;
			break;
		}

		case ID_RT_MENU_DEACTIVATE:
			global::OCPN::get().routeman().DeactivateRoute();
			m_pSelectedRoute->m_bRtIsSelected = false;

			break;

		case ID_RT_MENU_INSERT:

			if (m_pSelectedRoute->m_bIsInLayer)
				break;

			m_pSelectedRoute->InsertPointBefore(m_pFoundRoutePointSecond, zpos.lat(), zpos.lon());

			pSelect->DeleteAllSelectableRoutePoints(m_pSelectedRoute);
			pSelect->DeleteAllSelectableRouteSegments(m_pSelectedRoute);

			pSelect->AddAllSelectableRouteSegments(m_pSelectedRoute);
			pSelect->AddAllSelectableRoutePoints(m_pSelectedRoute);

			m_pSelectedRoute->RebuildGUIDList(); // ensure the GUID list is intact and good
			pConfig->UpdateRoute(m_pSelectedRoute);

			if (pRoutePropDialog && (pRoutePropDialog->IsShown())) {
				pRoutePropDialog->SetRouteAndUpdate(m_pSelectedRoute);
				pRoutePropDialog->UpdateProperties();
			}

			break;

		case ID_RT_MENU_APPEND:

			if (m_pSelectedRoute->m_bIsInLayer)
				break;

			m_pMouseRoute = m_pSelectedRoute;
			parent_frame->nRoute_State = m_pSelectedRoute->GetnPoints() + 1;
			m_pMouseRoute->m_lastMousePointIndex = m_pSelectedRoute->GetnPoints();
			pLast = m_pSelectedRoute->GetLastPoint();
			m_prev_route = pLast->get_position();
			m_prev_pMousePoint = pLast;
			m_bAppendingRoute = true;

			SetCursor(*pCursorPencil);
			break;

		case ID_RT_MENU_COPY:
			if (m_pSelectedRoute)
				Kml::CopyRouteToClipboard(m_pSelectedRoute);
			break;

		case ID_TK_MENU_COPY:
			if (m_pSelectedTrack)
				Kml::CopyTrackToClipboard((Track*)m_pSelectedTrack);
			break;

		case ID_WPT_MENU_COPY:
			if (m_pFoundRoutePoint)
				Kml::CopyWaypointToClipboard(m_pFoundRoutePoint);
			break;

		case ID_WPT_MENU_SENDTOGPS:
			if (m_pFoundRoutePoint) {
				if (m_active_upload_port.Length())
					m_pFoundRoutePoint->SendToGPS(m_active_upload_port, NULL);
				else {
					SendToGpsDlg dlg;
					dlg.SetWaypoint(m_pFoundRoutePoint);

					dlg.Create(NULL, -1, _("Send To GPS..."), _T(""));
					dlg.ShowModal();
				}
			}
			break;

		case ID_RT_MENU_SENDTOGPS:
			if (m_pSelectedRoute) {
				if (m_active_upload_port.Length())
					m_pSelectedRoute->SendToGPS(m_active_upload_port, true, NULL);
				else {
					SendToGpsDlg dlg;
					dlg.SetRoute(m_pSelectedRoute);

					dlg.Create(NULL, -1, _("Send To GPS..."), _T(""));
					dlg.ShowModal();
				}
			}
			break;

		case ID_PASTE_WAYPOINT:
			pupHandler_PasteWaypoint();
			break;

		case ID_PASTE_ROUTE:
			pupHandler_PasteRoute();
			break;

		case ID_PASTE_TRACK:
			pupHandler_PasteTrack();
			break;

		case ID_RT_MENU_DELPOINT:
			if (m_pSelectedRoute) {
				if (m_pSelectedRoute->m_bIsInLayer)
					break;

				pWayPointMan->DestroyWaypoint(m_pFoundRoutePoint);
				m_pFoundRoutePoint = NULL;

				// Selected route may have been deleted as one-point route, so check it
				if (!global::OCPN::get().routeman().IsRouteValid(m_pSelectedRoute))
					m_pSelectedRoute = NULL;

				if (pRoutePropDialog && (pRoutePropDialog->IsShown())) {
					if (m_pSelectedRoute) {
						pRoutePropDialog->SetRouteAndUpdate(m_pSelectedRoute);
						pRoutePropDialog->UpdateProperties();
					} else
						pRoutePropDialog->Hide();
				}

				if (pRouteManagerDialog && pRouteManagerDialog->IsShown()) {
					pRouteManagerDialog->UpdateWptListCtrl();
					pRouteManagerDialog->UpdateRouteListCtrl();
				}
			}

			break;

		case ID_RT_MENU_REMPOINT:
			if (m_pSelectedRoute) {
				if (m_pSelectedRoute->m_bIsInLayer)
					break;
				RemovePointFromRoute(m_pFoundRoutePoint, m_pSelectedRoute);
			}
			break;

		case ID_RT_MENU_ACTPOINT:
			if (global::OCPN::get().routeman().GetpActiveRoute() == m_pSelectedRoute) {
				global::OCPN::get().routeman().ActivateRoutePoint(m_pSelectedRoute,
																  m_pFoundRoutePoint);
				m_pSelectedRoute->m_bRtIsSelected = false;
			}

			break;

		case ID_RT_MENU_DEACTPOINT:
			break;

		case ID_RT_MENU_ACTNXTPOINT:
			if (global::OCPN::get().routeman().GetpActiveRoute() == m_pSelectedRoute) {
				global::OCPN::get().routeman().ActivateNextPoint(m_pSelectedRoute, true);
				m_pSelectedRoute->m_bRtIsSelected = false;
			}

			break;

		case ID_RT_MENU_PROPERTIES: {
			ShowRoutePropertiesDialog(_("Route Properties"), m_pSelectedRoute);
			break;
		}

		case ID_TK_MENU_PROPERTIES: {
			ShowTrackPropertiesDialog(m_pSelectedTrack);
			break;
		}

		case ID_TK_MENU_DELETE: {
			int dlg_return = wxID_YES;
			if (global::OCPN::get().gui().view().ConfirmObjectDelete) {
				dlg_return = OCPNMessageBox(this, _("Are you sure you want to delete this track?"),
											_("OpenCPN Track Delete"),
											(long)wxYES_NO | wxCANCEL | wxYES_DEFAULT);
			}

			if (dlg_return == wxID_YES) {

				if (global::OCPN::get().tracker().is_active_track(m_pSelectedTrack))
					global::OCPN::get().tracker().stop();

				pConfig->DeleteConfigRoute(m_pSelectedTrack);

				global::OCPN::get().routeman().DeleteTrack(m_pSelectedTrack);

				if (pTrackPropDialog && (pTrackPropDialog->IsShown())
					&& (m_pSelectedTrack == pTrackPropDialog->GetTrack())) {
					pTrackPropDialog->Hide();
				}

				m_pSelectedTrack = NULL;
				m_pFoundRoutePoint = NULL;
				m_pFoundRoutePointSecond = NULL;

				if (pRouteManagerDialog && pRouteManagerDialog->IsShown()) {
					pRouteManagerDialog->UpdateTrkListCtrl();
					pRouteManagerDialog->UpdateRouteListCtrl();
				}
			}
			break;
		}

		case ID_RC_MENU_SCALE_IN:
			parent_frame->DoStackDown();
			r = GetCanvasPointPix(zpos);
			WarpPointer(r.x, r.y);
			break;

		case ID_RC_MENU_SCALE_OUT:
			parent_frame->DoStackUp();
			r = GetCanvasPointPix(zpos);
			WarpPointer(r.x, r.y);
			break;

		case ID_RC_MENU_ZOOM_IN:
			SetVPScale(GetVPScale() * 2);
			r = GetCanvasPointPix(zpos);
			WarpPointer(r.x, r.y);
			break;

		case ID_RC_MENU_ZOOM_OUT:
			SetVPScale(GetVPScale() / 2);
			r = GetCanvasPointPix(zpos);
			WarpPointer(r.x, r.y);
			break;

		case ID_RC_MENU_FINISH:
			FinishRoute();
			gFrame->SurfaceToolbar();
			Refresh(false);
			break;

		default: {
			//  Look for PlugIn Context Menu selections
			//  If found, make the callback
			ArrayOfPlugInMenuItems item_array = g_pi_manager->GetPluginContextMenuItemArray();

			for (unsigned int i = 0; i < item_array.size(); i++) {
				PlugInMenuItemContainer* pimis = item_array.Item(i);
				{
					if (pimis->id == event.GetId()) {
						if (pimis->m_pplugin)
							pimis->m_pplugin->OnContextMenuItemCallback(pimis->id);
					}
				}
			}

			break;
		}
	}

	//  Chart Groups....
	if ((event.GetId() >= ID_DEF_MENU_GROUPBASE)
		&& (event.GetId() <= ID_DEF_MENU_GROUPBASE + (int)g_pGroupArray->size())) {
		gFrame->SetGroupIndex(event.GetId() - ID_DEF_MENU_GROUPBASE);
	}

	click_stop = 0; // Context menu was processed, all is well
}

void ChartCanvas::FinishRoute(void)
{
	parent_frame->nRoute_State = 0;
	m_prev_pMousePoint = NULL;

	parent_frame->SetToolbarItemState(ID_ROUTE, false);
	SetCursor(*pCursorArrow);
	m_bDrawingRoute = false;

	if (m_pMouseRoute) {
		if (m_bAppendingRoute)
			pConfig->UpdateRoute(m_pMouseRoute);
		else {
			if (m_pMouseRoute->GetnPoints() > 1) {
				pConfig->AddNewRoute(m_pMouseRoute, -1); // use auto next num
			} else {
				global::OCPN::get().routeman().DeleteRoute(m_pMouseRoute);
				m_pMouseRoute = NULL;
			}

			if (m_pMouseRoute)
				m_pMouseRoute->RebuildGUIDList(); // ensure the GUID list is intact and good
		}
		if (m_pMouseRoute)
			m_pMouseRoute->RebuildGUIDList(); // ensure the GUID list is intact and good

		if (pRoutePropDialog && (pRoutePropDialog->IsShown())) {
			pRoutePropDialog->SetRouteAndUpdate(m_pMouseRoute);
			pRoutePropDialog->UpdateProperties();
		}

		if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
			pRouteManagerDialog->UpdateRouteListCtrl();
	}
	m_bAppendingRoute = false;
	m_pMouseRoute = NULL;

	m_pSelectedRoute = NULL;
	m_pFoundRoutePointSecond = NULL;

	undo->InvalidateUndo();
}

void ChartCanvas::ShowAISTargetList(void)
{
	if (NULL == g_pAISTargetList) { // There is one global instance of the Dialog
		g_pAISTargetList = new ais::AISTargetListDialog(parent_frame, g_pauimgr, g_pAIS);
	}

	g_pAISTargetList->UpdateAISTargetList();
}

void ChartCanvas::RenderAllChartOutlines(ocpnDC& dc, const ViewPort& vp)
{
	const global::GUI::View& view = global::OCPN::get().gui().view();
	if (!view.show_outlines)
		return;

	int nEntry = ChartData->GetChartTableEntries();

	using namespace chart;

	for (int i = 0; i < nEntry; i++) {
		ChartTableEntry* pt = (ChartTableEntry*)&ChartData->GetChartTableEntry(i);

		// Check to see if the candidate chart is in the currently active group
		bool b_group_draw = false;
		if (view.GroupIndex > 0) {
			for (unsigned int ig = 0; ig < pt->GetGroupArray().size(); ig++) {
				int index = pt->GetGroupArray().at(ig);
				if (view.GroupIndex == index) {
					b_group_draw = true;
					break;
				}
			}
		} else
			b_group_draw = true;

		if (b_group_draw)
			RenderChartOutline(dc, i, vp);
	}

#ifdef USE_S57
	// On CM93 Composite Charts, draw the outlines of the next smaller scale cell
	if (Current_Ch && (Current_Ch->GetChartType() == CHART_TYPE_CM93COMP)) {
		cm93compchart* pch = (cm93compchart*)Current_Ch;
		if (pch) {
			wxPen mPen(global::OCPN::get().color().get_color(_T("UINFM")), 1, wxSOLID);
			dc.SetPen(mPen);
			pch->RenderNextSmallerCellOutlines(dc, vp);
		}
	}
#endif
}

void ChartCanvas::RenderChartOutline(ocpnDC& dc, int dbIndex, const ViewPort& vp)
{
	float plylat;
	float plylon;
	float plylat1;
	float plylon1;

	int pixx;
	int pixy;
	int pixx1;
	int pixy1;
	bool b_draw = false;
	double lon_bias = 0.;

	geo::BoundingBox box;
	ChartData->GetDBBoundingBox(dbIndex, &box);

	if (vp.GetBBox().Intersect(box, 0)
		!= geo::BoundingBox::_OUT) // chart is not outside of viewport
		b_draw = true;

	// Does simple test fail, and current vp cross international dateline?
	if (!b_draw && ((vp.GetBBox().GetMinX() < -180.0) || (vp.GetBBox().GetMaxX() > 180.0))) {
		//  If so, do an explicit test with alternate phasing
		if (vp.GetBBox().GetMinX() < -180.0) {
			box.Translate(-360.0, 0.0);
			if (vp.GetBBox().Intersect(box, 0) != geo::BoundingBox::_OUT) {
				// chart is not outside of viewport
				b_draw = true;
				lon_bias = -360.0;
			}
		} else {
			box.Translate(360.0, 0.0);
			if (vp.GetBBox().Intersect(box, 0) != geo::BoundingBox::_OUT) {
				// chart is not outside of viewport
				b_draw = true;
				lon_bias = 360.0;
			}
		}
	}

	// Does simple test fail, and chart box cross international dateline?
	if (!b_draw && (box.GetMinX() < 180.0) && (box.GetMaxX() > 180.0)) {
		box.Translate(-360.0, 0.0);
		if (vp.GetBBox().Intersect(box, 0) != geo::BoundingBox::_OUT) {
			// chart is not outside of viewport
			b_draw = true;
			lon_bias = -360.0;
		}
	}

	if (!b_draw)
		return;

	int nPly = ChartData->GetDBPlyPoint(dbIndex, 0, &plylat, &plylon);

	using namespace chart;
	const global::ColorManager& colors = global::OCPN::get().color();

	if (ChartData->GetDBChartType(dbIndex) == CHART_TYPE_S57)
		dc.SetPen(wxPen(colors.get_color(_T("UINFG")), 1, wxSOLID));
	else if (ChartData->GetDBChartType(dbIndex) == CHART_TYPE_CM93)
		dc.SetPen(wxPen(colors.get_color(_T("YELO1")), 1, wxSOLID));
	else
		dc.SetPen(wxPen(colors.get_color(_T("UINFR")), 1, wxSOLID));

	// Are there any aux ply entries?
	int nAuxPlyEntries = ChartData->GetnAuxPlyEntries(dbIndex);
	if (0 == nAuxPlyEntries) { // There are no aux Ply Point entries
		wxPoint r1;

		ChartData->GetDBPlyPoint(dbIndex, 0, &plylat, &plylon);
		plylon += lon_bias;

		wxPoint r = GetCanvasPointPix(geo::Position(plylat, plylon));
		pixx = r.x;
		pixy = r.y;

		for (int i = 0; i < nPly - 1; i++) {
			ChartData->GetDBPlyPoint(dbIndex, i + 1, &plylat1, &plylon1);
			plylon1 += lon_bias;

			r1 = GetCanvasPointPix(geo::Position(plylat1, plylon1));
			pixx1 = r1.x;
			pixy1 = r1.y;

			int pixxs1 = pixx1;
			int pixys1 = pixy1;

			bool b_skip = false;

			if (vp.chart_scale > 5e7) {
				// calculate projected distance between these two points in meters
				double dist = sqrt((double)((pixx1 - pixx) * (pixx1 - pixx))
								   + ((pixy1 - pixy) * (pixy1 - pixy))) / vp.view_scale();
				// calculate GC distance between these two points in meters
				double distgc = geo::DistGreatCircle(geo::Position(plylat, plylon),
													 geo::Position(plylat1, plylon1)) * 1852.0;

				// If the distances are nonsense, it means that the scale is very small and the
				// segment wrapped the world
				// So skip it....
				// TODO improve this to draw two segments
				if (fabs(dist - distgc) > 10000. * 1852.) // lotsa miles
					b_skip = true;
			}

			geo::ClipResult res = geo::cohen_sutherland_line_clip_i(&pixx, &pixy, &pixx1, &pixy1, 0,
																	vp.pix_width, 0, vp.pix_height);
			if (res != geo::Invisible && !b_skip)
				dc.DrawLine(pixx, pixy, pixx1, pixy1, false);

			plylat = plylat1;
			plylon = plylon1;
			pixx = pixxs1;
			pixy = pixys1;
		}

		ChartData->GetDBPlyPoint(dbIndex, 0, &plylat1, &plylon1);
		plylon1 += lon_bias;

		r1 = GetCanvasPointPix(geo::Position(plylat1, plylon1));
		pixx1 = r1.x;
		pixy1 = r1.y;

		geo::ClipResult res = geo::cohen_sutherland_line_clip_i(&pixx, &pixy, &pixx1, &pixy1, 0,
																vp.pix_width, 0, vp.pix_height);
		if (res != geo::Invisible)
			dc.DrawLine(pixx, pixy, pixx1, pixy1, false);
	} else { // Use Aux PlyPoints
		wxPoint r;
		wxPoint r1;

		int nAuxPlyEntries = ChartData->GetnAuxPlyEntries(dbIndex);
		for (int j = 0; j < nAuxPlyEntries; j++) {

			int nAuxPly = ChartData->GetDBAuxPlyPoint(dbIndex, 0, j, &plylat, &plylon);
			wxPoint r = GetCanvasPointPix(geo::Position(plylat, plylon));
			pixx = r.x;
			pixy = r.y;

			for (int i = 0; i < nAuxPly - 1; i++) {
				ChartData->GetDBAuxPlyPoint(dbIndex, i + 1, j, &plylat1, &plylon1);

				r1 = GetCanvasPointPix(geo::Position(plylat1, plylon1));
				pixx1 = r1.x;
				pixy1 = r1.y;

				int pixxs1 = pixx1;
				int pixys1 = pixy1;

				bool b_skip = false;

				if (vp.chart_scale > 5e7) {
					//    calculate projected distance between these two points in meters
					double dist = sqrt((double)((pixx1 - pixx) * (pixx1 - pixx))
									   + ((pixy1 - pixy) * (pixy1 - pixy))) / vp.view_scale();
					//    calculate GC distance between these two points in meters
					double distgc = geo::DistGreatCircle(geo::Position(plylat, plylon),
														 geo::Position(plylat1, plylon1)) * 1852.0;

					//    If the distances are nonsense, it means that the scale is very small and
					// the segment wrapped the world
					//    So skip it....
					//    TODO improve this to draw two segments
					if (fabs(dist - distgc) > 10000.0 * 1852.0) // lotsa miles
						b_skip = true;
				}

				geo::ClipResult res = geo::cohen_sutherland_line_clip_i(
					&pixx, &pixy, &pixx1, &pixy1, 0, vp.pix_width, 0, vp.pix_height);
				if (res != geo::Invisible && !b_skip)
					dc.DrawLine(pixx, pixy, pixx1, pixy1);

				plylat = plylat1;
				plylon = plylon1;
				pixx = pixxs1;
				pixy = pixys1;
			}

			ChartData->GetDBAuxPlyPoint(dbIndex, 0, j, &plylat1, &plylon1);
			r1 = GetCanvasPointPix(geo::Position(plylat1, plylon1));
			pixx1 = r1.x;
			pixy1 = r1.y;

			geo::ClipResult res = geo::cohen_sutherland_line_clip_i(&pixx, &pixy, &pixx1, &pixy1, 0,
																	vp.pix_width, 0, vp.pix_height);
			if (res != geo::Invisible)
				dc.DrawLine(pixx, pixy, pixx1, pixy1, false);
		}
	}
}

bool ChartCanvas::PurgeGLCanvasChartCache(chart::ChartBase* pc)
{
#ifdef ocpnUSE_GL
	if (global::OCPN::get().gui().view().opengl && m_glcc)
		m_glcc->PurgeChartTextures(pc);
#endif
	return true;
}

wxString ChartCanvas::FormatDistanceAdaptive(double distance)
{
	wxString result;
	if (distance < 0.1) {
		result << wxString::Format(_T("%3.0f "), distance * 1852.0) << _T("m");
		return result;
	}
	if (distance < 5.0) {
		result << wxString::Format(_T("%1.2f "), toUsrDistance(distance)) << getUsrDistanceUnit();
		return result;
	}
	if (distance < 100.0) {
		result << wxString::Format(_T("%2.1f "), toUsrDistance(distance)) << getUsrDistanceUnit();
		return result;
	}
	if (distance < 1000.0) {
		result << wxString::Format(_T("%3.0f "), toUsrDistance(distance)) << getUsrDistanceUnit();
		return result;
	}
	result << wxString::Format(_T("%4.0f "), toUsrDistance(distance)) << getUsrDistanceUnit();
	return result;
}

void RenderExtraRouteLegInfo(ocpnDC& dc, wxPoint ref_point, wxString s)
{
	wxFont* dFont = FontMgr::Get().GetFont(_("RouteLegInfoRollover"), 12);
	dc.SetFont(*dFont);

	int w;
	int h;
	int xp;
	int yp;
	int hilite_offset = 3;
#ifdef __WXMAC__
	wxScreenDC sdc;
	sdc.GetTextExtent(s, &w, &h, NULL, NULL, dFont);
#else
	dc.GetTextExtent(s, &w, &h);
#endif

	xp = ref_point.x - w;
	yp = ref_point.y + h;
	yp += hilite_offset;

	const global::ColorManager& colors = global::OCPN::get().color();

	dc.AlphaBlending(xp, yp, w, h, 0.0, colors.get_color(_T("YELO1")), 172);
	dc.SetPen(wxPen(colors.get_color(_T("UBLCK"))));
	dc.DrawText(s, xp, yp);
}

void ChartCanvas::RenderRouteLegs(ocpnDC& dc)
{
	if ((parent_frame->nRoute_State >= 2)
		|| (m_pMeasureRoute && m_bMeasure_Active && (m_nMeasureState >= 2))) {

		double rhumbBearing;
		double rhumbDist;
		double gcBearing;
		double gcBearing2;
		double gcDist;
		geo::DistanceBearingMercator(m_cursor_pos, m_prev_route, &rhumbBearing, &rhumbDist);
		geo::Geodesic::GreatCircleDistBear(m_prev_route, m_cursor_pos, &gcDist, &gcBearing,
										   &gcBearing2);
		double gcDistm = gcDist / 1852.0;

		if (m_prev_route == m_cursor_pos)
			rhumbBearing = 90.0;

		wxPoint destPoint, lastPoint;
		Route* route;

		if (m_pMeasureRoute) {
			route = m_pMeasureRoute;
		} else {
			route = m_pMouseRoute;
		}

		double brg = rhumbBearing;
		double dist = rhumbDist;
		route->m_NextLegGreatCircle = false;
		int milesDiff = rhumbDist - gcDistm;
		if (milesDiff > 1) {
			brg = gcBearing;
			dist = gcDistm;
			route->m_NextLegGreatCircle = true;
		}

		route->DrawPointWhich(dc, route->m_lastMousePointIndex, &lastPoint);

		if (route->m_NextLegGreatCircle) {
			for (int i = 1; i <= milesDiff; i++) {
				double p = (double)i * (1.0 / (double)milesDiff);
				double pLat, pLon;
				geo::Geodesic::GreatCircleTravel(m_prev_route, gcDist * p, brg, &pLon, &pLat,
												 &gcBearing2);
				destPoint = VPoint.GetPixFromLL(geo::Position(pLat, pLon));
				route->DrawSegment(dc, &lastPoint, &destPoint, GetVP(), false);
				lastPoint = destPoint;
			}
		} else {
			route->DrawSegment(dc, &lastPoint, &r_rband, GetVP(), false);
		}

		wxString routeInfo;
		if (global::OCPN::get().gui().view().ShowMag) // FIXME: code duplication
			routeInfo << wxString::Format(wxString("%03d°(M)  ", wxConvUTF8),
										  (int)navigation::GetTrueOrMag(brg));
		else
			routeInfo << wxString::Format(wxString("%03d°  ", wxConvUTF8),
										  (int)navigation::GetTrueOrMag(brg));

		routeInfo << _T(" ") << FormatDistanceAdaptive(dist);

		wxFont* dFont = FontMgr::Get().GetFont(_("RouteLegInfoRollover"), 12);
		dc.SetFont(*dFont);

		int w, h;
		int xp, yp;
		int hilite_offset = 3;
#ifdef __WXMAC__
		wxScreenDC sdc;
		sdc.GetTextExtent(routeInfo, &w, &h, NULL, NULL, dFont);
#else
		dc.GetTextExtent(routeInfo, &w, &h);
#endif
		xp = r_rband.x - w;
		yp = r_rband.y;
		yp += hilite_offset;

		const global::ColorManager& colors = global::OCPN::get().color();

		dc.AlphaBlending(xp, yp, w, h, 0.0, colors.get_color(_T("YELO1")), 172);
		dc.SetPen(wxPen(colors.get_color(_T("UBLCK"))));
		dc.DrawText(routeInfo, xp, yp);

		wxString s0;
		if (!route->m_bIsInLayer)
			s0.Append(_("Route: "));
		else
			s0.Append(_("Layer Route: "));

		s0 += FormatDistanceAdaptive(route->m_route_length + dist);
		RenderExtraRouteLegInfo(dc, r_rband, s0);
	}
}

void ChartCanvas::WarpPointerDeferred(int x, int y)
{
	warp_x = x;
	warp_y = y;
	warp_flag = true;
}

int spaint; // FIXME
int s_in_update; // FIXME

void ChartCanvas::OnPaint(wxPaintEvent&)
{
	wxPaintDC dc(this);

	const global::GUI::View& view = global::OCPN::get().gui().view();

	// Paint updates may have been externally disabled (temporarily, to avoid Yield() recursion performance loss)
	// It is important that the wxPaintDC is built, even if we elect to not process this paint message.
	// Otherwise, the paint message may not be removed from the message queue, esp on Windows. (FS#1213)
	// This would lead to a deadlock condition in ::wxYield()

	if (!m_b_paint_enable)
		return;

#ifdef ocpnUSE_GL
	if (!view.disable_opengl)
		m_glcc->Show(view.opengl);

	if (view.opengl) {
		if (!s_in_update) { // no recursion allowed, seen on lo-spec Mac
			s_in_update++;
			m_glcc->Update();
			s_in_update--;
		}

		return;
	}
#endif

	if ((GetVP().pix_width == 0) || (GetVP().pix_height == 0))
		return;

	wxRegion ru = GetUpdateRegion();

	int rx, ry, rwidth, rheight;
	ru.GetBox(rx, ry, rwidth, rheight);

	geo::BoundingBox BltBBox;

#ifdef ocpnUSE_DIBSECTION
	OCPNMemDC temp_dc;
#else
	wxMemoryDC temp_dc;
#endif

	wxRegion rgn_chart(0, 0, GetVP().pix_width, GetVP().pix_height);

	// In case Thumbnail is shown, set up dc clipper and blt iterator regions
	if (pthumbwin) {
		int thumbx, thumby, thumbsx, thumbsy;
		pthumbwin->GetPosition(&thumbx, &thumby);
		pthumbwin->GetSize(&thumbsx, &thumbsy);
		wxRegion rgn_thumbwin(thumbx, thumby, thumbsx - 1, thumbsy - 1);

		if (pthumbwin->IsShown()) {
			rgn_chart.Subtract(rgn_thumbwin);
			ru.Subtract(rgn_thumbwin);
		}
	}

	// Is this viewpoint the same as the previously painted one?
	bool b_newview = true;

	if ((m_cache_vp.view_scale() == VPoint.view_scale())
		&& (m_cache_vp.rotation == VPoint.rotation) && (m_cache_vp.latitude() == VPoint.latitude())
		&& (m_cache_vp.longitude() == VPoint.longitude()) && m_cache_vp.IsValid()) {
		b_newview = false;
	}

	const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();

	// If in COG UP Mode, we may be able to use the cached rotated bitmap
	bool b_rcache_ok = false;
	if (nav.CourseUp && (fabs(VPoint.rotation) > 0.01))
		b_rcache_ok = !b_newview;

	// If in skew compensation mode, with a skewed VP shown, we may be able to use the cached
	// rotated bitmap
	if (view.skew_comp && (fabs(VPoint.skew) > 0.01))
		b_rcache_ok = !b_newview;

	//  Make a special VP
	if (VPoint.b_MercatorProjectionOverride)
		VPoint.SetProjectionType(PROJECTION_MERCATOR);
	ViewPort svp = VPoint;

	svp.pix_width = svp.rv_rect.width;
	svp.pix_height = svp.rv_rect.height;

	OCPNRegion chart_get_region(wxRect(0, 0, svp.pix_width, svp.pix_height));

	// If we are going to use the cached rotated image, there is no need to fetch any chart data
	// and this will do it...
	if (b_rcache_ok)
		chart_get_region.Clear();

	// Blit pan acceleration
	if (VPoint.is_quilt()) {
		if (m_pQuilt && !m_pQuilt->IsComposed())
			return;

		if ((m_working_bm.GetWidth() != svp.pix_width)
			|| (m_working_bm.GetHeight() != svp.pix_height))
			m_working_bm.Create(svp.pix_width, svp.pix_height,
								-1); // make sure the target is big enoug

		if (!nav.CourseUp) {
			bool b_save = true;

			// If the saved wxBitmap from last OnPaint is useable
			// calculate the blit parameters

			// We can only do screen blit painting if subsequent ViewPorts differ by whole pixels
			// So, in small scale bFollow mode, force the full screen render.
			// This seems a hack....There may be better logic here.....

			if (m_bm_cache_vp.IsValid() && m_cache_vp.IsValid()) {
				if (b_newview) {
					wxPoint c_old = VPoint.GetPixFromLL(VPoint.get_position());
					wxPoint c_new = m_bm_cache_vp.GetPixFromLL(VPoint.get_position());

					int dy = c_new.y - c_old.y;
					int dx = c_new.x - c_old.x;

					if (m_pQuilt->IsVPBlittable(VPoint, dx, dy, true)) {
						if (dx || dy) {
							// Blit the reuseable portion of the cached wxBitmap to a working
							// bitmap
							temp_dc.SelectObject(m_working_bm);

							wxMemoryDC cache_dc;
							cache_dc.SelectObject(m_cached_chart_bm);

							if (dy > 0) {
								if (dx > 0) {
									temp_dc.Blit(0, 0, VPoint.pix_width - dx,
												 VPoint.pix_height - dy, &cache_dc, dx, dy);
								} else {
									temp_dc.Blit(-dx, 0, VPoint.pix_width + dx,
												 VPoint.pix_height - dy, &cache_dc, 0, dy);
								}

							} else {
								if (dx > 0) {
									temp_dc.Blit(0, -dy, VPoint.pix_width - dx,
												 VPoint.pix_height + dy, &cache_dc, dx, 0);
								} else {
									temp_dc.Blit(-dx, -dy, VPoint.pix_width + dx,
												 VPoint.pix_height + dy, &cache_dc, 0, 0);
								}
							}

							OCPNRegion update_region;
							if (dy) {
								if (dy > 0)
									update_region.Union(
										wxRect(0, VPoint.pix_height - dy, VPoint.pix_width, dy));
								else
									update_region.Union(wxRect(0, 0, VPoint.pix_width, -dy));
							}

							if (dx) {
								if (dx > 0)
									update_region.Union(
										wxRect(VPoint.pix_width - dx, 0, dx, VPoint.pix_height));
								else
									update_region.Union(wxRect(0, 0, -dx, VPoint.pix_height));
							}

							// Render the new region
							m_pQuilt->RenderQuiltRegionViewOnDC(temp_dc, svp, update_region);
							cache_dc.SelectObject(wxNullBitmap);
						} else {
							// No sensible (dx, dy) change in the view, so use the cached member
							// bitmap
							temp_dc.SelectObject(m_cached_chart_bm);
							b_save = false;
						}
						m_pQuilt->ComputeRenderRegion(svp, chart_get_region);

					} else {
						// not blitable
						temp_dc.SelectObject(m_working_bm);
						m_pQuilt->RenderQuiltRegionViewOnDC(temp_dc, svp, chart_get_region);
					}
				} else {
					// No change in the view, so use the cached member bitmap2
					temp_dc.SelectObject(m_cached_chart_bm);
					b_save = false;
				}
			} else {
				// cached bitmap is not yet valid
				temp_dc.SelectObject(m_working_bm);
				m_pQuilt->RenderQuiltRegionViewOnDC(temp_dc, svp, chart_get_region);
			}

			//  Save the fully rendered quilt image as a wxBitmap member of this class
			if (b_save) {
				wxMemoryDC scratch_dc_0;
				scratch_dc_0.SelectObject(m_cached_chart_bm);
				scratch_dc_0.Blit(0, 0, svp.pix_width, svp.pix_height, &temp_dc, 0, 0);

				scratch_dc_0.SelectObject(wxNullBitmap);

				m_bm_cache_vp = VPoint; // save the ViewPort associated with the cached wxBitmap
			}
		} else {
			// quilted, course-up
			temp_dc.SelectObject(m_working_bm);
			OCPNRegion chart_get_all_region(wxRect(0, 0, svp.pix_width, svp.pix_height));
			m_pQuilt->RenderQuiltRegionViewOnDC(temp_dc, svp, chart_get_all_region);
		}
	} else {
		// not quilted
		if (!Current_Ch) {
			dc.SetBackground(wxBrush(*wxLIGHT_GREY));
			dc.Clear();
			return;
		}

		Current_Ch->RenderRegionViewOnDC(temp_dc, svp, chart_get_region);
	}

	if (!temp_dc.IsOk())
		return;

	// Arrange to render the World Chart vector data behind the rendered current chart
	// so that uncovered canvas areas show at least the world chart.
	OCPNRegion chartValidRegion;
	if (!VPoint.is_quilt())
		Current_Ch->GetValidCanvasRegion(
			svp, &chartValidRegion); // Make a region covering the current chart on the canvas
	else
		chartValidRegion = m_pQuilt->GetFullQuiltRenderedRegion();

	// Copy current chart region
	wxRegion backgroundRegion(wxRect(0, 0, svp.pix_width, svp.pix_height));

	wxRegion clip_region;
	if (chartValidRegion.IsOk()) {
		clip_region = chartValidRegion.ConvertTowxRegion();
		backgroundRegion.Subtract(clip_region);
	}

	// Associate with temp_dc
	temp_dc.DestroyClippingRegion();
	temp_dc.SetClippingRegion(backgroundRegion);

	// Draw the Background Chart only in the areas NOT covered by the current chart view

	if ((fabs(GetVP().skew) < .01) && !backgroundRegion.IsEmpty()) {
		ocpnDC bgdc(temp_dc);
		pWorldBackgroundChart->RenderViewOnDC(bgdc, VPoint);
	}

	wxMemoryDC* pChartDC = &temp_dc;
	wxMemoryDC rotd_dc;

	if (((fabs(GetVP().rotation) > 0.01)) || (view.skew_comp && (fabs(GetVP().skew) > 0.01))) {

		//  Can we use the current rotated image cache?
		if (!b_rcache_ok) {
#ifdef __WXMSW__
			wxMemoryDC tbase_dc;
			wxBitmap bm_base(svp.pix_width, svp.pix_height, -1);
			tbase_dc.SelectObject(bm_base);
			tbase_dc.Blit(0, 0, svp.pix_width, svp.pix_height, &temp_dc, 0, 0);
			tbase_dc.SelectObject(wxNullBitmap);
#else
			const wxBitmap& bm_base = temp_dc.GetSelectedBitmap();
#endif

			wxImage base_image;
			if (bm_base.IsOk())
				base_image = bm_base.ConvertToImage();

			// Use a local static image rotator to improve wxWidgets code profile
			// Especially, on GTK the wxRound and wxRealPoint functions are very expensive.....
			double angle;
			angle = -GetVP().rotation;
			angle += GetVP().skew;

			wxImage ri;
			bool b_rot_ok = false;
			if (base_image.IsOk()) {
				ViewPort rot_vp = GetVP();

				m_b_rot_hidef = false;

				ri = Image_Rotate(base_image, angle,
								  wxPoint(GetVP().rv_rect.width / 2, GetVP().rv_rect.height / 2),
								  m_b_rot_hidef, &m_roffset);

				if ((rot_vp.view_scale() == VPoint.view_scale())
					&& (rot_vp.rotation == VPoint.rotation) && (rot_vp.latitude() == VPoint.latitude())
					&& (rot_vp.longitude() == VPoint.longitude()) && rot_vp.IsValid() && (ri.IsOk())) {
					b_rot_ok = true;
				}
			}

			if (b_rot_ok) {
				delete m_prot_bm;
				m_prot_bm = new wxBitmap(ri);
			}

			m_roffset.x += VPoint.rv_rect.x;
			m_roffset.y += VPoint.rv_rect.y;
		}

		if (m_prot_bm && m_prot_bm->IsOk()) {
			rotd_dc.SelectObject(*m_prot_bm);
			pChartDC = &rotd_dc;
		} else {
			pChartDC = &temp_dc;
			m_roffset = wxPoint(0, 0);
		}

	} else {
		pChartDC = &temp_dc;
		m_roffset = wxPoint(0, 0);
	}

	wxPoint offset = m_roffset;

	// Save the PixelCache viewpoint for next time
	m_cache_vp = VPoint;

	// Set up a scratch DC for overlay objects
	wxRegion rgn_blit;
	wxMemoryDC mscratch_dc;
	mscratch_dc.SelectObject(*pscratch_bm);

	mscratch_dc.ResetBoundingBox();
	mscratch_dc.DestroyClippingRegion();
	mscratch_dc.SetClippingRegion(rgn_chart);

	// Blit the externally invalidated areas of the chart onto the scratch dc
	rgn_blit = ru;
	wxRegionIterator upd(rgn_blit); // get the update rect list
	while (upd) {
		wxRect rect = upd.GetRect();

		mscratch_dc.Blit(rect.x, rect.y, rect.width, rect.height, pChartDC, rect.x - offset.x,
						 rect.y - offset.y);
		upd++;
	}

	// Draw the rest of the overlay objects directly on the scratch dc
	ocpnDC scratch_dc(mscratch_dc);
	DrawOverlayObjects(scratch_dc, ru);

	if (m_bShowTide)
		DrawAllTidesInBBox(scratch_dc, GetVP().GetBBox(), true, true);

	if (m_bShowCurrent)
		DrawAllCurrentsInBBox(scratch_dc, GetVP().GetBBox(), true, true);

	// quiting?
	if (g_bquiting) {
#ifdef ocpnUSE_DIBSECTION
		OCPNMemDC q_dc;
#else
		wxMemoryDC q_dc;
#endif
		wxBitmap qbm(GetVP().pix_width, GetVP().pix_height);
		q_dc.SelectObject(qbm);

		// Get a copy of the screen
		q_dc.Blit(0, 0, GetVP().pix_width, GetVP().pix_height, &mscratch_dc, 0, 0);

		// Draw a rectangle over the screen with a stipple brush
		wxBrush qbr(*wxBLACK, wxFDIAGONAL_HATCH);
		q_dc.SetBrush(qbr);
		q_dc.DrawRectangle(0, 0, GetVP().pix_width, GetVP().pix_height);

		// Blit back into source
		mscratch_dc.Blit(0, 0, GetVP().pix_width, GetVP().pix_height, &q_dc, 0, 0, wxCOPY);

		q_dc.SelectObject(wxNullBitmap);
	}

	// And finally, blit the scratch dc onto the physical dc
	wxRegionIterator upd_final(rgn_blit);
	while (upd_final) {
		wxRect rect = upd_final.GetRect();
		dc.Blit(rect.x, rect.y, rect.width, rect.height, &mscratch_dc, rect.x, rect.y);
		upd_final++;
	}

	// Deselect the chart bitmap from the temp_dc, so that it will not be destroyed in the
	// temp_dc dtor
	temp_dc.SelectObject(wxNullBitmap);
	// And for the scratch bitmap
	mscratch_dc.SelectObject(wxNullBitmap);

	dc.DestroyClippingRegion();

	PaintCleanup();
}

void ChartCanvas::PaintCleanup()
{
	// Handle the current graphic window, if present

	if (pCwin) {
		pCwin->Show();
		if (m_bTCupdate) {
			pCwin->Refresh();
			pCwin->Update();
		}
	}

	// And set flags for next time
	m_bTCupdate = false;

	// Handle deferred WarpPointer
	if (warp_flag) {
		WarpPointer(warp_x, warp_y);
		warp_flag = false;
	}
}

void ChartCanvas::CancelMouseRoute()
{
	parent_frame->nRoute_State = 0;
	m_pMouseRoute = NULL;
}

int ChartCanvas::GetNextContextMenuId()
{
	return ID_DEF_MENU_LAST + 100; // Allowing for 100 dynamic menu item identifiers
}

bool ChartCanvas::SetCursor(const wxCursor& c)
{
#ifdef ocpnUSE_GL
	if (global::OCPN::get().gui().view().opengl)
		return m_glcc->SetCursor(c);
#endif
	return wxWindow::SetCursor(c);
}

void ChartCanvas::Refresh(bool eraseBackground, const wxRect* rect)
{
	// Keep the mouse position members up to date
	m_cursor_pos = GetCanvasPixPoint(mouse_x, mouse_y);

	// Retrigger the route leg popup timer
	// This handles the case when the chart is moving in auto-follow mode, but no user mouse
	// input is made.
	// The timer handler may Hide() the popup if the chart moved enough
	// n.b.  We use slightly longer oneshot value to allow this method's Refresh() to complete
	// before
	// ptentially getting another Refresh() in the popup timer handler.
	if ((m_pRouteRolloverWin && m_pRouteRolloverWin->IsActive())
		|| (m_pAISRolloverWin && m_pAISRolloverWin->IsActive()))
		m_RolloverPopupTimer.Start(500, wxTIMER_ONE_SHOT);

#ifdef ocpnUSE_GL
	if (global::OCPN::get().gui().view().opengl) {

		m_glcc->Refresh(eraseBackground,
						NULL); // We always are going to render the entire screen anyway, so make
		// sure that the window managers understand the invalid area
		// is actually the entire client area.

		//  We need to selectively Refresh some child windows, if they are visible.
		//  Note that some children are refreshed elsewhere on timer ticks, so don't need attention
		// here.

		if (pthumbwin && pthumbwin->IsShown()) {
			pthumbwin->Raise();
			pthumbwin->Refresh(false);
		}

		// ChartInfo window
		if (m_pCIWin && m_pCIWin->IsShown()) {
			m_pCIWin->Raise();
			m_pCIWin->Refresh(false);
		}

	} else
#endif
		wxWindow::Refresh(eraseBackground, rect);
}

void ChartCanvas::Update()
{
	if (global::OCPN::get().gui().view().opengl) {
#ifdef ocpnUSE_GL
		m_glcc->Update();
#endif
	} else {
		wxWindow::Update();
	}
}

void ChartCanvas::EmbossCanvas_DC(ocpnDC& dc, EmbossData& emboss, int x, int y)
{
	const double factor = 200;

	wxMemoryDC* pmdc = dynamic_cast<wxMemoryDC*>(dc.GetDC());
	wxASSERT_MSG(pmdc, wxT("dc to EmbossCanvas not a memory dc"));

	// Grab a snipped image out of the chart
	wxMemoryDC snip_dc;
	wxBitmap snip_bmp(emboss.width(), emboss.height(), -1);
	snip_dc.SelectObject(snip_bmp);

	snip_dc.Blit(0, 0, emboss.width(), emboss.height(), pmdc, x, y);
	snip_dc.SelectObject(wxNullBitmap);

	wxImage snip_img = snip_bmp.ConvertToImage();

	// Apply Emboss map to the snip image
	unsigned char* pdata = snip_img.GetData();
	if (pdata) {
		for (int y = 0; y < emboss.height(); y++) {
			int map_index = (y * emboss.width());
			for (int x = 0; x < emboss.width(); x++) {
				double val = (emboss.at(map_index) * factor) / 256.0;

				int nred = (int)((*pdata) + val);
				nred = nred > 255 ? 255 : (nred < 0 ? 0 : nred);
				*pdata++ = (unsigned char)nred;

				int ngreen = (int)((*pdata) + val);
				ngreen = ngreen > 255 ? 255 : (ngreen < 0 ? 0 : ngreen);
				*pdata++ = (unsigned char)ngreen;

				int nblue = (int)((*pdata) + val);
				nblue = nblue > 255 ? 255 : (nblue < 0 ? 0 : nblue);
				*pdata++ = (unsigned char)nblue;

				map_index++;
			}
		}
	}

	// Convert embossed snip to a bitmap
	wxBitmap emb_bmp(snip_img);

	// Map to another memoryDC
	wxMemoryDC result_dc;
	result_dc.SelectObject(emb_bmp);

	// Blit to target
	pmdc->Blit(x, y, emboss.width(), emboss.height(), &result_dc, 0, 0);

	result_dc.SelectObject(wxNullBitmap);
}

void ChartCanvas::EmbossCanvas_GL(ocpnDC& dc, EmbossData& emboss, int x, int y)
{
#ifdef ocpnUSE_GL
	const double factor = 200;

	int a = emboss.width();
	int p = 0;
	while (a) {
		a = a >> 1;
		p++;
	}
	int width_p2 = 1 << p;

	a = emboss.height();
	p = 0;
	while (a) {
		a = a >> 1;
		p++;
	}
	int height_p2 = 1 << p;

	int w = emboss.width();
	int h = emboss.height();

	glEnable(GL_TEXTURE_2D);

	// render using opengl and alpha blending
	// FIXME: lazy initialization
	if (!emboss.gltex_index()) { // upload to texture
		// convert to luminance alpha map
		const int size = width_p2 * height_p2;
		char* data = new char[2 * size];
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < width_p2; j++) {
				if (j < w) {
					data[2 * ((i * width_p2) + j)] = emboss.at((i * w) + j) > 0 ? 0 : 255;
					data[2 * ((i * width_p2) + j) + 1] = abs(emboss.at((i * w) + j));
				}
			}
		}

		GLuint tex_index = 0;
		glGenTextures(1, &tex_index);
		glBindTexture(GL_TEXTURE_2D, tex_index);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width_p2, height_p2, 0,
					 GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		emboss.set_gltex_index(tex_index);

		delete[] data;
	}

	glBindTexture(GL_TEXTURE_2D, emboss.gltex_index());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

	glColor4f(1, 1, 1, factor / 256);

	double wp = (double)w / width_p2;
	double hp = (double)h / height_p2;

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0), glVertex2i(x, y);
	glTexCoord2f(wp, 0), glVertex2i(x + w, y);
	glTexCoord2f(wp, hp), glVertex2i(x + w, y + h);
	glTexCoord2f(0, hp), glVertex2i(x, y + h);
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
#endif
}

void ChartCanvas::EmbossCanvas(ocpnDC& dc, EmbossData& emboss, int x, int y)
{
	if (dc.GetDC()) {
		EmbossCanvas_DC(dc, emboss, x, y);
	} else {
		EmbossCanvas_GL(dc, emboss, x, y);
	}
}

void ChartCanvas::EmbossOverzoomIndicator(ocpnDC& dc)
{
	if (!global::OCPN::get().gui().view().show_overzoom_emboss)
		return;

	if (GetQuiltMode()) {
		double chart_native_ppm;
		chart_native_ppm = m_canvas_scale_factor / m_pQuilt->GetRefNativeScale();

		double zoom_factor = GetVP().view_scale() / chart_native_ppm;

		if (zoom_factor <= 4.0)
			return;
	} else {
		double chart_native_ppm;
		if (Current_Ch)
			chart_native_ppm = m_canvas_scale_factor / Current_Ch->GetNativeScale();
		else
			chart_native_ppm = m_true_scale_ppm;

		double zoom_factor = GetVP().view_scale() / chart_native_ppm;
		if (Current_Ch) {
#ifdef USE_S57
			using namespace chart;

			// Special case for cm93
			if (Current_Ch->GetChartType() == CHART_TYPE_CM93COMP) {
				if (zoom_factor > 8.0) {

					cm93compchart* pch = (cm93compchart*)Current_Ch;
					if (pch) {
						wxPen mPen(global::OCPN::get().color().get_color(_T("UINFM")), 2, wxSHORT_DASH);
						dc.SetPen(mPen);
						pch->RenderNextSmallerCellOutlines(dc, GetVP());
					}
				} else
					return;
			} else
#endif
			if (zoom_factor <= 4.0)
				return;
		}
	}

	EmbossCanvas(dc, *m_pEM_OverZoom, 0, 40);
}

void ChartCanvas::set_viewpoint_projection_type(int type)
{
	VPoint.SetProjectionType(type);
}

void ChartCanvas::DrawOverlayObjects(ocpnDC& dc, const wxRegion& ru)
{
	GridDraw(dc);

	if (g_pi_manager) {
		g_pi_manager->SendViewPortToRequestingPlugIns(GetVP());
		g_pi_manager->RenderAllCanvasOverlayPlugIns(dc, GetVP());
	}

	AISDrawAreaNotices(dc);

	EmbossDepthScale(dc);
	EmbossOverzoomIndicator(dc);

	DrawAllRoutesInBBox(dc, GetVP().GetBBox(), ru);
	DrawAllWaypointsInBBox(dc, GetVP().GetBBox(), ru, true); // true draws only isolated marks

	AISDraw(dc);
	ShipDraw(dc);
	AnchorWatchDraw(dc);

	RenderAllChartOutlines(dc, GetVP());
	RenderRouteLegs(dc);
	ScaleBarDraw(dc);
#ifdef USE_S57
	s57_DrawExtendedLightSectors(dc, VPoint, extendedSectorLegs);
#endif

	if (m_pRouteRolloverWin && m_pRouteRolloverWin->IsActive()) {
		dc.DrawBitmap(*(m_pRouteRolloverWin->GetBitmap()), m_pRouteRolloverWin->GetPosition().x,
					  m_pRouteRolloverWin->GetPosition().y, false);
	}
	if (m_pAISRolloverWin && m_pAISRolloverWin->IsActive()) {
		dc.DrawBitmap(*(m_pAISRolloverWin->GetBitmap()), m_pAISRolloverWin->GetPosition().x,
					  m_pAISRolloverWin->GetPosition().y, false);
	}
}

void ChartCanvas::EmbossDepthScale(ocpnDC& dc)
{
	if (!global::OCPN::get().gui().view().show_depth_units)
		return;

	using namespace chart;

	int depth_unit_type = DEPTH_UNIT_UNKNOWN;

	if (GetQuiltMode()) {
		wxString s = m_pQuilt->GetQuiltDepthUnit();
		s.MakeUpper();
		if (s == _T("FEET"))
			depth_unit_type = DEPTH_UNIT_FEET;
		else if (s.StartsWith(_T("FATHOMS")))
			depth_unit_type = DEPTH_UNIT_FATHOMS;
		else if (s.StartsWith(_T("METERS")))
			depth_unit_type = DEPTH_UNIT_METERS;
		else if (s.StartsWith(_T("METRES")))
			depth_unit_type = DEPTH_UNIT_METERS;
		else if (s.StartsWith(_T("METRIC")))
			depth_unit_type = DEPTH_UNIT_METERS;
		else if (s.StartsWith(_T("METER")))
			depth_unit_type = DEPTH_UNIT_METERS;

	} else {
		if (Current_Ch) {
			depth_unit_type = Current_Ch->GetDepthUnitType();
#ifdef USE_S57
			if (Current_Ch->GetChartFamily() == chart::CHART_FAMILY_VECTOR)
				depth_unit_type = ps52plib->m_nDepthUnitDisplay + 1;
#endif
		}
	}

	EmbossData* ped = NULL;
	switch (depth_unit_type) {
		case DEPTH_UNIT_FEET:
			ped = m_pEM_Feet;
			break;
		case DEPTH_UNIT_METERS:
			ped = m_pEM_Meters;
			break;
		case DEPTH_UNIT_FATHOMS:
			ped = m_pEM_Fathoms;
			break;
		default:
			ped = NULL;
			break;
	}

	if (ped)
		EmbossCanvas(dc, *ped, (GetVP().pix_width - ped->width()), 40);
}

void ChartCanvas::CreateDepthUnitEmbossMaps(global::ColorScheme cs)
{
	const ocpnStyle::Style& style = g_StyleManager->current();
	const wxString& embossFont = style.getEmbossFont();

	wxFont font;
	if (embossFont == wxEmptyString)
		font = wxFont(60, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	else
		font = wxFont(style.getEmbossHeight(), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
					  wxFONTWEIGHT_BOLD, false, embossFont);

	int emboss_width = 500;
	int emboss_height = 100;

	// Free any existing emboss maps
	delete m_pEM_Feet;
	delete m_pEM_Meters;
	delete m_pEM_Fathoms;

	// Create the 3 DepthUnit emboss map structures
	m_pEM_Feet = CreateEmbossMapData(font, emboss_width, emboss_height, _("Feet"), cs);
	m_pEM_Meters = CreateEmbossMapData(font, emboss_width, emboss_height, _("Meters"), cs);
	m_pEM_Fathoms = CreateEmbossMapData(font, emboss_width, emboss_height, _("Fathoms"), cs);
}

void ChartCanvas::CreateOZEmbossMapData(global::ColorScheme cs)
{
	delete m_pEM_OverZoom;
	m_pEM_OverZoom = NULL;

	const ocpnStyle::Style& style = g_StyleManager->current();
	const wxString& embossFont = style.getEmbossFont();

	wxFont font;
	if (embossFont == wxEmptyString)
		font = wxFont(40, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	else
		font = wxFont(style.getEmbossHeight(), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
					  wxFONTWEIGHT_BOLD, false, embossFont);

	wxClientDC dc(this);
	dc.SetFont(font);

	int w;
	int h;
	dc.GetTextExtent(_("OverZoom"), &w, &h);
	m_pEM_OverZoom = CreateEmbossMapData(font, w + 10, h + 10, _("OverZoom"), cs);
}

EmbossData* ChartCanvas::CreateEmbossMapData(wxFont& font, int width, int height, const wxChar* str,
											 global::ColorScheme cs)
{
	// Create a temporary bitmap
	wxBitmap bmp(width, height, -1);

	// Create a memory DC
	wxMemoryDC temp_dc;
	temp_dc.SelectObject(bmp);

	// Paint on it
	temp_dc.SetBackground(*wxWHITE_BRUSH);
	temp_dc.SetTextBackground(*wxWHITE);
	temp_dc.SetTextForeground(*wxBLACK);

	temp_dc.Clear();

	temp_dc.SetFont(font);

	int str_w, str_h;
	temp_dc.GetTextExtent(wxString(str, wxConvUTF8), &str_w, &str_h);
	temp_dc.DrawText(wxString(str, wxConvUTF8), width - str_w - 10, 10);

	// Deselect the bitmap
	temp_dc.SelectObject(wxNullBitmap);

	// Convert bitmap the wxImage for manipulation
	wxImage img = bmp.ConvertToImage();

	double val_factor;
	switch (cs) {
		case global::GLOBAL_COLOR_SCHEME_DAY:
		default:
			val_factor = 1;
			break;
		case global::GLOBAL_COLOR_SCHEME_DUSK:
			val_factor = 0.5;
			break;
		case global::GLOBAL_COLOR_SCHEME_NIGHT:
			val_factor = 0.25;
			break;
	}

	// Create emboss map by differentiating the emboss image
	// and storing integer results in pmap
	// n.b. since the image is B/W, it is sufficient to check
	// one channel (i.e. red) only
	EmbossData* emboss = new EmbossData(width, height);
	for (int y = 1; y < height - 1; y++) {
		for (int x = 1; x < width - 1; x++) {
			int val = img.GetRed(x + 1, y + 1) - img.GetRed(x - 1, y - 1); // range +/- 256
			val = (int)(val * val_factor);
			int index = (y * width) + x;
			emboss->at(index) = val;
		}
	}

	return emboss;
}

//----------------------------------------------------------------------------
//  Get a wxBitmap with wxMask associated containing the semi-static overlays
//----------------------------------------------------------------------------

wxBitmap* ChartCanvas::DrawTCCBitmap(wxDC* pbackground_dc, bool bAddNewSelpoints)
{
	wxBitmap* p_bmp = new wxBitmap(GetVP().pix_width, GetVP().pix_height, -1);

	//      Here is the new drawing DC
	wxMemoryDC ssdc;
	ssdc.SelectObject(*p_bmp);
	ssdc.SetBackground(*wxWHITE_BRUSH);

	//  if a background dc is provided, use it as wallpaper
	if (pbackground_dc)
		ssdc.Blit(0, 0, GetVP().pix_width, GetVP().pix_height, pbackground_dc, 0, 0);
	else
		ssdc.Clear();

	// Believe it or not, it is faster to REDRAW the overlay objects
	// onto a mono bitmap, and then convert it into a mask bitmap
	// than it is to create a mask from a colour bmp.
	// Look at the wx code.  It goes through wxImage conversion, etc...
	// So, create a mono DC, drawing white-on-black
	wxMemoryDC ssdc_mask;
	wxBitmap mask_bmp(GetVP().pix_width, GetVP().pix_height, 1);
	ssdc_mask.SelectObject(mask_bmp);

#ifndef __WXX11__
	// On X11, the drawing is Black on White, and the mask bitmap is inverted before
	// making into a mask.
	// On MSW and GTK, the drawing is White on Black, and no inversion is required
	// Todo....  Some wxWidgets problem with this....
	ssdc_mask.SetBackground(*wxBLACK_BRUSH);
#endif

	ssdc_mask.Clear();

	//    Maybe draw the Tide Points
	ocpnDC ossdc(ssdc), ossdc_mask(ssdc_mask);

	if (m_bShowTide) {
		// Rebuild Selpoints list on new map
		DrawAllTidesInBBox(ossdc, GetVP().GetBBox(), bAddNewSelpoints || !bShowingTide, true);
		DrawAllTidesInBBox(ossdc_mask, GetVP().GetBBox(), false, true, true); // onto the mask
		bShowingTide = true;
	} else
		bShowingTide = false;

	//    Maybe draw the current arrows
	if (m_bShowCurrent) {
		// Rebuild Selpoints list on new map
		// and force redraw
		DrawAllCurrentsInBBox(ossdc, GetVP().GetBBox(), bAddNewSelpoints || !bShowingCurrent, true);
		DrawAllCurrentsInBBox(ossdc_mask, GetVP().GetBBox(), false, true, true); // onto the mask
		bShowingCurrent = true;
	} else
		bShowingCurrent = false;

	ssdc.SelectObject(wxNullBitmap);

#ifdef __WXX11__
	// Invert the mono bmp, to make a useable mask bmp
	wxMemoryDC ssdc_mask_invert;
	wxBitmap mask_bmp_invert(GetVP().pix_width, GetVP().pix_height, 1);
	ssdc_mask_invert.SelectObject(mask_bmp_invert);
	ssdc_mask_invert.Blit(0, 0, GetVP().pix_width, GetVP().pix_height, &ssdc_mask, 0, 0,
						  wxSRC_INVERT);

	ssdc_mask_invert.SelectObject(wxNullBitmap);
	pss_overlay_mask = new wxMask(mask_bmp_invert);
	ssdc_mask.SelectObject(wxNullBitmap);
#else
	ssdc_mask.SelectObject(wxNullBitmap);
	pss_overlay_mask = new wxMask(mask_bmp);
#endif

	// Create and associate the mask
	p_bmp->SetMask(pss_overlay_mask);

	return p_bmp;
}

void ChartCanvas::DrawAllRoutesInBBox(ocpnDC& dc, const geo::LatLonBoundingBox& BltBBox,
									  const wxRegion& clipregion)
{
	Route* active_route = NULL;
	Route* active_track = NULL;

	wxDC* pdc = dc.GetDC();
	if (pdc) {
		pdc->DestroyClippingRegion();
		wxDCClipper(*pdc, clipregion);
	}

	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		bool b_run = false;
		bool b_drawn = false;
		Route* pRouteDraw = *i;
		if (pRouteDraw) {
			if (pRouteDraw->IsTrack()) {
				Track* trk = (Track*)pRouteDraw;
				if (trk->IsRunning()) {
					b_run = true;
					active_track = pRouteDraw;
				}

				if (pRouteDraw->IsActive() || pRouteDraw->IsSelected())
					active_route = pRouteDraw;
			}

			geo::BoundingBox test_box = pRouteDraw->RBBox;
			const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();

			if (b_run)
				test_box.Expand(nav.pos.lon(), nav.pos.lat());

			if (BltBBox.Intersect(test_box, 0) != geo::BoundingBox::_OUT) {
				// Route is not wholly outside window
				b_drawn = true;

				if ((pRouteDraw != active_route) && (pRouteDraw != active_track))
					pRouteDraw->Draw(dc, GetVP());
			} else if (pRouteDraw->CrossesIDL()) {
				geo::BoundingBox test_box1 = pRouteDraw->RBBox;
				test_box1.Translate(-360.0, 0.0);
				if (b_run)
					test_box1.Expand(nav.pos.lon(), nav.pos.lat());

				if (BltBBox.Intersect(test_box1, 0) != geo::BoundingBox::_OUT) {
					// Route is not wholly outside window
					b_drawn = true;
					if ((pRouteDraw != active_route) && (pRouteDraw != active_track))
						pRouteDraw->Draw(dc, GetVP());
				}
			}

			// Need to quick check for the case where VP crosses IDL
			if (!b_drawn) {
				if ((BltBBox.GetMinX() < -180.0) && (BltBBox.GetMaxX() > -180.0)) {
					geo::BoundingBox test_box2 = pRouteDraw->RBBox;
					test_box2.Translate(-360.0, 0.0);
					if (BltBBox.Intersect(test_box2, 0) != geo::BoundingBox::_OUT) {
						// Route is not wholly outside window
						b_drawn = true;
						if ((pRouteDraw != active_route) && (pRouteDraw != active_track))
							pRouteDraw->Draw(dc, GetVP());
					}
				} else if (!b_drawn && (BltBBox.GetMinX() < 180.0) && (BltBBox.GetMaxX() > 180.0)) {
					geo::BoundingBox test_box3 = pRouteDraw->RBBox;
					test_box3.Translate(360.0, 0.0);
					if (BltBBox.Intersect(test_box3, 0) != geo::BoundingBox::_OUT) {
						// Route is not wholly outside window
						b_drawn = true;
						if ((pRouteDraw != active_route) && (pRouteDraw != active_track))
							pRouteDraw->Draw(dc, GetVP());
					}
				}
			}
		}
	}

	//  Draw any active or selected route (or track) last, so that is is always on top
	if (active_route)
		active_route->Draw(dc, GetVP());
	if (active_track)
		active_track->Draw(dc, GetVP());
}

void ChartCanvas::DrawAllWaypointsInBBox(ocpnDC& dc, const geo::LatLonBoundingBox& BltBBox,
										 const wxRegion& clipregion, bool bDrawMarksOnly)
{
	wxDC* pdc = dc.GetDC();
	if (pdc) {
		wxDCClipper(*pdc, clipregion);
	}

	for (RoutePointList::iterator i = pWayPointMan->waypoints().begin();
		 i != pWayPointMan->waypoints().end(); ++i) {
		RoutePoint* point = *i;
		if (point) {
			if (bDrawMarksOnly && (point->m_bIsInRoute || point->is_in_track())) {
				continue;
			} else {
				if (BltBBox.GetValid()) {
					if (BltBBox.PointInBox(point->latitude(), point->longitude(), 0))
						point->Draw(dc, NULL);
				}
			}
		}
	}

	// draw anchor watch rings, if activated

	if (pAnchorWatchPoint1 || pAnchorWatchPoint2) {
		wxPoint r1;
		wxPoint r2;
		wxPoint lAnchorPoint1;
		wxPoint lAnchorPoint2;
		double lpp1 = 0.0;
		double lpp2 = 0.0;
		if (pAnchorWatchPoint1) {
			lpp1 = GetAnchorWatchRadiusPixels(pAnchorWatchPoint1);
			lAnchorPoint1 = GetCanvasPointPix(pAnchorWatchPoint1->get_position());
		}
		if (pAnchorWatchPoint2) {
			lpp2 = GetAnchorWatchRadiusPixels(pAnchorWatchPoint2);
			lAnchorPoint2 = GetCanvasPointPix(pAnchorWatchPoint2->get_position());
		}

		const global::ColorManager& colors = global::OCPN::get().color();

		wxPen ppPeng(colors.get_color(_T("UGREN")), 2);
		wxPen ppPenr(colors.get_color(_T("URED")), 2);

		wxBrush* ppBrush = wxTheBrushList->FindOrCreateBrush(wxColour(0, 0, 0), wxTRANSPARENT);
		dc.SetBrush(*ppBrush);

		if (lpp1 > 0) {
			dc.SetPen(ppPeng);
			dc.StrokeCircle(lAnchorPoint1.x, lAnchorPoint1.y, fabs(lpp1));
		}

		if (lpp2 > 0) {
			dc.SetPen(ppPeng);
			dc.StrokeCircle(lAnchorPoint2.x, lAnchorPoint2.y, fabs(lpp2));
		}

		if (lpp1 < 0) {
			dc.SetPen(ppPenr);
			dc.StrokeCircle(lAnchorPoint1.x, lAnchorPoint1.y, fabs(lpp1));
		}

		if (lpp2 < 0) {
			dc.SetPen(ppPenr);
			dc.StrokeCircle(lAnchorPoint2.x, lAnchorPoint2.y, fabs(lpp2));
		}
	}
}

double ChartCanvas::GetAnchorWatchRadiusPixels(RoutePoint* pAnchorWatchPoint)
{
	double lpp = 0.0;

	if (pAnchorWatchPoint) {
		const global::Navigation::Anchor& anchor = global::OCPN::get().nav().anchor();

		double d1 = 0.0;
		pAnchorWatchPoint->GetName().ToDouble(&d1);
		d1 = navigation::AnchorDistFix(d1, anchor.PointMinDist, anchor.AWMax);
		double dabs = fabs(d1 / 1852.0);
		geo::Position t = geo::ll_gc_ll(pAnchorWatchPoint->get_position(), 0, dabs);
		wxPoint r1 = GetCanvasPointPix(t);
		wxPoint lAnchorPoint = GetCanvasPointPix(pAnchorWatchPoint->get_position());
		lpp = sqrt(util::sqr((double)(lAnchorPoint.x - r1.x))
				   + util::sqr((double)(lAnchorPoint.y - r1.y)));

		// This is an entry watch
		if (d1 < 0.0)
			lpp = -lpp;
	}
	return lpp;
}

//------------------------------------------------------------------------------------------
//    Tides Support
//------------------------------------------------------------------------------------------

void ChartCanvas::DrawAllTidesInBBox(ocpnDC& dc, const geo::LatLonBoundingBox& BBox, bool bRebuildSelList,
									 bool bforce_redraw_tides, bool bdraw_mono_for_mask)
{
	const global::ColorManager& colors = global::OCPN::get().color();

	wxPen* pblack_pen = wxThePenList->FindOrCreatePen(colors.get_color(_T("UINFD")), 1, wxSOLID);
	wxPen* pyelo_pen = wxThePenList->FindOrCreatePen(colors.get_color(_T("YELO1")), 1, wxSOLID);
	wxPen* pblue_pen = wxThePenList->FindOrCreatePen(colors.get_color(_T("BLUE2")), 1, wxSOLID);

	wxBrush* pgreen_brush
		= wxTheBrushList->FindOrCreateBrush(colors.get_color(_T("GREEN1")), wxSOLID);
	wxBrush* brc_1 = wxTheBrushList->FindOrCreateBrush(colors.get_color(_T("BLUE2")), wxSOLID);
	wxBrush* brc_2 = wxTheBrushList->FindOrCreateBrush(colors.get_color(_T("YELO1")), wxSOLID);

	wxFont* dFont = FontMgr::Get().GetFont(_("ExtendedTideIcon"), 12);
	dc.SetTextForeground(FontMgr::Get().GetFontColor(_("ExtendedTideIcon")));
	int font_size = wxMax(8, dFont->GetPointSize());
	wxFont* plabelFont = wxTheFontList->FindOrCreateFont(font_size, dFont->GetFamily(),
														 dFont->GetStyle(), dFont->GetWeight());

	if (bdraw_mono_for_mask) {
#ifdef __WXX11__
		const wxPen* pmono_pen = wxBLACK_PEN;
		const wxBrush* pmono_brush = wxBLACK_BRUSH;
#else
		const wxPen* pmono_pen = wxWHITE_PEN;
		const wxBrush* pmono_brush = wxWHITE_BRUSH;
#endif

		pblack_pen = (wxPen*)pmono_pen;
		pgreen_brush = (wxBrush*)pmono_brush;
		brc_1 = (wxBrush*)pmono_brush;
		brc_2 = (wxBrush*)pmono_brush;
	}

	dc.SetPen(*pblack_pen);
	dc.SetBrush(*pgreen_brush);

	if (bRebuildSelList)
		pSelectTC->DeleteAllSelectableTypePoints(SelectItem::TYPE_TIDEPOINT);

	wxBitmap bm;
	switch (global::OCPN::get().gui().view().color_scheme) {
		case global::GLOBAL_COLOR_SCHEME_DAY:
			bm = m_bmTideDay;
			break;
		case global::GLOBAL_COLOR_SCHEME_DUSK:
			bm = m_bmTideDusk;
			break;
		case global::GLOBAL_COLOR_SCHEME_NIGHT:
			bm = m_bmTideNight;
			break;
		default:
			bm = m_bmTideDay;
			break;
	}

	int bmw = bm.GetWidth();
	int bmh = bm.GetHeight();

	wxDateTime this_now = wxDateTime::Now();
	time_t t_this_now = this_now.GetTicks();

	double lon_last = 0.0;
	double lat_last = 0.0;
	for (int i = 1; i < ptcmgr->Get_max_IDX() + 1; i++) {
		const tide::IDX_entry* pIDX = ptcmgr->GetIDX_entry(i);

		char type = pIDX->IDX_type; // Entry "TCtcIUu" identifier
		if ((type == 't') || (type == 'T')) { // only Tides
			double lon = pIDX->IDX_lon;
			double lat = pIDX->IDX_lat;
			bool b_inbox = false;
			double nlon;

			if (BBox.PointInBox(lon, lat, 0)) {
				nlon = lon;
				b_inbox = true;
			} else if (BBox.PointInBox(lon + 360.0, lat, 0)) {
				nlon = lon + 360.0;
				b_inbox = true;
			} else if (BBox.PointInBox(lon - 360.0, lat, 0)) {
				nlon = lon - 360.0;
				b_inbox = true;
			}

			// try to eliminate double entry , but the only good way is to clean the file!
			if (b_inbox && (lat != lat_last) && (lon != lon_last)) {

				// Manage the point selection list
				if (bRebuildSelList)
					pSelectTC->AddSelectablePoint(geo::Position(lat, lon), pIDX, SelectItem::TYPE_TIDEPOINT);

				wxPoint r = GetCanvasPointPix(geo::Position(lat, nlon));
				// draw standard icons
				if (GetVP().chart_scale > 500000) {
					if (bdraw_mono_for_mask)
						dc.DrawRectangle(r.x - bmw / 2, r.y - bmh / 2, bmw, bmh);
					else
						dc.DrawBitmap(bm, r.x - bmw / 2, r.y - bmh / 2, true);
				} else {
					// draw "extended" icons
					// set rectangle size and position (max text lengh)
					int wx, hx;
					dc.SetFont(*plabelFont);
					dc.GetTextExtent(_T("99.9ft "), &wx, &hx);
					int w = r.x - 6;
					int h = r.y - 22;
					// draw mask
					if (bdraw_mono_for_mask) {
						dc.DrawRectangle(r.x - (wx / 2), h, wx, hx + 45);
					} else {
						// process tides
						if (bforce_redraw_tides) {
							float val, nowlev;
							float ltleve = 0.0;
							float htleve = 0.0;
							time_t tctime;
							time_t lttime = 0;
							time_t httime = 0;
							bool wt;
							// define if flood or edd in the last ten minutes and verify if data
							// are useable
							if (ptcmgr->GetTideFlowSens(t_this_now, tide::BACKWARD_TEN_MINUTES_STEP,
														pIDX->IDX_rec_num, nowlev, val, wt)) {

								// search forward the first HW or LW near "now" ( starting at
								// "now" - ten minutes )
								ptcmgr->GetHightOrLowTide(t_this_now + tide::BACKWARD_TEN_MINUTES_STEP,
														  tide::FORWARD_TEN_MINUTES_STEP,
														  tide::FORWARD_ONE_MINUTES_STEP, val, wt,
														  pIDX->IDX_rec_num, val, tctime);
								if (wt) {
									httime = tctime;
									htleve = val;
								} else {
									lttime = tctime;
									ltleve = val;
								}
								wt = !wt;

								// then search opposite tide near "now"
								if (tctime > t_this_now) // search backward
									ptcmgr->GetHightOrLowTide(t_this_now, tide::BACKWARD_TEN_MINUTES_STEP,
															  tide::BACKWARD_ONE_MINUTES_STEP, nowlev, wt,
															  pIDX->IDX_rec_num, val, tctime);
								else
									// or search forward
									ptcmgr->GetHightOrLowTide(t_this_now, tide::FORWARD_TEN_MINUTES_STEP,
															  tide::FORWARD_ONE_MINUTES_STEP, nowlev, wt,
															  pIDX->IDX_rec_num, val, tctime);
								if (wt) {
									httime = tctime;
									htleve = val;
								} else {
									lttime = tctime;
									ltleve = val;
								}

								// process tide state  ( %height and flow sens )
								float ts = 1 - ((nowlev - ltleve) / (htleve - ltleve));
								int hs = (httime > lttime) ? -5 : 5;
								if (ts > 0.995 || ts < 0.005)
									hs = 0;
								int ht_y = (int)(45.0 * ts);

								// draw yellow rectangle as total amplitude (width=12 , height=45)
								dc.SetPen(*pblack_pen);
								dc.SetBrush(*brc_2);
								dc.DrawRectangle(w, h, 12, 45);
								// draw blue rectangle as water height
								dc.SetPen(*pblue_pen);
								dc.SetBrush(*brc_1);
								dc.DrawRectangle(w + 2, h + ht_y, 8, 45 - ht_y);

								// draw sens arrows (ensure they are not "under-drawn" by top
								// line of blue rectangle )

								int hl;
								wxPoint arrow[3];
								arrow[0].x = w + 1;
								arrow[1].x = w + 5;
								arrow[2].x = w + 11;
								if (ts > 0.35 || ts < 0.15) { // one arrow at 3/4 hight tide
									hl = (int)(45.0 * 0.25) + h;
									arrow[0].y = hl;
									arrow[1].y = hl + hs;
									arrow[2].y = hl;
									if (ts < 0.15)
										dc.SetPen(*pyelo_pen);
									else
										dc.SetPen(*pblue_pen);

									dc.DrawLines(3, arrow);
								}
								if (ts > 0.60 || ts < 0.40) { // one arrow at 1/2 hight tide
									hl = (int)(45.0 * 0.5) + h;
									arrow[0].y = hl;
									arrow[1].y = hl + hs;
									arrow[2].y = hl;
									if (ts < 0.40)
										dc.SetPen(*pyelo_pen);
									else
										dc.SetPen(*pblue_pen);
									dc.DrawLines(3, arrow);
								}
								if (ts < 0.65 || ts > 0.85) { // one arrow at 1/4 Hight tide
									hl = (int)(45.0 * 0.75) + h;
									arrow[0].y = hl;
									arrow[1].y = hl + hs;
									arrow[2].y = hl;
									if (ts < 0.65)
										dc.SetPen(*pyelo_pen);
									else
										dc.SetPen(*pblue_pen);
									dc.DrawLines(3, arrow);
								}
								// draw tide level text
								wxString s;
								s.Printf(_T("%3.1f"), nowlev);
								tide::Station_Data* pmsd = pIDX->pref_sta_data; // write unit
								if (pmsd)
									s.Append(wxString(pmsd->units_abbrv, wxConvUTF8));
								int wx1;
								dc.GetTextExtent(s, &wx1, NULL);
								dc.DrawText(s, r.x - (wx1 / 2), h + 45);
							}
						}
					}
				}
			}
			lon_last = lon;
			lat_last = lat;
		}
	}
}

//------------------------------------------------------------------------------------------
//    Currents Support
//------------------------------------------------------------------------------------------

void ChartCanvas::DrawAllCurrentsInBBox(ocpnDC& dc, const geo::LatLonBoundingBox& BBox,
										bool bRebuildSelList, bool bforce_redraw_currents,
										bool bdraw_mono_for_mask)
{
	float tcvalue;
	float dir;
	bool bnew_val;
	char sbuf[20];
	wxFont* pTCFont;

	const global::ColorManager& colors = global::OCPN::get().color();
	const global::GUI::View& view = global::OCPN::get().gui().view();

	wxPen* pblack_pen = wxThePenList->FindOrCreatePen(colors.get_color(_T("UINFD")), 1, wxSOLID);
	wxPen* porange_pen = wxThePenList->FindOrCreatePen(colors.get_color(_T("UINFO")), 1, wxSOLID);
	wxBrush* porange_brush
		= wxTheBrushList->FindOrCreateBrush(colors.get_color(_T("UINFO")), wxSOLID);
	wxBrush* pblack_brush
		= wxTheBrushList->FindOrCreateBrush(colors.get_color(_T("UINFD")), wxSOLID);

	double skew_angle = GetVPRotation();

	if (!global::OCPN::get().nav().get_data().CourseUp && !view.skew_comp)
		skew_angle = GetVPRotation() + GetVPSkew();

	if (bdraw_mono_for_mask) {
#ifdef __WXX11__
		const wxPen* pmono_pen = wxBLACK_PEN;
		const wxBrush* pmono_brush = wxBLACK_BRUSH;
#else
		const wxPen* pmono_pen = wxWHITE_PEN;
		const wxBrush* pmono_brush = wxWHITE_BRUSH;
#endif

		pblack_pen = (wxPen*)pmono_pen;
		porange_pen = (wxPen*)pmono_pen;
		porange_brush = (wxBrush*)pmono_brush;
	}

	pTCFont = wxTheFontList->FindOrCreateFont(12, wxDEFAULT, wxNORMAL, wxBOLD, FALSE,
											  wxString(_T("Eurostile Extended")));
	int now = time(NULL);

	if (bRebuildSelList)
		pSelectTC->DeleteAllSelectableTypePoints(SelectItem::TYPE_CURRENTPOINT);

	geo::Position pos_last(0.0, 0.0);
	for (int i = 1; i < ptcmgr->Get_max_IDX() + 1; i++) {
		const tide::IDX_entry* pIDX = ptcmgr->GetIDX_entry(i);
		geo::Position pos(pIDX->IDX_lat, pIDX->IDX_lon);

		char type = pIDX->IDX_type; // Entry "TCtcIUu" identifier
		if ((type == 'c') || (type == 'C')) {

			// TODO This is a ---HACK---
			// try to avoid double current arrows.  Select the first in the list only
			// Proper fix is to correct the TCDATA index file for depth indication
			bool b_dup = false;
			if ((type == 'c') && (pos == pos_last))
				b_dup = true;

			if (!b_dup && (BBox.PointInBox(pos.lon(), pos.lat(), 0))) {

				// Manage the point selection list
				if (bRebuildSelList)
					pSelectTC->AddSelectablePoint(pos, pIDX, SelectItem::TYPE_CURRENTPOINT);

				wxPoint r = GetCanvasPointPix(pos);

				wxPoint d[4];
				int dd = 6;
				d[0].x = r.x;
				d[0].y = r.y + dd;
				d[1].x = r.x + dd;
				d[1].y = r.y;
				d[2].x = r.x;
				d[2].y = r.y - dd;
				d[3].x = r.x - dd;
				d[3].y = r.y;

				if (ptcmgr->GetTideOrCurrent15(now, i, tcvalue, dir, bnew_val)) {
					porange_pen->SetWidth(1);
					dc.SetPen(*pblack_pen);
					dc.SetBrush(*porange_brush);
					dc.DrawPolygon(4, d);

					if (type == 'C') {
						dc.SetBrush(*pblack_brush);
						dc.DrawCircle(r.x, r.y, 2);
					} else if ((type == 'c') && (GetVP().chart_scale < 1000000)) {
						if (bnew_val || bforce_redraw_currents) {

							// Get the display pixel location of the current station
							wxPoint cpoint = GetCanvasPointPix(pos);
							int pixxc = cpoint.x;
							int pixyc = cpoint.y;

							// Draw arrow using preset parameters, see mm_per_knot variable
							// double scale = fabs ( tcvalue ) * current_draw_scaler;
							// Adjust drawing size using logarithmic scale
							double a1 = fabs(tcvalue) * 10.0;
							a1 = wxMax(1.0, a1); // Current values less than 0.1 knot
							// will be displayed as 0
							double a2 = log10(a1);

							double scale = current_draw_scaler * a2;

							porange_pen->SetWidth(2);
							dc.SetPen(*porange_pen);
							DrawArrow(dc, pixxc, pixyc, dir - 90 + (skew_angle * 180.0 / M_PI),
									  scale / 100);

							// Draw text
							dc.SetFont(*pTCFont);
							snprintf(sbuf, 19, "%3.1f", fabs(tcvalue));
							dc.DrawText(wxString(sbuf, wxConvUTF8), pixxc, pixyc);
						}
					}
				}
			}
			pos_last = pos;
		}
	}
}

void ChartCanvas::DrawTCWindow(int x, int y, tide::IDX_entry* pvIDX)
{
	pCwin = new TCWin(this, x, y, pvIDX);
}

#define NUM_CURRENT_ARROW_POINTS 9
static wxPoint CurrentArrowArray[NUM_CURRENT_ARROW_POINTS]
	= { wxPoint(0, 0),   wxPoint(0, -10), wxPoint(55, -10), wxPoint(55, -25), wxPoint(100, 0),
		wxPoint(55, 25), wxPoint(55, 10), wxPoint(0, 10),   wxPoint(0, 0) };

void ChartCanvas::DrawArrow(ocpnDC& dc, int x, int y, double rot_angle, double scale)
{
	if (scale > 1e-2) {

		float sin_rot = sin(rot_angle * M_PI / 180.0);
		float cos_rot = cos(rot_angle * M_PI / 180.0);

		// Move to the first point

		float xt = CurrentArrowArray[0].x;
		float yt = CurrentArrowArray[0].y;

		float xp = (xt * cos_rot) - (yt * sin_rot);
		float yp = (xt * sin_rot) + (yt * cos_rot);
		int x1 = (int)(xp * scale);
		int y1 = (int)(yp * scale);

		// Walk thru the point list
		for (int ip = 1; ip < NUM_CURRENT_ARROW_POINTS; ip++) {
			xt = CurrentArrowArray[ip].x;
			yt = CurrentArrowArray[ip].y;

			float xp = (xt * cos_rot) - (yt * sin_rot);
			float yp = (xt * sin_rot) + (yt * cos_rot);
			int x2 = (int)(xp * scale);
			int y2 = (int)(yp * scale);

			dc.DrawLine(x1 + x, y1 + y, x2 + x, y2 + y);

			x1 = x2;
			y1 = y2;
		}
	}
}

wxString ChartCanvas::FindValidUploadPort()
{
	const global::System::Config& sys = global::OCPN::get().sys().config();

	wxString port;
	// Try to use the saved persistent upload port first
	if (!sys.uploadConnection.IsEmpty() && sys.uploadConnection.StartsWith(_T("Serial"))) {
		port = sys.uploadConnection;
	} else if (g_pConnectionParams) {
		// If there is no persistent upload port recorded (yet)
		// then use the first available serial connection which has output defined.
		for (ArrayOfConnPrm::const_iterator i = g_pConnectionParams->begin();
			 i != g_pConnectionParams->end(); ++i) {
			if (i->isOutput() && i->getType() == ConnectionParams::SERIAL) {
				port << _T("Serial:") << i->getPort();
			}
		}
	}

	return port;
}

void ShowAISTargetQueryDialog(wxWindow* win, int mmsi)
{
	if (!win)
		return;

	if (NULL == g_pais_query_dialog_active) {
		wxPoint pos = global::OCPN::get().gui().ais_query_dialog().position;

		if (g_pais_query_dialog_active) {
			delete g_pais_query_dialog_active;
			g_pais_query_dialog_active = new ais::AISTargetQueryDialog;
		} else {
			g_pais_query_dialog_active = new ais::AISTargetQueryDialog;
		}

		g_pais_query_dialog_active->Create(win, -1, _("AIS Target Query"), pos);

		g_pais_query_dialog_active->SetMMSI(mmsi);
		g_pais_query_dialog_active->UpdateText();
		wxSize sz = g_pais_query_dialog_active->GetSize();

		bool b_reset_pos = false;
#ifdef __WXMSW__
		//  Support MultiMonitor setups which an allow negative window positions.
		//  If the requested window title bar does not intersect any installed monitor,
		//  then default to simple primary monitor positioning.
		RECT frame_title_rect;
		frame_title_rect.left = pos.x;
		frame_title_rect.top = pos.y;
		frame_title_rect.right = pos.x + sz.x;
		frame_title_rect.bottom = pos.y + 30;

		if (NULL == MonitorFromRect(&frame_title_rect, MONITOR_DEFAULTTONULL))
			b_reset_pos = true;
#else

		//    Make sure drag bar (title bar) of window intersects wxClient Area of screen, with a
		// little slop...
		wxRect window_title_rect; // conservative estimate
		window_title_rect.x = pos.x;
		window_title_rect.y = pos.y;
		window_title_rect.width = sz.x;
		window_title_rect.height = 30;

		wxRect ClientRect = wxGetClientDisplayRect();
		ClientRect.Deflate(60, 60); // Prevent the new window from being too close to the edge
		if (!ClientRect.Intersects(window_title_rect))
			b_reset_pos = true;

#endif

		if (b_reset_pos)
			g_pais_query_dialog_active->Move(50, 200);

	} else {
		g_pais_query_dialog_active->SetMMSI(mmsi);
		g_pais_query_dialog_active->UpdateText();
	}

	g_pais_query_dialog_active->Show();
}

#ifdef __WXGTK__
	#define BRIGHT_XCALIB
	#define __OPCPN_USEICC__
#endif

//--------------------------------------------------------------------------------------------------------
//    Screen Brightness Control Support Routines
//
//--------------------------------------------------------------------------------------------------------

#ifdef __OPCPN_USEICC__
int CreateSimpleICCProfileFile(const char* file_name, double co_red, double co_green,
							   double co_blue);

wxString temp_file_name;
#endif

#ifdef __WIN32__
#include <windows.h>

HMODULE hGDI32DLL;
typedef BOOL(WINAPI* SetDeviceGammaRamp_ptr_type)(HDC hDC, LPVOID lpRampTable);
typedef BOOL(WINAPI* GetDeviceGammaRamp_ptr_type)(HDC hDC, LPVOID lpRampTable);
SetDeviceGammaRamp_ptr_type g_pSetDeviceGammaRamp; // the API entry points in the dll
GetDeviceGammaRamp_ptr_type g_pGetDeviceGammaRamp;

WORD* g_pSavedGammaMap;

#endif

static int InitScreenBrightness(void)
{
#ifdef __WIN32__
	if (global::OCPN::get().gui().view().opengl) {
		HDC hDC;
		BOOL bbr;

		if (NULL == hGDI32DLL) {
			hGDI32DLL = LoadLibrary(TEXT("gdi32.dll"));

			if (NULL != hGDI32DLL) {
				// Get the entry points of the required functions
				g_pSetDeviceGammaRamp
					= (SetDeviceGammaRamp_ptr_type)GetProcAddress(hGDI32DLL, "SetDeviceGammaRamp");
				g_pGetDeviceGammaRamp
					= (GetDeviceGammaRamp_ptr_type)GetProcAddress(hGDI32DLL, "GetDeviceGammaRamp");

				//    If the functions are not found, unload the DLL and return false
				if ((NULL == g_pSetDeviceGammaRamp) || (NULL == g_pGetDeviceGammaRamp)) {
					FreeLibrary(hGDI32DLL);
					hGDI32DLL = NULL;
					return 0;
				}
			}
		}

		// Interface is ready, so....
		// Get some storage
		if (!g_pSavedGammaMap) {
			g_pSavedGammaMap = (WORD*)malloc(3 * 256 * sizeof(WORD));

			hDC = GetDC(NULL); // Get the full screen DC
			bbr = g_pGetDeviceGammaRamp(hDC, g_pSavedGammaMap); // Get the existing ramp table
			ReleaseDC(NULL, hDC); // Release the DC
		}

		// On Windows hosts, try to adjust the registry to allow full range setting of Gamma
		// table
		// This is an undocumented Windows hack.....
		wxRegKey* pRegKey = new wxRegKey(
			_T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ICM"));
		if (!pRegKey->Exists())
			pRegKey->Create();
		pRegKey->SetValue(_T("GdiIcmGammaRange"), 256);

		g_brightness_init = true;
		return 1;
	} else {
		if (NULL == g_pcurtain) {
			if (gFrame->CanSetTransparent()) {
				//    Build the curtain window
				g_pcurtain = new wxDialog(cc1, -1, _T(""), wxPoint(0, 0), ::wxGetDisplaySize(),
										  wxNO_BORDER | wxTRANSPARENT_WINDOW | wxSTAY_ON_TOP
										  | wxDIALOG_NO_PARENT);

				//                  g_pcurtain = new ocpnCurtain(gFrame,
				// wxPoint(0,0),::wxGetDisplaySize(),
				//                      wxNO_BORDER | wxTRANSPARENT_WINDOW |wxSTAY_ON_TOP |
				// wxDIALOG_NO_PARENT);

				g_pcurtain->Hide();

				HWND hWnd = GetHwndOf(g_pcurtain);
				SetWindowLong(hWnd, GWL_EXSTYLE,
							  GetWindowLong(hWnd, GWL_EXSTYLE) | ~WS_EX_APPWINDOW);
				g_pcurtain->SetBackgroundColour(wxColour(0, 0, 0));
				g_pcurtain->SetTransparent(0);

				g_pcurtain->Maximize();
				g_pcurtain->Show();

				//    All of this is obtuse, but necessary for Windows...
				g_pcurtain->Enable();
				g_pcurtain->Disable();

				gFrame->Disable();
				gFrame->Enable();
				cc1->SetFocus();
			}
		}
		g_brightness_init = true;

		return 1;
	}
#else
	// Look for "xcalib" application
	wxString cmd(_T ( "xcalib -version" ));

	wxArrayString output;
	long r = wxExecute(cmd, output);
	if (0 != r)
		wxLogMessage(
			_("   External application \"xcalib\" not found. Screen brightness not changed."));

	g_brightness_init = true;
	return 0;
#endif
}

int RestoreScreenBrightness(void)
{
#ifdef __WIN32__

	if (g_pSavedGammaMap) {
		HDC hDC = GetDC(NULL); // Get the full screen DC
		g_pSetDeviceGammaRamp(hDC, g_pSavedGammaMap); // Restore the saved ramp table
		ReleaseDC(NULL, hDC); // Release the DC

		free(g_pSavedGammaMap);
		g_pSavedGammaMap = NULL;
	}

	if (g_pcurtain) {
		g_pcurtain->Close();
		g_pcurtain->Destroy();
		g_pcurtain = NULL;
	}

	g_brightness_init = false;
	return 1;

#endif

#ifdef BRIGHT_XCALIB
	if (g_brightness_init) {
		wxString cmd;
		cmd = _T("xcalib -clear");
		wxExecute(cmd, wxEXEC_ASYNC);
		g_brightness_init = false;
	}

	return 1;
#endif

	return 0;
}

// Set brightness. [0..100]
static int SetScreenBrightness(int brightness)
{
#ifdef __WIN32__

	// Under Windows, we use the SetDeviceGammaRamp function which exists in some (most modern?)
	// versions of gdi32.dll
	// Load the required library dll, if not already in place
	if (global::OCPN::get().gui().view().opengl) {
		if (g_pcurtain) {
			g_pcurtain->Close();
			g_pcurtain->Destroy();
			g_pcurtain = NULL;
		}

		InitScreenBrightness();

		if (NULL == hGDI32DLL) {
			// Unicode stuff.....
			wchar_t wdll_name[80];
			MultiByteToWideChar(0, 0, "gdi32.dll", -1, wdll_name, 80);
			LPCWSTR cstr = wdll_name;

			hGDI32DLL = LoadLibrary(cstr);

			if (NULL != hGDI32DLL) {
				// Get the entry points of the required functions
				g_pSetDeviceGammaRamp
					= (SetDeviceGammaRamp_ptr_type)GetProcAddress(hGDI32DLL, "SetDeviceGammaRamp");
				g_pGetDeviceGammaRamp
					= (GetDeviceGammaRamp_ptr_type)GetProcAddress(hGDI32DLL, "GetDeviceGammaRamp");

				//    If the functions are not found, unload the DLL and return false
				if ((NULL == g_pSetDeviceGammaRamp) || (NULL == g_pGetDeviceGammaRamp)) {
					FreeLibrary(hGDI32DLL);
					hGDI32DLL = NULL;
					return 0;
				}
			}
		}

		HDC hDC = GetDC(NULL); // Get the full screen DC

		/*
		   int cmcap = GetDeviceCaps(hDC, COLORMGMTCAPS);
		   if (cmcap != CM_GAMMA_RAMP)
		   {
		   wxLogMessage(_T("    Video hardware does not support brightness control by gamma ramp
		   adjustment."));
		   return false;
		   }
		 */

		int increment = brightness * 256 / 100;

		// Build the Gamma Ramp table
		WORD GammaTable[3][256];

		int table_val = 0;
		for (int i = 0; i < 256; i++) {

			GammaTable[0][i] = r_gamma_mult * (WORD)table_val;
			GammaTable[1][i] = g_gamma_mult * (WORD)table_val;
			GammaTable[2][i] = b_gamma_mult * (WORD)table_val;

			table_val += increment;

			if (table_val > 65535)
				table_val = 65535;
		}

		g_pSetDeviceGammaRamp(hDC, GammaTable); // Set the ramp table
		ReleaseDC(NULL, hDC); // Release the DC

		return 1;
	} else {
		if (g_pSavedGammaMap) {
			HDC hDC = GetDC(NULL); // Get the full screen DC
			g_pSetDeviceGammaRamp(hDC, g_pSavedGammaMap); // Restore the saved ramp table
			ReleaseDC(NULL, hDC); // Release the DC
		}

		if (NULL == g_pcurtain)
			InitScreenBrightness();

		if (g_pcurtain) {
			int sbrite = wxMax(1, brightness);
			sbrite = wxMin(100, sbrite);

			g_pcurtain->SetTransparent((100 - sbrite) * 256 / 100);
		}
		return 1;
	}

#endif

#ifdef BRIGHT_XCALIB
	if (!g_brightness_init) {
		last_brightness = 100;
		g_brightness_init = true;
		temp_file_name = wxFileName::CreateTempFileName(_T(""));
		InitScreenBrightness();
	}

#ifdef __OPCPN_USEICC__
	//  Create a dead simple temporary ICC profile file, with gamma ramps set as desired,
	//  and then activate this temporary profile using xcalib <filename>
	if (!CreateSimpleICCProfileFile((const char*)temp_file_name.fn_str(), brightness * r_gamma_mult,
									brightness * g_gamma_mult, brightness * b_gamma_mult)) {
		wxString cmd(_T ( "xcalib " ));
		cmd += temp_file_name;

		wxExecute(cmd, wxEXEC_ASYNC);
	}

#else
	//    Or, use "xcalib -co" to set overall contrast value
	//    This is not as nice, since the -co parameter wants to be a fraction of the current
	// contrast,
	//    and values greater than 100 are not allowed.  As a result, increases of contrast must do a
	// "-clear" step
	//    first, which produces objectionable flashing.
	if (brightness > last_brightness) {
		wxString cmd;
		cmd = _T("xcalib -clear");
		wxExecute(cmd, wxEXEC_ASYNC);

		::wxMilliSleep(10);

		int brite_adj = wxMax(1, brightness);
		cmd.Printf(_T("xcalib -co %2d -a"), brite_adj);
		wxExecute(cmd, wxEXEC_ASYNC);
	} else {
		int brite_adj = wxMax(1, brightness);
		int factor = (brite_adj * 100) / last_brightness;
		factor = wxMax(1, factor);
		wxString cmd;
		cmd.Printf(_T("xcalib -co %2d -a"), factor);
		wxExecute(cmd, wxEXEC_ASYNC);
	}

#endif

	last_brightness = brightness;

#endif

	return 0;
}

#ifdef __OPCPN_USEICC__

#define MLUT_TAG 0x6d4c5554L
#define VCGT_TAG 0x76636774L

int GetIntEndian(unsigned char* s)
{
	int ret;
	unsigned char* p;
	int i;

	p = (unsigned char*)&ret;

	if (1)
		for (i = sizeof(int) - 1; i > -1; --i)
			*p++ = s[i];
	else
		for (i = 0; i < (int)sizeof(int); ++i)
			*p++ = s[i];

	return ret;
}

unsigned short GetShortEndian(unsigned char* s)
{
	unsigned short ret;
	unsigned char* p;
	int i;

	p = (unsigned char*)&ret;

	if (1)
		for (i = sizeof(unsigned short) - 1; i > -1; --i)
			*p++ = s[i];
	else
		for (i = 0; i < (int)sizeof(unsigned short); ++i)
			*p++ = s[i];

	return ret;
}

// Create a very simple Gamma correction file readable by xcalib
int CreateSimpleICCProfileFile(const char* file_name, double co_red, double co_green,
							   double co_blue)
{
	FILE* fp;

	if (file_name) {
		fp = fopen(file_name, "wb");
		if (!fp)
			return -1; /* file can not be created */
	} else
		return -1; /* filename char pointer not valid */

	//    Write header
	char header[128];
	for (int i = 0; i < 128; i++)
		header[i] = 0;

	fwrite(header, 128, 1, fp);

	//    Num tags
	int numTags0 = 1;
	int numTags = GetIntEndian((unsigned char*)&numTags0);
	fwrite(&numTags, 1, 4, fp);

	int tagName0 = VCGT_TAG;
	int tagName = GetIntEndian((unsigned char*)&tagName0);
	fwrite(&tagName, 1, 4, fp);

	int tagOffset0 = 128 + 4 * sizeof(int);
	int tagOffset = GetIntEndian((unsigned char*)&tagOffset0);
	fwrite(&tagOffset, 1, 4, fp);

	int tagSize0 = 1;
	int tagSize = GetIntEndian((unsigned char*)&tagSize0);
	fwrite(&tagSize, 1, 4, fp);

	fwrite(&tagName, 1, 4, fp); // another copy of tag

	fwrite(&tagName, 1, 4, fp); // dummy

	//  Table type

	/* VideoCardGammaTable (The simplest type) */
	int gammatype0 = 0;
	int gammatype = GetIntEndian((unsigned char*)&gammatype0);
	fwrite(&gammatype, 1, 4, fp);

	int numChannels0 = 3;
	unsigned short numChannels = GetShortEndian((unsigned char*)&numChannels0);
	fwrite(&numChannels, 1, 2, fp);

	int numEntries0 = 256;
	unsigned short numEntries = GetShortEndian((unsigned char*)&numEntries0);
	fwrite(&numEntries, 1, 2, fp);

	int entrySize0 = 1;
	unsigned short entrySize = GetShortEndian((unsigned char*)&entrySize0);
	fwrite(&entrySize, 1, 2, fp);

	unsigned char ramp[256];

	//    Red ramp
	for (int i = 0; i < 256; i++)
		ramp[i] = i * co_red / 100.;
	fwrite(ramp, 256, 1, fp);

	//    Green ramp
	for (int i = 0; i < 256; i++)
		ramp[i] = i * co_green / 100.;
	fwrite(ramp, 256, 1, fp);

	//    Blue ramp
	for (int i = 0; i < 256; i++)
		ramp[i] = i * co_blue / 100.;
	fwrite(ramp, 256, 1, fp);

	fclose(fp);

	return 0;
}
#endif // __OPCPN_USEICC__

