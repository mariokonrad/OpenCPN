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

#ifndef __CHART__CHARTDUMMY__H__
#define __CHART__CHARTDUMMY__H__

#include <chart/ChartBase.h>

class ChartDummy : public ChartBase
{
	public:
		ChartDummy();
		virtual ~ChartDummy();

		virtual InitReturn Init(const wxString & name, ChartInitFlag init_flags);
		virtual ThumbData * GetThumbData(int tnx, int tny, float lat, float lon);
		virtual ThumbData * GetThumbData();
		virtual bool UpdateThumbData(double lat, double lon);
		double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);
		double GetNormalScaleMax(double canvas_scale_factor, int canvas_width);
		virtual bool GetChartExtent(Extent * pext);

		virtual bool RenderRegionViewOnDC(
				wxMemoryDC & dc,
				const ViewPort & VPoint,
				const OCPNRegion & Region);

		virtual bool RenderRegionViewOnGL(
				const wxGLContext & glc,
				const ViewPort & VPoint,
				const OCPNRegion & Region);

		virtual bool AdjustVP(ViewPort & vp_last, ViewPort & vp_proposed);
		virtual void GetValidCanvasRegion(const ViewPort & VPoint, OCPNRegion * pValidRegion);
		virtual void SetColorScheme(ColorScheme cs, bool bApplyImmediate);
		virtual double GetNearestPreferredScalePPM(double target_scale_ppm);

	private:
		bool RenderViewOnDC(wxMemoryDC & dc, const ViewPort & VPoint);
		wxBitmap * m_pBM;
};

#endif
