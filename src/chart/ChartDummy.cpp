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

#include "ChartDummy.h"
#include "ViewPort.h"
#include "OCPNRegion.h"
#include <wx/bitmap.h>
#include <wx/dcmemory.h>

ChartDummy::ChartDummy()
	: m_pBM(NULL)
{
	m_ChartType = CHART_TYPE_DUMMY;
	m_ChartFamily = CHART_FAMILY_UNKNOWN;
	m_Chart_Scale = 22000000;

	m_FullPath = _("No Chart Available");
	m_Description = m_FullPath;
}

ChartDummy::~ChartDummy()
{
	delete m_pBM;
}

ThumbData * ChartDummy::GetThumbData()
{
	return pThumbData;
}

double ChartDummy::GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom)
{
	return 1.0;
}

double ChartDummy::GetNormalScaleMax(double canvas_scale_factor, int canvas_width)
{
	return 2.0e7;
}

InitReturn ChartDummy::Init(const wxString& name, ChartInitFlag init_flags)
{
	return INIT_OK;
}

double ChartDummy::GetNearestPreferredScalePPM(double target_scale_ppm)
{
	return target_scale_ppm;
}

void ChartDummy::SetColorScheme(ColorScheme cs, bool bApplyImmediate)
{
}

ThumbData *ChartDummy::GetThumbData(int tnx, int tny, float lat, float lon)
{
	return (ThumbData *)NULL;
}

bool ChartDummy::UpdateThumbData(double lat, double lon)
{
	return FALSE;
}

bool ChartDummy::GetChartExtent(Extent *pext)
{
	pext->NLAT = 80;
	pext->SLAT = -80;
	pext->ELON = 179;
	pext->WLON = -179;

	return true;
}

bool ChartDummy::RenderRegionViewOnGL(
		const wxGLContext & glc,
		const ViewPort & VPoint,
		const OCPNRegion & Region)
{
	return true;
}

bool ChartDummy::RenderRegionViewOnDC(
		wxMemoryDC & dc,
		const ViewPort & VPoint,
		const OCPNRegion & Region)
{
	return RenderViewOnDC(dc, VPoint);
}

bool ChartDummy::RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint)
{
	if( m_pBM  && m_pBM->IsOk() ) {
		if((m_pBM->GetWidth() != VPoint.pix_width) || (m_pBM->GetHeight() != VPoint.pix_height)) {
			delete m_pBM;
			m_pBM = NULL;
		}
	} else {
		delete m_pBM;
		m_pBM =NULL;
	}

	if( VPoint.pix_width && VPoint.pix_height ) {
		if(NULL == m_pBM)
			m_pBM = new wxBitmap(VPoint.pix_width, VPoint.pix_height,-1);

		dc.SelectObject(*m_pBM);

		dc.SetBackground(*wxBLACK_BRUSH);
		dc.Clear();
	}

	return true;
}

bool ChartDummy::AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed)
{
	return false;
}

void ChartDummy::GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion)
{
	pValidRegion->Clear();
	pValidRegion->Union(0, 0, 1, 1);
}
