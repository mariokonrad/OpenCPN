/***************************************************************************
 *
 * Author:   David Register
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

#ifndef _PLUGINMGR_H_
#define _PLUGINMGR_H_

#include <wx/wx.h>
#include <wx/dynarray.h>
#include <wx/dynlib.h>
#include <wx/hyperlink.h>
#include <wx/choice.h>
#include <wx/tglbtn.h>
#include <wx/bmpcbox.h>

#ifdef ocpnUSE_GL
	#include <wx/glcanvas.h>
#endif

#include <DataStream.h>

#include <chart/s52s57.h>

#include <sound/OCPN_Sound.h>

#include <ais/AIS_Target_Data.h>

#include <global/ColorScheme.h>

#include <plugin/ocpn_plugin.h>
#include <plugin/PlugInContainer.h>
#include <plugin/PlugInMenuItemContainer.h>
#include <plugin/PlugInToolbarToolContainer.h>
#include <plugin/ChartPlugInWrapper.h>

// Include wxJSON headers
// We undefine MIN/MAX so avoid warning of redefinition coming from
// json_defs.h
// Definitions checked manually, and are identical
#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

#include <wx/json_defs.h>
#include <wx/jsonwriter.h>

PlugIn_AIS_Target * Create_PI_AIS_Target(ais::AIS_Target_Data *ptarget);

class PluginListPanel;
class PluginPanel;
class MainFrame;
class ocpnDC;
class ViewPort;

struct BlackListedPlugin
{
	wxString name;      // name of the plugin
	int version_major;  // major version
	int version_minor;  // minor version
	bool hard;          // hard blacklist - if true, don't load it at all, if false, load it and just warn the user
	bool all_lower;     // if true, blacklist also all the lower versions of the plugin
};

const BlackListedPlugin PluginBlacklist[] = {
	{ _T("aisradar_pi"), 0, 95, false, true },
	{ _T("radar_pi"),    0, 95, false, true }, // GCC alias for aisradar_pi
};


extern const wxEventType wxEVT_OCPN_MSG;

WX_DEFINE_ARRAY_PTR(PlugInMenuItemContainer *, ArrayOfPlugInMenuItems);
WX_DEFINE_ARRAY_PTR(PlugInToolbarToolContainer *, ArrayOfPlugInToolbarTools);

class PlugInManager
{
public:
	PlugInManager(MainFrame* parent);
	virtual ~PlugInManager();

	bool LoadAllPlugIns(const wxString& plugin_dir);
	bool UnLoadAllPlugIns();
	bool DeactivateAllPlugIns();
	bool UpdatePlugIns();

	bool UpdateConfig();

	PlugInContainer* LoadPlugIn(wxString plugin_file);
	ArrayOfPlugIns* GetPlugInArray()
	{
		return &plugin_array;
	}

	bool RenderAllCanvasOverlayPlugIns(ocpnDC& dc, const ViewPort& vp);
	bool RenderAllGLCanvasOverlayPlugIns(wxGLContext* pcontext, const ViewPort& vp);
	void SendCursorLatLonToAllPlugIns(double lat, double lon);
	void SendViewPortToRequestingPlugIns(const ViewPort& vp);

	void NotifySetupOptions();
	void CloseAllPlugInPanels(int);

	ArrayOfPlugInToolbarTools& GetPluginToolbarToolArray()
	{
		return m_PlugInToolbarTools;
	}
	int AddToolbarTool(wxString label, wxBitmap* bitmap, wxBitmap* bmpDisabled, wxItemKind kind,
					   wxString shortHelp, wxString longHelp, wxObject* clientData, int position,
					   int tool_sel, opencpn_plugin* pplugin);

	void RemoveToolbarTool(int tool_id);
	void SetToolbarToolViz(int tool_id, bool viz);
	void SetToolbarItemState(int tool_id, bool toggle);
	void SetToolbarItemBitmaps(int item, wxBitmap* bitmap, wxBitmap* bmpDisabled);
	opencpn_plugin* FindToolOwner(const int id);
	wxString GetToolOwnerCommonName(const int id);

	ArrayOfPlugInMenuItems& GetPluginContextMenuItemArray()
	{
		return m_PlugInMenuItems;
	}
	int AddCanvasContextMenuItem(wxMenuItem* pitem, opencpn_plugin* pplugin);
	void RemoveCanvasContextMenuItem(int item);
	void SetCanvasContextMenuItemViz(int item, bool viz);
	void SetCanvasContextMenuItemGrey(int item, bool grey);

	void SendNMEASentenceToAllPlugIns(const wxString& sentence);
	void SendPositionFixToAllPlugIns(GenericPosDatEx* ppos);
	void SendAISSentenceToAllPlugIns(const wxString& sentence);
	void SendJSONMessageToAllPlugins(const wxString& message_id, wxJSONValue v);
	void SendMessageToAllPlugins(const wxString& message_id, const wxString& message_body);

	void SendResizeEventToAllPlugIns(int x, int y);
	void SetColorSchemeForAllPlugIns(global::ColorScheme cs);
	void NotifyAuiPlugIns(void);
	bool CallLateInit(void);

	wxArrayString GetPlugInChartClassNameArray(void);

	ListOfPI_S57Obj* GetPlugInObjRuleListAtLatLon(ChartPlugInWrapper* target, float zlat,
												  float zlon, float SelectRadius,
												  const ViewPort& vp);
	wxString CreateObjDescriptions(ChartPlugInWrapper* target, ListOfPI_S57Obj* rule_list);

	wxString GetLastError();
	MainFrame* GetParentFrame()
	{
		return pParent;
	}

	void DimeWindow(wxWindow* win);
	sound::OCPN_Sound m_plugin_sound;

private:
	bool CheckBlacklistedPlugin(opencpn_plugin* plugin);
	bool DeactivatePlugIn(PlugInContainer* pic);
	wxBitmap* BuildDimmedToolBitmap(wxBitmap* pbmp_normal, unsigned char dim_ratio);
	bool UpDateChartDataTypes(void);
	bool CheckPluginCompatibility(wxString plugin_file);

	MainFrame* pParent;

	ArrayOfPlugIns plugin_array;
	wxString m_last_error_string;

	ArrayOfPlugInMenuItems m_PlugInMenuItems;
	ArrayOfPlugInToolbarTools m_PlugInToolbarTools;

	wxString m_plugin_location;

	int m_plugin_tool_id_next;
	int m_plugin_menu_item_id_next;
	wxBitmap m_cached_overlay_bm;
};

//  API 1.11 adds access to S52 Presentation library
//  These are some wrapper conversion utilities

class S52PLIB_Context
{
public:
	S52PLIB_Context()
	{
		bBBObj_valid = false;
		bCS_Added = false;
		bFText_Added = false;
		CSrules = NULL;
		FText = NULL;
	};

	~S52PLIB_Context() {};

	geo::BoundingBox BBObj; // lat/lon BBox of the rendered object
	bool bBBObj_valid; // set after the BBObj has been calculated once.

	chart::Rules* CSrules; // per object conditional symbology
	int bCS_Added;

	chart::S52_TextC* FText;
	int bFText_Added;
	wxRect rText;

	chart::LUPrec* LUP;
};

void CreateCompatibleS57Object(PI_S57Obj* pObj, chart::S57Obj* cobj);
void UpdatePIObjectPlibContext(PI_S57Obj* pObj, chart::S57Obj* cobj);

ViewPort CreateCompatibleViewport(const PlugIn_ViewPort& pivp);

#endif
