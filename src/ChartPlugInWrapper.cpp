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

#include <wx/wx.h>
#include "ChartPlugInWrapper.h"
#include <OCPNRegion.h>
#include <MessageBox.h>
#include <ViewPort.h>
#include <PositionParser.h>
#include <UserColors.h>
#include <ocpn_pixel.h>
#include <geo/GeoRef.h>
#include <chart/s52s57.h>
#include <chart/s52plib.h>
#include <plugin/ocpn_plugin.h>
#include <plugin/PlugInManager.h>
#include <wx/image.h>
#include <wx/bitmap.h>

extern wxString gExe_path;
extern bool g_b_useStencil;

#ifdef USE_S57
extern chart::s52plib* ps52plib;
#endif

using namespace chart;

PlugIn_ViewPort CreatePlugInViewport(const ViewPort& vp);

ChartPlugInWrapper::ChartPlugInWrapper()
{
}

ChartPlugInWrapper::ChartPlugInWrapper(const wxString& chart_class)
{
	m_ppo = ::wxCreateDynamicObject(chart_class);
	m_ppicb = wxDynamicCast(m_ppo, PlugInChartBase);
}

ChartPlugInWrapper::~ChartPlugInWrapper()
{
	if (m_ppicb)
		delete m_ppicb;
}

wxString ChartPlugInWrapper::GetFileSearchMask(void)
{
	if (m_ppicb)
		return m_ppicb->GetFileSearchMask();
	else
		return _T("");
}

chart::InitReturn ChartPlugInWrapper::Init(const wxString& name, chart::ChartInitFlag init_flags)
{
	if (m_ppicb) {
		chart::InitReturn ret_val = (chart::InitReturn)m_ppicb->Init(name, (int)init_flags);

		// Here we transcribe all the required wrapped member elements up into the chartbase object
		// which is the parent of this class
		if (ret_val == chart::INIT_OK) {
			m_FullPath = m_ppicb->GetFullPath();
			m_ChartType = (chart::ChartTypeEnum)m_ppicb->GetChartType();
			m_ChartFamily = (chart::ChartFamilyEnum)m_ppicb->GetChartFamily();
			m_projection = (OcpnProjType)m_ppicb->GetChartProjection();
			m_EdDate = m_ppicb->GetEditionDate();
			m_Name = m_ppicb->GetName();
			m_ID = m_ppicb->GetID();
			m_DepthUnits = m_ppicb->GetDepthUnits();
			m_SoundingsDatum = m_ppicb->GetSoundingsDatum();
			m_datum_str = m_ppicb->GetDatumString();
			m_SE = m_ppicb->GetSE();
			m_EdDate = m_ppicb->GetEditionDate();
			m_ExtraInfo = m_ppicb->GetExtraInfo();
			Chart_Error_Factor = m_ppicb->GetChartErrorFactor();
			m_depth_unit_id = (chart::ChartDepthUnitType)m_ppicb->GetDepthUnitId();
			m_Chart_Skew = m_ppicb->GetChartSkew();
			m_Chart_Scale = m_ppicb->GetNativeScale();

			bReadyToRender = m_ppicb->IsReadyToRender();
		}

		return ret_val;
	} else
		return chart::INIT_FAIL_REMOVE;
}

//    Accessors
int ChartPlugInWrapper::GetCOVREntries() const
{
	if (m_ppicb)
		return m_ppicb->GetCOVREntries();
	else
		return 0;
}

int ChartPlugInWrapper::GetCOVRTablePoints(int iTable) const
{
	if (m_ppicb)
		return m_ppicb->GetCOVRTablePoints(iTable);
	else
		return 0;
}

int ChartPlugInWrapper::GetCOVRTablenPoints(int iTable) const
{
	if (m_ppicb)
		return m_ppicb->GetCOVRTablenPoints(iTable);
	else
		return 0;
}

float* ChartPlugInWrapper::GetCOVRTableHead(int iTable)
{
	if (m_ppicb)
		return m_ppicb->GetCOVRTableHead(iTable);
	else
		return 0;
}

// TODO
// PlugIn chart types do not properly support NoCovr Regions
// Proper fix is to update PlugIn Chart Type API
// Derive an extended PlugIn chart class from existing class,
// and use some kind of RTTI to figure out which class to call.
int ChartPlugInWrapper::GetNoCOVREntries() const
{
	return 0;
}

int ChartPlugInWrapper::GetNoCOVRTablePoints(int) const
{
	return 0;
}

