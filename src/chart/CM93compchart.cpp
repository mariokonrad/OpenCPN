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

#include "CM93compchart.h"
#include <chart/CM93Manager.h>
#include <chart/CM93Chart.h>
#include <chart/CM93Dictionary.h>
#include <chart/COVR_Set.h>
#include <chart/CM93OffsetDialog.h>
#include <chart/FindCM93Dictionary.h>
#include <CM93DSlide.h>
#include <OCPNRegionIterator.h>
#include <cutil.h>
#include <georef.h>
#include <MicrosoftCompatibility.h>
#include <wx/regex.h>

extern bool g_bDebugCM93; // FIXME
extern bool g_bShowCM93DetailSlider; // FIXME
extern int g_cm93_zoom_factor; // FIXME
extern int g_cm93detail_dialog_x; // FIXME
extern int g_cm93detail_dialog_y; // FIXME
extern CM93DSlide * pCM93DetailSlider; // FIXME
extern MyFrame * gFrame; // FIXME: through constructor?

// Answer the query: "Is there a cm93 cell at the specified scale which contains a given lat/lon?"
static bool Is_CM93Cell_Present ( wxString &fileprefix, double lat, double lon, int scale_index ) // FIXME: should be part of the class
{
	int scale;
	int dval;
	wxChar scale_char;

	switch (scale_index) {
		case 0: scale =  20000000; dval = 120; scale_char = 'Z'; break; // Z
		case 1: scale =   3000000; dval =  60; scale_char = 'A'; break; // A
		case 2: scale =   1000000; dval =  30; scale_char = 'B'; break; // B
		case 3: scale =    200000; dval =  12; scale_char = 'C'; break; // C
		case 4: scale =    100000; dval =   3; scale_char = 'D'; break; // D
		case 5: scale =     50000; dval =   1; scale_char = 'E'; break; // E
		case 6: scale =     20000; dval =   1; scale_char = 'F'; break; // F
		case 7: scale =      7500; dval =   1; scale_char = 'G'; break; // G
		default: scale = 20000000; dval = 120; scale_char = ' '; break;
	}

	int cellindex = Get_CM93_CellIndex(lat, lon, scale);

	//    Create the file name
	wxString file;

	int ilat = cellindex / 10000;
	int ilon = cellindex % 10000;

	int jlat = ( ( ( ilat - 30 ) / dval ) * dval ) + 30;        // normalize
	int jlon = ( ilon / dval ) * dval;

	int ilatroot = ( ( ( ilat - 30 ) / 60 ) * 60 ) + 30;
	int ilonroot = ( ilon / 60 ) * 60;

	wxString fileroot;
	fileroot.Printf ( _T ( "%04d%04d/" ), ilatroot, ilonroot );


	wxString sdir ( fileprefix );
	sdir += fileroot;
	sdir += scale_char;

	wxString tfile;
	tfile.Printf ( _T ( "?%03d%04d." ), jlat, jlon );
	tfile += scale_char;

	//    Validate that the directory exists, adjusting case if necessary
	if ( !::wxDirExists ( sdir ) )
	{
		wxString old_scalechar ( scale_char );
		wxString new_scalechar = old_scalechar.Lower();

		sdir = fileprefix;
		sdir += fileroot;
		sdir += new_scalechar;
	}


	if ( ::wxDirExists ( sdir ) )
	{
		wxDir dir ( sdir );

		wxArrayString file_array;
		int n_files = dir.GetAllFiles ( sdir, &file_array, tfile, wxDIR_FILES );

		if ( n_files )
			return true;

		else
		{

			//    Try with alternate case of m_scalechar
			wxString old_scalechar ( scale_char );
			wxString new_scalechar = old_scalechar.Lower();

			wxString tfile1;
			tfile1.Printf ( _T ( "?%03d%04d." ), jlat, jlon );
			tfile1 += new_scalechar;

			int n_files1 = dir.GetAllFiles ( sdir, &file_array, tfile1, wxDIR_FILES );

			return ( n_files1 > 0 );
		}
	}
	else
		return false;
}

cm93compchart::cm93compchart()
{
	m_ChartType = CHART_TYPE_CM93COMP;
	m_pDictComposite = NULL;

	//    Supply a default name for status bar field
	m_FullPath = _T ( "CM93" );

	//    Set the "Description", so that it paints nice on the screen
	m_Description = _T ( "CM93Composite" );

	m_SE = _T ( "" );
	m_datum_str = _T ( "WGS84" );
	m_SoundingsDatum = _T ( "Unknown" );


	for ( int i = 0 ; i < 8 ; i++ )
		m_pcm93chart_array[i] = NULL;

	m_pcm93chart_current = NULL;

	m_cmscale = -1;
	m_Chart_Skew = 0.0;

	m_pDummyBM = NULL;

	SetSpecialOutlineCellIndex ( 0, 0, 0 );
	m_pOffsetDialog = NULL;

	m_pcm93mgr = new cm93manager();
}

cm93compchart::~cm93compchart()
{
	for ( int i = 0 ; i < 8 ; i++ )
		delete m_pcm93chart_array[i];

	delete m_pDictComposite;
	delete m_pDummyBM;
	delete m_pcm93mgr;

}


InitReturn cm93compchart::Init ( const wxString& name, ChartInitFlag flags )
{
	m_FullPath = name;

	wxFileName fn ( name );

	wxString target;
	wxString path;

	wxString sep(wxFileName::GetPathSeparator());

	//    Verify that the passed file name exists
	if ( !fn.FileExists() )
	{
		// It may be a directory
		if( wxDir::Exists(name) )
		{
			target = name + sep;
			path = name + sep;
		}
		else {
			wxString msg ( _T ( "   CM93Composite Chart Init cannot find " ) );
			msg.Append ( name );
			wxLogMessage ( msg );
			return  INIT_FAIL_REMOVE;
		}
	}
	else              // its a file that exists
	{
		//    Get the cm93 cell database prefix
		path = fn.GetPath ( ( int ) ( wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME ) );

		//    Remove two subdirectories from the passed file name
		//    This will give a normal CM93 root
		wxFileName file_path ( path );
		file_path.RemoveLastDir();
		file_path.RemoveLastDir();

		target = file_path.GetPath ( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR );
	}

	m_prefixComposite = target;

	wxString msg ( _T ( "CM93Composite Chart Root is " ) );
	msg.Append ( m_prefixComposite );
	wxLogMessage ( msg );


	if ( flags == THUMB_ONLY )
	{
		//            SetColorScheme(cs, false);

		return INIT_OK;
	}


	if ( flags == HEADER_ONLY )
		return CreateHeaderData();


	//    Load the cm93 dictionary if necessary
	if ( !m_pDictComposite )
	{
		if ( !m_pDictComposite )                           // second try from the file
			m_pDictComposite = FindAndLoadDictFromDir ( path );

		if ( !m_pDictComposite )
		{
			wxLogMessage ( _T ( "   CM93Composite Chart Init cannot locate CM93 dictionary." ) );
			return INIT_FAIL_REMOVE;
		}
	}


	//    Set the color scheme
	SetColorScheme ( m_global_color_scheme, false );

	bReadyToRender = true;

	return INIT_OK;


}

