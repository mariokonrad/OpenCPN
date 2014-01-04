/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Portins Copyright (C) 2010 by David S. Register                       *
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

/////////////////////////////////////////////////////////////////////////////
// Name:        src/gtk/region.cpp
// Purpose:
// Author:      Robert Roebling
// Modified:    VZ at 05.10.00: use AllocExclusive(), comparison fixed
// Id:          $Id: region.cpp 42903 2006-11-01 12:56:38Z RR $
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#include "OCPNRegion.h"
#include <wx/log.h>

enum OGdkFillRule {
	OGDK_EVEN_ODD_RULE,
	OGDK_WINDING_RULE
};

enum OGdkOverlapType {
	OGDK_OVERLAP_RECTANGLE_IN,
	OGDK_OVERLAP_RECTANGLE_OUT,
	OGDK_OVERLAP_RECTANGLE_PART
};

struct OGdkPoint
{
	int x;
	int y;
};

struct OGdkRectangle
{
	int x;
	int y;
	int width;
	int height;
};

struct OGdkSegment
{
	int x1;
	int y1;
	int x2;
	int y2;
};

typedef OGdkSegment OGdkRegionBox;

struct OGdkRegion
{
	long size;
	long numRects;
	OGdkRegionBox* rects;
	OGdkRegionBox extents;
};

// number of points to buffer before sending them off
// to scanlines() :  Must be an even number
#define NUMPTSTOBUFFER 200

// used to allocate buffers for points and link
// the buffers together
struct OPOINTBLOCK
{
	OGdkPoint pts[NUMPTSTOBUFFER];
	struct OPOINTBLOCK* next;
};

#define INBOX(r, x, y) ((((r).x2 > x)) && (((r).x1 <= x)) && (((r).y2 > y)) && (((r).y1 <= y)))

// if two BOXs overlap.
// 0 if two BOXs do not overlap.
// Remember, x2 and y2 are not in the region
#define EXTENTCHECK(r1, r2) \
	((r1)->x2 > (r2)->x1&&(r1)->x1<(r2)->x2&&(r1)->y2>(r2)->y1&&(r1)->y1 < (r2)->y2)

/*
 * #define _OG_NEW(struct_type, n_structs, func) \
 *    ((struct_type *) malloc ((n_structs), sizeof (struct_type)))
 * #define _OG_RENEW(struct_type, mem, n_structs, func) \
 *    ((struct_type *) realloc (mem, (n_structs), sizeof (struct_type)))
 *
 * #define og_new(struct_type, n_structs)                   _OG_NEW (struct_type, n_structs, malloc)
 * #define og_renew(struct_type, mem, n_structs)            _OG_RENEW (struct_type, mem, n_structs, realloc)
 */

#define OGROWREGION(reg, nRects)                                                                  \
	{                                                                                             \
		if ((nRects) == 0) {                                                                      \
			if ((reg)->rects != &(reg)->extents) {                                                \
				free((reg)->rects);                                                               \
				(reg)->rects = &(reg)->extents;                                                   \
			}                                                                                     \
		} else if ((reg)->rects == &(reg)->extents) {                                             \
			(reg)->rects = (OGdkRegionBox*)malloc(nRects * sizeof(OGdkRegionBox));                \
			(reg)->rects[0] = (reg)->extents;                                                     \
		} else                                                                                    \
			(reg)->rects = (OGdkRegionBox*)realloc((reg)->rects, sizeof(OGdkRegionBox) * nRects); \
		(reg)->size = (nRects);                                                                   \
	}

/*
 *   Check to see if there is enough memory in the present region.
 */
#define OMEMCHECK(reg, rect, firstrect)             \
	{                                               \
		if ((reg)->numRects >= ((reg)->size - 1)) { \
			OGROWREGION(reg, 2 * (reg)->size);      \
			(rect) = &(firstrect)[(reg)->numRects]; \
		}                                           \
	}

#ifndef MIN
#define MIN(a, b) wxMin(a, b)
#endif

#ifndef MAX
#define MAX(a, b) wxMax(a, b)
#endif

