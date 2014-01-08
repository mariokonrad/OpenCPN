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

#ifndef __CHART__S57CHART_H__
#define __CHART__S57CHART_H__

#include <wx/wx.h>
#include <wx/progdlg.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/file.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include <wx/dynarray.h>

#include <vector>
#include <list>

#include <ogrsf_frmts.h>
#include "iso8211.h"
#include "gdal.h"
#include <ViewPort.h>
#include <OCPNRegion.h>

#include <chart/S57ClassRegistrar.h>
#include <chart/s52s57.h>
#include <chart/S57Sector.h>
#include <chart/ChartBase.h>

class ocpnBitmap;
class PixelCache;
class OGRS57DataSource;
class ocpnDC;
class S57Reader;

namespace chart {

class S57ClassRegistrar;
class ChartBase;

void s57_DrawExtendedLightSectors(ocpnDC& temp_dc, const ViewPort& VPoint,
								  std::vector<s57Sector_t>& sectorlegs);
bool s57_CheckExtendedLightSectors(int mx, int my, const ViewPort& VPoint,
								   std::vector<s57Sector_t>& sectorlegs);

typedef std::list<S57Obj*> ListOfS57Obj;
typedef std::list<ObjRazRules*> ListOfObjRazRules;

WX_DECLARE_HASH_MAP(int, wxString, wxIntegerHash, wxIntegerEqual, MyNatsurHash);
WX_DECLARE_HASH_MAP(int, int, wxIntegerHash, wxIntegerEqual, VectorHelperHash);

WX_DECLARE_HASH_MAP(int, VE_Element*, wxIntegerHash, wxIntegerEqual, VE_Hash);
WX_DECLARE_HASH_MAP(int, VC_Element*, wxIntegerHash, wxIntegerEqual, VC_Hash);

class s57chart : public ChartBase
{
private:
	enum {
		BUILD_SENC_OK,
		BUILD_SENC_NOK_RETRY,
		BUILD_SENC_NOK_PERMANENT
	};

public:
	s57chart();
	virtual ~s57chart();

	virtual InitReturn Init(const wxString& name, ChartInitFlag flags);

	// Accessors

	virtual ThumbData* GetThumbData(int tnx, int tny, float lat, float lon);
	virtual ThumbData* GetThumbData();
	bool UpdateThumbData(double lat, double lon);

	virtual int GetNativeScale() const;
	virtual double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom) const;
	virtual double GetNormalScaleMax(double canvas_scale_factor, int canvas_width) const;

	void SetNativeScale(int s);

	virtual bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
									  const OCPNRegion& Region);
	virtual bool RenderOverlayRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
											 const OCPNRegion& Region);

	virtual void GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion* pValidRegion);

	virtual void GetPointPix(ObjRazRules* rzRules, float rlat, float rlon, wxPoint* r);
	virtual void GetPointPix(ObjRazRules* rzRules, wxPoint2DDouble* en, wxPoint* r, int nPoints);
	virtual void GetPixPoint(int pixx, int pixy, double* plat, double* plon, const ViewPort& vpt);

	virtual void SetVPParms(const ViewPort& vpt);

	virtual bool AdjustVP(const ViewPort& vp_last, ViewPort& vp_proposed);

	virtual double GetNearestPreferredScalePPM(double target_scale_ppm);

	void SetFullExtent(Extent& ext);
	bool GetChartExtent(Extent& ext) const;

	void SetColorScheme(ColorScheme cs, bool bApplyImmediate = true);
	virtual void UpdateLUPs(s57chart* pOwner);

	int _insertRules(S57Obj* obj, LUPrec* LUP, s57chart* pOwner);

	virtual ListOfObjRazRules* GetObjRuleListAtLatLon(float lat, float lon, float select_radius,
													  const ViewPort& VPoint);
	bool DoesLatLonSelectObject(float lat, float lon, float select_radius, S57Obj* obj);
	bool IsPointInObjArea(float lat, float lon, float select_radius, S57Obj* obj);
	wxString GetObjectAttributeValueAsString(S57Obj* obj, int iatt, wxString curAttrName);

	wxString CreateObjDescriptions(ListOfObjRazRules* rule);
	wxString GetAttributeDecode(wxString& att, int ival);

	wxFileName GetSENCFileName();
	void SetSENCFileName(wxFileName fn);

	int BuildRAZFromSENCFile(const wxString& SENCPath);

	int my_fgets(char* buf, int buf_len_max, wxInputStream& ifs);

	// Initialize from an existing SENC file
	bool InitFromSENCMinimal(const wxString& FullPath);

	// DEPCNT VALDCO array access
	bool GetNearestSafeContour(double safe_cnt, double& next_safe_cnt);

	virtual ListOfS57Obj* GetAssociatedObjects(S57Obj* obj);

	virtual VE_Hash& Get_ve_hash(void);
	virtual VC_Hash& Get_vc_hash(void);

	virtual void ForceEdgePriorityEvaluate(void);

	void ClearRenderedTextCache();

//#ifdef ocpnUSE_GL
	virtual bool RenderRegionViewOnGL(const wxGLContext& glc, const ViewPort& VPoint,
									  const OCPNRegion& Region);
	virtual bool RenderOverlayRegionViewOnGL(const wxGLContext& glc, const ViewPort& VPoint,
											 const OCPNRegion& Region);