int ChartPlugInWrapper::GetNoCOVRTablenPoints(int) const
{
	return 0;
}

float* ChartPlugInWrapper::GetNoCOVRTableHead(int)
{
	return 0;
}

bool ChartPlugInWrapper::GetChartExtent(chart::Extent& ext) const
{
	if (m_ppicb) {
		ExtentPI xpi;
		if (m_ppicb->GetChartExtent(&xpi)) {
			ext.NLAT = xpi.NLAT;
			ext.SLAT = xpi.SLAT;
			ext.ELON = xpi.ELON;
			ext.WLON = xpi.WLON;

			return true;
		} else
			return false;
	} else
		return false;
}

ThumbData* ChartPlugInWrapper::GetThumbData(int tnx, int tny, float, float)
{
	if (m_ppicb) {

		// Create the bitmap if needed, doing a deep copy from the Bitmap owned by the PlugIn
		// Chart
		if (!pThumbData->pDIBThumb) {
			wxBitmap* pBMPOwnedByChart = m_ppicb->GetThumbnail(tnx, tny, m_global_color_scheme);
			if (pBMPOwnedByChart) {
				wxImage img = pBMPOwnedByChart->ConvertToImage();
				pThumbData->pDIBThumb = new wxBitmap(img);
			} else
				pThumbData->pDIBThumb = NULL;
		}

		pThumbData->Thumb_Size_X = tnx;
		pThumbData->Thumb_Size_Y = tny;

		pThumbData->ShipX = 0;
		pThumbData->ShipY = 0;

		return pThumbData;
	} else
		return NULL;
}

ThumbData* ChartPlugInWrapper::GetThumbData()
{
	return pThumbData;
}

bool ChartPlugInWrapper::UpdateThumbData(double, double)
{
	return true;
}

double ChartPlugInWrapper::GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom) const
{
	if (m_ppicb)
		return m_ppicb->GetNormalScaleMin(canvas_scale_factor, b_allow_overzoom);
	else
		return 1.0;
}

double ChartPlugInWrapper::GetNormalScaleMax(double canvas_scale_factor, int canvas_width) const
{
	if (m_ppicb)
		return m_ppicb->GetNormalScaleMax(canvas_scale_factor, canvas_width);
	else
		return 2.0e7;
}

bool ChartPlugInWrapper::RenderRegionViewOnGL(const wxGLContext& glc, const ViewPort& VPoint,
											  const OCPNRegion& Region)
{
	if (m_ppicb) {
		PlugIn_ViewPort pivp = CreatePlugInViewport(VPoint);
		OCPNRegion rg = Region;
		wxRegion r = rg.ConvertTowxRegion();
		PlugInChartBaseGL* ppicb_gl = dynamic_cast<PlugInChartBaseGL*>(m_ppicb);
		if (ppicb_gl) {
			ppicb_gl->RenderRegionViewOnGL(glc, pivp, r, g_b_useStencil);
		}
		return true;
	} else
		return false;
	return true;
}

bool ChartPlugInWrapper::RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
											  const OCPNRegion& Region)
{
	if (m_ppicb) {
		PlugIn_ViewPort pivp = CreatePlugInViewport(VPoint);
		OCPNRegion rg = Region;
		wxRegion r = rg.ConvertTowxRegion();
		dc.SelectObject(m_ppicb->RenderRegionView(pivp, r));
		return true;
	} else
		return false;
}

bool ChartPlugInWrapper::AdjustVP(const ViewPort& vp_last, ViewPort& vp_proposed)
{
	if (m_ppicb) {
		PlugIn_ViewPort pivp_last = CreatePlugInViewport(vp_last);
		PlugIn_ViewPort pivp_proposed = CreatePlugInViewport(vp_proposed);
		return m_ppicb->AdjustVP(pivp_last, pivp_proposed);
	} else
		return false;
}

void ChartPlugInWrapper::GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion* pValidRegion)
{
	if (m_ppicb) {
		PlugIn_ViewPort pivp = CreatePlugInViewport(VPoint);
		m_ppicb->GetValidCanvasRegion(pivp, pValidRegion);
	}

	return;
}

void ChartPlugInWrapper::SetColorScheme(ColorScheme cs, bool bApplyImmediate)
{
	if (m_ppicb)
		m_ppicb->SetColorScheme(cs, bApplyImmediate);
}

double ChartPlugInWrapper::GetNearestPreferredScalePPM(double target_scale_ppm)
{
	if (m_ppicb)
		return m_ppicb->GetNearestPreferredScalePPM(target_scale_ppm);
	else
		return 1.0;
}

