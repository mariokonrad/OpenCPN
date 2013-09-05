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

#ifndef __CHART__GSHHS__GSHHSPOLYREADER__H__
#define __CHART__GSHHS__GSHHSPOLYREADER__H__

#include <wx/colour.h>
#include <chart/gshhs/QLineF.h>
#include <chart/gshhs/PolygonFileHeader.h>

class ocpnDC;
class Projection;
class GshhsPolyCell;

class GshhsPolyReader
{
	public:
		GshhsPolyReader( int quality );
		~GshhsPolyReader();

		void drawGshhsPolyMapPlain( ocpnDC &pnt, Projection *proj, wxColor seaColor,
				wxColor landColor );

		void drawGshhsPolyMapSeaBorders( ocpnDC &pnt, Projection *proj );

		void InitializeLoadQuality( int quality ); // 5 levels: 0=low ... 4=full
		bool crossing( QLineF traject, QLineF trajectWorld ) const;
		bool crossing1( QLineF trajectWorld );
		int currentQuality;
		void setProj( Projection * p )
		{
			this->proj = p;
		}
		int ReadPolyVersion();
		int GetPolyVersion() { return polyHeader.version; }

	private:
		FILE *fpoly;
		GshhsPolyCell * allCells[360][180];

		PolygonFileHeader polyHeader;

		bool my_intersects( QLineF line1, QLineF line2 ) const;
		void readPolygonFileHeader( FILE *polyfile, PolygonFileHeader *header );
		bool abortRequested;
		Projection * proj;
};

#endif
