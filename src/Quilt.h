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

#ifndef __QUIT__H__
#define __QUIT__H__

#include <OCPNRegion.h>
#include <QuiltPatch.h>
#include <QuiltCandidate.h>
#include <ViewPort.h>
#include <vector>
#include <list>
#include <wx/dynarray.h>

namespace chart {
	struct ChartTableEntry;
	class ChartBase;
}

class wxMemoryDC;

WX_DEFINE_SORTED_ARRAY(QuiltCandidate *, ArrayOfSortedQuiltCandidates);

class Quilt
{
public:
	Quilt();
	~Quilt();

	void SetQuiltParameters(double CanvasScaleFactor, int CanvasWidth);
	bool BuildExtendedChartStackAndCandidateArray(bool b_fullscreen, int ref_db_index,
												  const ViewPort& vp_in);
	bool Compose(const ViewPort& vp);
	bool IsComposed() const;

	chart::ChartBase* GetFirstChart();
	chart::ChartBase* GetNextChart();
	chart::ChartBase* GetLargestScaleChart();
	std::vector<int> GetQuiltIndexArray(void);
	bool IsQuiltDelta(const ViewPort& vp);
	bool IsChartQuiltableRef(int db_index);

	ViewPort& GetQuiltVP();
	wxString GetQuiltDepthUnit() const;
	void SetRenderedVP(const ViewPort& vp);
	bool HasOverlays(void) const;
	int GetExtendedStackCount(void) const;
	int GetnCharts() const;

	void ComputeRenderRegion(const ViewPort& vp, OCPNRegion& chart_region);
	bool RenderQuiltRegionViewOnDC(wxMemoryDC& dc, const ViewPort& vp, OCPNRegion& chart_region);
	bool IsVPBlittable(const ViewPort& VPoint, int dx, int dy, bool b_allow_vector = false);
	chart::ChartBase* GetChartAtPix(wxPoint p);
	chart::ChartBase* GetOverlayChartAtPix(wxPoint p);
	int GetChartdbIndexAtPix(wxPoint p);
	void InvalidateAllQuiltPatchs(void);

	void Invalidate(void);
	void AdjustQuiltVP(ViewPort& vp_last, ViewPort& vp_proposed);

	OCPNRegion& GetFullQuiltRegion(void);
	OCPNRegion& GetFullQuiltRenderedRegion(void);
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

	std::vector<int> GetCandidatedbIndexArray(bool from_ref_chart, bool exclude_user_hidden);

	std::vector<int> GetExtendedStackIndexArray();
	std::vector<int> GetEclipsedStackIndexArray();
	unsigned long GetXStackHash() const;
	bool IsBusy() const;

	QuiltPatch* GetCurrentPatch();
	bool IsChartInQuilt(chart::ChartBase* pc);
	bool IsQuiltVector(void);
	OCPNRegion GetHiliteRegion(const ViewPort& vp);

private:
	typedef std::list<QuiltPatch*> PatchList;

	OCPNRegion GetChartQuiltRegion(const chart::ChartTableEntry& cte, const ViewPort& vp);
	wxRect GetChartQuiltBoundingRect(const chart::ChartTableEntry& cte, const ViewPort& vp);
	void EmptyCandidateArray(void);
	void SubstituteClearDC(wxMemoryDC& dc, const ViewPort& vp);
	int GetNewRefChart(void);
	unsigned int get_target_stack_index(int current_db_index) const;
	void destroy_patchlist();

	OCPNRegion m_covered_region;
	OCPNRegion m_rendered_region;

	PatchList m_PatchList;
	wxBitmap* m_pBM;

	bool m_bcomposed;
	PatchList::iterator current_node;
	bool m_bbusy; // FIXME: poor man's mutex to sync access to current_node, the interface to access
				  // current_node is crap...
	int m_quilt_proj;

	ArrayOfSortedQuiltCandidates* m_pcandidate_array;
	std::vector<int> m_last_index_array;
	std::vector<int> m_index_array;
	std::vector<int> m_extended_stack_array;
	std::vector<int> m_eclipsed_stack_array;

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
	int m_lost_refchart_dbIndex;
};

#endif
