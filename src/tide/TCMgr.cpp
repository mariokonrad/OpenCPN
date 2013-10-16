/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Tide and Current Manager
 * Author:   David Register
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

#include "TCMgr.h"
#include "dychart.h"

#include <tide/IDX_entry.h>
#include <tide/Station_Data.h>

#include <libtcd/tcd.h>

#include <geo/GeoRef.h>

#include <cstdlib>
#include <cmath>
#include <ctime>

#include <wx/datetime.h>
#include <wx/log.h>

#define USF_REMOVE 1
#define USF_UPDATE 2
#define USF_WRITE  3


#define TIDE_MAX_DERIV (2)      // Maximum derivative supported
/* TIDE_TIME_PREC
 *   Precision (in seconds) to which we will find roots
 */
#define TIDE_TIME_PREC (15)

/* TIDE_TIME_BLEND
 *   Half the number of seconds over which to blend the tides from
 *   one epoch to the next.
 */
#define TIDE_BLEND_TIME (3600)

/* TIDE_TIME_STEP
 *   We are guaranteed to find all high and low tides as long as their
 * spacing is greater than this value (in seconds).
 */
#define TIDE_TIME_STEP (TIDE_TIME_PREC)
#define TIDE_BAD_TIME   ((time_t) -1)

static double time2dt_tide(time_t t, int deriv, IDX_entry *pIDX);
static void happy_new_year(IDX_entry *pIDX, int new_year);
static void set_epoch(IDX_entry *pIDX, int year);
static double time2tide(time_t t, IDX_entry * pIDX);
static int yearoftimet(time_t t);
static double time2asecondary(time_t t, IDX_entry *pIDX);

time_t s_next_epoch      = TIDE_BAD_TIME; /* next years newyears */
time_t s_this_epoch      = TIDE_BAD_TIME; /* this years newyears */
int    s_this_year       = -1;


// TCMgr Implementation
TCMgr::TCMgr()
{
}

TCMgr::~TCMgr()
{
	PurgeData();
}

void TCMgr::PurgeData()
{
	//  Index entries are owned by the data sources
	//  so we need to clear them from the combined list without
	//  deleting them
	while(m_Combined_IDX_array.size()) {
		m_Combined_IDX_array.erase(m_Combined_IDX_array.begin());
	}

	//  Delete all the data sources
	std::vector<TCDataSource *>::iterator i = m_source_array.begin();
	std::vector<TCDataSource *>::iterator end = m_source_array.end();
	for (; i != end; ++i) {
		if (*i)
			delete *i;
	}
	m_source_array.clear();
}


int TCMgr::Get_max_IDX() const
{
	return m_Combined_IDX_array.size()-1;
}

bool TCMgr::IsReady(void) const
{
	return bTCMReady;
}

wxArrayString TCMgr::GetDataSet(void)
{
	return m_sourcefile_array;
}

TC_Error_Code TCMgr::LoadDataSources(wxArrayString &sources)
{
	PurgeData();

	//  Take a copy of dataset file name array
	m_sourcefile_array.Clear();
	m_sourcefile_array = sources;

	//  Arrange for the index array to begin counting at "one", FIXME: why?
	m_Combined_IDX_array.push_back((IDX_entry *)(NULL));
	int num_IDX = 1;

	for(unsigned int i=0 ; i < sources.GetCount() ; i++) {
		TCDataSource *s = new TCDataSource;
		TC_Error_Code r = s->LoadData(sources.Item(i));
		if(r != TC_NO_ERROR) {
			wxString msg;
			msg.Printf(_T("   Error loading Tide/Currect data source %s "), sources.Item(i).c_str());
			if( r == TC_FILE_NOT_FOUND)
				msg += _T("Error Code: TC_FILE_NOT_FOUND");
			else {
				wxString msg1;
				msg1.Printf(_T("Error code: %d"), r);
				msg += msg1;
			}
			wxLogMessage(msg);
			delete s;
		} else {
			m_source_array.push_back(s);

			for( int k=0 ; k < s->GetMaxIndex() ; k++ ) {
				IDX_entry *pIDX = s->GetIndexEntry(k);
				pIDX->IDX_rec_num = num_IDX;
				num_IDX++;
				m_Combined_IDX_array.push_back(pIDX);
			}
		}
	}

	bTCMReady = true;

	return  TC_NO_ERROR;
}

const IDX_entry * TCMgr::GetIDX_entry(int index) const
{
	if ((unsigned int)index < m_Combined_IDX_array.size())
		return m_Combined_IDX_array.at(index);
	else
		return NULL;
}


