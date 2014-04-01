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

#include "PlugInManager.h"
#include <MainFrame.h>
#include <Track.h>
#include <ocpnDC.h>
#include <Select.h>
#include <OptionDialog.h>
#include <Multiplexer.h>
#include <StatWin.h>
#include <OCPN_DataStreamEvent.h>
#include <RouteManagerDialog.h>
#include <NavObjectCollection.h>
#include <Units.h>
#include <ViewPort.h>
#include <DimeControl.h>
#include <ChartCanvas.h>
#include <Config.h>
#include <MessageBox.h>

#include <global/OCPN.h>
#include <global/System.h>
#include <global/ColorManager.h>

#include <gui/FontManager.h>
#include <gui/StyleManager.h>
#include <gui/Style.h>

#include <navigation/RouteManager.h>
#include <navigation/WaypointManager.h>

#include <geo/GeoRef.h>

#include <plugin/OCPN_MsgEvent.h>

#include <chart/ChartDB.h>
#include <chart/ChartDatabase.h>
#include <chart/gshhs/QLineF.h>
#include <chart/gshhs/GshhsReader.h>
#include <chart/gshhs/Projection.h>

#include <ais/ais.h>
#include <ais/AIS_Decoder.h>
#include <ais/AIS_Target_Data.h>

#include <util/uuid.h>

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/aui/aui.h>
#include <wx/statline.h>

#include <typeinfo>

extern Config* pConfig;
extern ais::AIS_Decoder* g_pAIS;
extern wxAuiManager* g_pauimgr;
extern wxLocale* plocale_def_lang;
extern chart::ChartDB* ChartData;
extern MainFrame* gFrame;
extern options* g_pOptions;
extern Multiplexer* g_pMUX;
extern StatWin* stats;
extern Select* pSelect;
extern RouteManagerDialog* pRouteManagerDialog;
extern RouteList* pRouteList;
extern PlugInManager* g_pi_manager;
extern options* g_options;

// Some static helper funtions
// Scope is local to this module

PlugIn_ViewPort CreatePlugInViewport(const ViewPort& vp)
{
	// Create a PlugIn Viewport
	ViewPort tvp = vp;
	PlugIn_ViewPort pivp;

	pivp.clat = tvp.latitude(); // center point
	pivp.clon = tvp.longitude();
	pivp.view_scale_ppm = tvp.view_scale();
	pivp.skew = tvp.skew;
	pivp.rotation = tvp.rotation;
	pivp.chart_scale = tvp.chart_scale;
	pivp.pix_width = tvp.pix_width;
	pivp.pix_height = tvp.pix_height;
	pivp.rv_rect = tvp.rv_rect;
	pivp.b_quilt = tvp.is_quilt();
	pivp.m_projection_type = tvp.m_projection_type;

	pivp.lat_min = tvp.GetBBox().GetMinY();
	pivp.lat_max = tvp.GetBBox().GetMaxY();
	pivp.lon_min = tvp.GetBBox().GetMinX();
	pivp.lon_max = tvp.GetBBox().GetMaxX();

	pivp.bValid = tvp.IsValid(); // This VP is valid

	return pivp;
}

ViewPort CreateCompatibleViewport(const PlugIn_ViewPort& pivp)
{
	// Create a system ViewPort
	ViewPort vp;

	vp.set_position(geo::Position(pivp.clat, pivp.clon));
	vp.set_view_scale(pivp.view_scale_ppm);
	vp.skew = pivp.skew;
	vp.rotation = pivp.rotation;
	vp.chart_scale = pivp.chart_scale;
	vp.pix_width = pivp.pix_width;
	vp.pix_height = pivp.pix_height;
	vp.rv_rect = pivp.rv_rect;
	vp.set_quilt(pivp.b_quilt);
	vp.m_projection_type = pivp.m_projection_type;

	vp.SetBBoxDirect(pivp.lat_min, pivp.lon_min, pivp.lat_max, pivp.lon_max);
	vp.Validate(); // This VP is valid

	return vp;
}


const wxEventType wxEVT_OCPN_MSG = wxNewEventType();

//-----------------------------------------------------------------------------------------------------
//
//          The PlugIn Manager Implementation
//
//-----------------------------------------------------------------------------------------------------
PlugInManager *s_ppim;

PlugInManager::PlugInManager(MainFrame* parent)
{
	pParent = parent;
	s_ppim = this;

	MainFrame* pFrame = GetParentFrame();
	if (pFrame) {
		// FIXME: interface transition
		m_plugin_menu_item_id_next = pFrame->GetCanvas()->GetNextContextMenuId();
		m_plugin_tool_id_next = pFrame->GetNextToolbarToolId();
	}
}

PlugInManager::~PlugInManager()
{
}

bool PlugInManager::LoadAllPlugIns(const wxString& plugin_dir)
{
	m_plugin_location = plugin_dir;

	wxString msg(_T("PlugInManager searching for PlugIns in location "));
	msg += m_plugin_location;
	wxLogMessage(msg);

#ifdef __WXMSW__
	wxString pispec = _T("*_pi.dll");
#else
#ifdef __WXOSX__
	wxString pispec = _T("*_pi.dylib");
#else
	wxString pispec = _T("*_pi.so");
#endif
#endif

	if (!::wxDirExists(m_plugin_location)) {
		msg = m_plugin_location;
		msg.Prepend(_T("   Directory "));
		msg.Append(_T(" does not exist."));
		wxLogMessage(msg);
		return false;
	}

	wxArrayString file_list;
	wxString plugin_file;

	int get_flags = wxDIR_FILES | wxDIR_DIRS;
#ifdef __WXMSW__
#ifdef _DEBUG
	get_flags = wxDIR_FILES;
#endif
#endif

	wxDir::GetAllFiles(m_plugin_location, &file_list, pispec, get_flags);

	for (unsigned int i = 0; i < file_list.GetCount(); i++) {
		wxString file_name = file_list[i];

		bool b_compat = CheckPluginCompatibility(file_name);

		if (!b_compat) {
			wxString msg(_("    Incompatible PlugIn detected:"));
			msg += file_name;
			wxLogMessage(msg);
		}

		PlugInContainer* pic = NULL;
		if (b_compat)
			pic = LoadPlugIn(file_name);
		if (pic) {
			if (pic->m_pplugin) {
				plugin_array.Add(pic);

				// The common name is available without initialization and startup of the PlugIn
				pic->m_common_name = pic->m_pplugin->GetCommonName();

				// Check the config file to see if this PlugIn is user-enabled
				wxString config_section = (_T("/PlugIns/"));
				config_section += pic->m_common_name;
				pConfig->SetPath(config_section);
				pConfig->Read(_T("bEnabled"), &pic->m_bEnabled);

				if (pic->m_bEnabled) {
					pic->m_cap_flag = pic->m_pplugin->Init();
					pic->m_bInitState = true;
				}

				pic->m_short_description = pic->m_pplugin->GetShortDescription();
				pic->m_long_description = pic->m_pplugin->GetLongDescription();
				pic->m_version_major = pic->m_pplugin->GetPlugInVersionMajor();
				pic->m_version_minor = pic->m_pplugin->GetPlugInVersionMinor();
				pic->m_bitmap = pic->m_pplugin->GetPlugInBitmap();

			} else {
				// not loaded
				wxString msg;
				msg.Printf(_T("    PlugInManager: Unloading invalid PlugIn, API version %d "),
						   pic->m_api_version);
				wxLogMessage(msg);

				pic->m_destroy_fn(pic->m_pplugin);

				delete pic->m_plibrary; // This will unload the PlugIn
				delete pic;
			}
		}
	}

	UpDateChartDataTypes();

	return true;
}

bool PlugInManager::CallLateInit(void)
{
	bool bret = true;

	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);

		switch (pic->m_api_version) {
			case 110:
				if (pic->m_cap_flag & WANTS_LATE_INIT) {
					wxString msg(_T("PlugInManager: Calling LateInit PlugIn: "));
					msg += pic->m_plugin_file;
					wxLogMessage(msg);

					opencpn_plugin_110* ppi = dynamic_cast<opencpn_plugin_110*>(pic->m_pplugin);
					ppi->LateInit();
				}
				break;
		}
	}

	return bret;
}

bool PlugInManager::UpdatePlugIns()
{
	bool bret = false;

	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);

		if (pic->m_bEnabled && !pic->m_bInitState) {
			wxString msg(_T("PlugInManager: Initializing PlugIn: "));
			msg += pic->m_plugin_file;
			wxLogMessage(msg);

			pic->m_cap_flag = pic->m_pplugin->Init();
			pic->m_pplugin->SetDefaults();
			pic->m_bInitState = true;
			pic->m_short_description = pic->m_pplugin->GetShortDescription();
			pic->m_long_description = pic->m_pplugin->GetLongDescription();
			pic->m_version_major = pic->m_pplugin->GetPlugInVersionMajor();
			pic->m_version_minor = pic->m_pplugin->GetPlugInVersionMinor();
			pic->m_bitmap = pic->m_pplugin->GetPlugInBitmap();
			bret = true;
		} else if (!pic->m_bEnabled && pic->m_bInitState) {
			bret = DeactivatePlugIn(pic);
		}
	}

	UpDateChartDataTypes();

	return bret;
}

bool PlugInManager::UpDateChartDataTypes(void)
{
	bool bret = false;
	if (NULL == ChartData)
		return bret;

	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);

		if ((pic->m_cap_flag & INSTALLS_PLUGIN_CHART)
			|| (pic->m_cap_flag & INSTALLS_PLUGIN_CHART_GL))
			bret = true;
	}

	if (bret)
		ChartData->UpdateChartClassDescriptorArray();

	return bret;
}

