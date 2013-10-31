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

#include "OCPNOffsetListCtrl.h"
#include <chart/CM93OffsetDialog.h>

OCPNOffsetListCtrl::OCPNOffsetListCtrl(
		CM93OffsetDialog * parent,
		wxWindowID id,
		const wxPoint & pos,
		const wxSize & size,
		long style)
	: wxListCtrl(parent, id, pos, size, style)
{
	m_parent = parent;
}

OCPNOffsetListCtrl::~OCPNOffsetListCtrl()
{
}

wxString OCPNOffsetListCtrl::OnGetItemText(long item, long column) const
{
	wxString ret;
	const M_COVR_Desc& pmcd = m_parent->getCovrDesc(item);

	switch (column) {
		case tlCELL:
			ret.Printf(_T("%d"), pmcd.m_cell_index);
			if (((int)'0') == pmcd.m_subcell)
				ret.Prepend(_T("0"));
			else {
				char t = (char)pmcd.m_subcell;
				wxString p;
				p.Printf(_T("%c"), t);
				ret.Prepend(p);
			}

			break;
		case tlMCOVR:
			ret.Printf(_T("%d"), pmcd.m_object_id);
			break;

		case tlSCALE:
			ret = m_parent->m_selected_chart_scale_char;
			break;

		case tlXOFF:
			ret.Printf(_T("%g"), pmcd.transform_WGS84_offset_x);
			break;

		case tlYOFF:
			ret.Printf(_T("%g"), pmcd.transform_WGS84_offset_y);
			break;

		case tlUXOFF:
			ret.Printf(_T("%6.0f"), pmcd.user_xoff * pmcd.m_centerlat_cos);
			break;

		case tlUYOFF:
			ret.Printf(_T("%6.0f"), pmcd.user_yoff * pmcd.m_centerlat_cos);
			break;

		default:
			break;
	}
	return ret;
}

int OCPNOffsetListCtrl::OnGetItemColumnImage(long item, long column) const
{
	return -1;
}

