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
#include "dychart.h"
#include "MainFrame.h"
#include "navutil.h"
#include "Track.h"
#include "ocpnDC.h"
#include "StyleManager.h"
#include "Style.h"
#include "Select.h"
#include "OptionDialog.h"
#include "Multiplexer.h"
#include "StatWin.h"
#include "Routeman.h"
#include "FontMgr.h"
#include "OCPN_DataStreamEvent.h"
#include "georef.h"
#include "WayPointman.h"
#include "RouteManagerDialog.h"
#include "NavObjectCollection.h"

#include <ViewPort.h>
#include <DimeControl.h>
#include <ChartCanvas.h>

#include <plugin/OCPN_MsgEvent.h>

#include <chart/ChartDB.h>
#include <chart/ChartDatabase.h>
#include <chart/gshhs/QLineF.h>
#include <chart/gshhs/GshhsReader.h>
#include <chart/gshhs/Projection.h>

#include <ais/ais.h>
#include <ais/AIS_Decoder.h>
#include <ais/AIS_Target_Data.h>

#include "gpx/GpxDocument.h"

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/aui/aui.h>
#include <wx/statline.h>

extern MyConfig        *pConfig;
extern wxString        g_SData_Locn;
extern AIS_Decoder     *g_pAIS;
extern wxAuiManager    *g_pauimgr;
extern wxLocale        *plocale_def_lang;
extern ChartDB         *ChartData;
extern MainFrame         *gFrame;
extern ocpnStyle::StyleManager * g_StyleManager;
extern options         *g_pOptions;
extern Multiplexer     *g_pMUX;
extern StatWin         *stats;
extern Routeman        *g_pRouteMan;
extern WayPointman     *pWayPointMan;
extern Select          *pSelect;
extern RouteManagerDialog *pRouteManagerDialog;
extern RouteList       *pRouteList;
extern PlugInManager   *g_pi_manager;

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(Plugin_WaypointList);


//    Some static helper funtions
//    Scope is local to this module

PlugIn_ViewPort CreatePlugInViewport( const ViewPort &vp)
{
    //    Create a PlugIn Viewport
    ViewPort tvp = vp;
    PlugIn_ViewPort pivp;

    pivp.clat =                   tvp.clat;                   // center point
    pivp.clon =                   tvp.clon;
    pivp.view_scale_ppm =         tvp.view_scale_ppm;
    pivp.skew =                   tvp.skew;
    pivp.rotation =               tvp.rotation;
    pivp.chart_scale =            tvp.chart_scale;
    pivp.pix_width =              tvp.pix_width;
    pivp.pix_height =             tvp.pix_height;
    pivp.rv_rect =                tvp.rv_rect;
    pivp.b_quilt =                tvp.b_quilt;
    pivp.m_projection_type =      tvp.m_projection_type;

    pivp.lat_min =                tvp.GetBBox().GetMinY();
    pivp.lat_max =                tvp.GetBBox().GetMaxY();
    pivp.lon_min =                tvp.GetBBox().GetMinX();
    pivp.lon_max =                tvp.GetBBox().GetMaxX();

    pivp.bValid =                 tvp.IsValid();                 // This VP is valid

    return pivp;
}


const wxEventType wxEVT_OCPN_MSG = wxNewEventType();

//-----------------------------------------------------------------------------------------------------
//
//          The PlugIn Manager Implementation
//
//-----------------------------------------------------------------------------------------------------
PlugInManager *s_ppim;

PlugInManager::PlugInManager(MainFrame *parent)
{
    pParent = parent;
    s_ppim = this;

    MainFrame *pFrame = GetParentFrame();
    if(pFrame)
    {
        m_plugin_menu_item_id_next = pFrame->GetCanvasWindow()->GetNextContextMenuId(); // FIXME: interface transition
        m_plugin_tool_id_next = pFrame->GetNextToolbarToolId();
    }

}

PlugInManager::~PlugInManager()
{
}


bool PlugInManager::LoadAllPlugIns(const wxString &plugin_dir)
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

    if(!::wxDirExists(m_plugin_location))
    {
        msg = m_plugin_location;
        msg.Prepend(_T("   Directory "));
        msg.Append(_T(" does not exist."));
        wxLogMessage(msg);
        return false;
    }



    wxDir pi_dir(m_plugin_location);

    if(pi_dir.IsOpened())
    {
        wxString plugin_file;
        bool b_more =pi_dir.GetFirst(&plugin_file, pispec);
        while(b_more)
        {
            wxString file_name = m_plugin_location + _T("/") + plugin_file;

            bool b_compat = CheckPluginCompatibility(file_name);

            if(!b_compat)
            {
                wxString msg(_("    Incompatible PlugIn detected:"));
                msg += file_name;
                wxLogMessage(msg);
            }

            PlugInContainer *pic = NULL;
            if(b_compat)
                pic = LoadPlugIn(file_name);
            if(pic)
            {
                if(pic->m_pplugin)
                {
                    plugin_array.Add(pic);

                    //    The common name is available without initialization and startup of the PlugIn
                    pic->m_common_name = pic->m_pplugin->GetCommonName();

                    //    Check the config file to see if this PlugIn is user-enabled
                    wxString config_section = ( _T ( "/PlugIns/" ) );
                    config_section += pic->m_common_name;
                    pConfig->SetPath ( config_section );
                    pConfig->Read ( _T ( "bEnabled" ), &pic->m_bEnabled );

                    if(pic->m_bEnabled)
                    {
                        pic->m_cap_flag = pic->m_pplugin->Init();
                        pic->m_bInitState = true;
                    }

                    pic->m_short_description = pic->m_pplugin->GetShortDescription();
                    pic->m_long_description = pic->m_pplugin->GetLongDescription();
                    pic->m_version_major = pic->m_pplugin->GetPlugInVersionMajor();
                    pic->m_version_minor = pic->m_pplugin->GetPlugInVersionMinor();
                    pic->m_bitmap = pic->m_pplugin->GetPlugInBitmap();

                }
                else        // not loaded
                {
                    wxString msg;
                    msg.Printf(_T("    PlugInManager: Unloading invalid PlugIn, API version %d "), pic->m_api_version );
                    wxLogMessage(msg);

                    pic->m_destroy_fn(pic->m_pplugin);

                    delete pic->m_plibrary;            // This will unload the PlugIn
                    delete pic;
                }
            }


            b_more =pi_dir.GetNext(&plugin_file);
        }

        UpDateChartDataTypes();

        return true;
    }
    else
        return false;
}

bool PlugInManager::CallLateInit(void)
{
    bool bret = true;

    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);

        switch(pic->m_api_version)
        {
            case 110:
                if(pic->m_cap_flag & WANTS_LATE_INIT) {
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

    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);

        if(pic->m_bEnabled && !pic->m_bInitState)
        {
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
        }
        else if(!pic->m_bEnabled && pic->m_bInitState)
        {
            bret = DeactivatePlugIn(pic);

        }
    }

    UpDateChartDataTypes();

    return bret;
}


bool PlugInManager::UpDateChartDataTypes(void)
{
    bool bret = false;
    if(NULL == ChartData)
        return bret;

    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);

        if(/*pic->m_bEnabled &&*/ (pic->m_cap_flag & INSTALLS_PLUGIN_CHART))
            bret = true;
    }

    if(bret)
        ChartData->UpdateChartClassDescriptorArray();

    return bret;
}