/*
 *  In scan converting polygons, we want to choose those pixels
 *  which are inside the polygon.  Thus, we add .5 to the starting
 *  x coordinate for both left and right edges.  Now we choose the
 *  first pixel which is inside the pgon for the left edge and the
 *  first pixel which is outside the pgon for the right edge.
 *  Draw the left pixel, but not the right.
 *
 *  How to add .5 to the starting x coordinate:
 *      If the edge is moving to the right, then subtract dy from the
 *  error term from the general form of the algorithm.
 *      If the edge is moving to the left, then add dy to the error term.
 *
 *  The reason for the difference between edges moving to the left
 *  and edges moving to the right is simple:  If an edge is moving
 *  to the right, then we want the algorithm to flip immediately.
 *  If it is moving to the left, then we don't want it to flip until
 *  we traverse an entire pixel.
 */
#define OBRESINITPGON(dy, x1, x2, xStart, d, m, m1, incr1, incr2)       \
	{                                                                   \
		int dx; /* local storage */                                     \
                                                                        \
		/*                                                              \
		 *  if the edge is horizontal, then it is ignored               \
		 *  and assumed not to be processed.  Otherwise, do this stuff. \
		 */                                                             \
		if ((dy) != 0) {                                                \
			xStart = (x1);                                              \
			dx = (x2) - xStart;                                         \
			if (dx < 0) {                                               \
				m = dx / (dy);                                          \
				m1 = m - 1;                                             \
				incr1 = -2 * dx + 2 * (dy) * m1;                        \
				incr2 = -2 * dx + 2 * (dy) * m;                         \
				d = 2 * m * (dy) - 2 * dx - 2 * (dy);                   \
			} else {                                                    \
				m = dx / (dy);                                          \
				m1 = m + 1;                                             \
				incr1 = 2 * dx - 2 * (dy) * m1;                         \
				incr2 = 2 * dx - 2 * (dy) * m;                          \
				d = -2 * m * (dy) + 2 * dx;                             \
			}                                                           \
		}                                                               \
	}

#define OBRESINCRPGON(d, minval, m, m1, incr1, incr2) \
	{                                                 \
		if (m1 > 0) {                                 \
			if (d > 0) {                              \
				minval += m1;                         \
				d += incr1;                           \
			} else {                                  \
				minval += m;                          \
				d += incr2;                           \
			}                                         \
		} else {                                      \
			if (d >= 0) {                             \
				minval += m1;                         \
				d += incr1;                           \
			} else {                                  \
				minval += m;                          \
				d += incr2;                           \
			}                                         \
		}                                             \
	}

/*
 *     This structure contains all of the information needed
 *     to run the bresenham algorithm.
 *     The variables may be hardcoded into the declarations
 *     instead of using this structure to make use of
 *     register declarations.
 */
typedef struct
{
	int minor_axis; /* minor axis        */
	int d; /* decision variable */
	int m, m1; /* slope and slope+1 */
	int incr1, incr2; /* error increments */
} OBRESINFO;

#define OBRESINITPGONSTRUCT(dmaj, min1, min2, bres)                                       \
	OBRESINITPGON(dmaj, min1, min2, bres.minor_axis, bres.d, bres.m, bres.m1, bres.incr1, \
				  bres.incr2)

#define OBRESINCRPGONSTRUCT(bres) \
	OBRESINCRPGON(bres.d, bres.minor_axis, bres.m, bres.m1, bres.incr1, bres.incr2)

/*
 * for the winding number rule
 */
#define CLOCKWISE 1
#define COUNTERCLOCKWISE -1

typedef struct _OEdgeTableEntry
{
	int ymax; /* ycoord at which we exit this edge. */
	OBRESINFO bres; /* Bresenham info to run the edge     */
	struct _OEdgeTableEntry* next; /* next in the list     */
	struct _OEdgeTableEntry* back; /* for insertion sort   */
	struct _OEdgeTableEntry* nextWETE; /* for winding num rule */
	int ClockWise; /* flag for winding number rule       */
} OEdgeTableEntry;

typedef struct _OScanLineList
{
	int scanline; /* the scanline represented */
	OEdgeTableEntry* edgelist; /* header node              */
	struct _OScanLineList* next; /* next in the list       */
} OScanLineList;

typedef struct
{
	int ymax; /* ymax for the polygon     */
	int ymin; /* ymin for the polygon     */
	OScanLineList scanlines; /* header node              */
} OEdgeTable;

