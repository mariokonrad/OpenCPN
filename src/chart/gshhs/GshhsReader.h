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

#ifndef __CHART__GSHHS__GSHHSREADER__H__
#define __CHART__GSHHS__GSHHSREADER__H__

#include <decl_exp.h>
#include <ocpnDC.h>
#include <chart/gshhs/QLineF.h>
#include <wx/colour.h>
#include <string>
#include <vector>

class Projection;
class GshhsPolygon;
class GshhsPolyReader;

class DECL_EXP GshhsReader
{
	public:
		GshhsReader( Projection* proj );
		~GshhsReader();

		void drawBackground( ocpnDC &pnt, Projection *proj, wxColor seaColor, wxColor backgroundColor );
		void drawContinents( ocpnDC &pnt, Projection *proj, wxColor seaColor, wxColor landColor );

		void drawSeaBorders( ocpnDC &pnt, Projection *proj );
		void drawBoundaries( ocpnDC &pnt, Projection *proj );
		void drawRivers( ocpnDC &pnt, Projection *proj );

		int GetPolyVersion();

		static wxString getNameExtension( int quality );
		static wxString getFileName_boundaries( int quality );
		static wxString getFileName_rivers( int quality );
		static wxString getFileName_Land( int quality );
		static bool gshhsFilesExists( int quality );

		int getQuality() { return quality; }

		bool crossing( QLineF traject, QLineF trajectWorld ) const;
		bool crossing1( QLineF trajectWorld );
		void setProj( Projection * p );
		int ReadPolyVersion();
		bool qualityAvailable[6];

	private:
		int quality;  // 5 levels: 0=low ... 4=full
		void LoadQuality( int quality );
		int selectBestQuality( Projection *proj );

		std::string fpath;     // directory containing gshhs files

		GshhsPolyReader * gshhsPoly_reader;

		std::vector<GshhsPolygon*> * lsPoly_boundaries[5];
		std::vector<GshhsPolygon*> * lsPoly_rivers[5];

		std::vector<GshhsPolygon*> & getList_boundaries();
		std::vector<GshhsPolygon*> & getList_rivers();
		//-----------------------------------------------------

		int GSHHS_scaledPoints(GshhsPolygon *pol, wxPoint *pts, double decx, Projection *proj);

		void GsshDrawLines(ocpnDC &pnt, std::vector<GshhsPolygon*> &lst, Projection *proj, bool isClosed);
		void clearLists();
};

#endif