bool PlugInManager::DeactivatePlugIn(PlugInContainer *pic)
{
    bool bret = false;

    if(pic)
    {
        wxString msg(_T("PlugInManager: Deactivating PlugIn: "));
        msg += pic->m_plugin_file;
        wxLogMessage(msg);

        pic->m_pplugin->DeInit();

        //    Deactivate (Remove) any ToolbarTools added by this PlugIn
        for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
        {
            PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);

            if(pttc->m_pplugin == pic->m_pplugin)
            {
                m_PlugInToolbarTools.Remove(pttc);
                delete pttc;
            }
        }

        //    Deactivate (Remove) any ContextMenu items addded by this PlugIn
        for(unsigned int i=0; i < m_PlugInMenuItems.GetCount(); i++)
        {
            PlugInMenuItemContainer *pimis = m_PlugInMenuItems.Item(i);
            if(pimis->m_pplugin == pic->m_pplugin)
            {
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

    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);

        wxString config_section = ( _T ( "/PlugIns/" ) );
        config_section += pic->m_common_name;
        pConfig->SetPath ( config_section );
        pConfig->Write ( _T ( "bEnabled" ), pic->m_bEnabled );
    }

    return true;
}

bool PlugInManager::UnLoadAllPlugIns()
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        wxString msg(_T("PlugInManager: UnLoading PlugIn: "));
        msg += pic->m_plugin_file;
        wxLogMessage(msg);

        pic->m_destroy_fn(pic->m_pplugin);

        delete pic->m_plibrary;            // This will unload the PlugIn

        pic->m_bInitState = false;

        delete pic;
    }
    return true;
}

bool PlugInManager::DeactivateAllPlugIns()
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic && pic->m_bEnabled && pic->m_bInitState)
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
    HRSRC resInfo = ::FindResource(module, MAKEINTRESOURCE(1), RT_MANIFEST); // resource id #1 should be the manifest

    if(!resInfo)
        resInfo = ::FindResource(module, MAKEINTRESOURCE(2), RT_MANIFEST); // try resource id #2

    if (resInfo) {
        HGLOBAL resData = ::LoadResource(module, resInfo);
        DWORD resSize = ::SizeofResource(module, resInfo);
        if (resData && resSize) {
            const char *res = (const char *)::LockResource(resData); // the manifest
            if (res) {
                // got the manifest as a char *
                wxString manifest(res, wxConvUTF8);
                if(wxNOT_FOUND != manifest.Find(_T("VC90.CRT")))	// cannot load with VC90 runtime (i.e. VS2008)
                    b_compat = false;
            }
            UnlockResource(resData);
        }
        ::FreeResource(resData);
    }
    ::FreeLibrary(module);

#endif

    return b_compat;
}

PlugInContainer *PlugInManager::LoadPlugIn(wxString plugin_file)
{
    wxString msg(_T("PlugInManager: Loading PlugIn: "));
    msg += plugin_file;
    wxLogMessage(msg);

    PlugInContainer *pic = new PlugInContainer;
    pic->m_plugin_file = plugin_file;

    // load the library
    wxDynamicLibrary *plugin = new wxDynamicLibrary(plugin_file);
    pic->m_plibrary = plugin;     // Save a pointer to the wxDynamicLibrary for later deletion

    if(!plugin->IsLoaded())
    {
        wxString msg(_T("   PlugInManager: Cannot load library: "));
        msg += plugin_file;
        msg += _T(" ");
#if !defined(__WXMSW__) && !defined(__WXOSX__)
        /* give good error reporting on non windows non mac platforms */
        dlopen(plugin_file.ToAscii(), RTLD_NOW);
        char *s =  dlerror();
        wxString c;
        for(char *t = s; *t; t++)
            c+=*t;
        msg += c;
#endif
        wxLogMessage(msg);
        delete plugin;
        delete pic;
        return NULL;
    }


    // load the factory symbols
    create_t* create_plugin = (create_t*)plugin->GetSymbol(_T("create_pi"));
    if (NULL == create_plugin)
    {
        wxString msg(_T("   PlugInManager: Cannot load symbol create_pi: "));
        msg += plugin_file;
        wxLogMessage(msg);
        delete plugin;
        delete pic;
        return NULL;
    }

    destroy_t* destroy_plugin = (destroy_t*) plugin->GetSymbol(_T("destroy_pi"));
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


    switch(ver)
    {
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

    default:
        break;
    }

    if(pic->m_pplugin)
    {
        msg = _T("  ");
        msg += plugin_file;
        wxString msg1;
        msg1.Printf(_T(" Version detected: %d"), ver);
        msg += msg1;
        wxLogMessage(msg);
    }
    else
    {
        msg = _T("    ");
        msg += plugin_file;
        wxString msg1 = _T(" cannot be loaded");
        msg += msg1;
        wxLogMessage(msg);
    }

    return pic;
}

bool PlugInManager::RenderAllCanvasOverlayPlugIns( ocpnDC &dc, const ViewPort &vp)
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_OVERLAY_CALLBACK)
            {
                PlugIn_ViewPort pivp = CreatePlugInViewport( vp );

                wxDC *pdc = dc.GetDC();
                if(pdc)                       // not in OpenGL mode
                {
                    switch(pic->m_api_version)
                    {
                    case 106:
                    {
                        opencpn_plugin_16 *ppi = dynamic_cast<opencpn_plugin_16 *>(pic->m_pplugin);
                        if(ppi)
                            ppi->RenderOverlay(*pdc, &pivp);
                        break;
                    }
                    case 107:
                    {
                        opencpn_plugin_17 *ppi = dynamic_cast<opencpn_plugin_17 *>(pic->m_pplugin);
                        if(ppi)
                            ppi->RenderOverlay(*pdc, &pivp);
                        break;
                    }
                    case 108:
                    case 109:
                    case 110:
                    {
                        opencpn_plugin_18 *ppi = dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
                        if(ppi)
                            ppi->RenderOverlay(*pdc, &pivp);
                        break;
                    }

                    default:
                        break;
                    }
                }
                else
                {
                    //    If in OpenGL mode, and the PlugIn has requested OpenGL render callbacks,
                    //    then there is no need to render by wxDC here.
                    if(pic->m_cap_flag & WANTS_OPENGL_OVERLAY_CALLBACK)
                        continue;


                    if((m_cached_overlay_bm.GetWidth() != vp.pix_width) || (m_cached_overlay_bm.GetHeight() != vp.pix_height))
                        m_cached_overlay_bm.Create(vp.pix_width, vp.pix_height, -1);

                    wxMemoryDC mdc;
                    mdc.SelectObject ( m_cached_overlay_bm );
                    mdc.SetBackground ( *wxBLACK_BRUSH );
                    mdc.Clear();


                    bool b_rendered = false;

                    switch(pic->m_api_version)
                    {
                    case 106:
                    {
                        opencpn_plugin_16 *ppi = dynamic_cast<opencpn_plugin_16 *>(pic->m_pplugin);
                        if(ppi)
                            b_rendered = ppi->RenderOverlay(mdc, &pivp);
                        break;
                    }
                    case 107:
                    {
                        opencpn_plugin_17 *ppi = dynamic_cast<opencpn_plugin_17 *>(pic->m_pplugin);
                        if(ppi)
                            b_rendered = ppi->RenderOverlay(mdc, &pivp);
                        break;
                    }
                    case 108:
                    case 109:
                    case 110:
                    {
                        opencpn_plugin_18 *ppi = dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
                        if(ppi)
                            b_rendered = ppi->RenderOverlay(mdc, &pivp);
                        break;
                    }

                    default:
                    {
                        b_rendered = pic->m_pplugin->RenderOverlay(&mdc, &pivp);
                        break;
                    }
                    }

                    mdc.SelectObject(wxNullBitmap);

                    if(b_rendered)
                    {
                        wxMask *p_msk = new wxMask(m_cached_overlay_bm, wxColour(0,0,0));
                        m_cached_overlay_bm.SetMask(p_msk);

                        dc.DrawBitmap(m_cached_overlay_bm, 0, 0, true);
                    }
                }
            }
            else if(pic->m_cap_flag & WANTS_OPENGL_OVERLAY_CALLBACK)
            {
            }

        }
    }

    return true;
}