bool TCMgr::GetTideOrCurrent(time_t t, int idx, float &tcvalue, float& dir)
{
	//    Return a sensible value of 0,0 by default
	dir = 0;
	tcvalue = 0;

	//    Load up this location data
	IDX_entry * pIDX = m_Combined_IDX_array.at(idx); // point to the index entry

	if( !pIDX ) {
		dir = 0;
		tcvalue = 0;
		return false;
	}

	if( !pIDX->IDX_Useable ) {
		dir = 0;
		tcvalue = 0;
		return(false);                                        // no error, but unuseable
	}

	if(pIDX->pDataSource) {
		if(pIDX->pDataSource->LoadHarmonicData(pIDX) != TC_NO_ERROR)
			return false;
	}

	pIDX->max_amplitude = 0.0;                // Force multiplier re-compute
	int yott = yearoftimet( t );

	happy_new_year (pIDX, yott);              //Calculate new multipliers

	//    Finally, calculate the tide/current

	double level = time2asecondary(t + (00 * 60), pIDX);  // 300. 240
	if(level >= 0)
		dir = pIDX->IDX_flood_dir;
	else
		dir = pIDX->IDX_ebb_dir;

	tcvalue = level;

	return(true); // Got it!
}

bool TCMgr::GetTideOrCurrent15(time_t WXUNUSED(t), int idx, float &tcvalue, float& dir, bool &bnew_val)
{
	int ret;
	IDX_entry * pIDX = m_Combined_IDX_array.at(idx);             // point to the index entry

	if( !pIDX ) {
		dir = 0;
		tcvalue = 0;
		return false;
	}

	//    Figure out this computer timezone minute offset
	wxDateTime this_now = wxDateTime::Now();
	wxDateTime this_gmt = this_now.ToGMT();
	wxTimeSpan diff = this_gmt.Subtract(this_now);
	int diff_mins = diff.GetMinutes();

	int station_offset = pIDX->IDX_time_zone;
	if(this_now.IsDST())
		station_offset += 60;
	int corr_mins = station_offset - diff_mins;

	wxDateTime today_00 = wxDateTime::Today();
	int t_today_00 = today_00.GetTicks();
	int t_today_00_at_station = t_today_00 - (corr_mins * 60);

	int t_at_station = this_gmt.GetTicks() - (station_offset * 60) + (corr_mins * 60);

	int t_mins = (t_at_station - t_today_00_at_station) / 60;
	int t_15s = t_mins / 15;

	if(pIDX->Valid15)                               // valid data available
	{

		int tref1 = t_today_00_at_station + t_15s * 15 * 60;
		if(tref1 == pIDX->Valid15)
		{
			tcvalue = pIDX->Value15;
			dir = pIDX->Dir15;
			bnew_val = false;
			return pIDX->Ret15;
		}
		else
		{
			int tref = t_today_00_at_station + t_15s * 15 * 60;
			ret = GetTideOrCurrent(tref, idx, tcvalue, dir);

			pIDX->Valid15 = tref;
			pIDX->Value15 = tcvalue;
			pIDX->Dir15 = dir;
			pIDX->Ret15 = !(ret == 0);
			bnew_val = true;

			return !(ret == 0);
		}
	}

	else {
		int tref = t_today_00_at_station + t_15s * 15 * 60;
		ret = GetTideOrCurrent(tref, idx, tcvalue, dir);

		pIDX->Valid15 = tref;
		pIDX->Value15 = tcvalue;
		pIDX->Dir15 = dir;
		pIDX->Ret15 = !(ret == 0);
		bnew_val = true;
	}

	return !(ret == 0);

}

bool TCMgr::GetTideFlowSens(time_t t, int sch_step, int idx, float &tcvalue_now, float &tcvalue_prev, bool &w_t)
{
	//    Return a sensible value of 0 by default
	tcvalue_now = 0;
	tcvalue_prev = 0;
	w_t = false;


	//    Load up this location data
	IDX_entry * pIDX = m_Combined_IDX_array.at(idx);             // point to the index entry

	if( !pIDX )
		return false;

	if( !pIDX->IDX_Useable )
		return false;                                        // no error, but unuseable

	if(pIDX->pDataSource) {
		if(pIDX->pDataSource->LoadHarmonicData(pIDX) != TC_NO_ERROR)
			return false;
	}

	pIDX->max_amplitude = 0.0;                // Force multiplier re-compute
	int yott = yearoftimet( t );
	happy_new_year (pIDX, yott);              //Force new multipliers

	//    Finally, process the tide flow sens

	tcvalue_now = time2asecondary (t , pIDX);
	tcvalue_prev = time2asecondary (t + sch_step , pIDX);

	w_t = tcvalue_now > tcvalue_prev;           // w_t = true --> flood , w_t = false --> ebb

	return true;
}