bool PlugInManager::DeactivatePlugIn(PlugInContainer* pic)
{
	bool bret = false;

	if (pic) {
		wxString msg(_T("PlugInManager: Deactivating PlugIn: "));
		msg += pic->m_plugin_file;
		wxLogMessage(msg);

		pic->m_pplugin->DeInit();

		// Deactivate (Remove) any ToolbarTools added by this PlugIn
		for (unsigned int i = 0; i < m_PlugInToolbarTools.size(); i++) {
			PlugInToolbarToolContainer* pttc = m_PlugInToolbarTools.Item(i);

			if (pttc->m_pplugin == pic->m_pplugin) {
				m_PlugInToolbarTools.Remove(pttc);
				delete pttc;
			}
		}

		// Deactivate (Remove) any ContextMenu items addded by this PlugIn
		for (unsigned int i = 0; i < m_PlugInMenuItems.size(); i++) {
			PlugInMenuItemContainer* pimis = m_PlugInMenuItems.Item(i);
			if (pimis->m_pplugin == pic->m_pplugin) {
				m_PlugInMenuItems.Remove(pimis);
				delete pimis;
			}
		}

		pic->m_bInitState = false;
		bret = true;
	}

	return bret;
}

bool PlugInManager::UpdateConfig()
{
	pConfig->SetPath(_T("/"));

	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);

		wxString config_section = (_T ( "/PlugIns/" ));
		config_section += pic->m_common_name;
		pConfig->SetPath(config_section);
		pConfig->Write(_T ( "bEnabled" ), pic->m_bEnabled);
	}

	return true;
}

bool PlugInManager::UnLoadAllPlugIns()
{
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		wxString msg(_T("PlugInManager: UnLoading PlugIn: "));
		msg += pic->m_plugin_file;
		wxLogMessage(msg);

		pic->m_destroy_fn(pic->m_pplugin);
		delete pic->m_plibrary; // This will unload the PlugIn
		pic->m_bInitState = false;
		delete pic;
	}
	return true;
}

bool PlugInManager::DeactivateAllPlugIns()
{
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic && pic->m_bEnabled && pic->m_bInitState)
			DeactivatePlugIn(pic);
	}
	return true;
}

bool PlugInManager::CheckPluginCompatibility(wxString plugin_file)
{
	bool b_compat = true;

#ifdef __WXMSW__

	//    Open the dll, and get the manifest
	HMODULE module = ::LoadLibraryEx(plugin_file.fn_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (module == NULL)
		return false;
	HRSRC resInfo = ::FindResource(module, MAKEINTRESOURCE(1),
								   RT_MANIFEST); // resource id #1 should be the manifest

	if (!resInfo)
		resInfo = ::FindResource(module, MAKEINTRESOURCE(2), RT_MANIFEST); // try resource id #2

	if (resInfo) {
		HGLOBAL resData = ::LoadResource(module, resInfo);
		DWORD resSize = ::SizeofResource(module, resInfo);
		if (resData && resSize) {
			const char* res = (const char*)::LockResource(resData); // the manifest
			if (res) {
				// got the manifest as a char *
				wxString manifest(res, wxConvUTF8);
				if (wxNOT_FOUND
					!= manifest.Find(_T("VC90.CRT"))) // cannot load with VC90 runtime (i.e. VS2008)
					b_compat = false;
			}
			UnlockResource(resData);
		}
		::FreeResource(resData);
	}
	::FreeLibrary(module);

#else
	(void)plugin_file; // prevent compiler warning
#endif

	return b_compat;
}

bool PlugInManager::CheckBlacklistedPlugin(opencpn_plugin* plugin)
{
	int len = sizeof(PluginBlacklist) / sizeof(BlackListedPlugin);
	int major = plugin->GetPlugInVersionMajor();
	int minor = plugin->GetPlugInVersionMinor();
	wxString name = wxString::FromAscii(typeid(*plugin).name());
	for (int i = 0; i < len; i++) {
		if ((PluginBlacklist[i].all_lower && name.EndsWith(PluginBlacklist[i].name)
			 && PluginBlacklist[i].version_major >= major
			 && PluginBlacklist[i].version_minor >= minor)
			|| (!PluginBlacklist[i].all_lower && name.EndsWith(PluginBlacklist[i].name)
				&& PluginBlacklist[i].version_major == major
				&& PluginBlacklist[i].version_minor == minor)) {
			wxString msg;
			wxString msg1;
			if (PluginBlacklist[i].hard) {
				msg = wxString::Format(
					_("PlugIn %s (%s), version %i.%i was detected.\n This version is known to be unstable and will not be loaded.\n Please update this PlugIn at the opencpn.org website."),
					PluginBlacklist[i].name.c_str(), plugin->GetCommonName().c_str(), major, minor),
				_("Blacklisted plugin detected...");
				msg1 = wxString::Format(_T("    PlugIn %s (%s), version %i.%i was detected. Hard ")
										_T("blacklisted. Not loaded."),
										PluginBlacklist[i].name.c_str(),
										plugin->GetCommonName().c_str(), major, minor);
			} else {
				msg = wxString::Format(
					_("PlugIn %s (%s), version %i.%i was detected.\n This version is known to be unstable.\n Please update this PlugIn at the opencpn.org website."),
					PluginBlacklist[i].name.c_str(), plugin->GetCommonName().c_str(), major, minor),
				_("Blacklisted plugin detected...");
				msg1 = wxString::Format(
					_T("    PlugIn %s (%s), version %i.%i was detected. Soft blacklisted. Loaded."),
					PluginBlacklist[i].name.c_str(), plugin->GetCommonName().c_str(), major, minor);
			}

			wxLogMessage(msg1);
			OCPNMessageBox(NULL, msg, wxString(_("OpenCPN Info")), wxICON_INFORMATION | wxOK, 5);
			return PluginBlacklist[i].hard;
		}
	}
	return false;
}

PlugInContainer* PlugInManager::LoadPlugIn(wxString plugin_file)
{
	wxLogMessage(_T("PlugInManager: Loading PlugIn: ") + plugin_file);

	PlugInContainer* pic = new PlugInContainer;
	pic->m_plugin_file = plugin_file;

	// load the library
	wxDynamicLibrary* plugin = new wxDynamicLibrary(plugin_file);
	pic->m_plibrary = plugin; // Save a pointer to the wxDynamicLibrary for later deletion

	if (!plugin->IsLoaded()) {
		wxString msg(_T("   PlugInManager: Cannot load library: "));
		msg += plugin_file;
		msg += _T(" ");
#if !defined(__WXMSW__) && !defined(__WXOSX__)
		/* give good error reporting on non windows non mac platforms */
		dlopen(plugin_file.ToAscii(), RTLD_NOW);
		char* s = dlerror();
		wxString c;
		for (char* t = s; *t; t++)
			c += *t;
		msg += c;
#endif
		wxLogMessage(msg);
		delete plugin;
		delete pic;
		return NULL;
	}

	// load the factory symbols
	create_t* create_plugin = (create_t*)plugin->GetSymbol(_T("create_pi"));
	if (NULL == create_plugin) {
		wxString msg(_T("   PlugInManager: Cannot load symbol create_pi: "));
		msg += plugin_file;
		wxLogMessage(msg);
		delete plugin;
		delete pic;
		return NULL;
	}

	destroy_t* destroy_plugin = (destroy_t*)plugin->GetSymbol(_T("destroy_pi"));
	pic->m_destroy_fn = destroy_plugin;
	if (NULL == destroy_plugin) {
		wxString msg(_T("   PlugInManager: Cannot load symbol destroy_pi: "));
		msg += plugin_file;
		wxLogMessage(msg);
		delete plugin;
		delete pic;
		return NULL;
	}

	// create an instance of the plugin class
	opencpn_plugin* plug_in = create_plugin(this);

	int api_major = plug_in->GetAPIVersionMajor();
	int api_minor = plug_in->GetAPIVersionMinor();
	int ver = (api_major * 100) + api_minor;
	pic->m_api_version = ver;

	int pi_major = plug_in->GetPlugInVersionMajor();
	int pi_minor = plug_in->GetPlugInVersionMinor();
	int pi_ver = (pi_major * 100) + pi_minor;

	if (CheckBlacklistedPlugin(plug_in)) {
		delete plugin;
		delete pic;
		return NULL;
	}

	switch (ver) {
		case 105:
			pic->m_pplugin = dynamic_cast<opencpn_plugin*>(plug_in);
			break;

		case 106:
			pic->m_pplugin = dynamic_cast<opencpn_plugin_16*>(plug_in);
			break;

		case 107:
			pic->m_pplugin = dynamic_cast<opencpn_plugin_17*>(plug_in);
			break;

		case 108:
			pic->m_pplugin = dynamic_cast<opencpn_plugin_18*>(plug_in);
			break;

		case 109:
			pic->m_pplugin = dynamic_cast<opencpn_plugin_19*>(plug_in);
			break;

		case 110:
			pic->m_pplugin = dynamic_cast<opencpn_plugin_110*>(plug_in);
			break;

		case 111:
			pic->m_pplugin = dynamic_cast<opencpn_plugin_111*>(plug_in);
			break;

		default:
			break;
	}

	if (pic->m_pplugin) {
		wxString msg = _T("  ");
		msg += plugin_file;
		wxString msg1;
		msg1.Printf(_T("\n              API Version detected: %d"), ver);
		msg += msg1;
		msg1.Printf(_T("\n              PlugIn Version detected: %d"), pi_ver);
		msg += msg1;
		wxLogMessage(msg);
	} else {
		wxLogMessage(_T("    ") + plugin_file + _T(" cannot be loaded"));
	}

	return pic;
}

bool PlugInManager::RenderAllCanvasOverlayPlugIns(ocpnDC& dc, const ViewPort& vp)
{
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState) {
			if (pic->m_cap_flag & WANTS_OVERLAY_CALLBACK) {
				PlugIn_ViewPort pivp = CreatePlugInViewport(vp);

				wxDC* pdc = dc.GetDC();
				if (pdc) {
					// not in OpenGL mode
					switch (pic->m_api_version) {
						case 106: {
							opencpn_plugin_16* ppi = dynamic_cast
								<opencpn_plugin_16*>(pic->m_pplugin);
							if (ppi)
								ppi->RenderOverlay(*pdc, &pivp);
							break;
						}
						case 107: {
							opencpn_plugin_17* ppi = dynamic_cast
								<opencpn_plugin_17*>(pic->m_pplugin);
							if (ppi)
								ppi->RenderOverlay(*pdc, &pivp);
							break;
						}
						case 108:
						case 109:
						case 110: {
							opencpn_plugin_18* ppi = dynamic_cast
								<opencpn_plugin_18*>(pic->m_pplugin);
							if (ppi)
								ppi->RenderOverlay(*pdc, &pivp);
							break;
						}

						default:
							break;
					}
				} else {
					// If in OpenGL mode, and the PlugIn has requested OpenGL render callbacks,
					// then there is no need to render by wxDC here.
					if (pic->m_cap_flag & WANTS_OPENGL_OVERLAY_CALLBACK)
						continue;

					if ((m_cached_overlay_bm.GetWidth() != vp.pix_width)
						|| (m_cached_overlay_bm.GetHeight() != vp.pix_height))
						m_cached_overlay_bm.Create(vp.pix_width, vp.pix_height, -1);

					wxMemoryDC mdc;
					mdc.SelectObject(m_cached_overlay_bm);
					mdc.SetBackground(*wxBLACK_BRUSH);
					mdc.Clear();

					bool b_rendered = false;

					switch (pic->m_api_version) {
						case 106: {
							opencpn_plugin_16* ppi = dynamic_cast
								<opencpn_plugin_16*>(pic->m_pplugin);
							if (ppi)
								b_rendered = ppi->RenderOverlay(mdc, &pivp);
							break;
						}
						case 107: {
							opencpn_plugin_17* ppi = dynamic_cast
								<opencpn_plugin_17*>(pic->m_pplugin);
							if (ppi)
								b_rendered = ppi->RenderOverlay(mdc, &pivp);
							break;
						}
						case 108:
						case 109:
						case 110: {
							opencpn_plugin_18* ppi = dynamic_cast
								<opencpn_plugin_18*>(pic->m_pplugin);
							if (ppi)
								b_rendered = ppi->RenderOverlay(mdc, &pivp);
							break;
						}

						default: {
							b_rendered = pic->m_pplugin->RenderOverlay(&mdc, &pivp);
							break;
						}
					}

					mdc.SelectObject(wxNullBitmap);

					if (b_rendered) {
						wxMask* p_msk = new wxMask(m_cached_overlay_bm, wxColour(0, 0, 0));
						m_cached_overlay_bm.SetMask(p_msk);

						dc.DrawBitmap(m_cached_overlay_bm, 0, 0, true);
					}
				}
			} else if (pic->m_cap_flag & WANTS_OPENGL_OVERLAY_CALLBACK) {
				// ??
			}
		}
	}

	return true;
}