bool PlugInManager::RenderAllGLCanvasOverlayPlugIns( wxGLContext *pcontext, const ViewPort &vp)
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_OPENGL_OVERLAY_CALLBACK)
            {
                PlugIn_ViewPort pivp = CreatePlugInViewport( vp );

                switch(pic->m_api_version)
                {
                case 107:
                {
                    opencpn_plugin_17 *ppi = dynamic_cast<opencpn_plugin_17 *>(pic->m_pplugin);
                    if(ppi)
                        ppi->RenderGLOverlay(pcontext, &pivp);
                    break;
                }

                case 108:
                case 109:
                case 110:
                {
                    opencpn_plugin_18 *ppi = dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
                    if(ppi)
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

void PlugInManager::SendViewPortToRequestingPlugIns( ViewPort &vp )
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_ONPAINT_VIEWPORT)
            {
                PlugIn_ViewPort pivp = CreatePlugInViewport( vp );
                pic->m_pplugin->SetCurrentViewPort(pivp);
            }
        }
    }
}


void PlugInManager::SendCursorLatLonToAllPlugIns( double lat, double lon)
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_CURSOR_LATLON)
                pic->m_pplugin->SetCursorLatLon(lat, lon);
        }
    }
}

void NotifySetupOptionsPlugin( PlugInContainer *pic )
{
    if(pic->m_bEnabled && pic->m_bInitState)
    {
        if(pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE)
        {
            switch(pic->m_api_version)
            {
            case 109:
            case 110:
            {
                opencpn_plugin_19 *ppi = dynamic_cast<opencpn_plugin_19 *>(pic->m_pplugin);
                if(ppi) {
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
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        NotifySetupOptionsPlugin( pic );
    }
}

void PlugInManager::CloseAllPlugInPanels( int ok_apply_cancel)
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if((pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE) && ( pic->m_bToolboxPanel))
            {
                pic->m_pplugin->OnCloseToolboxPanel(0, ok_apply_cancel);
                pic->m_bToolboxPanel = false;
            }
        }
    }

}

int PlugInManager::AddCanvasContextMenuItem(wxMenuItem *pitem, opencpn_plugin *pplugin )
{
    PlugInMenuItemContainer *pmic = new PlugInMenuItemContainer;
    pmic->pmenu_item = pitem;
    pmic->m_pplugin = pplugin;
    pmic->id = pitem->GetId()==wxID_SEPARATOR?wxID_SEPARATOR:m_plugin_menu_item_id_next;
    pmic->b_viz = true;
    pmic->b_grey = false;

    m_PlugInMenuItems.Add(pmic);

    m_plugin_menu_item_id_next++;

    return pmic->id;
}

void PlugInManager::RemoveCanvasContextMenuItem(int item)
{
    for(unsigned int i=0; i < m_PlugInMenuItems.GetCount(); i++)
    {
        PlugInMenuItemContainer *pimis = m_PlugInMenuItems.Item(i);
        {
            if(pimis->id == item)
            {
                m_PlugInMenuItems.Remove(pimis);
                delete pimis;
                break;
            }
        }
    }
}

void PlugInManager::SetCanvasContextMenuItemViz(int item, bool viz)
{
    for(unsigned int i=0; i < m_PlugInMenuItems.GetCount(); i++)
    {
        PlugInMenuItemContainer *pimis = m_PlugInMenuItems.Item(i);
        {
            if(pimis->id == item)
            {
                pimis->b_viz = viz;
                break;
            }
        }
    }
}

void PlugInManager::SetCanvasContextMenuItemGrey(int item, bool grey)
{
    for(unsigned int i=0; i < m_PlugInMenuItems.GetCount(); i++)
    {
        PlugInMenuItemContainer *pimis = m_PlugInMenuItems.Item(i);
        {
            if(pimis->id == item)
            {
                pimis->b_grey = grey;
                break;
            }
        }
    }
}

void PlugInManager::SendNMEASentenceToAllPlugIns(const wxString &sentence)
{
    wxString decouple_sentence(sentence); // decouples 'const wxString &' and 'wxString &' to keep bin compat for plugins
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_NMEA_SENTENCES)
                pic->m_pplugin->SetNMEASentence(decouple_sentence);
        }
    }
}

void PlugInManager::SendJSONMessageToAllPlugins(const wxString &message_id, wxJSONValue v)
{
    wxJSONWriter w;
    wxString out;
    w.Write(v, out);
    SendMessageToAllPlugins(message_id,out);
}

void PlugInManager::SendMessageToAllPlugins(const wxString &message_id, const wxString &message_body)
{
    wxString decouple_message_id(message_id); // decouples 'const wxString &' and 'wxString &' to keep bin compat for plugins
    wxString decouple_message_body(message_body); // decouples 'const wxString &' and 'wxString &' to keep bin compat for plugins
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_PLUGIN_MESSAGING)
            {
                switch(pic->m_api_version)
                {
                case 106:
                {
                    opencpn_plugin_16 *ppi = dynamic_cast<opencpn_plugin_16 *>(pic->m_pplugin);
                    if(ppi)
                        ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
                    break;
                }
                case 107:
                {
                    opencpn_plugin_17 *ppi = dynamic_cast<opencpn_plugin_17 *>(pic->m_pplugin);
                    if(ppi)
                        ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
                    break;
                }
                case 108:
                case 109:
                case 110:
                {
                    opencpn_plugin_18 *ppi = dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
                    if(ppi)
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


void PlugInManager::SendAISSentenceToAllPlugIns(const wxString &sentence)
{
    wxString decouple_sentence(sentence); // decouples 'const wxString &' and 'wxString &' to keep bin compat for plugins
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_AIS_SENTENCES)
                pic->m_pplugin->SetAISSentence(decouple_sentence);
        }
    }
}

void PlugInManager::SendPositionFixToAllPlugIns(GenericPosDatEx *ppos)
{
    //    Send basic position fix
    PlugIn_Position_Fix pfix;
    pfix.Lat = ppos->kLat;
    pfix.Lon = ppos->kLon;
    pfix.Cog = ppos->kCog;
    pfix.Sog = ppos->kSog;
    pfix.Var = ppos->kVar;
    pfix.FixTime = ppos->FixTime;
    pfix.nSats = ppos->nSats;

    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_NMEA_EVENTS)
                pic->m_pplugin->SetPositionFix(pfix);
        }
    }

    //    Send extended position fix to PlugIns at API 108 and later
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

    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_NMEA_EVENTS)
            {
                switch(pic->m_api_version)
                {
                case 108:
                case 109:
                case 110:
                {
                    opencpn_plugin_18 *ppi = dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
                    if(ppi)
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
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
            pic->m_pplugin->ProcessParentResize(x, y);
    }
}

void PlugInManager::SetColorSchemeForAllPlugIns(ColorScheme cs)
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
            pic->m_pplugin->SetColorScheme((PI_ColorScheme)cs);
    }
}

void PlugInManager::NotifyAuiPlugIns(void)
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState && (pic->m_cap_flag & USES_AUI_MANAGER))
            pic->m_pplugin->UpdateAuiStatus();
    }
}

