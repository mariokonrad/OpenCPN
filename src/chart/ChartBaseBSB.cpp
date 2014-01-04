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

#include "ChartBaseBSB.h"

#include <wx/dir.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/fileconf.h>
#include <wx/log.h>

#include <sys/stat.h>

#include <ocpn_pixel.h>
#include <OCPNRegionIterator.h>
#include <OCPNBitmap.h>
#include <MicrosoftCompatibility.h>
#include <chart/PlyPoint.h>
#include <algorithm>

using std::min;

#ifndef __WXMSW__
	#include <signal.h>
	#include <setjmp.h>

#define OCPN_USE_CONFIG 1

struct sigaction sa_all_chart;
struct sigaction sa_all_previous;

sigjmp_buf env_chart; // the context saved by sigsetjmp();

void catch_signals_chart(int signo)
{
	if (signo == SIGSEGV) {
		siglongjmp(env_chart, 1);
	}
}

#endif


#ifdef OCPN_USE_CONFIG
class MyConfig;
extern MyConfig* pConfig;
#endif

namespace chart {

typedef struct
{
	float y;
	float x;
} MyFlPoint;

bool G_FloatPtInPolygon(MyFlPoint* rgpts, int wnumpts, float x, float y);

// ============================================================================
// Palette implementation
// ============================================================================
opncpnPalette::opncpnPalette()
	: FwdPalette(NULL)
	, RevPalette(NULL)
{
	// Index into palette is 1-based, so predefine the first entry as null
	nFwd = 1;
	nRev = 1;
	FwdPalette = (int*)malloc(sizeof(int)); // FIXME: use std vector
	RevPalette = (int*)malloc(sizeof(int)); // FIXME: use std vector
	FwdPalette[0] = 0;
	RevPalette[0] = 0;
}

opncpnPalette::~opncpnPalette()
{
	if (FwdPalette)
		free(FwdPalette);
	if (RevPalette)
		free(RevPalette);
}

ChartBaseBSB::ChartBaseBSB()
{
	// Init some private data
	m_ChartFamily = chart::CHART_FAMILY_RASTER;

	ifs_buf = NULL;

	cached_image_ok = 0;

	cPoints.status = 0;
	bHaveEmbeddedGeoref = false;
	n_wpx = 0;
	n_wpy = 0;
	n_pwx = 0;
	n_pwy = 0;

	bUseLineCache = true;
	m_Chart_Skew = 0.0;

	pPixCache = NULL;

	pLineCache = NULL;

	m_bilinear_limit = 8; // bilinear scaling only up to n

	ifs_bitmap = NULL;
	ifss_bitmap = NULL;
	ifs_hdr = NULL;

	for (int i = 0; i < N_BSB_COLORS; i++)
		pPalettes[i] = NULL;

	bGeoErrorSent = false;
	m_Chart_DU = 0;
	m_cph = 0.;

	m_mapped_color_index = COLOR_RGB_DEFAULT;

	m_datum_str = _T("WGS84"); // assume until proven otherwise

	m_dtm_lat = 0.0;
	m_dtm_lon = 0.0;

	m_bIDLcross = false;

	m_dx = 0.0;
	m_dy = 0.0;
	m_proj_lat = 0.0;
	m_proj_lon = 0.0;
	m_proj_parameter = 0.0;
	m_b_SHOM = false;
	m_b_apply_dtm = true;

	m_b_cdebug = 0;

#ifdef OCPN_USE_CONFIG
	wxFileConfig* pfc = (wxFileConfig*)pConfig;
	pfc->SetPath(_T("/Settings"));
	pfc->Read(_T("DebugBSBImg"), &m_b_cdebug, 0);
#endif
}

ChartBaseBSB::~ChartBaseBSB()
{
	if (ifs_buf)
		free(ifs_buf);

	delete ifs_bitmap;
	delete ifs_hdr;
	delete ifss_bitmap;

	if (cPoints.status) {
		free(cPoints.tx);
		free(cPoints.ty);
		free(cPoints.lon);
		free(cPoints.lat);

		free(cPoints.pwx);
		free(cPoints.wpx);
		free(cPoints.pwy);
		free(cPoints.wpy);
	}

	// Free the line cache

	if (pLineCache) {
		CachedLine* pt;
		for (int ylc = 0; ylc < Size_Y; ylc++) {
			pt = &pLineCache[ylc];
			if (pt->pPix)
				free(pt->pPix);
			free(pt->pRGB);
		}
		free(pLineCache);
	}

	delete pPixCache;

	for (int i = 0; i < N_BSB_COLORS; i++)
		delete pPalettes[i];
}

// Report recommended minimum and maximum scale values for which use of this chart is valid
double ChartBaseBSB::GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom) const
{
	if (b_allow_overzoom)
		return (canvas_scale_factor / m_ppm_avg) / 32; // allow wide range overzoom overscale
	else
		return (canvas_scale_factor / m_ppm_avg) / 2; // don't suggest too much overscale
}

double ChartBaseBSB::GetNormalScaleMax(double canvas_scale_factor, int WXUNUSED(canvas_width)) const
{
	return (canvas_scale_factor / m_ppm_avg) * 4.0; // excessive underscale is slow, and unreadable
}

double ChartBaseBSB::GetNearestPreferredScalePPM(double target_scale_ppm)
{
	// changed from 32 to 64 to allow super small
	// scale BSB charts as quilt base
	return GetClosestValidNaturalScalePPM(target_scale_ppm, 0.01, 64.0);
}

double ChartBaseBSB::GetClosestValidNaturalScalePPM(double target_scale, double scale_factor_min,
													double scale_factor_max)
{
	double chart_1x_scale = GetPPM();

	double binary_scale_factor = 1.0;

	// Overzoom....
	if (chart_1x_scale > target_scale) {
		double binary_scale_factor_max = 1 / scale_factor_min;

		while (binary_scale_factor < binary_scale_factor_max) {
			if (fabs((chart_1x_scale / binary_scale_factor) - target_scale) < (target_scale * 0.05))
				break;
			if ((chart_1x_scale / binary_scale_factor) < target_scale)
				break;
			else
				binary_scale_factor *= 2.;
		}
	} else {
		// Underzoom.....
		int ibsf = 1;
		int isf_max = (int)scale_factor_max;
		while (ibsf < isf_max) {
			if (fabs((chart_1x_scale * ibsf) - target_scale) < (target_scale * 0.05))
				break;

			else if ((chart_1x_scale * ibsf) > target_scale) {
				if (ibsf > 1)
					ibsf /= 2;
				break;
			} else
				ibsf *= 2;
		}

		binary_scale_factor = 1.0 / ibsf;
	}

	return chart_1x_scale / binary_scale_factor;
}

ThumbData* ChartBaseBSB::GetThumbData()
{
	return pThumbData;
}

int ChartBaseBSB::GetNativeScale() const
{
	return m_Chart_Scale;
}

double ChartBaseBSB::GetRasterScaleFactor() const
{
	return m_raster_scale_factor;
}

wxRect ChartBaseBSB::GetSourceRect() const
{
	return Rsrc;
}

bool ChartBaseBSB::IsCacheValid() const
{
	return cached_image_ok;
}

void ChartBaseBSB::InvalidateCache()
{
	cached_image_ok = 0;
}

double ChartBaseBSB::GetPPM() const
{
	return m_ppm_avg;
}

InitReturn ChartBaseBSB::Init(const wxString& WXUNUSED(name), ChartInitFlag WXUNUSED(init_flags))
{
	m_global_color_scheme = GLOBAL_COLOR_SCHEME_RGB;
	return INIT_OK;
}

InitReturn ChartBaseBSB::PreInit(const wxString& WXUNUSED(name), ChartInitFlag WXUNUSED(init_flags),
								 ColorScheme cs)
{
	m_global_color_scheme = cs;
	return INIT_OK;
}

void ChartBaseBSB::CreatePaletteEntry(char* buffer, int palette_index)
{
	if (palette_index < N_BSB_COLORS) {
		if (!pPalettes[palette_index])
			pPalettes[palette_index] = new opncpnPalette;
		opncpnPalette* pp = pPalettes[palette_index];

		pp->FwdPalette = (int*)realloc(pp->FwdPalette, (pp->nFwd + 1) * sizeof(int));
		pp->RevPalette = (int*)realloc(pp->RevPalette, (pp->nRev + 1) * sizeof(int));
		pp->nFwd++;
		pp->nRev++;

		int i;
		int n, r, g, b;
		sscanf(&buffer[4], "%d,%d,%d,%d", &n, &r, &g, &b);

		i = n;

		int fcolor, rcolor;
		fcolor = (b << 16) + (g << 8) + r;
		rcolor = (r << 16) + (g << 8) + b;

		pp->RevPalette[i] = rcolor;
		pp->FwdPalette[i] = fcolor;
	}
}

int ChartBaseBSB::GetSize_X() const
{
	return Size_X;
}

int ChartBaseBSB::GetSize_Y() const
{
	return Size_Y;
}

InitReturn ChartBaseBSB::PostInit(void)
{
	// Validate the palette array, substituting DEFAULT for missing entries
	for (int i = 0; i < N_BSB_COLORS; i++) {
		if (pPalettes[i] == NULL) {
			opncpnPalette* pNullSubPal = new opncpnPalette;

			pNullSubPal->nFwd = pPalettes[COLOR_RGB_DEFAULT]->nFwd; // copy the palette count
			pNullSubPal->nRev = pPalettes[COLOR_RGB_DEFAULT]->nRev; // copy the palette count
			//  Deep copy the palette rgb tables
			free(pNullSubPal->FwdPalette);
			pNullSubPal->FwdPalette = (int*)malloc(pNullSubPal->nFwd * sizeof(int));
			memcpy(pNullSubPal->FwdPalette, pPalettes[COLOR_RGB_DEFAULT]->FwdPalette,
				   pNullSubPal->nFwd * sizeof(int));

			free(pNullSubPal->RevPalette);
			pNullSubPal->RevPalette = (int*)malloc(pNullSubPal->nRev * sizeof(int));
			memcpy(pNullSubPal->RevPalette, pPalettes[COLOR_RGB_DEFAULT]->RevPalette,
				   pNullSubPal->nRev * sizeof(int));

			pPalettes[i] = pNullSubPal;
		}
	}

	// Establish the palette type and default palette
	palette_direction = GetPaletteDir();

	SetColorScheme(m_global_color_scheme, false);

	// Allocate memory for ifs file buffering
	ifs_bufsize = Size_X * 4;
	ifs_buf = (unsigned char*)malloc(ifs_bufsize);
	if (!ifs_buf)
		return INIT_FAIL_REMOVE;

	ifs_bufend = ifs_buf + ifs_bufsize;
	ifs_lp = ifs_bufend;
	ifs_file_offset = -ifs_bufsize;

	// Create and load the line offset index table
	line_offset_table.clear();
	line_offset_table.resize(Size_Y + 1);

	ifs_bitmap->SeekI((Size_Y + 1) * -4, wxFromEnd); // go to Beginning of offset table
	line_offset_table[Size_Y] = ifs_bitmap->TellI(); // fill in useful last table entry

	int offset;
	for (int ifplt = 0; ifplt < Size_Y; ifplt++) {
		offset = 0;
		offset += (unsigned char)ifs_bitmap->GetC() * 256 * 256 * 256;
		offset += (unsigned char)ifs_bitmap->GetC() * 256 * 256;
		offset += (unsigned char)ifs_bitmap->GetC() * 256;
		offset += (unsigned char)ifs_bitmap->GetC();

		line_offset_table[ifplt] = offset;
	}

	// Try to validate the line index

	bool bline_index_ok = true;
	m_nLineOffset = 0;

	for (int iplt = 0; iplt < Size_Y - 1; iplt++) {
		if (wxInvalidOffset == ifs_bitmap->SeekI(line_offset_table[iplt], wxFromStart)) {
			wxString msg(_("   Chart File corrupt in PostInit() on chart "));
			msg.Append(m_FullPath);
			wxLogMessage(msg);

			return INIT_FAIL_REMOVE;
		}

		int thisline_size = line_offset_table[iplt + 1] - line_offset_table[iplt];

		if (thisline_size < 0) {
			wxString msg(_("   Chart File corrupt in PostInit() on chart "));
			msg.Append(m_FullPath);
			wxLogMessage(msg);

			return INIT_FAIL_REMOVE;
		}

		if (thisline_size > ifs_bufsize) {
			wxString msg(_T("   ifs_bufsize too small PostInit() on chart "));
			msg.Append(m_FullPath);
			wxLogMessage(msg);

			return INIT_FAIL_REMOVE;
		}

		ifs_bitmap->Read(ifs_buf, thisline_size);

		unsigned char* lp = ifs_buf;

		unsigned char byNext;
		int nLineMarker = 0;
		do {
			byNext = *lp++;
			nLineMarker = nLineMarker * 128 + (byNext & 0x7f);
		} while ((byNext & 0x80) != 0);

		// Linemarker Correction factor needed here
		// Some charts start with LineMarker = 0, some with LineMarker = 1
		// Assume the first LineMarker found is the index base, and use
		// as a correction offset

		if (iplt == 0)
			m_nLineOffset = nLineMarker;

		if (nLineMarker != iplt + m_nLineOffset) {
			bline_index_ok = false;
			break;
		}
	}
	// Recreate the scan line index if the embedded version seems corrupt
	if (!bline_index_ok) {
		wxString msg(_("   Line Index corrupt, recreating Index for chart "));
		msg.Append(m_FullPath);
		wxLogMessage(msg);
		if (!CreateLineIndex()) {
			wxString msg(_("   Error creating Line Index for chart "));
			msg.Append(m_FullPath);
			wxLogMessage(msg);
			return INIT_FAIL_REMOVE;
		}
	}

	//    Allocate the Line Cache
	if (bUseLineCache) {
		pLineCache = (CachedLine*)malloc(Size_Y * sizeof(CachedLine)); // FIXME: use std containers
		CachedLine* pt;

		for (int ylc = 0; ylc < Size_Y; ylc++) {
			pt = &pLineCache[ylc];
			pt->bValid = false;
			pt->xstart = 0;
			pt->xlength = 1;
			pt->pPix = NULL;
			pt->pRGB = NULL;
		}
	} else
		pLineCache = NULL;

	// Validate/Set Depth Unit Type
	wxString test_str = m_DepthUnits.Upper();
	if (test_str.IsSameAs(_T("FEET"), FALSE))
		m_depth_unit_id = DEPTH_UNIT_FEET;
	else if (test_str.IsSameAs(_T("METERS"), FALSE))
		m_depth_unit_id = DEPTH_UNIT_METERS;
	else if (test_str.IsSameAs(_T("METRES"), FALSE)) // Special case for alternate spelling
		m_depth_unit_id = DEPTH_UNIT_METERS;
	else if (test_str.IsSameAs(_T("FATHOMS"), FALSE))
		m_depth_unit_id = DEPTH_UNIT_FATHOMS;
	else if (test_str.Find(_T("FATHOMS")) != wxNOT_FOUND) // Special case for "Fathoms and Feet"
		m_depth_unit_id = DEPTH_UNIT_FATHOMS;
	else if (test_str.Find(_T("METERS")) != wxNOT_FOUND) // Special case for "Meters and decimeters"
		m_depth_unit_id = DEPTH_UNIT_METERS;

	// Setup the datum transform parameters
	char d_str[100];
	strncpy(d_str, m_datum_str.mb_str(), 99);
	d_str[99] = 0;

	m_datum_index = geo::GetDatumIndex(d_str);

	// Analyze Refpoints
	int analyze_ret_val = AnalyzeRefpoints();
	if (0 != analyze_ret_val)
		return INIT_FAIL_REMOVE;

	// Establish defaults, may be overridden later
	m_lon_datum_adjust = (-m_dtm_lon) / 3600.0;
	m_lat_datum_adjust = (-m_dtm_lat) / 3600.0;

	bReadyToRender = true;
	return INIT_OK;
}

