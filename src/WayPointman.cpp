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

#include "WayPointman.h"
#include "RoutePoint.h"
#include "MarkIcon.h"
#include "Routeman.h"
#include "Select.h"
#include "navutil.h"

#include <wx/imaglist.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(RoutePointList);

extern ocpnStyle::StyleManager * g_StyleManager;
extern wxString g_PrivateDataDir;
extern WayPointman * pWayPointMan;
extern RoutePoint * pAnchorWatchPoint1;
extern RoutePoint * pAnchorWatchPoint2;
extern Routeman * g_pRouteMan;
extern MyConfig * pConfig;
extern Select * pSelect;

WayPointman::WayPointman()
{
	m_pWayPointList = new RoutePointList;

	pmarkicon_image_list = NULL;

	ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
	m_pIconArray = new wxArrayPtrVoid();
	ProcessIcons( style );

	// Load user defined icons.

	wxString UserIconPath = g_PrivateDataDir;
	wxChar sep = wxFileName::GetPathSeparator();
	if( UserIconPath.Last() != sep ) UserIconPath.Append( sep );
	UserIconPath.Append( _T("UserIcons") );

	if( wxDir::Exists( UserIconPath ) ) {
		wxArrayString FileList;

		wxDir dir( UserIconPath );
		int n_files = dir.GetAllFiles( UserIconPath, &FileList );

		for( int ifile = 0; ifile < n_files; ifile++ ) {
			wxString name = FileList.Item( ifile );

			wxFileName fn( name );
			wxString iconname = fn.GetName();
			wxBitmap icon1;

			if( fn.GetExt().Lower() == _T("xpm") ) {
				if( icon1.LoadFile( name, wxBITMAP_TYPE_XPM ) ) {
					ProcessIcon( icon1, iconname, iconname );
				}
			}
			if( fn.GetExt().Lower() == _T("png") ) {
				if( icon1.LoadFile( name, wxBITMAP_TYPE_PNG ) ) {
					ProcessIcon( icon1, iconname, iconname );
				}
			}
		}
	}

	m_nGUID = 0;
}

WayPointman::~WayPointman()
{
	//    Two step here, since the RoutePoint dtor also touches the
	//    RoutePoint list.
	//    Copy the master RoutePoint list to a temporary list,
	//    then clear and delete objects from the temp list

	RoutePointList temp_list;

	wxRoutePointListNode *node = m_pWayPointList->GetFirst();
	while( node ) {
		RoutePoint *pr = node->GetData();

		temp_list.Append( pr );
		node = node->GetNext();
	}

	temp_list.DeleteContents( true );
	temp_list.Clear();

	m_pWayPointList->Clear();
	delete m_pWayPointList;

	for( unsigned int i = 0; i < m_pIconArray->GetCount(); i++ ) {
		MarkIcon *pmi = (MarkIcon *) m_pIconArray->Item( i );
		delete pmi->picon_bitmap;
		delete pmi;
	}

	m_pIconArray->Clear();
	delete m_pIconArray;

	if( pmarkicon_image_list ) pmarkicon_image_list->RemoveAll();
	delete pmarkicon_image_list;
}