void TCMgr::GetHightOrLowTide(time_t t, int sch_step_1, int sch_step_2, float tide_val ,bool w_t , int idx, float &tcvalue, time_t &tctime)
{
	//    Return a sensible value of 0,0 by default
	tcvalue = 0;
	tctime = t;

	//    Load up this location data
	IDX_entry * pIDX = m_Combined_IDX_array.at(idx);             // point to the index entry

	if( !pIDX )
		return;

	if( !pIDX->IDX_Useable )
		return;                                        // no error, but unuseable

	if(pIDX->pDataSource) {
		if(pIDX->pDataSource->LoadHarmonicData(pIDX) != TC_NO_ERROR)
			return;
	}

	pIDX->max_amplitude = 0.0;                // Force multiplier re-compute
	int yott = yearoftimet( t );
	happy_new_year (pIDX, yott);

	// Finally, calculate the Hight and low tides
	double newval = tide_val;
	double oldval = ( w_t ) ? newval - 1: newval + 1 ;
	int j = 0 ;
	int k = 0 ;
	int ttt = 0 ;
	while ( (newval > oldval) == w_t )                  //searching each ten minute
	{
		j++;
		oldval = newval;
		ttt = t + ( sch_step_1 * j );
		newval = time2asecondary (ttt, pIDX);
	}
	oldval = ( w_t ) ? newval - 1: newval + 1 ;
	while ( (newval > oldval) == w_t )                  // searching back each minute
	{
		oldval = newval ;
		k++;
		ttt = t +  ( sch_step_1 * j ) - ( sch_step_2 * k ) ;
		newval = time2asecondary (ttt, pIDX);
	}
	tcvalue = newval;
	tctime = ttt + sch_step_2 ;

}

int TCMgr::GetStationTimeOffset(IDX_entry *pIDX)
{
	return pIDX->IDX_time_zone;
}

int TCMgr::GetNextBigEvent(time_t *tm, int idx)
{
	float tcvalue[1];
	float dir;
	bool ret;
	double p, q;
	int flags = 0, slope = 0;
	ret = GetTideOrCurrent(*tm, idx, tcvalue[0],  dir);
	p = tcvalue[0];
	*tm += 60;
	ret = GetTideOrCurrent(*tm, idx, tcvalue[0],  dir);
	q = tcvalue[0];
	*tm += 60;
	if (p < q)
		slope = 1;
	while (1) {
		if ((slope == 1 && q < p) || (slope == 0 && p < q)) {
			/* Tide event */
			flags |= (1 << slope);
		}
		if (flags) {
			*tm -= 60;
			if (flags < 4)
				*tm -= 60;
			return flags;
		}
		p = q;
		ret = GetTideOrCurrent(*tm, idx, tcvalue[0],  dir);
		q = tcvalue[0];
		*tm += 60;
	}
	return 0;
}

int TCMgr::GetStationIDXbyName(const wxString & prefix, double xlat, double xlon) const
{
	const IDX_entry *lpIDX;
	int jx = 0;
	wxString locn;
	double distx = 100000.;

	int jmax = Get_max_IDX();

	for ( int j=1 ; j<Get_max_IDX() +1 ; j++ ) {
		lpIDX = GetIDX_entry ( j );
		char type = lpIDX->IDX_type;             // Entry "TCtcIUu" identifier
		wxString locnx ( lpIDX->IDX_station_name, wxConvUTF8 );

		if ( (( type == 't' ) ||  ( type == 'T' ) )   // only Tides
				&& (locnx.StartsWith(prefix))) {
			double brg, dist;
			geo::DistanceBearingMercator(xlat, xlon, lpIDX->IDX_lat, lpIDX->IDX_lon, &brg, &dist);
			if (dist < distx) {
				distx = dist;
				jx = j;
			}
		}
	} // end for loop
	//} // end if @~~ found in WP
return(jx);
}