bool ChartBaseBSB::CreateLineIndex()
{
	// Assumes file stream ifs_bitmap is currently open

	// Seek to start of data
	ifs_bitmap->SeekI(nFileOffsetDataStart); // go to Beginning of data

	for (int iplt = 0; iplt < Size_Y; iplt++) {
		int offset = ifs_bitmap->TellI();

		BSBScanScanline(ifs_bitmap);

		//  There is no sense reporting an error here, since we are recreating after an error
		line_offset_table[iplt] = offset;
	}

	return true;
}

// Invalidate and Free the line cache contents
void ChartBaseBSB::InvalidateLineCache(void)
{
	if (pLineCache) {
		CachedLine* pt;
		for (int ylc = 0; ylc < Size_Y; ylc++) {
			pt = &pLineCache[ylc];
			if (pt) {
				if (pt->pPix) {
					free(pt->pPix);
					pt->pPix = NULL;
				}
				pt->bValid = 0;
			}
		}
	}
}

bool ChartBaseBSB::GetChartExtent(Extent& ext) const
{
	ext.NLAT = m_LatMax;
	ext.SLAT = m_LatMin;
	ext.ELON = m_LonMax;
	ext.WLON = m_LonMin;

	return true;
}

bool ChartBaseBSB::SetMinMax(void)
{
	// Calculate the Chart Extents(M_LatMin, M_LonMin, etc.)
	// from the COVR data, for fast database search
	m_LonMax = -360.0;
	m_LonMin = 360.0;
	m_LatMax = -90.0;
	m_LatMin = 90.0;

	Plypoint* ppp = reinterpret_cast<Plypoint*>(GetCOVRTableHead(0)); // FIXME
	int cnPlypoint = GetCOVRTablenPoints(0);

	for (int u = 0; u < cnPlypoint; u++) {
		if (ppp->lnp > m_LonMax)
			m_LonMax = ppp->lnp;
		if (ppp->lnp < m_LonMin)
			m_LonMin = ppp->lnp;

		if (ppp->ltp > m_LatMax)
			m_LatMax = ppp->ltp;
		if (ppp->ltp < m_LatMin)
			m_LatMin = ppp->ltp;

		ppp++;
	}

	// Check for special cases

	// Case 1:  Chart spans International Date Line or Greenwich, Longitude min/max is non-obvious.
	if ((m_LonMax * m_LonMin) < 0) {
		// min/max are opposite signs
		// Georeferencing is not yet available, so find the reference points closest to min/max ply
		// points

		if (reference_points.empty())
			return false; // have to bail here

		// for m_LonMax
		double min_dist_x = 360;
		int imaxclose = 0;
		for (int ic = 0; ic < static_cast<int>(reference_points.size()); ic++) {
			double dist = sqrt(
				((m_LatMax - reference_points[ic].latr) * (m_LatMax - reference_points[ic].latr))
				+ ((m_LonMax - reference_points[ic].lonr)
				   * (m_LonMax - reference_points[ic].lonr)));

			if (dist < min_dist_x) {
				min_dist_x = dist;
				imaxclose = ic;
			}
		}

		// for m_LonMin
		double min_dist_n = 360;
		int iminclose = 0;
		for (int id = 0; id < static_cast<int>(reference_points.size()); id++) {
			double dist = sqrt(
				((m_LatMin - reference_points[id].latr) * (m_LatMin - reference_points[id].latr))
				+ ((m_LonMin - reference_points[id].lonr)
				   * (m_LonMin - reference_points[id].lonr)));

			if (dist < min_dist_n) {
				min_dist_n = dist;
				iminclose = id;
			}
		}

		// Is this chart crossing IDL or Greenwich?
		// Make the check
		if (reference_points[imaxclose].xr < reference_points[iminclose].xr) {
			// This chart crosses IDL and needs a flip, meaning that all negative longitudes need
			// to be normalized
			// and the min/max relcalculated
			// This code added to correct non-rectangular charts crossing IDL, such as
			// nz14605.kap

			m_LonMax = -360.0;
			m_LonMin = 360.0;
			m_LatMax = -90.0;
			m_LatMin = 90.0;

			Plypoint* ppp = reinterpret_cast<Plypoint*>(GetCOVRTableHead(0)); // Normalize the plypoints, FIXME
			int cnPlypoint = GetCOVRTablenPoints(0);

			for (int u = 0; u < cnPlypoint; u++) {
				if (ppp->lnp < 0.0)
					ppp->lnp += 360.0;

				if (ppp->lnp > m_LonMax)
					m_LonMax = ppp->lnp;
				if (ppp->lnp < m_LonMin)
					m_LonMin = ppp->lnp;

				if (ppp->ltp > m_LatMax)
					m_LatMax = ppp->ltp;
				if (ppp->ltp < m_LatMin)
					m_LatMin = ppp->ltp;

				ppp++;
			}
		}
	}

	// Case 2 Lons are both < -180, which means the extent will be reported incorrectly
	// and the plypoint structure will be wrong
	// This case is seen first on 81004_1.KAP, (Mariannas)

	if ((m_LonMax < -180.0) && (m_LonMin < -180.0)) {
		m_LonMin += 360.0; // Normalize the extents
		m_LonMax += 360.0;

		Plypoint* ppp = reinterpret_cast<Plypoint*>(GetCOVRTableHead(0)); // Normalize the plypoints, FIXME
		int cnPlypoint = GetCOVRTablenPoints(0);

		for (int u = 0; u < cnPlypoint; u++) {
			ppp->lnp += 360.0;
			ppp++;
		}
	}

	return true;
}

void ChartBaseBSB::SetColorScheme(ColorScheme cs, bool bApplyImmediate)
{
	// Here we convert (subjectively) the Global ColorScheme
	// to an appropriate BSB_Color_Capability index.

	switch (cs) {
		case GLOBAL_COLOR_SCHEME_RGB:
			m_mapped_color_index = COLOR_RGB_DEFAULT;
			break;
		case GLOBAL_COLOR_SCHEME_DAY:
			m_mapped_color_index = DAY;
			break;
		case GLOBAL_COLOR_SCHEME_DUSK:
			m_mapped_color_index = DUSK;
			break;
		case GLOBAL_COLOR_SCHEME_NIGHT:
			m_mapped_color_index = NIGHT;
			break;
		default:
			m_mapped_color_index = DAY;
			break;
	}

	pPalette = GetPalettePtr(m_mapped_color_index);

	m_global_color_scheme = cs;

	// Force a cache dump in a simple sideways manner
	if (bApplyImmediate) {
		m_cached_scale_ppm = 1.0;
	}

	// Force a new thumbnail
	if (pThumbData)
		pThumbData->pDIBThumb = NULL;
}

wxBitmap* ChartBaseBSB::CreateThumbnail(int tnx, int tny, ColorScheme cs)
{
	// Calculate the size and divisors

	int divx = wxMax(1, Size_X / (4 * tnx));
	int divy = wxMax(1, Size_Y / (4 * tny));

	int div_factor = min(divx, divy);

	int des_width = Size_X / div_factor;
	int des_height = Size_Y / div_factor;

	wxRect gts;
	gts.x = 0; // full chart
	gts.y = 0;
	gts.width = Size_X;
	gts.height = Size_Y;

	int this_bpp = 24; // for wxImage
	// Allocate the pixel storage needed for one line of chart bits
	unsigned char* pLineT = (unsigned char*)malloc((Size_X + 1) * BPP / 8);

	// Scale the data quickly
	unsigned char* pPixTN = (unsigned char*)malloc(des_width * des_height * this_bpp / 8);

	int ix = 0;
	int iy = 0;
	int iyd = 0;
	int ixd = 0;
	int yoffd;
	unsigned char* pxs;
	unsigned char* pxd;

	// Temporarily set the color scheme
	ColorScheme cs_tmp = m_global_color_scheme;
	SetColorScheme(cs, false);

	while (iyd < des_height) {
		if (0 == BSBGetScanline(pLineT, iy, 0, Size_X, 1)) {
			// get a line
			free(pLineT);
			free(pPixTN);
			return NULL;
		}

		yoffd = iyd * des_width * this_bpp / 8; // destination y

		ix = 0;
		ixd = 0;
		while (ixd < des_width) {
			pxs = pLineT + (ix * BPP / 8);
			pxd = pPixTN + (yoffd + (ixd * this_bpp / 8));
			*pxd++ = *pxs++;
			*pxd++ = *pxs++;
			*pxd = *pxs;

			ix += div_factor;
			ixd++;
		}

		iy += div_factor;
		iyd++;
	}

	free(pLineT);

	// Reset ColorScheme
	SetColorScheme(cs_tmp, false);

	wxBitmap* retBMP;

#ifdef ocpnUSE_ocpnBitmap
	wxBitmap* bmx2 = new OCPNBitmap(pPixTN, des_width, des_height, -1);
	wxImage imgx2 = bmx2->ConvertToImage();
	imgx2.Rescale(des_width / 4, des_height / 4, wxIMAGE_QUALITY_HIGH);
	retBMP = new wxBitmap(imgx2);
	delete bmx2;
#else
	wxImage thumb_image(des_width, des_height, pPixTN, true);
	thumb_image.Rescale(des_width / 4, des_height / 4, wxIMAGE_QUALITY_HIGH);
	retBMP = new wxBitmap(thumb_image);
#endif

	free(pPixTN);

	return retBMP;
}

//-------------------------------------------------------------------------------------------------
//          Get the Chart thumbnail data structure
//          Creating the thumbnail bitmap as required
//-------------------------------------------------------------------------------------------------

