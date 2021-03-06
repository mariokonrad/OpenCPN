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

#include "Multiplexer.h"
#include <NMEALogWindow.h>
#include <OCPN_DataStreamEvent.h>
#include <DataStream.h>

#include <global/OCPN.h>
#include <global/AIS.h>
#include <global/System.h>

#include <plugin/PlugInManager.h>

#include <nmea0183/nmea0183.h>

#include <garmin/jeeps/garmin_wrapper.h>

#include <wx/gauge.h>

#include <algorithm>

extern PlugInManager* g_pi_manager;

Multiplexer::Multiplexer()
	: m_aisconsumer(NULL)
	, m_gpsconsumer(NULL)
{
	Connect(wxEVT_OCPN_DATASTREAM,
			(wxObjectEventFunction)(wxEventFunction) &Multiplexer::OnEvtStream);
}

Multiplexer::~Multiplexer()
{
	ClearStreams();
}

void Multiplexer::AddStream(DataStream* stream)
{
	datastreams.push_back(stream);
}

void Multiplexer::StopAllStreams()
{
	for (DataStreams::iterator i = datastreams.begin(); i != datastreams.end(); ++i)
		(*i)->Close();
}

void Multiplexer::ClearStreams()
{
	StopAllStreams();
	// FIXME: delete objects?
	datastreams.clear();
}

DataStream* Multiplexer::FindStream(const wxString& port)
{
	for (DataStreams::iterator i = datastreams.begin(); i != datastreams.end(); ++i) {
		DataStream* stream = *i;
		if (stream && stream->GetPort() == port)
			return stream;
	}
	return NULL;
}

void Multiplexer::StopAndRemoveStream(DataStream* stream)
{
	if (stream)
		stream->Close();

	DataStreams::iterator i = std::find(datastreams.begin(), datastreams.end(), stream);
	if (i != datastreams.end()) {
		datastreams.erase(i);
		// FIXME: delete object?
	}
}

void Multiplexer::LogOutputMessageColor(
		const wxString & msg,
		const wxString & stream_name,
		const wxString & color)
{
	if (NMEALogWindow::Get().Active()) {
		wxDateTime now = wxDateTime::Now();
		wxString ss = now.FormatISOTime();
		ss.Prepend(_T("--> "));
		ss.Append(_T(" ("));
		ss.Append(stream_name);
		ss.Append(_T(") "));
		ss.Append(msg);
		ss.Prepend(color);

		NMEALogWindow::Get().Add(ss);
	}
}

void Multiplexer::LogOutputMessage(const wxString& msg, wxString stream_name, bool b_filter)
{
	if (b_filter)
		LogOutputMessageColor(msg, stream_name, _T("<AMBER>"));
	else
		LogOutputMessageColor(msg, stream_name, _T("<BLUE>"));
}

void Multiplexer::LogInputMessage(const wxString& msg, const wxString& stream_name, bool b_filter)
{
	if (NMEALogWindow::Get().Active()) {
		wxDateTime now = wxDateTime::Now();
		wxString ss = now.FormatISOTime();
		ss.Append(_T(" ("));
		ss.Append(stream_name);
		ss.Append(_T(") "));
		ss.Append(msg);
		if (b_filter)
			ss.Prepend(_T("<AMBER>"));
		else
			ss.Prepend(_T("<GREEN>"));

		NMEALogWindow::Get().Add(ss);
	}
}

void Multiplexer::SendNMEAMessage(const wxString& msg)
{
	// Send to all the outputs
	for (DataStreams::iterator i = datastreams.begin(); i != datastreams.end(); ++i) {
		DataStream* s = *i;
		if (s->IsOk()
			&& (s->GetIoSelect() == DS_TYPE_INPUT_OUTPUT || s->GetIoSelect() == DS_TYPE_OUTPUT)) {
			bool bout_filter = true;

			bool bxmit_ok = true;
			if (s->SentencePassesFilter(msg, ConnectionParams::FILTER_OUTPUT)) {
				bxmit_ok = s->SendSentence(msg);
				bout_filter = false;
			}
			// Send to the Debug Window, if open
			if (!bout_filter) {
				if (bxmit_ok)
					LogOutputMessageColor(msg, s->GetPort(), _T("<BLUE>"));
				else
					LogOutputMessageColor(msg, s->GetPort(), _T("<RED>"));
			} else
				LogOutputMessageColor(msg, s->GetPort(), _T("<AMBER>"));
		}
	}
	// Send to plugins
	if (g_pi_manager)
		g_pi_manager->SendNMEASentenceToAllPlugIns(msg);
}