bool PlugInManager::RenderAllGLCanvasOverlayPlugIns(wxGLContext* pcontext, const ViewPort& vp)
{
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState) {
			if (pic->m_cap_flag & WANTS_OPENGL_OVERLAY_CALLBACK) {
				PlugIn_ViewPort pivp = CreatePlugInViewport(vp);

				switch (pic->m_api_version) {
					case 107: {
						opencpn_plugin_17* ppi = dynamic_cast<opencpn_plugin_17*>(pic->m_pplugin);
						if (ppi)
							ppi->RenderGLOverlay(pcontext, &pivp);
						break;
					}

					case 108:
					case 109:
					case 110: {
						opencpn_plugin_18* ppi = dynamic_cast<opencpn_plugin_18*>(pic->m_pplugin);
						if (ppi)
							ppi->RenderGLOverlay(pcontext, &pivp);
						break;
					}
					default:
						break;
				}
			}
		}
	}

	return true;
}

void PlugInManager::SendViewPortToRequestingPlugIns(const ViewPort& vp)
{
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState) {
			if (pic->m_cap_flag & WANTS_ONPAINT_VIEWPORT) {
				PlugIn_ViewPort pivp = CreatePlugInViewport(vp);
				pic->m_pplugin->SetCurrentViewPort(pivp);
			}
		}
	}
}

void PlugInManager::SendCursorLatLonToAllPlugIns(double lat, double lon)
{
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState) {
			if (pic->m_cap_flag & WANTS_CURSOR_LATLON)
				pic->m_pplugin->SetCursorLatLon(lat, lon);
		}
	}
}

void NotifySetupOptionsPlugin(PlugInContainer* pic)
{
	if (pic->m_bEnabled && pic->m_bInitState) {
		if (pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE) {
			switch (pic->m_api_version) {
				case 109:
				case 110: {
					opencpn_plugin_19* ppi = dynamic_cast<opencpn_plugin_19*>(pic->m_pplugin);
					if (ppi) {
						ppi->OnSetupOptions();
						pic->m_bToolboxPanel = true;
					}
					break;
				}
				default:
					break;
			}
		}
	}
}

void PlugInManager::NotifySetupOptions()
{
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		NotifySetupOptionsPlugin(pic);
	}
}

void PlugInManager::CloseAllPlugInPanels(int ok_apply_cancel)
{
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState) {
			if ((pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE) && (pic->m_bToolboxPanel)) {
				pic->m_pplugin->OnCloseToolboxPanel(0, ok_apply_cancel);
				pic->m_bToolboxPanel = false;
			}
		}
	}
}

int PlugInManager::AddCanvasContextMenuItem(wxMenuItem* pitem, opencpn_plugin* pplugin)
{
	PlugInMenuItemContainer* pmic = new PlugInMenuItemContainer;
	pmic->pmenu_item = pitem;
	pmic->m_pplugin = pplugin;
	pmic->id = pitem->GetId() == wxID_SEPARATOR ? wxID_SEPARATOR : m_plugin_menu_item_id_next;
	pmic->b_viz = true;
	pmic->b_grey = false;

	m_PlugInMenuItems.Add(pmic);

	m_plugin_menu_item_id_next++;

	return pmic->id;
}

void PlugInManager::RemoveCanvasContextMenuItem(int item)
{
	for (unsigned int i = 0; i < m_PlugInMenuItems.size(); i++) {
		PlugInMenuItemContainer* pimis = m_PlugInMenuItems.Item(i);
		if (pimis->id == item) {
			m_PlugInMenuItems.Remove(pimis);
			delete pimis;
			break;
		}
	}
}

void PlugInManager::SetCanvasContextMenuItemViz(int item, bool viz)
{
	for (unsigned int i = 0; i < m_PlugInMenuItems.size(); i++) {
		PlugInMenuItemContainer* pimis = m_PlugInMenuItems.Item(i);
		if (pimis->id == item) {
			pimis->b_viz = viz;
			break;
		}
	}
}

void PlugInManager::SetCanvasContextMenuItemGrey(int item, bool grey)
{
	for (unsigned int i = 0; i < m_PlugInMenuItems.size(); i++) {
		PlugInMenuItemContainer* pimis = m_PlugInMenuItems.Item(i);
		if (pimis->id == item) {
			pimis->b_grey = grey;
			break;
		}
	}
}

void PlugInManager::SendNMEASentenceToAllPlugIns(const wxString& sentence)
{
	// decouples 'const wxString &' and 'wxString &' to keep bin compat for plugins
	wxString decouple_sentence(sentence);
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState) {
			if (pic->m_cap_flag & WANTS_NMEA_SENTENCES)
				pic->m_pplugin->SetNMEASentence(decouple_sentence);
		}
	}
}

void PlugInManager::SendJSONMessageToAllPlugins(const wxString& message_id, wxJSONValue v)
{
	wxJSONWriter w;
	wxString out;
	w.Write(v, out);
	SendMessageToAllPlugins(message_id, out);
}

void PlugInManager::SendMessageToAllPlugins(const wxString& message_id,
											const wxString& message_body)
{
	// decouples 'const wxString &' and 'wxString &' to keep bin compat for plugins
	wxString decouple_message_id(message_id);
	wxString decouple_message_body(message_body);

	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState) {
			if (pic->m_cap_flag & WANTS_PLUGIN_MESSAGING) {
				switch (pic->m_api_version) {
					case 106: {
						opencpn_plugin_16* ppi = dynamic_cast<opencpn_plugin_16*>(pic->m_pplugin);
						if (ppi)
							ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
						break;
					}
					case 107: {
						opencpn_plugin_17* ppi = dynamic_cast<opencpn_plugin_17*>(pic->m_pplugin);
						if (ppi)
							ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
						break;
					}
					case 108:
					case 109:
					case 110:
					case 111: {
						opencpn_plugin_18* ppi = dynamic_cast<opencpn_plugin_18*>(pic->m_pplugin);
						if (ppi)
							ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
						break;
					}
					default:
						break;
				}
			}
		}
	}
}

void PlugInManager::SendAISSentenceToAllPlugIns(const wxString& sentence)
{
	wxString decouple_sentence(
		sentence); // decouples 'const wxString &' and 'wxString &' to keep bin compat for plugins
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState) {
			if (pic->m_cap_flag & WANTS_AIS_SENTENCES)
				pic->m_pplugin->SetAISSentence(decouple_sentence);
		}
	}
}