void cm93compchart::Activate ( void )
{
#define CM93_ZOOM_FACTOR_MAX_RANGE 5 // FIXME: better solution (maybe over global infrastructure)

	if ( g_bShowCM93DetailSlider )
	{
		if ( !pCM93DetailSlider )
		{
			pCM93DetailSlider = new CM93DSlide(gFrame, -1 , 0, -CM93_ZOOM_FACTOR_MAX_RANGE, CM93_ZOOM_FACTOR_MAX_RANGE,
					wxPoint ( g_cm93detail_dialog_x, g_cm93detail_dialog_y ), wxDefaultSize,
					wxSIMPLE_BORDER , _T ( "cm93 Detail" ) );
		}

		//    Here is an ugly piece of code which prevents the slider from taking the keyboard focus
		//    Only seems to work for Windows.....
		pCM93DetailSlider->Disable();
		pCM93DetailSlider->Show();
		pCM93DetailSlider->Enable();
	}
}

void cm93compchart::Deactivate ( void )
{
	if ( pCM93DetailSlider )
	{
		pCM93DetailSlider-> Destroy();
		pCM93DetailSlider = NULL;
	}
}

int cm93compchart::GetCMScaleFromVP ( const ViewPort &vpt )
{
	static const double scale_breaks[] =
	{
		5000.,                  //G
		15000.,                 //F
		40000.,                 //E
		150000.,                //D
		300000.,                //C
		1000000.,               //B
		3000000.                //A
	};

	double scale_mpp = 3000 / vpt.view_scale_ppm;

	double scale_mpp_adj = scale_mpp;

	double scale_breaks_adj[7];

	for ( int i=0 ; i < 7 ; i++ )
		scale_breaks_adj[i] = scale_breaks[i];



	//    Completely intuitive exponential curve adjustment
	if ( g_cm93_zoom_factor )
	{
		double efactor = ( double ) ( g_cm93_zoom_factor ) * ( .176 / 7. );
		for ( int i=0 ; i < 7 ; i++ )
		{
			double efr = efactor * ( 7 - i );
			scale_breaks_adj[i] = scale_breaks[i] * pow ( 10., efr );
			if ( g_bDebugCM93 )
				printf ( "g_cm93_zoom_factor: %2d  efactor: %6g efr:%6g, scale_breaks[i]:%6g  scale_breaks_adj[i]: %6g\n",
						g_cm93_zoom_factor, efactor, efr, scale_breaks[i], scale_breaks_adj[i] );
		}
	}

	int cmscale_calc = 7;
	int brk_index = 0;
	while ( cmscale_calc > 0 )
	{
		if ( scale_mpp_adj < scale_breaks_adj[brk_index] )
			break;
		cmscale_calc--;
		brk_index++;
	}

	return cmscale_calc;
}

void cm93compchart::SetVPParms ( const ViewPort &vpt )
{
	m_vpt = vpt;                              // save a copy

	int cmscale = GetCMScaleFromVP ( vpt );         // First order calculation of cmscale

	m_cmscale = PrepareChartScale ( vpt, cmscale );

	//    Continuoesly update the composite chart edition date to the latest cell decoded
	if ( m_pcm93chart_array[cmscale] )
	{
		if ( m_pcm93chart_array[cmscale]->GetEditionDate().IsLaterThan ( m_EdDate ) )
			m_EdDate = m_pcm93chart_array[cmscale]->GetEditionDate();
	}
}

int cm93compchart::PrepareChartScale ( const ViewPort &vpt, int cmscale )
{

	if ( g_bDebugCM93 )
		printf ( "\non SetVPParms, cmscale:%d, %c\n", cmscale, ( char ) ( 'A' + cmscale -1 ) );

	wxChar ext;
	bool cellscale_is_useable = false;
	bool b_nochart = false;

	while ( !cellscale_is_useable )
	{
		//    Open the proper scale chart, if not already open
		while ( NULL == m_pcm93chart_array[cmscale] )
		{
			if ( Is_CM93Cell_Present ( m_prefixComposite, vpt.clat, vpt.clon, cmscale ) )
			{
				if ( g_bDebugCM93 )
					printf ( " chart %c at VP clat/clon is present\n", ( char ) ( 'A' + cmscale -1 ) );

				m_pcm93chart_array[cmscale] = new cm93chart();


				ext = ( wxChar ) ( 'A' + cmscale - 1 );
				if ( cmscale == 0 )
					ext = 'Z';

				wxString file_dummy = _T ( "CM93." );
				file_dummy << ext;

				m_pcm93chart_array[cmscale]->SetCM93Dict ( m_pDictComposite );
				m_pcm93chart_array[cmscale]->SetCM93Prefix ( m_prefixComposite );
				m_pcm93chart_array[cmscale]->SetCM93Manager ( m_pcm93mgr );

				m_pcm93chart_array[cmscale]->SetColorScheme ( m_global_color_scheme );
				m_pcm93chart_array[cmscale]->Init ( file_dummy, FULL_INIT );
			}
			else if ( cmscale == 0 )
			{
				//                        wxString msg;
				//                        msg.Printf ( _T ( "   CM93 finds no chart of any scale present at Lat/Lon  %g %g" ), vpt.clat, vpt.clon );
				//                        wxLogMessage ( msg );
				if ( g_bDebugCM93 )
					printf ( "   CM93 finds no chart of any scale present at Lat/Lon  %g %g\n", vpt.clat, vpt.clon );

				b_nochart = true;
				break;
			}

			else
			{
				cmscale--;                          // revert to larger scale if selected is not present
				if ( g_bDebugCM93 )
					printf ( " no %c scale chart present, adjusting cmscale to %c\n", ( char ) ( 'A' + cmscale ), ( char ) ( 'A' + cmscale -1 ) );
			}

		}



		m_pcm93chart_current = m_pcm93chart_array[cmscale];

		if ( b_nochart )
		{
			if ( g_bDebugCM93 )
				printf ( " b_nochart return\n" );

			m_pcm93chart_current = NULL;
			for ( int i = 0 ; i < 8 ; i++ )
				m_pcm93chart_array[i] = NULL;

			return cmscale;
		}

		if ( m_pcm93chart_current )
		{
			//    Pass the parameters to the proper scale chart
			//    Which will also load the needed cell(s)
			m_pcm93chart_current->SetVPParms ( vpt );

			//    Check to see if the viewpoint center is actually on the selected chart
			float yc = vpt.clat;
			float xc = vpt.clon;


			//    Bound the clon to 0-360. degrees
			while ( xc < 0 )
				xc += 360.;

			if ( xc > 360. )
				xc -= 360.;


			if ( !m_pcm93chart_current->GetCoverSet()->GetCoverCount() )
			{
				if ( g_bDebugCM93 )
					printf ( " chart %c has no M_COVR\n", ( char ) ( 'A' + cmscale -1 ) );
			}


			if ( m_pcm93chart_current->IsPointInLoadedM_COVR ( xc, yc ) )
			{
				if ( g_bDebugCM93 )
					printf ( " chart %c contains clat/clon\n", ( char ) ( 'A' + cmscale -1 ) );

				cellscale_is_useable = true;
				break;
			}

			//    This commented block assumed that scale 0 coverage is available worlwide.....
			//    Might not be so with partial CM93 sets
			/*
			   else if(cmscale == 0)
			   {
			   cellscale_is_useable = true;
			   break;
			   }
			 */

			else if ( vpt.b_quilt &&  vpt.b_FullScreenQuilt )
			{
				ViewPort vpa = vpt;
				ViewPort vp_positive = vpt;
				vp_positive.set_positive();

				covr_set *pcover = m_pcm93chart_current->GetCoverSet();
				if ( pcover )
				{
					bool boverlap = false;
					for ( unsigned int im=0 ; im < pcover->GetCoverCount() ; im++ )
					{
						M_COVR_Desc *mcd = pcover->GetCover ( im );

						if ( ! ( _OUT == vp_positive.GetBBox().Intersect ( mcd->m_covr_bbox ) ) || ! ( _OUT == vpa.GetBBox().Intersect ( mcd->m_covr_bbox ) ) )
						{
							boverlap = true;
							break;
						}
					}
					if ( boverlap )
						cellscale_is_useable = true;
				}
			}

			if ( !cellscale_is_useable )
			{
				if ( cmscale > 0 )
					cmscale--;        // revert to larger scale if the current scale cells do not contain VP
				else
					b_nochart = true;    // we have retired to scale 0, and still no chart coverage, so stop already...
				if ( g_bDebugCM93 )
					printf ( " VP is not in M_COVR, adjusting cmscale to %c\n", ( char ) ( 'A' + cmscale -1 ) );
			}
		}
	}

	return cmscale;
}