ThumbData* ChartBaseBSB::GetThumbData(int tnx, int tny, float lat, float lon)
{
	// Create the bitmap if needed
	if (!pThumbData->pDIBThumb)
		pThumbData->pDIBThumb = CreateThumbnail(tnx, tny, m_global_color_scheme);

	pThumbData->Thumb_Size_X = tnx;
	pThumbData->Thumb_Size_Y = tny;

	// Plot the supplied Lat/Lon on the thumbnail
	int divx = Size_X / tnx;
	int divy = Size_Y / tny;

	int div_factor = min(divx, divy);

	int pixx, pixy;

	// Using a temporary synthetic ViewPort and source rectangle,
	// calculate the ships position on the thumbnail
	ViewPort tvp;
	tvp.pix_width = tnx;
	tvp.pix_height = tny;
	tvp.view_scale_ppm = GetPPM() / div_factor;
	wxRect trex = Rsrc;
	Rsrc.x = 0;
	Rsrc.y = 0;
	latlong_to_pix_vp(lat, lon, pixx, pixy, tvp);
	Rsrc = trex;

	pThumbData->ShipX = pixx;
	pThumbData->ShipY = pixy;

	return pThumbData;
}

bool ChartBaseBSB::UpdateThumbData(double lat, double lon)
{
	// Plot the supplied Lat/Lon on the thumbnail
	// Return TRUE if the pixel location of ownship has changed

	int divx = Size_X / pThumbData->Thumb_Size_X;
	int divy = Size_Y / pThumbData->Thumb_Size_Y;

	int div_factor = min(divx, divy);

	int pixx_test, pixy_test;

	// Using a temporary synthetic ViewPort and source rectangle,
	// calculate the ships position on the thumbnail
	ViewPort tvp;
	tvp.pix_width = pThumbData->Thumb_Size_X;
	tvp.pix_height = pThumbData->Thumb_Size_Y;
	tvp.view_scale_ppm = GetPPM() / div_factor;
	wxRect trex = Rsrc;
	Rsrc.x = 0;
	Rsrc.y = 0;
	latlong_to_pix_vp(lat, lon, pixx_test, pixy_test, tvp);
	Rsrc = trex;

	if ((pixx_test != pThumbData->ShipX) || (pixy_test != pThumbData->ShipY)) {
		pThumbData->ShipX = pixx_test;
		pThumbData->ShipY = pixy_test;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------
//          Pixel to Lat/Long Conversion helpers
//-----------------------------------------------------------------------
static double polytrans(double* coeff, double lon, double lat);

int ChartBaseBSB::vp_pix_to_latlong(const ViewPort& vp, int pixx, int pixy, double* plat,
									double* plon) // FIXME: refactoring
{
	if (bHaveEmbeddedGeoref) {
		double raster_scale = GetPPM() / vp.view_scale_ppm;

		int px = (int)(pixx * raster_scale) + Rsrc.x;
		int py = (int)(pixy * raster_scale) + Rsrc.y;

		if (1) {
			double lon = polytrans(pwx, px, py);
			lon = (lon < 0) ? lon + m_cph : lon - m_cph;
			*plon = lon - m_lon_datum_adjust;
			*plat = polytrans(pwy, px, py) - m_lat_datum_adjust;
		}

		return 0;
	} else {
		double slat, slon;
		double xp, yp;

		if (m_projection == PROJECTION_TRANSVERSE_MERCATOR) {
			// Use Projected Polynomial algorithm

			double raster_scale = GetPPM() / vp.view_scale_ppm;

			// Apply poly solution to vp center point
			double easting, northing;
			geo::toTM(vp.latitude() + m_lat_datum_adjust, vp.longitude() + m_lon_datum_adjust,
					  m_proj_lat, m_proj_lon, &easting, &northing);
			double xc = polytrans(cPoints.wpx, easting, northing);
			double yc = polytrans(cPoints.wpy, easting, northing);

			// convert screen pixels to chart pixmap relative
			double px = xc + (pixx - (vp.pix_width / 2)) * raster_scale;
			double py = yc + (pixy - (vp.pix_height / 2)) * raster_scale;

			// Apply polynomial solution to chart relative pixels to get e/n
			double east = polytrans(cPoints.pwx, px, py);
			double north = polytrans(cPoints.pwy, px, py);

			// Apply inverse Projection to get lat/lon
			double lat, lon;
			geo::fromTM(east, north, m_proj_lat, m_proj_lon, &lat, &lon);

			// Datum adjustments.....
			double slon_p = lon - m_lon_datum_adjust;
			double slat_p = lat - m_lat_datum_adjust;

			slon = slon_p;
			slat = slat_p;

		} else if (m_projection == PROJECTION_MERCATOR) {
			// Use Projected Polynomial algorithm

			double raster_scale = GetPPM() / vp.view_scale_ppm;

			// Apply poly solution to vp center point
			double easting, northing;
			geo::toSM_ECC(vp.latitude() + m_lat_datum_adjust, vp.longitude() + m_lon_datum_adjust,
						  m_proj_lat, m_proj_lon, &easting, &northing);
			double xc = polytrans(cPoints.wpx, easting, northing);
			double yc = polytrans(cPoints.wpy, easting, northing);

			// convert screen pixels to chart pixmap relative
			double px = xc + (pixx - (vp.pix_width / 2)) * raster_scale;
			double py = yc + (pixy - (vp.pix_height / 2)) * raster_scale;

			// Apply polynomial solution to chart relative pixels to get e/n
			double east = polytrans(cPoints.pwx, px, py);
			double north = polytrans(cPoints.pwy, px, py);

			// Apply inverse Projection to get lat/lon
			double lat, lon;
			geo::fromSM_ECC(east, north, m_proj_lat, m_proj_lon, &lat, &lon);

			// Make Datum adjustments.....
			double slon_p = lon - m_lon_datum_adjust;
			double slat_p = lat - m_lat_datum_adjust;

			slon = slon_p;
			slat = slat_p;
		} else if (m_projection == PROJECTION_POLYCONIC) {
			// Use Projected Polynomial algorithm

			double raster_scale = GetPPM() / vp.view_scale_ppm;

			// Apply poly solution to vp center point
			double easting, northing;
			geo::toPOLY(vp.latitude() + m_lat_datum_adjust, vp.longitude() + m_lon_datum_adjust,
						m_proj_lat, m_proj_lon, &easting, &northing);
			double xc = polytrans(cPoints.wpx, easting, northing);
			double yc = polytrans(cPoints.wpy, easting, northing);

			// convert screen pixels to chart pixmap relative
			double px = xc + (pixx - (vp.pix_width / 2)) * raster_scale;
			double py = yc + (pixy - (vp.pix_height / 2)) * raster_scale;

			// Apply polynomial solution to chart relative pixels to get e/n
			double east = polytrans(cPoints.pwx, px, py);
			double north = polytrans(cPoints.pwy, px, py);

			// Apply inverse Projection to get lat/lon
			double lat, lon;
			geo::fromPOLY(east, north, m_proj_lat, m_proj_lon, &lat, &lon);

			// Make Datum adjustments.....
			double slon_p = lon - m_lon_datum_adjust;
			double slat_p = lat - m_lat_datum_adjust;

			slon = slon_p;
			slat = slat_p;

		} else {
			// Use a Mercator estimator, with Eccentricity corrrection applied
			int dx = pixx - (vp.pix_width / 2);
			int dy = (vp.pix_height / 2) - pixy;

			xp = (dx * cos(vp.skew)) - (dy * sin(vp.skew));
			yp = (dy * cos(vp.skew)) + (dx * sin(vp.skew));

			double d_east = xp / vp.view_scale_ppm;
			double d_north = yp / vp.view_scale_ppm;

			geo::fromSM_ECC(d_east, d_north, vp.latitude(), vp.longitude(), &slat, &slon);
		}

		*plat = slat;

		if (slon < -180.0)
			slon += 360.0;
		else if (slon > 180.0)
			slon -= 360.0;
		*plon = slon;

		return 0;
	}
}

int ChartBaseBSB::latlong_to_pix_vp(double lat, double lon, int& pixx, int& pixy,
									const ViewPort& vp) // FIXME: refactoring
{
	int px = 0;
	int py = 0;

	if (bHaveEmbeddedGeoref) {
		double alon = lon + m_lon_datum_adjust;
		double alat = lat + m_lat_datum_adjust;

		if (m_bIDLcross) {
			if (alon < 0.0)
				alon += 360.0;
		}

		if (1) {
			// change longitude phase (CPH)
			double lonp = (alon < 0) ? alon + m_cph : alon - m_cph;
			double xd = polytrans(wpx, lonp, alat);
			double yd = polytrans(wpy, lonp, alat);
			px = (int)(xd + 0.5);
			py = (int)(yd + 0.5);

			double raster_scale = GetPPM() / vp.view_scale_ppm;

			pixx = (int)(((px - Rsrc.x) / raster_scale) + 0.5);
			pixy = (int)(((py - Rsrc.y) / raster_scale) + 0.5);

			return 0;
		}
	} else {
		double easting;
		double northing;
		double xlon = lon;

		if (m_projection == PROJECTION_TRANSVERSE_MERCATOR) {
			// Use Projected Polynomial algorithm

			double alon = lon + m_lon_datum_adjust;
			double alat = lat + m_lat_datum_adjust;

			// Get e/n from TM Projection
			geo::toTM(alat, alon, m_proj_lat, m_proj_lon, &easting, &northing);

			// Apply poly solution to target point
			double xd = polytrans(cPoints.wpx, easting, northing);
			double yd = polytrans(cPoints.wpy, easting, northing);

			// Apply poly solution to vp center point
			geo::toTM(vp.latitude() + m_lat_datum_adjust, vp.longitude() + m_lon_datum_adjust,
					  m_proj_lat, m_proj_lon, &easting, &northing);
			double xc = polytrans(cPoints.wpx, easting, northing);
			double yc = polytrans(cPoints.wpy, easting, northing);

			// Calculate target point relative to vp center
			double raster_scale = GetPPM() / vp.view_scale_ppm;

			int xs = (int)xc - (int)(vp.pix_width * raster_scale / 2);
			int ys = (int)yc - (int)(vp.pix_height * raster_scale / 2);

			int pixx_p = (int)(((xd - xs) / raster_scale) + 0.5);
			int pixy_p = (int)(((yd - ys) / raster_scale) + 0.5);

			pixx = pixx_p;
			pixy = pixy_p;

		} else if (m_projection == PROJECTION_MERCATOR) {
			// Use Projected Polynomial algorithm

			double alon = lon + m_lon_datum_adjust;
			double alat = lat + m_lat_datum_adjust;

			// Get e/n from  Projection
			xlon = alon;
			if (m_bIDLcross) {
				if (xlon < 0.0)
					xlon += 360.0;
			}
			geo::toSM_ECC(alat, xlon, m_proj_lat, m_proj_lon, &easting, &northing);

			// Apply poly solution to target point
			double xd = polytrans(cPoints.wpx, easting, northing);
			double yd = polytrans(cPoints.wpy, easting, northing);

			// Apply poly solution to vp center point
			double xlonc = vp.longitude();
			if (m_bIDLcross) {
				if (xlonc < 0.0)
					xlonc += 360.0;
			}

			geo::toSM_ECC(vp.latitude() + m_lat_datum_adjust, xlonc + m_lon_datum_adjust,
						  m_proj_lat, m_proj_lon, &easting, &northing);
			double xc = polytrans(cPoints.wpx, easting, northing);
			double yc = polytrans(cPoints.wpy, easting, northing);

			// Calculate target point relative to vp center
			double raster_scale = GetPPM() / vp.view_scale_ppm;

			int xs = (int)xc - (int)(vp.pix_width * raster_scale / 2);
			int ys = (int)yc - (int)(vp.pix_height * raster_scale / 2);

			int pixx_p = (int)(((xd - xs) / raster_scale) + 0.5);
			int pixy_p = (int)(((yd - ys) / raster_scale) + 0.5);

			pixx = pixx_p;
			pixy = pixy_p;

		} else if (m_projection == PROJECTION_POLYCONIC) {
			// Use Projected Polynomial algorithm

			double alon = lon + m_lon_datum_adjust;
			double alat = lat + m_lat_datum_adjust;

			// Get e/n from  Projection
			xlon = alon;
			if (m_bIDLcross) {
				if (xlon < 0.0)
					xlon += 360.0;
			}
			geo::toPOLY(alat, xlon, m_proj_lat, m_proj_lon, &easting, &northing);

			// Apply poly solution to target point
			double xd = polytrans(cPoints.wpx, easting, northing);
			double yd = polytrans(cPoints.wpy, easting, northing);

			// Apply poly solution to vp center point
			double xlonc = vp.longitude();
			if (m_bIDLcross) {
				if (xlonc < 0.0)
					xlonc += 360.0;
			}

			geo::toPOLY(vp.latitude() + m_lat_datum_adjust, xlonc + m_lon_datum_adjust, m_proj_lat,
						m_proj_lon, &easting, &northing);
			double xc = polytrans(cPoints.wpx, easting, northing);
			double yc = polytrans(cPoints.wpy, easting, northing);

			// Calculate target point relative to vp center
			double raster_scale = GetPPM() / vp.view_scale_ppm;

			int xs = (int)xc - (int)(vp.pix_width * raster_scale / 2);
			int ys = (int)yc - (int)(vp.pix_height * raster_scale / 2);

			int pixx_p = (int)(((xd - xs) / raster_scale) + 0.5);
			int pixy_p = (int)(((yd - ys) / raster_scale) + 0.5);

			pixx = pixx_p;
			pixy = pixy_p;

		} else {
			geo::toSM_ECC(lat, xlon, vp.latitude(), vp.longitude(), &easting, &northing);

			double epix = easting * vp.view_scale_ppm;
			double npix = northing * vp.view_scale_ppm;

			double dx = epix * cos(vp.skew) + npix * sin(vp.skew);
			double dy = npix * cos(vp.skew) - epix * sin(vp.skew);

			pixx = (int)((vp.pix_width / 2) + dx);
			pixy = (int)((vp.pix_height / 2) - dy);
		}
		return 0;
	}

	return 1;
}

void ChartBaseBSB::latlong_to_chartpix(double lat, double lon, double& pixx, double& pixy) // FIXME: refactoring
{
	if (bHaveEmbeddedGeoref) {
		double alon = lon + m_lon_datum_adjust;
		double alat = lat + m_lat_datum_adjust;

		if (m_bIDLcross) {
			if (alon < 0.0)
				alon += 360.0;
		}

		// change longitude phase (CPH)
		double lonp = (alon < 0) ? alon + m_cph : alon - m_cph;
		pixx = polytrans(wpx, lonp, alat);
		pixy = polytrans(wpy, lonp, alat);
	} else {
		double easting;
		double northing;
		double xlon = lon;

		if (m_projection == PROJECTION_TRANSVERSE_MERCATOR) {
			// Use Projected Polynomial algorithm

			double alon = lon + m_lon_datum_adjust;
			double alat = lat + m_lat_datum_adjust;

			// Get e/n from TM Projection
			geo::toTM(alat, alon, m_proj_lat, m_proj_lon, &easting, &northing);

			// Apply poly solution to target point
			pixx = polytrans(cPoints.wpx, easting, northing);
			pixy = polytrans(cPoints.wpy, easting, northing);

		} else if (m_projection == PROJECTION_MERCATOR) {
			// Use Projected Polynomial algorithm

			double alon = lon + m_lon_datum_adjust;
			double alat = lat + m_lat_datum_adjust;

			// Get e/n from  Projection
			xlon = alon;
			if (m_bIDLcross) {
				if (xlon < 0.0)
					xlon += 360.0;
			}
			geo::toSM_ECC(alat, xlon, m_proj_lat, m_proj_lon, &easting, &northing);

			// Apply poly solution to target point
			pixx = polytrans(cPoints.wpx, easting, northing);
			pixy = polytrans(cPoints.wpy, easting, northing);

		} else if (m_projection == PROJECTION_POLYCONIC) {
			// Use Projected Polynomial algorithm

			double alon = lon + m_lon_datum_adjust;
			double alat = lat + m_lat_datum_adjust;

			// Get e/n from  Projection
			xlon = alon;
			if (m_bIDLcross) {
				if (xlon < 0.0)
					xlon += 360.0;
			}
			geo::toPOLY(alat, xlon, m_proj_lat, m_proj_lon, &easting, &northing);

			// Apply poly solution to target point
			pixx = polytrans(cPoints.wpx, easting, northing);
			pixy = polytrans(cPoints.wpy, easting, northing);
		}
	}
}

void ChartBaseBSB::chartpix_to_latlong(double pixx, double pixy, double* plat, double* plon) // FIXME: refactoring
{
	if (bHaveEmbeddedGeoref) {
		double lon = polytrans(pwx, pixx, pixy);
		lon = (lon < 0) ? lon + m_cph : lon - m_cph;
		*plon = lon - m_lon_datum_adjust;
		*plat = polytrans(pwy, pixx, pixy) - m_lat_datum_adjust;
	} else {
		double slat, slon;
		if (m_projection == PROJECTION_TRANSVERSE_MERCATOR) {
			// Use Projected Polynomial algorithm

			// Apply polynomial solution to chart relative pixels to get e/n
			double east = polytrans(cPoints.pwx, pixx, pixy);
			double north = polytrans(cPoints.pwy, pixx, pixy);

			// Apply inverse Projection to get lat/lon
			double lat, lon;
			geo::fromTM(east, north, m_proj_lat, m_proj_lon, &lat, &lon);

			// Datum adjustments.....
			slon = lon - m_lon_datum_adjust;
			slat = lat - m_lat_datum_adjust;
		} else if (m_projection == PROJECTION_MERCATOR) {
			// Use Projected Polynomial algorithm
			// Apply polynomial solution to chart relative pixels to get e/n
			double east = polytrans(cPoints.pwx, pixx, pixy);
			double north = polytrans(cPoints.pwy, pixx, pixy);

			// Apply inverse Projection to get lat/lon
			double lat, lon;
			geo::fromSM_ECC(east, north, m_proj_lat, m_proj_lon, &lat, &lon);

			// Make Datum adjustments.....
			slon = lon - m_lon_datum_adjust;
			slat = lat - m_lat_datum_adjust;
		} else if (m_projection == PROJECTION_POLYCONIC) {
			// Use Projected Polynomial algorithm
			// Apply polynomial solution to chart relative pixels to get e/n
			double east = polytrans(cPoints.pwx, pixx, pixy);
			double north = polytrans(cPoints.pwy, pixx, pixy);

			// Apply inverse Projection to get lat/lon
			double lat, lon;
			geo::fromPOLY(east, north, m_proj_lat, m_proj_lon, &lat, &lon);

			// Make Datum adjustments.....
			slon = lon - m_lon_datum_adjust;
			slat = lat - m_lat_datum_adjust;
		} else {
			slon = 0.0;
			slat = 0.0;
		}

		*plat = slat;

		if (slon < -180.0)
			slon += 360.0;
		else if (slon > 180.0)
			slon -= 360.0;
		*plon = slon;
	}
}

void ChartBaseBSB::ComputeSourceRectangle(const ViewPort& vp, wxRect* pSourceRect)
{
	// This funny contortion is necessary to allow scale factors < 1, i.e. overzoom
	double binary_scale_factor = (wxRound(100000 * GetPPM() / vp.view_scale_ppm)) / 100000.0;

	m_raster_scale_factor = binary_scale_factor;

	double xd, yd;
	latlong_to_chartpix(vp.latitude(), vp.longitude(), xd, yd);

	pSourceRect->x = wxRound(xd - (vp.pix_width * binary_scale_factor / 2));
	pSourceRect->y = wxRound(yd - (vp.pix_height * binary_scale_factor / 2));

	pSourceRect->width = (int)wxRound(vp.pix_width * binary_scale_factor);
	pSourceRect->height = (int)wxRound(vp.pix_height * binary_scale_factor);
}

void ChartBaseBSB::SetVPRasterParms(const ViewPort& vpt)
{
	// Calculate the potential datum offset parameters for this viewport, if not WGS84

	if (m_datum_index == DATUM_INDEX_WGS84) {
		m_lon_datum_adjust = 0.0;
		m_lat_datum_adjust = 0.0;
	} else if (m_datum_index == DATUM_INDEX_UNKNOWN) {
		m_lon_datum_adjust = (-m_dtm_lon) / 3600.0;
		m_lat_datum_adjust = (-m_dtm_lat) / 3600.0;
	} else {
		double to_lat;
		double to_lon;
		geo::MolodenskyTransform(vpt.latitude(), vpt.longitude(), &to_lat, &to_lon, m_datum_index,
								 DATUM_INDEX_WGS84);
		m_lon_datum_adjust = -(to_lon - vpt.longitude());
		m_lat_datum_adjust = -(to_lat - vpt.latitude());
		if (m_b_apply_dtm) {
			m_lon_datum_adjust -= m_dtm_lon / 3600.0;
			m_lat_datum_adjust -= m_dtm_lat / 3600.0;
		}
	}

	ComputeSourceRectangle(vpt, &Rsrc);

	if (vpt.IsValid())
		m_vp_render_last = vpt;
}

bool ChartBaseBSB::AdjustVP(const ViewPort& vp_last, ViewPort& vp_proposed)
{
	bool bInside = G_FloatPtInPolygon((MyFlPoint*)GetCOVRTableHead(0), GetCOVRTablenPoints(0),
									  vp_proposed.longitude(), vp_proposed.latitude());
	if (!bInside)
		return false;

	ViewPort vp_save = vp_proposed; // save a copy

	int ret_val = 0;
	double binary_scale_factor = GetPPM() / vp_proposed.view_scale_ppm;

	if (vp_last.IsValid()) {
		// We only need to adjust the VP if the cache is valid and potentially usable, i.e. the
		// scale factor is integer...
		// The objective here is to ensure that the VP center falls on an exact pixel boundary
		// within the cache

		if (cached_image_ok && (binary_scale_factor > 1.0)
			&& (fabs(binary_scale_factor - wxRound(binary_scale_factor)) < 1e-5)) {
			wxRect rprop;
			ComputeSourceRectangle(vp_proposed, &rprop);

			int pixx, pixy;
			double lon_adj, lat_adj;
			latlong_to_pix_vp(vp_proposed.latitude(), vp_proposed.longitude(), pixx, pixy,
							  vp_proposed);
			vp_pix_to_latlong(vp_proposed, pixx, pixy, &lat_adj, &lon_adj);

			vp_proposed.set_position(Position(lat_adj, lon_adj));
			ret_val = 1;
		}
	}

	return (ret_val > 0);
}

bool ChartBaseBSB::IsRenderCacheable(wxRect& source, wxRect& dest)
{
	double scale_x = (double)source.width / (double)dest.width;

	if (scale_x <= 1.0) {
		// overzoom
		return false;
	}

	// Using the cache only works for pure binary scale factors......
	if ((fabs(scale_x - wxRound(scale_x))) > 0.0001) {
		return false;
	}

	// Scale must be exactly digital...
	if ((int)(source.width / dest.width) != (int)wxRound(scale_x)) {
		return false;
	}

	return true;
}

void ChartBaseBSB::GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion* pValidRegion)
{
	SetVPRasterParms(VPoint);

	double raster_scale = VPoint.view_scale_ppm / GetPPM();

	int rxl, rxr;
	if (Rsrc.x < 0)
		rxl = (int)(-Rsrc.x * raster_scale);
	else
		rxl = 0;

	if (((Size_X - Rsrc.x) * raster_scale) < VPoint.pix_width)
		rxr = (int)((Size_X - Rsrc.x) * raster_scale);
	else
		rxr = VPoint.pix_width;

	int ryb, ryt;
	if (Rsrc.y < 0)
		ryt = (int)(-Rsrc.y * raster_scale);
	else
		ryt = 0;

	if (((Size_Y - Rsrc.y) * raster_scale) < VPoint.pix_height)
		ryb = (int)((Size_Y - Rsrc.y) * raster_scale);
	else
		ryb = VPoint.pix_height;

	pValidRegion->Clear();
	pValidRegion->Union(rxl, ryt, rxr - rxl, ryb - ryt);
}

bool ChartBaseBSB::GetViewUsingCache(wxRect& source, wxRect& dest, const OCPNRegion& Region,
									 ScaleTypeEnum scale_type)
{
	wxRect s1;
	ScaleTypeEnum scale_type_corrected;

	// Anything to do?
	if ((source == cache_rect) && (cached_image_ok)) {
		return false;
	}

	double scale_x = (double)source.width / (double)dest.width;

	// Enforce a limit on bilinear scaling, for performance reasons
	scale_type_corrected = scale_type; // RENDER_LODEF; //scale_type;
	if (scale_x > m_bilinear_limit)
		scale_type_corrected = RENDER_LODEF;

	// Using the cache only works for pure binary scale factors......
	if ((fabs(scale_x - wxRound(scale_x))) > 0.0001) {
		return GetView(source, dest, scale_type_corrected);
	}

	if (!cached_image_ok) {
		return GetView(source, dest, scale_type_corrected);
	}

	if (scale_x < 1.0) {
		// overzoom
		return GetView(source, dest, scale_type_corrected);
	}

	// Scale must be exactly digital...
	if ((int)(source.width / dest.width) != (int)wxRound(scale_x)) {
		return GetView(source, dest, scale_type_corrected);
	}

	// Calculate the digital scale, e.g. 1,2,4,8,,,
	int cs1d = source.width / dest.width;
	if (abs(source.x - cache_rect.x) % cs1d) {
		return GetView(source, dest, scale_type_corrected);
	}
	if (abs(source.y - cache_rect.y) % cs1d) {
		return GetView(source, dest, scale_type_corrected);
	}

	if (pPixCache
		&& ((pPixCache->GetWidth() != dest.width) || (pPixCache->GetHeight() != dest.height))) {
		return GetView(source, dest, scale_type_corrected);
	}

	int stride_rows = (source.y + source.height) - (cache_rect.y + cache_rect.height);
	int scaled_stride_rows = (int)(stride_rows / scale_x);

	if (abs(stride_rows) >= source.height) // Pan more than one screen
		return GetView(source, dest, scale_type_corrected);

	int stride_pixels = (source.x + source.width) - (cache_rect.x + cache_rect.width);
	int scaled_stride_pixels = (int)(stride_pixels / scale_x);

	if (abs(stride_pixels) >= source.width) // Pan more than one screen
		return GetView(source, dest, scale_type_corrected);

	ScaleTypeEnum pan_scale_type_x = scale_type_corrected;
	ScaleTypeEnum pan_scale_type_y = scale_type_corrected;

	// "Blit" the valid pixels out of the way
	int height = pPixCache->GetHeight();
	int width = pPixCache->GetWidth();
	int stride = pPixCache->GetLinePitch();

	unsigned char* ps;
	unsigned char* pd;

	if (stride_rows > 0) {
		// pan down
		ps = pPixCache->GetpData() + (abs(scaled_stride_rows) * stride);
		if (stride_pixels > 0)
			ps += scaled_stride_pixels * BPP / 8;

		pd = pPixCache->GetpData();
		if (stride_pixels <= 0)
			pd += abs(scaled_stride_pixels) * BPP / 8;

		for (int iy = 0; iy < (height - abs(scaled_stride_rows)); iy++) {
			memmove(pd, ps, (width - abs(scaled_stride_pixels)) * BPP / 8);

			ps += width * BPP / 8;
			pd += width * BPP / 8;
		}

	} else {
		ps = pPixCache->GetpData() + ((height - abs(scaled_stride_rows) - 1) * stride);
		if (stride_pixels > 0) // make a hole on right
			ps += scaled_stride_pixels * BPP / 8;

		pd = pPixCache->GetpData() + ((height - 1) * stride);
		if (stride_pixels <= 0) // make a hole on the left
			pd += abs(scaled_stride_pixels) * BPP / 8;

		for (int iy = 0; iy < (height - abs(scaled_stride_rows)); iy++) {
			memmove(pd, ps, (width - abs(scaled_stride_pixels)) * BPP / 8);

			ps -= width * BPP / 8;
			pd -= width * BPP / 8;
		}
	}

	// Y Pan
	if (source.y != cache_rect.y) {
		wxRect sub_dest = dest;
		sub_dest.height = abs(scaled_stride_rows);

		if (stride_rows > 0) {
			// pan down
			sub_dest.y = height - scaled_stride_rows;

		} else {
			sub_dest.y = 0;
		}

		// Get the new bits needed

		// A little optimization...
		// No sense in fetching bits that are not part of the ultimate render region
		wxRegionContain rc = Region.Contains(sub_dest);
		if ((wxPartRegion == rc) || (wxInRegion == rc)) {
			GetAndScaleData(pPixCache->GetpData(), source, source.width, sub_dest, width, cs1d,
							pan_scale_type_y);
		}
		pPixCache->Update();

		// Update the cached parameters, Y only

		cache_rect.y = source.y;
		cache_rect_scaled = dest;
		cached_image_ok = 1;
	}

	// X Pan
	if (source.x != cache_rect.x) {
		wxRect sub_dest = dest;
		sub_dest.width = abs(scaled_stride_pixels);

		if (stride_pixels > 0) {
			// pan right
			sub_dest.x = width - scaled_stride_pixels;
		} else {
			// pan left
			sub_dest.x = 0;
		}

		// Get the new bits needed

		// A little optimization...
		// No sense in fetching bits that are not part of the ultimate render region
		wxRegionContain rc = Region.Contains(sub_dest);
		if ((wxPartRegion == rc) || (wxInRegion == rc)) {
			GetAndScaleData(pPixCache->GetpData(), source, source.width, sub_dest, width, cs1d,
							pan_scale_type_x);
		}

		pPixCache->Update();

		// Update the cached parameters
		cache_rect = source;
		cache_rect_scaled = dest;
		cached_image_ok = 1;
	}

	return true;
}

bool ChartBaseBSB::RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint)
{
	SetVPRasterParms(VPoint);

	OCPNRegion rgn(0, 0, VPoint.pix_width, VPoint.pix_height);

	bool bsame_region = (rgn == m_last_region); // only want to do this once

	if (!bsame_region)
		cached_image_ok = false;

	m_last_region = rgn;

	return RenderRegionViewOnDC(dc, VPoint, rgn);
}

