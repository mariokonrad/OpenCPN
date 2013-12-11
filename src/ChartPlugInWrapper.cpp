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

#include "ChartPlugInWrapper.h"
#include <OCPNRegion.h>
#include <plugin/ocpn_plugin.h>
#include <wx/image.h>
#include <wx/bitmap.h>

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

bool ChartPlugInWrapper::GetChartExtent(chart::Extent* pext)
{
	if (m_ppicb) {
		ExtentPI xpi;
		if (m_ppicb->GetChartExtent(&xpi)) {
			pext->NLAT = xpi.NLAT;
			pext->SLAT = xpi.SLAT;
			pext->ELON = xpi.ELON;
			pext->WLON = xpi.WLON;

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
			wxImage img = pBMPOwnedByChart->ConvertToImage();
			pThumbData->pDIBThumb = new wxBitmap(img);
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

double ChartPlugInWrapper::GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom)
{
	if (m_ppicb)
		return m_ppicb->GetNormalScaleMin(canvas_scale_factor, b_allow_overzoom);
	else
		return 1.0;
}

double ChartPlugInWrapper::GetNormalScaleMax(double canvas_scale_factor, int canvas_width)
{
	if (m_ppicb)
		return m_ppicb->GetNormalScaleMax(canvas_scale_factor, canvas_width);
	else
		return 2.0e7;
}

bool ChartPlugInWrapper::RenderRegionViewOnGL(const wxGLContext&, const ViewPort&,
											  const OCPNRegion&)
{
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

void ChartPlugInWrapper::latlong_to_chartpix(double lat, double lon, double &pixx, double &pixy)
{
	if(m_ppicb)
		m_ppicb->latlong_to_chartpix(lat, lon, pixx, pixy);
}

