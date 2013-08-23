/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __QUIT_H__
#define __QUIT_H__

#include "chart1.h"
#include "OCPNRegion.h"
#include "QuiltPatch.h"
#include "QuiltCandidate.h"
#include "ViewPort.h"

struct ChartTableEntry;

WX_DECLARE_LIST(QuiltPatch, PatchList);
WX_DEFINE_SORTED_ARRAY(QuiltCandidate *, ArrayOfSortedQuiltCandidates);

class Quilt
{
	public:
		Quilt();
		~Quilt();

		void SetQuiltParameters(double CanvasScaleFactor, int CanvasWidth);
		bool BuildExtendedChartStackAndCandidateArray(bool b_fullscreen, int ref_db_index, ViewPort & vp_in);
		bool Compose(const ViewPort & vp);
		bool IsComposed() const;

		ChartBase * GetFirstChart();
		ChartBase * GetNextChart();
		ChartBase * GetLargestScaleChart();
		ArrayOfInts GetQuiltIndexArray(void);
		bool IsQuiltDelta(ViewPort & vp);
		bool IsChartQuiltableRef(int db_index);

		ViewPort & GetQuiltVP();
		wxString GetQuiltDepthUnit() const;
		void SetRenderedVP(ViewPort & vp);
		bool HasOverlays(void) const;
		int GetExtendedStackCount(void) const;
		int GetnCharts() const;

		void ComputeRenderRegion(ViewPort & vp, OCPNRegion & chart_region);
		bool RenderQuiltRegionViewOnDC(wxMemoryDC & dc, ViewPort & vp, OCPNRegion & chart_region);
		bool IsVPBlittable(ViewPort &VPoint, int dx, int dy, bool b_allow_vector = false);
		ChartBase * GetChartAtPix(wxPoint p);
		ChartBase * GetOverlayChartAtPix(wxPoint p);
		int GetChartdbIndexAtPix(wxPoint p);
		void InvalidateAllQuiltPatchs(void);

		void Invalidate(void);
		void AdjustQuiltVP(ViewPort & vp_last, ViewPort & vp_proposed);

		OCPNRegion & GetFullQuiltRegion(void);
		OCPNRegion & GetFullQuiltRenderedRegion(void);
		bool IsChartSmallestScale(int dbIndex);

		int AdjustRefOnZoomOut(double proposed_scale_onscreen);
		int AdjustRefOnZoomIn(double proposed_scale_onscreen);

		void SetHiliteIndex(int index);
		void SetReferenceChart(int dbIndex);
		int GetRefChartdbIndex(void) const;
		int GetQuiltProj(void) const;
		double GetMaxErrorFactor() const;
		double GetRefScale() const;
		double GetRefNativeScale();

		ArrayOfInts GetCandidatedbIndexArray(bool from_ref_chart, bool exclude_user_hidden);

		ArrayOfInts GetExtendedStackIndexArray();
		ArrayOfInts GetEclipsedStackIndexArray();
		unsigned long GetXStackHash() const;
		bool IsBusy() const;

		QuiltPatch *GetCurrentPatch();
		bool IsChartInQuilt(ChartBase * pc);
		bool IsQuiltVector(void);
		OCPNRegion GetHiliteRegion(ViewPort & vp);

	private:
		OCPNRegion GetChartQuiltRegion(const ChartTableEntry & cte, ViewPort & vp);
		void EmptyCandidateArray(void);
		void SubstituteClearDC(wxMemoryDC & dc, ViewPort & vp);
		int GetNewRefChart(void);

		OCPNRegion m_covered_region;
		OCPNRegion m_rendered_region;

		PatchList m_PatchList;
		wxBitmap *m_pBM;

		bool m_bcomposed;
		wxPatchListNode *cnode;
		bool m_bbusy;
		int m_quilt_proj;

		ArrayOfSortedQuiltCandidates *m_pcandidate_array;
		ArrayOfInts m_last_index_array;
		ArrayOfInts m_index_array;
		ArrayOfInts m_extended_stack_array;
		ArrayOfInts m_eclipsed_stack_array;

		ViewPort m_vp_quilt;
		ViewPort m_vp_rendered; // last VP rendered

		int m_nHiLiteIndex;
		int m_refchart_dbIndex;
		int m_reference_scale;
		int m_reference_type;
		int m_reference_family;
		bool m_bneed_clear;
		OCPNRegion m_back_region;
		wxString m_quilt_depth_unit;
		double m_max_error_factor;
		double m_canvas_scale_factor;
		int m_canvas_width;
		bool m_bquilt_has_overlays;
		unsigned long m_xa_hash;
		int m_zout_dbindex;
};

#endif