int PlugInManager::AddToolbarTool(wxString label, wxBitmap *bitmap, wxBitmap *bmpDisabled, wxItemKind kind,
                                  wxString shortHelp, wxString longHelp, wxObject *clientData, int position,
                                  int tool_sel, opencpn_plugin *pplugin )
{
    PlugInToolbarToolContainer *pttc = new PlugInToolbarToolContainer;
    pttc->label = label;

    if( !bitmap->IsOk() ) {
        ocpnStyle::Style*style = g_StyleManager->GetCurrentStyle();
        pttc->bitmap_day = new wxBitmap( style->GetIcon( _T("default_pi") ));
    } else {
        //  Force a non-reference copy of the bitmap from the PlugIn
        wxRect rb(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
        pttc->bitmap_day = new wxBitmap(rb.width, rb.height, -1);
        *pttc->bitmap_day = bitmap->GetSubBitmap( rb );
    }

    pttc->bitmap_dusk = BuildDimmedToolBitmap(pttc->bitmap_day, 128);
    pttc->bitmap_night = BuildDimmedToolBitmap(pttc->bitmap_day, 32);
    pttc->bitmap_Rollover = new wxBitmap(*pttc->bitmap_day);

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
    for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
    {
        PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);
        {
            if(pttc->id == tool_id)
            {
                m_PlugInToolbarTools.Remove(pttc);
                delete pttc;
                break;
            }
        }
    }

    pParent->RequestNewToolbar();
}

void PlugInManager::SetToolbarToolViz(int item, bool viz)
{
    for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
    {
        PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);
        {
            if(pttc->id == item)
            {
                pttc->b_viz = viz;
                break;
            }
        }
    }
}

void PlugInManager::SetToolbarItemState(int item, bool toggle)
{
    for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
    {
        PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);
        {
            if(pttc->id == item)
            {
                pttc->b_toggle = toggle;
                pParent->SetToolbarItemState(item, toggle);
                break;
            }
        }
    }
}

void PlugInManager::SetToolbarItemBitmaps(int item, wxBitmap *bitmap, wxBitmap *bmpRollover)
{
    for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
    {
        PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);
        {
            if(pttc->id == item)
            {
                delete pttc->bitmap_day;
                delete pttc->bitmap_dusk;
                delete pttc->bitmap_night;
                delete pttc->bitmap_Rollover;

                if( !bitmap->IsOk() ) {
                    ocpnStyle::Style*style = g_StyleManager->GetCurrentStyle();
                    pttc->bitmap_day = new wxBitmap( style->GetIcon( _T("default_pi") ));
                } else {
                    //  Force a non-reference copy of the bitmap from the PlugIn
                    wxRect rb(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
                    pttc->bitmap_day = new wxBitmap(rb.width, rb.height, -1);
                    *pttc->bitmap_day = bitmap->GetSubBitmap( rb );
                }

                pttc->bitmap_dusk = BuildDimmedToolBitmap(bitmap, 128);
                pttc->bitmap_night = BuildDimmedToolBitmap(bitmap, 32);
                pttc->bitmap_Rollover = new wxBitmap(*bmpRollover);

                pParent->SetToolbarItemBitmaps(item, pttc->bitmap_day, pttc->bitmap_Rollover);
                break;
            }
        }
    }

}

opencpn_plugin *PlugInManager::FindToolOwner(const int id)
{
    for(unsigned int i = 0 ; i < m_PlugInToolbarTools.GetCount() ; i++) {
        PlugInToolbarToolContainer *pc = m_PlugInToolbarTools.Item(i);
        if(id == pc->id)
            return pc->m_pplugin;
    }

    return NULL;
}

wxString PlugInManager::GetToolOwnerCommonName(const int id)
{
    opencpn_plugin *ppi = FindToolOwner(id);
    if(ppi) {
        for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++) {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic && (pic->m_pplugin == ppi)) return pic->m_common_name;
        }
    }

    return wxEmptyString;
}




wxString PlugInManager::GetLastError()
{
    return m_last_error_string;
}

wxBitmap *PlugInManager::BuildDimmedToolBitmap(wxBitmap *pbmp_normal, unsigned char dim_ratio)
{
    wxImage img_dup = pbmp_normal->ConvertToImage();

    if( !img_dup.IsOk() ) return NULL;

    if(dim_ratio < 200)
    {
        //  Create a dimmed version of the image/bitmap
        int gimg_width = img_dup.GetWidth();
        int gimg_height = img_dup.GetHeight();

        double factor = (double)(dim_ratio) / 256.0;

        for(int iy=0 ; iy < gimg_height ; iy++)
        {
            for(int ix=0 ; ix < gimg_width ; ix++)
            {
                if(!img_dup.IsTransparent(ix, iy))
                {
                    wxImage::RGBValue rgb(img_dup.GetRed(ix, iy), img_dup.GetGreen(ix, iy), img_dup.GetBlue(ix, iy));
                    wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
                    hsv.value = hsv.value * factor;
                    wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
                    img_dup.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);
                }
            }
        }
    }

    //  Make a bitmap
    wxBitmap *ptoolBarBitmap;

#ifdef __WXMSW__
    wxBitmap tbmp(img_dup.GetWidth(),img_dup.GetHeight(),-1);
    wxMemoryDC dwxdc;
    dwxdc.SelectObject(tbmp);

    ptoolBarBitmap = new wxBitmap(img_dup, (wxDC &)dwxdc);
#else
    ptoolBarBitmap = new wxBitmap(img_dup);
#endif

    // store it
    return ptoolBarBitmap;
}


