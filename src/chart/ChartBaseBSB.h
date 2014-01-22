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

#ifndef __CHART__CHARTIMG__H__
#define __CHART__CHARTIMG__H__

#include <chart/ChartBase.h>
#include <chart/Refpoint.h>

#include <OCPNRegion.h>
#include <ViewPort.h>

#include <geo/GeoRef.h>

#include <vector>

class wxInputStream;
class wxBufferedInputStream;
class wxFileInputStream;
class wxImage;

class PixelCache;

namespace chart {

enum ScaleTypeEnum
{
	RENDER_LODEF = 0,
	RENDER_HIDEF,
};

enum PaletteDir
{
	PaletteFwd,
	PaletteRev
};

enum BSB_Color_Capability
{
	COLOR_RGB_DEFAULT = 0, // Default corresponds to bsb entries "RGB"
	DAY,
	DUSK,
	NIGHT,
	NIGHTRED,
	GRAY,
	PRC,
	PRG,
	N_BSB_COLORS
};

class CachedLine
{
public:
	int xstart;
	int xlength;
	unsigned char* pPix;
	unsigned char* pRGB;
	bool bValid;
};

class opncpnPalette
{
public:
	opncpnPalette();
	~opncpnPalette();

public:
	int* FwdPalette; // FIXME: use std container, not this realloc clusterfuck
	int* RevPalette; // FIXME: use std container, not this realloc clusterfuck
	int nFwd;
	int nRev;
};

class ChartBaseBSB : public ChartBase
{
public:
	ChartBaseBSB();
	virtual ~ChartBaseBSB();

	virtual ThumbData* GetThumbData(int tnx, int tny, float lat, float lon);
	virtual ThumbData* GetThumbData();
	virtual bool UpdateThumbData(double lat, double lon);

	int GetNativeScale() const;
	double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom) const;
	double GetNormalScaleMax(double canvas_scale_factor, int canvas_width) const;

	virtual InitReturn Init(const wxString& name, ChartInitFlag init_flags);

	virtual int latlong_to_pix_vp(double lat, double lon, int& pixx, int& pixy, const ViewPort& vp);
	virtual int vp_pix_to_latlong(const ViewPort& vp, int pixx, int pixy, double* lat, double* lon);

	bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion& Region);

	virtual bool RenderRegionViewOnGL(const wxGLContext& glc, const ViewPort& VPoint,
									  const OCPNRegion& Region);

	virtual bool AdjustVP(const ViewPort& vp_last, ViewPort& vp_proposed);
	virtual double GetNearestPreferredScalePPM(double target_scale_ppm);

	void GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion* pValidRegion);

	virtual bool GetChartExtent(Extent& ext) const;

	void SetColorScheme(global::ColorScheme cs, bool bApplyImmediate);

	wxImage* GetImage();

	void SetVPRasterParms(const ViewPort& vpt);

	void ComputeSourceRectangle(const ViewPort& vp, wxRect* pSourceRect);
	double GetRasterScaleFactor() const;
	virtual bool GetChartBits(wxRect& source, unsigned char* pPix, int sub_samp);
	int GetSize_X() const;
	int GetSize_Y() const;

	void latlong_to_chartpix(double lat, double lon, double& pixx, double& pixy);
	void chartpix_to_latlong(double pixx, double pixy, double* plat, double* plon);

protected:
	wxRect GetSourceRect() const;

	virtual bool GetAndScaleData(unsigned char* ppn, wxRect& source, int source_stride,
								 wxRect& dest, int dest_stride, double scale_factor,
								 ScaleTypeEnum scale_type);

	bool RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint);

	bool IsCacheValid() const;
	void InvalidateCache();
	bool IsRenderCacheable(wxRect& source, wxRect& dest);

	void CreatePaletteEntry(char* buffer, int palette_index);
	PaletteDir GetPaletteDir(void);
	int* GetPalettePtr(BSB_Color_Capability);

	double GetClosestValidNaturalScalePPM(double target_scale, double scale_factor_min,
										  double scale_factor_max);

	double GetPPM() const;

	virtual void InvalidateLineCache();
	virtual bool CreateLineIndex(void);

	virtual wxBitmap* CreateThumbnail(int tnx, int tny, global::ColorScheme cs);
	virtual int BSBGetScanline(unsigned char* pLineBuf, int y, int xs, int xl, int sub_samp);

	bool GetViewUsingCache(wxRect& source, wxRect& dest, const OCPNRegion& Region,
						   ScaleTypeEnum scale_type);
	bool GetView(wxRect& source, wxRect& dest, ScaleTypeEnum scale_type);

	virtual int BSBScanScanline(wxInputStream* pinStream);
	virtual int ReadBSBHdrLine(wxFileInputStream*, char*, int);
	virtual int AnalyzeRefpoints(void);
	virtual bool AnalyzeSkew(void);

	virtual bool SetMinMax(void);

	InitReturn PreInit(const wxString& name, ChartInitFlag init_flags, global::ColorScheme cs);
	InitReturn PostInit(void);

	PixelCache* pPixCache;

	int Size_X; // Chart native pixel dimensions
	int Size_Y;
	int m_Chart_DU;
	double m_cph;
	double m_proj_parameter; // Mercator:Projection Latitude, Transverse Mercator: Central Meridian
	double m_dx; // Pixel scale factors, from KAP header
	double m_dy;

	wxString m_bsb_ver;
	bool m_b_SHOM;
	bool m_b_apply_dtm;

	int m_datum_index;
	geo::Position m_dtm;

	wxRect cache_rect;
	wxRect cache_rect_scaled;
	bool cached_image_ok;
	ScaleTypeEnum cache_scale_method;
	double m_cached_scale_ppm;
	wxRect m_last_vprect;

	wxRect Rsrc; // Current chart source rectangle
	std::vector<chart::Refpoint> reference_points;
	int nColorSize;
	std::vector<int> line_offset_table;

	CachedLine* pLineCache;

	wxFileInputStream* ifs_hdr;
	wxFileInputStream* ifss_bitmap;
	wxBufferedInputStream* ifs_bitmap;

	unsigned char* ifs_buf;
	unsigned char* ifs_bufend;
	int ifs_bufsize;
	unsigned char* ifs_lp;
	int ifs_file_offset;
	int nFileOffsetDataStart;
	int m_nLineOffset;

	geo::GeoRef cPoints;

	double wpx[12], wpy[12], pwx[12], pwy[12]; // Embedded georef coefficients
	int wpx_type;
	int wpy_type;
	int pwx_type;
	int pwy_type;
	int n_wpx;
	int n_wpy;
	int n_pwx;
	int n_pwy;
	bool bHaveEmbeddedGeoref;

	opncpnPalette* pPalettes[N_BSB_COLORS];

	BSB_Color_Capability m_mapped_color_index;

	// Integer digital scale value above which bilinear scaling is not allowed,
	// and subsampled scaling must be performed
	int m_bilinear_limit;

	bool bUseLineCache;

	float m_LonMax;
	float m_LonMin;
	float m_LatMax;
	float m_LatMin;

	int* pPalette;
	PaletteDir palette_direction;

	bool bGeoErrorSent;

	double m_ppm_avg; // Calculated true scale factor of the 1X chart, pixels per meter

	double m_raster_scale_factor; // exact scaling factor for pixel oversampling calcs

	bool m_bIDLcross;

	OCPNRegion m_last_region;

	int m_b_cdebug;

	geo::Position m_proj;
	ViewPort m_vp_render_last;
};

}

#endif
