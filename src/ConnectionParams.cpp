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

#include "ConnectionParams.h"
#include <wx/tokenzr.h>
#include <wx/intl.h>

ConnectionParams::ConnectionParams()
	: Type(SERIAL)
	, NetProtocol(TCP)
	, NetworkAddress(wxEmptyString)
	, NetworkPort(0)
	, LastNetworkAddress(wxEmptyString)
	, LastNetworkPort(0)
	, Protocol(PROTO_NMEA0183)
	, Port(wxEmptyString)
	, Baudrate(4800)
	, ChecksumCheck(true)
	, Garmin(false)
	, GarminUpload(false)
	, FurunoGP3X(false)
	, Output(false)
	, InputSentenceListType(WHITELIST)
	, OutputSentenceListType(WHITELIST)
	, Priority(0)
	, bEnabled(true)
	, Valid(true)
	, b_IsSetup(false)
{
}

ConnectionParams::ConnectionParams(const wxString& configStr)
{
	Deserialize(configStr);
}

ConnectionParams::ConnectionParams(const wxString& port, int baudrate, bool is_garmin)
	: Type(SERIAL)
	, NetProtocol(TCP)
	, NetworkAddress(wxEmptyString)
	, NetworkPort(0)
	, LastNetworkAddress(wxEmptyString)
	, LastNetworkPort(0)
	, Protocol(PROTO_NMEA0183)
	, Port(port)
	, Baudrate(baudrate)
	, ChecksumCheck(true)
	, Garmin(is_garmin)
	, GarminUpload(false)
	, FurunoGP3X(false)
	, Output(false)
	, InputSentenceListType(WHITELIST)
	, OutputSentenceListType(WHITELIST)
	, Priority(0)
	, bEnabled(true)
	, Valid(true)
	, b_IsSetup(false)
{
}

ConnectionParams ConnectionParams::createOutput(const wxString& port, const wxString& sentence)
{
	ConnectionParams param(port, 4800);
	param.enableOutput(sentence);
	return param;
}

void ConnectionParams::Deserialize(const wxString& configStr)
{
	Valid = true;
	wxArrayString prms = wxStringTokenize(configStr, _T(";"));
	if (prms.Count() < 17) {
		Valid = false;
		return;
	}

	Type = (ConnectionType)wxAtoi(prms[0]);
	NetProtocol = (NetworkProtocol)wxAtoi(prms[1]);
	NetworkAddress = prms[2];
	NetworkPort = (ConnectionType)wxAtoi(prms[3]);
	Protocol = (DataProtocol)wxAtoi(prms[4]);
	Port = prms[5];
	Baudrate = wxAtoi(prms[6]);
	ChecksumCheck = !!wxAtoi(prms[7]);
	Output = !!wxAtoi(prms[8]);
	InputSentenceListType = (ListType)wxAtoi(prms[9]);
	InputSentenceList = wxStringTokenize(prms[10], _T(","));
	OutputSentenceListType = (ListType)wxAtoi(prms[11]);
	OutputSentenceList = wxStringTokenize(prms[12], _T(","));
	Priority = wxAtoi(prms[13]);
	Garmin = !!wxAtoi(prms[14]);
	GarminUpload = !!wxAtoi(prms[15]);
	FurunoGP3X = !!wxAtoi(prms[16]);

	bEnabled = true;
	if (prms.Count() >= 18)
		bEnabled = !!wxAtoi(prms[17]);
}

wxString ConnectionParams::Serialize()
{
	wxString istcs;
	for (size_t i = 0; i < InputSentenceList.size(); i++) {
		if (i > 0)
			istcs.Append(_T(","));
		istcs.Append(InputSentenceList[i]);
	}
	wxString ostcs;
	for (size_t i = 0; i < OutputSentenceList.size(); i++) {
		if (i > 0)
			ostcs.Append(_T(","));
		ostcs.Append(OutputSentenceList[i]);
	}
	return wxString::Format(_T("%d;%d;%s;%d;%d;%s;%d;%d;%d;%d;%s;%d;%s;%d;%d;%d;%d;%d"), Type,
							NetProtocol, NetworkAddress.c_str(), NetworkPort, Protocol,
							Port.c_str(), Baudrate, ChecksumCheck, Output, InputSentenceListType,
							istcs.c_str(), OutputSentenceListType, ostcs.c_str(), Priority, Garmin,
							GarminUpload, FurunoGP3X, bEnabled);
}

wxString ConnectionParams::GetSourceTypeStr() const
{
	if (Type == SERIAL)
		return _("Serial");
	else
		return _("Net");
}

