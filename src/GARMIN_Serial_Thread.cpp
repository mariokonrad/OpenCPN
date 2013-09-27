/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 * Copyright (C) 2010 by David S. Register                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the                           *
 * Free Software Foundation, Inc.,                                         *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.           *
 **************************************************************************/

#include "GARMIN_Serial_Thread.h"
#include <DataStream.h>
#include <OCPN_DataStreamEvent.h>
#include <GarminProtocolHandler.h>
#include <garmin/jeeps/garmin_wrapper.h>
#include <nmea0183/nmea0183.h>

#include <wx/datetime.h>
#include <wx/timer.h>

struct D800_Pvt_Data_Type_Aligned
{
	float   alt;
	float   epe;
	float   eph;
	float   epv;
	short   fix;
	double  tow;
	double  lat;
	double  lon;
	float   east;
	float   north;
	float   up;
	float   msl_hght;
	short   leap_scnds;
	long    wn_days;
};

static D800_Pvt_Data_Type_Aligned mypvt;


GARMIN_Serial_Thread::GARMIN_Serial_Thread(
		GarminProtocolHandler *parent,
		DataStream * GParentStream,
		wxEvtHandler * MessageTarget,
		wxString port)
{
	m_parent = parent;                          // This thread's immediate "parent"
	m_parent_stream = GParentStream;
	m_pMessageTarget = MessageTarget;
	m_port = port;

	Create();
}

GARMIN_Serial_Thread::~GARMIN_Serial_Thread(void)
{
}


//    Entry Point
void *GARMIN_Serial_Thread::Entry()
{
	//   m_parent->SetSecThreadActive();               // I am alive
	m_bdetected = false;
	m_bconnected = false;

	bool not_done = true;
	wxDateTime last_rx_time;


#ifdef USE_GARMINHOST
	//    The main loop

	while((not_done) && (m_parent->m_Thread_run_flag > 0)) {

		if(TestDestroy()) {
			not_done = false;                               // smooth exit
			goto thread_exit;
		}

		while( !m_bdetected ) {

			//  Try to init the port once
			int v_init = Garmin_GPS_Init(m_port);
			if( v_init < 0 ){           //  Open failed, so sleep and try again
				for( int i=0 ; i < 4 ; i++) {
					wxSleep(1);
					if(TestDestroy())
						goto thread_exit;
					if( !m_parent->m_Thread_run_flag )
						goto thread_exit;
				}
			}
			else
				m_bdetected = true;
		}                       // while not detected

		// Detected OK

		//      Start PVT packet transmission
		if( !m_bconnected ) {
			if( !Garmin_GPS_PVT_On( m_port) ) {
				m_bdetected = false;            // error, would not accept PVT On
				m_bconnected = false;
			}
			else
				m_bconnected = true;
		}

		if( m_bconnected ) {

			D800_Pvt_Data_Type_Aligned *ppvt = &mypvt;
			int ret = Garmin_GPS_GetPVT(&ppvt);
			if(ret > 0) {
				if((mypvt.fix) >= 2 && (mypvt.fix <= 5)) {
					// Synthesize an NMEA GMRMC message
					SENTENCE snt;
					NMEA0183 oNMEA0183;
					oNMEA0183.TalkerID = _T ( "GM" );

					if ( mypvt.lat < 0. )
						oNMEA0183.Rmc.Position.Latitude.Set ( -mypvt.lat, _T ( "S" ) );
					else
						oNMEA0183.Rmc.Position.Latitude.Set ( mypvt.lat, _T ( "N" ) );

					if ( mypvt.lon < 0. )
						oNMEA0183.Rmc.Position.Longitude.Set ( -mypvt.lon, _T ( "W" ) );
					else
						oNMEA0183.Rmc.Position.Longitude.Set ( mypvt.lon, _T ( "E" ) );

					/* speed over ground */
					double sog = sqrt(mypvt.east*mypvt.east + mypvt.north*mypvt.north) * 3.6 / 1.852;
					oNMEA0183.Rmc.SpeedOverGroundKnots = sog;

					/* course over ground */
					double course = atan2(mypvt.east, mypvt.north);
					if (course < 0)
						course += 2 * M_PI;
					double cog = course * 180 / M_PI;
					oNMEA0183.Rmc.TrackMadeGoodDegreesTrue = cog;

					oNMEA0183.Rmc.IsDataValid = NTrue;

					oNMEA0183.Rmc.Write ( snt );
					wxString message = snt.Sentence;

					if( m_pMessageTarget ) {
						OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
						wxCharBuffer buffer=message.ToUTF8();
						if(buffer.data()) {
							Nevent.SetNMEAString( buffer.data() );
							Nevent.SetStream( m_parent_stream );

							m_pMessageTarget->AddPendingEvent(Nevent);
						}
					}

					last_rx_time = wxDateTime::Now();

				}
			}
			else {
				wxDateTime now = wxDateTime::Now();
				if( last_rx_time.IsValid() ) {
					wxTimeSpan delta_time = now - last_rx_time;
					if( delta_time.GetSeconds() > 5 ) {
						m_bdetected = false;
						m_bconnected = false;
						Garmin_GPS_ClosePortVerify();
					}
				}
			}
		}
	}                          // the big while...

thread_exit:

	Garmin_GPS_PVT_Off( m_port);
	Garmin_GPS_ClosePortVerify();

#else           //#ifdef USE_GARMINHOST

	while((not_done) && (m_parent->m_Thread_run_flag > 0)) {

		wxSleep(1);
		if(TestDestroy()) {
			not_done = false;                               // smooth exit
			goto thread_exit;
		}
	}

thread_exit:

#endif          //#ifdef USE_GARMINHOST

	m_parent->m_Thread_run_flag = -1;   // in GarminProtocolHandler
	return 0;
}

