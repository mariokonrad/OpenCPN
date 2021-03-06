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
#include <Select.h>
#include <OCPNFloatingToolbarDialog.h>
#include <MessageBox.h>
#include <Multiplexer.h>
#include <FloatingCompassWindow.h>
#include <ThumbWin.h>
#include <StatWin.h>
#include <PianoWin.h>
#include <ConsoleCanvas.h>
#include <GUI_IDs.h>
#include <LogMessageOnce.h>
#include <MemoryStatus.h>
#include <CM93DSlide.h>
#include <ChartCanvas.h>
#include <Config.h>
#include <UserColors.h>
#include <Layer.h>
#include <OCPN_Version.h>
#include <Routeman.h>
#include <WayPointman.h>
#include <FontMgr.h>

#include <gui/DefaultStyleManager.h>
#include <gui/Style.h>

#include <windows/compatibility.h>

#include <tide/TCMgr.h>

#include <plugin/PlugInManager.h>

#include <navigation/DefaultRouteTracker.h>

#include <chart/ChartStack.h>
#include <chart/ChartDB.h>
#include <chart/ChartDummy.h>
#include <chart/ChartGroup.h>

#include <ais/AIS_Decoder.h>
#include <ais/AISTargetAlertDialog.h>
#include <ais/AISTargetQueryDialog.h>

#include <global/OCPN.h>
#include <global/OCPN_GUI.h>
#include <global/OCPN_Navigation.h>
#include <global/OCPN_AIS.h>
#include <global/OCPN_WatchDog.h>
#include <global/OCPN_System.h>
#include <global/OCPN_Runtime.h>

#ifdef __WXMSW__
	#include <WinConsole.h>
#endif

#ifdef OCPN_USE_CRASHRPT
	#include "CrashRpt.h"
#endif

#ifndef __WXMSW__
	#include <signal.h>
	#include <setjmp.h>
#endif

#include <wx/cmdline.h>
#include <wx/datetime.h>
#include <wx/progdlg.h>
#include <wx/stdpaths.h>
#include <wx/apptrait.h>

#ifdef USE_S57
	#include "S57QueryDialog.h"
	#include "cpl_csv.h"
	#include <chart/S57RegistrarMgr.h>
	#include <chart/s52plib.h>
	#include <chart/CM93OffsetDialog.h>
	#include <chart/S57ClassRegistrar.h>
	#include <chart/S52ColorProvider.h>
#endif

unsigned int malloc_max;

extern void appendOSDirSlash(wxString &);
void RestoreSystemColors();

#ifndef __WXMSW__
extern struct sigaction sa_all;
extern struct sigaction sa_all_old;
void catch_signals(int signo);
#endif

#ifdef USE_S57
extern chart::s52plib* ps52plib;
extern chart::S57ClassRegistrar* g_poRegistrar;
extern chart::S57RegistrarMgr* m_pRegistrarMan;
extern S57QueryDialog* g_pObjectQueryDialog;
extern chart::CM93OffsetDialog* g_pCM93OffsetDialog;
#endif

#ifdef OCPN_USE_PORTAUDIO
namespace sound { extern bool portaudio_initialized; }
#endif

extern StatWin* stats;
extern ConsoleCanvas* console;
ChartCanvas* cc1;
extern RoutePoint* pAnchorWatchPoint1;
extern RoutePoint* pAnchorWatchPoint2;
extern bool bDBUpdateInProgress;
extern ThumbWin* pthumbwin;
extern tide::IDX_entry* gpIDX;
extern chart::ChartDB* ChartData;
extern ais::AISTargetQueryDialog* g_pais_query_dialog_active;
extern wxDateTime g_StartTime;
extern int g_StartTimeTZ;
extern int gpIDXn;
extern PlugInManager* g_pi_manager;
extern chart::ChartGroupArray* g_pGroupArray;
extern wxLocale* plocale_def_lang;
extern wxAuiManager* g_pauimgr;
extern FloatingCompassWindow* g_FloatingCompassDialog;
extern LayerList* pLayerList;
extern chart::ChartStack* pCurrentStack;
extern int g_unit_test_1;
extern OCPNFloatingToolbarDialog* g_FloatingToolbarDialog;
extern Config* pConfig;
extern Select* pSelect;
extern Select* pSelectTC;
extern Select* pSelectAIS;
extern MainFrame* gFrame;
extern bool g_btouch;
extern bool g_bresponsive;
extern double g_pix_per_mm;

#ifdef __WXMSW__
bool TestGLCanvas(wxString& prog_dir)
{
	wxString test_app = prog_dir;
	test_app += _T("ocpn_gltest1.exe");

	if (::wxFileExists(test_app)) {
		long proc_return = ::wxExecute(test_app, wxEXEC_SYNC);
		printf("OpenGL Test Process returned %0X\n", proc_return);
		if (proc_return == 0)
			printf("GLCanvas OK\n");
		else
			printf("GLCanvas failed to start, disabling OpenGL.\n");

		return (proc_return == 0);
	} else
		return true;
}
#endif


#ifdef USE_S57
#include "cpl_error.h"
// Global Static error reporting function
static void OCPN_CPLErrorHandler(CPLErr eErrClass, int nError, const char* pszErrorMsg)
{
	char msg[256];

	if (eErrClass == CE_Debug)
		snprintf(msg, sizeof(msg), "CPL: %s", pszErrorMsg);
	else if (eErrClass == CE_Warning)
		snprintf(msg, sizeof(msg), "CPL Warning %d: %s", nError, pszErrorMsg);
	else
		snprintf(msg, sizeof(msg), "CPL ERROR %d: %s", nError, pszErrorMsg);

	wxString str(msg, wxConvUTF8);
	wxLogMessage(str);
}
#endif

#ifdef OCPN_USE_CRASHRPT
// Define the crash callback
int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO* pInfo)
{
	// Flush log file
	if (pInfo->pUserParam) {
		wxLog* log = reinterpret_cast<wxLog*>(pInfo->pUserParam);
		log->Flush();
	}
	return CR_CB_DODEFAULT;
}
#endif

//------------------------------------------------------------------------------
//      Signal Handlers
//-----------------------------------------------------------------------
#ifndef __WXMSW__
sigjmp_buf env;
extern volatile int quitflag;

//These are the signals possibly expected
//      SIGUSR1
//      Raised externally to cause orderly termination of application
//      Intended to act just like pushing the "EXIT" button

//      SIGSEGV
// Some undefined segfault......
void catch_signals(int signo)
{
	switch (signo) {
		case SIGUSR1:
			quitflag++; // signal to the timer loop
			break;

		case SIGSEGV:
			siglongjmp(env, 1); // jump back to the setjmp() point
			break;

		default:
			break;
	}
}
#endif

static int ShowNavWarning()
{
	wxString msg0(
			_("\n\
OpenCPN is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied\n\
warranty of MERCHANTABILITY or FITNESS FOR A\n\
PARTICULAR PURPOSE.\n\
See the GNU General Public License for more details.\n\n\
OpenCPN must only be used in conjunction with approved\n\
paper charts and traditional methods of navigation.\n\n\
DO NOT rely upon OpenCPN for safety of life or property.\n\n\
Please click \"OK\" to agree and proceed, \"Cancel\" to quit.\n") );

	wxString vs = wxT(" .. Version ") + ocpn::Version().get_short();
	wxMessageDialog odlg(gFrame, msg0, _("Welcome to OpenCPN") + vs, wxCANCEL | wxOK);
	return odlg.ShowModal();
}


IMPLEMENT_APP(App)

BEGIN_EVENT_TABLE(App, wxApp)
	EVT_ACTIVATE_APP(App::OnActivateApp)
END_EVENT_TABLE()

#include "wx/dynlib.h"

App::App()
	: gui_instance(NULL)
	, nav_instance(NULL)
	, ais_instance(NULL)
	, wdt_instance(NULL)
	, sys_instance(NULL)
	, run_instance(NULL)
	, colors_instance(NULL)
	, s52_color_provider(NULL)
	, tracker_instance(NULL)
	, route_manager_instance(NULL)
	, waypoint_manager_instance(NULL)
	, tidecurrent_manager_instance(NULL)
	, style_manager_instance(NULL)
	, font_manager_instance(NULL)
	, start_fullscreen(false)
	, first_run(false)
	, logger(NULL)
	, old_logger(NULL)
	, file_log(NULL)
	, win_console(false)
	, m_checker(NULL)
{
}