void PlugInManager::SendPositionFixToAllPlugIns(GenericPosDatEx* ppos)
{
	// Send basic position fix
	PlugIn_Position_Fix pfix;
	pfix.Lat = ppos->kLat;
	pfix.Lon = ppos->kLon;
	pfix.Cog = ppos->kCog;
	pfix.Sog = ppos->kSog;
	pfix.Var = ppos->kVar;
	pfix.FixTime = ppos->FixTime;
	pfix.nSats = ppos->nSats;

	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState) {
			if (pic->m_cap_flag & WANTS_NMEA_EVENTS)
				pic->m_pplugin->SetPositionFix(pfix);
		}
	}

	// Send extended position fix to PlugIns at API 108 and later
	PlugIn_Position_Fix_Ex pfix_ex;
	pfix_ex.Lat = ppos->kLat;
	pfix_ex.Lon = ppos->kLon;
	pfix_ex.Cog = ppos->kCog;
	pfix_ex.Sog = ppos->kSog;
	pfix_ex.Var = ppos->kVar;
	pfix_ex.FixTime = ppos->FixTime;
	pfix_ex.nSats = ppos->nSats;
	pfix_ex.Hdt = ppos->kHdt;
	pfix_ex.Hdm = ppos->kHdm;

	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState) {
			if (pic->m_cap_flag & WANTS_NMEA_EVENTS) {
				switch (pic->m_api_version) {
					case 108:
					case 109:
					case 110: {
						opencpn_plugin_18* ppi = dynamic_cast<opencpn_plugin_18*>(pic->m_pplugin);
						if (ppi)
							ppi->SetPositionFixEx(pfix_ex);
						break;
					}

					default:
						break;
				}
			}
		}
	}
}

void PlugInManager::SendResizeEventToAllPlugIns(int x, int y)
{
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState)
			pic->m_pplugin->ProcessParentResize(x, y);
	}
}

void PlugInManager::SetColorSchemeForAllPlugIns(global::ColorScheme cs)
{
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState)
			pic->m_pplugin->SetColorScheme((PI_ColorScheme)cs);
	}
}

void PlugInManager::NotifyAuiPlugIns(void)
{
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState && (pic->m_cap_flag & USES_AUI_MANAGER))
			pic->m_pplugin->UpdateAuiStatus();
	}
}

int PlugInManager::AddToolbarTool(wxString label, wxBitmap* bitmap, wxBitmap* WXUNUSED(bmpDisabled),
								  wxItemKind kind, wxString shortHelp, wxString longHelp,
								  wxObject* clientData, int position, int tool_sel,
								  opencpn_plugin* pplugin)
{
	PlugInToolbarToolContainer* pttc = new PlugInToolbarToolContainer;
	pttc->label = label;

	if (!bitmap->IsOk()) {
		pttc->bitmap_day = new wxBitmap(global::OCPN::get().styleman().current().GetIcon(_T("default_pi")));
	} else {
		// Force a non-reference copy of the bitmap from the PlugIn
		pttc->bitmap_day = new wxBitmap(*bitmap);
		pttc->bitmap_day->UnShare();
	}

	pttc->bitmap_Rollover = new wxBitmap(*pttc->bitmap_day);
	pttc->bitmap_Rollover->UnShare();

	pttc->bitmap_dusk = BuildDimmedToolBitmap(pttc->bitmap_day, 128);
	pttc->bitmap_night = BuildDimmedToolBitmap(pttc->bitmap_day, 32);

	pttc->kind = kind;
	pttc->shortHelp = shortHelp;
	pttc->longHelp = longHelp;
	pttc->clientData = clientData;
	pttc->position = position;
	pttc->m_pplugin = pplugin;
	pttc->tool_sel = tool_sel;
	pttc->b_viz = true;
	pttc->b_toggle = false;
	pttc->id = m_plugin_tool_id_next;

	m_PlugInToolbarTools.Add(pttc);

	m_plugin_tool_id_next++;

	pParent->RequestNewToolbar();

	return pttc->id;
}

void PlugInManager::RemoveToolbarTool(int tool_id)
{
	for (unsigned int i = 0; i < m_PlugInToolbarTools.size(); i++) {
		PlugInToolbarToolContainer* pttc = m_PlugInToolbarTools.Item(i);
		if (pttc->id == tool_id) {
			m_PlugInToolbarTools.Remove(pttc);
			delete pttc;
			break;
		}
	}

	pParent->RequestNewToolbar();
}

void PlugInManager::SetToolbarToolViz(int item, bool viz)
{
	for (unsigned int i = 0; i < m_PlugInToolbarTools.size(); i++) {
		PlugInToolbarToolContainer* pttc = m_PlugInToolbarTools.Item(i);
		if (pttc->id == item) {
			pttc->b_viz = viz;
			break;
		}
	}
}

void PlugInManager::SetToolbarItemState(int item, bool toggle)
{
	for (unsigned int i = 0; i < m_PlugInToolbarTools.size(); i++) {
		PlugInToolbarToolContainer* pttc = m_PlugInToolbarTools.Item(i);
		if (pttc->id == item) {
			pttc->b_toggle = toggle;
			pParent->SetToolbarItemState(item, toggle);
			break;
		}
	}
}

void PlugInManager::SetToolbarItemBitmaps(int item, wxBitmap* bitmap, wxBitmap* bmpRollover)
{
	gui::StyleManager& styleman = global::OCPN::get().styleman();

	for (unsigned int i = 0; i < m_PlugInToolbarTools.size(); i++) {
		PlugInToolbarToolContainer* pttc = m_PlugInToolbarTools.Item(i);
		if (pttc->id == item) {
			delete pttc->bitmap_day;
			delete pttc->bitmap_dusk;
			delete pttc->bitmap_night;
			delete pttc->bitmap_Rollover;

			if (!bitmap->IsOk()) {
				pttc->bitmap_day = new wxBitmap(styleman.current().GetIcon(_T("default_pi")));
			} else {
				// Force a non-reference copy of the bitmap from the PlugIn
				pttc->bitmap_day = new wxBitmap(*bitmap);
				pttc->bitmap_day->UnShare();
			}

			if (!bmpRollover->IsOk()) {
				pttc->bitmap_Rollover = new wxBitmap(styleman.current().GetIcon(_T("default_pi")));
			} else {
				// Force a non-reference copy of the bitmap from the PlugIn
				pttc->bitmap_Rollover = new wxBitmap(*bmpRollover);
				pttc->bitmap_Rollover->UnShare();
			}

			pttc->bitmap_dusk = BuildDimmedToolBitmap(pttc->bitmap_day, 128);
			pttc->bitmap_night = BuildDimmedToolBitmap(pttc->bitmap_day, 32);

			pParent->SetToolbarItemBitmaps(item, pttc->bitmap_day, pttc->bitmap_Rollover);
			break;
		}
	}
}

opencpn_plugin* PlugInManager::FindToolOwner(const int id)
{
	for (unsigned int i = 0; i < m_PlugInToolbarTools.size(); i++) {
		PlugInToolbarToolContainer* pc = m_PlugInToolbarTools.Item(i);
		if (id == pc->id)
			return pc->m_pplugin;
	}

	return NULL;
}

wxString PlugInManager::GetToolOwnerCommonName(const int id)
{
	opencpn_plugin* ppi = FindToolOwner(id);
	if (ppi) {
		for (unsigned int i = 0; i < plugin_array.size(); i++) {
			PlugInContainer* pic = plugin_array.Item(i);
			if (pic && (pic->m_pplugin == ppi))
				return pic->m_common_name;
		}
	}

	return wxEmptyString;
}

wxString PlugInManager::GetLastError()
{
	return m_last_error_string;
}

wxBitmap* PlugInManager::BuildDimmedToolBitmap(wxBitmap* pbmp_normal, unsigned char dim_ratio)
{
	wxImage img_dup = pbmp_normal->ConvertToImage();

	if (!img_dup.IsOk())
		return NULL;

	if (dim_ratio < 200) {
		//  Create a dimmed version of the image/bitmap
		int gimg_width = img_dup.GetWidth();
		int gimg_height = img_dup.GetHeight();

		double factor = (double)(dim_ratio) / 256.0;

		for (int iy = 0; iy < gimg_height; iy++) {
			for (int ix = 0; ix < gimg_width; ix++) {
				if (!img_dup.IsTransparent(ix, iy)) {
					wxImage::RGBValue rgb(img_dup.GetRed(ix, iy), img_dup.GetGreen(ix, iy),
										  img_dup.GetBlue(ix, iy));
					wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
					hsv.value = hsv.value * factor;
					wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
					img_dup.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);
				}
			}
		}
	}

	//  Make a bitmap
	wxBitmap* ptoolBarBitmap;

#ifdef __WXMSW__
	wxBitmap tbmp(img_dup.GetWidth(), img_dup.GetHeight(), -1);
	wxMemoryDC dwxdc;
	dwxdc.SelectObject(tbmp);

	ptoolBarBitmap = new wxBitmap(img_dup, (wxDC&)dwxdc);
#else
	ptoolBarBitmap = new wxBitmap(img_dup);
#endif

	// store it
	return ptoolBarBitmap;
}

wxArrayString PlugInManager::GetPlugInChartClassNameArray(void)
{
	wxArrayString array;
	for (unsigned int i = 0; i < plugin_array.size(); i++) {
		PlugInContainer* pic = plugin_array.Item(i);
		if (pic->m_bEnabled && pic->m_bInitState
			&& ((pic->m_cap_flag & INSTALLS_PLUGIN_CHART)
				|| (pic->m_cap_flag & INSTALLS_PLUGIN_CHART_GL))) {
			wxArrayString carray = pic->m_pplugin->GetDynamicChartClassNameArray();

			for (unsigned int j = 0; j < carray.size(); j++)
				array.Add(carray.Item(j));
		}
	}

	// Scrub the list for duplicates
	// Corrects a flaw in BSB4 and NVC PlugIns
	unsigned int j = 0;
	while (j < array.size()) {
		wxString test = array.Item(j);
		unsigned int k = j + 1;
		while (k < array.size()) {
			if (test == array.Item(k)) {
				array.RemoveAt(k);
				j = -1;
				break;
			} else
				k++;
		}

		j++;
	}

	return array;
}