bool ChartBaseBSB::RenderRegionViewOnGL(const wxGLContext& WXUNUSED(glc),
										const ViewPort& WXUNUSED(VPoint),
										const OCPNRegion& WXUNUSED(Region))
{
	return true;
}

bool ChartBaseBSB::RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
										const OCPNRegion& Region)
{
	SetVPRasterParms(VPoint);

	wxRect dest(0, 0, VPoint.pix_width, VPoint.pix_height);
	double factor = m_raster_scale_factor;
	if (m_b_cdebug) {
		OCPNRegionIterator upd(Region); // get the requested rect list
		while (upd.HaveRects()) {
			upd.NextRect();
		}
	}

	// Invalidate the cache if the scale has changed or the viewport size has changed....
	if ((fabs(m_cached_scale_ppm - VPoint.view_scale_ppm) > 1e-9) || (m_last_vprect != dest)) {
		cached_image_ok = false;
		m_vp_render_last.Invalidate();
	}

	if (pPixCache) {
		if ((pPixCache->GetWidth() != dest.width) || (pPixCache->GetHeight() != dest.height)) {
			delete pPixCache;
			pPixCache = new PixelCache(dest.width, dest.height, BPP);
		}
	} else
		pPixCache = new PixelCache(dest.width, dest.height, BPP);

	m_cached_scale_ppm = VPoint.view_scale_ppm;
	m_last_vprect = dest;

	if (cached_image_ok) {
		//    Anything to do?
		bool bsame_region = (Region == m_last_region); // only want to do this once

		if ((bsame_region) && (Rsrc == cache_rect)) {
			pPixCache->SelectIntoDC(dc);
			return false;
		}
	}

	m_last_region = Region;

	// Analyze the region requested
	// When rendering complex regions, (more than say 4 rectangles)
	// .OR. small proportions, then rectangle rendering may be faster
	// Also true  if the scale is less than near unity, or overzoom.
	// This will be the case for backgrounds of the quilt.

	// Update for Version 2.4.0
	// This logic seems flawed, at least for quilts which contain charts having non-rectangular
	// coverage areas. These quilt regions decompose to ...LOTS... of rectangles, most of which are
	// 1 pixel in height. This is very slow, due to the overhead of GetAndScaleData().
	// However, remember that overzoom never uses the cache, nor does non-binary scale factors..
	// So, we check to see if this is a cacheable render, and that the number of rectangles is
	// "reasonable"

	// Get the region rectangle count

	int n_rect = 0;
	OCPNRegionIterator upd(Region); // get the requested rect list
	while (upd.HaveRects()) {
		n_rect++;
		upd.NextRect();
	}

	if ((!IsRenderCacheable(Rsrc, dest) && (n_rect > 4) && (n_rect < 20)) || (factor < 1)) {
		ScaleTypeEnum ren_type = RENDER_LODEF;

		// Decompose the region into rectangles, and fetch them into the target dc
		OCPNRegionIterator upd(Region); // get the requested rect list
		int ir = 0;
		while (upd.HaveRects()) {
			wxRect rect = upd.GetRect();
			GetAndScaleData(pPixCache->GetpData(), Rsrc, Rsrc.width, rect, dest.width, factor,
							ren_type);
			ir++;
			upd.NextRect();
			;
		}

		pPixCache->Update();

		// Update cache parameters
		cache_rect = Rsrc;
		cache_scale_method = ren_type;
		cached_image_ok = false; // Never cache this type of render

		// Select the data into the dc
		pPixCache->SelectIntoDC(dc);

		return true;
	}

	// A performance enhancement.....
	ScaleTypeEnum scale_type_zoom = RENDER_HIDEF;
	double binary_scale_factor = VPoint.view_scale_ppm / GetPPM();
	if (binary_scale_factor < .20)
		scale_type_zoom = RENDER_LODEF;

	bool bnewview = GetViewUsingCache(Rsrc, dest, Region, scale_type_zoom);

	// Select the data into the dc
	pPixCache->SelectIntoDC(dc);

	return bnewview;
}

