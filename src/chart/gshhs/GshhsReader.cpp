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

#include "GshhsReader.h"
#include <chart/gshhs/GshhsPoint.h>
#include <chart/gshhs/GshhsPolyReader.h>
#include <chart/gshhs/GshhsPolygon.h>
#include <chart/gshhs/Projection.h>

#include <global/OCPN.h>
#include <global/System.h>

#include <wx/log.h>
#include <wx/file.h>
#include <wx/stopwatch.h>

#ifdef __WXMSW__
#pragma warning(disable: 4251)   // relates to std::string fpath
#endif

GshhsReader::GshhsReader( Projection* proj )
{
	int maxQualityAvailable = -1;
	int minQualityAvailable = -1;

	for( int i=0; i<5; i++ ) {
		qualityAvailable[i] = false;
		if( GshhsReader::gshhsFilesExists( i ) ) {
			qualityAvailable[i] = true;
			if( minQualityAvailable < 0 ) minQualityAvailable = i;
			maxQualityAvailable = i;
		}
	}

	if (maxQualityAvailable < 0) {
		wxLogMessage(_T("Unable to initialize background world map. No GSHHS datafiles found in ") + global::OCPN::get().sys().data().world_map_location);
	}

	int q = selectBestQuality( proj );
	if( ! qualityAvailable[q] ) {
		q = minQualityAvailable;
	}

	gshhsPoly_reader = new GshhsPolyReader( q );
	setProj( proj );

	for( int qual = 0; qual < 5; qual++ ) {
		lsPoly_boundaries[qual] = new std::vector<GshhsPolygon*>;
		lsPoly_rivers[qual] = new std::vector<GshhsPolygon*>;
	}
	LoadQuality( q );
}

int GshhsReader::ReadPolyVersion()
{
	return gshhsPoly_reader->ReadPolyVersion();
}

int GshhsReader::GetPolyVersion()
{
	return gshhsPoly_reader->GetPolyVersion();
}

GshhsReader::~GshhsReader()
{
	clearLists();
	delete gshhsPoly_reader;
}

void GshhsReader::setProj( Projection * p )
{
	gshhsPoly_reader->setProj( p );
}

void GshhsReader::clearLists()
{
	std::vector<GshhsPolygon*>::iterator itp;
	for( int qual = 0; qual < 5; qual++ ) {
		for( itp = lsPoly_boundaries[qual]->begin(); itp != lsPoly_boundaries[qual]->end();
				itp++ ) {
			delete *itp;
			*itp = NULL;
		}
		for( itp = lsPoly_rivers[qual]->begin(); itp != lsPoly_rivers[qual]->end(); itp++ ) {
			delete *itp;
			*itp = NULL;
		}

		lsPoly_boundaries[qual]->clear();
		lsPoly_rivers[qual]->clear();
		delete lsPoly_boundaries[qual];
		delete lsPoly_rivers[qual];
	}
}

wxString GshhsReader::getNameExtension( int quality )
{
	wxString ext;
	switch( quality ){
		case 0:
			ext = _T("c");
			break;
		case 1:
			ext = _T("l");
			break;
		case 2:
			ext = _T("i");
			break;
		case 3:
			ext = _T("h");
			break;
		case 4:
			ext = _T("f");
			break;
		default:
			ext = _T("l");
			break;
	}
	return ext;
}

wxString GshhsReader::getFileName_Land(int quality)
{
	wxString ext = GshhsReader::getNameExtension(quality);
	wxString fname = global::OCPN::get().sys().data().world_map_location + wxString::Format(_T("poly-%c-1.dat"), ext.GetChar(0));
	return fname;
}

wxString GshhsReader::getFileName_boundaries(int quality)
{
	wxString ext = GshhsReader::getNameExtension( quality );
	wxString fname = global::OCPN::get().sys().data().world_map_location + wxString::Format(_T("wdb_borders_%c.b"), ext.GetChar(0));
	return fname;
}

wxString GshhsReader::getFileName_rivers(int quality)
{
	wxString ext = GshhsReader::getNameExtension( quality );
	wxString fname = global::OCPN::get().sys().data().world_map_location + wxString::Format(_T("wdb_rivers_%c.b"), ext.GetChar(0));
	return fname;
}