int TCMgr::GetStationIDXbyNameType(const wxString & prefix, double xlat, double xlon, char type) const
{
	const IDX_entry *lpIDX;
	int jx = 0;
	wxString locn;
	double distx = 100000.;

	// if (prp->m_MarkName.Find(_T("@~~")) != wxNOT_FOUND) {
	//tide_form = prp->m_MarkName.Mid(prp->m_MarkName.Find(_T("@~~"))+3);
	int jmax = Get_max_IDX();

	for ( int j=1 ; j<Get_max_IDX() +1 ; j++ ) {
		lpIDX = GetIDX_entry ( j );
		char typep = lpIDX->IDX_type;             // Entry "TCtcIUu" identifier
		wxString locnx ( lpIDX->IDX_station_name, wxConvUTF8 );

		if ( ( type == typep ) && (locnx.StartsWith(prefix))) {
			double brg, dist;
			geo::DistanceBearingMercator(xlat, xlon, lpIDX->IDX_lat, lpIDX->IDX_lon, &brg, &dist);
			if (dist < distx) {
				distx = dist;
				jx = j;
			}
		}
	} // end for loop
	return(jx);
}


static double time2tide(time_t t, IDX_entry *pIDX)
{
	return time2dt_tide(t, 0, pIDX);
}



/** BOGUS amplitude stuff - Added mgh
 * For knots^2 current stations, returns square root of (value * amplitude),
 * For normal stations, returns value * amplitude */

double BOGUS_amplitude(double mpy, IDX_entry *pIDX)
{
	Station_Data *pmsd = pIDX->pref_sta_data;

	if (!pmsd->have_BOGUS)                                // || !convert_BOGUS)   // Added mgh
		return(mpy * pIDX->max_amplitude);
	else {
		if (mpy >= 0.0)
			return( sqrt( mpy * pIDX->max_amplitude));
		else
			return(-sqrt(-mpy * pIDX->max_amplitude));
	}
}

/* Calculate the denormalized tide. */
double time2atide (time_t t, IDX_entry *pIDX)
{
	return BOGUS_amplitude(time2tide(t, pIDX), pIDX) + pIDX->pref_sta_data->DATUM;
}


/* Next high tide, low tide, transition of the mark level, or some
 *   combination.
 *       Bit      Meaning
 *        0       low tide
 *        1       high tide
 *        2       falling transition
 *        3       rising transition
 */
int next_big_event (time_t *tm, IDX_entry *pIDX)
{
	double p, q;
	int flags = 0, slope = 0;
	p = time2atide (*tm, pIDX);
	*tm += 60;
	q = time2atide (*tm, pIDX);
	*tm += 60;
	if (p < q)
		slope = 1;
	while (1) {
		if ((slope == 1 && q < p) || (slope == 0 && p < q)) {
			/* Tide event */
			flags |= (1 << slope);
		}
		/* Modes in which to return mark transitions: */
		/*    -text (no -graph)   */
		/*    -graph (no -text)   */
		/*    -ppm                */
		/*    -gif                */
		/*    -ps                 */


		//    if (mark && ((text && !graphmode) || (!text && graphmode)
		//    || ppm || gif || ps))
		//      int marklev = 0;
#if (0)
		if(0)
			if ((p > marklev && q <= marklev) || (p < marklev && q >= marklev)) {
				/* Transition event */
				if (p < q)
					flags |= 8;
				else
					flags |= 4;
				if (!(flags & 3)) {
					/* If we're incredibly unlucky, we could miss a tide event if we
					 *                       don't check for it here:
					 *
					 *                                       . <----   Value that would be returned
					 *                                  -----------    Mark level
					 *                                .           .
					 */
					p = q;
					q = time2atide (*tm, pIDX);
					if ((slope == 1 && q < p) || (slope == 0 && p < q)) {
						/* Tide event */
						flags |= (1 << slope);
					}
				}
			}
#endif

		if (flags) {
			*tm -= 60;
			/* Don't back up over a transition event, but do back up to where the
			 *               tide changed if possible.  If they happen at the same time, then
			 *               we're off by a minute on the tide, but if we back it up it will
			 *               get snagged on the transition event over and over. */
			if (flags < 4)
				*tm -= 60;
			return flags;
		}
		p = q;
		q = time2atide (*tm, pIDX);
		*tm += 60;
	}
}



/* Estimate the normalized mean tide level around a particular time by
 *   summing only the long-term constituents. */
/* Does not do any blending around year's end. */
/* This is used only by time2asecondary for finding the mean tide level */
static double time2mean(time_t t, IDX_entry *pIDX)
{
	double tide = 0.0;
	int a;
	int new_year = yearoftimet (t);
	if (pIDX->epoch_year != new_year)
		happy_new_year (pIDX, new_year);

	for (a=0; a<pIDX->num_csts; a++) {
		if (pIDX->m_cst_speeds[a] < 6e-6) {
			tide += pIDX->m_work_buffer[a] *
				cos (pIDX->m_cst_speeds[a] * ((long)(t - pIDX->epoch) + pIDX->pref_sta_data->meridian) +
						pIDX->m_cst_epochs[a][pIDX->epoch_year-pIDX->first_year] - pIDX->pref_sta_data->epoch[a]);
		}
	}

	return tide;
}