PlugInChartBase* ChartPlugInWrapper::GetPlugInChart(void)
{
	return m_ppicb;
}

void ChartPlugInWrapper::ComputeSourceRectangle(const ViewPort& VPoint, wxRect* pSourceRect)
{
	if (m_ppicb) {
		PlugIn_ViewPort pivp = CreatePlugInViewport(VPoint);
		m_ppicb->ComputeSourceRectangle(pivp, pSourceRect);
	}
}

double ChartPlugInWrapper::GetRasterScaleFactor() const
{
	if (m_ppicb)
		return m_ppicb->GetRasterScaleFactor();
	else
		return 1.0;
}

bool ChartPlugInWrapper::GetChartBits(wxRect& source, unsigned char* pPix, int sub_samp)
{
	if (m_ppicb)

		return m_ppicb->GetChartBits(source, pPix, sub_samp);
	else
		return false;
}

int ChartPlugInWrapper::GetSize_X() const
{
	if (m_ppicb)
		return m_ppicb->GetSize_X();
	else
		return 1;
}

int ChartPlugInWrapper::GetSize_Y() const
{
	if (m_ppicb)
		return m_ppicb->GetSize_Y();
	else
		return 1;
}

void ChartPlugInWrapper::latlong_to_chartpix(double lat, double lon, double& pixx, double& pixy)
{
	if (m_ppicb)
		m_ppicb->latlong_to_chartpix(lat, lon, pixx, pixy);
}

/* API 1.11  */

/* API 1.11  adds some more common functions to avoid unnecessary code duplication */

wxString toSDMM_PlugIn(int NEflag, double a, bool hi_precision)
{
	return toSDMM(NEflag, a, hi_precision);
}

wxColour GetBaseGlobalColor(wxString colorName)
{
	return GetGlobalColor(colorName);
}

int OCPNMessageBox_PlugIn(wxWindow* parent, const wxString& message, const wxString& caption,
						  int style, int x, int y)
{
	return OCPNMessageBox(parent, message, caption, style, 10, x, y);
}

wxString GetOCPN_ExePath(void)
{
	return gExe_path;
}

//      API 1.11 Access to Vector PlugIn charts

ListOfPI_S57Obj *PlugInManager::GetPlugInObjRuleListAtLatLon( ChartPlugInWrapper *target, float zlat, float zlon,
                                                 float SelectRadius, const ViewPort& vp )
{
    if(target) {
        PlugInChartBaseGL *picbgl = dynamic_cast <PlugInChartBaseGL *>(target->GetPlugInChart());
        if(picbgl){
            PlugIn_ViewPort pi_vp = CreatePlugInViewport( vp );
            ListOfPI_S57Obj *piol = picbgl->GetObjRuleListAtLatLon(zlat, zlon, SelectRadius, &pi_vp);

            return piol;
        }
        else
            return NULL;
    }
    else
        return NULL;
}

wxString PlugInManager::CreateObjDescriptions( ChartPlugInWrapper *target, ListOfPI_S57Obj *rule_list )
{
    wxString ret_str;
    if(target) {
        PlugInChartBaseGL *picbgl = dynamic_cast <PlugInChartBaseGL *>(target->GetPlugInChart());
        if(picbgl){
            ret_str = picbgl->CreateObjDescriptions( rule_list );
        }
    }

    return ret_str;
}



//      API 1.11 Access to S52 PLIB
wxString PI_GetPLIBColorScheme()
{
	return _T(""); // ps52plib->GetPLIBColorScheme()
}

int PI_GetPLIBDepthUnitInt()
{
    if(ps52plib)
        return ps52plib->m_nDepthUnitDisplay;
    else
        return 0;
}

int PI_GetPLIBSymbolStyle()
{
    if(ps52plib)
        return ps52plib->m_nSymbolStyle;
    else
        return 0;
}

int PI_GetPLIBBoundaryStyle()
{
    if(ps52plib)
        return ps52plib->m_nBoundaryStyle;
    else
        return 0;
}