wxImage* ChartBaseBSB::GetImage()
{
	int img_size_x = ((Size_X >> 2) * 4) + 4;
	wxImage* img = new wxImage(img_size_x, Size_Y, false);

	unsigned char* ppnx = img->GetData();

	for (int i = 0; i < Size_Y; i++) {
		wxRect source_rect(0, i, Size_X, 1);
		wxRect dest_rect(0, 0, Size_X, 1);

		GetAndScaleData(img->GetData(), source_rect, Size_X, dest_rect, Size_X, 1.0, RENDER_HIDEF);

		ppnx += img_size_x * 3;
	}

	return img;
}

bool ChartBaseBSB::GetView(wxRect& source, wxRect& dest, ScaleTypeEnum scale_type)
{
	// Get and Rescale the data directly into the temporary PixelCache data buffer
	double factor = ((double)source.width) / ((double)dest.width);

	GetAndScaleData(pPixCache->GetpData(), source, source.width, dest, dest.width, factor,
					scale_type);
	pPixCache->Update();

	// Update cache parameters

	cache_rect = source;
	cache_rect_scaled = dest;
	cache_scale_method = scale_type;

	cached_image_ok = 1;

	return TRUE;
}

bool ChartBaseBSB::GetAndScaleData(unsigned char* ppn, wxRect& source, int WXUNUSED(source_stride),
								   wxRect& dest, int dest_stride, double scale_factor,
								   ScaleTypeEnum scale_type)
{
	unsigned char* s_data = NULL;

	double factor = scale_factor;
	int Factor = (int)factor;

	int target_width = (int)wxRound((double)source.width / factor);
	int target_height = (int)wxRound((double)source.height / factor);

	if ((target_height == 0) || (target_width == 0))
		return false;

	unsigned char* target_data = ppn;
	unsigned char* data = ppn;

	if (factor > 1) {
		// downsampling

		if (scale_type == RENDER_HIDEF) {
			//    Allocate a working buffer based on scale factor
			int blur_factor = wxMax(2, Factor);
			int wb_size = (source.width) * (blur_factor * 2) * BPP / 8;
			s_data = (unsigned char*)malloc(wb_size); // work buffer
			unsigned char* pixel;
			int y_offset;

			for (int y = dest.y; y < (dest.y + dest.height); y++) {
				// Read "blur_factor" lines

				wxRect s1;
				s1.x = source.x;
				s1.y = source.y + (int)(y * factor);
				s1.width = source.width;
				s1.height = blur_factor;
				GetChartBits(s1, s_data, 1);

				target_data = data + (y * dest_stride * BPP / 8);

				for (int x = 0; x < target_width; x++) {
					unsigned int avgRed = 0;
					unsigned int avgGreen = 0;
					unsigned int avgBlue = 0;
					unsigned int pixel_count = 0;
					unsigned char* pix0 = s_data + BPP / 8 * ((int)(x * factor));
					y_offset = 0;

					if ((x * Factor) < (Size_X - source.x)) {
						// determine average
						for (int y1 = 0; y1 < blur_factor; ++y1) {
							pixel = pix0 + (BPP / 8 * y_offset);
							for (int x1 = 0; x1 < blur_factor; ++x1) {
								avgRed += pixel[0];
								avgGreen += pixel[1];
								avgBlue += pixel[2];

								pixel += BPP / 8;

								pixel_count++;
							}
							y_offset += source.width;
						}

						target_data[0] = avgRed / pixel_count; // >> scounter;
						target_data[1] = avgGreen / pixel_count; // >> scounter;
						target_data[2] = avgBlue / pixel_count; // >> scounter;
						target_data += BPP / 8;
					} else {
						target_data[0] = 0;
						target_data[1] = 0;
						target_data[2] = 0;
						target_data += BPP / 8;
					}
				}
			}
		} else if (scale_type == RENDER_LODEF) {
			int get_bits_submap = 1;

			int scaler = 16;

			if (source.width > 32767) // High underscale can exceed signed math bits
				scaler = 8;

			int wb_size = (Size_X) * ((/*Factor +*/ 1) * 2) * BPP / 8;
			s_data = (unsigned char*)malloc(wb_size); // work buffer

			long x_delta = (source.width << scaler) / target_width;
			long y_delta = (source.height << scaler) / target_height;

			int y = dest.y; // starting here
			long ys = dest.y * y_delta;

			while (y < dest.y + dest.height) {
				// Read 1 line at the right place from the source

				wxRect s1;
				s1.x = 0;
				s1.y = source.y + (ys >> scaler);
				s1.width = Size_X;
				s1.height = 1;
				GetChartBits(s1, s_data, get_bits_submap);

				target_data = data + (y * dest_stride * BPP / 8) + (dest.x * BPP / 8);

				long x = (source.x << scaler) + (dest.x * x_delta);
				long sizex16 = Size_X << scaler;
				int xt = dest.x;

				while ((xt < dest.x + dest.width) && (x < 0)) {
					target_data[0] = 0;
					target_data[1] = 0;
					target_data[2] = 0;

					target_data += BPP / 8;
					x += x_delta;
					xt++;
				}

				while ((xt < dest.x + dest.width) && (x < sizex16)) {

					unsigned char* src_pixel = &s_data[(x >> scaler) * BPP / 8];

					target_data[0] = src_pixel[0];
					target_data[1] = src_pixel[1];
					target_data[2] = src_pixel[2];

					target_data += BPP / 8;
					x += x_delta;
					xt++;
				}

				while (xt < dest.x + dest.width) {
					target_data[0] = 0;
					target_data[1] = 0;
					target_data[2] = 0;

					target_data += BPP / 8;
					xt++;
				}

				y++;
				ys += y_delta;
			}
		}
	} else {
		// factor < 1, overzoom
		int i = 0;
		int j = 0;
		unsigned char* target_line_start = NULL;
		unsigned char* target_data_x = NULL;
		int y_offset = 0;

#ifdef __WXGTK__
		sigaction(SIGSEGV, NULL, &sa_all_previous); // save existing action for this signal

		struct sigaction temp;
		sigaction(SIGSEGV, NULL, &temp); // inspect existing action for this signal

		temp.sa_handler = catch_signals_chart; // point to my handler
		sigemptyset(&temp.sa_mask); // make the blocking set
		// empty, so that all
		// other signals will be
		// unblocked during my handler
		temp.sa_flags = 0;
		sigaction(SIGSEGV, &temp, NULL);

		if (sigsetjmp(env_chart, 1)) {
			// Something in the below code block faulted....
			sigaction(SIGSEGV, &sa_all_previous, NULL); // reset signal handler

			wxLogMessage(_T("   Caught SIGSEGV on GetandScaleData, Factor < 1"));
			wxLogMessage(wxString::Format(
				_T("   m_raster_scale_factor:  %g   source.width: %d  dest.y: %d dest.x: %d dest.width: %d  dest.height: %d "),
				m_raster_scale_factor, source.width, dest.y, dest.x, dest.width,
				dest.height));
			wxLogMessage(wxString::Format(
				_T("   i: %d  j: %d dest_stride: %d  target_line_start: %p  target_data_x: %p  y_offset: %d"),
				i, j, dest_stride, target_line_start, target_data_x, y_offset));

			free(s_data);
			return true;

		} else
#endif
		{

			double xd, yd;
			latlong_to_chartpix(m_vp_render_last.latitude(), m_vp_render_last.longitude(), xd, yd);
			double xrd = xd - (m_vp_render_last.pix_width * m_raster_scale_factor / 2);
			double yrd = yd - (m_vp_render_last.pix_height * m_raster_scale_factor / 2);
			double x_vernier = (xrd - wxRound(xrd));
			double y_vernier = (yrd - wxRound(yrd));
			int x_vernier_i = (int)wxRound(x_vernier / m_raster_scale_factor);
			int y_vernier_i = (int)wxRound(y_vernier / m_raster_scale_factor);

			// Seems safe enough to read all the required data
			// Although we must adjust (increase) temporary allocation for negative source.x
			// and for vernier
			int sx = wxMax(source.x, 0);
			s_data
				= (unsigned char*)malloc((sx + source.width + 2) * (source.height + 2) * BPP / 8);

			wxRect vsource = source;
			vsource.height += 2; // get more bits to allow for vernier
			vsource.width += 2;
			vsource.x -= 1;
			vsource.y -= 1;

			GetChartBits(vsource, s_data, 1);
			unsigned char* source_data = s_data;

			j = dest.y;

			while (j < dest.y + dest.height) {
				y_offset = (int)((j - y_vernier_i) * m_raster_scale_factor)
						   * vsource.width; // into the source data

				target_line_start = target_data + (j * dest_stride * BPP / 8);
				target_data_x = target_line_start + ((dest.x) * BPP / 8);

				i = dest.x;

				while (i < dest.x + dest.width) {
					memcpy(target_data_x,
						   source_data + BPP / 8 * (y_offset + (int)((i + x_vernier_i)
																	 * m_raster_scale_factor)),
						   BPP / 8);
					target_data_x += BPP / 8;

					i++;
				}
				j++;
			}
		}
#ifdef __WXGTK__
		sigaction(SIGSEGV, &sa_all_previous, NULL); // reset signal handler
#endif
	}

	free(s_data);

	return true;
}

