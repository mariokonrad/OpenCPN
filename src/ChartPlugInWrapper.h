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

#ifndef __CHARTPLUGINWRAPPER__H__
#define __CHARTPLUGINWRAPPER__H__

#include <chart/ChartBase.h>

class PlugInChartBase;
class wxRect;
class wxObject;

class ChartPlugInWrapper : public chart::ChartBase
{
public:
	ChartPlugInWrapper();
	ChartPlugInWrapper(const wxString& chart_class);
	virtual ~ChartPlugInWrapper();

	virtual wxString GetFileSearchMask(void);

	virtual chart::InitReturn Init(const wxString& name, chart::ChartInitFlag init_flags);

	virtual ThumbData* GetThumbData(int tnx, int tny, const geo::Position& pos);
	virtual ThumbData* GetThumbData();
	virtual bool UpdateThumbData(const geo::Position& pos);

	double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom) const;
	double GetNormalScaleMax(double canvas_scale_factor, int canvas_width) const;
	virtual bool GetChartExtent(chart::Extent& ext) const;

	virtual bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
									  const OCPNRegion& Region);

	virtual bool RenderRegionViewOnGL(const wxGLContext& glc, const ViewPort& VPoint,
									  const OCPNRegion& Region);

	virtual bool AdjustVP(const ViewPort& vp_last, ViewPort& vp_proposed);
	virtual void GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion* pValidRegion);
	virtual void SetColorScheme(global::ColorScheme cs, bool bApplyImmediate);
	virtual double GetNearestPreferredScalePPM(double target_scale_ppm);
	virtual PlugInChartBase* GetPlugInChart(void);
	virtual int GetCOVREntries() const;
	virtual int GetCOVRTablePoints(int iTable) const;
	virtual int GetCOVRTablenPoints(int iTable) const;
	virtual float* GetCOVRTableHead(int iTable);
	virtual int GetNoCOVREntries() const;
	virtual int GetNoCOVRTablePoints(int iTable) const;
	virtual int GetNoCOVRTablenPoints(int iTable) const;
	virtual float* GetNoCOVRTableHead(int iTable);

	// The following set of methods apply to BSB (i.e. Raster) type PlugIn charts only
	// and need not be implemented if the ChartFamily is not CHART_FAMILY_RASTER
	virtual void ComputeSourceRectangle(const ViewPort& vp, wxRect* pSourceRect);
	virtual double GetRasterScaleFactor() const;
	virtual bool GetChartBits(wxRect& source, unsigned char* pPix, int sub_samp);
	virtual int GetSize_X() const;
	virtual int GetSize_Y() const;
	virtual void latlong_to_chartpix(const geo::Position& pos, double& pixx, double& pixy);

private:
	PlugInChartBase* m_ppicb;
	wxObject* m_ppo;
};

#endif