void App::OnInitCmdLine(wxCmdLineParser& parser) // FIXME: add option to set configuration to default (config-reset)
{
	static const wxCmdLineEntryDesc OPTIONS[] =
	{
		{
			wxCMD_LINE_SWITCH, NULL, _T("unit_test_1"),
			_T("executes a specific test"),
			wxCMD_LINE_VAL_NONE, 0
		},
		{
			wxCMD_LINE_SWITCH, _T("p"), _T("portable"),
			_T("enables the 'portable' behaviour"),
			wxCMD_LINE_VAL_NONE, 0
		},
		{
			wxCMD_LINE_SWITCH, NULL, _T("no_opengl"),
			_T("disables OpenGL, even if it's compiled in"),
			wxCMD_LINE_VAL_NONE, 0
		},
		{
			wxCMD_LINE_SWITCH, NULL, _T("fullscreen"),
			_T("starts the application in full screen"),
			wxCMD_LINE_VAL_NONE, 0
		},
#ifdef __WXMSW__
		{
			wxCMD_LINE_SWITCH, NULL, _T("winconsole"),
			_T("enables the console output on Windows"),
			wxCMD_LINE_VAL_NONE, 0
		},
#endif
		{ wxCMD_LINE_NONE, NULL, NULL, NULL, wxCMD_LINE_VAL_NONE, 0 }
	};

	parser.SetDesc(OPTIONS);
}

bool App::OnCmdLineParsed(wxCmdLineParser& parser)
{
	g_unit_test_1 = parser.Found(_T("unit_test_1"));
	global::OCPN::get().sys().set_config_portable(parser.Found(_T("p")));
	global::OCPN::get().gui().set_disable_opengl(parser.Found(_T("no_opengl")));
	start_fullscreen = parser.Found(_T("fullscreen"));

#ifdef __WXMSW__
	win_console = parser.Found(_T("winconsole"));
#endif

	return true;
}

void App::OnActivateApp(wxActivateEvent& event)
{
// Code carefully in this method.
// It is called in some unexpected places,
// such as on closure of dialogs, etc.

// Activating?

#ifdef __WXOSX__
	// On the Mac, this method gets hit when...
	// a) switching between apps by clicking title bars, coming and going
	// b) un-iconizing, activeate only/
	// It does NOT get hit on iconizing the app
	if (!event.GetActive()) {
		if (g_FloatingToolbarDialog) {
			if (g_FloatingToolbarDialog->IsShown())
				g_FloatingToolbarDialog->Submerge();
		}

		AppActivateList.Clear();
		if (cc1) {
			for (wxWindowList::iterator it = cc1->GetChildren().begin();
				 it != cc1->GetChildren().end(); ++it) {
				if ((*it)->IsShown()) {
					(*it)->Hide();
					AppActivateList.Append(*it);
				}
			}
		}

		if (gFrame) {
			for (wxWindowList::iterator it = gFrame->GetChildren().begin();
				 it != gFrame->GetChildren().end(); ++it) {
				if ((*it)->IsShown()) {
					if (!(*it)->IsKindOf(CLASSINFO(ChartCanvas))) {
						(*it)->Hide();
						AppActivateList.Append(*it);
					}
				}
			}
		}
	} else {
		gFrame->SubmergeToolbar(); // This is needed to reset internal wxWidgets logic
		// Also required for other TopLevelWindows here
		// reportedly not required for wx 2.9
		gFrame->SurfaceToolbar();

		wxWindow* pOptions = NULL;

		for (WindowList::iterator node = AppActivateList.begin(); node != AppActivateList.end(); ++node) {
			wxWindow* win = *node;
			win->Show();
			if (win->IsKindOf(CLASSINFO(options)))
				pOptions = win;
		}

		if (pOptions)
			pOptions->Raise();
		else
			gFrame->Raise();
	}
#endif

	if (!event.GetActive()) {
		if (g_FloatingToolbarDialog)
			g_FloatingToolbarDialog->HideTooltip(); // Hide any existing tip
	}

	event.Skip();
}

void App::inject_global_instances()
{
	gui_instance = new global::OCPN_GUI;
	global::OCPN::get().inject(gui_instance);

	nav_instance = new global::OCPN_Navigation;
	global::OCPN::get().inject(nav_instance);

	ais_instance = new global::OCPN_AIS;
	global::OCPN::get().inject(ais_instance);

	wdt_instance = new global::OCPN_WatchDog;
	global::OCPN::get().inject(wdt_instance);

	sys_instance = new global::OCPN_System;
	global::OCPN::get().inject(sys_instance);

	run_instance = new global::OCPN_Runtime;
	global::OCPN::get().inject(run_instance);

	colors_instance = new UserColors;
	global::OCPN::get().inject(colors_instance);

#ifdef USE_S57
	s52_color_provider = new chart::S52ColorProvider;
	colors_instance->inject_chart_color_provider(s52_color_provider);
#endif

	tracker_instance = new navigation::DefaultRouteTracker;
	global::OCPN::get().inject(tracker_instance);

	route_manager_instance = new Routeman;
	global::OCPN::get().inject(route_manager_instance);

	// init the waypoint manager (must be after UI style init).
	waypoint_manager_instance = new WayPointman;
	global::OCPN::get().inject(waypoint_manager_instance);

	tidecurrent_manager_instance = new tide::TCMgr;
	global::OCPN::get().inject(tidecurrent_manager_instance);

	font_manager_instance = new FontMgr;
	global::OCPN::get().inject(font_manager_instance);
}

void App::establish_home_location()
{
	wxStandardPathsBase& std_path = wxApp::GetTraits()->GetStandardPaths();

	global::System& sys = global::OCPN::get().sys();
	wxString home_location;

#ifdef __WXMSW__
	home_location.Append(std_path.GetConfigDir()); // on w98, produces "/windows/Application Data"
#else
	home_location.Append(std_path.GetUserConfigDir());
#endif

	if (sys.config().portable) {
		home_location.Clear();
		wxFileName f(std_path.GetExecutablePath());
		home_location.Append(f.GetPath());
	}

	appendOSDirSlash(home_location);

#ifdef __WXOSX__
	home_location.Append(_T("opencpn"));
	appendOSDirSlash(home_location);
#endif

	sys.set_home_location(home_location);
}

void App::determine_config_file()
{
	global::System& sys = global::OCPN::get().sys();
	wxStandardPathsBase& std_path = wxApp::GetTraits()->GetStandardPaths();
	wxString config_file;

// Establish the location of the config file
#ifdef __WXMSW__
	config_file = _T("opencpn.ini");
	config_file.Prepend(sys.data().home_location);

#elif defined(__WXOSX__)
	config_file = std_path.GetUserConfigDir(); // should be ~/Library/Preferences
	appendOSDirSlash(config_file);
	config_file.Append(_T("opencpn.ini"));
#else
	config_file = std_path.GetUserDataDir(); // should be ~/.opencpn
	appendOSDirSlash(config_file);
	config_file.Append(_T("opencpn.conf"));
#endif

	if (sys.config().portable) {
		config_file = sys.data().home_location;
#ifdef __WXMSW__
		config_file += _T("opencpn.ini");
#elif defined(__WXOSX__)
		config_file += _T("opencpn.ini");
#else
		config_file += _T("opencpn.conf");
#endif
	}
	sys.set_config_file(config_file);
}