//-----------------------------------------------------------------------
bool GshhsReader::gshhsFilesExists(int quality)
{
	if (!wxFile::Access(GshhsReader::getFileName_Land(quality), wxFile::read))
		return false;
	if (!wxFile::Access(GshhsReader::getFileName_boundaries(quality), wxFile::read))
		return false;
	if (!wxFile::Access(GshhsReader::getFileName_rivers(quality), wxFile::read))
		return false;
	return true;
}

//-----------------------------------------------------------------------
void GshhsReader::LoadQuality(int newQuality) // 5 levels: 0=low ... 4=full
{
	if (quality == newQuality)
		return;

	wxStopWatch perftimer;

	wxString fname;
	FILE *file;
	bool ok;

	quality = newQuality;
	if( quality < 0 ) quality = 0;
	else if( quality > 4 ) quality = 4;

	gshhsPoly_reader->InitializeLoadQuality( quality );

	if( lsPoly_boundaries[quality]->size() == 0 ) {
		fname = getFileName_boundaries( quality );
		file = fopen( fname.mb_str(), "rb" );

		if( file != NULL ) {
			ok = true;
			while( ok ) {
				GshhsPolygon *poly = new GshhsPolygon( file );

				ok = poly->isOk();
				if( ok )
					if( poly->getLevel() < 2 )
						lsPoly_boundaries[quality]->push_back( poly );
					else delete poly;
				else delete poly;
			}
			fclose( file );
		}
	}

	if( lsPoly_rivers[quality]->size() == 0 ) {
		fname = getFileName_rivers( quality );
		file = fopen( fname.mb_str(), "rb" );
		if( file != NULL ) {
			ok = true;
			while( ok ) {
				GshhsPolygon *poly = new GshhsPolygon( file );
				ok = poly->isOk();
				if( ok ) {
					lsPoly_rivers[quality]->push_back( poly );
				}
				else delete poly;
			}
			fclose( file );
		}
	}

	wxLogMessage( _T("Loading World Chart Q=%d in %ld ms."), quality, perftimer.Time());

}

//-----------------------------------------------------------------------
std::vector<GshhsPolygon*> & GshhsReader::getList_boundaries()
{
	return *lsPoly_boundaries[quality];
}
//-----------------------------------------------------------------------
std::vector<GshhsPolygon*> & GshhsReader::getList_rivers()
{
	return *lsPoly_rivers[quality];
}

//=====================================================================

int GshhsReader::GSHHS_scaledPoints( GshhsPolygon *pol, wxPoint *pts, double decx, Projection *proj )
{

	if( !proj->intersect( pol->west + decx, pol->east + decx, pol->south, pol->north ) ) {
		return 0;
	}

	// Remove small polygons.
	int a1, b1;
	int a2, b2;
	proj->map2screen( pol->west + decx, pol->north, &a1, &b1 );
	proj->map2screen( pol->east + decx, pol->south, &a2, &b2 );
	if( a1 == a2 && b1 == b2 ) {
		return 0;
	}

	double x, y;
	std::vector<GshhsPoint *>::iterator itp;
	int xx, yy, oxx = 0, oyy = 0;
	int j = 0;

	for( itp = ( pol->lsPoints ).begin(); itp != ( pol->lsPoints ).end(); itp++ ) {
		x = ( *itp )->lon + decx;
		y = ( *itp )->lat;
		// Adjust scale
		proj->map2screen( x, y, &xx, &yy );
		if( j == 0 || ( oxx != xx || oyy != yy ) )  // Remove close points
		{
			oxx = xx;
			oyy = yy;
			pts[j].x = xx;
			pts[j].y = yy;
			j++;
		}
	}

	return j;
}