//----------------------------------------------------------------------------------------------------------
//    The PlugIn CallBack API Implementation
//    The definitions of this API are found in ocpn_plugin.h
//----------------------------------------------------------------------------------------------------------

int InsertPlugInTool(wxString label, wxBitmap* bitmap, wxBitmap* bmpDisabled, wxItemKind kind,
					 wxString shortHelp, wxString longHelp, wxObject* clientData, int position,
					 int tool_sel, opencpn_plugin* pplugin)
{
	if (s_ppim)
		return s_ppim->AddToolbarTool(label, bitmap, bmpDisabled, kind, shortHelp, longHelp,
									  clientData, position, tool_sel, pplugin);
	else
		return -1;
}

void RemovePlugInTool(int tool_id)
{
	if (s_ppim)
		s_ppim->RemoveToolbarTool(tool_id);
}

void SetToolbarToolViz(int item, bool viz)
{
	if (s_ppim)
		s_ppim->SetToolbarToolViz(item, viz);
}

void SetToolbarItemState(int item, bool toggle)
{
	if (s_ppim)
		s_ppim->SetToolbarItemState(item, toggle);
}

void SetToolbarToolBitmaps(int item, wxBitmap* bitmap, wxBitmap* bmprollover)
{
	if (s_ppim)
		s_ppim->SetToolbarItemBitmaps(item, bitmap, bmprollover);
}

int AddCanvasContextMenuItem(wxMenuItem* pitem, opencpn_plugin* pplugin)
{
	if (s_ppim)
		return s_ppim->AddCanvasContextMenuItem(pitem, pplugin);
	else
		return -1;
}

void SetCanvasContextMenuItemViz(int item, bool viz)
{
	if (s_ppim)
		s_ppim->SetCanvasContextMenuItemViz(item, viz);
}

void SetCanvasContextMenuItemGrey(int item, bool grey)
{
	if (s_ppim)
		s_ppim->SetCanvasContextMenuItemGrey(item, grey);
}

void RemoveCanvasContextMenuItem(int item)
{
	if (s_ppim)
		s_ppim->RemoveCanvasContextMenuItem(item);
}

wxFileConfig* GetOCPNConfigObject(void)
{
	if (s_ppim)
		return pConfig; // return the global application config object
	else
		return NULL;
}

wxWindow* GetOCPNCanvasWindow()
{
	wxWindow* pret = NULL;
	if (s_ppim) {
		MainFrame* pFrame = s_ppim->GetParentFrame();
		pret = (wxWindow*)pFrame->GetCanvas();
	}
	return pret;
}

void RequestRefresh(wxWindow* win)
{
	if (win)
		win->Refresh();
}

void GetCanvasPixLL(PlugIn_ViewPort* vp, wxPoint* pp, double lat, double lon)
{
	// Make enough of an application viewport to run its method....
	ViewPort ocpn_vp;
	ocpn_vp.set_position(geo::Position(vp->clat, vp->clon));
	ocpn_vp.m_projection_type = vp->m_projection_type;
	ocpn_vp.set_view_scale(vp->view_scale_ppm);
	ocpn_vp.skew = vp->skew;
	ocpn_vp.rotation = vp->rotation;
	ocpn_vp.pix_width = vp->pix_width;
	ocpn_vp.pix_height = vp->pix_height;

	wxPoint ret = ocpn_vp.GetPixFromLL(geo::Position(lat, lon));
	pp->x = ret.x;
	pp->y = ret.y;
}

void GetCanvasLLPix(PlugIn_ViewPort* vp, wxPoint p, double* plat, double* plon)
{
	// Make enough of an application viewport to run its method....
	ViewPort ocpn_vp;
	ocpn_vp.set_position(geo::Position(vp->clat, vp->clon));
	ocpn_vp.m_projection_type = vp->m_projection_type;
	ocpn_vp.set_view_scale(vp->view_scale_ppm);
	ocpn_vp.skew = vp->skew;
	ocpn_vp.rotation = vp->rotation;
	ocpn_vp.pix_width = vp->pix_width;
	ocpn_vp.pix_height = vp->pix_height;

	geo::Position pos = ocpn_vp.GetLLFromPix(p);
	*plat = pos.lat();
	*plon = pos.lon();
}

bool GetGlobalColor(wxString colorName, wxColour* pcolour)
{
	*pcolour = global::OCPN::get().color().get_color(colorName);
	return true;
}

wxFont* OCPNGetFont(wxString TextElement, int default_size)
{
	return global::OCPN::get().font().GetFont(TextElement, default_size);
}

wxString* GetpSharedDataLocation(void) // FIXME: stupid interface, really, I mean it
{
	static wxString interface_adapter = _T("");

	interface_adapter = global::OCPN::get().sys().data().sound_data_location;
	return &interface_adapter;
}

ArrayOfPlugIn_AIS_Targets* GetAISTargetArray(void)
{
	using ais::AIS_Target_Hash;
	using ais::AIS_Target_Data;

	if (!g_pAIS)
		return NULL;

	ArrayOfPlugIn_AIS_Targets* pret = new ArrayOfPlugIn_AIS_Targets;

	// Iterate over the AIS Target Hashmap
	AIS_Target_Hash::iterator it;

	AIS_Target_Hash* current_targets = g_pAIS->GetTargetList();

	for (it = (*current_targets).begin(); it != (*current_targets).end(); ++it) {
		AIS_Target_Data* td = it->second;
		PlugIn_AIS_Target* ptarget = Create_PI_AIS_Target(td);
		pret->Add(ptarget);
	}

	return pret;
}

wxAuiManager* GetFrameAuiManager(void)
{
	return g_pauimgr;
}

bool AddLocaleCatalog(wxString catalog)
{
	if (plocale_def_lang)
		return plocale_def_lang->AddCatalog(catalog);
	else
		return false;
}

void PushNMEABuffer(wxString buf)
{
	OCPN_DataStreamEvent event(wxEVT_OCPN_DATASTREAM, 0);
	std::string s = std::string(buf.mb_str());
	event.SetNMEAString(s);
	event.SetStream(NULL);

	g_pMUX->AddPendingEvent(event);
}

wxXmlDocument GetChartDatabaseEntryXML(int dbIndex, bool b_getGeom)
{
	wxXmlDocument doc = ChartData->GetXMLDescription(dbIndex, b_getGeom);
	return doc;
}

bool UpdateChartDBInplace(wxArrayString dir_array, bool b_force_update, bool b_ProgressDialog)
{
	// Make an array of CDI
	ChartDirectories ChartDirArray;
	for (unsigned int i = 0; i < dir_array.size(); i++) {
		ChartDirArray.push_back(ChartDirectoryInfo(dir_array.Item(i), _T("")));
	}

	bool b_ret = gFrame->UpdateChartDatabaseInplace(ChartDirArray, b_force_update, b_ProgressDialog,
													ChartData->GetDBFileName());

	ViewPort vp;
	gFrame->ChartsRefresh(-1, vp);

	return b_ret;
}

wxArrayString GetChartDBDirArrayString()
{
	return ChartData->GetChartDirArrayString();
}

int AddChartToDBInPlace(wxString& full_path, bool WXUNUSED(b_ProgressDialog))
{
	// extract the path from the chart name
	wxFileName fn(full_path);
	wxString fdir = fn.GetPath();

	bool bret = false;
	if (ChartData) {

		bret = ChartData->AddSingleChart(full_path);

		if (bret) {
			// Save to disk
			pConfig->UpdateChartDirs(ChartData->GetChartDirArray());
			ChartData->SaveBinary(global::OCPN::get().sys().data().chartlist_filename);

			// Completely reload the chart database, for a fresh start
			ChartDirectories XnewChartDirArray;
			pConfig->LoadChartDirArray(XnewChartDirArray);
			delete ChartData;
			ChartData = new chart::ChartDB(gFrame);
			ChartData->LoadBinary(global::OCPN::get().sys().data().chartlist_filename, XnewChartDirArray);

			if (g_options) {
				g_options->UpdateDisplayedChartDirList(ChartData->GetChartDirArray());
			}

			ViewPort vp;
			gFrame->ChartsRefresh(-1, vp);
		}
	}
	return bret;
}

int RemoveChartFromDBInPlace(wxString& full_path)
{
	bool bret = false;
	if (ChartData) {
		bret = ChartData->RemoveSingleChart(full_path);

		// Save to disk
		pConfig->UpdateChartDirs(ChartData->GetChartDirArray());
		ChartData->SaveBinary(global::OCPN::get().sys().data().chartlist_filename);

		// Completely reload the chart database, for a fresh start
		ChartDirectories XnewChartDirArray;
		pConfig->LoadChartDirArray(XnewChartDirArray);
		delete ChartData;
		ChartData = new chart::ChartDB(gFrame);
		ChartData->LoadBinary(global::OCPN::get().sys().data().chartlist_filename, XnewChartDirArray);

		if (g_options) {
			g_options->UpdateDisplayedChartDirList(ChartData->GetChartDirArray());
		}

		ViewPort vp;
		gFrame->ChartsRefresh(-1, vp);
	}

	return bret;
}

void SendPluginMessage(wxString message_id, wxString message_body)
{
	s_ppim->SendMessageToAllPlugins(message_id, message_body);

	// We will send an event to the main application frame (gFrame)
	// for informational purposes.
	// Of course, gFrame is encouraged to use any or all the
	// data flying by if judged useful and dependable....

	OCPN_MsgEvent Nevent(wxEVT_OCPN_MSG, 0);
	Nevent.SetID(message_id);
	Nevent.SetJSONText(message_body);
	gFrame->GetEventHandler()->AddPendingEvent(Nevent);
}

void DimeWindow(wxWindow* win)
{
	DimeControl(win);
}

void JumpToPosition(double lat, double lon, double scale)
{
	gFrame->JumpToPosition(geo::Position(lat, lon), scale);
}