bool ChartBaseBSB::GetChartBits(wxRect& source, unsigned char* pPix, int sub_samp)
{
	int iy;
#define FILL_BYTE 0

	// Decode the KAP file RLL stream into image pPix

	unsigned char* pCP;
	pCP = pPix;

	iy = source.y;

	while (iy < source.y + source.height) {
		if ((iy >= 0) && (iy < Size_Y)) {
			if (source.x >= 0) {
				if ((source.x + source.width) > Size_X) {
					if ((Size_X - source.x) < 0)
						memset(pCP, FILL_BYTE, source.width * BPP / 8);
					else {

						BSBGetScanline(pCP, iy, source.x, Size_X, sub_samp);
						memset(pCP + (Size_X - source.x) * BPP / 8, FILL_BYTE,
							   (source.x + source.width - Size_X) * BPP / 8);
					}
				} else
					BSBGetScanline(pCP, iy, source.x, source.x + source.width, sub_samp);
			} else {
				if ((source.width + source.x) >= 0) {
					// Special case, black on left side
					// must ensure that (black fill length % sub_samp) == 0

					int xfill_corrected = -source.x + (source.x % sub_samp); //+ve
					memset(pCP, FILL_BYTE, (xfill_corrected * BPP / 8));
					BSBGetScanline(pCP + (xfill_corrected * BPP / 8), iy, 0,
								   source.width + source.x, sub_samp);

				} else {
					memset(pCP, FILL_BYTE, source.width * BPP / 8);
				}
			}
		} else {
			// requested y is off chart
			memset(pCP, FILL_BYTE, source.width * BPP / 8);
		}

		pCP += source.width * BPP / 8 * sub_samp;

		iy += sub_samp;
	}

	return true;
}

/// ReadBSBHdrLine
/// Read and return count of a line of BSB header file
int ChartBaseBSB::ReadBSBHdrLine(wxFileInputStream* ifs, char* buf, int buf_len_max)
{
	char read_char;
	char cr_test;
	int line_length = 0;
	char* lbuf;

	lbuf = buf;

	while (!ifs->Eof() && line_length < buf_len_max) {
		read_char = ifs->GetC();
		if (0x1A == read_char) {
			ifs->Ungetch(read_char);
			return (0);
		}

		if (0 == read_char) // embedded erroneous unicode character?
			read_char = 0x20;

		// Manage continued lines
		if (read_char == 10 || read_char == 13) {

			// Check to see if there is an extra CR
			cr_test = ifs->GetC();
			if (cr_test == 13)
				cr_test = ifs->GetC(); // skip any extra CR

			if (cr_test != 10 && cr_test != 13)
				ifs->Ungetch(cr_test);
			read_char = '\n';
		}

		// Look for continued lines, indicated by ' ' in first position
		if (read_char == '\n') {
			cr_test = 0;
			cr_test = ifs->GetC();

			if (cr_test != ' ') {
				ifs->Ungetch(cr_test);
				*lbuf = '\0';
				return line_length;
			}

			// Merge out leading spaces
			while (cr_test == ' ')
				cr_test = ifs->GetC();
			ifs->Ungetch(cr_test);

			// Add a comma
			*lbuf = ',';
			lbuf++;
		} else {
			*lbuf = read_char;
			lbuf++;
			line_length++;
		}
	}

	// Terminate line
	*(lbuf - 1) = '\0';

	return line_length;
}

/// Scan a BSB Scan Line from raw data
/// Leaving stream pointer at start of next line
int ChartBaseBSB::BSBScanScanline(wxInputStream* pinStream)
{
	int nLineMarker = 0;
	int nValueShift = 0;
	int iPixel = 0;
	unsigned char byValueMask;
	unsigned char byCountMask;
	unsigned char byNext;

	// Read the line number.
	do {
		byNext = pinStream->GetC();
		nLineMarker = nLineMarker * 128 + (byNext & 0x7f);
	} while ((byNext & 0x80) != 0);

	// Setup masking values.
	nValueShift = 7 - nColorSize;
	byValueMask = (((1 << nColorSize)) - 1) << nValueShift;
	byCountMask = (1 << (7 - nColorSize)) - 1;

	// Read and simulate expansion of runs.

	while (((byNext = pinStream->GetC()) != 0) && (iPixel < Size_X)) {

		int nRunCount = byNext & byCountMask;

		while ((byNext & 0x80) != 0) {
			byNext = pinStream->GetC();
			nRunCount = nRunCount * 128 + (byNext & 0x7f);
		}

		if (iPixel + nRunCount + 1 > Size_X)
			nRunCount = Size_X - iPixel - 1;

		// Store nPixValue in the destination
		iPixel += nRunCount + 1;
	}

	return nLineMarker;
}

/// Get a BSB Scan Line Using Cache and scan line index if available
int ChartBaseBSB::BSBGetScanline(unsigned char* pLineBuf, int y, int xs, int xl, int sub_samp)
{
	int nLineMarker;
	int nValueShift;
	int iPixel = 0;
	unsigned char byValueMask;
	unsigned char byCountMask;
	unsigned char byNext;
	CachedLine* pt = NULL;
	unsigned char* pCL;
	int rgbval;
	unsigned char* lp;
	unsigned char* xtemp_line;
	register int ix = xs;

	if (bUseLineCache && pLineCache) {
		//    Is the requested line in the cache, and valid?
		pt = &pLineCache[y];
		if (!pt->bValid) // not valid, so get it
		{
			if (pt->pPix)
				free(pt->pPix);
			pt->pPix = static_cast<unsigned char*>(malloc(Size_X));
		}

		xtemp_line = pt->pPix;
	} else {
		xtemp_line = static_cast<unsigned char*>(malloc(Size_X));
	}

	if ((bUseLineCache && !pt->bValid) || (!bUseLineCache)) {
		if (line_offset_table[y] == 0) {
			free(xtemp_line);
			return 0;
		}

		if (line_offset_table[y + 1] == 0) {
			free(xtemp_line);
			return 0;
		}

		int thisline_size = line_offset_table[y + 1] - line_offset_table[y];

		if (thisline_size > ifs_bufsize) {
			unsigned char* tmp = ifs_buf;
			ifs_buf = (unsigned char*)realloc(ifs_buf, thisline_size);
			if (NULL == ifs_buf) {
				free(tmp);
				tmp = NULL;
				free(xtemp_line);
				return 0;
			}
		}

		if (wxInvalidOffset == ifs_bitmap->SeekI(line_offset_table[y], wxFromStart)) {
			free(xtemp_line);
			return 0;
		}

		ifs_bitmap->Read(ifs_buf, thisline_size);
		lp = ifs_buf;

		// At this point, the unexpanded, raw line is at *lp, and the expansion destination is
		// xtemp_line

		// Read the line number.
		nLineMarker = 0;
		do {
			byNext = *lp++;
			nLineMarker = nLineMarker * 128 + (byNext & 0x7f);
		} while ((byNext & 0x80) != 0);

		// Setup masking values.
		nValueShift = 7 - nColorSize;
		byValueMask = (((1 << nColorSize)) - 1) << nValueShift;
		byCountMask = (1 << (7 - nColorSize)) - 1;

		// Read and expand runs.

		pCL = xtemp_line;

		while (((byNext = *lp++) != 0) && (iPixel < Size_X)) {
			int nPixValue;
			int nRunCount;
			nPixValue = (byNext & byValueMask) >> nValueShift;

			nRunCount = byNext & byCountMask;

			while ((byNext & 0x80) != 0) {
				byNext = *lp++;
				nRunCount = nRunCount * 128 + (byNext & 0x7f);
			}

			if (iPixel + nRunCount + 1 > Size_X) // protection
				nRunCount = Size_X - iPixel - 1;

			if (nRunCount < 0) // against corrupt data
				nRunCount = 0;

			// Store nPixValue in the destination
			memset(pCL, nPixValue, nRunCount + 1);
			pCL += nRunCount + 1;
			iPixel += nRunCount + 1;
		}
	}

	if (bUseLineCache)
		pt->bValid = true;

	// Line is valid, de-reference thru proper pallete directly to target

	if (xl > Size_X - 1)
		xl = Size_X - 1;

	pCL = xtemp_line + xs;
	unsigned char* prgb = pLineBuf;

	// Optimization for most usual case
	if ((BPP == 24) && (1 == sub_samp)) {
		ix = xs;
		while (ix < xl - 1) {
			unsigned char cur_by = *pCL;
			rgbval = pPalette[cur_by];
			while ((ix < xl - 1)) {
				if (cur_by != *pCL)
					break;
				*((int*)prgb) = rgbval;
				prgb += 3;
				pCL++;
				ix++;
			}
		}
	} else {
		int dest_inc_val_bytes = (BPP / 8) * sub_samp;
		ix = xs;
		while (ix < xl - 1) {
			unsigned char cur_by = *pCL;
			rgbval = pPalette[cur_by];
			while ((ix < xl - 1)) {
				if (cur_by != *pCL)
					break;
				*((int*)prgb) = rgbval;
				prgb += dest_inc_val_bytes;
				pCL += sub_samp;
				ix += sub_samp;
			}
		}
	}

	// Get the last pixel explicitely irrespective of the sub_sampling factor

	if (xs < xl - 1) {
		unsigned char* pCLast = xtemp_line + (xl - 1);
		unsigned char* prgb_last = pLineBuf + ((xl - 1) - xs) * BPP / 8;

		rgbval = pPalette[*pCLast]; // last pixel
		unsigned char a = rgbval & 0xff;
		*prgb_last++ = a;
		a = (rgbval >> 8) & 0xff;
		*prgb_last++ = a;
		a = (rgbval >> 16) & 0xff;
		*prgb_last = a;
	}

	if (!bUseLineCache)
		free(xtemp_line);

	return 1;
}