//    Populate the member bool array describing which chart scales are available at any location
void cm93compchart::FillScaleArray ( double lat, double lon )
{
	for ( int cmscale = 0 ; cmscale < 8 ; cmscale++ )
		m_bScale_Array[cmscale] = Is_CM93Cell_Present ( m_prefixComposite, lat, lon, cmscale );
}

//    These methods simply pass the called parameters to the currently active cm93chart


wxString cm93compchart::GetPubDate()
{
	wxString data;

	if ( NULL != m_pcm93chart_current )

		data.Printf ( _T ( "%4d" ), m_current_cell_pub_date );
	else
		data = _T ( "????" );
	return data;
}

int cm93compchart::GetNativeScale()
{
	if ( m_pcm93chart_current )
		return m_pcm93chart_current->GetNativeScale();
	else
		return ( int ) 1e8;
}

double cm93compchart::GetNormalScaleMin ( double canvas_scale_factor, bool b_allow_overzoom )
{
	//Adjust overzoom factor based on  b_allow_overzoom option setting
	double oz_factor;
	if ( b_allow_overzoom )
		oz_factor = 40.;
	else
		oz_factor = 4.;

	if ( m_pcm93chart_current )
	{
		if ( m_pcm93chart_current->m_last_vp.IsValid() )
			FillScaleArray ( m_pcm93chart_current->m_last_vp.clat,m_pcm93chart_current-> m_last_vp.clon );

		//    Find out what the smallest available scale is
		int cmscale = 7;
		while ( cmscale > 0 )
		{
			if ( m_bScale_Array[cmscale] )
				break;
			cmscale--;
		}


		//    And return a sensible minimum scale, allowing selected overzoom.
		switch ( cmscale )
		{
			case  0: return 20000000. / oz_factor;            // Z
			case  1: return 3000000.  / oz_factor;            // A
			case  2: return 1000000.  / oz_factor;            // B
			case  3: return 200000.   / oz_factor;            // C
			case  4: return 100000.   / oz_factor;            // D
			case  5: return 50000.    / oz_factor;            // E
			case  6: return 20000.    / oz_factor;            // F
			case  7: return 500.;                             // G
			default: return 500.     / oz_factor;
		}
	}
	else
		return 500.;
}

double cm93compchart::GetNormalScaleMax ( double canvas_scale_factor, int canvas_width )
{
	return ( 180. / 360. ) * PI  * 2 * ( WGS84_semimajor_axis_meters / ( canvas_width / canvas_scale_factor ) );
	//return 1.0e8;
}

void cm93compchart::GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion)
{
	OCPNRegion screen_region(0, 0, VPoint.pix_width, VPoint.pix_height);
	OCPNRegion ret = GetValidScreenCanvasRegion ( VPoint, screen_region );
	*pValidRegion = ret;
}



OCPNRegion cm93compchart::GetValidScreenCanvasRegion ( const ViewPort& VPoint, const OCPNRegion &ScreenRegion )
{
	OCPNRegion ret_region;

	ViewPort vp_positive = VPoint;
	vp_positive.set_positive();

	vp_positive.rotation = 0.0;

	if ( m_pcm93chart_current )
	{
		int chart_native_scale = m_pcm93chart_current->GetNativeScale();


		for ( unsigned int im=0 ; im < m_pcm93chart_current->m_pcovr_array_loaded.GetCount() ; im++ )
		{
			M_COVR_Desc *pmcd = ( m_pcm93chart_current->m_pcovr_array_loaded.Item ( im ) );

			//    We can make a quick test based on the bbox of the M_COVR and the bbox of the ViewPort
			BoundingBox rtwbb = pmcd->m_covr_bbox;
			wxPoint2DDouble rtw ( 360., 0. );
			rtwbb.Translate ( rtw );

			if ( ( vp_positive.GetBBox().Intersect ( pmcd->m_covr_bbox ) == _OUT ) &&
					( vp_positive.GetBBox().Intersect ( rtwbb ) == _OUT ) )
				continue;

			wxPoint *DrawBuf = m_pcm93chart_current->GetDrawBuffer ( pmcd->m_nvertices );

			OCPNRegion rgn_covr = vp_positive.GetVPRegionIntersect ( ScreenRegion, pmcd->m_nvertices, ( float * ) pmcd->pvertices, chart_native_scale, DrawBuf );

			ret_region.Union( rgn_covr );

		}

	}
	else
		ret_region.Union(OCPNRegion( 0, 0, 1,1 ));

	return ret_region;

}

bool cm93compchart::RenderRegionViewOnGL(const wxGLContext &glc, const ViewPort& VPoint, const OCPNRegion &Region)
{
	SetVPParms ( VPoint );

	if ( m_pOffsetDialog && m_pOffsetDialog->IsShown() )
		m_pOffsetDialog->UpdateMCOVRList ( VPoint );

	return DoRenderRegionViewOnGL ( glc, VPoint, Region );

}