void WayPointman::ProcessIcons( ocpnStyle::Style* style )
{
	ProcessIcon( style->GetIcon( _T("empty") ), _T("empty"), _T("Empty") );
	ProcessIcon( style->GetIcon( _T("airplane") ), _T("airplane"), _T("Airplane") );
	ProcessIcon( style->GetIcon( _T("anchorage") ), _T("anchorage"), _T("Anchorage") );
	ProcessIcon( style->GetIcon( _T("anchor") ), _T("anchor"), _T("Anchor") );
	ProcessIcon( style->GetIcon( _T("boarding") ), _T("boarding"), _T("Boarding Location") );
	ProcessIcon( style->GetIcon( _T("boundary") ), _T("boundary"), _T("Boundary Mark") );
	ProcessIcon( style->GetIcon( _T("bouy1") ), _T("bouy1"), _T("Bouy Type A") );
	ProcessIcon( style->GetIcon( _T("bouy2") ), _T("bouy2"), _T("Bouy Type B") );
	ProcessIcon( style->GetIcon( _T("campfire") ), _T("campfire"), _T("Campfire") );
	ProcessIcon( style->GetIcon( _T("camping") ), _T("camping"), _T("Camping Spot") );
	ProcessIcon( style->GetIcon( _T("coral") ), _T("coral"), _T("Coral") );
	ProcessIcon( style->GetIcon( _T("fishhaven") ), _T("fishhaven"), _T("Fish Haven") );
	ProcessIcon( style->GetIcon( _T("fishing") ), _T("fishing"), _T("Fishing Spot") );
	ProcessIcon( style->GetIcon( _T("fish") ), _T("fish"), _T("Fish") );
	ProcessIcon( style->GetIcon( _T("float") ), _T("float"), _T("Float") );
	ProcessIcon( style->GetIcon( _T("food") ), _T("food"), _T("Food") );
	ProcessIcon( style->GetIcon( _T("fuel") ), _T("fuel"), _T("Fuel") );
	ProcessIcon( style->GetIcon( _T("greenlite") ), _T("greenlite"), _T("Green Light") );
	ProcessIcon( style->GetIcon( _T("kelp") ), _T("kelp"), _T("Kelp") );
	ProcessIcon( style->GetIcon( _T("light") ), _T("light1"), _T("Light Type A") );
	ProcessIcon( style->GetIcon( _T("light1") ), _T("light"), _T("Light Type B") );
	ProcessIcon( style->GetIcon( _T("litevessel") ), _T("litevessel"), _T("Light Vessel") );
	ProcessIcon( style->GetIcon( _T("mob") ), _T("mob"), _T("MOB") );
	ProcessIcon( style->GetIcon( _T("mooring") ), _T("mooring"), _T("Mooring Bouy") );
	ProcessIcon( style->GetIcon( _T("oilbouy") ), _T("oilbouy"), _T("Oil Bouy") );
	ProcessIcon( style->GetIcon( _T("platform") ), _T("platform"), _T("Platform") );
	ProcessIcon( style->GetIcon( _T("redgreenlite") ), _T("redgreenlite"), _T("Red/Green Light") );
	ProcessIcon( style->GetIcon( _T("redlite") ), _T("redlite"), _T("Red Light") );
	ProcessIcon( style->GetIcon( _T("rock1") ), _T("rock1"), _T("Rock (exposed)") );
	ProcessIcon( style->GetIcon( _T("rock2") ), _T("rock2"), _T("Rock, (awash)") );
	ProcessIcon( style->GetIcon( _T("sand") ), _T("sand"), _T("Sand") );
	ProcessIcon( style->GetIcon( _T("scuba") ), _T("scuba"), _T("Scuba") );
	ProcessIcon( style->GetIcon( _T("shoal") ), _T("shoal"), _T("Shoal") );
	ProcessIcon( style->GetIcon( _T("snag") ), _T("snag"), _T("Snag") );
	ProcessIcon( style->GetIcon( _T("square") ), _T("square"), _T("Square") );
	ProcessIcon( style->GetIcon( _T("triangle") ), _T("triangle"), _T("Triangle") );
	ProcessIcon( style->GetIcon( _T("diamond") ), _T("diamond"), _T("Diamond") );
	ProcessIcon( style->GetIcon( _T("circle") ), _T("circle"), _T("Circle") );
	ProcessIcon( style->GetIcon( _T("wreck1") ), _T("wreck1"), _T("Wreck A") );
	ProcessIcon( style->GetIcon( _T("wreck2") ), _T("wreck2"), _T("Wreck B") );
	ProcessIcon( style->GetIcon( _T("xmblue") ), _T("xmblue"), _T("Blue X") );
	ProcessIcon( style->GetIcon( _T("xmgreen") ), _T("xmgreen"), _T("Green X") );
	ProcessIcon( style->GetIcon( _T("xmred") ), _T("xmred"), _T("Red X") );
	ProcessIcon( style->GetIcon( _T("activepoint") ), _T("activepoint"), _T("Active WP") );
}

void WayPointman::ProcessIcon(wxBitmap pimage, const wxString & key, const wxString & description)
{
	MarkIcon *pmi;

	bool newIcon = true;

	for( unsigned int i = 0; i < m_pIconArray->GetCount(); i++ ) {
		pmi = (MarkIcon *) m_pIconArray->Item( i );
		if( pmi->icon_name.IsSameAs( key ) ) {
			newIcon = false;
			delete pmi->picon_bitmap;
			break;
		}
	}

	if( newIcon ) {
		pmi = new MarkIcon;
		m_pIconArray->Add( (void *) pmi );
		pmi->icon_name = key;
		pmi->icon_description = description;
	}

	pmi->picon_bitmap = new wxBitmap( pimage );
}