/*
 * Here is a struct to help with storage allocation
 * so we can allocate a big chunk at a time, and then take
 * pieces from this heap when we need to.
 */
#define SLLSPERBLOCK 25

typedef struct _OScanLineListBlock
{
	OScanLineList SLLs[SLLSPERBLOCK];
	struct _OScanLineListBlock* next;
} OScanLineListBlock;

/*
 *
 *     a few macros for the inner loops of the fill code where
 *     performance considerations don't allow a procedure call.
 *
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The winding number rule is in effect, so we must notify
 *     the caller when the edge has been removed so he
 *     can reorder the Winding Active Edge Table.
 */
#define OEVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET) \
	{                                                    \
		if (pAET->ymax == y) { /* leaving this edge */   \
			pPrevAET->next = pAET->next;                 \
			pAET = pPrevAET->next;                       \
			fixWAET = 1;                                 \
			if (pAET)                                    \
				pAET->back = pPrevAET;                   \
		} else {                                         \
			OBRESINCRPGONSTRUCT(pAET->bres);             \
			pPrevAET = pAET;                             \
			pAET = pAET->next;                           \
		}                                                \
	}

/*
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The even-odd rule is in effect.
 */
#define OEVALUATEEDGEEVENODD(pAET, pPrevAET, y)        \
	{                                                  \
		if (pAET->ymax == y) { /* leaving this edge */ \
			pPrevAET->next = pAET->next;               \
			pAET = pPrevAET->next;                     \
			if (pAET)                                  \
				pAET->back = pPrevAET;                 \
		} else {                                       \
			OBRESINCRPGONSTRUCT(pAET->bres);           \
			pPrevAET = pAET;                           \
			pAET = pAET->next;                         \
		}                                              \
	}

OGdkRegion* gdk_region_copy(const OGdkRegion* region);
void gdk_region_destroy(OGdkRegion* region);
OGdkRegion* gdk_region_rectangle(const OGdkRectangle* rectangle);
bool ogdk_region_equal(const OGdkRegion* region1, const OGdkRegion* region2);
bool gdk_region_point_in(const OGdkRegion* region, int x, int y);
OGdkOverlapType gdk_region_rect_in(const OGdkRegion* region, const OGdkRectangle* rectangle);
void gdk_region_offset(OGdkRegion* region, int dx, int dy);
void gdk_region_union(OGdkRegion* source1, const OGdkRegion* source2);
void gdk_region_intersect(OGdkRegion* source1, const OGdkRegion* source2);
OGdkRegion* gdk_region_polygon(const OGdkPoint* points, int n_points, OGdkFillRule fill_rule);

OGdkRegion* gdk_region_new(void);
void gdk_region_subtract(OGdkRegion* source1, const OGdkRegion* source2);
bool gdk_region_empty(const OGdkRegion* region);

void gdk_region_get_rectangles(const OGdkRegion* region, OGdkRectangle** rectangles,
							   int* n_rectangles);
void gdk_region_get_clipbox(const OGdkRegion* region, OGdkRectangle* rectangle);


// ----------------------------------------------------------------------------
// OCPNRegionRefData: private class containing the information about the region
// ----------------------------------------------------------------------------

class OCPNRegionRefData : public wxObjectRefData
{
public:
	OCPNRegionRefData()
		: m_region(NULL)
	{
	}

	OCPNRegionRefData(const OCPNRegionRefData& refData)
		: wxObjectRefData()
		, m_region(NULL)
	{
		m_region = gdk_region_copy(refData.m_region);
	}

	virtual ~OCPNRegionRefData()
	{
		if (m_region)
			gdk_region_destroy(m_region);
	}

	OGdkRegion* m_region;
};

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------

#define M_REGIONDATA ((OCPNRegionRefData *)m_refData)
#define M_REGIONDATA_OF(rgn) ((OCPNRegionRefData *)(rgn.m_refData))

IMPLEMENT_DYNAMIC_CLASS(OCPNRegion, wxGDIObject)

// ----------------------------------------------------------------------------
// OCPNRegion construction
// ----------------------------------------------------------------------------

#define M_REGIONDATA ((OCPNRegionRefData *)m_refData)

