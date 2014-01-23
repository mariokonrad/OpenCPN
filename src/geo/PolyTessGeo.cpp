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

#include "PolyTessGeo.h"

#include <MicrosoftCompatibility.h>
#include <dychart.h>

#include <geo/BoundingBox.h>
#include <geo/GeoRef.h>
#include <geo/TriPrim.h>
#include <geo/PolyTriGroup.h>
#include <geo/ExtendedGeometry.h>
#include <geo/triangulate.h>

#include <chart/s52s57.h>

#include <wx/mstream.h>
#include <wx/tokenzr.h>

#define TESS_VERT   0  // constants describing preferred tess orientation
#define TESS_HORZ   1

#define EQUAL_EPS 1.0e-7   // tolerance value

using chart::pt;



#ifdef USE_GLU_TESS
static int s_nvcall;
static int s_nvmax;
static double* s_pwork_buf;
static int s_buf_len;
static int s_buf_idx;
static geo::TriPrim::Type s_gltri_type;
geo::TriPrim* s_pTPG_Head;
geo::TriPrim* s_pTPG_Last;
static GLUtesselator* GLUtessobj;
static double s_ref_lat;
static double s_ref_lon;
static bool s_bSENC_SM;

static bool s_bmerc_transform;
static double s_transform_x_rate;
static double s_transform_x_origin;
static double s_transform_y_rate;
static double s_transform_y_origin;
wxArrayPtrVoid* s_pCombineVertexArray;

static const double CM93_semimajor_axis_meters = 6378388.0;
#endif

// For __WXMSW__ builds using GLU_TESS and glu32.dll
// establish the dll entry points
#ifdef __WXMSW__
#ifdef USE_GLU_TESS
#ifdef USE_GLU_DLL

//  Formal definitions of required functions
typedef void (CALLBACK* LPFNDLLTESSPROPERTY)      ( GLUtesselator *tess,
                                                    GLenum        which,
                                                    GLdouble      value );
typedef GLUtesselator * (CALLBACK* LPFNDLLNEWTESS)( void);
typedef void (CALLBACK* LPFNDLLTESSBEGINCONTOUR)  ( GLUtesselator *);
typedef void (CALLBACK* LPFNDLLTESSENDCONTOUR)    ( GLUtesselator *);
typedef void (CALLBACK* LPFNDLLTESSBEGINPOLYGON)  ( GLUtesselator *, void*);
typedef void (CALLBACK* LPFNDLLTESSENDPOLYGON)    ( GLUtesselator *);
typedef void (CALLBACK* LPFNDLLDELETETESS)        ( GLUtesselator *);
typedef void (CALLBACK* LPFNDLLTESSVERTEX)        ( GLUtesselator *, GLdouble *, GLdouble *);
typedef void (CALLBACK* LPFNDLLTESSCALLBACK)      ( GLUtesselator *, const int, void (CALLBACK *fn)() );

//  Static pointers to the functions
static LPFNDLLTESSPROPERTY      s_lpfnTessProperty;
static LPFNDLLNEWTESS           s_lpfnNewTess;
static LPFNDLLTESSBEGINCONTOUR  s_lpfnTessBeginContour;
static LPFNDLLTESSENDCONTOUR    s_lpfnTessEndContour;
static LPFNDLLTESSBEGINPOLYGON  s_lpfnTessBeginPolygon;
static LPFNDLLTESSENDPOLYGON    s_lpfnTessEndPolygon;
static LPFNDLLDELETETESS        s_lpfnDeleteTess;
static LPFNDLLTESSVERTEX        s_lpfnTessVertex;
static LPFNDLLTESSCALLBACK      s_lpfnTessCallback;

//  Mapping of pointers to glu functions by substitute macro
#define gluTessProperty         s_lpfnTessProperty
#define gluNewTess              s_lpfnNewTess
#define gluTessBeginContour     s_lpfnTessBeginContour
#define gluTessEndContour       s_lpfnTessEndContour
#define gluTessBeginPolygon     s_lpfnTessBeginPolygon
#define gluTessEndPolygon       s_lpfnTessEndPolygon
#define gluDeleteTess           s_lpfnDeleteTess
#define gluTessVertex           s_lpfnTessVertex
#define gluTessCallback         s_lpfnTessCallback

#endif
#endif

//  Flag to tell that dll is ready
bool s_glu_dll_ready = false;
HINSTANCE s_hGLU_DLL; // Handle to DLL

#endif

