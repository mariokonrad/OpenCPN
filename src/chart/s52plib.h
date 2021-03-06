/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S52 Presentation Library
 * Author:   David Register
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

#ifndef _S52PLIB_H_
#define _S52PLIB_H_

#include "s52s57.h"

class wxGLContext;
#ifdef ocpnUSE_GL
	#include <wx/glcanvas.h>
#endif

#include <wx/dcgraph.h>
#include <wx/hashmap.h>

#include <vector>
#include <list>

class ViewPort;
class PixelCache;
class wxPoint2DDouble;

namespace chart {

class RuleHash;

WX_DECLARE_HASH_MAP(wxString, Rule*, wxStringHash, wxStringEqual, RuleHash);
WX_DECLARE_HASH_MAP(int, wxString, wxIntegerHash, wxIntegerEqual, MyNatsurHash);
WX_DEFINE_SORTED_ARRAY(LUPrec*, wxArrayOfLUPrec);
WX_DECLARE_STRING_HASH_MAP(int, CARC_Hash);


class RenderFromHPGL;
class TexFont;

//-----------------------------------------------------------------------------
//      LUP Array container, and friends
//-----------------------------------------------------------------------------
struct LUPHashIndex
{
	int n_start;
	int count;
};

WX_DECLARE_STRING_HASH_MAP(LUPHashIndex*, LUPArrayIndexHash);

class LUPArrayContainer // FIXME: separate
{
public:
	LUPArrayContainer();
	~LUPArrayContainer();

	wxArrayOfLUPrec* GetLUPArray(void)
	{
		return LUPArray;
	}

	LUPHashIndex* GetArrayIndexHelper(const char* objectName);

private:
	wxArrayOfLUPrec* LUPArray; // Sorted Array
	LUPArrayIndexHash IndexHash;
};

//-----------------------------------------------------------------------------
//    s52plib definition
//-----------------------------------------------------------------------------

class s52plib
{
public:
	s52plib(const wxString& PLib, bool b_forceLegacy = false);
	~s52plib();

	void SetPPMM(float ppmm);
	float GetPPMM() const;

	LUPrec* S52_LUPLookup(LUPname LUP_name, const char* objectName, S57Obj* pObj, bool bStrict = 0);
	int _LUP2rules(LUPrec* LUP, S57Obj* pObj);
	S52color* getColor(const char* colorName);
	wxColour getwxColour(const wxString& colorName);

	void UpdateMarinerParams(void);
	void ClearCNSYLUPArray(void);

	void GenerateStateHash();
	long GetStateHash() const;

	void SetPLIBColorScheme(wxString scheme);
	wxString GetPLIBColorScheme(void);
	void SetGLRendererString(const wxString& renderer);

	bool ObjectRenderCheck(ObjRazRules* rzRules, const ViewPort& vp);
	bool ObjectRenderCheckPos(ObjRazRules* rzRules, const ViewPort& vp);
	bool ObjectRenderCheckCat(ObjRazRules* rzRules, const ViewPort& vp);
	bool ObjectRenderCheckCS(ObjRazRules* rzRules, const ViewPort& vp);

	static void DestroyLUP(LUPrec* pLUP);
	static void ClearRulesCache(Rule* pR);

	// Temporarily save/restore the current colortable index
	// Useful for Thumbnail rendering
	void SaveColorScheme(void);
	void RestoreColorScheme(void);

	// Rendering stuff
	void PrepareForRender(void);
	void AdjustTextList(int dx, int dy, int screenw, int screenh);
	void ClearTextList(void);
	int SetLineFeaturePriority(ObjRazRules* rzRules, int npriority);
	void FlushSymbolCaches();

	// For DC's
	int RenderObjectToDC(wxDC* pdc, ObjRazRules* rzRules, const ViewPort& vp);
	int RenderAreaToDC(wxDC* pdc, ObjRazRules* rzRules, const ViewPort& vp,
					   render_canvas_parms* pb_spec);