bool cm93compchart::DoRenderRegionViewOnGL (const wxGLContext &glc, const ViewPort& VPoint, const OCPNRegion &Region )
{
	//      g_bDebugCM93 = true;

	//      CALLGRIND_START_INSTRUMENTATION

	if ( g_bDebugCM93 ) {
		printf ( "\nOn DoRenderRegionViewOnGL Ref scale is %d, %c %g\n", m_cmscale, ( char ) ( 'A' + m_cmscale -1 ), VPoint.view_scale_ppm );
		OCPNRegionIterator upd ( Region );
		while ( upd.HaveRects() )
		{
			wxRect rect = upd.GetRect();
			rect.Offset ( -VPoint.rv_rect.x, -VPoint.rv_rect.y );
			printf ( "   Region Rect:  %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height );
			upd.NextRect();
		}
	}


	ViewPort vp_positive = VPoint;
	vp_positive.set_positive();

	bool render_return = false;
	if ( m_pcm93chart_current )
	{
		m_pcm93chart_current->SetVPParms ( vp_positive );

		//    Check the current chart scale to see if it covers the requested region totally
		if ( VPoint.b_quilt )
		{
			OCPNRegion vpr_empty = Region;

			OCPNRegion chart_region =  GetValidScreenCanvasRegion ( vp_positive, Region );

			if ( g_bDebugCM93 )
			{
				printf ( "On DoRenderRegionViewOnGL : Intersecting Ref region rectangles\n" );
				OCPNRegionIterator upd ( chart_region );
				while ( upd.HaveRects() )
				{
					wxRect rect = upd.GetRect();
					rect.Offset ( -VPoint.rv_rect.x, -VPoint.rv_rect.y );
					printf ( "   Region Rect:  %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height );
					upd.NextRect();
				}
			}

			if ( !chart_region.IsEmpty() )
				vpr_empty.Subtract ( chart_region );

			if ( g_bDebugCM93 )
			{
				printf ( "On DoRenderRegionViewOnGL : Region rectangles to fill with smaller scale\n" );
				OCPNRegionIterator upd ( vpr_empty );
				while ( upd.HaveRects() )
				{
					wxRect rect = upd.GetRect();
					rect.Offset ( -VPoint.rv_rect.x, -VPoint.rv_rect.y );
					printf ( "   Region Rect:  %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height );
					upd.NextRect();
				}
			}


			if ( !vpr_empty.Empty() && m_cmscale )        // This chart scale does not fully cover the region
			{
				//    Save the current cm93 chart pointer for restoration later
				cm93chart *m_pcm93chart_save = m_pcm93chart_current;
				int cmscale_save = m_cmscale;

				int cmscale_next = m_cmscale;

				//    Render smaller scale cells the entire requested region is full
				while ( !vpr_empty.Empty() && cmscale_next )
				{
					//    get the next smaller scale chart
					cmscale_next--;
					m_cmscale = PrepareChartScale ( vp_positive, cmscale_next );

					if ( m_pcm93chart_current )
					{
						if ( g_bDebugCM93 )
							printf ( "  In DRRVOD,  add quilt patch at %d, %c\n", m_cmscale, ( char ) ( 'A' + m_cmscale -1 ) );


						OCPNRegion sscale_region = GetValidScreenCanvasRegion ( vp_positive, Region );

						//    Only need to render that part of the vp that is not yet full
						sscale_region.Intersect ( vpr_empty );

						if ( g_bDebugCM93 )
						{
							printf ( "On DoRenderRegionViewOnGL : sscale_region rectangles\n" );
							OCPNRegionIterator upd ( sscale_region );
							while ( upd.HaveRects() )
							{
								wxRect rect = upd.GetRect();
								rect.Offset ( -VPoint.rv_rect.x, -VPoint.rv_rect.y );
								printf ( "   Region Rect:  %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height );
								upd.NextRect();;
							}
						}

						render_return |= m_pcm93chart_current->RenderRegionViewOnGL ( glc, vp_positive, sscale_region );

						//    Update the remaining empty region
						if ( !sscale_region.IsEmpty() )
							vpr_empty.Subtract ( sscale_region );
					}

				}     // while

				// restore the base chart pointer
				m_pcm93chart_current = m_pcm93chart_save;
				m_cmscale = cmscale_save;

				if ( g_bDebugCM93 )
				{
					printf ( "On DoRenderRegionViewOnGL : Final (chart_region) rectangles\n" );
					OCPNRegionIterator upd ( chart_region );
					while ( upd.HaveRects() )
					{
						wxRect rect = upd.GetRect();
						rect.Offset ( -VPoint.rv_rect.x, -VPoint.rv_rect.y );
						printf ( "   Region Rect:  %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height );
						upd.NextRect();
					}
				}

				//    Finally, render the target scale chart
				if ( !chart_region.IsEmpty() )
					render_return |= m_pcm93chart_current->RenderRegionViewOnGL ( glc, vp_positive, chart_region );

			}
			else
				render_return = m_pcm93chart_current->RenderRegionViewOnGL ( glc, vp_positive, chart_region );

			m_Name = m_pcm93chart_current->GetName();

		}
		else  // Single chart mode
		{
			render_return = m_pcm93chart_current->RenderRegionViewOnGL ( glc, vp_positive, Region );
			m_Name = m_pcm93chart_current->GetLastFileName();
		}
	}



	//    Render the cm93 cell's M_COVR outlines if called for
	if ( m_cell_index_special_outline )
	{
		ocpnDC dc;
		covr_set *pcover = m_pcm93chart_current->GetCoverSet();

		for ( unsigned int im=0 ; im < pcover->GetCoverCount() ; im++ )
		{
			M_COVR_Desc *pmcd = pcover->GetCover ( im );
			if ( ( pmcd->m_cell_index == m_cell_index_special_outline ) &&
					( pmcd->m_object_id == m_object_id_special_outline ) &&
					( pmcd->m_subcell == m_subcell_special_outline ) )

			{
				//    Draw this MCD's represented outline

				//    Case:  vpBBox is completely inside the mcd box
				//                        if(!(_OUT == vp_positive.vpBBox.Intersect(pmcd->m_covr_bbox)) || !(_OUT == vp.vpBBox.Intersect(pmcd->m_covr_bbox)))
				{

					float_2Dpt *p = pmcd->pvertices;
					wxPoint *pwp = m_pcm93chart_current->GetDrawBuffer ( pmcd->m_nvertices );

					for ( int ip = 0 ; ip < pmcd->m_nvertices ; ip++ )
					{

						double plon = p->x;
						if ( fabs ( plon - VPoint.clon ) > 180. )
						{
							if ( plon > VPoint.clon )
								plon -= 360.;
							else
								plon += 360.;
						}


						double easting, northing, epix, npix;
						toSM ( p->y, plon + 360., VPoint.clat, VPoint.clon + 360, &easting, &northing );

						//    Outlines stored in MCDs are not adjusted for offsets
						//                                    easting -= pmcd->transform_WGS84_offset_x;
						easting -= pmcd->user_xoff;
						//                                    northing -= pmcd->transform_WGS84_offset_y;
						northing -= pmcd->user_yoff;

						epix = easting  * VPoint.view_scale_ppm;
						npix = northing * VPoint.view_scale_ppm;

						pwp[ip].x = ( int ) round ( ( VPoint.pix_width  / 2 ) + epix );
						pwp[ip].y = ( int ) round ( ( VPoint.pix_height / 2 ) - npix );

						p++;
					}

					bool btest = true;
					if ( btest )
					{
						wxPen pen ( wxTheColourDatabase->Find ( _T ( "YELLOW" ) ), 3);
						wxDash dash1[2];
						dash1[0] = 4; // Long dash
						dash1[1] = 4; // Short gap
						pen.SetStyle(wxUSER_DASH);
						pen.SetDashes( 2, dash1 );

						dc.SetPen ( pen );

						for ( int iseg=0 ; iseg < pmcd->m_nvertices-1 ; iseg++ )
						{

							int x0 = pwp[iseg].x;
							int y0 = pwp[iseg].y;
							int x1 = pwp[iseg+1].x;
							int y1 = pwp[iseg+1].y;

							ClipResult res = cohen_sutherland_line_clip_i ( &x0, &y0, &x1, &y1,
									0, VPoint.pix_width, 0, VPoint.pix_height );

							if ( res == Invisible )                                                 // Do not bother with segments that are invisible
								continue;

							dc.DrawLine ( x0, y0, x1, y1 );
						}
					}
				}
			}
		}
	}

	return render_return;
}



bool cm93compchart::RenderRegionViewOnDC ( wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region )
{
	SetVPParms ( VPoint );

	if ( m_pOffsetDialog && m_pOffsetDialog->IsShown() )
		m_pOffsetDialog->UpdateMCOVRList ( VPoint );

	return DoRenderRegionViewOnDC ( dc, VPoint, Region );
}

bool cm93compchart::RenderViewOnDC ( wxMemoryDC& dc, const ViewPort& VPoint )
{
	const OCPNRegion vpr ( 0,0,VPoint.pix_width, VPoint.pix_height );

	SetVPParms ( VPoint );

	return DoRenderRegionViewOnDC ( dc, VPoint, vpr );

}

int s_dc1;
bool cm93compchart::DoRenderRegionViewOnDC ( wxMemoryDC& dc, const ViewPort& VPoint, const OCPNRegion &Region )
{
	//      g_bDebugCM93 = true;

	//      CALLGRIND_START_INSTRUMENTATION
	if ( g_bDebugCM93 )
	{
		printf ( "\nOn DoRenderRegionViewOnDC Ref scale is %d, %c\n", m_cmscale, ( char ) ( 'A' + m_cmscale -1 ) );
		OCPNRegionIterator upd ( Region );
		while ( upd.HaveRects() )
		{
			wxRect rect = upd.GetRect();
			printf ( "   Region Rect:  %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height );
			upd.NextRect();;
		}
	}

	ViewPort vp_positive = VPoint;
	vp_positive.set_positive();

	bool render_return = false;
	if ( m_pcm93chart_current )
	{
		m_pcm93chart_current->SetVPParms ( vp_positive );

		//    Check the current chart scale to see if it covers the requested region totally
		if ( VPoint.b_quilt )
		{
			OCPNRegion vpr_empty = Region;

			OCPNRegion chart_region = GetValidScreenCanvasRegion ( vp_positive, Region );

			if ( g_bDebugCM93 )
			{
				printf ( "On DoRenderRegionViewOnDC : Intersecting Ref region rectangles\n" );
				OCPNRegionIterator upd ( chart_region );
				while ( upd.HaveRects() )
				{
					wxRect rect = upd.GetRect();
					printf ( "   Region Rect:  %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height );
					upd.NextRect();
				}
			}

			if ( !chart_region.IsEmpty() )
				vpr_empty.Subtract ( chart_region );


			if ( !vpr_empty.Empty() && m_cmscale )        // This chart scale does not fully cover the region
			{
				//    Render the target scale chart on a temp dc for safekeeping
#ifdef ocpnUSE_DIBSECTION
				OCPNMemDC temp_dc;
#else
				wxMemoryDC temp_dc;
#endif
				render_return = m_pcm93chart_current->RenderRegionViewOnDC ( temp_dc, vp_positive, chart_region );

				//    Save the current cm93 chart pointer for restoration later
				cm93chart *m_pcm93chart_save = m_pcm93chart_current;

				//    Prepare a blank quilt bitmap to build up the quilt upon
				//    We need to do this in order to avoid polluting any of the sub-chart cached bitmaps
				if ( m_pDummyBM )
				{
					if ( ( m_pDummyBM->GetWidth() != VPoint.rv_rect.width ) || ( m_pDummyBM->GetHeight() != VPoint.rv_rect.height ) )
					{
						delete m_pDummyBM;
						m_pDummyBM = NULL;
					}
				}
				if ( NULL == m_pDummyBM )
					m_pDummyBM = new wxBitmap ( VPoint.rv_rect.width, VPoint.rv_rect.height,-1 );

				//    Clear the quilt
#ifdef ocpnUSE_DIBSECTION
				OCPNMemDC dumm_dc;
#else
				wxMemoryDC dumm_dc;
#endif
				dumm_dc.SelectObject ( *m_pDummyBM );
				dumm_dc.SetBackground ( *wxBLACK_BRUSH );
				dumm_dc.Clear();

				int cmscale_next = m_cmscale;

				//    Render smaller scale cells onto a temporary DC, blitting the valid region onto the quilt dc until the region is full
				while ( !vpr_empty.Empty() && cmscale_next )
				{
					//    get the next smaller scale chart
					cmscale_next--;
					m_cmscale = PrepareChartScale ( vp_positive, cmscale_next );
#ifdef ocpnUSE_DIBSECTION
					OCPNMemDC build_dc;
#else
					wxMemoryDC build_dc;
#endif

					if ( m_pcm93chart_current )
					{
						if ( g_bDebugCM93 )
							printf ( "  In DRRVOD,  add quilt patch at %d, %c\n", m_cmscale, ( char ) ( 'A' + m_cmscale -1 ) );

						m_pcm93chart_current->RenderRegionViewOnDC ( build_dc, vp_positive, Region );

						OCPNRegion sscale_region = GetValidScreenCanvasRegion ( vp_positive, Region );

						//    Only need to render that part of the vp that is not yet full
						sscale_region.Intersect ( vpr_empty );

						//    Blit the smaller scale chart patch onto the target DC
						OCPNRegionIterator upd ( sscale_region );
						while ( upd.HaveRects() )
						{
							wxRect rect = upd.GetRect();
							dumm_dc.Blit ( rect.x, rect.y, rect.width, rect.height, &build_dc, rect.x, rect.y );
							upd.NextRect();
						}
						build_dc.SelectObject ( wxNullBitmap );          // safely unmap the bmp

						//    Update the remaining empty region
						if ( !sscale_region.IsEmpty() )
							vpr_empty.Subtract ( sscale_region );
					}

				}     // while

				//    Finally, Blit the target scale chart as saved on temp_dc to quilt dc
				OCPNRegionIterator updt ( chart_region );
				while ( updt.HaveRects() )
				{
					wxRect rect = updt.GetRect();
					dumm_dc.Blit ( rect.x, rect.y, rect.width, rect.height, &temp_dc, rect.x, rect.y );
					updt.NextRect();
				}
				temp_dc.SelectObject ( wxNullBitmap );          // safely unmap the base chart bmp


				// restore the base chart pointer
				m_pcm93chart_current = m_pcm93chart_save;

				//    And the return dc is the quilt
				dc.SelectObject ( *m_pDummyBM );

				render_return = true;
			}
			else {
				m_pcm93chart_current->RenderRegionViewOnDC ( dc, vp_positive, Region );
				render_return = true;
			}
			m_Name = m_pcm93chart_current->GetName();

		}
		else  // Single chart mode
		{
			render_return = m_pcm93chart_current->RenderRegionViewOnDC ( dc, vp_positive, Region );
			m_Name = m_pcm93chart_current->GetLastFileName();
		}

	}
	else
	{
		//    one must always return a valid bitmap selected into the specified DC
		//    Since the CM93 cell is not available at this location, select a dummy placeholder
		if ( m_pDummyBM )
		{
			if ( ( m_pDummyBM->GetWidth() != VPoint.pix_width ) || ( m_pDummyBM->GetHeight() != VPoint.pix_height ) )
			{
				delete m_pDummyBM;
				m_pDummyBM = NULL;
			}
		}

		if ( NULL == m_pDummyBM )
			m_pDummyBM = new wxBitmap ( VPoint.pix_width, VPoint.pix_height,-1 );


		// Clear the bitmap
		wxMemoryDC mdc;
		mdc.SelectObject ( *m_pDummyBM );
		mdc.SetBackground ( *wxBLACK_BRUSH );
		mdc.Clear();
		mdc.SelectObject ( wxNullBitmap );


		dc.SelectObject ( *m_pDummyBM );
	}

	//      CALLGRIND_STOP_INSTRUMENTATION

	//    Render the cm93 cell's M_COVR outlines if called for
	if ( m_cell_index_special_outline )
	{
		covr_set *pcover = m_pcm93chart_current->GetCoverSet();

		for ( unsigned int im=0 ; im < pcover->GetCoverCount() ; im++ )
		{
			M_COVR_Desc *pmcd = pcover->GetCover ( im );
			if ( ( pmcd->m_cell_index == m_cell_index_special_outline ) &&
					( pmcd->m_object_id == m_object_id_special_outline ) &&
					( pmcd->m_subcell == m_subcell_special_outline ) )

			{
				//    Draw this MCD's represented outline

				//    Case:  vpBBox is completely inside the mcd box
				//                        if(!(_OUT == vp_positive.vpBBox.Intersect(pmcd->m_covr_bbox)) || !(_OUT == vp.vpBBox.Intersect(pmcd->m_covr_bbox)))
				{

					float_2Dpt *p = pmcd->pvertices;
					wxPoint *pwp = m_pcm93chart_current->GetDrawBuffer ( pmcd->m_nvertices );

					for ( int ip = 0 ; ip < pmcd->m_nvertices ; ip++ )
					{

						double plon = p->x;
						if ( fabs ( plon - VPoint.clon ) > 180. )
						{
							if ( plon > VPoint.clon )
								plon -= 360.;
							else
								plon += 360.;
						}


						double easting, northing, epix, npix;
						toSM ( p->y, plon + 360., VPoint.clat, VPoint.clon + 360, &easting, &northing );

						//    Outlines stored in MCDs are not adjusted for offsets
						//                                    easting -= pmcd->transform_WGS84_offset_x;
						easting -= pmcd->user_xoff;
						//                                    northing -= pmcd->transform_WGS84_offset_y;
						northing -= pmcd->user_yoff;

						epix = easting  * VPoint.view_scale_ppm;
						npix = northing * VPoint.view_scale_ppm;

						pwp[ip].x = ( int ) round ( ( VPoint.pix_width  / 2 ) + epix );
						pwp[ip].y = ( int ) round ( ( VPoint.pix_height / 2 ) - npix );

						p++;
					}

					//    Scrub the points
					//   looking for segments for which the wrong longitude decision was made
					//    TODO all this mole needs to be rethought, again
					bool btest = true;
					/*
					   wxPoint p0 = pwp[0];
					   for(int ip = 1 ; ip < pmcd->m_nvertices ; ip++)
					   {
					//                                   if(((p0.x > VPoint.pix_width) && (pwp[ip].x < 0)) || ((p0.x < 0) && (pwp[ip].x > VPoint.pix_width)))
					//                                         btest = false;

					p0 = pwp[ip];
					}
					 */
					if ( btest )
					{
						dc.SetPen ( wxPen ( wxTheColourDatabase->Find ( _T ( "YELLOW" ) ), 4, wxLONG_DASH ) );

						for ( int iseg=0 ; iseg < pmcd->m_nvertices-1 ; iseg++ )
						{

							int x0 = pwp[iseg].x;
							int y0 = pwp[iseg].y;
							int x1 = pwp[iseg+1].x;
							int y1 = pwp[iseg+1].y;

							ClipResult res = cohen_sutherland_line_clip_i ( &x0, &y0, &x1, &y1,
									0, VPoint.pix_width, 0, VPoint.pix_height );

							if ( res == Invisible )                                                 // Do not bother with segments that are invisible
								continue;

							dc.DrawLine ( x0, y0, x1, y1 );
						}
					}
				}

			}
		}
	}

	return render_return;
}


void cm93compchart::UpdateRenderRegions ( const ViewPort& VPoint )
{
	OCPNRegion full_screen_region(0,0,VPoint.rv_rect.width, VPoint.rv_rect.height);

	ViewPort vp_positive = VPoint;
	vp_positive.set_positive();

	SetVPParms ( VPoint );

	if ( m_pcm93chart_current )
	{
		m_pcm93chart_current->SetVPParms ( vp_positive );

		//    Check the current chart scale to see if it covers the requested region totally
		if ( VPoint.b_quilt )
		{
			//    Clear all the subchart regions
			for ( int i = 0 ; i < 8 ; i++ )
			{
				if ( m_pcm93chart_array[i] )
					m_pcm93chart_array[i]->m_render_region.Clear();
			}

			OCPNRegion vpr_empty = full_screen_region;

			OCPNRegion chart_region = GetValidScreenCanvasRegion ( vp_positive, full_screen_region );
			m_pcm93chart_current->m_render_region = chart_region;       // update

			if ( !chart_region.IsEmpty() )
				vpr_empty.Subtract ( chart_region );


			if ( !vpr_empty.Empty() && m_cmscale )        // This chart scale does not fully cover the region
			{

				//    Save the current cm93 chart pointer for restoration later
				cm93chart *m_pcm93chart_save = m_pcm93chart_current;

				int cmscale_next = m_cmscale;

				while ( !vpr_empty.Empty() && cmscale_next )
				{
					//    get the next smaller scale chart
					cmscale_next--;
					m_cmscale = PrepareChartScale ( vp_positive, cmscale_next );

					if ( m_pcm93chart_current )
					{
						OCPNRegion sscale_region = GetValidScreenCanvasRegion ( vp_positive, full_screen_region );
						sscale_region.Intersect ( vpr_empty );
						m_pcm93chart_current->m_render_region = sscale_region;

						//    Update the remaining empty region
						if ( !sscale_region.IsEmpty() )
							vpr_empty.Subtract ( sscale_region );
					}

				}     // while

				// restore the base chart pointer
				m_pcm93chart_current = m_pcm93chart_save;
			}
		}
	}
}





void cm93compchart::SetSpecialCellIndexOffset ( int cell_index, int object_id, int subcell, int xoff, int yoff )
{
	m_special_offset_x = xoff;
	m_special_offset_y = yoff;

	if ( m_pcm93chart_current )
		m_pcm93chart_current->SetUserOffsets ( cell_index, object_id, subcell, xoff, yoff );
}

bool cm93compchart::RenderNextSmallerCellOutlines ( ocpnDC &dc, ViewPort& vp )
{
	ViewPort vp_positive = vp;
	vp_positive.set_positive();

	if ( m_cmscale < 7 )
	{
		//    Something like an effective true_scale
		double top_scale = vp.chart_scale * 0.25;

		int nss_max = m_cmscale;
		while ( nss_max < 7 )
		{
			double candidate_cell_scale;
			switch ( nss_max )
			{
				case  0: candidate_cell_scale = 20000000.; break;            // Z
				case  1: candidate_cell_scale =  3000000.; break;           // A
				case  2: candidate_cell_scale =  1000000.; break;            // B
				case  3: candidate_cell_scale =  200000. ; break;            // C
				case  4: candidate_cell_scale =  100000. ; break;            // D
				case  5: candidate_cell_scale =  50000.  ; break;            // E
				case  6: candidate_cell_scale =  20000.  ; break;            // F
				case  7: candidate_cell_scale =  7500.   ; break;           // G
				default: candidate_cell_scale =  10.;break;
			}

			if ( candidate_cell_scale < top_scale )
				break;
			nss_max ++;
		}

		nss_max = wxMax ( nss_max, m_cmscale+1 );

		if ( g_bDebugCM93 )
		{
			printf ( " RenderNextSmallerCellOutline, base chart scale is %c\n", ( char ) ( 'A' +m_cmscale - 1 ) );
			printf ( "    top_scale: %8.0f   VP.chart_scale: %8.0f\n", top_scale, vp.chart_scale );
			printf ( "    nss_max is %c\n", ( char ) ( 'A' +nss_max - 1 ) );
		}


		int nss = m_cmscale +1;

		//    A little magic here.
		//    Drawing all larger scale cell outlines is way too expensive.
		//    So, stop the loop after we have rendered "something"
		//    But don't stop at all if the viewport scale is less than 3 million.
		//    This will have the effect of bringing in outlines of isolated large scale cells
		//    embedded within small scale cells, like isolated islands in the Pacific.
		bool bdrawn = false;
		nss_max = 7;
		while ( nss <= nss_max && ( !bdrawn || ( vp.chart_scale < 3e6 ) ) )
		{
			cm93chart *psc = m_pcm93chart_array[nss];

			if ( !psc )
			{
				m_pcm93chart_array[nss] = new cm93chart();
				psc = m_pcm93chart_array[nss];

				wxChar ext = ( wxChar ) ( 'A' + nss - 1 );
				if ( nss == 0 )
					ext = 'Z';

				wxString file_dummy = _T ( "CM93." );
				file_dummy << ext;

				psc->SetCM93Dict ( m_pDictComposite );
				psc->SetCM93Prefix ( m_prefixComposite );
				psc->SetCM93Manager ( m_pcm93mgr );

				psc->SetColorScheme ( m_global_color_scheme );
				psc->Init ( file_dummy, FULL_INIT );
			}

			if ( ( psc ) && ( nss != 1 ) )       // skip rendering the A scale outlines
			{
				bool mcr = psc->UpdateCovrSet ( &vp );

				//    Render the chart outlines
				if ( mcr )
				{
					covr_set *pcover = psc->GetCoverSet();

					for ( unsigned int im=0 ; im < pcover->GetCoverCount() ; im++ )
					{
						M_COVR_Desc *mcd = pcover->GetCover ( im );



						//    Case:  vpBBox is completely inside the mcd box
						if ( ! ( _OUT == vp_positive.GetBBox().Intersect ( mcd->m_covr_bbox ) ) || ! ( _OUT == vp.GetBBox().Intersect ( mcd->m_covr_bbox ) ) )
						{

							float_2Dpt *p = mcd->pvertices;
							wxPoint *pwp = psc->GetDrawBuffer ( mcd->m_nvertices );

							for ( int ip = 0 ; ip < mcd->m_nvertices ; ip++ ,  p++)
							{

								pwp[ip] = vp_positive.GetPixFromLL( p->y, p->x );

								//    Outlines stored in MCDs are not adjusted for offsets
								pwp[ip].x -= mcd->user_xoff * vp.view_scale_ppm;
								pwp[ip].y -= mcd->user_yoff * vp.view_scale_ppm;

							}

							//    Scrub the points
							//    looking for segments for which the wrong longitude decision was made
							//    TODO all this mole needs to be rethought, again
							bool btest = true;
							wxPoint p0 = pwp[0];
							for ( int ip = 1 ; ip < mcd->m_nvertices ; ip++ )
							{
								if ( ( ( p0.x > vp.pix_width ) && ( pwp[ip].x < 0 ) ) || ( ( p0.x < 0 ) && ( pwp[ip].x > vp.pix_width ) ) )
									btest = false;

								p0 = pwp[ip];
							}

							if ( btest )
							{
								dc.DrawLines ( mcd->m_nvertices, pwp, 0, 0, false );
								bdrawn = true;
							}
						}
					}
				}
			}
			nss ++;
		}
	}


	return true;
}


void cm93compchart::GetPointPix ( ObjRazRules *rzRules, float rlat, float rlon, wxPoint *r )
{
	m_pcm93chart_current->GetPointPix ( rzRules, rlat, rlon, r );
}

void cm93compchart::GetPointPix ( ObjRazRules *rzRules, wxPoint2DDouble *en, wxPoint *r, int nPoints )
{
	m_pcm93chart_current->GetPointPix ( rzRules, en, r, nPoints );
}

void cm93compchart::GetPixPoint ( int pixx, int pixy, double *plat, double *plon, ViewPort *vpt )
{
	m_pcm93chart_current->GetPixPoint ( pixx, pixy, plat, plon, vpt );
}

void cm93compchart::UpdateLUPs ( s57chart *pOwner )
{
	for ( int i = 0 ; i < 8 ; i++ )
	{
		if ( m_pcm93chart_array[i] )
			m_pcm93chart_array[i]->UpdateLUPs ( pOwner );
	}
}

ListOfS57Obj *cm93compchart::GetAssociatedObjects ( S57Obj *obj )
{
	if ( m_pcm93chart_current )
		return  m_pcm93chart_current->GetAssociatedObjects ( obj );
	else
		return NULL;
}


void cm93compchart::InvalidateCache()
{
	for ( int i = 0 ; i < 8 ; i++ )
	{
		if ( m_pcm93chart_array[i] )
			m_pcm93chart_array[i]->InvalidateCache();
	}
}

void cm93compchart::ForceEdgePriorityEvaluate ( void )
{
	for ( int i = 0 ; i < 8 ; i++ )
	{
		if ( m_pcm93chart_array[i] )
			m_pcm93chart_array[i]->ForceEdgePriorityEvaluate();
	}
}

void cm93compchart::SetColorScheme(ColorScheme cs, bool bApplyImmediate)
{
	m_global_color_scheme = cs;

	for ( int i = 0 ; i < 8 ; i++ )
	{
		if ( m_pcm93chart_array[i] )
			m_pcm93chart_array[i]->SetColorScheme(cs, bApplyImmediate);
	}
}

ListOfObjRazRules *cm93compchart::GetObjRuleListAtLatLon ( float lat, float lon, float select_radius, ViewPort *VPoint )
{
	float alon = lon;

	while ( alon < 0 )            // CM93 longitudes are all positive
		alon += 360;

	ViewPort vp_positive = *VPoint; // needs a new ViewPort also for ObjectRenderCheck()
	vp_positive.set_positive();

	if ( !VPoint->b_quilt )
		if( m_pcm93chart_current )
			return  m_pcm93chart_current->GetObjRuleListAtLatLon ( lat, alon, select_radius, &vp_positive );
		else {
			//     As default, return an empty list
			ListOfObjRazRules *ret_ptr = new ListOfObjRazRules;
			return ret_ptr;
		}
	else
	{
		UpdateRenderRegions ( *VPoint );

		//    Search all of the subcharts, looking for the one whose render region contains the requested point
		wxPoint p = VPoint->GetPixFromLL ( lat, lon );

		for ( int i = 0 ; i < 8 ; i++ )
		{
			if ( m_pcm93chart_array[i] )
			{

				if ( !m_pcm93chart_array[i]->m_render_region.IsEmpty() )
				{
					if ( wxInRegion == m_pcm93chart_array[i]->m_render_region.Contains ( p ) )
						return  m_pcm93chart_array[i]->GetObjRuleListAtLatLon ( lat, alon, select_radius, &vp_positive );
				}
			}
		}

		//     As default, return an empty list
		ListOfObjRazRules *ret_ptr = new ListOfObjRazRules;

		return ret_ptr;

	}

}

VE_Hash& cm93compchart::Get_ve_hash ( void )
{
	return m_pcm93chart_current->Get_ve_hash();
}

VC_Hash& cm93compchart::Get_vc_hash ( void )
{
	return m_pcm93chart_current->Get_vc_hash();
}




bool cm93compchart::AdjustVP ( ViewPort &vp_last, ViewPort &vp_proposed )
{
	//    This may be a partial screen render
	//    If it is, the cmscale value on this render must match the same parameter
	//    on the last render.
	//    If it does not, the partial render will not quilt correctly with the previous data
	//    Detect this case, and indicate that the entire screen must be rendered.


	int cmscale = GetCMScaleFromVP ( vp_proposed );                   // This is the scale that should be used, based on the vp

	int cmscale_actual = PrepareChartScale ( vp_proposed, cmscale );  // this is the scale that will be used, based on cell coverage

	if ( g_bDebugCM93 )
		printf ( "  In AdjustVP,  adjustment subchart scale is %c\n", ( char ) ( 'A' + cmscale_actual -1 ) );

	//    We always need to do a VP adjustment, independent of this method's return value.
	//    so, do an AdjustVP() based on the chart scale that WILL BE USED
	//    And be sure to return false if that adjust method suggests so.

	bool single_adjust = false;
	if ( m_pcm93chart_array[cmscale_actual] )
		single_adjust = m_pcm93chart_array[cmscale_actual]->AdjustVP ( vp_last, vp_proposed );

	if ( m_cmscale != cmscale_actual )
		return false;

	//    In quilt mode, always indicate that the adjusted vp requires a full repaint
	if ( vp_last.b_quilt )
		return false;

	return single_adjust;
}

ThumbData *cm93compchart::GetThumbData ( int tnx, int tny, float lat, float lon )
{
	return ( ThumbData * ) NULL;
}

InitReturn cm93compchart::CreateHeaderData()
{

	m_Chart_Scale = 20000000;

	//        Read the root directory, getting subdirectories to build a small scale coverage region
	wxRect extent_rect;

	wxDir dirt(m_prefixComposite);
	wxString candidate;
	wxRegEx test(_T("[0-9]+"));

	bool b_cont = dirt.GetFirst(&candidate);

	while(b_cont) {
		if(test.Matches(candidate)&& (candidate.Len() == 8)) {
			wxString dir = m_prefixComposite;
			dir += candidate;
			if( wxDir::Exists(dir) ) {
				wxFileName name( dir );
				wxString num_name = name.GetName();
				long number;
				if( num_name.ToLong( &number ) ) {
					int ilat = number / 10000;
					int ilon = number % 10000;

					int lat_base = ( ilat - 270 ) / 3.;
					int lon_base = ilon / 3.;
					extent_rect.Union(wxRect(lon_base, lat_base, 20, 20));
				}
			}
		}
		b_cont = dirt.GetNext(&candidate);
	}

	//    Specify the chart coverage
	m_FullExtent.ELON = ((double)extent_rect.x + (double)extent_rect.width );
	m_FullExtent.WLON = ((double)extent_rect.x);
	m_FullExtent.NLAT = ((double)extent_rect.y + (double)extent_rect.height );
	m_FullExtent.SLAT = ((double)extent_rect.y);
	m_bExtentSet = true;


	//    Populate one M_COVR Entry
	m_nCOVREntries = 1;
	m_pCOVRTablePoints = ( int * ) malloc ( sizeof ( int ) );
	*m_pCOVRTablePoints = 4;
	m_pCOVRTable = ( float ** ) malloc ( sizeof ( float * ) );
	float *pf = ( float * ) malloc ( 2 * 4 * sizeof ( float ) );
	*m_pCOVRTable = pf;
	float *pfe = pf;

	*pfe++ = m_FullExtent.NLAT; //LatMax;
	*pfe++ = m_FullExtent.WLON; //LonMin;

	*pfe++ = m_FullExtent.NLAT; //LatMax;
	*pfe++ = m_FullExtent.ELON; //LonMax;

	*pfe++ = m_FullExtent.SLAT; //LatMin;
	*pfe++ = m_FullExtent.ELON; //LonMax;

	*pfe++ = m_FullExtent.SLAT; //LatMin;
	*pfe++ = m_FullExtent.WLON; //LonMin;


	return INIT_OK;
}

cm93_dictionary *cm93compchart::FindAndLoadDictFromDir ( const wxString &dir )
{
	cm93_dictionary *retval = NULL;
	cm93_dictionary *pdict = new cm93_dictionary();

	//    Quick look at the supplied directory...
	if ( pdict->LoadDictionary ( dir ) )
		return pdict;


	//    Otherwise, search for the dictionary files all along the path of the passed parameter

	wxString path = dir;
	wxString target;
	unsigned int i = 0;

	while ( i < path.Len() )
	{
		target.Append ( path[i] );
		if ( path[i] == wxFileName::GetPathSeparator() )
		{
			//                  wxString msg = _T ( " Looking for CM93 dictionary in " );
			//                  msg.Append ( target );
			//                  wxLogMessage ( msg );

			if ( pdict->LoadDictionary ( target ) )
			{
				retval = pdict;
				break;
			}
		}
		i++;
	}

	if ( NULL != retval )                           // Found it....
		return retval;




	//    Dictionary was not found in linear path of supplied dir.
	//    Could be on branch, so, look at entire tree the hard way.

	wxFileName fnc ( dir );
	wxString found_dict_file_name;

	bool bdone = false;
	while ( !bdone )
	{
		path = fnc.GetPath ( wxPATH_GET_VOLUME );     // get path without sep

		wxString msg = _T ( " Looking harder for CM93 dictionary in " );
		msg.Append ( path );
		wxLogMessage ( msg );


		if ( ( path.Len() == 0 ) || path.IsSameAs ( fnc.GetPathSeparator() ) )
		{
			bdone = true;
			wxLogMessage ( _T ( "Early break1" ) );
			break;
		}

		//    Abort the search loop if the directory tree does not contain some indication of CM93
		if ( ( wxNOT_FOUND == path.Lower().Find ( _T ( "cm93" ) ) ) )
		{
			bdone = true;
			wxLogMessage ( _T ( "Early break2" ) );
			break;
		}

		//    Search here
		//    This takes a while to search a fully populated cm93 tree....
		wxDir dir ( path );

		if ( dir.IsOpened() )
		{
			// Find the dictionary name, case insensitively
			FindCM93Dictionary cm93Dictionary ( found_dict_file_name );
			dir.Traverse ( cm93Dictionary );
			bdone = found_dict_file_name.Len() != 0;
		}

		fnc.Assign ( path );                              // convert the path to a filename for next loop
	}

	if ( found_dict_file_name.Len() )
	{
		wxFileName fnd ( found_dict_file_name );
		wxString dpath = fnd.GetPath ( ( int ) ( wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME ) );

		if ( pdict->LoadDictionary ( dpath ) )
			retval = pdict;
	}


	if ( NULL == retval )
		delete pdict;

	return retval;


}

void cm93compchart::CloseandReopenCurrentSubchart ( void )
{
	delete  m_pcm93chart_current;
	m_pcm93chart_current = NULL;
	m_pcm93chart_array[m_cmscale] = NULL;

	SetVPParms ( m_vpt );
	InvalidateCache();
}