wxImageList *WayPointman::Getpmarkicon_image_list( void )
{
	// First find the largest bitmap size
	int w = 0;
	int h = 0;

	MarkIcon *pmi;

	for( unsigned int i = 0; i < m_pIconArray->GetCount(); i++ ) {
		pmi = (MarkIcon *) m_pIconArray->Item( i );
		w = wxMax(w, pmi->picon_bitmap->GetWidth());
		h = wxMax(h, pmi->picon_bitmap->GetHeight());

		// toh, 10.09.29
		// User defined icons won't be displayed in the list if they are larger than 32x32 pixels (why???)
		// Work-around: limit size
		if( w > 32 ) w = 32;
		if( h > 32 ) h = 32;

	}

	// Build an image list large enough

	if( NULL != pmarkicon_image_list ) {
		pmarkicon_image_list->RemoveAll();
		delete pmarkicon_image_list;
	}
	pmarkicon_image_list = new wxImageList( w, h );

	// Add the icons
	for( unsigned int ii = 0; ii < m_pIconArray->GetCount(); ii++ ) {
		pmi = (MarkIcon *) m_pIconArray->Item( ii );
		wxImage icon_image = pmi->picon_bitmap->ConvertToImage();

		// toh, 10.09.29
		// After limiting size user defined icons will be cut off
		// Work-around: rescale in one or both directions
		int h0 = icon_image.GetHeight();
		int w0 = icon_image.GetWidth();

		wxImage icon_larger;
		if( h0 <= h && w0 <= w ) {
			// Resize & Center smaller icons in the bitmap, so menus won't look so weird.
			icon_larger = icon_image.Resize( wxSize( w, h ), wxPoint( (w-w0)/2, (h-h0)/2 ) );
		} else {
			// rescale in one or two directions to avoid cropping, then resize to fit to cell
			int h1 = h;
			int w1 = w;
			if( h0 > h ) w1 = wxRound( (double) w0 * ( (double) h / (double) h0 ) );

			else if( w0 > w ) h1 = wxRound( (double) h0 * ( (double) w / (double) w0 ) );

			icon_larger = icon_image.Rescale( w1, h1 );
			icon_larger = icon_larger.Resize( wxSize( w, h ), wxPoint( 0, 0 ) );
		}

		pmarkicon_image_list->Add( icon_larger );
	}

	m_markicon_image_list_base_count = pmarkicon_image_list->GetImageCount();

	// Create and add "x-ed out" icons,
	// Being careful to preserve (some) transparency
	for( unsigned int ii = 0; ii < m_pIconArray->GetCount(); ii++ ) {

		wxImage img = pmarkicon_image_list->GetBitmap( ii ).ConvertToImage() ;
		img.ConvertAlphaToMask( 128 );

		unsigned char r,g,b;
		img.GetOrFindMaskColour(&r, &g, &b);
		wxColour unused_color(r,g,b);

		wxBitmap bmp0( img );

		wxBitmap bmp(w, h, -1 );
		wxMemoryDC mdc( bmp );
		mdc.SetBackground( wxBrush( unused_color) );
		mdc.Clear();
		mdc.DrawBitmap( bmp0, 0, 0 );
		wxPen red(GetGlobalColor(_T( "URED" )), 2 );
		mdc.SetPen( red );
		int xm = bmp.GetWidth();
		int ym = bmp.GetHeight();
		mdc.DrawLine( 2, 2, xm-2, ym-2 );
		mdc.DrawLine( xm-2, 2, 2, ym-2 );
		mdc.SelectObject( wxNullBitmap );

		wxMask *pmask = new wxMask(bmp, unused_color);
		bmp.SetMask( pmask );

		pmarkicon_image_list->Add( bmp );
	}

	return pmarkicon_image_list;
}

wxBitmap *WayPointman::CreateDimBitmap( wxBitmap *pBitmap, double factor )
{
	wxImage img = pBitmap->ConvertToImage();
	int sx = img.GetWidth();
	int sy = img.GetHeight();

	wxImage new_img( img );

	for( int i = 0; i < sx; i++ ) {
		for( int j = 0; j < sy; j++ ) {
			if( !img.IsTransparent( i, j ) ) {
				new_img.SetRGB( i, j, (unsigned char) ( img.GetRed( i, j ) * factor ),
						(unsigned char) ( img.GetGreen( i, j ) * factor ),
						(unsigned char) ( img.GetBlue( i, j ) * factor ) );
			}
		}
	}

	wxBitmap *pret = new wxBitmap( new_img );

	return pret;

}

void WayPointman::SetColorScheme( ColorScheme cs )
{
	ProcessIcons( g_StyleManager->GetCurrentStyle() );

	//    Iterate on the RoutePoint list, requiring each to reload icon

	wxRoutePointListNode *node = m_pWayPointList->GetFirst();
	while( node ) {
		RoutePoint *pr = node->GetData();
		pr->ReLoadIcon();
		node = node->GetNext();
	}
}