wxString ConnectionParams::GetAddressStr() const
{
	if (Type == SERIAL)
		return wxString::Format(_T("%s"), Port.c_str());
	else
		return wxString::Format(_T("%s:%d"), NetworkAddress.c_str(), NetworkPort);
}

wxString ConnectionParams::GetParametersStr() const
{
	if (Type == SERIAL)
		return wxString::Format(_T("%d"), Baudrate);
	else if (NetProtocol == TCP)
		return _("TCP");
	else if (NetProtocol == UDP)
		return _("UDP");
	else
		return _("GPSD");
}

wxString ConnectionParams::GetOutputValueStr() const
{
	if (Output)
		return _("Yes");
	else
		return _("No");
}

wxString ConnectionParams::FilterTypeToStr(ListType type, FilterDirection dir) const
{
	if (dir == FILTER_INPUT) {
		if (type == BLACKLIST)
			return _("Reject");
		else
			return _("Accept");
	} else {
		if (type == BLACKLIST)
			return _("Drop");
		else
			return _("Send");
	}
}

wxString ConnectionParams::GetFiltersStr() const
{
	wxString istcs;
	for (size_t i = 0; i < InputSentenceList.size(); i++) {
		if (i > 0)
			istcs.Append(_T(","));
		istcs.Append(InputSentenceList[i]);
	}
	wxString ostcs;
	for (size_t i = 0; i < OutputSentenceList.size(); i++) {
		if (i > 0)
			ostcs.Append(_T(","));
		ostcs.Append(OutputSentenceList[i]);
	}
	wxString ret = wxEmptyString;
	if (istcs.Len() > 0) {
		ret.Append(_("In"));
		ret.Append(wxString::Format(_T(": %s %s"),
									FilterTypeToStr(InputSentenceListType, FILTER_INPUT).c_str(),
									istcs.c_str()));
	} else
		ret.Append(_("In: None"));

	if (ostcs.Len() > 0) {
		ret.Append(_T(", "));
		ret.Append(_("Out"));
		ret.Append(wxString::Format(_T(": %s %s"),
									FilterTypeToStr(OutputSentenceListType, FILTER_OUTPUT).c_str(),
									ostcs.c_str()));
	} else
		ret.Append(_(", Out: None"));
	return ret;
}

wxString ConnectionParams::GetDSPort() const
{
	if (Type == SERIAL)
		return wxString::Format(_T("Serial:%s"), Port.c_str());
	else {
		wxString proto;
		if (NetProtocol == TCP)
			proto = _T("TCP");
		else if (NetProtocol == UDP)
			proto = _T("UDP");
		else
			proto = _T("GPSD");
		return wxString::Format(_T("%s:%s:%d"), proto.c_str(), NetworkAddress.c_str(), NetworkPort);
	}
}

bool ConnectionParams::isSetup() const
{
	return b_IsSetup;
}

bool ConnectionParams::isEnabled() const
{
	return bEnabled;
}

bool ConnectionParams::isOutput() const
{
	return Output;
}

bool ConnectionParams::isGarmin() const
{
	return Garmin;
}

bool ConnectionParams::isChecksumCheck() const
{
	return ChecksumCheck;
}

ConnectionParams::ConnectionType ConnectionParams::getType() const
{
	return Type;
}

int ConnectionParams::getBaudrate() const
{
	return Baudrate;
}

int ConnectionParams::getPriority() const
{
	return Priority;
}

const wxString& ConnectionParams::getPort() const
{
	return Port;
}

ConnectionParams::ListType ConnectionParams::getInputSentenceListType() const
{
	return InputSentenceListType;
}

const wxArrayString& ConnectionParams::getInputSentenceList() const
{
	return InputSentenceList;
}

ConnectionParams::ListType ConnectionParams::getOutputSentenceListType() const
{
	return OutputSentenceListType;
}

const wxArrayString& ConnectionParams::getOutputSentenceList() const
{
	return OutputSentenceList;
}

void ConnectionParams::enableOutput(const wxString& sentence)
{
	Output = true;
	OutputSentenceListType = ConnectionParams::WHITELIST;
	OutputSentenceList.push_back(sentence);
}

void ConnectionParams::toggleEnabled()
{
	bEnabled = !bEnabled;
}

wxString ConnectionParams::GetLastDSPort()
{
	if (Type == SERIAL)
		return wxString::Format(_T("Serial:%s"), Port.c_str());
	else {
		wxString proto;
		if (NetProtocol == TCP)
			proto = _T("TCP");
		else if (NetProtocol == UDP)
			proto = _T("UDP");
		else
			proto = _T("GPSD");
		return wxString::Format(_T("%s:%s:%d"), proto.c_str(), LastNetworkAddress.c_str(),
								LastNetworkPort);
	}
}