	// Accessors
	bool GetShowSoundings() const;
	void SetShowSoundings(bool f);
	bool GetShowS57Text() const;
	void SetShowS57Text(bool f);
	bool GetShowS57ImportantTextOnly() const;
	void SetShowS57ImportantTextOnly(bool f);
	int GetMajorVersion(void) const;
	int GetMinorVersion(void) const;
	void SetTextOverlapAvoid(bool f);
	void SetShowNationalText(bool f);
	void SetShowAtonText(bool f);
	void SetShowLdisText(bool f);
	void SetExtendLightSectors(bool f);

	wxArrayOfLUPrec* SelectLUPARRAY(LUPname TNAM);
	LUPArrayContainer* SelectLUPArrayContainer(LUPname TNAM);

	void DestroyPatternRuleNode(Rule* pR);
	void DestroyRuleNode(Rule* pR);

//#ifdef ocpnUSE_GL
	// For OpenGL
	int RenderObjectToGL(const wxGLContext& glcc, ObjRazRules* rzRules, const ViewPort& vp,
						 wxRect& render_rect);
	int RenderAreaToGL(const wxGLContext& glcc, ObjRazRules* rzRules, const ViewPort& vp,
					   wxRect& render_rect);
//#endif

	// Todo accessors
	DisCat m_nDisplayCategory;
	LUPname m_nSymbolStyle;
	LUPname m_nBoundaryStyle;
	bool m_bOK;

	bool m_bShowSoundg;
	bool m_bShowMeta;
	bool m_bShowS57Text;
	bool m_bUseSCAMIN;
	bool m_bShowAtonText;
	bool m_bShowLdisText;
	bool m_bExtendLightSectors;
	bool m_bShowS57ImportantTextOnly;
	bool m_bDeClutterText;
	bool m_bShowNationalTexts;

	int m_VersionMajor;
	int m_VersionMinor;

	int m_nDepthUnitDisplay;

	// Library data
	wxArrayPtrVoid* pAlloc;

	RuleHash* _line_sym; // line symbolisation rules
	RuleHash* _patt_sym; // pattern symbolisation rules
	RuleHash* _cond_sym; // conditional symbolisation rules
	RuleHash* _symb_symR; // symbol symbolisation rules, Raster

	LUPArrayContainer* line_LAC;
	LUPArrayContainer* areaPlain_LAC;
	LUPArrayContainer* areaSymbol_LAC;
	LUPArrayContainer* pointSimple_LAC;
	LUPArrayContainer* pointPaper_LAC;

	wxArrayOfLUPrec* condSymbolLUPArray; // Dynamic Conditional Symbology

	typedef std::vector<OBJLElement*> OBJLContainer;
	OBJLContainer OBJLArray; // Used for Display Filtering

	std::vector<wxString> OBJLDescriptions;

	RuleHash* _symb_sym; // symbol symbolisation rules
	MyNatsurHash m_natsur_hash; // hash table for cacheing NATSUR string values from int attributes

private:
	int S52_load_Plib(const wxString& PLib, bool b_forceLegacy);
	bool S52_flush_Plib();

	bool PreloadOBJLFromCSV(const wxString& csv_file);

	int DoRenderObject(wxDC* pdcin, ObjRazRules* rzRules, const ViewPort& vp);

	// Area Renderers
	int RenderToBufferAC(ObjRazRules* rzRules, Rules* rules, const ViewPort& vp,
						 render_canvas_parms* pb_spec);
	int RenderToBufferAP(ObjRazRules* rzRules, Rules* rules, const ViewPort& vp,
						 render_canvas_parms* pb_spec);
	int RenderToGLAC(ObjRazRules* rzRules, Rules* rules, const ViewPort& vp);
	int RenderToGLAP(ObjRazRules* rzRules, Rules* rules, const ViewPort& vp);

	// Object Renderers
	int RenderTX(ObjRazRules* rzRules, Rules* rules, const ViewPort& vp);
	int RenderTE(ObjRazRules* rzRules, Rules* rules, const ViewPort& vp);
	int RenderSY(ObjRazRules* rzRules, Rules* rules, const ViewPort& vp);
	int RenderLS(ObjRazRules* rzRules, Rules* rules, const ViewPort& vp);
	int RenderLC(ObjRazRules* rzRules, Rules* rules, const ViewPort& vp);
	int RenderMPS(ObjRazRules* rzRules, Rules* rules, const ViewPort& vp);
	int RenderCARC(ObjRazRules* rzRules, Rules* rules, const ViewPort& vp);
	char* RenderCS(ObjRazRules* rzRules, Rules* rules);