/* API 1.9 */
wxScrolledWindow* AddOptionsPage(OptionsParentPI parent, wxString title)
{
	if (!g_pOptions)
		return NULL;

	size_t parentid;
	switch (parent) {
		case PI_OPTIONS_PARENT_DISPLAY:
			parentid = g_pOptions->get_pageDisplay();
			break;
		case PI_OPTIONS_PARENT_CONNECTIONS:
			parentid = g_pOptions->get_pageConnections();
			break;
		case PI_OPTIONS_PARENT_CHARTS:
			parentid = g_pOptions->get_pageCharts();
			break;
		case PI_OPTIONS_PARENT_SHIPS:
			parentid = g_pOptions->get_pageShips();
			break;
		case PI_OPTIONS_PARENT_UI:
			parentid = g_pOptions->get_pageUI();
			break;
		case PI_OPTIONS_PARENT_PLUGINS:
			parentid = g_pOptions->get_pagePlugins();
			break;
		default:
			wxLogMessage(_T("Error in PluginManager::AddOptionsPage: Unknown parent"));
			return NULL;
			break;
	}

	return g_pOptions->AddPage(parentid, title);
}

bool DeleteOptionsPage(wxScrolledWindow* page)
{
	if (!g_pOptions)
		return false;
	return g_pOptions->DeletePage(page);
}

bool DecodeSingleVDOMessage(const wxString& str, PlugIn_Position_Fix_Ex* pos, wxString* accumulator)
{
	using namespace ais;

	if (!pos)
		return false;

	GenericPosDatEx gpd;
	AIS_Error nerr = AIS_GENERIC_ERROR;
	if (g_pAIS)
		nerr = g_pAIS->DecodeSingleVDO(str, &gpd, accumulator);
	if (nerr == AIS_NoError) {
		pos->Lat = gpd.kLat;
		pos->Lon = gpd.kLon;
		pos->Cog = gpd.kCog;
		pos->Sog = gpd.kSog;
		pos->Hdt = gpd.kHdt;

		// Fill in the dummy values
		pos->FixTime = 0;
		pos->Hdm = 1000;
		pos->Var = 1000;
		pos->nSats = 0;

		return true;
	}

	return false;
}

int GetChartbarHeight(void)
{
	if (stats && stats->IsShown()) {
		wxSize s = stats->GetSize();
		return s.GetHeight();
	} else
		return 0;
}

bool GetRoutepointGPX(RoutePoint* pRoutePoint, char* buffer, unsigned int buffer_length)
{
	bool ret = false;

	NavObjectCollection* pgpx = new NavObjectCollection;
	pgpx->AddGPXWaypoint(pRoutePoint);
	wxString gpxfilename = wxFileName::CreateTempFileName(wxT("gpx"));
	pgpx->SaveFile(gpxfilename);
	delete pgpx;

	wxFFile gpxfile(gpxfilename);
	wxString s;
	if (gpxfile.ReadAll(&s)) {
		if (s.Length() < buffer_length) {
			strncpy(buffer, (const char*)s.mb_str(wxConvUTF8), buffer_length - 1);
			ret = true;
		}
	}

	gpxfile.Close();
	::wxRemoveFile(gpxfilename);

	return ret;
}

bool GetActiveRoutepointGPX(char* buffer, unsigned int buffer_length)
{
	navigation::RouteManager& routemanager = global::OCPN::get().routeman();

	if (routemanager.IsAnyRouteActive())
		return GetRoutepointGPX(routemanager.GetpActivePoint(), buffer, buffer_length);
	else
		return false;
}

void PositionBearingDistanceMercator_Plugin(double lat, double lon, double brg, double dist,
											double* dlat, double* dlon)
{
	geo::Position p = geo::PositionBearingDistanceMercator(geo::Position(lat, lon), brg, dist);
	*dlat = p.lat();
	*dlon = p.lon();
}

void DistanceBearingMercator_Plugin(double lat0, double lon0, double lat1, double lon1, double* brg,
									double* dist)
{
	geo::DistanceBearingMercator(geo::Position(lat0, lon0), geo::Position(lat1, lon1), brg, dist);
}

double DistGreatCircle_Plugin(double slat, double slon, double dlat, double dlon)
{
	return geo::DistGreatCircle(geo::Position(slat, slon), geo::Position(dlat, dlon));
}

void toTM_Plugin(float lat, float lon, float lat0, float lon0, double* x, double* y)
{
	geo::toTM(geo::Position(lat, lon), geo::Position(lat0, lon0), x, y);
}

void fromTM_Plugin(double x, double y, double lat0, double lon0, double* lat, double* lon)
{
	geo::Position t = geo::fromTM(x, y, geo::Position(lat0, lon0));
	*lat = t.lat();
	*lon = t.lon();
}

void toSM_Plugin(double lat, double lon, double lat0, double lon0, double* x, double* y)
{
	geo::toSM(geo::Position(lat, lon), geo::Position(lat0, lon0), x, y);
}

void fromSM_Plugin(double x, double y, double lat0, double lon0, double* lat, double* lon)
{
	geo::Position t = geo::fromSM(x, y, geo::Position(lat0, lon0));
	*lat = t.lat();
	*lon = t.lon();
}

void toSM_ECC_Plugin(double lat, double lon, double lat0, double lon0, double* x, double* y)
{
	geo::toSM_ECC(geo::Position(lat, lon), geo::Position(lat0, lon0), x, y);
}

void fromSM_ECC_Plugin(double x, double y, double lat0, double lon0, double* lat, double* lon)
{
	geo::Position t = geo::fromSM_ECC(x, y, geo::Position(lat0, lon0));
	*lat = t.lat();
	*lon = t.lon();
}

double toUsrDistance_Plugin(double nm_distance, int unit)
{
	return toUsrDistance(nm_distance, static_cast<DistanceUnit>(unit));
}

double fromUsrDistance_Plugin(double usr_distance, int unit)
{
	return fromUsrDistance(usr_distance, static_cast<DistanceUnit>(unit));
}

double toUsrSpeed_Plugin(double kts_speed, int unit)
{
	return toUsrSpeed(kts_speed, static_cast<SpeedUnit>(unit));
}

double fromUsrSpeed_Plugin(double usr_speed, int unit)
{
	return fromUsrSpeed(usr_speed, static_cast<SpeedUnit>(unit));
}

wxString getUsrDistanceUnit_Plugin(int unit)
{
	return getUsrDistanceUnit(static_cast<DistanceUnit>(unit));
}

wxString getUsrSpeedUnit_Plugin(int unit)
{
	return getUsrSpeedUnit(static_cast<SpeedUnit>(unit));
}

bool PlugIn_GSHHS_CrossesLand(double lat1, double lon1, double lat2, double lon2)
{
	static GshhsReader* reader = NULL;
	static Projection proj;

	if (reader == NULL)
		reader = new GshhsReader(&proj);

	while (lon1 < 0)
		lon1 += 360;
	while (lon2 < 0)
		lon2 += 360;

	return reader->crossing1(QLineF(lon1, lat1, lon2, lat2));
}

void PlugInPlaySound(wxString& sound_file)
{
	if (g_pi_manager) {
		g_pi_manager->m_plugin_sound.Stop();
		g_pi_manager->m_plugin_sound.UnLoad();

		g_pi_manager->m_plugin_sound.Create(sound_file);

		if (g_pi_manager->m_plugin_sound.IsOk())
			g_pi_manager->m_plugin_sound.Play();
	}
}

// API 1.10 Route and Waypoint Support
// wxBitmap *FindSystemWaypointIcon( wxString& icon_name );

// PlugInWaypoint implementation
PlugIn_Waypoint::PlugIn_Waypoint()
	: m_HyperlinkList(NULL)
{
}

PlugIn_Waypoint::PlugIn_Waypoint(double lat, double lon, const wxString& icon_ident,
								 const wxString& wp_name, const wxString& GUID)
	: m_HyperlinkList(NULL)
{
	wxDateTime now = wxDateTime::Now();
	m_CreateTime = now.ToUTC();

	m_lat = lat;
	m_lon = lon;
	m_IconName = icon_ident;
	m_MarkName = wp_name;
	m_GUID = GUID;
}

PlugIn_Waypoint::~PlugIn_Waypoint()
{
}

// PlugInRoute implementation
PlugIn_Route::PlugIn_Route(void)
{
	pWaypointList = new Plugin_WaypointList;
}

PlugIn_Route::~PlugIn_Route(void)
{
	pWaypointList->clear();
	delete pWaypointList;
	pWaypointList = NULL;
}

// PlugInTrack implementation
PlugIn_Track::PlugIn_Track(void)
{
	pWaypointList = new Plugin_WaypointList;
}

PlugIn_Track::~PlugIn_Track(void)
{
	pWaypointList->clear();
	delete pWaypointList;
	pWaypointList = NULL;
}

wxString GetNewGUID(void)
{
	return wxString(util::uuid().c_str(), wxConvUTF8);
}

bool AddCustomWaypointIcon(wxBitmap* pimage, wxString key, wxString description)
{
	global::OCPN::get().waypointman().ProcessIcon(*pimage, key, description);
	return true;
}