void App::install_crash_reporting()
{
#ifdef OCPN_USE_CRASHRPT
#ifndef _DEBUG
	// Install Windows crash reporting

	CR_INSTALL_INFO info;
	memset(&info, 0, sizeof(CR_INSTALL_INFO));
	info.cb = sizeof(CR_INSTALL_INFO);
	info.pszAppName = _T("OpenCPN");

	wxString version_crash = ocpn::Version().get_short();
	info.pszAppVersion = version_crash.c_str();

	// Include the data sections from all loaded modules.
	// This results in the inclusion of global variables
	info.uMiniDumpType = MiniDumpWithDataSegs;

	// URL for sending error reports over HTTP.
	info.pszEmailTo = _T("opencpn@bigdumboat.com");
	info.pszSmtpProxy = _T("mail.bigdumboat.com:587");
	info.pszUrl = _T("http://bigdumboat.com/crashrpt/ocpn_crashrpt.php");
	info.uPriorities[CR_HTTP] = 1; // First try send report over HTTP
	info.uPriorities[CR_SMTP] = CR_NEGATIVE_PRIORITY; // Second try send report over SMTP
	info.uPriorities[CR_SMAPI]
		= CR_NEGATIVE_PRIORITY; // 1; // Third try send report over Simple MAPI

	// Install all available exception handlers.
	info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;

	// Use binary encoding for HTTP uploads (recommended).
	info.dwFlags |= CR_INST_HTTP_BINARY_ENCODING;

	// Provide privacy policy URL
	wxStandardPathsBase& std_path_crash = wxApp::GetTraits()->GetStandardPaths();
	std_path_crash.Get();
	wxFileName exec_path_crash(std_path_crash.GetExecutablePath());
	wxString policy_file = exec_path_crash.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	policy_file += _T("PrivacyPolicy.txt");
	policy_file.Prepend(_T("file:"));

	info.pszPrivacyPolicyURL = policy_file.c_str();

	int nResult = crInstall(&info);
	if (nResult != 0) {
		TCHAR buff[256];
		crGetLastErrorMsg(buff, 256);
		MessageBox(NULL, buff, _T("crInstall error, Crash Reporting disabled."), MB_OK);
	}

	// Establish the crash callback function
	crSetCrashCallback(CrashCallback, logger);

	// Take screenshot of the app window at the moment of crash
	crAddScreenshot2(CR_AS_PROCESS_WINDOWS | CR_AS_USE_JPEG_FORMAT, 95);

	// Mark some files to add to the crash report
	wxString home_data_crash = std_path_crash.GetConfigDir();
	if (global::OCPN::get().sys().config().portable) {
		wxFileName f(std_path_crash.GetExecutablePath());
		home_data_crash = f.GetPath();
	}
	appendOSDirSlash(home_data_crash);

	wxString config_crash = _T("opencpn.ini");
	config_crash.Prepend(home_data_crash);
	crAddFile2(config_crash.c_str(), NULL, NULL, CR_AF_MISSING_FILE_OK | CR_AF_ALLOW_DELETE);

	wxString log_crash = _T("opencpn.log");
	log_crash.Prepend(home_data_crash);
	crAddFile2(log_crash.c_str(), NULL, NULL, CR_AF_MISSING_FILE_OK | CR_AF_ALLOW_DELETE);

#endif
#endif
}

void App::seed_random_generator()
{
	// Seed the random number generator
	wxDateTime x = wxDateTime::UNow();
	long seed = x.GetMillisecond();
	seed *= x.GetTicks();
	srand(seed);
}

void App::determine_world_map_location()
{
	global::System& sys = global::OCPN::get().sys();

	// Establish the GSHHS Dataset location
	wxString location = _T("gshhs");
	location.Prepend(sys.data().sound_data_location);
	location.Append(wxFileName::GetPathSeparator());

	sys.set_world_map_location(location);
}

void App::install_signal_handler()
{
#ifndef __WXMSW__
	// Setup Linux SIGNAL handling, for external program control

	// Build the sigaction structure
	sa_all.sa_handler = catch_signals; // point to my handler
	sigemptyset(&sa_all.sa_mask); // make the blocking set
	// empty, so that all
	// other signals will be
	// unblocked during my handler
	sa_all.sa_flags = 0;

	sigaction(SIGUSR1, NULL, &sa_all_old); // save existing action for this signal

	// Register my request for some signals
	sigaction(SIGUSR1, &sa_all, NULL);

	sigaction(SIGUSR1, NULL, &sa_all_old); // inspect existing action for this signal
#endif
}

bool App::create_opencpn_home()
{
	const global::System::Data& sys = global::OCPN::get().sys().data();

	// create the opencpn "home" directory if we need to
	wxFileName wxHomeFiledir(sys.home_location);
	if (true != wxHomeFiledir.DirExists(wxHomeFiledir.GetPath())) {
		if (!wxHomeFiledir.Mkdir(wxHomeFiledir.GetPath())) {
			wxASSERT_MSG(false, _T("Cannot create opencpn home directory"));
			return false;
		}
	}
	return true;
}

bool App::create_opencpn_log()
{
	const global::System::Data & sys = global::OCPN::get().sys().data();

	// create the opencpn "log" directory if we need to
	wxFileName wxLogFiledir(sys.log_file);
	if (true != wxLogFiledir.DirExists(wxLogFiledir.GetPath())) {
		if (!wxLogFiledir.Mkdir(wxLogFiledir.GetPath())) {
			wxASSERT_MSG(false, _T("Cannot create opencpn log directory"));
			return false;
		}
	}
	return true;
}

wxString App::constrain_logfile_size()
{
	const global::System::Data& sys = global::OCPN::get().sys().data();

	// Constrain the size of the log file
	wxString large_log_message;
	if (::wxFileExists(sys.log_file)) {
		if (wxFileName::GetSize(sys.log_file) > 1000000) {
			// Defer the showing of this messagebox until the system locale is established.
			wxString oldlog = sys.log_file + _T(".log");
			large_log_message = _("Old log will be moved to ") + oldlog;
			::wxRenameFile(sys.log_file, oldlog);
		}
	}
	return large_log_message;
}

void App::validate_OpenGL()
{
	// Validate OpenGL functionality, if selected

#ifdef ocpnUSE_GL

#ifdef __WXMSW__
	global::GUI& gui = global::OCPN::get().gui();
	if (!gui.view().disable_opengl) {
		wxFileName fn(wxApp::GetTraits()->GetStandardPaths().GetExecutablePath());
		gui.set_disable_opengl(!TestGLCanvas(fn.GetPathWithSep()));
		if (gui.view().disable_opengl)
			wxLogMessage(_T("OpenGL disabled due to test app failure."));
	}
#endif

#else
	global::OCPN::get().gui().set_disable_opengl(true);
#endif
}

void App::setup_s57()
{
#ifdef USE_S57
	const global::System::Data& sys = global::OCPN::get().sys().data();
	const global::System::Config& cfg = global::OCPN::get().sys().config();

	// Set up a useable CPL library error handler for S57 stuff
	CPLSetErrorHandler(OCPN_CPLErrorHandler);

	// Init the s57 chart object, specifying the location of the required csv files
	wxString csv_location = sys.sound_data_location + _T("s57data");
	if (cfg.portable) {
		csv_location = _T(".");
		appendOSDirSlash(csv_location);
		csv_location.Append(_T("s57data"));
	}
	global::OCPN::get().sys().set_csv_location(csv_location);

	// If the config file contains an entry for SENC file prefix, use it.
	// Otherwise, default to PrivateDataDir
	wxString senc_prefix = sys.SENCPrefix;
	if (sys.SENCPrefix.IsEmpty()) {
		senc_prefix = sys.private_data_dir;
		appendOSDirSlash(senc_prefix);
		senc_prefix.Append(_T("SENC"));
	}
	if (cfg.portable) {
		wxFileName f(senc_prefix);
		if (f.MakeRelativeTo(sys.private_data_dir))
			senc_prefix = f.GetFullPath();
		else
			senc_prefix = _T("SENC");
	}
	global::OCPN::get().sys().set_SENCPrefix(senc_prefix);

	// If the config file contains an entry for PresentationLibraryData, use it.
	// Otherwise, default to conditionally set spot under g_pcsv_locn
	wxString plib_data;
	bool b_force_legacy = false;

	if (sys.UserPresLibData.IsEmpty()) {
		plib_data = sys.csv_location;
		appendOSDirSlash(plib_data);
		plib_data.Append(_T("S52RAZDS.RLE"));
	} else {
		plib_data = sys.UserPresLibData;
		b_force_legacy = true;
	}

	ps52plib = new chart::s52plib(plib_data, b_force_legacy);

	// If the library load failed, try looking for the s57 data elsewhere

	// First, look in UserDataDir
	// From wxWidgets documentation:
	//
	// wxStandardPaths::GetUserDataDir
	// wxString GetUserDataDir() const
	// Return the directory for the user-dependent application data files:
	// * Unix: ~/.appname
	// * Windows: C:\Documents and Settings\username\Application Data\appname
	// * Mac: ~/Library/Application Support/appname
	if (!ps52plib->m_bOK) {
		delete ps52plib;

		wxString look_data_dir;
		look_data_dir.Append(wxApp::GetTraits()->GetStandardPaths().GetUserDataDir());
		appendOSDirSlash(look_data_dir);
		wxString tentative_SData_Locn = look_data_dir;
		look_data_dir.Append(_T("s57data"));

		plib_data = look_data_dir;
		appendOSDirSlash(plib_data);
		plib_data.Append(_T("S52RAZDS.RLE"));

		wxLogMessage(_T("Looking for s57data in ") + look_data_dir);
		ps52plib = new chart::s52plib(plib_data);

		if (ps52plib->m_bOK) {
			global::OCPN::get().sys().set_csv_location(look_data_dir);
			global::OCPN::get().sys().set_sound_data_location(tentative_SData_Locn);
		}
	}

	// And if that doesn't work, look again in the original SData Location
	// This will cover the case in which the .ini file entry is corrupted or moved

	if (!ps52plib->m_bOK) {
		delete ps52plib;

		wxString look_data_dir;
		look_data_dir = sys.sound_data_location;
		look_data_dir.Append(_T("s57data"));

		plib_data = look_data_dir;
		appendOSDirSlash(plib_data);
		plib_data.Append(_T("S52RAZDS.RLE"));

		wxLogMessage(_T("Looking for s57data in ") + look_data_dir);
		ps52plib = new chart::s52plib(plib_data);

		if (ps52plib->m_bOK)
			global::OCPN::get().sys().set_csv_location(look_data_dir);
	}

	if (ps52plib->m_bOK)
		wxLogMessage(_T("Using s57data in ") + sys.csv_location);
	else
		wxLogMessage(_T("   S52PLIB Initialization failed, disabling Vector charts."));

	// Todo Maybe initialize only when an s57 chart is actually opened???
	if (ps52plib->m_bOK)
		m_pRegistrarMan = new chart::S57RegistrarMgr(sys.csv_location, file_log);

	if (!ps52plib->m_bOK) {
		delete ps52plib;
		ps52plib = NULL;
	}

#endif
}

