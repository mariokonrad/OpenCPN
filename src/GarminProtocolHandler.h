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

#ifndef __GARMINPROTOCOLHANDLER__H__
#define __GARMINPROTOCOLHANDLER__H__

#include <wx/event.h>
#include <wx/timer.h>
#include <wx/string.h>

#include <garmin/cpo_satellite_data.h>
#include <garmin/unit_info.h>

#ifdef __WXMSW__
	#include <garmin/usb_packet.h>
#endif

class GARMIN_Serial_Thread;
class GARMIN_USB_Thread;
class DataStream;

class GarminProtocolHandler : public wxEvtHandler
{
		DECLARE_EVENT_TABLE()

	public:
		GarminProtocolHandler(DataStream *parent, wxEvtHandler *MessageTarget,  bool bsel_usb);
		virtual ~GarminProtocolHandler();

		void Close(void);

		void StopIOThread(bool b_pause);
		void RestartIOThread(void);

		void StopSerialThread(void);

		void OnTimerGarmin1(wxTimerEvent & event);

		bool FindGarminDeviceInterface();

		wxEvtHandler            *m_pMainEventHandler;
		DataStream              *m_pparent;

		int                     m_max_tx_size;
		int                     m_receive_state;
		cpo_sat_data            m_sat_data[12];
		unit_info_type          grmin_unit_info[2];
		int                     m_nSats;
		wxTimer                 TimerGarmin1;

		int                     m_Thread_run_flag;
		GARMIN_Serial_Thread    *m_garmin_serial_thread;
		GARMIN_USB_Thread       *m_garmin_usb_thread;
		bool                    m_bneed_int_reset;
		int                     m_ndelay;
		bool                    m_bOK;
		bool                    m_busb;
		wxString                m_port;

#ifdef __WXMSW__
		HANDLE garmin_usb_start();
		bool ResetGarminUSBDriver();
		bool IsGarminPlugged();
		bool gusb_syncup(void);

		int gusb_win_get(garmin_usb_packet *ibuf, size_t sz);
		int gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz);
		int gusb_win_send(const garmin_usb_packet *opkt, size_t sz);

		int gusb_cmd_send(const garmin_usb_packet *opkt, size_t sz);
		int gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz);

		HANDLE m_usb_handle;

		WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif
};


#endif