/* If offsets are in effect, interpolate the 'corrected' denormalized
 * tide.  The normalized is derived from this, instead of the other way
 * around, because the application of height offsets requires the
 * denormalized tide. */
static double time2asecondary(time_t t, IDX_entry *pIDX)
{
	time_t tadj = t + pIDX->station_tz_offset;

	/* Get rid of the normals. */
	if (!(pIDX->have_offsets))
		return time2atide (tadj, pIDX);

	{
		/* Intervalwidth of 14 (was originally 13) failed on this input:
		 *        -location Dublon -hloff +0.0001 -gstart 1997:09:10:00:00 -raw 1997:09:15:00:00
		 */
#define intervalwidth 15
#define stretchfactor 3

		static time_t lowtime=0, hightime=0;
		static double lowlvl, highlvl; /* Normalized tide levels for MIN, MAX */
		time_t T;  /* Adjusted t */
		double S, Z, HI, HS, magicnum;
		time_t interval = 3600 * intervalwidth;
		long difflow, diffhigh;
		int badlowflag=0, badhighflag=0;


		/* Algorithm by Jean-Pierre Lapointe (scipur@collegenotre-dame.qc.ca) */
		/* as interpreted, munged, and implemented by DWF */

		/* This is the initial guess (average of time offsets) */
		//    T = t - (httimeoff + lttimeoff) / 2;
		T = tadj - (pIDX->IDX_ht_time_off * 60 + pIDX->IDX_lt_time_off * 60) / 2;
		/* The usage of an estimate of mean tide level here is to correct
		 *           for seasonal changes in tide level.  Previously I had simply used
		 *           the zero of the tide function as the mean, but this gave bad
		 *           results around summer and winter for locations with large seasonal
		 *           variations. */
		//        printf("-----time2asecondary  %ld %ld %d %d\n", t, T, pIDX->IDX_ht_time_off ,pIDX->IDX_lt_time_off);

		Z = time2mean(T, pIDX);
		S = time2tide(T, pIDX) - Z;

		/* Find MAX and MIN.  I use the highest high tide and the lowest
		 *           low tide over a 26 hour period, but I allow the interval to stretch
		 *           a lot if necessary to avoid creating discontinuities.  The
		 *           heuristic used is not perfect but will hopefully be good enough.
		 *
		 *           It is an assumption in the algorithm that the tide level will
		 *           be above the mean tide level for MAX and below it for MIN.  A
		 *           changeover occurs at mean tide level.  It would be nice to
		 *           always use the two tides that immediately bracket T and to put
		 *           the changeover at mid tide instead of always at mean tide
		 *           level, since this would eliminate much of the inaccuracy.
		 *           Unfortunately if you change the location of the changeover it
		 *           causes the tide function to become discontinuous.
		 *
		 *           Now that I'm using time2mean, the changeover does move, but so
		 *           slowly that it makes no difference.
		 */

		if (lowtime < T)
			difflow = T - lowtime;
		else
			difflow = lowtime - T;
		if (hightime < T)
			diffhigh = T - hightime;
		else
			diffhigh = hightime - T;

		/* Update MIN? */
		if (difflow > interval * stretchfactor)
			badlowflag = 1;
		if (badlowflag || (difflow > interval && S > 0)) {
			time_t tt;
			double tl;
			tt = T - interval;
			next_big_event (&tt, pIDX);
			lowlvl = time2tide (tt, pIDX);
			lowtime = tt;
			while (tt < T + interval) {
				next_big_event (&tt, pIDX);
				tl = time2tide (tt, pIDX);
				if (tl < lowlvl && tt < T + interval) {
					lowlvl = tl;
					lowtime = tt;
				}
			}
		}
		/* Update MAX? */
		if (diffhigh > interval * stretchfactor)
			badhighflag = 1;
		if (badhighflag || (diffhigh > interval && S < 0)) {
			time_t tt;
			double tl;
			tt = T - interval;
			next_big_event (&tt, pIDX);
			highlvl = time2tide (tt, pIDX);
			hightime = tt;
			while (tt < T + interval) {
				next_big_event (&tt, pIDX);
				tl = time2tide (tt, pIDX);
				if (tl > highlvl && tt < T + interval) {
					highlvl = tl;
					hightime = tt;
				}
			}
		}

#if 0
		/* UNFORTUNATELY there are times when the tide level NEVER CROSSES
		 *           THE MEAN for extended periods of time.  ARRRGH!  */
		if (lowlvl >= 0.0)
			lowlvl = -1.0;
		if (highlvl <= 0.0)
			highlvl = 1.0;
#endif
		/* Now that I'm using time2mean, I should be guaranteed to get
		 *           an appropriate low and high. */


		/* Improve the initial guess. */
		if (S > 0)
			magicnum = 0.5 * S / fabs(highlvl - Z);
		else
			magicnum = 0.5 * S / fabs(lowlvl - Z);
		//    T = T - magicnum * (httimeoff - lttimeoff);
		T = T - (time_t)(magicnum * ((pIDX->IDX_ht_time_off * 60) - (pIDX->IDX_lt_time_off * 60)));
		HI = time2tide(T, pIDX);

		//    Correct the amplitude offsets for BOGUS knot^2 units
		double ht_off, lt_off;
		if (pIDX->pref_sta_data->have_BOGUS)
		{
			ht_off = pIDX->IDX_ht_off * pIDX->IDX_ht_off;         // Square offset in kts to adjust for kts^2
			lt_off = pIDX->IDX_lt_off * pIDX->IDX_lt_off;
		}
		else
		{
			ht_off = pIDX->IDX_ht_off;
			lt_off = pIDX->IDX_lt_off;
		}


		/* Denormalize and apply the height offsets. */
		HI = BOGUS_amplitude(HI, pIDX) + pIDX->pref_sta_data->DATUM;
		{
			double RH=1.0, RL=1.0, HH=0.0, HL=0.0;
			RH = pIDX->IDX_ht_mpy;
			HH = ht_off;
			RL = pIDX->IDX_lt_mpy;
			HL = lt_off;

			/* I patched the usage of RH and RL to avoid big ugly
			 *            discontinuities when they are not equal.  -- DWF */

			HS =  HI * ((RH+RL)/2 + (RH-RL)*magicnum)
				+ (HH+HL)/2 + (HH-HL)*magicnum;
		}

		return HS;
	}
}






