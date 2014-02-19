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

// If defined, update the system time using GPS receiver data.
// Time update is applied if the system time and GPS time differ
// by more than one minute, and only once per session.
// On Linux, this option requires root privileges, obtained by sudo.
// Thus, the following line is required in etc/sudoers:
//
//     nav ALL=NOPASSWD:/bin/date -s *
//
// Where "nav" is the user's user name.
//
// Also, the opencpn configuration file must contain the key
// [Settings]
//     SetSystemTime=1
// For security, this option is not available on the "Options" dialog
#define ocpnUPDATE_SYSTEM_TIME



#include "SystemTime.h"

#include <global/OCPN.h>
#include <global/System.h>

#include <wx/string.h>
#include <wx/datetime.h>
#include <wx/log.h>
#include <wx/utils.h>

#ifdef __WXMSW__
	#include <windows.h>
#endif


/// Sets the system time, depending on the global configuration.
///
/// This method is platform specific.
///
/// @retval true The system time was set.
/// @retval false The system time was not set.
bool SystemTime::set(const wxString& sfixtime)
{
#ifdef ocpnUPDATE_SYSTEM_TIME
	// Use the fix time to update the local system clock, only once per session
	if ((sfixtime.Len()) && global::OCPN::get().sys().config().SetSystemTime) {
		wxDateTime Fix_Time;

		if (6 == sfixtime.Len()) // perfectly recognised format?
		{
			wxString a;
			long b;
			int hr = 0;
			int min = 0;
			int sec = 0;

			a = sfixtime.Mid(0, 2);
			if (a.ToLong(&b))
				hr = b;
			a = sfixtime.Mid(2, 2);
			if (a.ToLong(&b))
				min = b;
			a = sfixtime.Mid(4, 2);
			if (a.ToLong(&b))
				sec = b;

			Fix_Time.Set(hr, min, sec);
		}
		wxString fix_time_format
			= Fix_Time.Format(_T("%Y-%m-%dT%H:%M:%S")); // this should show as LOCAL

		// Compare the server (fix) time to the current system time
		wxDateTime sdt;
		sdt.SetToCurrent();
		wxDateTime cwxft = Fix_Time; // take a copy
		wxTimeSpan ts;
		ts = cwxft.Subtract(sdt);

		int b = (ts.GetSeconds()).ToLong();

		// Correct system time if necessary
		// Only set the time if wrong by more than 1 minute, and less than 2 hours
		// This should eliminate bogus times which may come from faulty GPS units

		if ((abs(b) > 60) && (abs(b) < (2 * 60 * 60))) {

#ifdef __WXMSW__
			// Fix up the fix_time to convert to GMT
			Fix_Time = Fix_Time.ToGMT();

			// Code snippet following borrowed from wxDateCtrl, MSW

			const wxDateTime::Tm tm(Fix_Time.GetTm());

			SYSTEMTIME stm;
			stm.wYear = (WXWORD)tm.year;
			stm.wMonth = (WXWORD)(tm.mon - wxDateTime::Jan + 1);
			stm.wDay = tm.mday;

			stm.wDayOfWeek = 0;
			stm.wHour = Fix_Time.GetHour();
			stm.wMinute = tm.min;
			stm.wSecond = tm.sec;
			stm.wMilliseconds = 0;

			::SetSystemTime(&stm); // in GMT

#else

			// This contortion sets the system date/time on POSIX host
			// Requires the following line in /etc/sudoers
			//   nav ALL=NOPASSWD:/bin/date -s *

			wxString msg;
			msg.Printf(_T("Setting system time, delta t is %d seconds"), b);
			wxLogMessage(msg);

			wxString sdate(Fix_Time.Format(_T("%D")));
			sdate.Prepend(_T("sudo /bin/date -s \""));

			wxString stime(Fix_Time.Format(_T("%T")));
			stime.Prepend(_T(" "));
			sdate.Append(stime);
			sdate.Append(_T("\""));

			msg.Printf(_T("Linux command is:"));
			msg += sdate;
			wxLogMessage(msg);
			wxExecute(sdate, wxEXEC_ASYNC);

#endif
			return true;

		}
	}

#endif
	return false;
}

