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

#ifndef __CHART__M_COVR_DESC__H__
#define __CHART__M_COVR_DESC__H__

#include <geo/BoundingBox.h>
#include <OCPNRegion.h>

class wxFFileInputStream;
class ViewPort;

namespace geo { struct PointF; }

namespace chart {

class M_COVR_Desc // FIXME: change member data from public to private
{
public:
	/// This strucutre help to find a specific outline.
	/// It is intended to be used as function object in conjunction with
	/// standard algorithms.
	class Finder
	{
	public:
		Finder(int cell_index, int object_id, int subcell)
			: cell_index(cell_index)
			, object_id(object_id)
			, subcell(subcell)
		{
		}

		Finder(const M_COVR_Desc& desc)
			: cell_index(desc.m_cell_index)
			, object_id(desc.m_object_id)
			, subcell(desc.m_subcell)
		{
		}

		bool operator()(const M_COVR_Desc* desc) const
		{
			return desc
				&& (desc->m_cell_index == cell_index)
				&& (desc->m_object_id == object_id)
				&& (desc->m_subcell == subcell);
		}

	private:
		int cell_index;
		int object_id;
		int subcell;
	};

public:
	M_COVR_Desc();
	~M_COVR_Desc();

	int GetWKBSize();
	bool WriteWKB(void* p);
	int ReadWKB(wxFFileInputStream& ifs);
	void Update(M_COVR_Desc* pmcd);
	OCPNRegion GetRegion(const ViewPort& vp, wxPoint* pwp) const;

	int m_cell_index;
	int m_object_id;
	int m_subcell;

	int m_nvertices;
	geo::PointF* pvertices; // FIXME: use std container
	int m_npub_year;
	double transform_WGS84_offset_x;
	double transform_WGS84_offset_y;
	double m_covr_lat_min;
	double m_covr_lat_max;
	double m_covr_lon_min;
	double m_covr_lon_max;
	double user_xoff;
	double user_yoff;
	double m_centerlat_cos;

	geo::BoundingBox m_covr_bbox;
	bool m_buser_offsets;
};

}

#endif
