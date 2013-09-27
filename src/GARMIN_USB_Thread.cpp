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

#ifdef __WXMSW__
	#include <windows.h>
	#include <dbt.h>
	#include <windows.h>
	#include <winioctl.h>
	#include <initguid.h>
#endif

#include "GARMIN_USB_Thread.h"
#include <DataStream.h>
#include <OCPN_DataStreamEvent.h>
#include <GarminProtocolHandler.h>
#include <garmin/jeeps/garmin_wrapper.h>
#include <nmea0183/nmea0183.h>


GARMIN_USB_Thread::GARMIN_USB_Thread(
		GarminProtocolHandler * parent,
		DataStream * GParentStream,
		wxEvtHandler * MessageTarget,
		unsigned int device_handle,
		size_t max_tx_size)
{
	m_parent = parent; // This thread's immediate "parent"
	m_parent_stream = GParentStream;
	m_pMessageTarget = MessageTarget;
	m_max_tx_size = max_tx_size;

#ifdef __WXMSW__
	m_usb_handle = (HANDLE)(device_handle & 0xffff);
#endif

	Create();
}

GARMIN_USB_Thread::~GARMIN_USB_Thread()
{
}

void *GARMIN_USB_Thread::Entry()
{
	garmin_usb_packet iresp;
	m_receive_state = rs_fromintr;

	//    Here comes the big while loop
	while(m_parent->m_Thread_run_flag > 0)
	{
		if(TestDestroy())
			goto thread_prexit; // smooth exit

		//    Get one  packet

		int nr = gusb_cmd_get(&iresp, sizeof(iresp));

		if(iresp.gusb_pkt.pkt_id[0] == GUSB_RESPONSE_SDR) //Satellite Data Record
		{
			unsigned char *t = (unsigned char *)&(iresp.gusb_pkt.databuf[0]);
			for(int i=0 ; i < 12 ; i++)
			{
				m_sat_data[i].svid =  *t++;
				m_sat_data[i].snr =   ((*t)<<8) + *(t+1); t += 2;
				m_sat_data[i].elev =  *t++;
				m_sat_data[i].azmth = ((*t)<<8) + *(t+1); t += 2;
				m_sat_data[i].status = *t++;
			}

			m_nSats = 0;
			for(int i=0 ; i < 12 ; i++)
			{
				if(m_sat_data[i].svid != 255)
					m_nSats++;
			}

			// Synthesize an NMEA GMGSV message
			SENTENCE snt;
			NMEA0183 oNMEA0183;
			oNMEA0183.TalkerID = _T ( "GM" );
			oNMEA0183.Gsv.SatsInView = m_nSats;

			oNMEA0183.Gsv.Write ( snt );
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
		}

		if(iresp.gusb_pkt.pkt_id[0] == GUSB_RESPONSE_PVT)     //PVT Data Record
		{
			D800_Pvt_Data_Type *ppvt = (D800_Pvt_Data_Type *)&(iresp.gusb_pkt.databuf[0]);

			if((ppvt->fix) >= 2 && (ppvt->fix <= 5)) {
				// Synthesize an NMEA GMRMC message
				SENTENCE snt;
				NMEA0183 oNMEA0183;
				oNMEA0183.TalkerID = _T ( "GM" );

				if ( ppvt->lat < 0. )
					oNMEA0183.Rmc.Position.Latitude.Set ( -ppvt->lat*180.0/M_PI, _T ( "S" ) );
				else
					oNMEA0183.Rmc.Position.Latitude.Set ( ppvt->lat*180.0/M_PI, _T ( "N" ) );

				if ( ppvt->lon < 0. )
					oNMEA0183.Rmc.Position.Longitude.Set ( -ppvt->lon*180.0/M_PI, _T ( "W" ) );
				else
					oNMEA0183.Rmc.Position.Longitude.Set ( ppvt->lon*180.0/M_PI, _T ( "E" ) );

				/* speed over ground */
				double sog = sqrt(ppvt->east*ppvt->east + ppvt->north*ppvt->north) * 3.6 / 1.852;
				oNMEA0183.Rmc.SpeedOverGroundKnots = sog;

				/* course over ground */
				double course = atan2(ppvt->east, ppvt->north);
				if (course < 0)
					course += 2.0 * M_PI;
				double cog = course * 180.0 / M_PI;
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
			}
		}
	}

thread_prexit:
	m_parent->m_Thread_run_flag = -1;
	return 0;
}


int GARMIN_USB_Thread::gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz)
{
	int rv = 0;
	int orig_receive_state;
top: // FIXME: no, just no
	orig_receive_state = m_receive_state;
	switch (m_receive_state) {
		case rs_fromintr:
			rv = gusb_win_get(ibuf, sz);
			break;
		case rs_frombulk:
			rv = gusb_win_get_bulk(ibuf, sz);
			break;
	}

	/* Adjust internal state and retry the read */
	if ((rv > 0) && (ibuf->gusb_pkt.pkt_id[0] == GUSB_REQUEST_BULK)) {
		m_receive_state = rs_frombulk;
		goto top;
	}
	/*
	 * If we were reading from the bulk pipe and we just got
	 * a zero request, adjust our internal state.
	 * It's tempting to retry the read here to hide this "stray"
	 * packet from our callers, but that only works when you know
	 * there's another packet coming.   That works in every case
	 * except the A000 discovery sequence.
	 */
	if ((m_receive_state == rs_frombulk) && (rv <= 0)) {
		m_receive_state = rs_fromintr;
	}

	return rv;
}

int GARMIN_USB_Thread::gusb_win_get(garmin_usb_packet *ibuf, size_t sz)
{
	int tsz=0;
#ifdef __WXMSW__
	DWORD rxed = GARMIN_USB_INTERRUPT_DATA_SIZE;
	unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];

	while (sz)
	{
		/* The driver wrongly (IMO) rejects reads smaller than
		 * GARMIN_USB_INTERRUPT_DATA_SIZE
		 */
		if(!DeviceIoControl(m_usb_handle, IOCTL_GARMIN_USB_INTERRUPT_IN, NULL, 0,
					buf, GARMIN_USB_INTERRUPT_DATA_SIZE, &rxed, NULL))
		{
			//                GPS_Serial_Error("Ioctl");
			//                fatal("ioctl\n");
		}

		buf += rxed;
		sz  -= rxed;
		tsz += rxed;
		if (rxed < GARMIN_USB_INTERRUPT_DATA_SIZE)
			break;
	}

#endif
	return tsz;
}

int GARMIN_USB_Thread::gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz)
{
	int ret_val = 0;

#ifdef __WXMSW__
	DWORD rsz;
	unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];

	int n = ReadFile(m_usb_handle, buf, sz, &rsz, NULL);
	ret_val = rsz;
#endif

	return ret_val;
}