//-----------------------------------------------------------------------
void GshhsReader::GsshDrawLines( ocpnDC &pnt, std::vector<GshhsPolygon*> &lst, Projection *proj,
		bool isClosed )
{
	std::vector<GshhsPolygon*>::iterator iter;
	GshhsPolygon *pol;
	wxPoint *pts = NULL;
	int i;
	int nbp;

	int nbmax = 10000;
	pts = new wxPoint[nbmax];
	assert( pts );

	for( i = 0, iter = lst.begin(); iter != lst.end(); iter++, i++ ) {
		pol = *iter;

		if( nbmax < pol->n + 2 ) {
			nbmax = pol->n + 2;
			delete[] pts;
			pts = new wxPoint[nbmax];
			assert( pts );
		}

		nbp = GSHHS_scaledPoints( pol, pts, 0, proj );
		if( nbp > 1 ) {
			if( pol->isAntarctic() ) {
				pts++;
				nbp -= 2;
				pnt.DrawLines( nbp, pts );
				pts--;
			} else {
				pnt.DrawLines( nbp, pts );
				if( isClosed ) pnt.DrawLine( pts[0].x, pts[0].y, pts[nbp - 1].x, pts[nbp - 1].y );
			}
		}

		nbp = GSHHS_scaledPoints( pol, pts, -360, proj );
		if( nbp > 1 ) {
			if( pol->isAntarctic() ) {
				pts++;
				nbp -= 2;
				pnt.DrawLines( nbp, pts );
				pts--;
			} else {
				pnt.DrawLines( nbp, pts );
				if( isClosed ) pnt.DrawLine( pts[0].x, pts[0].y, pts[nbp - 1].x, pts[nbp - 1].y );
			}
		}
	}
	delete[] pts;
}

//-----------------------------------------------------------------------
void GshhsReader::drawBackground( ocpnDC &pnt, Projection *proj, wxColor seaColor,
		wxColor backgroundColor )
{
	pnt.SetBrush( backgroundColor );
	pnt.SetPen( backgroundColor );
	pnt.DrawRectangle( 0, 0, proj->getW(), proj->getH() );

	pnt.SetBrush( seaColor );
	pnt.SetPen( seaColor );
	int x0, y0, x1, y1;
	proj->map2screen( 0, 90, &x0, &y0 );
	proj->map2screen( 0, -90, &x1, &y1 );

	pnt.DrawRectangle( 0, y0, proj->getW(), y1 - y0 );

}
//-----------------------------------------------------------------------
void GshhsReader::drawContinents( ocpnDC &pnt, Projection *proj, wxColor seaColor,
		wxColor landColor )
{
	LoadQuality( selectBestQuality( proj ) );
	gshhsPoly_reader->drawGshhsPolyMapPlain( pnt, proj, seaColor, landColor );
}

//-----------------------------------------------------------------------
void GshhsReader::drawSeaBorders( ocpnDC &pnt, Projection *proj )
{
	pnt.SetBrush( *wxTRANSPARENT_BRUSH );
	gshhsPoly_reader->drawGshhsPolyMapSeaBorders( pnt, proj );
}

//-----------------------------------------------------------------------
void GshhsReader::drawBoundaries( ocpnDC &pnt, Projection *proj )
{
	pnt.SetBrush( *wxTRANSPARENT_BRUSH );

	if( pnt.GetDC() ) {
		wxPen* pen = wxThePenList->FindOrCreatePen( *wxBLACK, 1, wxDOT );
		pnt.SetPen( *pen );
	} else {
		wxPen* pen = wxThePenList->FindOrCreatePen( wxColor( 0, 0, 0, 80 ), 1, wxDOT );
		pnt.SetPen( *pen );
	}
	GsshDrawLines( pnt, getList_boundaries(), proj, false );
}

//-----------------------------------------------------------------------
void GshhsReader::drawRivers( ocpnDC &pnt, Projection *proj )
{
	GsshDrawLines( pnt, getList_rivers(), proj, false );
}

//-----------------------------------------------------------------------
int GshhsReader::selectBestQuality( Projection *proj )
{
	int bestQuality = 0;
	if( proj->getCoefremp() > 60 ) bestQuality = 0;
	else if( proj->getCoefremp() > 1 ) bestQuality = 1;
	else if( proj->getCoefremp() > 0.05 ) bestQuality = 2;
	else if( proj->getCoefremp() > 0.005 ) bestQuality = 3;
	else bestQuality = 4;

	while( !qualityAvailable[bestQuality] ) {
		bestQuality--;
		if( bestQuality < 0 ) break;
	}

	if( bestQuality < 0 )
		for( int i=0; i<5; i++ )
			if( qualityAvailable[i] ) bestQuality = i;

	return bestQuality;
}


bool GshhsReader::crossing( QLineF traject, QLineF trajectWorld ) const
{
	return this->gshhsPoly_reader->crossing( traject, trajectWorld );
}

bool GshhsReader::crossing1(QLineF trajectWorld )
{
	return this->gshhsPoly_reader->crossing1(trajectWorld );
}