bool AddSingleWaypoint(PlugIn_Waypoint* pwaypoint, bool b_permanent)
{
	// Validate the waypoint parameters a little bit

	// Make sure that this GUID is indeed unique in the Routepoint list
	if (global::OCPN::get().waypointman().find(pwaypoint->m_GUID) != NULL)
		return false;

	RoutePoint* pWP
		= new RoutePoint(geo::Position(pwaypoint->m_lat, pwaypoint->m_lon), pwaypoint->m_IconName,
						 pwaypoint->m_MarkName, pwaypoint->m_GUID);

	pWP->m_bIsolatedMark = true; // This is an isolated mark

	// Transcribe (clone) the html HyperLink List, if present
	if (pwaypoint->m_HyperlinkList) {
		if (pwaypoint->m_HyperlinkList->size() > 0) {
			for (Plugin_HyperlinkList::iterator linknode = pwaypoint->m_HyperlinkList->begin();
				 linknode != pwaypoint->m_HyperlinkList->end(); ++linknode) {
				Plugin_Hyperlink* link = *linknode;
				pWP->add_link(Hyperlink(link->DescrText, link->Link, link->Type));
			}
		}
	}

	pWP->set_description(pwaypoint->m_MarkDescription);
	pWP->m_btemp = (b_permanent == false);

	pSelect->AddSelectableRoutePoint(geo::Position(pwaypoint->m_lat, pwaypoint->m_lon), pWP);
	if (b_permanent)
		pConfig->AddNewWayPoint(pWP, -1);

	if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
		pRouteManagerDialog->UpdateWptListCtrl();

	return true;
}

bool DeleteSingleWaypoint(wxString& GUID)
{
	// Find the RoutePoint
	bool b_found = false;
	RoutePoint* prp = global::OCPN::get().waypointman().find(GUID);

	if (prp)
		b_found = true;

	if (b_found) {
		global::OCPN::get().waypointman().DestroyWaypoint(prp);
		if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
			pRouteManagerDialog->UpdateWptListCtrl();
	}

	return b_found;
}

bool UpdateSingleWaypoint(PlugIn_Waypoint* pwaypoint)
{
	// Find the RoutePoint
	bool b_found = false;
	RoutePoint* prp = global::OCPN::get().waypointman().find(pwaypoint->m_GUID);

	if (prp)
		b_found = true;

	if (b_found) {
		geo::Position position_save = prp->get_position();

		prp->set_position(geo::Position(pwaypoint->m_lat, pwaypoint->m_lon));
		prp->set_icon_name(pwaypoint->m_IconName);
		prp->SetName(pwaypoint->m_MarkName);
		prp->set_description(pwaypoint->m_MarkDescription);

		//  Transcribe (clone) the html HyperLink List, if present

		if (pwaypoint->m_HyperlinkList) {
			prp->m_HyperlinkList.clear();
			if (pwaypoint->m_HyperlinkList->size() > 0) {
				for (Plugin_HyperlinkList::iterator linknode = pwaypoint->m_HyperlinkList->begin();
					 linknode != pwaypoint->m_HyperlinkList->end(); ++linknode) {
					Plugin_Hyperlink* link = *linknode;
					prp->add_link(Hyperlink(link->DescrText, link->Link, link->Type));
				}
			}
		}

		SelectItem* pFind = pSelect->FindSelection(position_save, SelectItem::TYPE_ROUTEPOINT);
		if (pFind) {
			pFind->pos1 = geo::Position(pwaypoint->m_lat, pwaypoint->m_lon);
		}

		if (!prp->m_btemp)
			pConfig->UpdateWayPoint(prp);

		if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
			pRouteManagerDialog->UpdateWptListCtrl();
	}

	return b_found;
}

bool AddPlugInRoute(PlugIn_Route* proute, bool b_permanent)
{
	Route* route = new Route();

	RoutePoint* pWP_src;
	int ip = 0;

	for (Plugin_WaypointList::iterator pwpnode = proute->pWaypointList->begin();
		 pwpnode != proute->pWaypointList->end(); ++pwpnode) {
		PlugIn_Waypoint* pwp = *pwpnode;

		RoutePoint* pWP = new RoutePoint(geo::Position(pwp->m_lat, pwp->m_lon), pwp->m_IconName,
										 pwp->m_MarkName, pwp->m_GUID);

		// Transcribe (clone) the html HyperLink List, if present
		if (pwp->m_HyperlinkList) {
			if (pwp->m_HyperlinkList->size() > 0) {
				for (Plugin_HyperlinkList::iterator linknode = pwp->m_HyperlinkList->begin();
					 linknode != pwp->m_HyperlinkList->end(); ++linknode) {
					Plugin_Hyperlink* link = *linknode;
					pWP->add_link(Hyperlink(link->DescrText, link->Link, link->Type));
				}
			}
		}

		pWP->set_description(pwp->m_MarkDescription);
		pWP->set_show_name(false);
		pWP->SetCreateTime(pwp->m_CreateTime);

		route->AddPoint(pWP);

		pSelect->AddSelectableRoutePoint(pWP->get_position(), pWP);

		if (ip > 0)
			pSelect->AddSelectableRouteSegment(pWP_src->get_position(), pWP->get_position(),
											   pWP_src, pWP, route);
		ip++;
		pWP_src = pWP;
	}

	route->set_name(proute->m_NameString);
	route->set_startString(proute->m_StartString);
	route->set_endString(proute->m_EndString);
	route->set_guid(proute->m_GUID);
	route->m_btemp = (b_permanent == false);

	pRouteList->push_back(route);

	if (b_permanent)
		pConfig->AddNewRoute(route);

	if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
		pRouteManagerDialog->UpdateRouteListCtrl();

	return true;
}

bool DeletePlugInRoute(wxString& GUID)
{
	bool b_found = false;

	navigation::RouteManager& routemanager = global::OCPN::get().routeman();

	// Find the Route
	Route* pRoute = routemanager.FindRouteByGUID(GUID);
	if (pRoute) {
		routemanager.DeleteRoute(pRoute);
		b_found = true;
	}
	return b_found;
}

bool UpdatePlugInRoute(PlugIn_Route* proute)
{
	bool b_found = false;

	navigation::RouteManager& routemanager = global::OCPN::get().routeman();

	// Find the Route
	Route* pRoute = routemanager.FindRouteByGUID(proute->m_GUID);
	if (pRoute)
		b_found = true;

	if (b_found) {
		bool b_permanent = (pRoute->m_btemp == false);
		routemanager.DeleteRoute(pRoute);

		b_found = AddPlugInRoute(proute, b_permanent);
	}

	return b_found;
}

bool AddPlugInTrack(PlugIn_Track* ptrack, bool b_permanent)
{
	Track* track = new Track();

	RoutePoint* pWP_src;
	int ip = 0;

	for (Plugin_WaypointList::iterator pwpnode = ptrack->pWaypointList->begin();
		 pwpnode != ptrack->pWaypointList->end(); ++pwpnode) {
		PlugIn_Waypoint* pwp = *pwpnode;

		RoutePoint* pWP = new RoutePoint(geo::Position(pwp->m_lat, pwp->m_lon), pwp->m_IconName,
										 pwp->m_MarkName, pwp->m_GUID);

		pWP->set_description(pwp->m_MarkDescription);
		pWP->set_show_name(false);
		pWP->SetCreateTime(pwp->m_CreateTime);

		track->AddPoint(pWP);

		pSelect->AddSelectableRoutePoint(pWP->get_position(), pWP);

		if (ip > 0)
			pSelect->AddSelectableRouteSegment(pWP_src->get_position(), pWP->get_position(),
											   pWP_src, pWP, track);
		ip++;
		pWP_src = pWP;
	}

	track->set_name(ptrack->m_NameString);
	track->set_startString(ptrack->m_StartString);
	track->set_endString(ptrack->m_EndString);
	track->set_guid(ptrack->m_GUID);
	track->m_btemp = (b_permanent == false);

	pRouteList->push_back(track);

	if (b_permanent)
		pConfig->AddNewRoute(track);

	if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
		pRouteManagerDialog->UpdateTrkListCtrl();

	return true;
}

bool DeletePluginTrack(wxString& GUID)
{
	bool b_found = false;

	navigation::RouteManager& routemanager = global::OCPN::get().routeman();

	// Find the Route
	Route* pRoute = routemanager.FindRouteByGUID(GUID);
	if (pRoute) {
		routemanager.DeleteTrack((Track*)pRoute);
		b_found = true;
	}

	if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
		pRouteManagerDialog->UpdateRouteListCtrl();

	return b_found;
}

bool UpdatePlugInTrack(PlugIn_Track* ptrack)
{
	bool b_found = false;

	navigation::RouteManager& routemanager = global::OCPN::get().routeman();

	// Find the Track
	Route* pRoute = routemanager.FindRouteByGUID(ptrack->m_GUID);
	if (pRoute)
		b_found = true;

	if (b_found) {
		bool b_permanent = (pRoute->m_btemp == false);
		routemanager.DeleteTrack((Track*)pRoute);

		b_found = AddPlugInTrack(ptrack, b_permanent);
	}

	return b_found;
}

//-----------------------------------------------------------------------------------------
//    The opencpn_plugin base class implementation
//-----------------------------------------------------------------------------------------

opencpn_plugin::~opencpn_plugin()
{
}

int opencpn_plugin::Init(void)
{
	return 0;
}

bool opencpn_plugin::DeInit(void)
{
	return true;
}

int opencpn_plugin::GetAPIVersionMajor()
{
	return 1;
}

int opencpn_plugin::GetAPIVersionMinor()
{
	return 2;
}

int opencpn_plugin::GetPlugInVersionMajor()
{
	return 1;
}

int opencpn_plugin::GetPlugInVersionMinor()
{
	return 0;
}

wxBitmap* opencpn_plugin::GetPlugInBitmap()
{
	return new wxBitmap(global::OCPN::get().styleman().current().GetIcon(_T("default_pi")));
}

wxString opencpn_plugin::GetCommonName()
{
	return _T("BaseClassCommonName");
}

wxString opencpn_plugin::GetShortDescription()
{
	return _T("OpenCPN PlugIn Base Class");
}

wxString opencpn_plugin::GetLongDescription()
{
	return _T("OpenCPN PlugIn Base Class\nPlugInManager created this base class");
}

void opencpn_plugin::SetPositionFix(PlugIn_Position_Fix& WXUNUSED(pfix))
{
}

void opencpn_plugin::SetNMEASentence(wxString& WXUNUSED(sentence))
{
}