/*
 * We will need a function for tidal height as a function of time
 * which is continuous (and has continuous first and second derivatives)
 * for all times.
 *
 * Since the epochs & multipliers for the tidal constituents change
 * with the year, the regular time2tide(t) function has small
 * discontinuities at new years.  These discontinuities really
 * fry the fast root-finders.
 *
 * We will eliminate the new-years discontinuities by smoothly
 * interpolating (or "blending") between the tides calculated with one
 * year's coefficients, and the tides calculated with the next year's
 * coefficients.
 *
 * i.e. for times near a new years, we will "blend" a tide
 * as follows:
 *
 * tide(t) = tide(year-1, t)
 *                  + w((t - t0) / Tblend) * (tide(year,t) - tide(year-1,t))
 *
 * Here:  t0 is the time of the nearest new-year.
 *        tide(year-1, t) is the tide calculated using the coefficients
 *           for the year just preceding t0.
 *        tide(year, t) is the tide calculated using the coefficients
 *           for the year which starts at t0.
 *        Tblend is the "blending" time scale.  This is set by
 *           the macro TIDE_BLEND_TIME, currently one hour.
 *        w(x) is the "blending function", whice varies smoothly
 *           from 0, for x < -1 to 1 for x > 1.
 *
 * Derivatives of the blended tide can be evaluated in terms of derivatives
 * of w(x), tide(year-1, t), and tide(year, t).  The blended tide is
 * guaranteed to have as many continuous derivatives as w(x).  */

/* time2dt_tide(time_t t, int n)
 *
 *   Calculate nth time derivative the normalized tide.
 *
 * Notes: This function does not check for changes in year.
 *  This is important to our algorithm, since for times near
 *  new years, we interpolate between the tides calculated
 *  using one years coefficients, and the next years coefficients.
 *
 *  Except for this detail, time2dt_tide(t,0) should return a value
 *  identical to time2tide(t).
 */
static double _time2dt_tide(time_t t, int deriv, IDX_entry *pIDX)
{
	double dt_tide = 0.0;
	int a, b;
	double term, tempd;

	tempd = M_PI / 2.0 * deriv;
	for (a=0; a<pIDX->num_csts; a++)
	{
		term = pIDX->m_work_buffer[a] *
			cos(tempd +
					pIDX->m_cst_speeds[a] * ((long)(t - pIDX->epoch) + pIDX->pref_sta_data->meridian) +
					pIDX->m_cst_epochs[a][pIDX->epoch_year-pIDX->first_year] - pIDX->pref_sta_data->epoch[a]);
		for (b = deriv; b > 0; b--)
			term *= pIDX->m_cst_speeds[a];
		dt_tide += term;
	}
	return dt_tide;
}