bool WayPointman::DoesIconExist(const wxString & icon_key) const
{
	MarkIcon *pmi;
	unsigned int i;

	for( i = 0; i < m_pIconArray->GetCount(); i++ ) {
		pmi = (MarkIcon *) m_pIconArray->Item( i );
		if( pmi->icon_name.IsSameAs( icon_key ) ) return true;
	}

	return false;
}

wxBitmap *WayPointman::GetIconBitmap( const wxString& icon_key )
{
	wxBitmap *pret = NULL;
	MarkIcon *pmi = NULL;
	unsigned int i;

	for( i = 0; i < m_pIconArray->GetCount(); i++ ) {
		pmi = (MarkIcon *) m_pIconArray->Item( i );
		if( pmi->icon_name.IsSameAs( icon_key ) ) break;
	}

	if( i == m_pIconArray->GetCount() )              // key not found
	{
		for( i = 0; i < m_pIconArray->GetCount(); i++ ) {
			pmi = (MarkIcon *) m_pIconArray->Item( i );
			if( pmi->icon_name.IsSameAs( _T("circle") ) ) break;
		}
	}

	if( i == m_pIconArray->GetCount() )              // not found again
		pmi = (MarkIcon *) m_pIconArray->Item( 0 );       // use item 0

	pret = pmi->picon_bitmap;

	return pret;
}

wxBitmap *WayPointman::GetIconBitmap( int index )
{
	wxBitmap *pret = NULL;

	if( index >= 0 ) {
		MarkIcon *pmi = (MarkIcon *) m_pIconArray->Item( index );
		pret = pmi->picon_bitmap;
	}
	return pret;
}

wxString *WayPointman::GetIconDescription( int index )
{
	wxString *pret = NULL;

	if( index >= 0 ) {
		MarkIcon *pmi = (MarkIcon *) m_pIconArray->Item( index );
		pret = &pmi->icon_description;
	}
	return pret;
}

wxString *WayPointman::GetIconKey( int index )
{
	wxString *pret = NULL;

	if( index >= 0 ) {
		MarkIcon *pmi = (MarkIcon *) m_pIconArray->Item( index );
		pret = &pmi->icon_name;
	}
	return pret;
}

int WayPointman::GetIconIndex( const wxBitmap *pbm )
{
	unsigned int i;

	for( i = 0; i < m_pIconArray->GetCount(); i++ ) {
		MarkIcon *pmi = (MarkIcon *) m_pIconArray->Item( i );
		if( pmi->picon_bitmap == pbm ) break;
	}

	return i;                                           // index of base icon in the image list

}

int WayPointman::GetXIconIndex( const wxBitmap *pbm )
{
	unsigned int i;

	for( i = 0; i < m_pIconArray->GetCount(); i++ ) {
		MarkIcon *pmi = (MarkIcon *) m_pIconArray->Item( i );
		if( pmi->picon_bitmap == pbm ) break;
	}

	return i + m_markicon_image_list_base_count;        // index of "X-ed out" icon in the image list

}

//  Create the unique identifier

wxString WayPointman::CreateGUID( RoutePoint *pRP )
{
	//FIXME: this method is not needed at all (if GetUUID works...)
	/*wxDateTime now = wxDateTime::Now();
	  time_t ticks = now.GetTicks();
	  wxString GUID;
	  GUID.Printf(_T("%d-%d-%d-%d"), ((int)fabs(pRP->m_lat * 1e4)), ((int)fabs(pRP->m_lon * 1e4)), (int)ticks, m_nGUID);

	  m_nGUID++;

	  return GUID;*/
	return GpxDocument::GetUUID();
}

RoutePoint *WayPointman::FindRoutePointByGUID(const wxString &guid)
{
	wxRoutePointListNode *prpnode = pWayPointMan->m_pWayPointList->GetFirst();
	while( prpnode ) {
		RoutePoint *prp = prpnode->GetData();

		if( prp->m_GUID == guid ) return ( prp );

		prpnode = prpnode->GetNext(); //RoutePoint
	}

	return NULL;
}

RoutePoint *WayPointman::GetNearbyWaypoint( double lat, double lon, double radius_meters )
{
	//    Iterate on the RoutePoint list, checking distance

	wxRoutePointListNode *node = m_pWayPointList->GetFirst();
	while( node ) {
		RoutePoint *pr = node->GetData();

		double a = lat - pr->m_lat;
		double b = lon - pr->m_lon;
		double l = sqrt( ( a * a ) + ( b * b ) );

		if( ( l * 60. * 1852. ) < radius_meters ) return pr;

		node = node->GetNext();
	}
	return NULL;

}

