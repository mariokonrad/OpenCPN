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
 ***************************************************************************

 ***************************************************************************
 *  Parts of this file were adapted from source code found in              *
 *  John F. Waers (jfwaers@csn.net) public domain program MacGPS45         *
 **************************************************************************/

#include "DataStream.h"

#include "dychart.h"
#include <OCPN_DataStreamEvent.h>
#include <OCP_DataStreamInput_Thread.h>
#include <GarminProtocolHandler.h>

#include <garmin/jeeps/garmin_wrapper.h>

#include <cstdlib>
#include <cmath>
#include <ctime>
#include <vector>

#include <wx/tokenzr.h>
#include <wx/datetime.h>
#include <wx/log.h>

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

const wxEventType wxEVT_OCPN_DATASTREAM = wxNewEventType();

#define N_DOG_TIMEOUT   5

//------------------------------------------------------------------------------
//    DataStream Implementation
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(DataStream, wxEvtHandler)
	EVT_SOCKET(DS_SOCKET_ID, DataStream::OnSocketEvent)
	EVT_SOCKET(DS_SERVERSOCKET_ID, DataStream::OnServerSocketEvent)
	EVT_SOCKET(DS_ACTIVESERVERSOCKET_ID, DataStream::OnActiveServerEvent)
	EVT_TIMER(TIMER_SOCKET, DataStream::OnTimerSocket)
	EVT_TIMER(TIMER_SOCKET + 1, DataStream::OnSocketReadWatchdogTimer)
END_EVENT_TABLE()

// constructor
DataStream::DataStream(
		wxEvtHandler *input_consumer,
		const wxString& Port,
		const wxString& BaudRate,
		dsPortType io_select,
		int priority,
		bool bGarmin,
		int WXUNUSED(EOS_type),
		int handshake_type,
		void *user_data )
	: m_net_protocol(ConnectionParams::GPSD)
	, m_connection_type(ConnectionParams::SERIAL)
{
	m_consumer = input_consumer;
	m_portstring = Port;
	m_BaudRate = BaudRate;
	m_io_select = io_select;
	m_priority = priority;
	m_handshake = handshake_type;
	m_user_data = user_data;
	m_bGarmin_GRMN_mode = bGarmin;

	Init();

	Open();
}

void DataStream::Init(void)
{
	m_pSecondary_Thread = NULL;
	m_GarminHandler = NULL;
	m_bok = false;
	SetSecThreadInActive();
	m_Thread_run_flag = -1;
	m_sock = 0;
	m_tsock = 0;
	m_socket_server_active = 0;
	m_socket_server = 0;
	m_txenter = 0;

	m_socket_timer.SetOwner(this, TIMER_SOCKET);
	m_socketread_watchdog_timer.SetOwner(this, TIMER_SOCKET + 1);

}