wxArrayString PlugInManager::GetPlugInChartClassNameArray(void)
{
    wxArrayString array;
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState && (pic->m_cap_flag & INSTALLS_PLUGIN_CHART))
        {
            wxArrayString carray = pic->m_pplugin->GetDynamicChartClassNameArray();

            for(unsigned int j = 0 ; j < carray.GetCount() ; j++)
                array.Add(carray.Item(j));

        }
    }

    //    Scrub the list for duplicates
    //    Corrects a flaw in BSB4 and NVC PlugIns
    unsigned int j=0;
    while(j < array.GetCount())
    {
        wxString test = array.Item(j);
        unsigned int k = j+1;
        while(k < array.GetCount())
        {
            if(test == array.Item(k))
            {
                array.RemoveAt(k);
                j = -1;
                break;
            }
            else
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


int InsertPlugInTool(wxString label, wxBitmap *bitmap, wxBitmap *bmpDisabled, wxItemKind kind,
                     wxString shortHelp, wxString longHelp, wxObject *clientData, int position,
                     int tool_sel, opencpn_plugin *pplugin)
{
    if(s_ppim)
        return s_ppim->AddToolbarTool(label, bitmap, bmpDisabled, kind,
                                      shortHelp, longHelp, clientData, position,
                                      tool_sel, pplugin );
    else
        return -1;
}


void  RemovePlugInTool(int tool_id)
{
    if(s_ppim)
        s_ppim->RemoveToolbarTool(tool_id);
}

void SetToolbarToolViz(int item, bool viz)
{
    if(s_ppim)
        s_ppim->SetToolbarToolViz(item, viz);
}

void SetToolbarItemState(int item, bool toggle)
{
    if(s_ppim)
        s_ppim->SetToolbarItemState(item, toggle);
}

void SetToolbarToolBitmaps(int item, wxBitmap *bitmap, wxBitmap *bmprollover)
{
    if(s_ppim)
        s_ppim->SetToolbarItemBitmaps(item, bitmap, bmprollover);
}

int AddCanvasContextMenuItem(wxMenuItem *pitem, opencpn_plugin *pplugin )
{
    if(s_ppim)
        return s_ppim->AddCanvasContextMenuItem(pitem, pplugin );
    else
        return -1;
}


void SetCanvasContextMenuItemViz(int item, bool viz)
{
    if(s_ppim)
        s_ppim->SetCanvasContextMenuItemViz(item, viz);
}

void SetCanvasContextMenuItemGrey(int item, bool grey)
{
    if(s_ppim)
        s_ppim->SetCanvasContextMenuItemGrey(item, grey);
}

void RemoveCanvasContextMenuItem(int item)
{
    if(s_ppim)
        s_ppim->RemoveCanvasContextMenuItem(item);
}

wxFileConfig *GetOCPNConfigObject(void)
{
    if(s_ppim)
        return pConfig;         // return the global application config object
    else
        return NULL;
}

wxWindow *GetOCPNCanvasWindow()
{
    wxWindow *pret = NULL;
    if (s_ppim) {
        MainFrame * pFrame = s_ppim->GetParentFrame();
        pret = (wxWindow *)pFrame->GetCanvasWindow();
    }
    return pret;
}

void RequestRefresh(wxWindow *win)
{
    if(win)
        win->Refresh();
}

void GetCanvasPixLL(PlugIn_ViewPort *vp, wxPoint *pp, double lat, double lon)
{
    //    Make enough of an application viewport to run its method....
    ViewPort ocpn_vp;
    ocpn_vp.clat = vp->clat;
    ocpn_vp.clon = vp->clon;
    ocpn_vp.m_projection_type = vp->m_projection_type;
    ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
    ocpn_vp.skew = vp->skew;
    ocpn_vp.rotation = vp->rotation;
    ocpn_vp.pix_width = vp->pix_width;
    ocpn_vp.pix_height = vp->pix_height;

    wxPoint ret = ocpn_vp.GetPixFromLL(lat, lon);
    pp->x = ret.x;
    pp->y = ret.y;
}

void GetCanvasLLPix( PlugIn_ViewPort *vp, wxPoint p, double *plat, double *plon)
{
    //    Make enough of an application viewport to run its method....
    ViewPort ocpn_vp;
    ocpn_vp.clat = vp->clat;
    ocpn_vp.clon = vp->clon;
    ocpn_vp.m_projection_type = vp->m_projection_type;
    ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
    ocpn_vp.skew = vp->skew;
    ocpn_vp.rotation = vp->rotation;
    ocpn_vp.pix_width = vp->pix_width;
    ocpn_vp.pix_height = vp->pix_height;

    return ocpn_vp.GetLLFromPix( p, plat, plon);
}

bool GetGlobalColor(wxString colorName, wxColour *pcolour)
{
    wxColour c = GetGlobalColor(colorName);
    *pcolour = c;

    return true;
}

wxFont *OCPNGetFont(wxString TextElement, int default_size)
{
    return FontMgr::Get().GetFont(TextElement, default_size);
}

wxString *GetpSharedDataLocation(void)
{
    return &g_SData_Locn;
}


ArrayOfPlugIn_AIS_Targets *GetAISTargetArray(void)
{
    if ( !g_pAIS )
        return NULL;


    ArrayOfPlugIn_AIS_Targets *pret = new ArrayOfPlugIn_AIS_Targets;

    //      Iterate over the AIS Target Hashmap
    AIS_Target_Hash::iterator it;

    AIS_Target_Hash *current_targets = g_pAIS->GetTargetList();

    for ( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it )
    {
        AIS_Target_Data *td = it->second;
        PlugIn_AIS_Target *ptarget = Create_PI_AIS_Target(td);
        pret->Add(ptarget);
    }

    return pret;
}


wxAuiManager *GetFrameAuiManager(void)
{
    return g_pauimgr;
}

bool AddLocaleCatalog( wxString catalog )
{
    if(plocale_def_lang)
        return plocale_def_lang->AddCatalog( catalog );
    else
        return false;
}

void PushNMEABuffer( wxString buf )
{
    OCPN_DataStreamEvent event( wxEVT_OCPN_DATASTREAM, 0 );
    std::string s = std::string( buf.mb_str() );
    event.SetNMEAString( s );
    event.SetStream( NULL );

    g_pMUX->AddPendingEvent( event );
}

wxXmlDocument GetChartDatabaseEntryXML(int dbIndex, bool b_getGeom)
{

    wxXmlDocument doc = ChartData->GetXMLDescription(dbIndex, b_getGeom);

    return doc;
}

bool UpdateChartDBInplace(wxArrayString dir_array,
                          bool b_force_update,
                          bool b_ProgressDialog)
{
    //    Make an array of CDI
    ArrayOfCDI ChartDirArray;
    for(unsigned int i=0 ; i < dir_array.GetCount(); i++)
    {
        wxString dirname = dir_array.Item(i);
        ChartDirInfo cdi;
        cdi.fullpath = dirname;
        cdi.magic_number = _T("");
        ChartDirArray.Add ( cdi );
    }

    bool b_ret = gFrame->UpdateChartDatabaseInplace(ChartDirArray,
                b_force_update, b_ProgressDialog,
                ChartData->GetDBFileName());

    ViewPort vp;
    gFrame->ChartsRefresh(-1, vp);

    return b_ret;
}

wxArrayString GetChartDBDirArrayString()
{
    return ChartData->GetChartDirArrayString();
}

void SendPluginMessage( wxString message_id, wxString message_body )
{
    s_ppim->SendMessageToAllPlugins(message_id, message_body);

    //  We will send an event to the main application frame (gFrame)
    //  for informational purposes.
    //  Of course, gFrame is encouraged to use any or all the
    //  data flying by if judged useful and dependable....

    OCPN_MsgEvent Nevent(wxEVT_OCPN_MSG, 0);
    Nevent.SetID(message_id);
    Nevent.SetJSONText(message_body);
    gFrame->GetEventHandler()->AddPendingEvent( Nevent );
}

void DimeWindow(wxWindow *win)
{
    DimeControl(win);
}

void JumpToPosition(double lat, double lon, double scale)
{
    gFrame->JumpToPosition(lat, lon, scale);
}

/* API 1.9 */
wxScrolledWindow *AddOptionsPage( OptionsParentPI parent, wxString title )
{
    if (! g_pOptions) return NULL;

    size_t parentid;
    switch (parent) {
    case PI_OPTIONS_PARENT_DISPLAY:
        parentid = g_pOptions->m_pageDisplay;
    break;
    case PI_OPTIONS_PARENT_CONNECTIONS:
        parentid = g_pOptions->m_pageConnections;
    break;
    case PI_OPTIONS_PARENT_CHARTS:
        parentid = g_pOptions->m_pageCharts;
    break;
    case PI_OPTIONS_PARENT_SHIPS:
        parentid = g_pOptions->m_pageShips;
    break;
    case PI_OPTIONS_PARENT_UI:
        parentid = g_pOptions->m_pageUI;
    break;
    case PI_OPTIONS_PARENT_PLUGINS:
        parentid = g_pOptions->m_pagePlugins;
    break;
    default:
        wxLogMessage( _T("Error in PluginManager::AddOptionsPage: Unknown parent") );
        return NULL;
    break;
    }

    return g_pOptions->AddPage( parentid, title );
}

bool DeleteOptionsPage( wxScrolledWindow* page )
{
    if (! g_pOptions) return false;
    return g_pOptions->DeletePage( page );
}

bool DecodeSingleVDOMessage( const wxString& str, PlugIn_Position_Fix_Ex *pos, wxString *accumulator )
{
    if(!pos)
        return false;

    GenericPosDatEx gpd;
    AIS_Error nerr = AIS_GENERIC_ERROR;
    if(g_pAIS)
        nerr = g_pAIS->DecodeSingleVDO(str, &gpd, accumulator);
    if(nerr == AIS_NoError){
        pos->Lat = gpd.kLat;
        pos->Lon = gpd.kLon;
        pos->Cog = gpd.kCog;
        pos->Sog = gpd.kSog;
        pos->Hdt = gpd.kHdt;

        //  Fill in the dummy values
        pos->FixTime = 0;
        pos->Hdm = 1000;
        pos->Var = 1000;
        pos->nSats = 0;

        return true;
    }

    return false;
}

int GetChartbarHeight( void )
{
    if( stats && stats->IsShown() ){
        wxSize s = stats->GetSize();
        return s.GetHeight();
    }
    else
        return 0;
}


bool GetRoutepointGPX( RoutePoint *pRoutePoint, char *buffer, unsigned int buffer_length)
{
    bool ret = false;

    NavObjectCollection *pgpx = new NavObjectCollection;
    pgpx->AddGPXWaypoint( pRoutePoint);
    wxString gpxfilename = wxFileName::CreateTempFileName(wxT("gpx"));
    pgpx->SaveFile(gpxfilename);
    delete pgpx;

    wxFFile gpxfile( gpxfilename );
    wxString s;
    if( gpxfile.ReadAll( &s ) ) {
        if(s.Length() < buffer_length) {
            strncpy(buffer, (const char*)s.mb_str(wxConvUTF8), buffer_length -1);
            ret = true;
        }
    }

    gpxfile.Close();
    ::wxRemoveFile(gpxfilename);

    return ret;
}

bool GetActiveRoutepointGPX( char *buffer, unsigned int buffer_length )
{
    if( g_pRouteMan->IsAnyRouteActive() )
        return GetRoutepointGPX( g_pRouteMan->GetpActivePoint(), buffer, buffer_length);
    else
        return false;
}

void PositionBearingDistanceMercator_Plugin(double lat, double lon,
                                            double brg, double dist,
                                            double *dlat, double *dlon)
{
    PositionBearingDistanceMercator(lat, lon, brg, dist, dlat, dlon);
}

void DistanceBearingMercator_Plugin(double lat0, double lon0, double lat1, double lon1, double *brg, double *dist)
{
    DistanceBearingMercator( lat0, lon0, lat1, lon1, brg, dist);
}

double DistGreatCircle_Plugin(double slat, double slon, double dlat, double dlon)
{
    return DistGreatCircle(slat, slon, dlat, dlon);
}

void toTM_Plugin(float lat, float lon, float lat0, float lon0, double *x, double *y)
{
    toTM(lat, lon, lat0, lon0, x, y);
}

void fromTM_Plugin(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
    fromTM(x, y, lat0, lon0, lat, lon);
}

void toSM_Plugin(double lat, double lon, double lat0, double lon0, double *x, double *y)
{
    toSM(lat, lon, lat0, lon0, x, y);
}

void fromSM_Plugin(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
    fromSM(x, y, lat0, lon0, lat, lon);
}

void toSM_ECC_Plugin(double lat, double lon, double lat0, double lon0, double *x, double *y)
{
    toSM_ECC(lat, lon, lat0, lon0, x, y);
}

void fromSM_ECC_Plugin(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
    fromSM_ECC(x, y, lat0, lon0, lat, lon);
}

double toUsrDistance_Plugin( double nm_distance, int unit )
{
    return toUsrDistance( nm_distance, unit );
}

double fromUsrDistance_Plugin( double usr_distance, int unit )
{
    return fromUsrDistance( usr_distance, unit );
}

double toUsrSpeed_Plugin( double kts_speed, int unit )
{
    return toUsrSpeed( kts_speed, unit );
}

double fromUsrSpeed_Plugin( double usr_speed, int unit )
{
    return fromUsrSpeed( usr_speed, unit );
}

wxString getUsrDistanceUnit_Plugin( int unit )
{
    return getUsrDistanceUnit( unit );
}

wxString getUsrSpeedUnit_Plugin( int unit )
{
    return getUsrSpeedUnit( unit );
}

bool PlugIn_GSHHS_CrossesLand(double lat1, double lon1, double lat2, double lon2)
{
	static GshhsReader * reader = NULL;
	static Projection proj;

	if(reader == NULL)
		reader = new GshhsReader(&proj);

	while (lon1 < 0)
		lon1 += 360;
	while (lon2 < 0)
		lon2 += 360;

	return reader->crossing1(QLineF(lon1, lat1, lon2, lat2));
}


void PlugInPlaySound( wxString &sound_file )
{
    if(g_pi_manager) {
        g_pi_manager->m_plugin_sound.Stop();
        g_pi_manager->m_plugin_sound.UnLoad();

        g_pi_manager->m_plugin_sound.Create( sound_file );

        if( g_pi_manager->m_plugin_sound.IsOk() )
            g_pi_manager->m_plugin_sound.Play();
    }
}

// API 1.10 Route and Waypoint Support
//wxBitmap *FindSystemWaypointIcon( wxString& icon_name );

//      PlugInWaypoint implementation
PlugIn_Waypoint::PlugIn_Waypoint()
{
    m_HyperlinkList = NULL;
}

PlugIn_Waypoint::PlugIn_Waypoint(double lat, double lon,
                const wxString& icon_ident, const wxString& wp_name,
                const wxString& GUID )
{
    wxDateTime now = wxDateTime::Now();
    m_CreateTime = now.ToUTC();
    m_HyperlinkList = NULL;

    m_lat = lat;
    m_lon = lon;
    m_IconName = icon_ident;
    m_MarkName = wp_name;
    m_GUID = GUID;
}

PlugIn_Waypoint::~PlugIn_Waypoint()
{
}

//      PlugInRoute implementation
PlugIn_Route::PlugIn_Route(void )
{
    pWaypointList = new Plugin_WaypointList;
}

PlugIn_Route::~PlugIn_Route(void )
{
    pWaypointList->DeleteContents( false );            // do not delete Waypoints
    pWaypointList->Clear();

    delete pWaypointList;
}

//      PlugInTrack implementation
PlugIn_Track::PlugIn_Track(void )
{
    pWaypointList = new Plugin_WaypointList;
}

PlugIn_Track::~PlugIn_Track(void )
{
    pWaypointList->DeleteContents( false );            // do not delete Waypoints
    pWaypointList->Clear();

    delete pWaypointList;
}



wxString GetNewGUID( void )
{
    return GpxDocument::GetUUID();
}

bool AddCustomWaypointIcon( wxBitmap *pimage, wxString key, wxString description )
{
    pWayPointMan->ProcessIcon( *pimage, key, description );
    return true;
}


bool AddSingleWaypoint( PlugIn_Waypoint *pwaypoint, bool b_permanent)
{
    //  Validate the waypoint parameters a little bit

    //  GUID
    //  Make sure that this GUID is indeed unique in the Routepoint list
    bool b_unique = true;
    wxRoutePointListNode *prpnode = pWayPointMan->m_pWayPointList->GetFirst();
    while( prpnode ) {
        RoutePoint *prp = prpnode->GetData();

        if( prp->m_GUID == pwaypoint->m_GUID ) {
            b_unique = false;
            break;
        }
        prpnode = prpnode->GetNext(); //RoutePoint
    }

    if( !b_unique )
        return false;

    RoutePoint *pWP = new RoutePoint( pwaypoint->m_lat, pwaypoint->m_lon,
                                      pwaypoint->m_IconName, pwaypoint->m_MarkName,
                                      pwaypoint->m_GUID );

    pWP->m_bIsolatedMark = true;                      // This is an isolated mark


    //  Transcribe (clone) the html HyperLink List, if present
    if( pwaypoint->m_HyperlinkList ) {
        if( pwaypoint->m_HyperlinkList->GetCount() > 0 ) {
            wxPlugin_HyperlinkListNode *linknode = pwaypoint->m_HyperlinkList->GetFirst();
            while( linknode ) {
                Plugin_Hyperlink *link = linknode->GetData();

                Hyperlink* h = new Hyperlink();
                h->DescrText = link->DescrText;
                h->Link = link->Link;
                h->LType = link->Type;

                pWP->m_HyperlinkList->Append( h );

                linknode = linknode->GetNext();
            }
        }
    }

    pWP->m_MarkDescription = pwaypoint->m_MarkDescription;
    pWP->m_btemp = (b_permanent == false);

    pSelect->AddSelectableRoutePoint( pwaypoint->m_lat, pwaypoint->m_lon, pWP );
    if(b_permanent)
        pConfig->AddNewWayPoint( pWP, -1 );

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
        pRouteManagerDialog->UpdateWptListCtrl();

    return true;
}

bool DeleteSingleWaypoint( wxString &GUID )
{
    //  Find the RoutePoint
    bool b_found = false;
    RoutePoint *prp = pWayPointMan->FindRoutePoint(GUID);

    if(prp)
        b_found = true;

    if( b_found ) {
        pWayPointMan->DestroyWaypoint( prp );
        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
            pRouteManagerDialog->UpdateWptListCtrl();
    }

    return b_found;
}


bool UpdateSingleWaypoint( PlugIn_Waypoint *pwaypoint )
{
    //  Find the RoutePoint
    bool b_found = false;
    RoutePoint *prp = pWayPointMan->FindRoutePoint(pwaypoint->m_GUID);

    if(prp)
        b_found = true;

    if( b_found ) {
        double lat_save = prp->m_lat;
        double lon_save = prp->m_lon;

        prp->m_lat = pwaypoint->m_lat;
        prp->m_lon = pwaypoint->m_lon;
        prp->m_IconName = pwaypoint->m_IconName;
        prp->SetName( pwaypoint->m_MarkName );
        prp->m_MarkDescription = pwaypoint->m_MarkDescription;

        //  Transcribe (clone) the html HyperLink List, if present

        if( pwaypoint->m_HyperlinkList ) {
            prp->m_HyperlinkList->Clear();
            if( pwaypoint->m_HyperlinkList->GetCount() > 0 ) {
                wxPlugin_HyperlinkListNode *linknode = pwaypoint->m_HyperlinkList->GetFirst();
                while( linknode ) {
                    Plugin_Hyperlink *link = linknode->GetData();

                    Hyperlink* h = new Hyperlink();
                    h->DescrText = link->DescrText;
                    h->Link = link->Link;
                    h->LType = link->Type;

                    prp->m_HyperlinkList->Append( h );

                    linknode = linknode->GetNext();
                }
            }
        }

        SelectItem *pFind = pSelect->FindSelection( lat_save, lon_save, Select::TYPE_ROUTEPOINT );
        if( pFind ) {
            pFind->m_slat = pwaypoint->m_lat;             // update the SelectList entry
            pFind->m_slon = pwaypoint->m_lon;
        }

        if(!prp->m_btemp)
            pConfig->UpdateWayPoint( prp );

        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
            pRouteManagerDialog->UpdateWptListCtrl();
    }

    return b_found;
}


bool AddPlugInRoute( PlugIn_Route *proute, bool b_permanent )
{
    Route *route = new Route();

    PlugIn_Waypoint *pwp;
    RoutePoint *pWP_src;
    int ip = 0;

    wxPlugin_WaypointListNode *pwpnode = proute->pWaypointList->GetFirst();
    while( pwpnode ) {
        pwp = pwpnode->GetData();

        RoutePoint *pWP = new RoutePoint( pwp->m_lat, pwp->m_lon,
                                          pwp->m_IconName, pwp->m_MarkName,
                                          pwp->m_GUID );

        //  Transcribe (clone) the html HyperLink List, if present
        if( pwp->m_HyperlinkList ) {
            if( pwp->m_HyperlinkList->GetCount() > 0 ) {
                wxPlugin_HyperlinkListNode *linknode = pwp->m_HyperlinkList->GetFirst();
                while( linknode ) {
                    Plugin_Hyperlink *link = linknode->GetData();

                    Hyperlink* h = new Hyperlink();
                    h->DescrText = link->DescrText;
                    h->Link = link->Link;
                    h->LType = link->Type;

                    pWP->m_HyperlinkList->Append( h );

                    linknode = linknode->GetNext();
                }
            }
        }

        pWP->m_MarkDescription = pwp->m_MarkDescription;
        pWP->m_bShowName = false;
        pWP->SetCreateTime(pwp->m_CreateTime);

        route->AddPoint( pWP );


        pSelect->AddSelectableRoutePoint( pWP->m_lat, pWP->m_lon, pWP );

        if(ip > 0)
            pSelect->AddSelectableRouteSegment( pWP_src->m_lat, pWP_src->m_lon, pWP->m_lat,
                                            pWP->m_lon, pWP_src, pWP, route );
        ip++;
        pWP_src = pWP;

        pwpnode = pwpnode->GetNext(); //PlugInWaypoint
    }

    route->m_RouteNameString = proute->m_NameString;
    route->m_RouteStartString = proute->m_StartString;
    route->m_RouteEndString = proute->m_EndString;
    route->m_GUID = proute->m_GUID;
    route->m_btemp = (b_permanent == false);

    pRouteList->Append( route );

    if(b_permanent)
        pConfig->AddNewRoute( route );

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
        pRouteManagerDialog->UpdateRouteListCtrl();

    return true;
}



bool DeletePluginRoute( wxString& GUID )
{
    bool b_found = false;

    //  Find the Route
    Route *pRoute = g_pRouteMan->FindRouteByGUID( GUID );
    if(pRoute) {
        g_pRouteMan->DeleteRoute( pRoute );
        b_found = true;
    }
    return b_found;
}

bool UpdatePlugInRoute ( PlugIn_Route *proute )
{
    bool b_found = false;

    //  Find the Route
    Route *pRoute = g_pRouteMan->FindRouteByGUID( proute->m_GUID );
    if(pRoute)
        b_found = true;

    if(b_found) {
        bool b_permanent = (pRoute->m_btemp == false);
        g_pRouteMan->DeleteRoute( pRoute );

        b_found = AddPlugInRoute( proute, b_permanent );
    }

    return b_found;
}


bool AddPlugInTrack( PlugIn_Track *ptrack, bool b_permanent )
{
    Track *track = new Track();

    PlugIn_Waypoint *pwp;
    RoutePoint *pWP_src;
    int ip = 0;

    wxPlugin_WaypointListNode *pwpnode = ptrack->pWaypointList->GetFirst();
    while( pwpnode ) {
        pwp = pwpnode->GetData();

        RoutePoint *pWP = new RoutePoint( pwp->m_lat, pwp->m_lon,
                                          pwp->m_IconName, pwp->m_MarkName,
                                          pwp->m_GUID );


        pWP->m_MarkDescription = pwp->m_MarkDescription;
        pWP->m_bShowName = false;
        pWP->SetCreateTime( pwp->m_CreateTime );

        track->AddPoint( pWP );

        pSelect->AddSelectableRoutePoint( pWP->m_lat, pWP->m_lon, pWP );

        if(ip > 0)
            pSelect->AddSelectableRouteSegment( pWP_src->m_lat, pWP_src->m_lon, pWP->m_lat,
                                                pWP->m_lon, pWP_src, pWP, track );
        ip++;
        pWP_src = pWP;

        pwpnode = pwpnode->GetNext(); //PlugInWaypoint
    }

    track->m_RouteNameString = ptrack->m_NameString;
    track->m_RouteStartString = ptrack->m_StartString;
    track->m_RouteEndString = ptrack->m_EndString;
    track->m_GUID = ptrack->m_GUID;
    track->m_btemp = (b_permanent == false);

    pRouteList->Append( track );

    if(b_permanent)
        pConfig->AddNewRoute( track );

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
        pRouteManagerDialog->UpdateTrkListCtrl();

    return true;
}



bool DeletePluginTrack( wxString& GUID )
{
    bool b_found = false;

    //  Find the Route
    Route *pRoute = g_pRouteMan->FindRouteByGUID( GUID );
    if(pRoute) {
        g_pRouteMan->DeleteTrack( (Track *)pRoute );
        b_found = true;
    }

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
        pRouteManagerDialog->UpdateRouteListCtrl();

    return b_found;
 }

bool UpdatePlugInTrack ( PlugIn_Track *ptrack )
{
    bool b_found = false;

    //  Find the Track
    Route *pRoute = g_pRouteMan->FindRouteByGUID( ptrack->m_GUID );
    if(pRoute)
        b_found = true;

    if(b_found) {
        bool b_permanent = (pRoute->m_btemp == false);
        g_pRouteMan->DeleteTrack( (Track *)pRoute );

        b_found = AddPlugInTrack( ptrack, b_permanent );
    }

    return b_found;
}



//-----------------------------------------------------------------------------------------
//    The opencpn_plugin base class implementation
//-----------------------------------------------------------------------------------------

opencpn_plugin::~opencpn_plugin()
{}

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

wxBitmap *opencpn_plugin::GetPlugInBitmap()
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    return new wxBitmap(style->GetIcon( _T("default_pi") ) );
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
    return _T("OpenCPN PlugIn Base Class\n\
PlugInManager created this base class");
}



void opencpn_plugin::SetPositionFix(PlugIn_Position_Fix &pfix)
{}

void opencpn_plugin::SetNMEASentence(wxString &sentence)
{}

void opencpn_plugin::SetAISSentence(wxString &sentence)
{}

int opencpn_plugin::GetToolbarToolCount(void)
{
    return 0;
}

int opencpn_plugin::GetToolboxPanelCount(void)
{
    return 0;
}

void opencpn_plugin::SetupToolboxPanel(int page_sel, wxNotebook* pnotebook)
{}

void opencpn_plugin::OnCloseToolboxPanel(int page_sel, int ok_apply_cancel)
{}

void opencpn_plugin::ShowPreferencesDialog( wxWindow* parent )
{}

void opencpn_plugin::OnToolbarToolCallback(int id)
{}

void opencpn_plugin::OnContextMenuItemCallback(int id)
{}

bool opencpn_plugin::RenderOverlay(wxMemoryDC *dc, PlugIn_ViewPort *vp)
{
    return false;
}

void opencpn_plugin::SetCursorLatLon(double lat, double lon)
{}

void opencpn_plugin::SetCurrentViewPort(PlugIn_ViewPort &vp)
{}

void opencpn_plugin::SetDefaults(void)
{}

void opencpn_plugin::ProcessParentResize(int x, int y)
{}

void opencpn_plugin::SetColorScheme(PI_ColorScheme cs)
{}

void opencpn_plugin::UpdateAuiStatus(void)
{}


wxArrayString opencpn_plugin::GetDynamicChartClassNameArray()
{
    wxArrayString array;
    return array;
}


//    Opencpn_Plugin_16 Implementation
opencpn_plugin_16::opencpn_plugin_16(void *pmgr)
    : opencpn_plugin(pmgr)
{
}

opencpn_plugin_16::~opencpn_plugin_16(void)
{}

bool opencpn_plugin_16::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    return false;
}

