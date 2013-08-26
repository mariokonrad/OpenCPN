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
#include <wx/glcanvas.h>
#include <wx/hyperlink.h>
#include <wx/choice.h>
#include <wx/tglbtn.h>
#include <wx/bmpcbox.h>

#include "ocpn_plugin.h"
#include "chart1.h"
#include "chcanv.h"
#include "datastream.h"
#include "OCPN_Sound.h"
#include "plugin/PlugInContainer.h"
#include "plugin/PlugInMenuItemContainer.h"
#include "plugin/PlugInToolbarToolContainer.h"

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

#include "wx/json_defs.h"
#include "wx/jsonwriter.h"

PlugIn_AIS_Target *Create_PI_AIS_Target(AIS_Target_Data *ptarget);

class PluginListPanel;
class PluginPanel;

extern const wxEventType wxEVT_OCPN_MSG;


WX_DEFINE_ARRAY_PTR(PlugInMenuItemContainer *, ArrayOfPlugInMenuItems);
WX_DEFINE_ARRAY_PTR(PlugInToolbarToolContainer *, ArrayOfPlugInToolbarTools);

class PlugInManager
{

	public:
		PlugInManager(MyFrame *parent);
		virtual ~PlugInManager();

		bool LoadAllPlugIns(const wxString &plugin_dir);
		bool UnLoadAllPlugIns();
		bool DeactivateAllPlugIns();
		bool UpdatePlugIns();

		bool UpdateConfig();

		PlugInContainer *LoadPlugIn(wxString plugin_file);
		ArrayOfPlugIns *GetPlugInArray(){ return &plugin_array; }

		bool RenderAllCanvasOverlayPlugIns( ocpnDC &dc, const ViewPort &vp);
		bool RenderAllGLCanvasOverlayPlugIns( wxGLContext *pcontext, const ViewPort &vp);
		void SendCursorLatLonToAllPlugIns( double lat, double lon);
		void SendViewPortToRequestingPlugIns( ViewPort &vp );

		void NotifySetupOptions();
		void CloseAllPlugInPanels( int );

		ArrayOfPlugInToolbarTools &GetPluginToolbarToolArray(){ return m_PlugInToolbarTools; }
		int AddToolbarTool(wxString label, wxBitmap *bitmap, wxBitmap *bmpDisabled,
				wxItemKind kind, wxString shortHelp, wxString longHelp,
				wxObject *clientData, int position,
				int tool_sel, opencpn_plugin *pplugin );

		void RemoveToolbarTool(int tool_id);
		void SetToolbarToolViz(int tool_id, bool viz);
		void SetToolbarItemState(int tool_id, bool toggle);
		void SetToolbarItemBitmaps(int item, wxBitmap *bitmap, wxBitmap *bmpDisabled);
		opencpn_plugin *FindToolOwner(const int id);
		wxString GetToolOwnerCommonName(const int id);

		ArrayOfPlugInMenuItems &GetPluginContextMenuItemArray(){ return m_PlugInMenuItems; }
		int AddCanvasContextMenuItem(wxMenuItem *pitem, opencpn_plugin *pplugin );
		void RemoveCanvasContextMenuItem(int item);
		void SetCanvasContextMenuItemViz(int item, bool viz);
		void SetCanvasContextMenuItemGrey(int item, bool grey);

		void SendNMEASentenceToAllPlugIns(const wxString &sentence);
		void SendPositionFixToAllPlugIns(GenericPosDatEx *ppos);
		void SendAISSentenceToAllPlugIns(const wxString &sentence);
		void SendJSONMessageToAllPlugins(const wxString &message_id, wxJSONValue v);
		void SendMessageToAllPlugins(const wxString &message_id, const wxString &message_body);

		void SendResizeEventToAllPlugIns(int x, int y);
		void SetColorSchemeForAllPlugIns(ColorScheme cs);
		void NotifyAuiPlugIns(void);
		bool CallLateInit(void);

		wxArrayString GetPlugInChartClassNameArray(void);

		wxString GetLastError();
		MyFrame *GetParentFrame(){ return pParent; }

		void DimeWindow(wxWindow *win);
		OCPN_Sound m_plugin_sound;

	private:
		bool DeactivatePlugIn(PlugInContainer *pic);
		wxBitmap *BuildDimmedToolBitmap(wxBitmap *pbmp_normal, unsigned char dim_ratio);
		bool UpDateChartDataTypes(void);
		bool CheckPluginCompatibility(wxString plugin_file);

		MyFrame * pParent;

		ArrayOfPlugIns plugin_array;
		wxString m_last_error_string;

		ArrayOfPlugInMenuItems m_PlugInMenuItems;
		ArrayOfPlugInToolbarTools m_PlugInToolbarTools;

		wxString m_plugin_location;

		int m_plugin_tool_id_next;
		int m_plugin_menu_item_id_next;
		wxBitmap m_cached_overlay_bm;
};

#endif