void DataStream::Open(void)
{
	//  Open a port
	wxLogMessage( wxString::Format(_T("Opening NMEA Datastream %s"), m_portstring.c_str()) );

	if( (m_io_select == DS_TYPE_INPUT) || (m_io_select == DS_TYPE_INPUT_OUTPUT) ) {

		//    Data Source is specified serial port
		if(m_portstring.Contains(_T("Serial"))) {
			m_connection_type = ConnectionParams::SERIAL;
			wxString comx;
			comx =  m_portstring.AfterFirst(':');      // strip "Serial:"

			wxString port_uc = m_portstring.Upper();

			if( (wxNOT_FOUND != port_uc.Find(_T("USB"))) && (wxNOT_FOUND != port_uc.Find(_T("GARMIN"))) ) {
				m_GarminHandler = new GarminProtocolHandler(this, m_consumer,  true);
			}
			else if( m_bGarmin_GRMN_mode ) {
				m_GarminHandler = new GarminProtocolHandler(this, m_consumer,  false);
			}
			else {
				m_connection_type = ConnectionParams::SERIAL;
				wxString comx;
				comx =  m_portstring.AfterFirst(':');      // strip "Serial:"

#ifdef __WXMSW__
				wxString scomx = comx;
				scomx.Prepend(_T("\\\\.\\"));                  // Required for access to Serial Ports greater than COM9

				//  As a quick check, verify that the specified port is available
				HANDLE hSerialComm = CreateFile(scomx.fn_str(),       // Port Name
						GENERIC_READ,
						0,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);

				if(hSerialComm == INVALID_HANDLE_VALUE) {
					m_last_error = DS_ERROR_PORTNOTFOUND;
					wxLogMessage( _T("   Error, comm port open failure, INVALID_HANDLE_VALUE, Datastream skipped.") );
					return;
				}
				else
					CloseHandle(hSerialComm);
#endif
				//    Kick off the DataSource RX thread
				m_pSecondary_Thread = new OCP_DataStreamInput_Thread(this,
						m_consumer,
						comx, m_BaudRate,
						&m_output_mutex, m_io_select);
				m_Thread_run_flag = 1;
				m_pSecondary_Thread->Run();

				m_bok = true;
			}
		}
		else if(m_portstring.Contains(_T("GPSD"))){
			m_net_addr = _T("127.0.0.1");              // defaults
			m_net_port = _T("2947");
			m_net_protocol = ConnectionParams::GPSD;
			m_connection_type = ConnectionParams::NETWORK;
		}
		else if(m_portstring.StartsWith(_T("TCP"))) {
			m_net_addr = _T("127.0.0.1");              // defaults
			m_net_port = _T("2947");
			m_net_protocol = ConnectionParams::TCP;
			m_connection_type = ConnectionParams::NETWORK;
		}
		else if(m_portstring.StartsWith(_T("UDP"))) {
			m_net_addr =  _T("0.0.0.0");              // any address
			m_net_port = _T("0");                     // any port
			m_net_protocol = ConnectionParams::UDP;
			m_connection_type = ConnectionParams::NETWORK;
		}

		if(m_connection_type == ConnectionParams::NETWORK){

			//  Capture the  parameters from the portstring

			wxStringTokenizer tkz(m_portstring, _T(":"));
			wxString token = tkz.GetNextToken();                //GPSD, TCP or UDP

			token = tkz.GetNextToken();                         //ip
			if(!token.IsEmpty())
				m_net_addr = token;

			token = tkz.GetNextToken();                         //port
			if(!token.IsEmpty())
				m_net_port = token;


			m_addr.Hostname(m_net_addr);
			m_addr.Service(m_net_port);

			// Create the socket
			switch(m_net_protocol){
				case ConnectionParams::GPSD:
					{
						m_sock = new wxSocketClient();
						m_sock->SetEventHandler(*this, DS_SOCKET_ID);
						m_sock->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
						m_sock->Notify(TRUE);
						m_sock->SetTimeout(1);              // Short timeout

						wxSocketClient * tcp_socket = dynamic_cast<wxSocketClient*>(m_sock);
						tcp_socket->Connect(m_addr, FALSE);
						m_brx_connect_event = false;

						break;
					}

				case ConnectionParams::TCP:
					//  TCP Datastreams can be either input or output, but not both...
					if((m_io_select == DS_TYPE_INPUT_OUTPUT) || (m_io_select == DS_TYPE_OUTPUT)) {
						m_socket_server = new wxSocketServer(m_addr, wxSOCKET_REUSEADDR );
						m_socket_server->SetEventHandler(*this, DS_SERVERSOCKET_ID);
						m_socket_server->SetNotify( wxSOCKET_CONNECTION_FLAG );
						m_socket_server->Notify(TRUE);
						m_socket_server->SetTimeout(1);              // Short timeout
					}
					else {
						m_sock = new wxSocketClient();
						m_sock->SetEventHandler(*this, DS_SOCKET_ID);
						m_sock->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
						m_sock->Notify(TRUE);
						m_sock->SetTimeout(1);              // Short timeout

						m_brx_connect_event = false;
						m_socket_timer.Start(100, wxTIMER_ONE_SHOT);    // schedule a connection

					}
					break;

				case ConnectionParams::UDP:
					//  We need a local (bindable) address to create the Datagram receive socket
					// Set up the receive socket
					wxIPV4address conn_addr;
					conn_addr.Service(m_net_port);
					conn_addr.AnyAddress();
					m_sock = new wxDatagramSocket(conn_addr, wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR);

					// Set up another socket for transmit
					if((m_io_select == DS_TYPE_INPUT_OUTPUT) || (m_io_select == DS_TYPE_OUTPUT)) {
						wxIPV4address tconn_addr;
						tconn_addr.Service(0);          // use ephemeral out port
						tconn_addr.AnyAddress();
						m_tsock = new wxDatagramSocket(tconn_addr, wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR);
						wxString addr = m_addr.IPAddress();
						if( addr.EndsWith(_T("255")) ) {
							int broadcastEnable=1;
							m_tsock->SetOption(SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
						}
					}

					m_sock->SetEventHandler(*this, DS_SOCKET_ID);

					m_sock->SetNotify(wxSOCKET_CONNECTION_FLAG |
							wxSOCKET_INPUT_FLAG |
							wxSOCKET_LOST_FLAG);
					m_sock->Notify(TRUE);
					m_sock->SetTimeout(1); // Short timeout
					break;
			}

			m_bok = true;
		}
	}
	m_connect_time = wxDateTime::Now();
}


DataStream::~DataStream()
{
	Close();
}

void DataStream::Close()
{
	wxLogMessage( wxString::Format(_T("Closing NMEA Datastream %s"), m_portstring.c_str()) );

	//    Kill off the Secondary RX Thread if alive
	if(m_pSecondary_Thread)
	{
		if(m_bsec_thread_active)              // Try to be sure thread object is still alive
		{
			wxLogMessage(_T("Stopping Secondary Thread"));

			m_Thread_run_flag = 0;
			int tsec = 10;
			while((m_Thread_run_flag >= 0) && (tsec--))
				wxSleep(1);

			wxString msg;
			if(m_Thread_run_flag < 0)
				msg.Printf(_T("Stopped in %d sec."), 10 - tsec);
			else
				msg.Printf(_T("Not Stopped after 10 sec."));
			wxLogMessage(msg);
		}

		m_pSecondary_Thread = NULL;
		m_bsec_thread_active = false;
	}

	//    Kill off the TCP Socket if alive
	if(m_sock)
	{
		m_sock->Notify(FALSE);
		m_sock->Destroy();
	}

	if(m_tsock)
	{
		m_tsock->Notify(FALSE);
		m_tsock->Destroy();
	}

	if(m_socket_server)
	{
		m_socket_server->Notify(FALSE);
		m_socket_server->Destroy();
	}

	if(m_socket_server_active)
	{
		m_socket_server_active->Notify(FALSE);
		m_socket_server_active->Destroy();
	}

	//  Kill off the Garmin handler, if alive
	if(m_GarminHandler) {
		m_GarminHandler->Close();
		delete m_GarminHandler;
	}

	m_socket_timer.Stop();
	m_socketread_watchdog_timer.Stop();
}

void DataStream::OnSocketReadWatchdogTimer(wxTimerEvent &)
{
	m_dog_value--;
	if( m_dog_value <= 0 ) {            // No receive in n seconds, assume connection lost
		wxLogMessage( wxString::Format(_T("    TCP Datastream watchdog timeout: %s"), m_portstring.c_str()) );

		if(m_net_protocol == ConnectionParams::TCP) {
			wxSocketClient* tcp_socket = dynamic_cast<wxSocketClient*>(m_sock);
			if(tcp_socket)
				tcp_socket->Close();

			m_socket_timer.Start(5000, wxTIMER_ONE_SHOT);    // schedule a reconnect
			m_socketread_watchdog_timer.Stop();
		}
	}
}

void DataStream::OnTimerSocket(wxTimerEvent &)
{
	//  Attempt a connection
	wxSocketClient* tcp_socket = dynamic_cast<wxSocketClient*>(m_sock);

	if(tcp_socket) {
		if(tcp_socket->IsDisconnected() ) {
			m_brx_connect_event = false;
			tcp_socket->Connect(m_addr, FALSE);
			m_socket_timer.Start(5000, wxTIMER_ONE_SHOT);    // schedule another attempt
		}
	}
}


void DataStream::OnSocketEvent(wxSocketEvent& event)
{
	//#define RD_BUF_SIZE    200
#define RD_BUF_SIZE    4096 // Allows handling of high volume data streams, such as a National AIS stream with 100s of msgs a second.

	switch(event.GetSocketEvent())
	{
		case wxSOCKET_INPUT :                     // from gpsd Daemon
			{
				// TODO determine if the follwing SetFlags needs to be done at every socket event or only once when socket is created, it it needs to be done at all!
				//m_sock->SetFlags(wxSOCKET_WAITALL | wxSOCKET_BLOCK);      // was (wxSOCKET_NOWAIT);

				// We use wxSOCKET_BLOCK to avoid Yield() reentrancy problems
				// if a long ProgressDialog is active, as in S57 SENC creation.


				//    Disable input event notifications to preclude re-entrancy on non-blocking socket
				//           m_sock->SetNotify(wxSOCKET_LOST_FLAG);

				//          Read the reply, one character at a time, looking for 0x0a (lf)
				//          If the reply contains no lf, break on the buffer full

				std::vector<char> data(RD_BUF_SIZE+1);
				event.GetSocket()->Read(&data.front(),RD_BUF_SIZE);
				if(!event.GetSocket()->Error())
				{
					size_t count = event.GetSocket()->LastCount();
					if(count)
					{
						data[count]=0;
						m_sock_buffer.Append(wxString::FromAscii(&data.front()));
					}
				}

				bool done = false;

				while(!done){
					size_t nmea_end = m_sock_buffer.find('*'); // detect the potential end of a NMEA string by finding the checkum marker
					if(nmea_end != wxString::npos && nmea_end < m_sock_buffer.size()-2){
						nmea_end += 3; // move to the char after the 2 checksum digits
						wxString nmea_line = m_sock_buffer.substr(0,nmea_end);
						m_sock_buffer = m_sock_buffer.substr(nmea_end);

						size_t nmea_start = nmea_line.find_last_of(_T("$!")); // detect the potential start of a NMEA string, skipping preceding chars that may look like the start of a string.
						if(nmea_start != wxString::npos){
							nmea_line = nmea_line.substr(nmea_start);
							nmea_line += _T("\r\n");        // Add cr/lf, possibly superfluous
							if( m_consumer && ChecksumOK(nmea_line)){
								OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
								wxCharBuffer buffer=nmea_line.ToUTF8();
								if(buffer.data()) {
									Nevent.SetNMEAString( buffer.data() );
									Nevent.SetStream( this );

									m_consumer->AddPendingEvent(Nevent);
								}
							}
						}
					}
					else
						done = true;
				}

				// Prevent non-nmea junk from consuming to much memory by limiting carry-over buffer size.
				if(m_sock_buffer.size()>RD_BUF_SIZE)
					m_sock_buffer = m_sock_buffer.substr(m_sock_buffer.size()-RD_BUF_SIZE);

				m_dog_value = N_DOG_TIMEOUT;                // feed the dog
				break;
			}

		case wxSOCKET_LOST:
			{
				//          wxSocketError e = m_sock->LastError();          // this produces wxSOCKET_WOULDBLOCK.
				if(m_net_protocol == ConnectionParams::TCP) {
					wxLogMessage( wxString::Format(_T("TCP Datastream connection lost: %s"), m_portstring.c_str()) );
					wxDateTime now = wxDateTime::Now();
					wxTimeSpan since_connect = now - m_connect_time;

					int retry_time = 5000;          // default

					//  If the socket has never connected, and it is a short interval since the connect request
					//  then stretch the time a bit.  This happens on Windows if there is no dafault IP on any interface

					if(!m_brx_connect_event && (since_connect.GetSeconds() < 5) )
						retry_time = 10000;         // 10 secs

					m_socketread_watchdog_timer.Stop();
					m_socket_timer.Start(retry_time, wxTIMER_ONE_SHOT);     // Schedule a re-connect attempt

					break;
				}
			}

		case wxSOCKET_CONNECTION :
			{
				if(m_net_protocol == ConnectionParams::GPSD) {
					//      Sign up for watcher mode, Cooked NMEA
					//      Note that SIRF devices will be converted by gpsd into pseudo-NMEA
					char cmd[] = "?WATCH={\"class\":\"WATCH\", \"nmea\":true}";
					m_sock->Write(cmd, strlen(cmd));
				}
				else if(m_net_protocol == ConnectionParams::TCP) {
					wxLogMessage( wxString::Format(_T("TCP Datastream connection established: %s"), m_portstring.c_str()) );
					m_dog_value = N_DOG_TIMEOUT;                // feed the dog
					m_socketread_watchdog_timer.Start(1000);
					m_socket_timer.Stop();
					m_brx_connect_event = true;
				}


				m_connect_time = wxDateTime::Now();
				break;
			}

		default :
			break;
	}
}



void DataStream::OnServerSocketEvent(wxSocketEvent& event)
{

	switch(event.GetSocketEvent())
	{
		case wxSOCKET_CONNECTION :
			{
				m_socket_server_active = m_socket_server->Accept(false);

				if( m_socket_server_active ) {
					m_socket_server_active->SetEventHandler(*this, DS_ACTIVESERVERSOCKET_ID);
					m_socket_server_active->SetNotify( wxSOCKET_LOST_FLAG );
					m_socket_server_active->Notify(true);
				}

				break;
			}

		default :
			break;
	}
}

void DataStream::OnActiveServerEvent(wxSocketEvent& event)
{
	wxSocketBase *sock = event.GetSocket();

	switch(event.GetSocketEvent())
	{
		case wxSOCKET_LOST:
			{
				sock->Destroy();
				m_socket_server_active = 0;
				break;
			}


		default :
			break;
	}
}



bool DataStream::IsOk() const
{
	return m_bok;
}

wxString DataStream::GetPort() const
{
	return m_portstring;
}

dsPortType DataStream::GetIoSelect() const
{
	return m_io_select;
}

int DataStream::GetPriority() const
{
	return m_priority;
}

void * DataStream::GetUserData()
{
	return m_user_data;
}

int DataStream::GetLastError()
{
	return m_last_error;
}

void DataStream::SetSecThreadActive(void)
{
	m_bsec_thread_active = true;
}

void DataStream::SetSecThreadInActive(void)
{
	m_bsec_thread_active = false;
}

bool DataStream::IsSecThreadActive() const
{
	return m_bsec_thread_active;
}

void DataStream::SetChecksumCheck(bool check)
{
	m_bchecksumCheck = check;
}

void DataStream::SetInputFilter(wxArrayString filter)
{
	m_input_filter = filter;
}

void DataStream::SetInputFilterType(ConnectionParams::ListType filter_type)
{
	m_input_filter_type = filter_type;
}

void DataStream::SetOutputFilter(wxArrayString filter)
{
	m_output_filter = filter;
}

void DataStream::SetOutputFilterType(ConnectionParams::ListType filter_type)
{
	m_output_filter_type = filter_type;
}

bool DataStream::GetGarminMode() const
{
	return m_bGarmin_GRMN_mode;
}

wxString DataStream::GetBaudRate() const
{
	return m_BaudRate;
}

dsPortType DataStream::GetPortType() const
{
	return m_io_select;
}

wxArrayString DataStream::GetInputSentenceList()
{
	return m_input_filter;
}

wxArrayString DataStream::GetOutputSentenceList()
{
	return m_output_filter;
}

ConnectionParams::ListType DataStream::GetInputSentenceListType() const
{
	return m_input_filter_type;
}

ConnectionParams::ListType DataStream::GetOutputSentenceListType() const
{
	return m_output_filter_type;
}

bool DataStream::GetChecksumCheck() const
{
	return m_bchecksumCheck;
}

ConnectionParams::ConnectionType DataStream::GetConnectionType() const
{
	return m_connection_type;
}


bool DataStream::SentencePassesFilter(const wxString& sentence, ConnectionParams::FilterDirection direction)
{
	wxArrayString filter;
	bool listype = false;

	if (direction == ConnectionParams::FILTER_INPUT)
	{
		filter = m_input_filter;
		if (m_input_filter_type == ConnectionParams::WHITELIST)
			listype = true;
	}
	else
	{
		filter = m_output_filter;
		if (m_output_filter_type == ConnectionParams::WHITELIST)
			listype = true;
	}

	if (filter.Count() == 0) //Empty list means everything passes
		return true;

	wxString fs;
	for (size_t i = 0; i < filter.Count(); i++)
	{
		fs = filter.Item(i);
		switch (fs.Length())
		{
			case 2:
				if (fs == sentence.Mid(1, 2))
					return listype;
				break;
			case 3:
				if (fs == sentence.Mid(3, 3))
					return listype;
				break;
			case 5:
				if (fs == sentence.Mid(1, 5))
					return listype;
				break;
		}
	}
	return !listype;
}

bool DataStream::ChecksumOK( const wxString &sentence )
{
	if (!m_bchecksumCheck)
		return true;

	size_t check_start = sentence.find('*');
	if(check_start == wxString::npos || check_start > sentence.size() - 3)
		return false; // * not found, or it didn't have 2 characters following it.

	wxString check_str = sentence.substr(check_start+1,2);
	unsigned long checksum;
	if(!check_str.ToULong(&checksum,16))
		return false;

	unsigned char calculated_checksum = 0;
	for(wxString::const_iterator i = sentence.begin()+1; i != sentence.end() && *i != '*'; ++i)
		calculated_checksum ^= static_cast<unsigned char> (*i);

	return calculated_checksum == checksum;
}

bool DataStream::SendSentence( const wxString &sentence )
{
	wxString payload = sentence;
	if( !sentence.EndsWith(_T("\r\n")) )
		payload += _T("\r\n");

	switch( m_connection_type ) {
		case ConnectionParams::SERIAL:
			if( m_pSecondary_Thread ) {
				if( IsSecThreadActive() )
				{
					int retry = 10;
					while( retry ) {
						if(m_output_mutex.TryLock() == wxMUTEX_NO_ERROR) {
							if( m_pSecondary_Thread->SetOutMsg( payload )){
								m_output_mutex.Unlock();
								return true;
							}
							else {
								m_output_mutex.Unlock();        // output buffer is full
								return false;                   // no sense retrying
							}
						}
						else {          // could not get mutex, stall a bit
							retry--;
							wxMilliSleep(1);
						}
					}
					return false;   // could not get mutex after 10 msec....
				}
				else
					return false;
			}
			break;

		case ConnectionParams::NETWORK:
			if(m_txenter)
				return false;                 // do not allow recursion, could happen with non-blocking sockets
			m_txenter++;

			bool ret = true;
			wxDatagramSocket* udp_socket;
			switch(m_net_protocol){
				case ConnectionParams::TCP:
					if( m_socket_server_active && m_socket_server_active->IsOk() ) {
						m_socket_server_active->Write( payload.mb_str(), strlen( payload.mb_str() ) );
					}
					else
						ret = false;
					break;
				case ConnectionParams::UDP:
					udp_socket = dynamic_cast<wxDatagramSocket*>(m_tsock);
					if( udp_socket->IsOk() ) {
						udp_socket->SendTo(m_addr, payload.mb_str(), payload.size() );
						if( udp_socket->Error())
							ret = false;
					}
					else
						ret = false;
					break;

				case ConnectionParams::GPSD:
				default:
					ret = false;
					break;
			}
			m_txenter--;
			return ret;
			break;
	}

	return true;
}