/* blend_weight (double x, int deriv)
 *
 * Returns the value nth derivative of the "blending function" w(x):
 *
 *   w(x) =  0,     for x <= -1
 *
 *   w(x) =  1/2 + (15/16) x - (5/8) x^3 + (3/16) x^5,
 *                  for  -1 < x < 1
 *
 *   w(x) =  1,     for x >= 1
 *
 * This function has the following desirable properties:
 *
 *    w(x) is exactly either 0 or 1 for |x| > 1
 *
 *    w(x), as well as its first two derivatives are continuous for all x.
 */
static double blend_weight(double x, int deriv)
{
	double x2 = x * x;

	if (x2 >= 1.0)
		return deriv == 0 && x > 0.0 ? 1.0 : 0.0;

	switch (deriv) {
		case 0:
			return ((3.0 * x2 -10.0) * x2 + 15.0) * x / 16.0 + 0.5;
		case 1:
			return ((x2 - 2.0) * x2 + 1.0) * (15.0/16.0);
		case 2:
			return (x2 - 1.0) * x * (15.0/4.0);
	}
	return(0); // mgh+ to get rid of compiler warning
}

/*
 * This function does the actual "blending" of the tide
 * and its derivatives.
 */
static double blend_tide(time_t t, unsigned int deriv, int first_year, double blend, IDX_entry *pIDX)
{
	double        fl[TIDE_MAX_DERIV + 1];
	double        fr[TIDE_MAX_DERIV + 1];
	double *      fp      = fl;
	double        w[TIDE_MAX_DERIV + 1];
	double        fact = 1.0;
	double        f;
	unsigned int  n;


	/*
	 * If we are already happy_new_year()ed into one of the two years
	 * of interest, compute that years tide values first.
	 */
	int year = yearoftimet(t);
	if (year == first_year + 1)
		fp = fr;
	else if (year != first_year)
		happy_new_year(pIDX, first_year);
	for (n = 0; n <= deriv; n++)
		fp[n] = _time2dt_tide(t, n, pIDX);

	/*
	 * Compute tide values for the other year of interest,
	 *  and the needed values of w(x) and its derivatives.
	 */
	if (fp == fl)
	{
		happy_new_year(pIDX, first_year + 1);
		fp = fr;
	}
	else
	{
		happy_new_year(pIDX, first_year);
		fp = fl;
	}
	for (n = 0; n <= deriv; n++)
	{
		fp[n] = _time2dt_tide(t, n, pIDX);
		w[n] = blend_weight(blend, n);
	}

	/*
	 * Do the blending.
	 */


	f = fl[deriv];
	for (n = 0; n <= deriv; n++)
	{
		f += fact * w[n] * (fr[deriv-n] - fl[deriv-n]);
		fact *= (double)(deriv - n)/(n+1) * (1.0/TIDE_BLEND_TIME);
	}
	printf(" %ld  %g     %g %g %g\n", t, blend, fr[0], fl[0], f);
	return f;
}

static double time2dt_tide(time_t t, int deriv, IDX_entry *pIDX)
{
	int           new_year;
	int yott = yearoftimet( t );
	new_year = yott;

	/* Make sure our values of next_epoch and epoch are up to date. */
	if (new_year != s_this_year)
	{
		if (new_year + 1 < pIDX->first_year + pIDX->num_epochs)
		{
			set_epoch(pIDX, new_year + 1);
			s_next_epoch = pIDX->epoch;
		}
		else
			s_next_epoch = TIDE_BAD_TIME;

		happy_new_year(pIDX, s_this_year = new_year);
		s_this_epoch = pIDX->epoch;
	}


	/*
	 * If we're close to either the previous or the next
	 * new years we must blend the two years tides.
	 */
	if (t - s_this_epoch <= TIDE_BLEND_TIME && s_this_year > pIDX->first_year)
		return blend_tide(t, deriv,
				s_this_year - 1,
				(double)(t - s_this_epoch)/TIDE_BLEND_TIME,
				pIDX );
	else if (s_next_epoch - t <= TIDE_BLEND_TIME
			&& s_this_year + 1 < pIDX->first_year + pIDX->num_epochs)
		return blend_tide(t, deriv,
				s_this_year,
				-(double)(s_next_epoch - t)/TIDE_BLEND_TIME,
				pIDX );

	/*
	 * Else, we're far enough from newyears to ignore the blending.
	 */
	if (pIDX->epoch_year != new_year)
		happy_new_year(pIDX, new_year);

	return _time2dt_tide(t, deriv, pIDX);
}



