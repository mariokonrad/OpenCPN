/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __CONNECTIONPARAMS_H__
#define __CONNECTIONPARAMS_H__

#include <wx/string.h>
#include <wx/arrstr.h>
#include <vector>

class ConnectionParams
{
	public:
		enum ConnectionType
		{
			SERIAL = 0,
			NETWORK = 1
		};

		enum NetworkProtocol
		{
			TCP = 0,
			UDP = 1,
			GPSD = 2
		};

		enum ListType
		{
			WHITELIST = 0,
			BLACKLIST = 1
		};

		enum FilterDirection
		{
			FILTER_INPUT = 0,
			FILTER_OUTPUT = 1
		};

		enum DataProtocol
		{
			PROTO_NMEA0183 = 0,
			PROTO_SEATALK = 1,
			PROTO_NMEA2000 = 2
		};

	public:
		ConnectionParams();
		ConnectionParams(const wxString & configStr);

		// FIXME: attributes should be private

		ConnectionType Type;
		NetworkProtocol NetProtocol;
		wxString NetworkAddress;
		int NetworkPort;

		DataProtocol Protocol;
		wxString Port;
		int Baudrate;
		bool ChecksumCheck;
		bool Garmin;
		bool GarminUpload;
		bool FurunoGP3X;
		bool Output;
		ListType InputSentenceListType;
		wxArrayString InputSentenceList;
		ListType OutputSentenceListType;
		wxArrayString OutputSentenceList;
		int Priority;
		bool bEnabled;

		wxString Serialize();
		void Deserialize(const wxString &configStr);

		wxString GetSourceTypeStr();
		wxString GetAddressStr() const;
		wxString GetParametersStr();
		wxString GetOutputValueStr();
		wxString GetFiltersStr();
		wxString GetDSPort() const;

		bool Valid;
		bool b_IsSetup;

	private:
		wxString FilterTypeToStr(ListType type, FilterDirection dir);
};

typedef std::vector<ConnectionParams> ArrayOfConnPrm;

#endif