//#endif

	// Public data
	// Todo Accessors here
	//  Object arrays used by S52PLIB TOPMAR rendering logic
	wxArrayPtrVoid* pFloatingATONArray;
	wxArrayPtrVoid* pRigidATONArray;

	double ref_lat, ref_lon; // Common reference point, derived from FullExtent
	Extent m_FullExtent;
	bool m_bExtentSet;
	bool m_bLinePrioritySet;

	// SM Projection parms, stored as convenience to expedite pixel conversions
	double m_easting_vp_center, m_northing_vp_center;
	double m_pixx_vp_center, m_pixy_vp_center;
	double m_view_scale_ppm;

	// Last ViewPort succesfully rendered, stored as an aid to calculating pixel cache address
	// offsets and regions
	ViewPort m_last_vp;
	OCPNRegion m_last_Region;

	virtual bool IsCacheValid();
	virtual void InvalidateCache();
	virtual bool RenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint);

	virtual void BuildDepthContourArray(void);
	int ValidateAndCountUpdates(const wxFileName file000, const wxString CopyDir,
								wxString& LastUpdateDate, bool b_copyfiles);
	wxString GetISDT(void);

	char GetUsageChar(void);
	static bool IsCellOverlayType(const char* pFullPath);

	bool m_b2pointLUPS;
	bool m_b2lineLUPS;

	struct chart_context* m_this_chart_context;

private:
	bool DoRenderViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, RenderTypeEnum option,
						  bool force_new_view);

	bool DoRenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion& Region,
								bool b_overlay);

	int DCRenderRect(wxMemoryDC& dcinput, const ViewPort& vp, wxRect* rect);
	bool DCRenderLPB(wxMemoryDC& dcinput, const ViewPort& vp, wxRect* rect);

	InitReturn PostInit(ChartInitFlag flags, ColorScheme cs);
	InitReturn FindOrCreateSenc(const wxString& name);
	int BuildSENCFile(const wxString& FullPath000, const wxString& SENCFileName);

	void CreateSENCRecord(OGRFeature* pFeature, FILE* fpOut, int mode, S57Reader* poReader);
	void CreateSENCVectorEdgeTable(FILE* fpOut, S57Reader* poReader);
	void CreateSENCConnNodeTable(FILE* fpOut, S57Reader* poReader);

	void SetLinePriorities(void);

	void GetChartNameFromTXT(const wxString& FullPath, wxString& Name);
	bool BuildThumbnail(const wxString& bmpname);
	bool CreateHeaderDataFromENC(void);
	bool CreateHeaderDataFromSENC(void);
	bool GetBaseFileAttr(wxFileName fn);

	void ResetPointBBoxes(const ViewPort& vp_last, const ViewPort& vp_this);

	// Access to raw ENC DataSet
	bool InitENCMinimal(const wxString& FullPath);
	int GetENCScale();
	OGRFeature* GetChartFirstM_COVR(int& catcov);
	OGRFeature* GetChartNextM_COVR(int& catcov);

	void FreeObjectsAndRules();
	const char* getName(OGRFeature* feature);
	int GetUpdateFileArray(const wxFileName file000, wxArrayString* UpFiles);

//#ifdef ocpnUSE_GL
	bool DoRenderRectOnGL(const wxGLContext& glc, const ViewPort& VPoint, wxRect& rect);
	bool DoRenderRegionViewOnGL(const wxGLContext& glc, const ViewPort& VPoint,
								const OCPNRegion& Region, bool b_overlay);
	void SetClipRegionGL(const wxGLContext& glc, const ViewPort& VPoint, const wxRect& Rect,
						 bool b_render_nodta = true);
	void SetClipRegionGL(const wxGLContext& glc, const ViewPort& VPoint, const OCPNRegion& Region,
						 bool b_render_nodta = true);
//#endif

	wxString* m_pcsv_locn;

	char* hdr_buf;
	char* mybuf_ptr;
	int hdr_len;
	wxFileName m_SENCFileName;
	ObjRazRules* razRules[PRIO_NUM][LUPNAME_NUM];

	wxArrayString* m_tmpup_array;
	PixelCache* pDIB;

	wxBitmap* m_pCloneBM;
	wxMask* m_pMask;

	bool bGLUWarningSent;

	wxBitmap* m_pDIBThumbDay;
	wxBitmap* m_pDIBThumbDim;
	wxBitmap* m_pDIBThumbOrphan;
	bool m_bneed_new_thumbnail;

	bool m_bbase_file_attr_known;
	wxDateTime m_date000; // extracted from DSID:ISDT
	wxString m_edtn000; // extracted from DSID:EDTN
	int m_nGeoRecords; // extracted from DSSI:NOGR
	int m_native_scale; // extracted from DSPM:CSCL

	// Raw ENC DataSet members
	OGRS57DataSource* m_pENCDS;

	// DEPCNT VALDCO array members
	int m_nvaldco;
	int m_nvaldco_alloc;
	double* m_pvaldco_array;

	VectorHelperHash m_vector_helper_hash;

	VE_Hash m_ve_hash;
	VC_Hash m_vc_hash;

	MyNatsurHash m_natsur_hash; // hash table for cacheing NATSUR string values from int attributes

	bool m_blastS57TextRender;
	wxString m_lastColorScheme;
	wxRect m_last_vprect;
	long m_plib_state_hash;
	bool m_btex_mem;
	char m_usage_char;
};

}

#endif
