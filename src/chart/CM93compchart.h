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

#ifndef __CHART__CM93COMPCHART__H__
#define __CHART__CM93COMPCHART__H__

#include <chart/S57Chart.h>
#include <chart/s52s57.h>

#include <wx/string.h>

class CM93OffsetDialog;
class OCPNRegion;
class cm93_dictionary;
class cm93manager;
class cm93chart;

class cm93compchart : public s57chart
{
public:
	cm93compchart();
	virtual ~cm93compchart();

	InitReturn Init(const wxString& name, ChartInitFlag flags);

	void Activate(void);
	void Deactivate(void);

	double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);
	double GetNormalScaleMax(double canvas_scale_factor, int canvas_width);
	int GetNativeScale(void);

	wxString GetPubDate();

	void SetVPParms(const ViewPort& vpt);
	void GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion* pValidRegion);

	ThumbData* GetThumbData(int tnx, int tny, float lat, float lon);
	ThumbData* GetThumbData()
	{
		return (ThumbData*)NULL;
	}

	bool AdjustVP(ViewPort& vp_last, ViewPort& vp_proposed);
	bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion& Region);
	virtual bool RenderRegionViewOnGL(const wxGLContext& glc, const ViewPort& VPoint,
									  const OCPNRegion& Region);
	void SetColorScheme(ColorScheme cs, bool bApplyImmediate);
	bool RenderNextSmallerCellOutlines(ocpnDC& dc, ViewPort& vp);
	void GetPointPix(ObjRazRules* rzRules, float rlat, float rlon, wxPoint* r);
	void GetPixPoint(int pixx, int pixy, double* plat, double* plon, ViewPort* vpt);
	void GetPointPix(ObjRazRules* rzRules, wxPoint2DDouble* en, wxPoint* r, int nPoints);
	ListOfObjRazRules* GetObjRuleListAtLatLon(float lat, float lon, float select_radius,
											  ViewPort* VPoint);

	VE_Hash& Get_ve_hash(void);
	VC_Hash& Get_vc_hash(void);

	void UpdateLUPs(s57chart* pOwner);
	void ForceEdgePriorityEvaluate(void);
	ListOfS57Obj* GetAssociatedObjects(S57Obj* obj);

	cm93chart* GetCurrentSingleScaleChart()
	{
		return m_pcm93chart_current;
	}

	void SetSpecialOutlineCellIndex(int cell_index, int object_id, int subcell)
	{
		m_cell_index_special_outline = cell_index;
		m_object_id_special_outline = object_id;
		m_subcell_special_outline = subcell;
	}

	void SetSpecialCellIndexOffset(int cell_index, int object_id, int subcell, int xoff, int yoff);
	void CloseandReopenCurrentSubchart(void);

	void SetOffsetDialog(CM93OffsetDialog* dialog)
	{
		m_pOffsetDialog = dialog;
	}

	void InvalidateCache();

private:
	void UpdateRenderRegions(const ViewPort& VPoint);
	OCPNRegion GetValidScreenCanvasRegion(const ViewPort& VPoint, const OCPNRegion& ScreenRegion);
	bool RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint);

	InitReturn CreateHeaderData();
	cm93_dictionary* FindAndLoadDictFromDir(const wxString& dir);
	void FillScaleArray(double lat, double lon);
	int PrepareChartScale(const ViewPort& vpt, int cmscale);
	int GetCMScaleFromVP(const ViewPort& vpt);
	bool DoRenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion& Region);

	bool DoRenderRegionViewOnGL(const wxGLContext& glc, const ViewPort& VPoint,
								const OCPNRegion& Region);

	// Data members

	cm93_dictionary* m_pDictComposite;
	cm93manager* m_pcm93mgr;

	cm93chart* m_pcm93chart_array[8];
	bool m_bScale_Array[8];
	cm93chart* m_pcm93chart_current;
	int m_cmscale;

	wxString m_prefixComposite;

	int m_current_cell_pub_date; // the (integer) publish date of the cell at the current VP

	wxBitmap* m_pDummyBM;
	int m_cell_index_special_outline;
	int m_object_id_special_outline;
	int m_subcell_special_outline;
	int m_special_offset_x;
	int m_special_offset_y;
	ViewPort m_vpt;

	CM93OffsetDialog* m_pOffsetDialog;
};

#endif