bool PI_PLIBObjectRenderCheck(PI_S57Obj* pObj, PlugIn_ViewPort* vp)
{
    if(ps52plib) {
        //  Create and populate a compatible s57 Object
        chart::S57Obj cobj;
        CreateCompatibleS57Object( pObj, &cobj );

        ViewPort cvp = CreateCompatibleViewport( *vp );

        S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;

        //  Create and populate a minimally compatible object container
        ObjRazRules rzRules;
        rzRules.obj = &cobj;
        rzRules.LUP = pContext->LUP;
        rzRules.sm_transform_parms = 0;
        rzRules.child = NULL;
        rzRules.next = NULL;

        return ps52plib->ObjectRenderCheck( &rzRules, cvp );
    }
    else
        return false;
}

int PI_GetPLIBStateHash()
{
    if(ps52plib)
        return ps52plib->GetStateHash();
    else
        return 0;
}

void CreateCompatibleS57Object(PI_S57Obj* pObj, chart::S57Obj* cobj)
{
	strncpy(cobj->FeatureName, pObj->FeatureName, 8);
	cobj->Primitive_type = (GeoPrim_t)pObj->Primitive_type;
	cobj->att_array = pObj->att_array;
	cobj->attVal = pObj->attVal;
	cobj->n_attr = pObj->n_attr;

	cobj->x = pObj->x;
	cobj->y = pObj->y;
	cobj->z = pObj->z;
	cobj->npt = pObj->npt;

	cobj->iOBJL = pObj->iOBJL;
	cobj->Index = pObj->Index;

	cobj->geoPt = (pt*)pObj->geoPt;
	cobj->geoPtz = pObj->geoPtz;
	cobj->geoPtMulti = pObj->geoPtMulti;

	cobj->m_lat = pObj->m_lat;
	cobj->m_lon = pObj->m_lon;

	cobj->m_DisplayCat = (DisCat)pObj->m_DisplayCat;
	cobj->x_rate = pObj->x_rate;
	cobj->y_rate = pObj->y_rate;
	cobj->x_origin = pObj->x_origin;
	cobj->y_origin = pObj->y_origin;

	cobj->Scamin = pObj->Scamin;
	cobj->nRef = pObj->nRef;
	cobj->bIsAton = pObj->bIsAton;
	cobj->bIsAssociable = pObj->bIsAssociable;

	cobj->m_n_lsindex = pObj->m_n_lsindex;
	cobj->m_lsindex_array = pObj->m_lsindex_array;
	cobj->m_n_edge_max_points = pObj->m_n_edge_max_points;

	cobj->pPolyTessGeo = (geo::PolyTessGeo*)pObj->pPolyTessGeo;
	cobj->m_chart_context = (chart_context*)pObj->m_chart_context;

	cobj->bBBObj_valid = pObj->bBBObj_valid;
	if (pObj->bBBObj_valid) {
		cobj->BBObj.SetMin(pObj->lon_min, pObj->lat_min);
		cobj->BBObj.SetMax(pObj->lon_max, pObj->lat_max);
	}

	S52PLIB_Context* pContext = (S52PLIB_Context*)pObj->S52_Context;

	cobj->CSrules = pContext->CSrules;
	cobj->bCS_Added = pContext->bCS_Added;

	cobj->FText = pContext->FText;
	cobj->bFText_Added = pContext->bFText_Added;
	cobj->rText = pContext->rText;

	cobj->bIsClone = true; // Protect cloned object pointers in S57Obj dtor
}

bool PI_PLIBSetContext(PI_S57Obj* pObj)
{
	S52PLIB_Context* ctx;
	if (!pObj->S52_Context) {
		ctx = new S52PLIB_Context;
		pObj->S52_Context = ctx;
	}

	ctx = (S52PLIB_Context*)pObj->S52_Context;

	S57Obj cobj;
	CreateCompatibleS57Object(pObj, &cobj);

	LUPname LUP_Name;

	//      Force a re-evaluation of CS rules
	ctx->CSrules = NULL;
	ctx->bCS_Added = false;

	//      Clear the rendered text cache
	if (ctx->bFText_Added) {
		ctx->bFText_Added = false;
		delete ctx->FText;
		ctx->FText = NULL;
	}

	if (pObj->child) {
		wxASSERT(0);
	}

	// This is where Simplified or Paper-Type point features are selected
	switch (cobj.Primitive_type) {
		case chart::GEO_POINT:
		case chart::GEO_META:
		case chart::GEO_PRIM:

			if (PAPER_CHART == ps52plib->m_nSymbolStyle)
				LUP_Name = PAPER_CHART;
			else
				LUP_Name = SIMPLIFIED;

			break;

		case chart::GEO_LINE:
			LUP_Name = LINES;
			break;

		case chart::GEO_AREA:
			if (PLAIN_BOUNDARIES == ps52plib->m_nBoundaryStyle)
				LUP_Name = PLAIN_BOUNDARIES;
			else
				LUP_Name = SYMBOLIZED_BOUNDARIES;

			break;
	}

	LUPrec* lup = ps52plib->S52_LUPLookup(LUP_Name, cobj.FeatureName, &cobj);
	ctx->LUP = lup;

	//              Convert LUP to rules set
	ps52plib->_LUP2rules(lup, &cobj);
	return true;
}