void App::setup_for_empty_config(bool novicemode)
{
	// Override some config options for initial user startup with empty config file
	if (novicemode) {
		global::GUI& gui = global::OCPN::get().gui();
		global::AIS& ais = global::OCPN::get().ais();
		global::Navigation& nav = global::OCPN::get().nav();

		gui.set_view_show_outlines(true);
		ais.set_CPAMax_NM(20.0);
		ais.set_CPAWarn_NM(2.0);
		ais.set_TCPA_Max_min(30.0);
		ais.set_MarkLost(true);
		ais.set_MarkLost_Mins(8);
		ais.set_RemoveLost(true);
		ais.set_RemoveLost_Mins(10);
		ais.set_ShowCOG(true);
		ais.set_ShowCOG_Mins(6);
		ais.set_ShowMoored(true);
		ais.set_ShowMoored_Kts(0.2);
		nav.set_TrackDaily(false);
		nav.set_PlanSpeed(6.0);
		gui.set_view_fullscreen_quilt(true);
		gui.set_view_quilt_enable(true);
		gui.set_skew_comp(false);
		ais.set_ShowAreaNotices(false);
		gui.set_DrawAISSize(false);
		gui.set_ShowAISName(false);

#ifdef USE_S57
		if (ps52plib && ps52plib->m_bOK) {
			using namespace chart;

			ps52plib->m_bShowSoundg = true;
			ps52plib->m_nDisplayCategory = (enum DisCat)STANDARD;
			ps52plib->m_nSymbolStyle = (LUPname)PAPER_CHART;
			ps52plib->m_nBoundaryStyle = (LUPname)PLAIN_BOUNDARIES;
			ps52plib->m_bUseSCAMIN = true;
			ps52plib->m_bShowAtonText = true;

			// Preset some object class visibilites for "Mariner's Standard" disply category
			for (unsigned int iPtr = 0; iPtr < ps52plib->OBJLArray.size(); ++iPtr) {
				OBJLElement* pOLE = ps52plib->OBJLArray.at(iPtr);
				if (!strncmp(pOLE->OBJLName, "DEPARE", 6))
					pOLE->nViz = 1;
				if (!strncmp(pOLE->OBJLName, "LNDARE", 6))
					pOLE->nViz = 1;
				if (!strncmp(pOLE->OBJLName, "COALNE", 6))
					pOLE->nViz = 1;
			}
		}
#endif
	}
}

void App::check_tide_current()
{
	const global::System::Data& sys = global::OCPN::get().sys().data();
	const global::System::Config& cfg = global::OCPN::get().sys().config();

	// Check the global Tide/Current data source array
	// If empty, preset one default (US) Ascii data source
	if (!sys.current_tide_dataset.size()) {
		wxString default_tcdata = sys.sound_data_location + _T("tcdata")
								  + wxFileName::GetPathSeparator() + _T("HARMONIC.IDX");

			std::vector<wxString> dataset;
		if (cfg.portable) {
			wxFileName f(default_tcdata);
			f.MakeRelativeTo(sys.private_data_dir);
			dataset.push_back(f.GetFullPath());
		} else {
			dataset.push_back(default_tcdata);
		}
		global::OCPN::get().sys().set_current_tide_dataset(dataset);
	}
}

void App::check_ais_alarm_sound_file()
{
	const global::System::Data& sys = global::OCPN::get().sys().data();
	const global::System::Config& cfg = global::OCPN::get().sys().config();
	const global::AIS::Data& ais = global::OCPN::get().ais().get_data();

	// Check the global AIS alarm sound file. If empty, preset default.
	if (ais.AIS_Alert_Sound_File.IsEmpty()) {
		wxString default_sound = sys.sound_data_location + _T("sounds")
								 + wxFileName::GetPathSeparator() + _T("2bells.wav");

		wxString filename;
		if (cfg.portable) {
			wxFileName f(default_sound);
			f.MakeRelativeTo(sys.private_data_dir);
			filename = f.GetFullPath();
		} else {
			filename = default_sound;
		}
		global::OCPN::get().ais().set_AIS_Alert_Sound_File(filename);
	}
}

void App::setup_frame_size_and_position(wxPoint & position, wxSize & new_frame_size)
{
	// Set up the frame initial visual parameters
	// Default size, resized later
	new_frame_size = wxSize(-1, -1);
	int cx;
	int cy;
	int cw;
	int ch;
	::wxClientDisplayRect(&cx, &cy, &cw, &ch);

	global::GUI& gui = global::OCPN::get().gui();
	const global::GUI::Frame& frame_config = global::OCPN::get().gui().frame();

	if (true
			&& (frame_config.size.GetWidth() > 100)
			&& (frame_config.size.GetHeight() > 100)
			&& (frame_config.size.GetWidth() <= cw)
			&& (frame_config.size.GetHeight() <= ch))
		new_frame_size = frame_config.size;
	else
		new_frame_size.Set(cw * 7 / 10, ch * 7 / 10);

	// Try to detect any change in physical screen configuration
	// This can happen when drivers are changed, for instance....
	// and can confuse the WUI layout perspective stored in the config file.
	// If detected, force a nominal window size and position....
	if (false
			|| (frame_config.last_position.x != cx)
			|| (frame_config.last_position.y != cy)
			|| (frame_config.last_size.GetWidth() != cw)
			|| (frame_config.last_size.GetHeight() != ch)) {
		new_frame_size.Set(cw * 7 / 10, ch * 7 / 10);
		gui.set_frame_maximized(false);
	}

	gui.set_frame_last_position(wxPoint(cx, cy));
	gui.set_frame_last_size(wxSize(cw, ch));

	// Validate config file position
	position = wxPoint(0, 0);
	wxSize dsize = wxGetDisplaySize();

#ifdef __WXMAC__
	gui.set_frame_position(wxPoint(frame_config.position.x, wxMax(frame_config.position.y, 22)));
#endif

	if ((frame_config.position.x < dsize.x) && (frame_config.position.y < dsize.y))
		position = frame_config.position;

#ifdef __WXMSW__
	// Support MultiMonitor setups which an allow negative window positions.
	RECT frame_rect;
	frame_rect.left = position.x;
	frame_rect.top = position.y;
	frame_rect.right = position.x + new_frame_size.x;
	frame_rect.bottom = position.y + new_frame_size.y;

	//  If the requested frame window does not intersect any installed monitor,
	//  then default to simple primary monitor positioning.
	if (NULL == MonitorFromRect(&frame_rect, MONITOR_DEFAULTTONULL))
		position = wxPoint(10, 10);
#endif
}