#ifndef USE_NEW_REGION
OCPNRegion::OCPNRegion(wxCoord x, wxCoord y, wxCoord w, wxCoord h)
	: wxRegion(x, y, w, h)
{
}

OCPNRegion::OCPNRegion(const wxPoint& topLeft, const wxPoint& bottomRight)
	: wxRegion(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y)
{
}

OCPNRegion::OCPNRegion(const wxRect& rect) : wxRegion(rect.x, rect.y, rect.width, rect.height)
{
}

#ifdef __WXOSX__
OCPNRegion::OCPNRegion(size_t n, const wxPoint* points, int fillStyle)
	: wxRegion(n, points, (wxPolygonFillMode)fillStyle)
{
}
#else
OCPNRegion::OCPNRegion(size_t n, const wxPoint* points, int fillStyle)
	: wxRegion(n, points, fillStyle)
{
}
#endif

wxRegion& OCPNRegion::ConvertTowxRegion()
{
	return *reinterpret_cast<wxRegion*>(this);
}


#endif

#ifdef USE_NEW_REGION

OCPNRegion::OCPNRegion(wxCoord x, wxCoord y, wxCoord w, wxCoord h)
{
	InitRect(x, y, w, h);
}

OCPNRegion::OCPNRegion(const wxPoint& topLeft, const wxPoint& bottomRight)
{
	InitRect(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y);
}

OCPNRegion::OCPNRegion(const wxRect& rect)
{
	InitRect(rect.x, rect.y, rect.width, rect.height);
}

void OCPNRegion::InitRect(wxCoord x, wxCoord y, wxCoord w, wxCoord h)
{
	OGdkRectangle rect;
	rect.x = x;
	rect.y = y;
	rect.width = w;
	rect.height = h;

	m_refData = new OCPNRegionRefData();

	M_REGIONDATA->m_region = gdk_region_rectangle(&rect);
}

OCPNRegion::OCPNRegion(size_t n, const wxPoint* points, int fillStyle)
{
	OGdkPoint* gdkpoints = new OGdkPoint[n];
	for (size_t i = 0; i < n; i++) {
		gdkpoints[i].x = points[i].x;
		gdkpoints[i].y = points[i].y;
	}

	m_refData = new OCPNRegionRefData();

	OGdkRegion* reg = gdk_region_polygon(
		gdkpoints, n, fillStyle == wxWINDING_RULE ? OGDK_WINDING_RULE : OGDK_EVEN_ODD_RULE);

	M_REGIONDATA->m_region = reg;

	delete[] gdkpoints;
}

wxObjectRefData* OCPNRegion::CreateRefData() const
{
	return new OCPNRegionRefData;
}

wxObjectRefData* OCPNRegion::CloneRefData(const wxObjectRefData* data) const
{
	return new OCPNRegionRefData(*(OCPNRegionRefData*)data);
}

// ----------------------------------------------------------------------------
// OCPNRegion comparison
// ----------------------------------------------------------------------------

bool OCPNRegion::ODoIsEqual(const OCPNRegion& region) const
{
	OGdkRegion* a = ((OCPNRegionRefData*)m_refData)->m_region;

	if (!region.m_refData)
		return false;

	OGdkRegion* b = ((OCPNRegionRefData*)(region.m_refData))->m_region;
	return ogdk_region_equal(M_REGIONDATA->m_region, M_REGIONDATA_OF(region)->m_region);
}

// ----------------------------------------------------------------------------
// OCPNRegion operations
// ----------------------------------------------------------------------------

void OCPNRegion::Clear()
{
	UnRef();
}

bool OCPNRegion::ODoUnionWithRect(const wxRect& r)
{
	// workaround for a strange GTK/X11 bug: taking union with an empty
	// rectangle results in an empty region which is definitely not what we
	// want
	if (r.IsEmpty())
		return true;

	if (!m_refData) {
		InitRect(r.x, r.y, r.width, r.height);
	} else {
		AllocExclusive();

		OGdkRectangle rect;
		rect.x = r.x;
		rect.y = r.y;
		rect.width = r.width;
		rect.height = r.height;
	}

	return true;
}