void UpdatePIObjectPlibContext(PI_S57Obj* pObj, chart::S57Obj* cobj)
{
	//  Update the PLIB context after the render operation
	S52PLIB_Context* pContext = (S52PLIB_Context*)pObj->S52_Context;

	pContext->CSrules = cobj->CSrules;
	pContext->bCS_Added = cobj->bCS_Added;

	pContext->FText = cobj->FText;
	pContext->bFText_Added = cobj->bFText_Added;
	pContext->rText = cobj->rText;

	if (cobj->bBBObj_valid)
		pContext->BBObj = cobj->BBObj;
	pContext->bBBObj_valid = cobj->bBBObj_valid;
}

PI_LUPname PI_GetObjectLUPName(PI_S57Obj* pObj)
{
	S52PLIB_Context* pContext = (S52PLIB_Context*)pObj->S52_Context;
	if (pContext) {
		LUPrec* lup = pContext->LUP;
		if (lup)
			return (PI_LUPname)(lup->TNAM);
	}
	return (PI_LUPname)(-1);
}

PI_DisPrio PI_GetObjectDisplayPriority(PI_S57Obj* pObj)
{
	S52PLIB_Context* pContext = (S52PLIB_Context*)pObj->S52_Context;
	if (pContext) {
		LUPrec* lup = pContext->LUP;
		if (lup)
			return (PI_DisPrio)(lup->DPRI);
	}

	return (PI_DisPrio)(-1);
}

PI_DisCat PI_GetObjectDisplayCategory(PI_S57Obj* pObj)
{
	S52PLIB_Context* pContext = (S52PLIB_Context*)pObj->S52_Context;
	if (pContext) {
		LUPrec* lup = pContext->LUP;
		if (lup)
			return (PI_DisCat)(lup->DISC);
	}
	return (PI_DisCat)(-1);
}

void PI_PLIBSetLineFeaturePriority(PI_S57Obj* pObj, int prio)
{
	//  Create and populate a compatible s57 Object
	S57Obj cobj;

	CreateCompatibleS57Object(pObj, &cobj);

	S52PLIB_Context* pContext = (S52PLIB_Context*)pObj->S52_Context;

	//  Create and populate a minimally compatible object container
	ObjRazRules rzRules;
	rzRules.obj = &cobj;
	rzRules.LUP = pContext->LUP;
	rzRules.sm_transform_parms = 0;
	rzRules.child = NULL;
	rzRules.next = NULL;

	ps52plib->SetLineFeaturePriority(&rzRules, prio);

	//  Update the PLIB context after the render operation
	UpdatePIObjectPlibContext(pObj, &cobj);
}

void PI_PLIBPrepareForNewRender(void)
{
	if (ps52plib) {
		ps52plib->PrepareForRender();
		ps52plib->ClearTextList();
	}
}

int PI_PLIBRenderObjectToDC(wxDC* pdc, PI_S57Obj* pObj, PlugIn_ViewPort* vp)
{
	//  Create and populate a compatible s57 Object
	S57Obj cobj;

	CreateCompatibleS57Object(pObj, &cobj);

	S52PLIB_Context* pContext = (S52PLIB_Context*)pObj->S52_Context;

	//  Set up object SM rendering constants
	chart::sm_parms transform;
	geo::toSM(vp->clat, vp->clon, pObj->chart_ref_lat, pObj->chart_ref_lon, &transform.easting_vp_center,
		 &transform.northing_vp_center);

	//  Create and populate a minimally compatible object container
	ObjRazRules rzRules;
	rzRules.obj = &cobj;
	rzRules.LUP = pContext->LUP;
	rzRules.sm_transform_parms = &transform;
	rzRules.child = NULL;
	rzRules.next = NULL;

	ViewPort cvp = CreateCompatibleViewport(*vp);

	//  Do the render
	ps52plib->RenderObjectToDC(pdc, &rzRules, cvp);

	//  Update the PLIB context after the render operation
	UpdatePIObjectPlibContext(pObj, &cobj);

	return 1;
}