void App::setup_gps_watchdog()
{
	// establish GPS timeout value as multiple of frame timer
	// This will override any nonsense or unset value from the config file
	global::WatchDog& wdt = global::OCPN::get().wdt();
	int timeout_ticks = wdt.get_data().gps_watchdog_timeout_ticks;
	if ((timeout_ticks > 60) || (timeout_ticks <= 0))
		wdt.set_gps_timeout_ticks((GPS_TIMEOUT_SECONDS * 1000) / TIMER_GFRAME_1);
	wxLogMessage(wxString::Format(_T("GPS Watchdog Timeout is: %d sec."),
								  wdt.get_data().gps_watchdog_timeout_ticks));

	wdt.set_sat_timeout_ticks(12);
	wdt.set_gps_watchdog(2);
	wdt.set_hdx_watchdog(2);
	wdt.set_hdt_watchdog(2);
	wdt.set_sat_watchdog(2);
	wdt.set_var_watchdog(2);
}

void App::setup_layers()
{
	// Import Layer-wise any .gpx files from /Layers directory
	wxString layerdir = global::OCPN::get().sys().data().private_data_dir;
	appendOSDirSlash(layerdir);
	layerdir.Append(_T("layers"));

	if (wxDir::Exists(layerdir)) {
		wxString laymsg;
		laymsg.Printf(wxT("Getting .gpx layer files from: %s"), layerdir.c_str());
		wxLogMessage(laymsg);
		pConfig->LoadLayers(layerdir);
	}
}

void App::determine_chartlist_filename()
{
	wxString filename;
	const global::System::Data& sys = global::OCPN::get().sys().data();
	const global::System::Config& cfg = global::OCPN::get().sys().config();

	// Establish location and name of chart database
#ifdef __WXMSW__
	filename = _T("CHRTLIST.DAT");
	filename.Prepend(sys.home_location);
#else
	filename = wxApp::GetTraits()->GetStandardPaths().GetUserDataDir();
	appendOSDirSlash(filename);
	filename.Append(_T("chartlist.dat"));
#endif

	if (cfg.portable) {
		filename.Clear();
#ifdef __WXMSW__
		filename.Append(_T("CHRTLIST.DAT"));
#else
		filename.Append(_T("chartlist.dat"));
#endif
		filename.Prepend(sys.home_location);
	}

	global::OCPN::get().sys().set_chartlist_fileame(filename);
}

void App::set_init_chart_dir()
{
	global::System& sys = global::OCPN::get().sys();

	wxString path = sys.data().init_chart_dir;

	// Establish guessed location of chart tree
	if (path.IsEmpty()) {
		if (!sys.config().portable) {
			path.Append(wxApp::GetTraits()->GetStandardPaths().GetDocumentsDir());
			sys.set_init_chart_dir(path);
		}
	}
}

