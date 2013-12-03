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

#ifndef __CHART__CM93CHART__H__
#define __CHART__CM93CHART__H__

#include <chart/S57Chart.h>
#include <chart/M_COVR_Desc.h>
#include <chart/CM93Manager.h>
#include <vector>

class cm93_dictionary;
class covr_set;

namespace geo { class ExtendedGeometry; }

struct header_struct;
struct vector_record_descriptor;
struct Object;
struct Cell_Info_Block;
struct cm93_point;

int Get_CM93_CellIndex(double lat, double lon, int scale);

class cm93chart : public s57chart
{
public:
	cm93chart();
	virtual ~cm93chart();

	cm93chart(int scale_index);
	InitReturn Init(const wxString& name, ChartInitFlag flags);

	void ResetSubcellKey();

	double GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom);
	double GetNormalScaleMax(double canvas_scale_factor);

	bool AdjustVP(ViewPort& vp_last, ViewPort& vp_proposed);
	void SetVPParms(const ViewPort& vpt);
	void GetPointPix(ObjRazRules* rzRules, float northing, float easting, wxPoint* r);
	void GetPointPix(ObjRazRules* rzRules, wxPoint2DDouble* en, wxPoint* r, int nPoints);
	void GetPixPoint(int pixx, int pixy, double* plat, double* plon, ViewPort* vpt);

	void SetCM93Dict(cm93_dictionary* pDict);
	void SetCM93Prefix(const wxString& prefix);
	void SetCM93Manager(chart::cm93manager* pManager);

	bool UpdateCovrSet(ViewPort* vpt);
	bool IsPointInLoadedM_COVR(double xc, double yc);

	covr_set* GetCoverSet() // FIXME: breaks encapsulation
	{
		return m_pcovr_set;
	}

	const wxString& GetLastFileName(void) const;
	std::vector<int> GetVPCellArray(const ViewPort& vpt);

	typedef std::vector<M_COVR_Desc*> CovrDescContainer;
	CovrDescContainer m_pcovr_array_loaded; // FIXME: public attribute

	void SetUserOffsets(int cell_index, int object_id, int subcell, int xoff, int yoff);

	wxString GetScaleChar() const;
	wxPoint* GetDrawBuffer(int nSize);

	OCPNRegion m_render_region; // FIXME: public attribute

private:
	InitReturn CreateHeaderDataFromCM93Cell(void);
	int read_header_and_populate_cib(header_struct* ph, Cell_Info_Block* pCIB);
	geo::ExtendedGeometry* BuildGeom(Object* pobject, wxFileOutputStream* postream, int iobject);

	S57Obj * CreateS57Obj(
			int cell_index,
			int iobject,
			int subcell,
			Object * pobject,
			cm93_dictionary * pDict,
			geo::ExtendedGeometry * xgeom,
			double ref_lat,
			double ref_lon,
			double scale);

	void ProcessMCOVRObjects(int cell_index, char subcell);
	void translate_colmar(const wxString& sclass, S57attVal* pattValTmp);
	int CreateObjChain(int cell_index, int subcell);
	void Unload_CM93_Cell(void);

	// cm93 point manipulation methods
	void Transform(cm93_point* s, double trans_x, double trans_y, double* lat, double* lon);

	int loadcell_in_sequence(int, char);
	int loadsubcell(int, wxChar);
	void ProcessVectorEdges(void);

	wxPoint2DDouble FindM_COVROffset(double lat, double lon);
	M_COVR_Desc* FindM_COVR_InWorkingSet(double lat, double lon);

	Cell_Info_Block* m_CIB;
	cm93_dictionary* m_pDict;
	chart::cm93manager* m_pManager;
	wxString m_prefix;
	double m_sfactor;
	wxString m_scalechar;
	std::vector<int> m_cells_loaded_array;
	int m_current_cell_vearray_offset;
	int* m_pcontour_array;
	int m_ncontour_alloc;
	ViewPort m_vp_current;
	wxChar m_loadcell_key;
	double m_dval;
	covr_set* m_pcovr_set;
	wxPoint* m_pDrawBuffer; // shared outline drawing buffer
	int m_nDrawBufferSize;
	wxString m_LastFileName;
};

#endif