int PI_PLIBRenderAreaToDC(wxDC* pdc, PI_S57Obj* pObj, PlugIn_ViewPort* vp, wxRect rect,
						  unsigned char* pixbuf)
{
	// Create a compatible render canvas
	render_canvas_parms pb_spec;

	pb_spec.depth = BPP;
	pb_spec.pb_pitch = ((rect.width * pb_spec.depth / 8));
	pb_spec.lclip = rect.x;
	pb_spec.rclip = rect.x + rect.width - 1;
	pb_spec.pix_buff = pixbuf; // the passed buffer
	pb_spec.width = rect.width;
	pb_spec.height = rect.height;
	pb_spec.x = rect.x;
	pb_spec.y = rect.y;
#ifdef ocpnUSE_ocpnBitmap
	pb_spec.b_revrgb = true;
#else
	pb_spec.b_revrgb = false;
#endif

	// Create and populate a compatible s57 Object
	S57Obj cobj;

	CreateCompatibleS57Object(pObj, &cobj);

	S52PLIB_Context* pContext = (S52PLIB_Context*)pObj->S52_Context;

	// Set up object SM rendering constants
	chart::sm_parms transform;
	geo::toSM(vp->clat, vp->clon, pObj->chart_ref_lat, pObj->chart_ref_lon, &transform.easting_vp_center,
		 &transform.northing_vp_center);

	// Create and populate a minimally compatible object container
	ObjRazRules rzRules;
	rzRules.obj = &cobj;
	rzRules.LUP = pContext->LUP;
	rzRules.sm_transform_parms = &transform;
	rzRules.child = NULL;
	rzRules.next = NULL;

	ViewPort cvp = CreateCompatibleViewport(*vp);

	// Do the render
	ps52plib->RenderAreaToDC(pdc, &rzRules, cvp, &pb_spec);

	// Update the PLIB context after the render operation
	UpdatePIObjectPlibContext(pObj, &cobj);

	return 1;
}

int PI_PLIBRenderAreaToGL(const wxGLContext& glcc, PI_S57Obj* pObj, PlugIn_ViewPort* vp,
						  wxRect& render_rect)
{
	//  Create and populate a compatible s57 Object
	S57Obj cobj;

	CreateCompatibleS57Object(pObj, &cobj);

	S52PLIB_Context* pContext = (S52PLIB_Context*)pObj->S52_Context;

	//  Set up object SM rendering constants
	sm_parms transform;
	geo::toSM(vp->clat, vp->clon, pObj->chart_ref_lat, pObj->chart_ref_lon, &transform.easting_vp_center,
		 &transform.northing_vp_center);

	//  Create and populate a minimally compatible object container
	ObjRazRules rzRules;
	rzRules.obj = &cobj;
	rzRules.LUP = pContext->LUP;
	rzRules.sm_transform_parms = &transform;
	rzRules.child = NULL;
	rzRules.next = NULL;

	ViewPort cvp = CreateCompatibleViewport(*vp);

	//  Do the render
	ps52plib->RenderAreaToGL(glcc, &rzRules, cvp, render_rect);

	//  Update the PLIB context after the render operation
	UpdatePIObjectPlibContext(pObj, &cobj);

	return 1;
}

int PI_PLIBRenderObjectToGL(const wxGLContext& glcc, PI_S57Obj* pObj, PlugIn_ViewPort* vp,
							wxRect& render_rect)
{
	//  Create and populate a compatible s57 Object
	S57Obj cobj;

	CreateCompatibleS57Object(pObj, &cobj);

	S52PLIB_Context* pContext = (S52PLIB_Context*)pObj->S52_Context;

	//  Set up object SM rendering constants
	sm_parms transform;
	geo::toSM(vp->clat, vp->clon, pObj->chart_ref_lat, pObj->chart_ref_lon, &transform.easting_vp_center,
		 &transform.northing_vp_center);

	//  Create and populate a minimally compatible object container
	ObjRazRules rzRules;
	rzRules.obj = &cobj;
	rzRules.LUP = pContext->LUP;
	rzRules.sm_transform_parms = &transform;
	rzRules.child = NULL;
	rzRules.next = NULL;

	ViewPort cvp = CreateCompatibleViewport(*vp);

	//  Do the render
	ps52plib->RenderObjectToGL(glcc, &rzRules, cvp, render_rect);

	//  Update the PLIB context after the render operation
	UpdatePIObjectPlibContext(pObj, &cobj);

	return 1;
}

