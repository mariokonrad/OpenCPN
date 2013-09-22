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

#ifndef __GARMIN_SERIAL_THREAD__H__
#define __GARMIN_SERIAL_THREAD__H__

#include <wx/thread.h>
#include <wx/string.h>

class GarminProtocolHandler;
class DataStream;
class wxEvtHandler;

// Garmin Serial Port Worker Thread
//
// This thread manages reading the positioning data stream from the declared Garmin GRMN Mode serial device
class GARMIN_Serial_Thread : public wxThread
{
	public:
		GARMIN_Serial_Thread(
				GarminProtocolHandler * parent,
				DataStream * GParentStream,
				wxEvtHandler * MessageTarget,
				wxString port);
		virtual ~GARMIN_Serial_Thread(void);
		void *Entry();
		void string(wxCharBuffer mb_str);

	private:
		wxEvtHandler * m_pMessageTarget;
		GarminProtocolHandler * m_parent;
		DataStream * m_parent_stream;

		wxString m_port;
		bool m_bconnected;
		bool m_bdetected;
};

#endif
