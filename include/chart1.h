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

#ifndef __CHART1_H__
#define __CHART1_H__

#include <wx/wx.h>
#include <wx/toolbar.h>
#include <wx/timer.h>
#include <wx/print.h>
#include <wx/datetime.h>
#include <wx/cmdline.h>
#include <wx/snglinst.h>

#ifdef __WXMSW__
#include "wx/msw/private.h"
#endif

#include "nmea0183.h"
#include "ChartDirInfo.h"
#include "NMEA_Msg_Container.h"
#include <chart/ChartType.h>
#include <chart/ChartFamily.h>
#include "ColorScheme.h"

#ifdef USE_S57
#include "cpl_error.h"

// Global Static error reporting function
extern "C" void MyCPLErrorHandler(CPLErr eErrClass, int nError, const char * pszErrorMsg);
#endif

wxArrayString * EnumerateSerialPorts(void);
wxColour GetGlobalColor(wxString colorName);

int GetApplicationMemoryUse(void);

// The point for anchor watch should really be a class...
double AnchorDistFix(double const d, double const AnchorPointMinDist, double const AnchorPointMaxDist);   //  pjotrc 2010.02.22

class OCPN_NMEAEvent;
class ChartCanvas;
class ocpnFloatingToolbarDialog;
class OCPN_MsgEvent;
class options;
class Track;
class ViewPort;

#define TIMER_GFRAME_1 999

enum
{
	ID_ZOOMIN = 1550,
	ID_ZOOMOUT,
	ID_STKUP,
	ID_STKDN,
	ID_ROUTE,
	ID_FOLLOW,
	ID_SETTINGS,
	ID_AIS,           // pjotrc 2010.02.09
	ID_TEXT,
	ID_CURRENT,
	ID_TIDE,
	ID_HELP,
	ID_TBEXIT,
	ID_TBSTAT,
	ID_PRINT,
	ID_COLSCHEME,
	ID_ROUTEMANAGER,
	ID_TRACK,
	ID_TBSTATBOX,
	ID_MOB,
	ID_PLUGIN_BASE

};


// Define a constant GPS signal watchdog timeout value
#define GPS_TIMEOUT_SECONDS  6

#define MAX_COG_AVERAGE_SECONDS        60
#define MAX_COGSOG_FILTER_SECONDS      60

class ChartBase;
class wxSocketEvent;
class ToolBarSimple;
class OCPN_DataStreamEvent;
class DataStream;

class MyFrame : public wxFrame
{
		DECLARE_EVENT_TABLE()

	public:
		MyFrame(
				wxFrame * frame,
				const wxString & title,
				const wxPoint & pos,
				const wxSize & size,
				long style);

		virtual ~MyFrame();

		int GetApplicationMemoryUse(void);

		void OnEraseBackground(wxEraseEvent& event);
		void OnActivate(wxActivateEvent& event);
		void OnMaximize(wxMaximizeEvent& event);
		void OnCloseWindow(wxCloseEvent& event);
		void OnExit(wxCommandEvent& event);
		void OnSize(wxSizeEvent& event);
		void OnMove(wxMoveEvent& event);
		void OnFrameTimer1(wxTimerEvent& event);
		bool DoChartUpdate(void);
		void OnEvtTHREADMSG(wxCommandEvent& event);
		void OnEvtOCPN_NMEA(OCPN_DataStreamEvent & event);
		void OnEvtPlugInMessage(OCPN_MsgEvent & event);
		void OnMemFootTimer(wxTimerEvent& event);

		void UpdateAllFonts(void);
		void PositionConsole(void);
		void OnToolLeftClick(wxCommandEvent& event);
		void ClearRouteTool();
		void DoStackUp(void);
		void DoStackDown(void);

		void MouseEvent(wxMouseEvent& event);
		void SelectChartFromStack(
				int index,
				bool bDir = false,
				ChartTypeEnum New_Type = CHART_TYPE_DONTCARE,
				ChartFamilyEnum New_Family = CHART_FAMILY_DONTCARE);
		void SelectdbChart(int dbindex);
		void SelectQuiltRefChart(int selected_index);
		void SelectQuiltRefdbChart(int db_index);

		void JumpToPosition(double lat, double lon, double scale);

		void ProcessCanvasResize(void);

		void ApplyGlobalSettings(bool bFlyingUpdate, bool bnewtoolbar);
		void SetChartThumbnail(int index);
		int DoOptionsDialog();
		int ProcessOptionsDialog(int resultFlags , options* dialog);
		void DoPrint(void);
		void StopSockets(void);
		void ResumeSockets(void);
		void TogglebFollow(void);
		void ToggleFullScreen();
		void SetbFollow(void);
		void ClearbFollow(void);
		void ToggleChartOutlines(void);
		void ToggleENCText(void);
		void ToggleSoundings(void);
		void ToggleRocks(void);
		bool ToggleLights(bool doToggle = true, bool temporary = false);
		void ToggleAnchor(void);
		void TrackOn(void);
		Track *TrackOff(bool do_add_point = false);
		void TrackMidnightRestart(void);
		void ToggleColorScheme();
		int GetnChartStack(void);
		void SetToolbarItemState(int tool_id, bool state);
		void SetToolbarItemBitmaps(int tool_id, wxBitmap *bitmap, wxBitmap *bmpDisabled);
		void ToggleQuiltMode(void);
		void ToggleCourseUp(void);
		void SetQuiltMode(bool bquilt);
		bool GetQuiltMode(void);
		void UpdateControlBar(void);
		void RemoveChartFromQuilt(int dbIndex);