RoutePoint *WayPointman::GetOtherNearbyWaypoint( double lat, double lon, double radius_meters,
		const wxString &guid )
{
	//    Iterate on the RoutePoint list, checking distance

	wxRoutePointListNode *node = m_pWayPointList->GetFirst();
	while( node ) {
		RoutePoint *pr = node->GetData();

		double a = lat - pr->m_lat;
		double b = lon - pr->m_lon;
		double l = sqrt( ( a * a ) + ( b * b ) );

		if( ( l * 60. * 1852. ) < radius_meters ) if( pr->m_GUID != guid ) return pr;

		node = node->GetNext();
	}
	return NULL;

}

void WayPointman::ClearRoutePointFonts( void )
{
	//    Iterate on the RoutePoint list, clearing Font pointers
	//    This is typically done globally after a font switch

	wxRoutePointListNode *node = m_pWayPointList->GetFirst();
	while( node ) {
		RoutePoint *pr = node->GetData();

		pr->m_pMarkFont = NULL;
		node = node->GetNext();
	}
}

bool WayPointman::SharedWptsExist()
{
	wxRoutePointListNode *node = m_pWayPointList->GetFirst();
	while( node ) {
		RoutePoint *prp = node->GetData();
		if (prp->m_bKeepXRoute && ( prp->m_bIsInRoute || prp->m_bIsInTrack || prp == pAnchorWatchPoint1 || prp == pAnchorWatchPoint2))
			return true;
		node = node->GetNext();
	}
	return false;
}

void WayPointman::DeleteAllWaypoints( bool b_delete_used )
{
	//    Iterate on the RoutePoint list, deleting all
	wxRoutePointListNode *node = m_pWayPointList->GetFirst();
	while( node ) {
		RoutePoint *prp = node->GetData();
		// if argument is false, then only delete non-route waypoints
		if( !prp->m_bIsInLayer && ( prp->m_IconName != _T("mob") )
				&& ( ( b_delete_used && prp->m_bKeepXRoute )
					|| ( ( !prp->m_bIsInRoute ) && ( !prp->m_bIsInTrack )
						&& !( prp == pAnchorWatchPoint1 ) && !( prp == pAnchorWatchPoint2 ) ) ) ) {
			DestroyWaypoint(prp);
			delete prp;
			node = m_pWayPointList->GetFirst();
		} else
			node = node->GetNext();
	}
	return;

}

void WayPointman::DestroyWaypoint( RoutePoint *pRp )
{
	if( pRp ) {
		// Get a list of all routes containing this point
		// and remove the point from them all
		wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining( pRp );
		if( proute_array ) {
			for( unsigned int ir = 0; ir < proute_array->GetCount(); ir++ ) {
				Route *pr = (Route *) proute_array->Item( ir );

				/*  FS#348
					if ( g_pRouteMan->GetpActiveRoute() == pr )            // Deactivate any route containing this point
					g_pRouteMan->DeactivateRoute();
				 */
				pr->RemovePoint( pRp );

			}

			//    Scrub the routes, looking for one-point routes
			for( unsigned int ir = 0; ir < proute_array->GetCount(); ir++ ) {
				Route *pr = (Route *) proute_array->Item( ir );
				if( pr->GetnPoints() < 2 ) {
					pConfig->m_bIsImporting = true;
					pConfig->DeleteConfigRoute( pr );
					g_pRouteMan->DeleteRoute( pr );
					pConfig->m_bIsImporting = false;
				}
			}

			delete proute_array;
		}

		// Now it is safe to delete the point
		pConfig->DeleteWayPoint( pRp );
		pSelect->DeleteSelectablePoint( pRp, SELTYPE_ROUTEPOINT );

		//TODO  FIXME
		// Some memory corruption occurs if the wp is deleted here.
		// To continue running OK, it is sufficient to simply remove the wp from the global list
		// This will leak, although called infrequently....
		//  12/15/10...Seems to occur only on MOB delete....

		if( NULL != pWayPointMan )
			pWayPointMan->m_pWayPointList->DeleteObject( pRp );

		//    The RoutePoint might be currently in use as an anchor watch point
		if( pRp == pAnchorWatchPoint1 ) pAnchorWatchPoint1 = NULL;
		if( pRp == pAnchorWatchPoint2 ) pAnchorWatchPoint2 = NULL;

	}
}

int WayPointman::GetNumIcons(void) const
{
	return m_pIconArray->Count();
}