void opencpn_plugin::SetAISSentence(wxString& WXUNUSED(sentence))
{
}

int opencpn_plugin::GetToolbarToolCount(void)
{
	return 0;
}

int opencpn_plugin::GetToolboxPanelCount(void)
{
	return 0;
}

void opencpn_plugin::SetupToolboxPanel(int WXUNUSED(page_sel), wxNotebook* WXUNUSED(pnotebook))
{
}

void opencpn_plugin::OnCloseToolboxPanel(int WXUNUSED(page_sel), int WXUNUSED(ok_apply_cancel))
{
}

void opencpn_plugin::ShowPreferencesDialog(wxWindow* WXUNUSED(parent))
{
}

void opencpn_plugin::OnToolbarToolCallback(int WXUNUSED(id))
{
}

void opencpn_plugin::OnContextMenuItemCallback(int WXUNUSED(id))
{
}

bool opencpn_plugin::RenderOverlay(wxMemoryDC* WXUNUSED(dc), PlugIn_ViewPort* WXUNUSED(vp))
{
	return false;
}

void opencpn_plugin::SetCursorLatLon(double WXUNUSED(lat), double WXUNUSED(lon))
{
}

void opencpn_plugin::SetCurrentViewPort(PlugIn_ViewPort& WXUNUSED(vp))
{
}

void opencpn_plugin::SetDefaults(void)
{
}

void opencpn_plugin::ProcessParentResize(int WXUNUSED(x), int WXUNUSED(y))
{
}

void opencpn_plugin::SetColorScheme(PI_ColorScheme WXUNUSED(cs))
{
}

void opencpn_plugin::UpdateAuiStatus(void)
{
}

wxArrayString opencpn_plugin::GetDynamicChartClassNameArray()
{
	wxArrayString array;
	return array;
}

// Opencpn_Plugin_16 Implementation
opencpn_plugin_16::opencpn_plugin_16(void* pmgr) : opencpn_plugin(pmgr)
{
}

opencpn_plugin_16::~opencpn_plugin_16(void)
{
}

bool opencpn_plugin_16::RenderOverlay(wxDC& WXUNUSED(dc), PlugIn_ViewPort* WXUNUSED(vp))
{
	return false;
}

void opencpn_plugin_16::SetPluginMessage(wxString& WXUNUSED(message_id),
										 wxString& WXUNUSED(message_body))
{
}

// Opencpn_Plugin_17 Implementation
opencpn_plugin_17::opencpn_plugin_17(void* pmgr) : opencpn_plugin(pmgr)
{
}

opencpn_plugin_17::~opencpn_plugin_17(void)
{
}

bool opencpn_plugin_17::RenderOverlay(wxDC& WXUNUSED(dc), PlugIn_ViewPort* WXUNUSED(vp))
{
	return false;
}

bool opencpn_plugin_17::RenderGLOverlay(wxGLContext* WXUNUSED(pcontext),
										PlugIn_ViewPort* WXUNUSED(vp))
{
	return false;
}

void opencpn_plugin_17::SetPluginMessage(wxString& WXUNUSED(message_id),
										 wxString& WXUNUSED(message_body))
{
}

// Opencpn_Plugin_18 Implementation
opencpn_plugin_18::opencpn_plugin_18(void* pmgr) : opencpn_plugin(pmgr)
{
}

opencpn_plugin_18::~opencpn_plugin_18(void)
{
}

bool opencpn_plugin_18::RenderOverlay(wxDC& WXUNUSED(dc), PlugIn_ViewPort* WXUNUSED(vp))
{
	return false;
}

bool opencpn_plugin_18::RenderGLOverlay(wxGLContext* WXUNUSED(pcontext),
										PlugIn_ViewPort* WXUNUSED(vp))
{
	return false;
}

void opencpn_plugin_18::SetPluginMessage(wxString& WXUNUSED(message_id),
										 wxString& WXUNUSED(message_body))
{
}

void opencpn_plugin_18::SetPositionFixEx(PlugIn_Position_Fix_Ex& WXUNUSED(pfix))
{
}

// Opencpn_Plugin_19 Implementation
opencpn_plugin_19::opencpn_plugin_19(void* pmgr) : opencpn_plugin_18(pmgr)
{
}

opencpn_plugin_19::~opencpn_plugin_19(void)
{
}

void opencpn_plugin_19::OnSetupOptions(void)
{
}

// Opencpn_Plugin_110 Implementation
opencpn_plugin_110::opencpn_plugin_110(void* pmgr) : opencpn_plugin_19(pmgr)
{
}

opencpn_plugin_110::~opencpn_plugin_110(void)
{
}

void opencpn_plugin_110::LateInit(void)
{
}

//    Opencpn_Plugin_111 Implementation
opencpn_plugin_111::opencpn_plugin_111(void* pmgr) : opencpn_plugin_110(pmgr)
{
}

opencpn_plugin_111::~opencpn_plugin_111(void)
{
}

//          Helper and interface classes

//-------------------------------------------------------------------------------
//    PlugIn_AIS_Target Implementation
//-------------------------------------------------------------------------------

PlugIn_AIS_Target *Create_PI_AIS_Target(ais::AIS_Target_Data *ptarget)
{
	PlugIn_AIS_Target *pret = new PlugIn_AIS_Target;

	pret->MMSI =            ptarget->MMSI;
	pret->Class =           ptarget->Class;
	pret->NavStatus =       ptarget->NavStatus;
	pret->SOG =             ptarget->SOG;
	pret->COG =             ptarget->COG;
	pret->HDG =             ptarget->HDG;
	pret->Lon =             ptarget->Lon;
	pret->Lat =             ptarget->Lat;
	pret->ROTAIS =          ptarget->ROTAIS;
	pret->ShipType =        ptarget->ShipType;
	pret->IMO =             ptarget->IMO;

	pret->Range_NM =        ptarget->Range_NM;
	pret->Brg =             ptarget->Brg;

	//      Per target collision parameters
	pret->bCPA_Valid =      ptarget->bCPA_Valid;
	pret->TCPA =            ptarget->TCPA;                     // Minutes
	pret->CPA =             ptarget->CPA;                      // Nautical Miles

	pret->alarm_state =     (plugin_ais_alarm_type)ptarget->n_alarm_state;

	strncpy(pret->CallSign, ptarget->CallSign, sizeof(ptarget->CallSign));
	strncpy(pret->ShipName, ptarget->ShipName, sizeof(ptarget->ShipName));

	return pret;
}


// ----------------------------------------------------------------------------
// PlugInChartBase Implmentation
//  This class is the base class for Plug-able chart types
// ----------------------------------------------------------------------------

PlugInChartBase::PlugInChartBase()
{}

PlugInChartBase::~PlugInChartBase()
{}

wxString PlugInChartBase::GetFileSearchMask(void)
{
	return _T("");
}

int PlugInChartBase::Init(const wxString &, int)
{
	return 0;
}

double PlugInChartBase::GetNormalScaleMin(double, bool) const
{
	return 1.0;
}

double PlugInChartBase::GetNormalScaleMax(double, int) const
{
	return 2.0e7;
}

bool PlugInChartBase::GetChartExtent(ExtentPI *)
{
	return false;
}

wxBitmap& PlugInChartBase::RenderRegionView(const PlugIn_ViewPort &, const wxRegion &)
{
	return wxNullBitmap;
}

bool PlugInChartBase::AdjustVP(PlugIn_ViewPort &, PlugIn_ViewPort &)
{
	return false;
}

void PlugInChartBase::GetValidCanvasRegion(const PlugIn_ViewPort &, wxRegion *)
{}

void PlugInChartBase::SetColorScheme(int, bool)
{}

double PlugInChartBase::GetNearestPreferredScalePPM(double) const
{
	return 1.0;
}

wxBitmap *PlugInChartBase::GetThumbnail(int, int, int)
{
	return NULL;
}

void PlugInChartBase::ComputeSourceRectangle(const PlugIn_ViewPort &, wxRect *)
{}

double PlugInChartBase::GetRasterScaleFactor()
{
	return 1.0;
}

bool PlugInChartBase::GetChartBits(wxRect &, unsigned char *, int)
{
	return false;
}

int PlugInChartBase::GetSize_X()
{
	return 1;
}

int PlugInChartBase::GetSize_Y()
{
	return 1;
}

void PlugInChartBase::latlong_to_chartpix(const geo::Position&, double &, double &)
{}

// ----------------------------------------------------------------------------
// PlugInChartBaseGL Implmentation
//
// ----------------------------------------------------------------------------

PlugInChartBaseGL::PlugInChartBaseGL()
{}

PlugInChartBaseGL::~PlugInChartBaseGL()
{}

int PlugInChartBaseGL::RenderRegionViewOnGL(const wxGLContext& WXUNUSED(glc),
											const PlugIn_ViewPort& WXUNUSED(VPoint),
											const wxRegion& WXUNUSED(Region),
											bool WXUNUSED(b_use_stencil))
{
    return 0;
}

ListOfPI_S57Obj* PlugInChartBaseGL::GetObjRuleListAtLatLon(float WXUNUSED(lat), float WXUNUSED(lon),
														   float WXUNUSED(select_radius),
														   PlugIn_ViewPort* WXUNUSED(VPoint))
{
    return NULL;
}

wxString PlugInChartBaseGL::CreateObjDescriptions(ListOfPI_S57Obj* WXUNUSED(obj_list))
{
    return _T("");
}

int PlugInChartBaseGL::GetNoCOVREntries()
{
    return 0;
}

int PlugInChartBaseGL::GetNoCOVRTablePoints(int WXUNUSED(iTable))
{
    return 0;
}

int  PlugInChartBaseGL::GetNoCOVRTablenPoints(int WXUNUSED(iTable))
{
    return 0;
}

float *PlugInChartBaseGL::GetNoCOVRTableHead(int WXUNUSED(iTable))
{
    return 0;
}