bool App::OnInit()
{
	inject_global_instances();

	if (!wxApp::OnInit())
		return false;

	int mem_total = 0;
	int mem_initial = 0;

	global::GUI& gui = global::OCPN::get().gui();

	// default values for toolbar
	gui.set_toolbar_config(_T("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX"));

	// default value is DAY
	gui.set_color_scheme(global::GLOBAL_COLOR_SCHEME_DAY);

#ifdef __WXMSW__
	// On Windows
	// We allow only one instance unless the portable option is used
	m_checker = new wxSingleInstanceChecker(_T("OpenCPN"));
	if (!global::OCPN::get().sys().config().portable) {
		if (m_checker->IsAnotherRunning())
			return false; // exit quietly
	}
#endif

	install_crash_reporting();
	seed_random_generator();

#ifdef __WXMSW__
	// On MSW, force the entire process to run on one CPU core only
	// This resolves some difficulty with wxThread syncronization
	// Gets the current process handle
	HANDLE hProc = GetCurrentProcess();
	DWORD procMask;
	DWORD sysMask;
	HANDLE hDup;
	DuplicateHandle(hProc, hProc, hProc, &hDup, 0, FALSE, DUPLICATE_SAME_ACCESS);

	// Gets the current process affinity mask
	GetProcessAffinityMask(hDup, &procMask, &sysMask);

	// Take a simple approach, and assume up to 4 processors
	DWORD newMask;
	if ((procMask & 1) == 1)
		newMask = 1;
	else if ((procMask & 2) == 2)
		newMask = 2;
	else if ((procMask & 4) == 4)
		newMask = 4;
	else if ((procMask & 8) == 8)
		newMask = 8;

	// Set te affinity mask for the process
	SetProcessAffinityMask(hDup, (DWORD_PTR)newMask);
#endif

	// Fulup: force floating point to use dot as separation.
	// This needs to be set early to catch numerics in config file.
	setlocale(LC_NUMERIC, "C");

	global::Runtime& run = global::OCPN::get().run();
	run.set_app_start_time(wxDateTime::Now());

	wxDateTime log_last_time = run.data().app_start_time;
	log_last_time.MakeGMT();
	log_last_time.Subtract(wxTimeSpan(0, 29, 0, 0)); // give 1 minute for GPS to get a fix
	run.set_loglast_time(log_last_time);

	global::OCPN::get().nav().set_anchor_PointMinDist(5.0);

#ifdef __WXMSW__
	// Handle any Floating Point Exceptions which may leak thru from other
	// processes.
	// Seems to only happen for W98

	wxPlatformInfo platform;
	if (platform.GetOperatingSystemId() == wxOS_WINDOWS_9X)
		SetUnhandledExceptionFilter(&MyUnhandledExceptionFilter);
#endif

	// Set up some drawing factors
	int mmx, mmy;
	wxDisplaySizeMM(&mmx, &mmy);
	int sx, sy;
	wxDisplaySize(&sx, &sy);
	g_pix_per_mm = ((double)sx) / ((double)mmx);

	install_signal_handler();

	// Init the private memory manager
	malloc_max = 0;

	// Record initial memory status
	GetMemoryStatus(mem_total, mem_initial);

	// Set up default FONT encoding, which should have been done by wxWidgets some time before
	// this......
	wxFont temp_font(10, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, wxString(_T("")),
					 wxFONTENCODING_SYSTEM);
	temp_font.SetDefaultEncoding(wxFONTENCODING_SYSTEM);

	establish_home_location();

	// Establish Log File location
	global::System& sys = global::OCPN::get().sys();
	sys.set_log_file(sys.data().home_location);

#ifdef  __WXOSX__
	wxFileName LibPref(sys.data().log_file); // starts like "~/Library/Preferences"
	LibPref.RemoveLastDir();// takes off "Preferences"
	wxString logfile = LibPref.GetFullPath();
	appendOSDirSlash(logfile);
	logfile.Append(_T("Logs/"));// so, on OS X, opencpn.log ends up in ~/Library/Logs
	// which makes it accessible to Applications/Utilities/Console....
	sys.set_log_file(logfile);
#endif

	if (!create_opencpn_home())
		return false;

	if (!create_opencpn_log())
		return false;

	sys.set_log_file(sys.data().log_file + _T("opencpn.log"));

	wxString large_log_message = constrain_logfile_size();

	file_log = fopen(sys.data().log_file.mb_str(), "a");
	logger = new wxLogStderr(file_log);
	old_logger = wxLog::SetActiveTarget(logger);

#ifdef __WXMSW__
	if (win_console) {
		RedirectIOToConsole();
	}
#endif

#ifndef __WXMSW__
	logger->SetTimestamp(_T("%H:%M:%S %Z"));
#endif

	// version
	wxString vs = ocpn::Version().get_version().Trim(true).Trim(false);

	// Send init message
	wxLogMessage( _T("\n\n________\n") );
	wxLogMessage(wxDateTime::Now().FormatISODate());
	wxLogMessage(_T(" ------- Starting OpenCPN -------"));
	wxLogMessage(vs);
	wxLogMessage(_T("wxWidgets version: " + wxString(wxVERSION_STRING)));
	wxLogMessage(_T("MemoryStatus:  mem_total: %d mb,  mem_initial: %d mb"), mem_total / 1024, mem_initial / 1024);

	// Initialize embedded PNG icon graphics
	::wxInitAllImageHandlers();

	// Establish a "shared data" location
	// From the wxWidgets documentation...

	// wxStandardPaths::GetDataDir
	// wxString GetDataDir() const
	// Return the location of the applications global, i.e. not user-specific, data files.
	// * Unix: prefix/share/appname
	// * Windows: the directory where the executable file is located
	// * Mac: appname.app/Contents/SharedSupport bundle subdirectory
	wxStandardPathsBase & std_path = wxApp::GetTraits()->GetStandardPaths();
	std_path.Get();
	wxString sound_data_location = std_path.GetDataDir();
	appendOSDirSlash(sound_data_location);

	sys.set_exe_path(std_path.GetExecutablePath());

	const global::System::Config& cfg = global::OCPN::get().sys().config();
	if (cfg.portable)
		sound_data_location = sys.data().home_location;

	sys.set_sound_data_location(sound_data_location);

	wxLogMessage(_T("SData_Locn is ") + sys.data().sound_data_location);

	// Establish an empty ChartCroupArray
	g_pGroupArray = new chart::ChartGroupArray;

	// Establish the prefix of the location of user specific data files
#ifdef __WXMSW__
	sys.set_private_data_dir(sys.data().home_location); // should be {Documents and Settings}\......
#elif defined __WXOSX__
	sys.set_private_data_dir(std_path.GetUserConfigDir()); // should be ~/Library/Preferences
#else
	sys.set_private_data_dir(std_path.GetUserDataDir()); // should be ~/.opencpn
#endif

	if (cfg.portable)
		sys.set_private_data_dir(sys.data().home_location);

	// Get the PlugIns directory location
	// linux  : {prefix}/lib/opencpn
	// Mac    : appname.app/Contents/PlugIns
	// Windows: {exe dir}/plugins
	wxString plugin_dir = std_path.GetPluginsDir();
#ifdef __WXMSW__
	plugin_dir += _T("\\plugins");
#endif

	if (cfg.portable) {
		plugin_dir = sys.data().home_location;
		plugin_dir += _T("plugins");
	}
	global::OCPN::get().sys().set_plugin_dir(plugin_dir);

	// Init the Selectable Route Items List
	pSelect = new Select;
	pSelect->SetSelectPixelRadius(12);

	// Init the Selectable Tide/Current Items List
	pSelectTC = new Select;

	// Increase the select radius for tide/current stations
	pSelectTC->SetSelectPixelRadius(25);

	// Init the Selectable AIS Target List
	pSelectAIS = new Select;
	pSelectAIS->SetSelectPixelRadius(12);

	// Initially AIS display is always on
	global::OCPN::get().gui().set_ShowAIS(true);
	g_pais_query_dialog_active = NULL;

	determine_config_file();

	bool novicemode = false;

	wxFileName config_test_file_name(sys.data().config_file);
	if (config_test_file_name.FileExists()) {
		wxLogMessage(_T("Using existing Config_File: ") + sys.data().config_file);
	} else {
		wxLogMessage(_T("Creating new Config_File: ") + sys.data().config_file);

		// Flag to preset some options for initial config file creation
		novicemode = true;

		if (true != config_test_file_name.DirExists(config_test_file_name.GetPath()))
			if (!config_test_file_name.Mkdir(config_test_file_name.GetPath()))
				wxLogMessage(_T("Cannot create config file directory for ")
							 + sys.data().config_file);
	}

	// Now initialize UI Style.
	style_manager_instance = new gui::DefaultStyleManager;
	if (!style_manager_instance->initialize()) {
		wxString msg = _("Failed to initialize the user interface. ");
		msg << _("OpenCPN cannot start. ");
		msg << _("The necessary configuration files were not found. ");
		msg << _("See the log file at ") << sys.data().log_file << _(" for details.");
		wxMessageDialog w(NULL, msg, _("Failed to initialize the user interface. "),
						  wxCANCEL | wxICON_ERROR);
		w.ShowModal();
		exit(EXIT_FAILURE);
	}
	global::OCPN::get().inject(style_manager_instance);

	// Init the WayPoint Manager (Must be after UI Style init).
	dynamic_cast<WayPointman*>(waypoint_manager_instance)->initialize();

	// Open/Create the Config Object (Must be after UI Style init).
	pConfig = new Config(wxString(_T("")), wxString(_T("")), sys.data().config_file);
	pConfig->LoadConfig(0);

	if (g_btouch) {
		int SelectPixelRadius = 50;
		pSelect->SetSelectPixelRadius(SelectPixelRadius);
		pSelectTC->SetSelectPixelRadius(wxMax(25, SelectPixelRadius));
		pSelectAIS->SetSelectPixelRadius(SelectPixelRadius);
	}

	// Is this the first run after a clean install?
	if (!sys.config().nav_message_shown)
		first_run = true;

	// Now we can set the locale

	// Manage internationalization of embedded messages
	// using wxWidgets/gettext methodology....

#ifdef __WXMSW__
	// Add a new prefix for search order.
	wxString locale_location = sys.data().sound_data_location;
	locale_location += _T("share/locale");
	wxLocale::AddCatalogLookupPathPrefix(locale_location);
#endif

	// Get the default language info
	wxString def_lang_canonical;
	const wxLanguageInfo* languageInfo = wxLocale::GetLanguageInfo(wxLANGUAGE_DEFAULT);
	if (languageInfo) {
		def_lang_canonical = languageInfo->CanonicalName;
		wxLogMessage(_T("System default Language:  ") + def_lang_canonical);
	}

#ifdef __WXMSW__
	// For windows, installer may have left information in the registry defining the
	// user's selected install language.
	// If so, override the config file value and use this selection for opencpn...
	if (first_run) {
		wxRegKey RegKey(wxString(_T("HKEY_LOCAL_MACHINE\\SOFTWARE\\OpenCPN")));
		if (RegKey.Exists()) {
			wxLogMessage(_("Retrieving initial language selection from Windows Registry"));
			wxString locale;
			RegKey.QueryValue(wxString(_T("InstallerLanguage")), locale);
			global::OCPN::get().sys().set_locale(locale);
		}
	}
#endif

	// Find the language specified by the config file
	const wxLanguageInfo* pli = wxLocale::FindLanguageInfo(global::OCPN::get().sys().data().locale);
	wxString loc_lang_canonical;
	bool b_initok;
	plocale_def_lang = new wxLocale;

	if (pli) {
		b_initok = plocale_def_lang->Init(pli->Language, 0);
		loc_lang_canonical = pli->CanonicalName;
	}

	if (!pli || !b_initok) {
		delete plocale_def_lang;
		plocale_def_lang = new wxLocale;
		plocale_def_lang->Init(wxLANGUAGE_ENGLISH_US, 0);
		loc_lang_canonical = wxLocale::GetLanguageInfo(wxLANGUAGE_ENGLISH_US)->CanonicalName;
	}

	wxLogMessage(_T("Opencpn language set to:  ") + loc_lang_canonical);

	// Set filename without extension (example : opencpn_fr_FR)
	// i.e. : Set-up the filename needed for translation
	//        wxString loc_lang_filename = _T("opencpn_") + loc_lang_canonical;
	wxString loc_lang_filename = _T("opencpn");

	// Get translation file (example : opencpn_fr_FR.mo)
	// No problem if the file doesn't exist
	// as this case is handled by wxWidgets
	if (plocale_def_lang)
		plocale_def_lang->AddCatalog(loc_lang_filename);

	// Always use dot as decimal
	setlocale(LC_NUMERIC, "C");

	wxLog::SetVerbose(false); // log no verbose messages

	// French language locale is assumed to include the AZERTY keyboard
	// This applies to either the system language, or to OpenCPN language selection
	sys.set_config_assume_azerty(false); // default value
	if (loc_lang_canonical == _T("fr_FR"))
		sys.set_config_assume_azerty(true);
	if (def_lang_canonical == _T("fr_FR"))
		sys.set_config_assume_azerty(true);

	// Send the Welcome/warning message if it has never been sent before,
	// or if the version string has changed at all
	// We defer until here to allow for localization of the message
	if (!sys.config().nav_message_shown || (vs != sys.config().version_string)) {
		if (wxID_CANCEL == ShowNavWarning())
			return false;
		sys.set_config_nav_message_shown(true);
	}

	sys.set_config_version_string(vs);

	// Show deferred log restart message, if it exists.
	if (!large_log_message.IsEmpty())
		OCPNMessageBox(NULL, large_log_message, wxString(_("OpenCPN Info")),
					   wxICON_INFORMATION | wxOK, 5);

	validate_OpenGL();

	setup_s57();

	// Set default color scheme
	global::OCPN::get().gui().set_color_scheme(global::GLOBAL_COLOR_SCHEME_DAY);

#ifdef __WXMSW__
	// On Windows platforms, establish a default cache managment policy
	// as allowing OpenCPN a percentage of available physical memory,
	// not to exceed 1 GB
	// Note that this logic implies that Windows platforms always use
	// the memCacheLimit policy, and never use the fallback nCacheLimit policy
	if (0 == sys.config().memCacheLimit)
		sys.set_config_memCacheLimit(mem_total * 0.5);
	sys.set_config_memCacheLimit(wxMin(sys.config().memCacheLimit, 1024 * 1024));
#endif

	determine_chartlist_filename();
	set_init_chart_dir();
	determine_world_map_location();

	// Reload the config data, to pick up any missing data class configuration info
	// e.g. s52plib, which could not be created until first config load completes
	// Think catch-22
	pConfig->LoadConfig(1);

	setup_for_empty_config(novicemode);
	check_tide_current();
	check_ais_alarm_sound_file();

	g_StartTime = wxInvalidDateTime;
	g_StartTimeTZ = 1; // start with local times
	gpIDX = NULL;
	gpIDXn = 0;

	wxPoint position;
	wxSize new_frame_size;
	setup_frame_size_and_position(position, new_frame_size);

	// For Windows and GTK, provide the expected application Minimize/Close bar
	long app_style = wxDEFAULT_FRAME_STYLE | wxWANTS_CHARS;

	// Create the main frame window
	wxString myframe_window_title = wxT("OpenCPN ") + ocpn::Version().get_short();
	if (cfg.portable) {
		myframe_window_title += _(" -- [Portable(-p) executing from ");
		myframe_window_title += sys.data().home_location;
		myframe_window_title += _T("]");
	}

	gFrame = new MainFrame(NULL, myframe_window_title, position, new_frame_size, app_style);
	gFrame->init_bell_sounds();

	// AUI manager
	g_pauimgr = new wxAuiManager;
	g_pauimgr->SetManagedWindow(gFrame);

	// Create Children of Frame
	// n.b.  if only one child exists, wxWindows expands the child
	// to the parent client area automatically, (as a favor?)
	// Here, we'll do explicit sizing on SIZE events

	const global::GUI::View& view = global::OCPN::get().gui().view();

	cc1 = new ChartCanvas(gFrame); // the chart display canvas
	gFrame->SetCanvasWindow(cc1);

	cc1->SetQuiltMode(view.quilt_enable); // set initial quilt mode
	cc1->set_follow(pConfig->follow()); // set initial state
	cc1->SetViewPoint(global::OCPN::get().nav().get_data().view_point, view.initial_scale_ppm, 0.0, 0.0);

	gFrame->Enable();
	cc1->SetFocus();

	console = new ConsoleCanvas(gFrame); // the console
	pthumbwin = new ThumbWin(cc1);
	gFrame->ApplyGlobalSettings(1, false); // done once on init with resize

	gui_instance->ensure_toolbar_position_range(wxPoint(0, 0),
												::wxGetClientDisplayRect().GetBottomRight());
	gui_instance->ensure_ais_alert_dialog_position_range(wxPoint(0, 0), wxGetDisplaySize());

	g_FloatingToolbarDialog = new OCPNFloatingToolbarDialog(cc1, gui_instance->toolbar().position,
															gui_instance->toolbar().orientation);
	g_FloatingToolbarDialog->LockPosition(true);
	gFrame->SetAndApplyColorScheme(view.color_scheme);

	// The position and size of the static frame children (i.e. the canvas, and the status bar) are now set
	// So now we can establish the AUI panes for them.
	// It is important to have set the chartcanvas and status bar sizes before this point,
	// so that the pane.BestSize values are correctly captured by the AuiManager.

	g_pauimgr->AddPane(cc1);
	g_pauimgr->GetPane(cc1).Name(_T("ChartCanvas"));
	g_pauimgr->GetPane(cc1).Fixed();
	g_pauimgr->GetPane(cc1).CaptionVisible(false);
	g_pauimgr->GetPane(cc1).CenterPane();
	g_pauimgr->GetPane(cc1).BestSize(cc1->GetSize());

	// Load and initialize any PlugIns
	g_pi_manager = new PlugInManager(gFrame);
	g_pi_manager->LoadAllPlugIns(global::OCPN::get().sys().data().plugin_dir);

	// Show the frame

	gFrame->ClearBackground();
	gFrame->Show(true);

	if (global::OCPN::get().gui().frame().maximized)
		gFrame->Maximize(true);

	if (g_bresponsive && (g_pix_per_mm > 4.0))
		gFrame->Maximize(true);

	stats = new StatWin(cc1);
	stats->SetColorScheme(view.color_scheme);
	gui::Style& style = style_manager_instance->current();
	if (cc1->GetQuiltMode()) {
		stats->pPiano->SetVizIcon(new wxBitmap(style.GetIcon(_T("viz"))));
		stats->pPiano->SetInVizIcon(new wxBitmap(style.GetIcon(_T("redX"))));
		stats->pPiano->SetRoundedRectangles(true);
	}
	stats->pPiano->SetTMercIcon(new wxBitmap(style.GetIcon(_T("tmercprj"))));
	stats->pPiano->SetPolyIcon(new wxBitmap(style.GetIcon(_T("polyprj"))));
	stats->pPiano->SetSkewIcon(new wxBitmap(style.GetIcon(_T("skewprj"))));

	// Yield to pick up the OnSize() calls that result from Maximize()
	Yield();

	wxString perspective;
	pConfig->SetPath(_T("/AUI"));
	pConfig->Read(_T("AUIPerspective"), &perspective);

	// Make sure the perspective saved in the config file is "reasonable"
	// In particular, the perspective should have an entry for every
	// windows added to the AUI manager so far.
	// If any are not found, then use the default layout

	bool bno_load = false;
	wxAuiPaneInfoArray pane_array_val = g_pauimgr->GetAllPanes();
	for (unsigned int i = 0; i < pane_array_val.size(); i++) {
		wxAuiPaneInfo pane = pane_array_val.Item(i);
		if (perspective.Find(pane.name) == wxNOT_FOUND) {
			bno_load = true;
			break;
		}
	}

	if (!bno_load)
		g_pauimgr->LoadPerspective(perspective, false);
	g_pauimgr->Update();

	// Notify all the AUI PlugIns so that they may syncronize with the Perspective
	g_pi_manager->NotifyAuiPlugIns();

	bool b_SetInitialPoint = false;

	// Build the initial chart dir array
	ChartDirectories ChartDirArray;
	pConfig->LoadChartDirArray(ChartDirArray);

#ifdef __WXMSW__
	// Windows installer may have left hints regarding the initial chart dir selection
	if (first_run) {
		int ndirs = 0;

		wxRegKey RegKey(wxString(_T("HKEY_LOCAL_MACHINE\\SOFTWARE\\OpenCPN")));
		if (RegKey.Exists()) {
			wxLogMessage(_("Retrieving initial Chart Directory set from Windows Registry"));
			wxString dirs;
			RegKey.QueryValue(wxString(_T("ChartDirs")), dirs);

			wxStringTokenizer tkz(dirs, _T(";"));
			while (tkz.HasMoreTokens()) {
				wxString token = tkz.GetNextToken();
				ChartDirArray.push_back(ChartDirectoryInfo(token.Trim(), _T("")));
				ndirs++;
			}
		}

		if (ndirs)
			pConfig->UpdateChartDirs(ChartDirArray);

		// As a favor to new users, poll the database and
		// move the initial viewport so that a chart will come up.
		if (ndirs)
			b_SetInitialPoint = true;
	}
#endif

	// If the ChartDirArray is empty at this point, any existing chart database file must be declared invalid,
	// So it is best to simply delete it if present.
	// TODO: There is a possibility of recreating the dir list from the database itself......

	if (ChartDirArray.empty())
		::wxRemoveFile(sys.data().chartlist_filename);

	// Try to load the current chart list Data file
	ChartData = new chart::ChartDB(gFrame);
	if (!ChartData->LoadBinary(sys.data().chartlist_filename, ChartDirArray)) {
		bDBUpdateInProgress = true;

		if (ChartDirArray.size()) {
			// Create and Save a new Chart Database based on the hints given in the config file

			delete ChartData;
			ChartData = new chart::ChartDB(gFrame);

			wxString line(_("Rebuilding chart database from configuration file entries..."));
			// The following 3 strings are embeded in wxProgressDialog but must be included by
			// xgettext to be localized properly. See {wxWidgets}src/generic/progdlgg.cpp:190
			wxString dummy1 = _("Elapsed time : ");
			wxString dummy2 = _("Estimated time : ");
			wxString dummy3 = _("Remaining time : ");
			wxProgressDialog* pprog = new wxProgressDialog(
				_("OpenCPN Chart Update"), line, 100, NULL,
				wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);

			ChartData->Create(ChartDirArray, pprog);
			ChartData->SaveBinary(sys.data().chartlist_filename);

			delete pprog;
		} else {
			// No chart database, no config hints, so bail to Options....
			wxLogMessage(_T("Chartlist file not found, config chart dir array is empty.  ")
						 _T("Chartlist target file is:") + sys.data().chartlist_filename);
			wxString msg1(
				_("No Charts Installed.\nPlease select chart folders in Options > Charts."));
			OCPNMessageBox(gFrame, msg1, wxString(_("OpenCPN Info")), wxICON_INFORMATION | wxOK);
			gFrame->DoOptionsDialog();
			b_SetInitialPoint = true;
		}

		bDBUpdateInProgress = false;

		// As a favor to new users, poll the database and
		// move the initial viewport so that a chart will come up.

		if (b_SetInitialPoint) {
			global::Navigation& nav = global::OCPN::get().nav();
			double clat;
			double clon;
			if (ChartData->GetCentroidOfLargestScaleChart(&clat, &clon, chart::CHART_FAMILY_RASTER)) {
				nav.set_latitude(clat);
				nav.set_longitude(clon);
				gFrame->ClearbFollow();
			} else {
				if (ChartData->GetCentroidOfLargestScaleChart(&clat, &clon, chart::CHART_FAMILY_VECTOR)) {
					nav.set_latitude(clat);
					nav.set_longitude(clon);
					gFrame->ClearbFollow();
				}
			}
		}
	}

	// Apply the inital Group Array structure to the chart data base
	ChartData->ApplyGroupArray(g_pGroupArray);

	// Make sure that the Selected Group is sensible...
	if (gui.view().GroupIndex > (int)g_pGroupArray->size())
		gui.set_GroupIndex(0);
	if (!gFrame->CheckGroup(gui.view().GroupIndex))
		gui.set_GroupIndex(0);

	// Delete any stack built by no-chart startup case
	if (pCurrentStack)
		delete pCurrentStack;

	pCurrentStack = new chart::ChartStack;

	// A useability enhancement....
	// if the chart database is truly empty on startup, switch to SCMode
	// so that the WVS chart will at least be shown
	if (ChartData && (0 == ChartData->GetChartTableEntries())) {
		cc1->SetQuiltMode(false);
		gFrame->SetupQuiltMode();
	}

	// All set to go.....

	setup_gps_watchdog();

	// Start up a new track if enabled in config file
	if (global::OCPN::get().ais().get_data().TrackCarryOver)
		global::OCPN::get().tracker().start();

	// Re-enable anchor watches if set in config file
	const global::Navigation::Anchor& anchor = global::OCPN::get().nav().anchor();
	if (!anchor.AW1GUID.IsEmpty()) {
		pAnchorWatchPoint1 = waypoint_manager_instance->find(anchor.AW1GUID);
	}
	if (!anchor.AW2GUID.IsEmpty()) {
		pAnchorWatchPoint2 = waypoint_manager_instance->find(anchor.AW2GUID);
	}

	stats->Show(true);
	Yield();
	gFrame->DoChartUpdate();
	g_FloatingToolbarDialog->LockPosition(false);
	gFrame->RequestNewToolbar();

	// Start up the ticker....
	gFrame->FrameTimer1.Start(TIMER_GFRAME_1, wxTIMER_CONTINUOUS);

	// Start up the ViewPort Rotation angle Averaging Timer....
	gFrame->FrameCOGTimer.Start(10, wxTIMER_CONTINUOUS);

	setup_layers();
	cc1->ReloadVP(); // once more, and good to go

	g_FloatingCompassDialog = new FloatingCompassWindow(cc1);
	if (g_FloatingCompassDialog)
		g_FloatingCompassDialog->UpdateStatus(true);

	g_FloatingToolbarDialog->Raise();
	g_FloatingToolbarDialog->Show();

	gFrame->Refresh(false);
	gFrame->Raise();

	cc1->Enable();
	cc1->SetFocus();

#ifdef ocpnUSE_GL
	// This little hack fixes a problem seen with some UniChrome OpenGL drivers
	// We need a deferred resize to get glDrawPixels() to work right.
	// So we set a trigger to generate a resize after 5 seconds....
	// See the "UniChrome" hack elsewhere
	if (!view.disable_opengl) {
		glChartCanvas* pgl = static_cast<glChartCanvas*>(cc1->GetglCanvas());
		if (pgl && (pgl->GetRendererString().Find(_T("UniChrome")) != wxNOT_FOUND)) {
			gFrame->m_defer_size = gFrame->GetSize();
			gFrame->SetSize(gFrame->m_defer_size.x - 10, gFrame->m_defer_size.y);
			g_pauimgr->Update();
			gFrame->m_bdefer_resize = true;
		}
	}
#endif

	g_pi_manager->CallLateInit();

	if (start_fullscreen)
		gFrame->ToggleFullScreen();

	return true;
}

