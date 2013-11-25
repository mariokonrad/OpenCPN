/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#include "QuiltCandidate.h"
#include <ViewPort.h>
#include <chart/ChartTableEntry.h>
#include <chart/ChartFamily.h>
#include <chart/ChartDB.h>

extern ChartDB *ChartData; // FIXME: global data

QuiltCandidate::QuiltCandidate()
	: b_include(false)
	, b_eclipsed(false)
{}

OCPNRegion& QuiltCandidate::GetCandidateVPRegion(ViewPort& vp) // FIXME: really belongs here, it
															   // introduces many dependencies
{
	if (candidate_region.IsOk())
		return candidate_region;

	const ChartTableEntry& cte = ChartData->GetChartTableEntry(dbIndex);

	OCPNRegion screen_region(vp.rv_rect);

	// Special case for charts which extend around the world, or near to it
	// Mostly this means cm93....
	// Take the whole screen, clipped at +/- 80 degrees lat
	if (fabs(cte.GetLonMax() - cte.GetLonMin()) > 180.) {
		int n_ply_entries = 4;
		float ply[8];
		ply[0] = 80.;
		ply[1] = vp.GetBBox().GetMinX();
		ply[2] = 80.;
		ply[3] = vp.GetBBox().GetMaxX();
		ply[4] = -80.;
		ply[5] = vp.GetBBox().GetMaxX();
		ply[6] = -80.;
		ply[7] = vp.GetBBox().GetMinX();

		candidate_region = vp.GetVPRegionIntersect(screen_region, 4, &ply[0], cte.GetScale());
		return candidate_region;
	}

	// If the chart has an aux ply table, use it for finer region precision
	int nAuxPlyEntries = cte.GetnAuxPlyEntries();
	if (nAuxPlyEntries >= 1) {
		for (int ip = 0; ip < nAuxPlyEntries; ip++) {
			const float* pfp = cte.GetpAuxPlyTableEntry(ip);
			int nAuxPly = cte.GetAuxCntTableEntry(ip);

			candidate_region = vp.GetVPRegionIntersect(screen_region, nAuxPly, pfp, cte.GetScale());
		}
	} else {
		int n_ply_entries = cte.GetnPlyEntries();
		const float* pfp = cte.GetpPlyTable();

		// could happen with old database and some charts, e.g. SHOM 2381.kap
		if (n_ply_entries >= 3) {
			candidate_region
				= vp.GetVPRegionIntersect(screen_region, n_ply_entries, pfp, cte.GetScale());
		} else
			candidate_region = screen_region;
	}

	// Remove the NoCovr regions
	int nNoCovrPlyEntries = cte.GetnNoCovrPlyEntries();
	if (nNoCovrPlyEntries) {
		for (int ip = 0; ip < nNoCovrPlyEntries; ip++) {
			float* pfp = cte.GetpNoCovrPlyTableEntry(ip);
			int nNoCovrPly = cte.GetNoCovrCntTableEntry(ip);

			OCPNRegion t_region
				= vp.GetVPRegionIntersect(screen_region, nNoCovrPly, pfp, cte.GetScale());

			// We do a test removal of the NoCovr region.
			// If the result iz empty, it must be that the NoCovr region is
			// the full extent M_COVR(CATCOV=2) feature found in NOAA ENCs.
			// We ignore it.

			if (!t_region.IsEmpty()) {
				OCPNRegion test_region = candidate_region;
				test_region.Subtract(t_region);

				if (!test_region.IsEmpty())
					candidate_region = test_region;
			}
		}
	}

	// Another superbad hack....
	// Super small scale raster charts like bluemarble.kap usually cross the prime meridian
	// and Plypoints georef is problematic......
	// So, force full screen coverage in the quilt
	if ((cte.GetScale() > 90000000) && (cte.GetChartFamily() == chart::CHART_FAMILY_RASTER))
		candidate_region = screen_region;

	// Clip the region to the current viewport
	candidate_region.Intersect(vp.rv_rect);

	if (!candidate_region.IsOk())
		candidate_region = OCPNRegion(0, 0, 100, 100);

	return candidate_region;
}

