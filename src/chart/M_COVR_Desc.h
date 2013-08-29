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

#include "BoundingBox.h"
#include "OCPNRegion.h"
#include <wx/dynarray.h>

class wxFFileInputStream;
class ViewPort;
struct float_2Dpt;

class M_COVR_Desc
{
	public:
		M_COVR_Desc();
		~M_COVR_Desc();

		int GetWKBSize();
		bool WriteWKB(void *p);
		int ReadWKB(wxFFileInputStream & ifs);
		void Update(M_COVR_Desc *pmcd);
		OCPNRegion GetRegion(const ViewPort & vp, wxPoint * pwp);

		int m_cell_index;
		int m_object_id;
		int m_subcell;

		int m_nvertices;
		float_2Dpt * pvertices;
		int m_npub_year;
		double transform_WGS84_offset_x;
		double transform_WGS84_offset_y;
		double m_covr_lat_min;
		double m_covr_lat_max;
		double m_covr_lon_min;
		double m_covr_lon_max;
		double user_xoff;
		double user_yoff;

		BoundingBox m_covr_bbox;
		bool m_buser_offsets;
};

WX_DECLARE_OBJARRAY(M_COVR_Desc, Array_Of_M_COVR_Desc);
WX_DECLARE_OBJARRAY(M_COVR_Desc *, Array_Of_M_COVR_Desc_Ptr);
WX_DECLARE_LIST(M_COVR_Desc, List_Of_M_COVR_Desc);

#endif