void opencpn_plugin_16::SetPluginMessage(wxString &message_id, wxString &message_body)
{}

//    Opencpn_Plugin_17 Implementation
opencpn_plugin_17::opencpn_plugin_17(void *pmgr)
    : opencpn_plugin(pmgr)
{
}

opencpn_plugin_17::~opencpn_plugin_17(void)
{}


bool opencpn_plugin_17::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    return false;
}

bool opencpn_plugin_17::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    return false;
}

void opencpn_plugin_17::SetPluginMessage(wxString &message_id, wxString &message_body)
{}


//    Opencpn_Plugin_18 Implementation
opencpn_plugin_18::opencpn_plugin_18(void *pmgr)
    : opencpn_plugin(pmgr)
{
}

opencpn_plugin_18::~opencpn_plugin_18(void)
{}


bool opencpn_plugin_18::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    return false;
}

bool opencpn_plugin_18::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    return false;
}

void opencpn_plugin_18::SetPluginMessage(wxString &message_id, wxString &message_body)
{}

void opencpn_plugin_18::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix)
{}


//    Opencpn_Plugin_19 Implementation
opencpn_plugin_19::opencpn_plugin_19(void *pmgr)
    : opencpn_plugin_18(pmgr)
{
}

opencpn_plugin_19::~opencpn_plugin_19(void)
{
}