void Multiplexer::SetAISHandler(wxEvtHandler* handler)
{
	m_aisconsumer = handler;
}

void Multiplexer::SetGPSHandler(wxEvtHandler* handler)
{
	m_gpsconsumer = handler;
}

void Multiplexer::OnEvtStream(OCPN_DataStreamEvent& event)
{
	wxString message = wxString(event.GetNMEAString().c_str(), wxConvUTF8);

	DataStream* stream = event.GetStream();
	wxString port(_T("Virtual:"));
	if (stream)
		port = wxString(stream->GetPort());

	if (!message.IsEmpty()) {
		// Send to core consumers
		// if it passes the source's input filter
		//  If there is no datastream, as for PlugIns, then pass everything
		bool bpass = true;
		if (stream)
			bpass = stream->SentencePassesFilter(message, ConnectionParams::FILTER_INPUT);

		const global::AIS::Data& ais = global::OCPN::get().ais().get_data();

		if (bpass) {
			if (message.Mid(3, 3).IsSameAs(_T("VDM")) || message.Mid(1, 5).IsSameAs(_T("FRPOS"))
				|| message.Mid(1, 2).IsSameAs(_T("CD")) || message.Mid(3, 3).IsSameAs(_T("TLL"))
				|| message.Mid(3, 3).IsSameAs(_T("TTM")) || message.Mid(3, 3).IsSameAs(_T("OSD"))
				|| (ais.WplIsAprsPosition && message.Mid(3, 3).IsSameAs(_T("WPL")))) {
				if (m_aisconsumer)
					m_aisconsumer->AddPendingEvent(event);
			} else {
				if (m_gpsconsumer)
					m_gpsconsumer->AddPendingEvent(event);
			}
		}

		// Send to the Debug Window, if open
		LogInputMessage(message, port, !bpass);

		// Send to plugins
		if (g_pi_manager)
			g_pi_manager->SendNMEASentenceToAllPlugIns(message);

		// Send to all the other outputs
		for (DataStreams::iterator i = datastreams.begin(); i != datastreams.end(); ++i) {
			DataStream* s = *i;
			if (!s->IsOk())
				continue;

			if ((s->GetConnectionType() == ConnectionParams::SERIAL) || (s->GetPort() != port)) {
				if ((s->GetIoSelect() == DS_TYPE_INPUT_OUTPUT)
					|| (s->GetIoSelect() == DS_TYPE_OUTPUT)) {
					bool bout_filter = true;

					bool bxmit_ok = true;
					if (s->SentencePassesFilter(message, ConnectionParams::FILTER_OUTPUT)) {
						bxmit_ok = s->SendSentence(message);
						bout_filter = false;
					}

					// Send to the Debug Window, if open
					if (!bout_filter) {
						if (bxmit_ok)
							LogOutputMessageColor(message, s->GetPort(), _T("<BLUE>"));
						else
							LogOutputMessageColor(message, s->GetPort(), _T("<RED>"));
					} else
						LogOutputMessageColor(message, s->GetPort(), _T("<AMBER>"));
				}
			}
		}
	}
}

void Multiplexer::SaveStreamProperties(DataStream* stream)
{
	if (!stream)
		return;

	port_save = stream->GetPort();
	baud_rate_save = stream->GetBaudRate();
	port_type_save = stream->GetPortType();
	priority_save = stream->GetPriority();
	input_sentence_list_save = stream->GetInputSentenceList();
	input_sentence_list_type_save = stream->GetInputSentenceListType();
	output_sentence_list_save = stream->GetOutputSentenceList();
	output_sentence_list_type_save = stream->GetOutputSentenceListType();
	bchecksum_check_save = stream->GetChecksumCheck();
	bGarmin_GRMN_mode_save = stream->GetGarminMode();
}

bool Multiplexer::CreateAndRestoreSavedStreamProperties()
{
	DataStream* dstr = new DataStream(this, port_save, baud_rate_save, port_type_save,
									  priority_save, bGarmin_GRMN_mode_save);
	dstr->SetInputFilter(input_sentence_list_save);
	dstr->SetInputFilterType(input_sentence_list_type_save);
	dstr->SetOutputFilter(output_sentence_list_save);
	dstr->SetOutputFilterType(output_sentence_list_type_save);
	dstr->SetChecksumCheck(bchecksum_check_save);

	AddStream(dstr);

	return true;
}