int* ChartBaseBSB::GetPalettePtr(BSB_Color_Capability color_index)
{
	if (pPalettes[color_index]) {
		if (palette_direction == PaletteFwd)
			return pPalettes[color_index]->FwdPalette;
		else
			return pPalettes[color_index]->RevPalette;
	}
	return NULL;
}

PaletteDir ChartBaseBSB::GetPaletteDir(void)
{
	// make a pixel cache
	PixelCache* pc = new PixelCache(4, 4, BPP);
	RGBO r = pc->GetRGBO();
	delete pc;

	if (r == RGB)
		return PaletteFwd;
	else
		return PaletteRev;
}

bool ChartBaseBSB::AnalyzeSkew(void)
{
	double lonmin = 1000;
	double lonmax = -1000;
	double latmin = 90.0;
	double latmax = -90.0;

	int plonmin = 100000;
	int plonmax = 0;
	int platmin = 100000;
	int platmax = 0;

	int nlonmin = 0;
	int nlonmax = 0;
	int nlatmax = 0;
	int nlatmin = 0;

	if (reference_points.empty()) // bad chart georef...
		return false;

	// FIXME: partially code duplication, see AnalyzeRefpoints

	for (unsigned int n = 0; n < reference_points.size(); ++n) {
		// Longitude
		if (reference_points[n].lonr > lonmax) {
			lonmax = reference_points[n].lonr;
			plonmax = static_cast<int>(reference_points[n].xr);
			nlonmax = n;
		}
		if (reference_points[n].lonr < lonmin) {
			lonmin = reference_points[n].lonr;
			plonmin = static_cast<int>(reference_points[n].xr);
			nlonmin = n;
		}

		// Latitude
		if (reference_points[n].latr < latmin) {
			latmin = reference_points[n].latr;
			platmin = static_cast<int>(reference_points[n].yr);
			nlatmin = n;
		}
		if (reference_points[n].latr > latmax) {
			latmax = reference_points[n].latr;
			platmax = static_cast<int>(reference_points[n].yr);
			nlatmax = n;
		}
	}

	// Special case for charts which cross the IDL
	if ((lonmin * lonmax) < 0) {
		if (reference_points[nlonmin].xr > reference_points[nlonmax].xr) {
			// walk the reference table and add 360 to any longitude which is < 0
			for (unsigned int n = 0; n < reference_points.size(); n++) {
				if (reference_points[n].lonr < 0.0)
					reference_points[n].lonr += 360.0;
			}

			// And recalculate the  min/max
			lonmin = 1000;
			lonmax = -1000;

			for (unsigned int n = 0; n < reference_points.size(); n++) {
				// Longitude
				if (reference_points[n].lonr > lonmax) {
					lonmax = reference_points[n].lonr;
					plonmax = static_cast<int>(reference_points[n].xr);
					nlonmax = n;
				}
				if (reference_points[n].lonr < lonmin) {
					lonmin = reference_points[n].lonr;
					plonmin = static_cast<int>(reference_points[n].xr);
					nlonmin = n;
				}

				// Latitude
				if (reference_points[n].latr < latmin) {
					latmin = reference_points[n].latr;
					platmin = static_cast<int>(reference_points[n].yr);
					nlatmin = n;
				}
				if (reference_points[n].latr > latmax) {
					latmax = reference_points[n].latr;
					platmax = static_cast<int>(reference_points[n].yr);
					nlatmax = n;
				}
			}
			m_bIDLcross = true;
		}
	}

	// Find the two REF points that are farthest apart
	double dist_max = 0.0;
	int imax = 0;
	int jmax = 0;

	for (unsigned int i = 0; i < reference_points.size(); i++) {
		for (unsigned int j = i + 1; j < reference_points.size(); j++) {
			double dx = reference_points[i].xr - reference_points[j].xr;
			double dy = reference_points[i].yr - reference_points[j].yr;
			double dist = (dx * dx) + (dy * dy);
			if (dist > dist_max) {
				dist_max = dist;
				imax = i;
				jmax = j;
			}
		}
	}

	if (m_projection == PROJECTION_MERCATOR) {
		double easting0, easting1, northing0, northing1;
		//  Get the Merc projection of the two REF points
		geo::toSM_ECC(reference_points[imax].latr, reference_points[imax].lonr, m_proj_lat,
					  m_proj_lon, &easting0, &northing0);
		geo::toSM_ECC(reference_points[jmax].latr, reference_points[jmax].lonr, m_proj_lat,
					  m_proj_lon, &easting1, &northing1);

		double skew_proj = atan2((easting1 - easting0), (northing1 - northing0)) * 180.0 / M_PI;
		double skew_points = atan2((reference_points[jmax].yr - reference_points[imax].yr),
								   (reference_points[jmax].xr - reference_points[imax].xr)) * 180.0
							 / M_PI;

		double apparent_skew = skew_points - skew_proj + 90.0;

		// normalize to +/- 180.
		if (fabs(apparent_skew) > 180.0) {
			if (apparent_skew < 0.0)
				apparent_skew += 360.0;
			else
				apparent_skew -= 360.0;
		}

		if (fabs(apparent_skew - m_Chart_Skew) > 2) { // measured skew is more than 2 degrees
			m_Chart_Skew = apparent_skew; // different from stated skew
			wxLogMessage(_T("   Warning: Skew override on chart ") + m_FullPath
						 + wxString::Format(_T(" is %5g degrees"), apparent_skew));
			return false;
		}
	}

	return true;
}