bool OCPNRegion::ODoUnionWithRegion(const OCPNRegion& region)
{
	wxCHECK_MSG(region.Ok(), false, _T("invalid region"));

	if (!m_refData) {
		m_refData = new OCPNRegionRefData();
		M_REGIONDATA->m_region = gdk_region_new();
	} else {
		AllocExclusive();
	}
	gdk_region_union(M_REGIONDATA->m_region, (OGdkRegion*)region.GetRegion());
	return true;
}

bool OCPNRegion::ODoIntersect(const OCPNRegion& region)
{
	wxCHECK_MSG(region.Ok(), false, _T("invalid region"));

	if (!m_refData) {
		// intersecting with invalid region doesn't make sense
		return false;
	}

	AllocExclusive();
	gdk_region_intersect(M_REGIONDATA->m_region, (OGdkRegion*)region.GetRegion());
	return true;
}

bool OCPNRegion::ODoSubtract(const OCPNRegion& region)
{
	wxCHECK_MSG(region.Ok(), false, _T("invalid region"));
	if (!m_refData) {
		// subtracting from an invalid region doesn't make sense
		return false;
	}

	AllocExclusive();
	gdk_region_subtract(M_REGIONDATA->m_region, (OGdkRegion*)region.GetRegion());
	return true;
}

bool OCPNRegion::DoXor(const OCPNRegion& region)
{
	wxCHECK_MSG(region.Ok(), false, _T("invalid region"));

	if (!m_refData) {
		return false;
	}

	AllocExclusive();
	return true;
}

bool OCPNRegion::ODoOffset(wxCoord x, wxCoord y)
{
	if (!m_refData)
		return false;

	AllocExclusive();
	gdk_region_offset(M_REGIONDATA->m_region, x, y);
	return true;
}

// ----------------------------------------------------------------------------
// OCPNRegion tests
// ----------------------------------------------------------------------------

bool OCPNRegion::ODoGetBox(wxCoord& x, wxCoord& y, wxCoord& w, wxCoord& h) const
{
	if (m_refData) {
		OGdkRectangle rect;
		gdk_region_get_clipbox(M_REGIONDATA->m_region, &rect);
		x = rect.x;
		y = rect.y;
		w = rect.width;
		h = rect.height;
		return true;
	} else {
		x = 0;
		y = 0;
		w = -1;
		h = -1;
		return false;
	}
}

bool OCPNRegion::IsEmpty() const
{
	if (!m_refData)
		return true;

	return gdk_region_empty(M_REGIONDATA->m_region);
}

wxRegionContain OCPNRegion::ODoContainsPoint(wxCoord x, wxCoord y) const
{
	if (!m_refData)
		return wxOutRegion;

	if (gdk_region_point_in(M_REGIONDATA->m_region, x, y))
		return wxInRegion;
	else
		return wxOutRegion;
}

wxRegionContain OCPNRegion::ODoContainsRect(const wxRect& r) const
{
	if (!m_refData)
		return wxOutRegion;

	OGdkRectangle rect;
	rect.x = r.x;
	rect.y = r.y;
	rect.width = r.width;
	rect.height = r.height;
	OGdkOverlapType res = gdk_region_rect_in(M_REGIONDATA->m_region, &rect);
	switch (res) {
		case OGDK_OVERLAP_RECTANGLE_IN:
			return wxInRegion;
		case OGDK_OVERLAP_RECTANGLE_OUT:
			return wxOutRegion;
		case OGDK_OVERLAP_RECTANGLE_PART:
			return wxPartRegion;
	}

	return wxOutRegion;
}

void* OCPNRegion::GetRegion() const
{
	if (!m_refData)
		return NULL;

	return M_REGIONDATA->m_region;
}

wxRegion& OCPNRegion::ConvertTowxRegion()
{
	wxRegion* r = new wxRegion;

	OGdkRectangle* gdkrects = NULL;
	int numRects = 0;
	gdk_region_get_rectangles((OGdkRegion*)GetRegion(), &gdkrects, &numRects);

	if (numRects) {
		for (int i = 0; i < numRects; ++i) {
			OGdkRectangle& gr = gdkrects[i];

			wxRect wr;
			wr.x = gr.x;
			wr.y = gr.y;
			wr.width = gr.width;
			wr.height = gr.height;

			r->Union(wr);
		}
	}
	free(gdkrects);
	return *r;
}

#endif

