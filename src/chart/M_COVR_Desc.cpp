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

#include "M_COVR_Desc.h"
#include <ViewPort.h>
#include <MicrosoftCompatibility.h>

#include <geo/GeoRef.h>
#include <geo/Polygon.h>

#include <cmath>

#include <wx/wfstream.h>

namespace chart {

M_COVR_Desc::M_COVR_Desc()
	: m_cell_index(0)
	, m_object_id(0)
	, m_subcell(0)
	, m_nvertices(0)
	, pvertices(NULL)
	, m_npub_year(0)
	, transform_WGS84_offset_x(0.0)
	, transform_WGS84_offset_y(0.0)
	, m_covr_lat_min(0.0)
	, m_covr_lat_max(0.0)
	, m_covr_lon_min(0.0)
	, m_covr_lon_max(0.0)
	, user_xoff(0.0)
	, user_yoff(0.0)
	, m_centerlat_cos(1.0)
	, m_buser_offsets(false)
{
}

M_COVR_Desc::~M_COVR_Desc()
{
	delete[] pvertices;
	pvertices = NULL;
}

int M_COVR_Desc::GetWKBSize()
{
	int size = 0;

	size = sizeof(int); // size itself
	size += sizeof(int); // m_cell_index;
	size += sizeof(int); // m_object_id;
	size += sizeof(int); // m_subcell
	size += sizeof(int); // m_nvertices;
	size += m_nvertices * sizeof(geo::PointF);

	size += sizeof(int); // m_npub_year;
	size += 8 * sizeof(double); // all the rest

	return size;
}

bool M_COVR_Desc::WriteWKB(void* p)
{
	if (p) {
		int* pr = (int*)p;
		*pr++ = GetWKBSize();

		*pr++ = m_cell_index;
		*pr++ = m_object_id;
		*pr++ = m_subcell;

		*pr++ = m_nvertices;

		geo::PointF* pfo = (geo::PointF*)pr;
		const geo::PointF* pfi = pvertices;
		for (int i = 0; i < m_nvertices; i++)
			*pfo++ = *pfi++;

		int* pi = (int*)pfo;
		*pi++ = m_npub_year;

		double* pd = (double*)pi;
		*pd++ = transform_WGS84_offset_x;
		*pd++ = transform_WGS84_offset_y;
		*pd++ = m_covr_lat_min;
		*pd++ = m_covr_lat_max;
		*pd++ = m_covr_lon_min;
		*pd++ = m_covr_lon_max;

		double centerlat_cos = cos(((m_covr_lat_min + m_covr_lat_max) / 2.0) * M_PI / 180.0);

		*pd++ = user_xoff * centerlat_cos;
		*pd++ = user_yoff * centerlat_cos;
	}

	return true;
}

int M_COVR_Desc::ReadWKB(wxFFileInputStream& ifs)
{
	// Read the length of the WKB
	int length = 0;
	if (!ifs.Read(&length, sizeof(int)).Eof()) {
		ifs.Read(&m_cell_index, sizeof(int));
		ifs.Read(&m_object_id, sizeof(int));
		ifs.Read(&m_subcell, sizeof(int));

		ifs.Read(&m_nvertices, sizeof(int));

		pvertices = new geo::PointF[m_nvertices];

		ifs.Read(pvertices, m_nvertices * sizeof(geo::PointF));

		ifs.Read(&m_npub_year, sizeof(int));

		ifs.Read(&transform_WGS84_offset_x, sizeof(double));
		ifs.Read(&transform_WGS84_offset_y, sizeof(double));
		ifs.Read(&m_covr_lat_min, sizeof(double));
		ifs.Read(&m_covr_lat_max, sizeof(double));
		ifs.Read(&m_covr_lon_min, sizeof(double));
		ifs.Read(&m_covr_lon_max, sizeof(double));

		m_centerlat_cos = cos(((m_covr_lat_min + m_covr_lat_max) / 2.0) * M_PI / 180.0);

		ifs.Read(&user_xoff, sizeof(double));
		ifs.Read(&user_yoff, sizeof(double));

		user_xoff /= m_centerlat_cos;
		user_yoff /= m_centerlat_cos;

		if ((fabs(user_xoff) > 1.0) || (fabs(user_yoff) > 1.0))
			m_buser_offsets = true;
		else
			m_buser_offsets = false;

		m_covr_bbox
			= geo::BoundingBox(m_covr_lon_min, m_covr_lat_min, m_covr_lon_max, m_covr_lat_max);
	}
	return length;
}

OCPNRegion M_COVR_Desc::GetRegion(const ViewPort& vp, wxPoint* pwp) const
{
	const geo::PointF* p = pvertices;

	for (int ip = 0; ip < m_nvertices; ++ip) {
		double plon = p->x;
		if (fabs(plon - vp.longitude()) > 180.0) {
			if (plon > vp.longitude())
				plon -= 360.0;
			else
				plon += 360.0;
		}

		double easting;
		double northing;
		double epix;
		double npix;
		geo::toSM(geo::Position(p->y, plon + 360.0),
				  geo::Position(vp.latitude(), vp.longitude() + 360.0), &easting, &northing);

		easting -= user_xoff;
		northing -= user_yoff;

		epix = easting * vp.view_scale();
		npix = northing * vp.view_scale();

		pwp[ip].x = (int)round((vp.pix_width / 2) + epix);
		pwp[ip].y = (int)round((vp.pix_height / 2) - npix);

		p++;
	}

	return OCPNRegion(m_nvertices, pwp);
}

}

