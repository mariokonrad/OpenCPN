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

#ifndef __MULTIPLEXER_H__
#define __MULTIPLEXER_H__

#include <dsPortType.h>
#include <ConnectionParams.h>
#include <Route.h>
#include <RoutePoint.h>

#include <wx/event.h>

#include <vector>

class wxGauge;
class OCPN_DataStreamEvent;
class DataStream;

// Garmin interface private error codes
#define ERR_GARMIN_INITIALIZE           -1
#define ERR_GARMIN_GENERAL              -2

class Multiplexer : public wxEvtHandler
{
public:
	Multiplexer();
	virtual ~Multiplexer();
	void AddStream(DataStream* stream);
	void StopAllStreams();
	void ClearStreams();
	DataStream* FindStream(const wxString& port);
	void StopAndRemoveStream(DataStream* stream);
	void SaveStreamProperties(DataStream* stream);
	bool CreateAndRestoreSavedStreamProperties();

	void SendNMEAMessage(const wxString& msg);
	void SetAISHandler(wxEvtHandler* handler);
	void SetGPSHandler(wxEvtHandler* handler);

	int SendRouteToGPS(Route* pr, const wxString& com_name, bool bsend_waypoints,
					   wxGauge* pProgress);
	int SendWaypointToGPS(RoutePoint* prp, const wxString& com_name, wxGauge* pProgress);

	void LogOutputMessage(const wxString& msg, wxString stream_name, bool b_filter);
	void LogOutputMessageColor(const wxString& msg, const wxString& stream_name,
							   const wxString& color);
	void LogInputMessage(const wxString& msg, const wxString& stream_name, bool b_filter);

private:
	void OnEvtStream(OCPN_DataStreamEvent& event);

	typedef std::vector<DataStream*> DataStreams;
	DataStreams datastreams;

	wxEvtHandler* m_aisconsumer;
	wxEvtHandler* m_gpsconsumer;

	// A set of temporarily saved parameters for a DataStream
	wxString port_save;
	wxString baud_rate_save;
	dsPortType port_type_save;
	int priority_save;
	wxArrayString input_sentence_list_save;
	ConnectionParams::ListType input_sentence_list_type_save;
	wxArrayString output_sentence_list_save;
	ConnectionParams::ListType output_sentence_list_type_save;
	bool bchecksum_check_save;
	bool bGarmin_GRMN_mode_save;
};

#endif
