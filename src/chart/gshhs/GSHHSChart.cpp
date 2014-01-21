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

#include "GSHHSChart.h"
#include <ViewPort.h>
#include <ocpnDC.h>
#include <chart/gshhs/Projection.h>
#include <chart/gshhs/GshhsReader.h>

#include <global/OCPN.h>
#include <global/System.h>

#include <wx/log.h>

GSHHSChart::GSHHSChart()
	: proj(NULL)
	, reader(NULL)
{}

GSHHSChart::~GSHHSChart()
{
	if (proj)
		delete proj;
	if (reader)
		delete reader;
}

void GSHHSChart::SetColorScheme(global::ColorScheme scheme)
{
	land = wxColor(170, 175, 80);
	water = wxColor(170, 195, 240);

	float dim = 1.0;

	switch (scheme) {
		case global::GLOBAL_COLOR_SCHEME_DUSK:
			dim = 0.5;
			break;
		case global::GLOBAL_COLOR_SCHEME_NIGHT:
			dim = 0.25;
			break;
		default:
			return;
	}

	land.Set(land.Red() * dim, land.Green() * dim, land.Blue() * dim);
	water.Set(water.Red() * dim, water.Green() * dim, water.Blue() * dim);
}

void GSHHSChart::RenderViewOnDC(ocpnDC& dc, ViewPort& vp)
{
	if (!proj)
		proj = new Projection();
	proj->SetCenterInMap(vp.longitude(), vp.latitude());
	proj->SetScreenSize(vp.rv_rect.width, vp.rv_rect.height);

	// Calculate the horizontal extents in degrees for the enlarged rotated ViewPort
	ViewPort nvp = vp;
	nvp.SetRotationAngle(0.);
	nvp.pix_width = vp.rv_rect.width;
	nvp.pix_height = vp.rv_rect.height;

	geo::Position pos_ul = nvp.GetLLFromPix(wxPoint(0, 0));
	geo::Position pos_ur = nvp.GetLLFromPix(wxPoint(nvp.pix_width, nvp.pix_height));

	double lat_ul = pos_ul.lat();
	double lon_ul = pos_ul.lon();
	double lat_ur = pos_ur.lat();
	double lon_ur = pos_ur.lon();

	if (nvp.longitude() < 0.0) {
		if ((lon_ul > 0.0) && (lon_ur < 0.0)) {
			lon_ul -= 360.0;
		}
	} else {
		if ((lon_ul > 0.0) && (lon_ur < 0.0)) {
			lon_ur += 360.0;
		}
	}

	if (lon_ur < lon_ul) {
		lon_ur += 360.0;
	}

	if (lon_ur > 360.0) {
		lon_ur -= 360.0;
		lon_ul -= 360.0;
	}

	//  And set the scale for the gshhs renderer
	proj->SetScale((float)vp.rv_rect.width / (fabs(lon_ul - lon_ur)));

	if (!reader) {
		reader = new GshhsReader(proj);
		if (reader->GetPolyVersion() < 210 || reader->GetPolyVersion() > 220) {
			wxLogMessage(
				_T("GSHHS World chart files have wrong version. Found %ld, expected 210-220."),
				reader->GetPolyVersion());
		} else {
			wxLogMessage(_T("Background world map loaded from GSHHS datafiles found in: ")
						 + global::OCPN::get().sys().data().world_map_location);
		}
	}

	dc.SetBrush(wxBrush(water));
	dc.DrawRectangle(0, 0, vp.rv_rect.width, vp.rv_rect.height);

	reader->drawContinents(dc, proj, water, land);
	reader->drawBoundaries(dc, proj);
}