namespace geo {

static int tess_orient;

static void destroy_combined_vertices()
{
	// Free up any "Combine" vertices created
	for (unsigned int i = 0; i < s_pCombineVertexArray->size(); ++i)
		free(s_pCombineVertexArray->Item(i));
	delete s_pCombineVertexArray;
}

static bool ispolysame(polyout* p1, polyout* p2)
{
	int i2;

	if (p1->nvert != p2->nvert)
		return false;

	int v10 = p1->vertex_index_list[0];

	for (i2 = 0; i2 < p2->nvert; i2++) {
		if (p2->vertex_index_list[i2] == v10)
			break;
	}
	if (i2 == p2->nvert)
		return false;

	for (int j = 0; j < p1->nvert; j++) {
		if (p1->vertex_index_list[j] != p2->vertex_index_list[i2])
			return false;
		i2++;
		if (i2 == p2->nvert)
			i2 = 0;
	}

	return true;
}

PolyTessGeo::PolyTessGeo()
{
}

// Build PolyTessGeo Object from ExtendedGeometry
PolyTessGeo::PolyTessGeo(ExtendedGeometry* pxGeom)
{
	m_ppg_head = NULL;
	m_bOK = false;

	m_pxgeom = pxGeom;
}

// Build PolyTessGeo Object from OGR Polygon
PolyTessGeo::PolyTessGeo(OGRPolygon* poly, bool bSENC_SM, double ref_lat, double ref_lon,
						 bool bUseInternalTess)
{
	ErrorCode = 0;
	m_ppg_head = NULL;
	m_pxgeom = NULL;

	m_ref_lat = ref_lat;
	m_ref_lon = ref_lon;

	if (bUseInternalTess) {
		ErrorCode = PolyTessGeoTri(poly, bSENC_SM, ref_lat, ref_lon);
	} else {
#ifdef USE_GLU_TESS
		ErrorCode = PolyTessGeoGL(poly, bSENC_SM, ref_lat, ref_lon);
#else
		ErrorCode = PolyTessGeoTri(poly, bSENC_SM, ref_lat, ref_lon);
#endif
	}
}

// Build PolyGeo Object from SENC file record
PolyTessGeo::PolyTessGeo(unsigned char* polybuf, int nrecl, int WXUNUSED(index))
{
#define POLY_LINE_HDR_MAX 1000
	// Todo Add a try/catch set here, in case SENC file is corrupted??

	m_pxgeom = NULL;

	char hdr_buf[POLY_LINE_HDR_MAX];
	int twkb_len;

	m_buf_head = (char*)polybuf; // buffer beginning
	m_buf_ptr = m_buf_head;
	m_nrecl = nrecl;

	my_bufgets(hdr_buf, POLY_LINE_HDR_MAX);
	//  Read the s57obj extents as lat/lon
	sscanf(hdr_buf, "  POLYTESSGEOPROP %lf %lf %lf %lf", &xmin, &ymin, &xmax, &ymax);

	PolyTriGroup* ppg = new PolyTriGroup;
	ppg->m_bSMSENC = true;

	int nctr;
	my_bufgets(hdr_buf, POLY_LINE_HDR_MAX);
	sscanf(hdr_buf, "Contours/nWKB %d %d", &nctr, &twkb_len);
	ppg->nContours = nctr;
	ppg->pn_vertex = (int*)malloc(nctr * sizeof(int));
	int* pctr = ppg->pn_vertex;

	char* buf = (char*)malloc(twkb_len + 2); // allocate a buffer guaranteed big enough

	my_bufgets(buf, twkb_len + 2); // contour nVert, plus geometry

	wxString ivc_str(buf + 10, wxConvUTF8);
	wxStringTokenizer tkc(ivc_str, wxT(" ,\n"));
	long icv = 0;

	while (tkc.HasMoreTokens()) {
		wxString token = tkc.GetNextToken();
		if (token.ToLong(&icv)) {
			if (icv) {
				*pctr = icv;
				pctr++;
			}
		}
	}

	//  Read Raw Geometry

	float* ppolygeo = (float*)malloc(twkb_len + 1); // allow for crlf
	memmove(ppolygeo, m_buf_ptr, twkb_len + 1);
	m_buf_ptr += twkb_len + 1;
	ppg->pgroup_geom = ppolygeo;

	geo::TriPrim** p_prev_triprim = &(ppg->tri_prim_head);

	// Read the PTG_Triangle Geometry in a loop
	geo::TriPrim::Type tri_type;
	int nvert;
	int nvert_max = 0;
	bool not_finished = true;
	while (not_finished) {
		if ((m_buf_ptr - m_buf_head) != m_nrecl) {
			int* pi = (int*)m_buf_ptr;
			tri_type = static_cast<geo::TriPrim::Type>(*pi++);
			nvert = *pi;
			m_buf_ptr += 2 * sizeof(int);

			// Here is the usual stop condition, which results from
			// interpreting the string "POLYEND" as an int
			if (tri_type == 0x594c4f50) {
				not_finished = false;
				break;
			}

			geo::TriPrim* tp = new geo::TriPrim;
			*p_prev_triprim = tp; // make the link
			p_prev_triprim = &(tp->p_next);
			tp->p_next = NULL;

			tp->type = tri_type;
			tp->nVert = nvert;

			if (nvert > nvert_max) // Keep a running tab of largest vertex count
				nvert_max = nvert;

			int byte_size = nvert * 2 * sizeof(double);

			tp->p_vertex = (double*)malloc(byte_size);
			memmove(tp->p_vertex, m_buf_ptr, byte_size);
			m_buf_ptr += byte_size;

			// Read the triangle primitive bounding box as lat/lon
			double* pbb = (double*)m_buf_ptr;

#ifdef ARMHF
			double abox[4];
			memcpy(&abox[0], pbb, 4 * sizeof(double));
			tp->minx = abox[0];
			tp->maxx = abox[1];
			tp->miny = abox[2];
			tp->maxy = abox[3];
#else
			tp->minx = *pbb++;
			tp->maxx = *pbb++;
			tp->miny = *pbb++;
			tp->maxy = *pbb;
#endif

			m_buf_ptr += 4 * sizeof(double);

		} else // got end of poly
			not_finished = false;
	} // while

	m_ppg_head = ppg;
	m_nvertex_max = nvert_max;

	free(buf);

	m_bOK = true;
}

// Build PolyTessGeo Object from OGR Polygon
// Using internal Triangle tesselator
int PolyTessGeo::PolyTessGeoTri(OGRPolygon* poly, bool bSENC_SM, double ref_lat, double ref_lon)
{
	// Make a quick sanity check of the polygon coherence
	bool b_ok = true;
	OGRLineString* tls = poly->getExteriorRing();
	if (!tls) {
		b_ok = false;
	} else {
		int tnpta = poly->getExteriorRing()->getNumPoints();
		if (tnpta < 3)
			b_ok = false;
	}

	for (int iir = 0; iir < poly->getNumInteriorRings(); iir++) {
		int tnptr = poly->getInteriorRing(iir)->getNumPoints();
		if (tnptr < 3)
			b_ok = false;
	}

	if (!b_ok)
		return ERROR_BAD_OGRPOLY;

	m_pxgeom = NULL;

	int iir, ip;

	tess_orient = TESS_HORZ; // prefer horizontal tristrips

	// PolyGeo BBox
	OGREnvelope Envelope;
	poly->getEnvelope(&Envelope);
	xmin = Envelope.MinX;
	ymin = Envelope.MinY;
	xmax = Envelope.MaxX;
	ymax = Envelope.MaxY;

	// Get total number of contours
	ncnt = 1; // always exterior ring
	int nint = poly->getNumInteriorRings(); // interior rings
	ncnt += nint;

	// Allocate cntr array
	int* cntr = (int*)malloc(ncnt * sizeof(int));

	// Get total number of points(vertices)
	int npta = poly->getExteriorRing()->getNumPoints();
	npta += 2; // fluff

	for (iir = 0; iir < nint; iir++) {
		int nptr = poly->getInteriorRing(iir)->getNumPoints();
		npta += nptr + 2;
	}

	pt* geoPt = (pt*)calloc((npta + 1) * sizeof(pt), 1); // vertex array

	// Create input structures

	// Exterior Ring
	int npte = poly->getExteriorRing()->getNumPoints();
	cntr[0] = npte;

	pt* ppt = geoPt;
	ppt->x = 0.;
	ppt->y = 0.;
	ppt++; // vertex 0 is unused

	// Check and account for winding direction of ring
	bool cw = !(poly->getExteriorRing()->isClockwise() == 0);

	double x0, y0, x, y;
	OGRPoint p;

	if (cw) {
		poly->getExteriorRing()->getPoint(0, &p);
		x0 = p.getX();
		y0 = p.getY();
	} else {
		poly->getExteriorRing()->getPoint(npte - 1, &p);
		x0 = p.getX();
		y0 = p.getY();
	}

	// Transcribe points to vertex array, in proper order with no duplicates
	for (ip = 0; ip < npte; ip++) {

		int pidx;
		if (cw)
			pidx = npte - ip - 1;

		else
			pidx = ip;

		poly->getExteriorRing()->getPoint(pidx, &p);
		x = p.getX();
		y = p.getY();

		if ((fabs(x - x0) > EQUAL_EPS) || (fabs(y - y0) > EQUAL_EPS)) {
			ppt->x = x;
			ppt->y = y;

			ppt++;
		} else
			cntr[0]--;

		x0 = x;
		y0 = y;
	}

	// Now the interior contours
	for (iir = 0; iir < nint; iir++) {
		int npti = poly->getInteriorRing(iir)->getNumPoints();
		cntr[iir + 1] = npti;

		// Check and account for winding direction of ring
		bool cw = !(poly->getInteriorRing(iir)->isClockwise() == 0);

		if (!cw) {
			poly->getInteriorRing(iir)->getPoint(0, &p);
			x0 = p.getX();
			y0 = p.getY();
		} else {
			poly->getInteriorRing(iir)->getPoint(npti - 1, &p);
			x0 = p.getX();
			y0 = p.getY();
		}

		// Transcribe points to vertex array, in proper order with no duplicates
		for (int ip = 0; ip < npti; ip++) {
			OGRPoint p;
			int pidx;
			if (!cw) // interior contours must be cw
				pidx = npti - ip - 1;
			else
				pidx = ip;

			poly->getInteriorRing(iir)->getPoint(pidx, &p);
			x = p.getX();
			y = p.getY();

			if ((fabs(x - x0) > EQUAL_EPS) || (fabs(y - y0) > EQUAL_EPS)) {
				ppt->x = x;
				ppt->y = y;
				ppt++;
			} else
				cntr[iir + 1]--;

			x0 = x;
			y0 = y;
		}
	}

	polyout* polys = triangulate_polygon(ncnt, cntr, (double(*)[2])geoPt);

	// Check the triangles
	// Especially looking for poorly formed polys
	// These may come from several sources, all
	// of which should be considered latent bugs in the trapezator.

	// Known to occur:
	// Trapezator fails if any two inner contours share a common vertex.
	// Found on US5VA19M.000

	polyout* pck = polys;
	while (NULL != pck) {
		if (pck->is_valid) {
			int* ivs = pck->vertex_index_list;

			for (int i3 = 0; i3 < pck->nvert - 1; i3++) {
				int ptest = ivs[i3];
				for (int i4 = i3 + 1; i4 < pck->nvert; i4++) {
					if (ptest == ivs[i4]) {
						pck->is_valid = false;
					}
				}
			}
		}

		pck = (polyout*)pck->poly_next;
	}

	// Walk the list once to get poly count
	polyout* pr;
	pr = polys;
	int npoly0 = 0;
	while (NULL != pr) {
		pr = (polyout*)pr->poly_next;
		npoly0++;
	}

	// Check the list for duplicates

	pr = polys;
	for (int idt = 0; idt < npoly0 - 1; idt++) {
		polyout* p1 = pr;

		polyout* p2 = (polyout*)pr->poly_next;
		while (NULL != p2) {
			if (p1->is_valid && p2->is_valid) {
				if (ispolysame(p1, p2))
					p1->is_valid = false;
			}
			p2 = (polyout*)p2->poly_next;
		}

		pr = (polyout*)pr->poly_next;
	}

	// Walk the list again to get unique poly count
	pr = polys;
	int npoly = 0;
	while (NULL != pr) {
		if (pr->is_valid)
			npoly++;
		pr = (polyout*)pr->poly_next;
	}

	// Create the data structures

	m_nvertex_max = 0;

	m_ppg_head = new PolyTriGroup;
	m_ppg_head->m_bSMSENC = s_bSENC_SM;

	m_ppg_head->nContours = ncnt;

	m_ppg_head->pn_vertex = cntr; // pointer to array of poly vertex counts

	// Transcribe the raw geometry buffer
	// Converting to float as we go, and
	// allowing for tess_orient

	nwkb = (npta + 1) * 2 * sizeof(float);
	m_ppg_head->pgroup_geom = (float*)malloc(nwkb);
	float* vro = m_ppg_head->pgroup_geom;
	float tx;
	float ty;

	for (ip = 1; ip < npta + 1; ip++) {
		if (TESS_HORZ == tess_orient) {
			ty = geoPt[ip].y;
			tx = geoPt[ip].x;
		} else {
			tx = geoPt[ip].x;
			ty = geoPt[ip].y;
		}

		if (bSENC_SM) {
			//  Calculate SM from chart common reference point
			double easting, northing;
			toSM(geo::Position(ty, tx), geo::Position(ref_lat, ref_lon), &easting, &northing);
			*vro++ = easting; // x
			*vro++ = northing; // y
		} else {
			*vro++ = tx; // lon
			*vro++ = ty; // lat
		}
	}

	// Now the Triangle Primitives

	geo::TriPrim* pTP = NULL;
	geo::TriPrim* pTP_Head = NULL;
	geo::TriPrim* pTP_Last = NULL;

	pr = polys;
	while (NULL != pr) {
		if (pr->is_valid) {
			pTP = new geo::TriPrim;
			if (NULL == pTP_Last) {
				pTP_Head = pTP;
				pTP_Last = pTP;
			} else {
				pTP_Last->p_next = pTP;
				pTP_Last = pTP;
			}

			pTP->p_next = NULL;
			pTP->type = TriPrim::PTG_TRIANGLES;
			pTP->nVert = pr->nvert;

			if (pr->nvert > m_nvertex_max)
				m_nvertex_max = pr->nvert; // keep track of largest vertex count

			//  Convert to SM
			pTP->p_vertex = (double*)malloc(pr->nvert * 2 * sizeof(double));
			double* pdd = pTP->p_vertex;
			int* ivr = pr->vertex_index_list;
			if (bSENC_SM) {
				for (int i = 0; i < pr->nvert; i++) {
					int ivp = ivr[i];
					double dlon = geoPt[ivp].x;
					double dlat = geoPt[ivp].y;

					double easting, northing;
					toSM(geo::Position(dlat, dlon), geo::Position(ref_lat, ref_lon), &easting,
						 &northing);
					*pdd++ = easting;
					*pdd++ = northing;
				}
			} else {
				for (int i = 0; i < pr->nvert; i++) {
					int ivp = ivr[i];

					memcpy(pdd++, &geoPt[ivp].x, sizeof(double));
					memcpy(pdd++, &geoPt[ivp].y, sizeof(double));
				}
			}
			// Calculate bounding box as lat/lon

			float sxmax = -179; // this poly BBox
			float sxmin = 170;
			float symax = -90;
			float symin = 90;

			for (int iv = 0; iv < pr->nvert; iv++) {
				int* ivr = pr->vertex_index_list;
				int ivp = ivr[iv];
				double xd = geoPt[ivp].x;
				double yd = geoPt[ivp].y;

				sxmax = fmax(xd, sxmax);
				sxmin = fmin(xd, sxmin);
				symax = fmax(yd, symax);
				symin = fmin(yd, symin);
			}

			pTP->minx = sxmin;
			pTP->miny = symin;
			pTP->maxx = sxmax;
			pTP->maxy = symax;
		}
		pr = (polyout*)pr->poly_next;
	}

	m_ppg_head->tri_prim_head = pTP_Head; // head of linked list of TriPrims

	// Free the polyout structure
	pr = polys;
	while (NULL != pr) {
		free(pr->vertex_index_list);

		polyout* pf = pr;
		pr = (polyout*)pr->poly_next;
		free(pf);
	}

	free(geoPt);

	m_bOK = true;

	return 0;
}

bool PolyTessGeo::IsOk() const
{
	return m_bOK;
}

double PolyTessGeo::Get_xmin() const
{
	return xmin;
}

double PolyTessGeo::Get_xmax() const
{
	return xmax;
}

double PolyTessGeo::Get_ymin() const
{
	return ymin;
}

double PolyTessGeo::Get_ymax() const
{
	return ymax;
}

PolyTriGroup* PolyTessGeo::Get_PolyTriGroup_head()
{
	return m_ppg_head;
}

int PolyTessGeo::GetnVertexMax() const
{
	return m_nvertex_max;
}

int PolyTessGeo::Write_PolyTriGroup(FILE* ofs) const
{
	wxString sout;
	wxString stemp;

	const PolyTriGroup* pPTG = m_ppg_head;

	// Begin creating the output record
	// Use a wxMemoryStream for temporary record output.
	// When all finished, we'll touch up a few items before
	// committing to disk.

	wxMemoryOutputStream* ostream1 = new wxMemoryOutputStream(NULL, 0); // auto buffer creation
	wxMemoryOutputStream* ostream2 = new wxMemoryOutputStream(NULL, 0); // auto buffer creation

	// Create initial known part of the output record

	// PolyTessGeo Properties
	sout += wxString::Format(_T("  POLYTESSGEOPROP %f %f %f %f\n"), xmin, ymin, xmax, ymax);

	// Transcribe the true number of  contours, and the raw geometry wkb size
	sout += wxString::Format(_T("Contours/nWKB %d %d\n"), ncnt, nwkb);

	// Transcribe the contour counts
	sout += wxString::Format(_T("Contour nV"));
	for (int i = 0; i < ncnt; i++) {
		sout += wxString::Format(_T(" %d"), pPTG->pn_vertex[i]);
	}
	sout += wxString::Format(_T("\n"));
	ostream1->Write(sout.mb_str(), sout.Len());

	// Transcribe the raw geometry buffer
	ostream1->Write(pPTG->pgroup_geom, nwkb);
	stemp.sprintf(_T("\n"));
	ostream1->Write(stemp.mb_str(), stemp.Len());

	// Transcribe the TriPrim chain

	const geo::TriPrim* pTP = pPTG->tri_prim_head; // head of linked list of TriPrims

	while (pTP) {
		ostream2->Write(&pTP->type, sizeof(int));
		ostream2->Write(&pTP->nVert, sizeof(int));

		ostream2->Write(pTP->p_vertex, pTP->nVert * 2 * sizeof(double));

		// Write out the object bounding box as lat/lon
		ostream2->Write(&pTP->minx, sizeof(double));
		ostream2->Write(&pTP->maxx, sizeof(double));
		ostream2->Write(&pTP->miny, sizeof(double));
		ostream2->Write(&pTP->maxy, sizeof(double));

		pTP = pTP->p_next;
	}

	stemp.sprintf(_T("POLYEND\n"));
	ostream2->Write(stemp.mb_str(), stemp.Len());

	int nrecl = ostream1->GetSize() + ostream2->GetSize();
	stemp.sprintf(_T("  POLYTESSGEO  %08d %g %g\n"), nrecl, m_ref_lat, m_ref_lon);

	fwrite(stemp.mb_str(), 1, stemp.Len(), ofs); // Header, + record length

	char* tb = new char[ostream1->GetSize()];
	ostream1->CopyTo(tb, ostream1->GetSize());
	fwrite(tb, 1, ostream1->GetSize(), ofs);
	delete [] tb;

	tb = new char[ostream2->GetSize()];
	ostream2->CopyTo(tb, ostream2->GetSize());
	fwrite(tb, 1, ostream2->GetSize(), ofs);
	delete [] tb;

	delete ostream1;
	delete ostream2;

	return 0;
}

int PolyTessGeo::my_bufgets(char* buf, int buf_len_max)
{
	char chNext;
	int nLineLen = 0;
	char* lbuf;

	lbuf = buf;

	while ((nLineLen < buf_len_max) && ((m_buf_ptr - m_buf_head) < m_nrecl)) {
		chNext = *m_buf_ptr++;

		// each CR/LF (or LF/CR) as if just "CR"
		if (chNext == 10 || chNext == 13) {
			chNext = '\n';
		}

		*lbuf = chNext;
		lbuf++;
		nLineLen++;

		if (chNext == '\n') {
			*lbuf = '\0';
			return nLineLen;
		}
	}

	*(lbuf) = '\0';
	return nLineLen;
}

PolyTessGeo::~PolyTessGeo()
{
	delete m_ppg_head;
	delete m_pxgeom;

#ifdef USE_GLU_TESS
	if (s_pwork_buf)
		free(s_pwork_buf);
	s_pwork_buf = NULL;
#endif
}

int PolyTessGeo::BuildDeferredTess(void)
{
#ifdef USE_GLU_TESS
	return BuildTessGL();
#else
	return 0;
#endif
}

#ifdef USE_GLU_TESS
#ifdef __WXMSW__
#define __CALL_CONVENTION __stdcall
#else
#define __CALL_CONVENTION
#endif


void __CALL_CONVENTION beginCallback(GLenum which);
void __CALL_CONVENTION errorCallback(GLenum errorCode);
void __CALL_CONVENTION endCallback(void);
void __CALL_CONVENTION vertexCallback(GLvoid *vertex);
void __CALL_CONVENTION combineCallback(
		GLdouble coords[3],
		GLdouble *vertex_data[4],
		GLfloat weight[4],
		GLdouble **dataOut);


// Build PolyTessGeo Object from OGR Polygon
// Using OpenGL/GLU tesselator
int PolyTessGeo::PolyTessGeoGL(OGRPolygon* poly, bool bSENC_SM, double ref_lat, double ref_lon)
{
#ifdef ocpnUSE_GL

	int iir;
	int ip;
	int* cntr;
	GLdouble* geoPt;

	wxString sout;
	wxString sout1;
	wxString stemp;

	// Make a quick sanity check of the polygon coherence
	bool b_ok = true;
	OGRLineString* tls = poly->getExteriorRing();
	if (!tls) {
		b_ok = false;
	} else {
		int tnpta = poly->getExteriorRing()->getNumPoints();
		if (tnpta < 3)
			b_ok = false;
	}

	for (iir = 0; iir < poly->getNumInteriorRings(); iir++) {
		int tnptr = poly->getInteriorRing(iir)->getNumPoints();
		if (tnptr < 3)
			b_ok = false;
	}

	if (!b_ok)
		return ERROR_BAD_OGRPOLY;

#ifdef __WXMSW__
	// If using the OpenGL dlls provided with Windows,
	// load the dll and establish addresses of the entry points needed

#ifdef USE_GLU_DLL

	if (!s_glu_dll_ready) {

		s_hGLU_DLL = LoadLibrary("glu32.dll");
		if (s_hGLU_DLL != NULL) {
			s_lpfnTessProperty = (LPFNDLLTESSPROPERTY)GetProcAddress(s_hGLU_DLL, "gluTessProperty");
			s_lpfnNewTess = (LPFNDLLNEWTESS)GetProcAddress(s_hGLU_DLL, "gluNewTess");
			s_lpfnTessBeginContour
				= (LPFNDLLTESSBEGINCONTOUR)GetProcAddress(s_hGLU_DLL, "gluTessBeginContour");
			s_lpfnTessEndContour
				= (LPFNDLLTESSENDCONTOUR)GetProcAddress(s_hGLU_DLL, "gluTessEndContour");
			s_lpfnTessBeginPolygon
				= (LPFNDLLTESSBEGINPOLYGON)GetProcAddress(s_hGLU_DLL, "gluTessBeginPolygon");
			s_lpfnTessEndPolygon
				= (LPFNDLLTESSENDPOLYGON)GetProcAddress(s_hGLU_DLL, "gluTessEndPolygon");
			s_lpfnDeleteTess = (LPFNDLLDELETETESS)GetProcAddress(s_hGLU_DLL, "gluDeleteTess");
			s_lpfnTessVertex = (LPFNDLLTESSVERTEX)GetProcAddress(s_hGLU_DLL, "gluTessVertex");
			s_lpfnTessCallback = (LPFNDLLTESSCALLBACK)GetProcAddress(s_hGLU_DLL, "gluTessCallback");

			s_glu_dll_ready = true;
		} else {
			return ERROR_NO_DLL;
		}
	}

#endif
#endif

// Allocate a work buffer, which will be grown as needed
#define NINIT_BUFFER_LEN 10000
	s_pwork_buf = (GLdouble*)malloc(NINIT_BUFFER_LEN * 2 * sizeof(GLdouble));
	s_buf_len = NINIT_BUFFER_LEN * 2;
	s_buf_idx = 0;

	// Create an array to hold pointers to allocated vertices created by "combine" callback,
	// so that they may be deleted after tesselation.
	s_pCombineVertexArray = new wxArrayPtrVoid;

	// Create tesselator
	GLUtessobj = gluNewTess();

	// Register the callbacks
	gluTessCallback(GLUtessobj, GLU_TESS_BEGIN, (GLvoid(__CALL_CONVENTION*)()) & beginCallback);
	gluTessCallback(GLUtessobj, GLU_TESS_BEGIN, (GLvoid(__CALL_CONVENTION*)()) & beginCallback);
	gluTessCallback(GLUtessobj, GLU_TESS_VERTEX, (GLvoid(__CALL_CONVENTION*)()) & vertexCallback);
	gluTessCallback(GLUtessobj, GLU_TESS_END, (GLvoid(__CALL_CONVENTION*)()) & endCallback);
	gluTessCallback(GLUtessobj, GLU_TESS_COMBINE, (GLvoid(__CALL_CONVENTION*)()) & combineCallback);

	gluTessProperty(GLUtessobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);

	// gluTess algorithm internally selects vertically oriented triangle strips and fans.
	// This orientation is not optimal for conventional memory-mapped raster display shape filling.
	// We can "fool" the algorithm by interchanging the x and y vertex values passed to
	// gluTessVertex
	// and then reverting x and y on the resulting vertexCallbacks.
	// In this implementation, we will explicitely set the preferred orientation.

	// Set the preferred orientation
	tess_orient = TESS_HORZ; // prefer horizontal tristrips

	// PolyGeo BBox as lat/lon
	OGREnvelope Envelope;
	poly->getEnvelope(&Envelope);
	xmin = Envelope.MinX;
	ymin = Envelope.MinY;
	xmax = Envelope.MaxX;
	ymax = Envelope.MaxY;

	// Get total number of contours
	ncnt = 1; // always exterior ring
	int nint = poly->getNumInteriorRings(); // interior rings
	ncnt += nint;

	// Allocate cntr array
	cntr = (int*)malloc(ncnt * sizeof(int));

	// Get total number of points(vertices)
	int npta = poly->getExteriorRing()->getNumPoints();
	cntr[0] = npta;
	npta += 2; // fluff

	for (iir = 0; iir < nint; iir++) {
		int nptr = poly->getInteriorRing(iir)->getNumPoints();
		cntr[iir + 1] = nptr;

		npta += nptr + 2;
	}

	geoPt = (GLdouble*)malloc((npta) * 3 * sizeof(GLdouble)); // vertex array

	// Grow the work buffer if necessary

	if ((npta * 4) > s_buf_len) {
		s_pwork_buf = (GLdouble*)realloc(s_pwork_buf, npta * 4 * 2 * sizeof(GLdouble*));
		s_buf_len = npta * 4 * 2;
	}

	// Define the polygon
	gluTessBeginPolygon(GLUtessobj, NULL);

	// Create input structures

	// Exterior Ring
	int npte = poly->getExteriorRing()->getNumPoints();
	cntr[0] = npte;

	GLdouble* ppt = geoPt;

	// Check and account for winding direction of ring
	bool cw = !(poly->getExteriorRing()->isClockwise() == 0);

	double x0, y0, x, y;
	OGRPoint p;

	if (cw) {
		poly->getExteriorRing()->getPoint(0, &p);
		x0 = p.getX();
		y0 = p.getY();
	} else {
		poly->getExteriorRing()->getPoint(npte - 1, &p);
		x0 = p.getX();
		y0 = p.getY();
	}

	gluTessBeginContour(GLUtessobj);

	// Transcribe points to vertex array, in proper order with no duplicates
	// also, accounting for tess_orient
	for (ip = 0; ip < npte; ip++) {
		int pidx;
		if (cw)
			pidx = npte - ip - 1;
		else
			pidx = ip;

		poly->getExteriorRing()->getPoint(pidx, &p);
		x = p.getX();
		y = p.getY();

		if ((fabs(x - x0) > EQUAL_EPS) || (fabs(y - y0) > EQUAL_EPS)) {
			GLdouble* ppt_temp = ppt;
			if (tess_orient == TESS_VERT) {
				*ppt++ = x;
				*ppt++ = y;
			} else {
				*ppt++ = y;
				*ppt++ = x;
			}

			*ppt++ = 0.0;

			gluTessVertex(GLUtessobj, ppt_temp, ppt_temp);

		} else
			cntr[0]--;

		x0 = x;
		y0 = y;
	}

	gluTessEndContour(GLUtessobj);

	// Now the interior contours
	for (iir = 0; iir < nint; iir++) {
		gluTessBeginContour(GLUtessobj);

		int npti = poly->getInteriorRing(iir)->getNumPoints();

		// Check and account for winding direction of ring
		bool cw = !(poly->getInteriorRing(iir)->isClockwise() == 0);

		if (!cw) {
			poly->getInteriorRing(iir)->getPoint(0, &p);
			x0 = p.getX();
			y0 = p.getY();
		} else {
			poly->getInteriorRing(iir)->getPoint(npti - 1, &p);
			x0 = p.getX();
			y0 = p.getY();
		}

		// Transcribe points to vertex array, in proper order with no duplicates
		// also, accounting for tess_orient
		for (int ip = 0; ip < npti; ip++) {
			OGRPoint p;
			int pidx;
			if (!cw) // interior contours must be cw
				pidx = npti - ip - 1;
			else
				pidx = ip;

			poly->getInteriorRing(iir)->getPoint(pidx, &p);
			x = p.getX();
			y = p.getY();

			if ((fabs(x - x0) > EQUAL_EPS) || (fabs(y - y0) > EQUAL_EPS)) {
				GLdouble* ppt_temp = ppt;
				if (tess_orient == TESS_VERT) {
					*ppt++ = x;
					*ppt++ = y;
				} else {
					*ppt++ = y;
					*ppt++ = x;
				}
				*ppt++ = 0.0;

				gluTessVertex(GLUtessobj, ppt_temp, ppt_temp);

			} else
				cntr[iir + 1]--;

			x0 = x;
			y0 = y;
		}

		gluTessEndContour(GLUtessobj);
	}

	// Store some SM conversion data in static store,
	// for callback access
	s_ref_lat = ref_lat;
	s_ref_lon = ref_lon;
	s_bSENC_SM = bSENC_SM;

	s_bmerc_transform = false;

	// Ready to kick off the tesselator

	s_pTPG_Last = NULL;
	s_pTPG_Head = NULL;

	s_nvmax = 0;

	gluTessEndPolygon(GLUtessobj); // here it goes

	m_nvertex_max = s_nvmax; // record largest vertex count, updates in callback

	// Tesselation all done, so...

	// Create the data structures

	m_ppg_head = new PolyTriGroup;
	m_ppg_head->m_bSMSENC = s_bSENC_SM;

	m_ppg_head->nContours = ncnt;

	m_ppg_head->pn_vertex = cntr; // pointer to array of poly vertex counts

	// Transcribe the raw geometry buffer
	// Converting to float as we go, and
	// allowing for tess_orient
	// Also, convert to SM if requested

	nwkb = (npta + 1) * 2 * sizeof(float);
	m_ppg_head->pgroup_geom = (float*)malloc(nwkb);
	float* vro = m_ppg_head->pgroup_geom;
	ppt = geoPt;
	float tx;
	float ty;

	for (ip = 0; ip < npta; ip++) {
		if (TESS_HORZ == tess_orient) {
			ty = *ppt++;
			tx = *ppt++;
		} else {
			tx = *ppt++;
			ty = *ppt++;
		}

		if (bSENC_SM) {
			// Calculate SM from chart common reference point
			double easting, northing;
			toSM(geo::Position(ty, tx), geo::Position(ref_lat, ref_lon), &easting, &northing);
			*vro++ = easting; // x
			*vro++ = northing; // y
		} else {
			*vro++ = tx; // lon
			*vro++ = ty; // lat
		}

		ppt++; // skip z
	}

	m_ppg_head->tri_prim_head = s_pTPG_Head; // head of linked list of TriPrims

	gluDeleteTess(GLUtessobj);

	free(s_pwork_buf);
	s_pwork_buf = NULL;

	free(geoPt);

	destroy_combined_vertices();

	m_bOK = true;

#endif //    #ifdef ocpnUSE_GL

	return 0;
}

int PolyTessGeo::BuildTessGL(void)
{
#ifdef ocpnUSE_GL

	int iir;
	int ip;
	int* cntr;
	GLdouble* geoPt;

	wxString sout;
	wxString sout1;
	wxString stemp;

#ifdef __WXMSW__
	// If using the OpenGL dlls provided with Windows,
	// load the dll and establish addresses of the entry points needed

#ifdef USE_GLU_DLL

	if (!s_glu_dll_ready) {

		s_hGLU_DLL = LoadLibrary("glu32.dll");
		if (s_hGLU_DLL != NULL) {
			s_lpfnTessProperty = (LPFNDLLTESSPROPERTY)GetProcAddress(s_hGLU_DLL, "gluTessProperty");
			s_lpfnNewTess = (LPFNDLLNEWTESS)GetProcAddress(s_hGLU_DLL, "gluNewTess");
			s_lpfnTessBeginContour
				= (LPFNDLLTESSBEGINCONTOUR)GetProcAddress(s_hGLU_DLL, "gluTessBeginContour");
			s_lpfnTessEndContour
				= (LPFNDLLTESSENDCONTOUR)GetProcAddress(s_hGLU_DLL, "gluTessEndContour");
			s_lpfnTessBeginPolygon
				= (LPFNDLLTESSBEGINPOLYGON)GetProcAddress(s_hGLU_DLL, "gluTessBeginPolygon");
			s_lpfnTessEndPolygon
				= (LPFNDLLTESSENDPOLYGON)GetProcAddress(s_hGLU_DLL, "gluTessEndPolygon");
			s_lpfnDeleteTess = (LPFNDLLDELETETESS)GetProcAddress(s_hGLU_DLL, "gluDeleteTess");
			s_lpfnTessVertex = (LPFNDLLTESSVERTEX)GetProcAddress(s_hGLU_DLL, "gluTessVertex");
			s_lpfnTessCallback = (LPFNDLLTESSCALLBACK)GetProcAddress(s_hGLU_DLL, "gluTessCallback");

			s_glu_dll_ready = true;
		} else {
			return ERROR_NO_DLL;
		}
	}

#endif
#endif

	// Allocate a work buffer, which will be grown as needed
#define NINIT_BUFFER_LEN 10000
	s_pwork_buf = (GLdouble*)malloc(NINIT_BUFFER_LEN * 2 * sizeof(GLdouble));
	s_buf_len = NINIT_BUFFER_LEN * 2;
	s_buf_idx = 0;

	// Create an array to hold pointers to allocated vertices created by "combine" callback,
	// so that they may be deleted after tesselation.
	s_pCombineVertexArray = new wxArrayPtrVoid;

	// Create tesselator
	GLUtessobj = gluNewTess();

	// Register the callbacks
	gluTessCallback(GLUtessobj, GLU_TESS_BEGIN, (GLvoid(__CALL_CONVENTION*)()) & beginCallback);
	gluTessCallback(GLUtessobj, GLU_TESS_BEGIN, (GLvoid(__CALL_CONVENTION*)()) & beginCallback);
	gluTessCallback(GLUtessobj, GLU_TESS_VERTEX, (GLvoid(__CALL_CONVENTION*)()) & vertexCallback);
	gluTessCallback(GLUtessobj, GLU_TESS_END, (GLvoid(__CALL_CONVENTION*)()) & endCallback);
	gluTessCallback(GLUtessobj, GLU_TESS_COMBINE, (GLvoid(__CALL_CONVENTION*)()) & combineCallback);

	gluTessProperty(GLUtessobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);

	// gluTess algorithm internally selects vertically oriented triangle strips and fans.
	// This orientation is not optimal for conventional memory-mapped raster display shape filling.
	// We can "fool" the algorithm by interchanging the x and y vertex values passed to
	// gluTessVertex
	// and then reverting x and y on the resulting vertexCallbacks.
	// In this implementation, we will explicitely set the preferred orientation.

	// Set the preferred orientation
	tess_orient = TESS_HORZ; // prefer horizontal tristrips

	// Get total number of contours
	ncnt = m_pxgeom->n_contours;

	// Allocate cntr array
	cntr = (int*)malloc(ncnt * sizeof(int));

	// Get total number of points(vertices)
	int npta = m_pxgeom->contour_array[0];
	cntr[0] = npta;
	npta += 2; // fluff

	for (iir = 0; iir < ncnt - 1; iir++) {
		int nptr = m_pxgeom->contour_array[iir + 1];
		cntr[iir + 1] = nptr;

		npta += nptr + 2; // fluff
	}

	geoPt = (GLdouble*)malloc((npta) * 3 * sizeof(GLdouble)); // vertex array

	// Grow the work buffer if necessary

	if ((npta * 4) > s_buf_len) {
		s_pwork_buf = (GLdouble*)realloc(s_pwork_buf, npta * 4 * 2 * sizeof(GLdouble*));
		s_buf_len = npta * 4 * 2;
	}

	//  Define the polygon
	gluTessBeginPolygon(GLUtessobj, NULL);

	// Create input structures

	// Exterior Ring
	int npte = m_pxgeom->contour_array[0];
	cntr[0] = npte;

	GLdouble* ppt = geoPt;

	//  Check and account for winding direction of ring
	bool cw = true;

	double x0, y0, x, y;
	OGRPoint p;

	wxPoint2DDouble* pp = m_pxgeom->vertex_array;
	pp++; // skip 0?

	if (cw) {
		x0 = pp->m_x;
		y0 = pp->m_y;
	} else {
		x0 = pp[npte - 1].m_x;
		y0 = pp[npte - 1].m_y;
	}

	gluTessBeginContour(GLUtessobj);

	// Transcribe points to vertex array, in proper order with no duplicates
	// also, accounting for tess_orient
	for (ip = 0; ip < npte; ip++) {
		int pidx;
		if (cw)
			pidx = npte - ip - 1;
		else
			pidx = ip;

		x = pp[pidx].m_x;
		y = pp[pidx].m_y;

		if ((fabs(x - x0) > EQUAL_EPS) || (fabs(y - y0) > EQUAL_EPS)) {
			GLdouble* ppt_temp = ppt;
			if (tess_orient == TESS_VERT) {
				*ppt++ = x;
				*ppt++ = y;
			} else {
				*ppt++ = y;
				*ppt++ = x;
			}

			*ppt++ = 0.0;

			gluTessVertex(GLUtessobj, ppt_temp, ppt_temp);

		} else
			cntr[0]--;

		x0 = x;
		y0 = y;
	}

	gluTessEndContour(GLUtessobj);

	int index_offset = npte;
#if 1
	// Now the interior contours
	for (iir = 0; iir < ncnt - 1; iir++) {
		gluTessBeginContour(GLUtessobj);

		int npti = m_pxgeom->contour_array[iir + 1];

		// Check and account for winding direction of ring
		bool cw = false; //!(poly->getInteriorRing(iir)->isClockwise() == 0);

		if (!cw) {
			x0 = pp[index_offset].m_x;
			y0 = pp[index_offset].m_y;
		} else {
			x0 = pp[index_offset + npti - 1].m_x;
			y0 = pp[index_offset + npti - 1].m_y;
		}

		// Transcribe points to vertex array, in proper order with no duplicates
		// also, accounting for tess_orient
		for (int ip = 0; ip < npti; ip++) {
			OGRPoint p;
			int pidx;
			if (!cw) // interior contours must be cw
				pidx = npti - ip - 1;
			else
				pidx = ip;

			x = pp[pidx + index_offset].m_x;
			y = pp[pidx + index_offset].m_y;

			if ((fabs(x - x0) > EQUAL_EPS) || (fabs(y - y0) > EQUAL_EPS)) {
				GLdouble* ppt_temp = ppt;
				if (tess_orient == TESS_VERT) {
					*ppt++ = x;
					*ppt++ = y;
				} else {
					*ppt++ = y;
					*ppt++ = x;
				}
				*ppt++ = 0.0;

				gluTessVertex(GLUtessobj, ppt_temp, ppt_temp);

			} else
				cntr[iir + 1]--;

			x0 = x;
			y0 = y;
		}

		gluTessEndContour(GLUtessobj);

		index_offset += npti;
	}
#endif

	// Store some SM conversion data in static store,
	// for callback access
	s_bSENC_SM = false;

	s_bmerc_transform = true;
	s_transform_x_rate = m_pxgeom->x_rate;
	s_transform_x_origin = m_pxgeom->x_offset;
	s_transform_y_rate = m_pxgeom->y_rate;
	s_transform_y_origin = m_pxgeom->y_offset;

	// Ready to kick off the tesselator

	s_pTPG_Last = NULL;
	s_pTPG_Head = NULL;

	s_nvmax = 0;

	gluTessEndPolygon(GLUtessobj); // here it goes

	m_nvertex_max = s_nvmax; // record largest vertex count, updates in callback

	// Tesselation all done, so...

	// Create the data structures

	m_ppg_head = new PolyTriGroup;
	m_ppg_head->m_bSMSENC = s_bSENC_SM;

	m_ppg_head->nContours = ncnt;
	m_ppg_head->pn_vertex = cntr; // pointer to array of poly vertex counts

	// Transcribe the raw geometry buffer
	// Converting to float as we go, and
	// allowing for tess_orient
	// Also, convert to SM if requested

	nwkb = (npta + 1) * 2 * sizeof(float);
	m_ppg_head->pgroup_geom = (float*)malloc(nwkb);
	float* vro = m_ppg_head->pgroup_geom;
	ppt = geoPt;
	float tx;
	float ty;

	for (ip = 0; ip < npta; ip++) {
		if (TESS_HORZ == tess_orient) {
			ty = *ppt++;
			tx = *ppt++;
		} else {
			tx = *ppt++;
			ty = *ppt++;
		}

		if (0 /*bSENC_SM*/) {
			// Calculate SM from chart common reference point
			double easting, northing;
			*vro++ = easting; // x
			*vro++ = northing; // y
		} else {
			*vro++ = tx; // lon
			*vro++ = ty; // lat
		}

		ppt++; // skip z
	}

	m_ppg_head->tri_prim_head = s_pTPG_Head; // head of linked list of TriPrims

	gluDeleteTess(GLUtessobj);

	free(s_pwork_buf);
	s_pwork_buf = NULL;

	free(geoPt);

	// All allocated buffers are owned now by the m_ppg_head
	// And will be freed on dtor of this object
	delete m_pxgeom;

	destroy_combined_vertices();

	m_pxgeom = NULL;

	m_bOK = true;

#endif

	return 0;
}

// GLU tesselation support functions
void __CALL_CONVENTION beginCallback(GLenum which)
{
	s_buf_idx = 0;
	s_nvcall = 0;
	s_gltri_type = static_cast<geo::TriPrim::Type>(which);
}

void __CALL_CONVENTION endCallback(void)
{
	// Create a TriPrim

	char buf[40];

	if (s_nvcall > s_nvmax) // keep track of largest number of triangle vertices
		s_nvmax = s_nvcall;

	switch (s_gltri_type) {
		case GL_TRIANGLE_FAN:
		case GL_TRIANGLE_STRIP:
		case GL_TRIANGLES: {
			geo::TriPrim* pTPG = new geo::TriPrim;
			if (NULL == s_pTPG_Last) {
				s_pTPG_Head = pTPG;
				s_pTPG_Last = pTPG;
			} else {
				s_pTPG_Last->p_next = pTPG;
				s_pTPG_Last = pTPG;
			}

			pTPG->p_next = NULL;
			pTPG->type = s_gltri_type;
			pTPG->nVert = s_nvcall;

			//  Calculate bounding box
			float sxmax = -1000; // this poly BBox
			float sxmin = 1000;
			float symax = -90;
			float symin = 90;

			GLdouble* pvr = s_pwork_buf;
			for (int iv = 0; iv < s_nvcall; iv++) {
				GLdouble xd, yd;
				xd = *pvr++;
				yd = *pvr++;

				if (s_bmerc_transform) {
					double valx = (xd * s_transform_x_rate) + s_transform_x_origin;
					double valy = (yd * s_transform_y_rate) + s_transform_y_origin;

					//    Convert to lat/lon
					double lat = (2.0 * atan(exp(valy / CM93_semimajor_axis_meters)) - M_PI / 2.0)
								 / (M_PI / 180.0);
					double lon = (valx / ((M_PI / 180.0) * CM93_semimajor_axis_meters));

					sxmax = fmax(lon, sxmax);
					sxmin = fmin(lon, sxmin);
					symax = fmax(lat, symax);
					symin = fmin(lat, symin);
				} else {
					sxmax = fmax(xd, sxmax);
					sxmin = fmin(xd, sxmin);
					symax = fmax(yd, symax);
					symin = fmin(yd, symin);
				}
			}

			pTPG->minx = sxmin;
			pTPG->miny = symin;
			pTPG->maxx = sxmax;
			pTPG->maxy = symax;

			//  Transcribe this geometry to TriPrim, converting to SM if called for

			if (s_bSENC_SM) {
				double* pds = s_pwork_buf;
				pTPG->p_vertex = (double*)malloc(s_nvcall * 2 * sizeof(double));
				double* pdd = pTPG->p_vertex;

				for (int ip = 0; ip < s_nvcall; ip++) {
					double dlon = *pds++;
					double dlat = *pds++;

					double easting, northing;
					toSM(geo::Position(dlat, dlon), geo::Position(s_ref_lat, s_ref_lon), &easting,
						 &northing);
					double deast = easting;
					double dnorth = northing;
					*pdd++ = deast;
					*pdd++ = dnorth;
				}
			} else {
				pTPG->p_vertex = (double*)malloc(s_nvcall * 2 * sizeof(double));
				memcpy(pTPG->p_vertex, s_pwork_buf, s_nvcall * 2 * sizeof(double));
			}

			break;
		}

		default: {
			sprintf(buf, "....begin Callback  unknown\n");
			break;
		}
	}
}

void __CALL_CONVENTION vertexCallback(GLvoid* vertex)
{
	GLdouble* pointer = static_cast<GLdouble*>(vertex);

	if (s_buf_idx > s_buf_len - 4) {
		int new_buf_len = s_buf_len + 100;
		GLdouble* tmp = s_pwork_buf;

		s_pwork_buf = (GLdouble*)realloc(s_pwork_buf, new_buf_len * sizeof(GLdouble));
		if (NULL == s_pwork_buf) {
			free(tmp);
			tmp = NULL;
		} else
			s_buf_len = new_buf_len;
	}

	if (tess_orient == TESS_VERT) {
		s_pwork_buf[s_buf_idx++] = pointer[0];
		s_pwork_buf[s_buf_idx++] = pointer[1];
	} else {
		s_pwork_buf[s_buf_idx++] = pointer[1];
		s_pwork_buf[s_buf_idx++] = pointer[0];
	}

	s_nvcall++;
}

// combineCallback is used to create a new vertex when edges
// intersect.
void __CALL_CONVENTION
combineCallback(GLdouble coords[3], GLdouble* vertex_data[4], GLfloat weight[4], GLdouble** dataOut)
{
	// prevent compiler warnings
	(void)vertex_data;
	(void)weight;

	GLdouble* vertex = (GLdouble*)malloc(6 * sizeof(GLdouble));

	vertex[0] = coords[0];
	vertex[1] = coords[1];
	vertex[2] = coords[2];
	vertex[3] = vertex[4] = vertex[5] = 0.0; // 01/13/05 bugfix

	*dataOut = vertex;

	s_pCombineVertexArray->Add(vertex);
}

#endif

}