int App::OnExit()
{
	// Send current nav status data to log file

	wxDateTime lognow = wxDateTime::Now().MakeGMT();
	wxLogMessage(MainFrame::prepare_logbook_message(lognow));
	global::OCPN::get().run().set_loglast_time(lognow);

	wxLogMessage(_T("opencpn::App exiting cleanly...\n"));
	delete pConfig;
	delete pSelect;
	delete pSelectTC;
	delete pSelectAIS;

#ifdef USE_S57
	delete ps52plib;
#endif

	delete g_pGroupArray;

	if (logger) {
		wxLog::SetActiveTarget(old_logger);
		delete logger;
	}

	LogMessageOnce::destroy();

	dynamic_cast<WayPointman*>(waypoint_manager_instance)->clean_points();
	delete pLayerList;

#ifdef USE_S57
	delete m_pRegistrarMan;
	CSVDeaccess(NULL);
#endif

	global::OCPN::get().inject(static_cast<gui::StyleManager*>(NULL));
	delete style_manager_instance;
	style_manager_instance = NULL;

#ifdef USE_S57
#ifdef __WXMSW__
#ifdef USE_GLU_TESS
#ifdef USE_GLU_DLL
	if (s_glu_dll_ready)
		FreeLibrary(s_hGLU_DLL); // free the glu32.dll
#endif
#endif
#endif
#endif

#ifdef OCPN_USE_PORTAUDIO
	if (sound::portaudio_initialized)
		Pa_Terminate();
#endif

	// Restore any changed system colors
#ifdef __WXMSW__
	RestoreSystemColors();
#endif

#ifdef __MSVC__LEAK
	DeInitAllocCheck();
#endif

	delete g_pauimgr;
	delete plocale_def_lang;
	delete m_checker;

#ifdef OCPN_USE_CRASHRPT
#ifndef _DEBUG
	// Uninstall Windows crash reporting
	crUninstall();
#endif
#endif

	global::OCPN::get().clear();
	delete colors_instance;
	delete s52_color_provider;
	delete gui_instance;
	delete nav_instance;
	delete wdt_instance;
	delete sys_instance;
	delete run_instance;
	delete ais_instance;
	delete route_manager_instance;
	delete waypoint_manager_instance;
	delete tracker_instance;
	delete tidecurrent_manager_instance;
	delete font_manager_instance;

	return true;
}

