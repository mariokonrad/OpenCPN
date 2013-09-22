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

#ifndef __GARMIN_USB_THREAD__H__
#define __GARMIN_USB_THREAD__H__

#include <wx/thread.h>
#include <wx/event.h>

#include <garmin/cpo_satellite_data.h>
#include <garmin/unit_info.h>
#include <garmin/usb_packet.h>

class DataStream;
class GarminProtocolHandler;

// Garmin USB Worker Thread
//
// This thread manages reading the positioning data stream from the declared Garmin USB device
class GARMIN_USB_Thread : public wxThread
{
	public:
		GARMIN_USB_Thread(
				GarminProtocolHandler * parent,
				DataStream *GParentStream,
				wxEvtHandler *MessageTarget,
				unsigned int device_handle,
				size_t max_tx_size);
		virtual ~GARMIN_USB_Thread(void);
		void *Entry();

	private:
		DataStream * m_parent_stream;

		int gusb_win_get(garmin_usb_packet *ibuf, size_t sz);
		int gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz);
		int gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz);

		wxEvtHandler * m_pMessageTarget;
		GarminProtocolHandler * m_parent;

		int m_receive_state;
		cpo_sat_data m_sat_data[12];
		unit_info_type grmin_unit_info[2];
		int m_nSats;
		int m_max_tx_size;
#ifdef __WXMSW__
		HANDLE m_usb_handle;
#endif
};

#endif