		void SubmergeToolbar(void);
		void SubmergeToolbarIfOverlap(int x, int y, int margin = 0);
		void SurfaceToolbar(void);

		void HandlePianoClick(int selected_index, int selected_dbIndex);
		void HandlePianoRClick(int x, int y,int selected_index, int selected_dbIndex);
		void HandlePianoRollover(int selected_index, int selected_dbIndex);
		void HandlePianoRolloverIcon(int selected_index, int selected_dbIndex);

		void PianoPopupMenu(int x, int y, int selected_index, int selected_dbIndex);
		void OnPianoMenuDisableChart(wxCommandEvent& event);
		void OnPianoMenuEnableChart(wxCommandEvent& event);

		void SetGroupIndex(int index);

		double GetBestVPScale(ChartBase *pchart);

		ChartCanvas *GetCanvasWindow();
		void SetCanvasWindow(ChartCanvas *pcanv);

		ColorScheme GetColorScheme();
		void SetAndApplyColorScheme(ColorScheme cs);

		void OnFrameTCTimer(wxTimerEvent& event);
		void OnFrameCOGTimer(wxTimerEvent& event);
		void SetupQuiltMode(void);

		void ChartsRefresh(int dbi_hint, ViewPort &vp, bool b_purge = true);

		bool CheckGroup(int igroup);

		void TouchAISActive(void);
		void UpdateAISTool(void);

		static wxString prepare_logbook_message(const wxDateTime &);
		static wxString get_cog();
		static wxString get_sog();

		bool hasStatusBar() const;
		int nRoute_State;
		int nBlinkerTick;

		wxTimer FrameTimer1; // FIXME: attribute must be private
		wxTimer FrameCOGTimer; // FIXME: attribute must be private

		// PlugIn support
		int GetNextToolbarToolId();
		void RequestNewToolbarArgEvent(wxCommandEvent &);
		void RequestNewToolbar();

		void ActivateMOB(void);
		void UpdateGPSCompassStatusBox(bool b_force_new = false);
		bool UpdateChartDatabaseInplace(
				ArrayOfCDI &DirArray,
				bool b_force,
				bool b_prog,
				const wxString & ChartListFileName);

		bool m_bdefer_resize;
		wxSize m_defer_size;

		void performUniChromeOpenGLResizeHack();

	private:
		void ODoSetSize(void);
		void DoCOGSet(void);

		// Toolbar support
		ToolBarSimple *CreateAToolbar();
		void DestroyMyToolbar();
		void UpdateToolbar(ColorScheme cs);

		void EnableToolbar(bool newstate);

		bool CheckAndAddPlugInTool(ToolBarSimple *tb);
		bool AddDefaultPositionPlugInTools(ToolBarSimple *tb);
		void FilterCogSog(void);
		void SetChartUpdatePeriod(ViewPort &vp);

		void ApplyGlobalColorSchemetoStatusBar(void);
		void PostProcessNNEA(bool pos_valid, const wxString &sfixtime);

		void ScrubGroupArray();
		wxString GetGroupName(int igroup);
		void LoadHarmonics();

		bool EvalPriority(const wxString & message, DataStream * pDS);

		int m_StatusBarFieldCount;

		ChartCanvas * m_pchart_canvas;

		NMEA0183 m_NMEA0183; // Used to parse messages from NMEA threads

		wxDateTime m_MMEAeventTime;
		unsigned long m_ulLastNEMATicktime;

		wxMutex m_mutexNMEAEvent;         // Mutex to handle static data from NMEA threads

		wxString m_last_reported_chart_name;
		wxString m_last_reported_chart_pubdate;

		double COGTable[MAX_COG_AVERAGE_SECONDS];

		wxString m_lastAISiconName;

		bool m_toolbar_scale_tools_shown;

		// Plugin Support
		int m_next_available_plugin_tool_id;

		double m_COGFilterLast;
		double COGFilterTable[MAX_COGSOG_FILTER_SECONDS];
		double SOGFilterTable[MAX_COGSOG_FILTER_SECONDS];

		bool m_bpersistent_quilt;
		int m_ChartUpdatePeriod;
		bool m_last_bGPSValid;

		wxString prev_locale;
		bool bPrevQuilt;
		bool bPrevFullScreenQuilt;
		bool bPrevOGL;

		MsgPriorityHash NMEA_Msg_Hash;
		wxString m_VDO_accumulator;

		time_t m_fixtime;
		wxStatusBar * m_pStatusBar;
		bool m_bTimeIsSet;
		wxTimer FrameTCTimer;
		wxTimer MemFootTimer;
};

// A global definition for window, timer and other ID's as needed.
enum
{
	ID_NMEA_WINDOW = wxID_HIGHEST,
	ID_AIS_WINDOW,
	FRAME_TIMER_1,
	FRAME_TIMER_2,
	TIMER_AIS1,
	TIMER_AISAUDIO,
	AIS_SOCKET_ID,
	FRAME_TIMER_DOG,
	FRAME_TC_TIMER,
	FRAME_COG_TIMER,
	MEMORY_FOOTPRINT_TIMER,
	ID_NMEA_THREADMSG
};

#endif