	void UpdateOBJLArray(S57Obj* obj);

	render_canvas_parms* CreatePatternBufferSpec(ObjRazRules* rzRules, Rules* rules,
												 const ViewPort& vp, bool b_revrgb,
												 bool b_pot = false);

	void RenderToBufferFilledPolygon(ObjRazRules* rzRules, S57Obj* obj, S52color* c,
									 const geo::BoundingBox& BBView, render_canvas_parms* pb_spec,
									 render_canvas_parms* patt_spec, const ViewPort& vp);

	void draw_lc_poly(wxDC* pdc, wxColor& color, int width, wxPoint* ptp, int npt, float sym_len,
					  float sym_factor, Rule* draw_rule, const ViewPort& vp);

	bool RenderHPGL(ObjRazRules* rzRules, Rule* rule_in, wxPoint& r, const ViewPort& vp,
					float rot_angle = 0.);
	bool RenderRasterSymbol(ObjRazRules* rzRules, Rule* prule, wxPoint& r, const ViewPort& vp,
							float rot_angle = 0.);
	wxImage RuleXBMToImage(Rule* prule);

	bool RenderText(wxDC* pdc, S52_TextC* ptext, int x, int y, wxRect* pRectDrawn, S57Obj* pobj,
					bool bCheckOverlap, const ViewPort& vp);

	bool CheckTextRectList(const wxRect& test_rect, S52_TextC* ptext);
	int RenderT_All(ObjRazRules* rzRules, Rules* rules, const ViewPort& vp, bool bTX);

	int PrioritizeLineFeature(ObjRazRules* rzRules, int npriority);

	int dda_tri(wxPoint* ptp, S52color* c, render_canvas_parms* pb_spec,
				render_canvas_parms* pPatt_spec);
	int dda_trap(wxPoint* segs, int lseg, int rseg, int ytop, int ybot, S52color* c,
				 render_canvas_parms* pb_spec, render_canvas_parms* pPatt_spec);

	LUPrec* FindBestLUP(wxArrayOfLUPrec* LUPArray, unsigned int startIndex, unsigned int count,
						S57Obj* pObj, bool bStrict);

	Rules* StringToRules(const wxString& str_in);
	void GetAndAddCSRules(ObjRazRules* rzRules, Rules* rules);

	void DestroyPattRules(RuleHash* rh);
	void DestroyRules(RuleHash* rh);
	void DestroyLUPArray(wxArrayOfLUPrec* pLUPArray);

	bool TextRenderCheck(ObjRazRules* rzRules);
	bool inter_tri_rect(wxPoint* ptp, render_canvas_parms* pb_spec);

	bool GetPointPixArray(ObjRazRules* rzRules, wxPoint2DDouble* pd, wxPoint* pp, int nv,
						  const ViewPort& vp);
	bool GetPointPixSingle(ObjRazRules* rzRules, float north, float east, wxPoint* r,
						   const ViewPort& vp);
	void GetPixPointSingle(int pixx, int pixy, double* plat, double* plon, const ViewPort& vp);

	wxString m_plib_file;

	float canvas_pix_per_mm; // Set by parent, used to scale symbols/lines/patterns

	S52color m_unused_color;
	wxColor m_unused_wxColor;

	bool bUseRasterSym;
	bool useLegacyRaster;

	wxDC* m_pdc; // The current DC

//#ifdef ocpnUSE_GL
	wxGLContext* m_glcc;
//#endif

	int* ledge;
	int* redge;

	int m_colortable_index;
	int m_colortable_index_save;

	typedef std::list<S52_TextC*> TextObjList;
	TextObjList m_textObjList;

	double m_display_pix_per_mm;

	wxString m_ColorScheme;

	long m_state_hash;

	wxRect m_render_rect;

	bool m_txf_ready;
	TexFont* m_txf;
	int m_txf_avg_char_width;
	int m_txf_avg_char_height;
	CARC_Hash m_CARC_hashmap;
	RenderFromHPGL* HPGL;
};

}

#endif