int Multiplexer::SendRouteToGPS(Route* pr, const wxString& com_name, bool bsend_waypoints,
								 wxGauge* pProgress)
{
	// FIXME: refactoring of method: too long, preprocessor stuff

	int ret_val = 0;
	DataStream* old_stream = FindStream(com_name);
	if (old_stream) {
		SaveStreamProperties(old_stream);
		StopAndRemoveStream(old_stream);
	}

	const global::System::Config& cfg = global::OCPN::get().sys().config();

#ifdef USE_GARMINHOST
#ifdef __WXMSW__
	if (com_name.Upper().Matches(_T("*GARMIN*"))) { // Garmin USB Mode
		int v_init = Garmin_GPS_Init(wxString(_T("usb:")));

		if (v_init < 0) {
			wxString msg(_T(" Garmin USB GPS could not be initialized"));
			wxLogMessage(msg);
			msg.Printf(_T(" Error Code is %d"), v_init);
			wxLogMessage(msg);
			msg = _T(" LastGarminError is: ");
			msg += GetLastGarminError();
			wxLogMessage(msg);

			ret_val = ERR_GARMIN_INITIALIZE;
		} else {
			wxLogMessage(_T("Garmin USB Initialized"));

			wxString msg = _T("USB Unit identifies as: ");
			wxString GPS_Unit = Garmin_GPS_GetSaveString();
			msg += GPS_Unit;
			wxLogMessage(msg);

			wxLogMessage(_T("Sending Routes..."));
			int ret1 = Garmin_GPS_SendRoute(wxString(_T("usb:")), pr, pProgress);

			if (ret1 != 1) {
				wxLogMessage(_T(" Error Sending Routes"));
				wxString msg;
				msg = _T(" LastGarminError is: ");
				msg += GetLastGarminError();
				wxLogMessage(msg);

				ret_val = ERR_GARMIN_GENERAL;
			} else {
				ret_val = 0;
			}
		}

		goto ret_point_1; // FIXME: spaghetti code
	}
#endif

	if (cfg.GarminHostUpload) {
		int lret_val; // FIXME: refactoring, WTF?
		if (pProgress) {
			pProgress->SetValue(20);
			pProgress->Refresh();
			pProgress->Update();
		}

		wxString short_com = com_name.Mid(7);
		// Initialize the Garmin receiver, build required Jeeps internal data structures
		int v_init = Garmin_GPS_Init(short_com);
		if (v_init < 0) {
			wxString msg(_T("Garmin GPS could not be initialized on port: "));
			msg += short_com;
			wxString err;
			err.Printf(_T(" Error Code is %d"), v_init);
			msg += err;

			msg += _T("\n LastGarminError is: ");
			msg += GetLastGarminError();

			wxLogMessage(msg);

			ret_val = ERR_GARMIN_INITIALIZE;
			goto ret_point;
		} else {
			wxString msg(_T("Sent Route to Garmin GPS on port: "));
			msg += short_com;
			msg += _T("\n Unit identifies as: ");
			wxString GPS_Unit = Garmin_GPS_GetSaveString();
			msg += GPS_Unit;

			wxLogMessage(msg);
		}

		if (pProgress) {
			pProgress->SetValue(40);
			pProgress->Refresh();
			pProgress->Update();
		}

		lret_val = Garmin_GPS_SendRoute(short_com, pr, pProgress);
		if (lret_val != 1) {
			wxString msg(_T("Error Sending Route to Garmin GPS on port: "));
			msg += short_com;
			wxString err;
			err.Printf(_T(" Error Code is %d"), ret_val);

			msg += _T("\n LastGarminError is: ");
			msg += GetLastGarminError();

			msg += err;
			wxLogMessage(msg);

			ret_val = ERR_GARMIN_GENERAL;
			goto ret_point;
		} else {
			ret_val = 0;
		}

ret_point:

		if (pProgress) {
			pProgress->SetValue(100);
			pProgress->Refresh();
			pProgress->Update();
		}

		wxMilliSleep(500);

			goto ret_point_1;
	} else
#endif //USE_GARMINHOST

	{
		{ // Standard NMEA mode

			// If the port was temporarily closed, reopen as I/O type
			// Otherwise, open another port using default properties
			wxString baud;

			if (old_stream) {
				baud = baud_rate_save;
			} else {
				baud = _T("4800");
			}

			DataStream* dstr = new DataStream(this, com_name, baud, DS_TYPE_INPUT_OUTPUT, 0);

			// Wait up to 1 seconds for Datastream secondary thread to come up
			int timeout = 0;
			while (!dstr->IsSecThreadActive() && (timeout < 10)) {
				wxMilliSleep(100);
				timeout++;
			}

			SENTENCE snt;
			NMEA0183 oNMEA0183;
			oNMEA0183.TalkerID = _T("EC");

			int nProg = pr->routepoints().size() + 1;
			if (pProgress)
				pProgress->SetRange(100);

			int progress_stall = 500;
			if (pr->routepoints().size() > 10)
				progress_stall = 200;

			if (!pProgress)
				progress_stall = 200; // 80 chars at 4800 baud is ~160 msec

			// Send out the waypoints, in order
			if (bsend_waypoints) {

				int ip = 1;
				for (RoutePointList::iterator node = pr->routepoints().begin();
					 node != pr->routepoints().end(); ++node) {
					RoutePoint* prp = *node;

					if (cfg.GPS_Ident == _T("Generic")) {
						if (prp->latitude() < 0.0)
							oNMEA0183.Wpl.Position.Latitude.Set(-prp->latitude(), _T("S"));
						else
							oNMEA0183.Wpl.Position.Latitude.Set(prp->latitude(), _T("N"));

						if (prp->longitude() < 0.0)
							oNMEA0183.Wpl.Position.Longitude.Set(-prp->longitude(), _T("W"));
						else
							oNMEA0183.Wpl.Position.Longitude.Set(prp->longitude(), _T("E"));

						wxString name = prp->GetName();
						oNMEA0183.Wpl.To = name.Truncate(6);

						oNMEA0183.Wpl.Write(snt);

					} else if (cfg.GPS_Ident == _T("FurunoGP3X")) {
						oNMEA0183.TalkerID = _T("PFEC,");

						if (prp->latitude() < 0.0)
							oNMEA0183.GPwpl.Position.Latitude.Set(-prp->latitude(), _T("S"));
						else
							oNMEA0183.GPwpl.Position.Latitude.Set(prp->latitude(), _T("N"));

						if (prp->longitude() < 0.0)
							oNMEA0183.GPwpl.Position.Longitude.Set(-prp->longitude(), _T("W"));
						else
							oNMEA0183.GPwpl.Position.Longitude.Set(prp->longitude(), _T("E"));

						wxString name = prp->GetName();
						name += _T("000000");
						name.Truncate(6);
						oNMEA0183.GPwpl.To = name;

						oNMEA0183.GPwpl.Write(snt);
					}

					if (dstr->SendSentence(snt.Sentence))
						LogOutputMessage(snt.Sentence, dstr->GetPort(), false);

					wxString msg(_T("-->GPS Port:"));
					msg += com_name;
					msg += _T(" Sentence: ");
					msg += snt.Sentence;
					msg.Trim();
					wxLogMessage(msg);

					if (pProgress) {
						pProgress->SetValue((ip * 100) / nProg);
						pProgress->Refresh();
						pProgress->Update();
					}

					wxMilliSleep(progress_stall);
					ip++;
				}
			}

			// Create the NMEA Rte sentence
			// Try to create a single sentence, and then check the length to see if too long
			oNMEA0183.Rte.Empty();
			oNMEA0183.Rte.TypeOfRoute = CompleteRoute;

			if (pr->get_name().IsEmpty())
				oNMEA0183.Rte.RouteName = _T("1");
			else
				oNMEA0183.Rte.RouteName = pr->get_name();

			if (cfg.GPS_Ident == _T("FurunoGP3X")) {
				oNMEA0183.Rte.RouteName = _T("01");
				oNMEA0183.TalkerID = _T("GP");
			}

			oNMEA0183.Rte.total_number_of_messages = 1;
			oNMEA0183.Rte.message_number = 1;

			// add the waypoints
			for (RoutePointList::iterator node = pr->routepoints().begin();
				 node != pr->routepoints().end(); ++node) {
				RoutePoint* prp = *node;
				wxString name = prp->GetName();
				name.Truncate(6);

				if (cfg.GPS_Ident == _T("FurunoGP3X")) {
					name = prp->GetName();
					name += _T("000000");
					name.Truncate(6);
					name.Prepend(_T(" ")); // What Furuno calls "Skip Code", space means use the WP
				}

				oNMEA0183.Rte.AddWaypoint(name);
			}

			oNMEA0183.Rte.Write(snt);

			unsigned int max_length = 76;

			if (snt.Sentence.Len() > max_length) { // Do we need split sentences?
				// Make a route with zero waypoints to get tare load.
				NMEA0183 tNMEA0183;
				SENTENCE tsnt;
				tNMEA0183.TalkerID = _T("EC");

				tNMEA0183.Rte.Empty();
				tNMEA0183.Rte.TypeOfRoute = CompleteRoute;

				if (cfg.GPS_Ident != _T("FurunoGP3X")) {
					if (pr->get_name().IsEmpty())
						tNMEA0183.Rte.RouteName = _T("1");
					else
						tNMEA0183.Rte.RouteName = pr->get_name();

				} else {
					tNMEA0183.Rte.RouteName = _T("01");
				}

				tNMEA0183.Rte.Write(tsnt);

				unsigned int tare_length = tsnt.Sentence.Len();

				wxArrayString sentence_array;

				// Trial balloon: add the waypoints, with length checking
				int n_total = 1;
				bool bnew_sentence = true;
				int sent_len = 0;

				RoutePointList::iterator i = pr->routepoints().begin();
				while (i != pr->routepoints().end()) {
					RoutePoint* prp = *i;
					wxString name = prp->GetName();
					name.Truncate(6);
					unsigned int name_len = name.Len();
					if (cfg.GPS_Ident == _T("FurunoGP3X"))
						name_len = 7; // six chars, with leading space for "Skip Code"

					if (bnew_sentence) {
						sent_len = tare_length;
						sent_len += name_len + 1; // with comma
						bnew_sentence = false;
						++i;

					} else {
						if (sent_len + name_len > max_length) {
							n_total++;
							bnew_sentence = true;
						} else {
							sent_len += name_len + 1; // with comma
							++i;
						}
					}
				}

				// Now we have the sentence count, so make the real sentences using the same
				// counting logic
				int final_total = n_total;
				int n_run = 1;
				bnew_sentence = true;

				RoutePointList::iterator node = pr->routepoints().begin();
				while (node != pr->routepoints().end()) {
					RoutePoint* prp = *node;
					wxString name = prp->GetName();
					name.Truncate(6);
					if (cfg.GPS_Ident == _T("FurunoGP3X")) {
						name = prp->GetName();
						name += _T("000000");
						name.Truncate(6);
						// What Furuno calls "Skip Code", space means use the WP
						name.Prepend(_T(" "));
					}

					unsigned int name_len = name.Len();

					if (bnew_sentence) {
						sent_len = tare_length;
						sent_len += name_len + 1; // comma
						bnew_sentence = false;

						oNMEA0183.Rte.Empty();
						oNMEA0183.Rte.TypeOfRoute = CompleteRoute;

						if (cfg.GPS_Ident != _T("FurunoGP3X")) {
							if (pr->get_name().IsEmpty())
								oNMEA0183.Rte.RouteName = _T("1");
							else
								oNMEA0183.Rte.RouteName = pr->get_name();
						} else {
							oNMEA0183.Rte.RouteName = _T("01");
						}

						oNMEA0183.Rte.total_number_of_messages = final_total;
						oNMEA0183.Rte.message_number = n_run;
						snt.Sentence.Clear();

						oNMEA0183.Rte.AddWaypoint(name);
						++node;
					} else {
						if (sent_len + name_len > max_length) {
							n_run++;
							bnew_sentence = true;
							oNMEA0183.Rte.Write(snt);
							sentence_array.Add(snt.Sentence);
						} else {
							sent_len += name_len + 1; // comma
							oNMEA0183.Rte.AddWaypoint(name);
							++node;
						}
					}
				}

				oNMEA0183.Rte.Write(snt); // last one...
				if (snt.Sentence.Len() > tare_length)
					sentence_array.Add(snt.Sentence);

				for (unsigned int ii = 0; ii < sentence_array.size(); ii++) {
					if (dstr->SendSentence(sentence_array.Item(ii)))
						LogOutputMessage(sentence_array.Item(ii), dstr->GetPort(), false);

					wxString msg(_T("-->GPS Port:"));
					msg += com_name;
					msg += _T(" Sentence: ");
					msg += sentence_array.Item(ii);
					msg.Trim();
					wxLogMessage(msg);

					wxMilliSleep(progress_stall);
				}
			} else {
				if (dstr->SendSentence(snt.Sentence))
					LogOutputMessage(snt.Sentence, dstr->GetPort(), false);

				wxString msg(_T("-->GPS Port:"));
				msg += com_name;
				msg += _T(" Sentence: ");
				msg += snt.Sentence;
				msg.Trim();
				wxLogMessage(msg);
			}

			if (cfg.GPS_Ident == _T("FurunoGP3X")) {
				wxString term;
				term.Printf(_T("$PFEC,GPxfr,CTL,E%c%c"), 0x0d, 0x0a);

				if (dstr->SendSentence(term))
					LogOutputMessage(term, dstr->GetPort(), false);

				wxString msg(_T("-->GPS Port:"));
				msg += com_name;
				msg += _T(" Sentence: ");
				msg += term;
				msg.Trim();
				wxLogMessage(msg);
			}

			if (pProgress) {
				pProgress->SetValue(100);
				pProgress->Refresh();
				pProgress->Update();
			}

			wxMilliSleep(progress_stall);

			ret_val = 0;

			// All finished with the temp port
			dstr->Close();
		}
	}

ret_point_1:

	if (old_stream)
		CreateAndRestoreSavedStreamProperties();

	return ret_val;
}