void opencpn_plugin_19::OnSetupOptions(void)
{
}

//    Opencpn_Plugin_110 Implementation
opencpn_plugin_110::opencpn_plugin_110(void *pmgr)
: opencpn_plugin_19(pmgr)
{
}

opencpn_plugin_110::~opencpn_plugin_110(void)
{
}

void opencpn_plugin_110::LateInit(void)
{
}



//          Helper and interface classes

//-------------------------------------------------------------------------------
//    PlugIn_AIS_Target Implementation
//-------------------------------------------------------------------------------

PlugIn_AIS_Target *Create_PI_AIS_Target(AIS_Target_Data *ptarget)
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

int PlugInChartBase::Init( const wxString& name, int init_flags )
{
    return 0;
}

//    Accessors

double PlugInChartBase::GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom)
{
    return 1.0;
}

double PlugInChartBase::GetNormalScaleMax(double canvas_scale_factor, int canvas_width)
{
    return 2.0e7;
}

bool PlugInChartBase::GetChartExtent(ExtentPI *pext)
{
    return false;
}


wxBitmap& PlugInChartBase::RenderRegionView(const PlugIn_ViewPort& VPoint,
        const wxRegion &Region)
{
    return wxNullBitmap;
}


bool PlugInChartBase::AdjustVP(PlugIn_ViewPort &vp_last, PlugIn_ViewPort &vp_proposed)
{
    return false;
}

void PlugInChartBase::GetValidCanvasRegion(const PlugIn_ViewPort& VPoint, wxRegion *pValidRegion)
{}

void PlugInChartBase::SetColorScheme(int cs, bool bApplyImmediate)
{}

double PlugInChartBase::GetNearestPreferredScalePPM(double target_scale_ppm)
{
    return 1.0;
}

wxBitmap *PlugInChartBase::GetThumbnail(int tnx, int tny, int cs)
{
    return NULL;
}

void PlugInChartBase::ComputeSourceRectangle(const PlugIn_ViewPort &vp, wxRect *pSourceRect)
{}

double PlugInChartBase::GetRasterScaleFactor()
{
    return 1.0;
}

bool PlugInChartBase::GetChartBits( wxRect& source, unsigned char *pPix, int sub_samp )
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

void PlugInChartBase::latlong_to_chartpix(double lat, double lon, double &pixx, double &pixy)
{}