int ChartBaseBSB::AnalyzeRefpoints(void)
{
	double elt, elg;

	// Calculate the max/min reference points

	float lonmin = 1000;
	float lonmax = -1000;
	float latmin = 90.0;
	float latmax = -90.0;

	int plonmin = 100000;
	int plonmax = 0;
	int platmin = 100000;
	int platmax = 0;
	int nlonmin = 0;
	int nlonmax = 0;
	int nlatmax = 0;
	int nlatmin = 0;

	if (reference_points.empty()) // bad chart georef...
		return 1;

	// FIXME: partially code duplication, see AnalyzeSkew

	for (unsigned int n = 0; n < reference_points.size(); n++) {
		// Longitude
		if (reference_points[n].lonr > lonmax) {
			lonmax = reference_points[n].lonr;
			plonmax = static_cast<int>(reference_points[n].xr);
			nlonmax = n;
		}
		if (reference_points[n].lonr < lonmin) {
			lonmin = reference_points[n].lonr;
			plonmin = static_cast<int>(reference_points[n].xr);
			nlonmin = n;
		}

		// Latitude
		if (reference_points[n].latr < latmin) {
			latmin = reference_points[n].latr;
			platmin = static_cast<int>(reference_points[n].yr);
			nlatmin = n;
		}
		if (reference_points[n].latr > latmax) {
			latmax = reference_points[n].latr;
			platmax = static_cast<int>(reference_points[n].yr);
			nlatmax = n;
		}
	}

	// Special case for charts which cross the IDL
	if ((lonmin * lonmax) < 0) {
		if (reference_points[nlonmin].xr > reference_points[nlonmax].xr) {
			// walk the reference table and add 360 to any longitude which is < 0
			for (unsigned int n = 0; n < reference_points.size(); n++) {
				if (reference_points[n].lonr < 0.0)
					reference_points[n].lonr += 360.0;
			}

			// And recalculate the  min/max
			lonmin = 1000;
			lonmax = -1000;

			for (unsigned int n = 0; n < reference_points.size(); n++) {
				// Longitude
				if (reference_points[n].lonr > lonmax) {
					lonmax = reference_points[n].lonr;
					plonmax = static_cast<int>(reference_points[n].xr);
					nlonmax = n;
				}
				if (reference_points[n].lonr < lonmin) {
					lonmin = reference_points[n].lonr;
					plonmin = static_cast<int>(reference_points[n].xr);
					nlonmin = n;
				}

				// Latitude
				if (reference_points[n].latr < latmin) {
					latmin = reference_points[n].latr;
					platmin = static_cast<int>(reference_points[n].yr);
					nlatmin = n;
				}
				if (reference_points[n].latr > latmax) {
					latmax = reference_points[n].latr;
					platmax = static_cast<int>(reference_points[n].yr);
					nlatmax = n;
				}
			}
			m_bIDLcross = true;
		}
	}

	// Build the Control Point Structure, etc
	cPoints.count = reference_points.size();

	cPoints.tx = static_cast<double*>(malloc(reference_points.size() * sizeof(double)));
	cPoints.ty = static_cast<double*>(malloc(reference_points.size() * sizeof(double)));
	cPoints.lon = static_cast<double*>(malloc(reference_points.size() * sizeof(double)));
	cPoints.lat = static_cast<double*>(malloc(reference_points.size() * sizeof(double)));

	cPoints.pwx = static_cast<double*>(malloc(12 * sizeof(double)));
	cPoints.wpx = static_cast<double*>(malloc(12 * sizeof(double)));
	cPoints.pwy = static_cast<double*>(malloc(12 * sizeof(double)));
	cPoints.wpy = static_cast<double*>(malloc(12 * sizeof(double)));

	// Find the two REF points that are farthest apart
	double dist_max = 0.;
	unsigned int imax = 0;
	unsigned int jmax = 0;

	for (unsigned int i = 0; i < reference_points.size(); i++) {
		for (unsigned int j = i + 1; j < reference_points.size(); j++) {
			double dx = reference_points[i].xr - reference_points[j].xr;
			double dy = reference_points[i].yr - reference_points[j].yr;
			double dist = (dx * dx) + (dy * dy);
			if (dist > dist_max) {
				dist_max = dist;
				imax = i;
				jmax = j;
			}
		}
	}

	// Georef solution depends on projection type

	if (m_projection == PROJECTION_TRANSVERSE_MERCATOR) {
		double easting0, easting1, northing0, northing1;
		// Get the TMerc projection of the two REF points
		geo::toTM(reference_points[imax].latr, reference_points[imax].lonr, m_proj_lat, m_proj_lon,
				  &easting0, &northing0);
		geo::toTM(reference_points[jmax].latr, reference_points[jmax].lonr, m_proj_lat, m_proj_lon,
				  &easting1, &northing1);

		// Calculate the scale factor using exact REF point math
		double dx2 = (reference_points[jmax].xr - reference_points[imax].xr)
					 * (reference_points[jmax].xr - reference_points[imax].xr);
		double dy2 = (reference_points[jmax].yr - reference_points[imax].yr)
					 * (reference_points[jmax].yr - reference_points[imax].yr);
		double dn2 = (northing1 - northing0) * (northing1 - northing0);
		double de2 = (easting1 - easting0) * (easting1 - easting0);

		m_ppm_avg = sqrt(dx2 + dy2) / sqrt(dn2 + de2);

		// Set up and solve polynomial solution for pix<->east/north as projected
		// Fill the cpoints structure with pixel points and transformed lat/lon

		for (unsigned int n = 0; n < reference_points.size(); n++) {
			double easting, northing;
			geo::toTM(reference_points[n].latr, reference_points[n].lonr, m_proj_lat, m_proj_lon,
					  &easting, &northing);

			cPoints.tx[n] = reference_points[n].xr;
			cPoints.ty[n] = reference_points[n].yr;
			cPoints.lon[n] = easting;
			cPoints.lat[n] = northing;
		}

		// Helper parameters
		cPoints.txmax = plonmax;
		cPoints.txmin = plonmin;
		cPoints.tymax = platmax;
		cPoints.tymin = platmin;
		geo::toTM(latmax, lonmax, m_proj_lat, m_proj_lon, &cPoints.lonmax, &cPoints.latmax);
		geo::toTM(latmin, lonmin, m_proj_lat, m_proj_lon, &cPoints.lonmin, &cPoints.latmin);

		cPoints.status = 1;

		Georef_Calculate_Coefficients_Proj(&cPoints);

	} else if (m_projection == PROJECTION_MERCATOR) {
		double easting0, easting1, northing0, northing1;
		// Get the Merc projection of the two REF points
		geo::toSM_ECC(reference_points[imax].latr, reference_points[imax].lonr, m_proj_lat,
					  m_proj_lon, &easting0, &northing0);
		geo::toSM_ECC(reference_points[jmax].latr, reference_points[jmax].lonr, m_proj_lat,
					  m_proj_lon, &easting1, &northing1);

		double dx2 = (reference_points[jmax].xr - reference_points[imax].xr)
					 * (reference_points[jmax].xr - reference_points[imax].xr);
		double dy2 = (reference_points[jmax].yr - reference_points[imax].yr)
					 * (reference_points[jmax].yr - reference_points[imax].yr);
		double dn2 = (northing1 - northing0) * (northing1 - northing0);
		double de2 = (easting1 - easting0) * (easting1 - easting0);

		m_ppm_avg = sqrt(dx2 + dy2) / sqrt(dn2 + de2);

		// Set up and solve polynomial solution for pix<->east/north as projected
		// Fill the cpoints structure with pixel points and transformed lat/lon

		for (unsigned int n = 0; n < reference_points.size(); n++) {
			double easting, northing;
			geo::toSM_ECC(reference_points[n].latr, reference_points[n].lonr, m_proj_lat,
						  m_proj_lon, &easting, &northing);

			cPoints.tx[n] = reference_points[n].xr;
			cPoints.ty[n] = reference_points[n].yr;
			cPoints.lon[n] = easting;
			cPoints.lat[n] = northing;
		}

		// Helper parameters
		cPoints.txmax = plonmax;
		cPoints.txmin = plonmin;
		cPoints.tymax = platmax;
		cPoints.tymin = platmin;
		geo::toSM_ECC(latmax, lonmax, m_proj_lat, m_proj_lon, &cPoints.lonmax, &cPoints.latmax);
		geo::toSM_ECC(latmin, lonmin, m_proj_lat, m_proj_lon, &cPoints.lonmin, &cPoints.latmin);

		cPoints.status = 1;

		Georef_Calculate_Coefficients_Proj(&cPoints);
	} else if (m_projection == PROJECTION_POLYCONIC) {
		// This is interesting
		// On some BSB V 1.0 Polyconic charts (e.g. 14500_1, 1995), the projection parameter
		// Which is taken to be the central meridian of the projection is of the wrong sign....

		// We check for this case, and make a correction if necessary.....
		// Obviously, the projection meridian should be on the chart, i.e. between the min and max
		// longitudes....
		double proj_meridian = m_proj_lon;

		if ((reference_points[nlonmax].lonr >= -proj_meridian)
			&& (-proj_meridian >= reference_points[nlonmin].lonr))
			m_proj_lon = -m_proj_lon;

		double easting0, easting1, northing0, northing1;
		// Get the Poly projection of the two REF points
		geo::toPOLY(reference_points[imax].latr, reference_points[imax].lonr, m_proj_lat,
					m_proj_lon, &easting0, &northing0);
		geo::toPOLY(reference_points[jmax].latr, reference_points[jmax].lonr, m_proj_lat,
					m_proj_lon, &easting1, &northing1);

		// Calculate the scale factor using exact REF point math
		double dx2 = (reference_points[jmax].xr - reference_points[imax].xr)
					 * (reference_points[jmax].xr - reference_points[imax].xr);
		double dy2 = (reference_points[jmax].yr - reference_points[imax].yr)
					 * (reference_points[jmax].yr - reference_points[imax].yr);
		double dn2 = (northing1 - northing0) * (northing1 - northing0);
		double de2 = (easting1 - easting0) * (easting1 - easting0);

		m_ppm_avg = sqrt(dx2 + dy2) / sqrt(dn2 + de2);

		// Set up and solve polynomial solution for pix<->cartesian east/north as projected
		// Fill the cpoints structure with pixel points and transformed lat/lon

		for (unsigned int n = 0; n < reference_points.size(); n++) {
			double easting;
			double northing;
			geo::toPOLY(reference_points[n].latr, reference_points[n].lonr, m_proj_lat, m_proj_lon,
						&easting, &northing);

			cPoints.tx[n] = reference_points[n].xr;
			cPoints.ty[n] = reference_points[n].yr;
			cPoints.lon[n] = easting;
			cPoints.lat[n] = northing;
		}

		// Helper parameters
		cPoints.txmax = plonmax;
		cPoints.txmin = plonmin;
		cPoints.tymax = platmax;
		cPoints.tymin = platmin;
		geo::toPOLY(latmax, lonmax, m_proj_lat, m_proj_lon, &cPoints.lonmax, &cPoints.latmax);
		geo::toPOLY(latmin, lonmin, m_proj_lat, m_proj_lon, &cPoints.lonmin, &cPoints.latmin);

		cPoints.status = 1;

		Georef_Calculate_Coefficients_Proj(&cPoints);

	} else if (bHaveEmbeddedGeoref) {
		// Any other projection had better have embedded coefficients
		// Use a Mercator Projection to get a rough ppm.
		double easting0, easting1, northing0, northing1;
		// Get the Merc projection of the two REF points
		geo::toSM_ECC(reference_points[imax].latr, reference_points[imax].lonr, m_proj_lat,
					  m_proj_lon, &easting0, &northing0);
		geo::toSM_ECC(reference_points[jmax].latr, reference_points[jmax].lonr, m_proj_lat,
					  m_proj_lon, &easting1, &northing1);

		// Calculate the scale factor using exact REF point math in x(longitude) direction

		double dx = (reference_points[jmax].xr - reference_points[imax].xr);
		double de = (easting1 - easting0);

		m_ppm_avg = fabs(dx / de);

		m_ExtraInfo = _("---<<< Warning:  Chart georef accuracy may be poor. >>>---");
	} else {
		m_ppm_avg = 1.0; // absolute fallback to prevent div-0 errors
	}

	// Do a last little test using a synthetic ViewPort of nominal size.....
	ViewPort vp;
	vp.set_position(Position(reference_points[0].latr, reference_points[0].lonr));
	vp.view_scale_ppm = m_ppm_avg;
	vp.skew = 0.;
	vp.pix_width = 1000;
	vp.pix_height = 1000;
	SetVPRasterParms(vp);

	double xpl_err_max = 0;
	double ypl_err_max = 0;
	int px, py;

	int pxref, pyref;
	pxref = (int)reference_points[0].xr;
	pyref = (int)reference_points[0].yr;

	for (unsigned int i = 0; i < reference_points.size(); i++) {
		px = (int)(vp.pix_width / 2 + reference_points[i].xr) - pxref;
		py = (int)(vp.pix_height / 2 + reference_points[i].yr) - pyref;

		vp_pix_to_latlong(vp, px, py, &elt, &elg);

		double lat_error = elt - reference_points[i].latr;
		reference_points[i].ypl_error = lat_error;

		double lon_error = elg - reference_points[i].lonr;

		// Longitude errors could be compounded by prior adjustment to ref points
		if (fabs(lon_error) > 180.0) {
			if (lon_error > 0.0)
				lon_error -= 360.0;
			else if (lon_error < 0.0)
				lon_error += 360.0;
		}
		reference_points[i].xpl_error = lon_error;

		if (fabs(reference_points[i].ypl_error) > fabs(ypl_err_max))
			ypl_err_max = reference_points[i].ypl_error;
		if (fabs(reference_points[i].xpl_error) > fabs(xpl_err_max))
			xpl_err_max = reference_points[i].xpl_error;
	}

	Chart_Error_Factor
		= fmax(fabs(xpl_err_max / (lonmax - lonmin)), fabs(ypl_err_max / (latmax - latmin)));
	double chart_error_meters
		= fmax(fabs(xpl_err_max * 60.0 * 1852.0), fabs(ypl_err_max * 60.0 * 1852.0));
	// calculate a nominal pixel error
	// Assume a modern display has about 4000 pixels/meter.
	// Assume the chart is to be displayed at nominal printed scale
	double chart_error_pixels = chart_error_meters * 4000.0 / m_Chart_Scale;

	// Good enough for navigation?
	if (chart_error_pixels > 10) {
		wxLogMessage(_("   VP Final Check: Georeference Chart_Error_Factor on chart ") + m_FullPath
					 + wxString::Format(_T(" is %5g \n     nominal pixel error is: %5g"),
										Chart_Error_Factor, chart_error_pixels));
		m_ExtraInfo = _("---<<< Warning:  Chart georef accuracy is poor. >>>---");
	}

	// Try again with my calculated georef
	// This problem was found on NOAA 514_1.KAP.  The embedded coefficients are just wrong....
	if ((chart_error_pixels > 10) && bHaveEmbeddedGeoref) {
		wxString msg = _("   Trying again with internally calculated georef solution ");
		wxLogMessage(msg);

		bHaveEmbeddedGeoref = false;
		SetVPRasterParms(vp);

		xpl_err_max = 0;
		ypl_err_max = 0;

		pxref = (int)reference_points[0].xr;
		pyref = (int)reference_points[0].yr;

		for (unsigned int i = 0; i < reference_points.size(); i++) {
			px = (int)(vp.pix_width / 2 + reference_points[i].xr) - pxref;
			py = (int)(vp.pix_height / 2 + reference_points[i].yr) - pyref;

			vp_pix_to_latlong(vp, px, py, &elt, &elg);

			double lat_error = elt - reference_points[i].latr;
			reference_points[i].ypl_error = lat_error;

			double lon_error = elg - reference_points[i].lonr;

			// Longitude errors could be compounded by prior adjustment to ref points
			if (fabs(lon_error) > 180.0) {
				if (lon_error > 0.0)
					lon_error -= 360.0;
				else if (lon_error < 0.0)
					lon_error += 360.0;
			}
			reference_points[i].xpl_error = lon_error;

			if (fabs(reference_points[i].ypl_error) > fabs(ypl_err_max))
				ypl_err_max = reference_points[i].ypl_error;
			if (fabs(reference_points[i].xpl_error) > fabs(xpl_err_max))
				xpl_err_max = reference_points[i].xpl_error;
		}

		Chart_Error_Factor
			= fmax(fabs(xpl_err_max / (lonmax - lonmin)), fabs(ypl_err_max / (latmax - latmin)));

		chart_error_meters
			= fmax(fabs(xpl_err_max * 60.0 * 1852.0), fabs(ypl_err_max * 60.0 * 1852.0));
		chart_error_pixels = chart_error_meters * 4000.0 / m_Chart_Scale;

		// Good enough for navigation?
		if (chart_error_pixels > 10) {
			wxLogMessage(_("   VP Final Check with internal georef: Georeference Chart_Error_Factor on chart ")
				+ m_FullPath
				+ wxString::Format(_T(" is %5g\n     nominal pixel error is: %5g"),
					Chart_Error_Factor, chart_error_pixels));
			m_ExtraInfo = _("---<<< Warning:  Chart georef accuracy is poor. >>>---");
		} else {
			wxLogMessage(_("   Result: OK, Internal georef solution used."));
		}
	}

	return 0;
}

/*
*  Extracted from bsb_io.c - implementation of libbsb reading and writing
*
*  Copyright (C) 2000  Stuart Cunningham <stuart_hc@users.sourceforge.net>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
 * generic polynomial to convert georeferenced lat/lon to char's x/y
 *
 * @param coeff list of polynomial coefficients
 * @param lon longitute or x
 * @param lat latitude or y
 *
 * @return coordinate corresponding to the coeff list
 */
static double polytrans(double* coeff, double lon, double lat)
{
	double ret = coeff[0] + coeff[1] * lon + coeff[2] * lat;
	ret += coeff[3] * lon * lon;
	ret += coeff[4] * lon * lat;
	ret += coeff[5] * lat * lat;
	ret += coeff[6] * lon * lon * lon;
	ret += coeff[7] * lon * lon * lat;
	ret += coeff[8] * lon * lat * lat;
	ret += coeff[9] * lat * lat * lat;
	return ret;
}

}

