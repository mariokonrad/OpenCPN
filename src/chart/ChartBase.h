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

#ifndef __CHART__CHARTBASE__H__
#define __CHART__CHARTBASE__H__

#include "dychart.h"
#include <ColorScheme.h>
#include <ThumbData.h>
#include <ProjectionType.h>

#include <chart/ChartType.h>
#include <chart/ChartFamily.h>

#include <wx/datetime.h>

class wxGLContext;
class wxMemoryDC;
class ViewPort;
class OCPNRegion;

namespace chart {

enum ChartInitFlag
{
	FULL_INIT = 0,
	HEADER_ONLY,
	THUMB_ONLY
};

enum RenderTypeEnum
{
	DC_RENDER_ONLY = 0,
	DC_RENDER_RETURN_DIB,
	DC_RENDER_RETURN_IMAGE
};

enum InitReturn
{
	INIT_OK = 0,
	INIT_FAIL_RETRY,        // Init failed, retry suggested
	INIT_FAIL_REMOVE,       // Init failed, suggest remove from further use
	INIT_FAIL_NOERROR       // Init failed, request no explicit error message
};

struct Extent
{
	double SLAT;
	double WLON;
	double NLAT;
	double ELON;
};

// Depth unit type enum
enum ChartDepthUnitType
{
	DEPTH_UNIT_UNKNOWN,
	DEPTH_UNIT_FEET,
	DEPTH_UNIT_METERS,
	DEPTH_UNIT_FATHOMS
};

class ChartBase
{
public:
	ChartBase();
	virtual ~ChartBase();

	virtual InitReturn Init(const wxString& name, ChartInitFlag init_flags) = 0;

	virtual void Activate(void);
	virtual void Deactivate(void);

	virtual ThumbData* GetThumbData(int tnx, int tny, float lat, float lon) = 0;
	virtual ThumbData* GetThumbData() = 0;
	virtual bool UpdateThumbData(double lat, double lon) = 0;
	virtual double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom) const = 0;
	virtual double GetNormalScaleMax(double canvas_scale_factor, int canvas_width) const = 0;
	virtual bool GetChartExtent(Extent& ext) const = 0;

	virtual OcpnProjType GetChartProjectionType() const;
	virtual wxDateTime GetEditionDate(void);
	virtual wxString GetPubDate() const;
	virtual int GetNativeScale() const;
	wxString GetFullPath() const;
	wxString GetName() const;
	wxString GetDescription() const;
	wxString GetID() const;
	wxString GetSE() const;
	wxString GetDepthUnits() const;
	wxString GetSoundingsDatum() const;
	wxString GetDatumString() const;
	wxString GetExtraInfo() const;
	double GetChart_Error_Factor() const;
	ChartTypeEnum GetChartType() const;
	chart::ChartFamilyEnum GetChartFamily() const;
	double GetChartSkew() const;
	virtual ChartDepthUnitType GetDepthUnitType(void) const;
	virtual bool IsReadyToRender() const;

	virtual bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
									  const OCPNRegion& Region) = 0;

	virtual bool RenderRegionViewOnGL(const wxGLContext& glc, const ViewPort& VPoint,
									  const OCPNRegion& Region) = 0;

	virtual bool AdjustVP(const ViewPort& vp_last, ViewPort& vp_proposed) = 0;

	virtual void GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion* pValidRegion) = 0;

	virtual void SetColorScheme(ColorScheme cs, bool bApplyImmediate = true) = 0;

	virtual double GetNearestPreferredScalePPM(double target_scale_ppm) = 0;

	virtual int GetCOVREntries() const;
	virtual int GetCOVRTablePoints(int iTable) const;
	virtual int GetCOVRTablenPoints(int iTable) const;
	virtual float* GetCOVRTableHead(int iTable);

	virtual int GetNoCOVREntries() const;
	virtual int GetNoCOVRTablePoints(int iTable) const;
	virtual int GetNoCOVRTablenPoints(int iTable) const;
	virtual float* GetNoCOVRTableHead(int iTable);

protected:
	int m_Chart_Scale;
	ChartTypeEnum m_ChartType;
	chart::ChartFamilyEnum m_ChartFamily;
	wxString m_FullPath;
	wxString m_Name;
	wxString m_Description;
	wxString m_ID;
	wxString m_SE;
	wxString m_SoundingsDatum;
	wxString m_datum_str;
	wxString m_ExtraInfo;
	wxString m_PubYear;
	wxString m_DepthUnits;
	OcpnProjType m_projection;
	ChartDepthUnitType m_depth_unit_id;
	wxDateTime m_EdDate;
	wxBitmap* pcached_bitmap;
	ThumbData* pThumbData;
	ColorScheme m_global_color_scheme;
	bool bReadyToRender;
	double Chart_Error_Factor;
	double m_lon_datum_adjust; // Add these numbers to WGS84 position to obtain internal chart
							   // position
	double m_lat_datum_adjust;
	double m_Chart_Skew;

	// Chart region coverage information
	// Charts may have multiple valid regions within the lat/lon box described by the chart extent
	// The following table structure contains this embedded information

	// Typically, BSB charts will contain only one entry, corresponding to the PLY information in
	// the chart header ENC charts often contain multiple entries

	int m_nCOVREntries; // number of coverage table entries
	int* m_pCOVRTablePoints; // int table of number of points in each coverage table entry
	float** m_pCOVRTable; // table of pointers to list of floats describing valid COVR

	int m_nNoCOVREntries; // number of NoCoverage table entries
	int* m_pNoCOVRTablePoints; // int table of number of points in each NoCoverage table entry
	float** m_pNoCOVRTable; // table of pointers to list of floats describing valid NOCOVR
};

}

#endif