/* Figure out max amplitude over all the years in the node factors table. */
/* This function by Geoffrey T. Dairiki */
static void figure_max_amplitude(IDX_entry *pIDX)
{
	int       i, a;

	if (pIDX->max_amplitude == 0.0) {
		for (i = 0; i < pIDX->num_nodes; i++) {
			double year_amp = 0.0;

			for (a=0; a < pIDX->num_csts; a++)
				year_amp += pIDX->pref_sta_data->amplitude[a] * pIDX->m_cst_nodes[a][i];
			if (year_amp > pIDX->max_amplitude)
				pIDX->max_amplitude = year_amp;
		}
	}
}

/* Figure out normalized multipliers for constituents for a particular year. */
static void figure_multipliers(IDX_entry *pIDX, int year)
{
	int a;

	figure_max_amplitude( pIDX );
	for (a = 0; a < pIDX->num_csts; a++) {
		pIDX->m_work_buffer[a] = pIDX->pref_sta_data->amplitude[a] * pIDX->m_cst_nodes[a][year-pIDX->first_year] / pIDX->max_amplitude;  // BOGUS_amplitude?
	}
}


/* This idiotic function is needed by the new tm2gmt. */
#define compare_int(a,b) (((int)(a))-((int)(b)))
int compare_tm(struct tm *a, struct tm *b)
{
	int temp;
	/* printf ("A is %d:%d:%d:%d:%d:%d   B is %d:%d:%d:%d:%d:%d\n",
	 *      a->tm_year+1900, a->tm_mon+1, a->tm_mday, a->tm_hour,
	 *      a->tm_min, a->tm_sec,
	 *      b->tm_year+1900, b->tm_mon+1, b->tm_mday, b->tm_hour,
	 *      b->tm_min, b->tm_sec); */

	temp = compare_int (a->tm_year, b->tm_year);
	if (temp)
		return temp;
	temp = compare_int (a->tm_mon, b->tm_mon);
	if (temp)
		return temp;
	temp = compare_int (a->tm_mday, b->tm_mday);
	if (temp)
		return temp;
	temp = compare_int (a->tm_hour, b->tm_hour);
	if (temp)
		return temp;
	temp = compare_int (a->tm_min, b->tm_min);
	if (temp)
		return temp;
	return compare_int (a->tm_sec, b->tm_sec);
}

/* Convert a struct tm in GMT back to a time_t.  isdst is ignored, since
 *   it never should have been needed by mktime in the first place.
 */
time_t tm2gmt(struct tm *ht)
{
	time_t guess, newguess, thebit;
	int loopcounter, compare;
	struct tm *gt;

	guess = 0;
	loopcounter = (sizeof(time_t) * 8)-1;
	thebit = ((time_t)1) << (loopcounter-1);

	/* For simplicity, I'm going to insist that the time_t we want is
	 *       positive.  If time_t is signed, skip the sign bit.
	 */
	if ((signed long)thebit < (time_t)(0)) {
		/* You can't just shift thebit right because it propagates the sign bit. */
		loopcounter--;
		thebit = ((time_t)1) << (loopcounter-1);
	}

	for (; loopcounter; loopcounter--) {
		newguess = guess | thebit;
		gt = gmtime(&newguess);
		if(NULL != gt)
		{
			compare = compare_tm (gt, ht);
			if (compare <= 0)
				guess = newguess;
		}
		thebit >>= 1;
	}

	return guess;
}

static int yearoftimet(time_t t)
{
	return ((gmtime (&t))->tm_year) + 1900;
}


/* Calculate time_t of the epoch. */
static void set_epoch(IDX_entry *pIDX, int year)
{
	struct tm ht;

	ht.tm_year = year - 1900;
	ht.tm_sec = ht.tm_min = ht.tm_hour = ht.tm_mon = 0;
	ht.tm_mday = 1;
	pIDX->epoch = tm2gmt (&ht);
}

/* Re-initialize for a different year */
static void happy_new_year(IDX_entry *pIDX, int new_year)
{
	pIDX->epoch_year = new_year;
	figure_multipliers ( pIDX, new_year );
	set_epoch ( pIDX, new_year );
}