int Multiplexer::SendWaypointToGPS(RoutePoint* prp, const wxString& com_name, wxGauge* pProgress)
{
	// FIXME: refactoring of method: too long, preprocessor stuff

	int ret_val = 0;
	DataStream* old_stream = FindStream(com_name);
	if (old_stream) {
		SaveStreamProperties(old_stream);
		StopAndRemoveStream(old_stream);
	}

#ifdef USE_GARMINHOST
#ifdef __WXMSW__
	if(com_name.Upper().Matches(_T("*GARMIN*"))) { // Garmin USB Mode
		int v_init = Garmin_GPS_Init(wxString(_T("usb:")));

		if (v_init < 0) {
			wxString msg(_T(" Garmin USB GPS could not be initialized"));
			wxLogMessage(msg);
			msg.Printf(_T(" Error Code is %d"), v_init);
			wxLogMessage(msg);
			msg = _T(" LastGarminError is: ");
			msg += GetLastGarminError();
			wxLogMessage(msg);

			ret_val = ERR_GARMIN_INITIALIZE;
		} else {
			wxLogMessage(_T("Garmin USB Initialized"));

			wxString msg = _T("USB Unit identifies as: ");
			wxString GPS_Unit = Garmin_GPS_GetSaveString();
			msg += GPS_Unit;
			wxLogMessage(msg);

			wxLogMessage(_T("Sending Waypoint..."));

			RoutePointList rplist;
			rplist.push_back(prp);

			int ret1 = Garmin_GPS_SendWaypoints(wxString(_T("usb:")), &rplist);

			if (ret1 != 1) {
				wxLogMessage(_T(" Error Sending Waypoint to Garmin USB"));
				wxString msg;
				msg = _T(" LastGarminError is: ");
				msg += GetLastGarminError();
				wxLogMessage(msg);

				ret_val = ERR_GARMIN_GENERAL;
			} else {
				ret_val = 0;
			}
		}
		return ret_val;
	}
#endif

	// Are we using Garmin Host mode for uploads?
	const global::System::Config& cfg = global::OCPN::get().sys().config();
	if (cfg.GarminHostUpload) {
		wxString short_com = com_name.Mid(7);
		// Initialize the Garmin receiver, build required Jeeps internal data structures
		int v_init = Garmin_GPS_Init(short_com);
		if (v_init < 0) {
			wxString msg(_T("Garmin GPS could not be initialized on port: "));
			msg += com_name;
			wxString err;
			err.Printf(_T(" Error Code is %d"), v_init);
			msg += err;

			msg += _T("\n LastGarminError is: ");
			msg += GetLastGarminError();

			wxLogMessage(msg);

			ret_val = ERR_GARMIN_INITIALIZE;
			goto ret_point;
		} else {
			wxString msg(_T("Sent waypoint(s) to Garmin GPS on port: "));
			msg += com_name;
			msg += _T("\n Unit identifies as: ");
			wxString GPS_Unit = Garmin_GPS_GetSaveString();
			msg += GPS_Unit;
			wxLogMessage(msg);
		}

		RoutePointList rplist;
		rplist.push_back(prp);

		int ret_val = Garmin_GPS_SendWaypoints(short_com, &rplist);
		if (ret_val != 1) {
			wxString msg(_T("Error Sending Waypoint(s) to Garmin GPS on port: "));
			msg += com_name;
			wxString err;
			err.Printf(_T(" Error Code is %d"), ret_val);
			msg += err;

			msg += _T("\n LastGarminError is: ");
			msg += GetLastGarminError();

			wxLogMessage(msg);

			ret_val = ERR_GARMIN_GENERAL;
			goto ret_point;
		} else
			ret_val = 0;

		goto ret_point;
	}
	else
#endif //USE_GARMINHOST

	{ // Standard NMEA mode

		// If the port was temporarily closed, reopen as I/O type
		// Otherwise, open another port using default properties
		wxString baud;

		if (old_stream) {
			baud = baud_rate_save;
		} else {
			baud = _T("4800");
		}

		DataStream* dstr = new DataStream(this, com_name, baud, DS_TYPE_INPUT_OUTPUT, 0);

		// Wait up to 1 seconds for Datastream secondary thread to come up
		int timeout = 0;
		while (!dstr->IsSecThreadActive() && (timeout < 10)) {
			wxMilliSleep(100);
			timeout++;
		}

		SENTENCE snt;
		NMEA0183 oNMEA0183;
		oNMEA0183.TalkerID = _T("EC");

		if (pProgress)
			pProgress->SetRange(100);

		if (cfg.GPS_Ident == _T("Generic")) {
			if (prp->latitude() < 0.0)
				oNMEA0183.Wpl.Position.Latitude.Set(-prp->latitude(), _T("S"));
			else
				oNMEA0183.Wpl.Position.Latitude.Set(prp->latitude(), _T("N"));

			if (prp->longitude() < 0.0)
				oNMEA0183.Wpl.Position.Longitude.Set(-prp->longitude(), _T("W"));
			else
				oNMEA0183.Wpl.Position.Longitude.Set(prp->longitude(), _T("E"));

			wxString name = prp->GetName();
			oNMEA0183.Wpl.To = name.Truncate(6);
			oNMEA0183.Wpl.Write(snt);
		} else if (cfg.GPS_Ident == _T("FurunoGP3X")) {
			oNMEA0183.TalkerID = _T("PFEC,");

			if (prp->latitude() < 0.0)
				oNMEA0183.GPwpl.Position.Latitude.Set(-prp->latitude(), _T("S"));
			else
				oNMEA0183.GPwpl.Position.Latitude.Set(prp->latitude(), _T("N"));

			if (prp->longitude() < 0.0)
				oNMEA0183.GPwpl.Position.Longitude.Set(-prp->longitude(), _T("W"));
			else
				oNMEA0183.GPwpl.Position.Longitude.Set(prp->longitude(), _T("E"));

			wxString name = prp->GetName();
			name += _T("000000");
			name.Truncate(6);

			oNMEA0183.GPwpl.To = name;
			oNMEA0183.GPwpl.Write(snt);
		}

		if (dstr->SendSentence(snt.Sentence))
			LogOutputMessage(snt.Sentence, dstr->GetPort(), false);

		wxString msg(_T("-->GPS Port:"));
		msg += com_name;
		msg += _T(" Sentence: ");
		msg += snt.Sentence;
		msg.Trim();
		wxLogMessage(msg);

		if (cfg.GPS_Ident == _T("FurunoGP3X")) {
			wxString term;
			term.Printf(_T("$PFEC,GPxfr,CTL,E%c%c"), 0x0d, 0x0a);

			if (dstr->SendSentence(term))
				LogOutputMessage(term, dstr->GetPort(), false);

			wxString msg(_T("-->GPS Port:"));
			msg += com_name;
			msg += _T(" Sentence: ");
			msg += term;
			msg.Trim();
			wxLogMessage(msg);
		}

		if (pProgress) {
			pProgress->SetValue(100);
			pProgress->Refresh();
			pProgress->Update();
		}

		wxMilliSleep(500);

		// All finished with the temp port
		dstr->Close();

		ret_val = 0;
	}

ret_point:

	if (old_stream)
		CreateAndRestoreSavedStreamProperties();

	return ret_val;
}

