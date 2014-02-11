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

#ifndef __DATASTREAM__H__
#define __DATASTREAM__H__

#include <wx/timer.h>
#include <wx/datetime.h>

#ifdef __WXGTK__
	// newer versions of glib define its own GSocket but we unfortunately use this
	// name in our own (semi-)public header and so can't change it -- rename glib
	// one instead
	//#include <gtk/gtk.h>
	#define GSocket GlibGSocket
	#include <wx/socket.h>
	#undef GSocket
#else
	#include <wx/socket.h>
#endif

#ifndef __WXMSW__
	#include <sys/socket.h>
	#include <netinet/in.h>
#endif

#ifdef __WXMSW__
	#include <windows.h>
	#include <dbt.h>
	#include <windows.h>
	#include <winioctl.h>
	#include <initguid.h>
#endif

#include <ConnectionParams.h>
#include <dsPortType.h>

class OCP_DataStreamInput_Thread;

namespace garmin { class GarminProtocolHandler; }

#define TIMER_SOCKET 7006

// Error codes, returned by GetLastError()
enum {
	DS_ERROR_PORTNOTFOUND
};

// End-of-sentence types
enum {
	DS_EOS_CRLF,
	DS_EOS_LF,
	DS_EOS_CR
};

// Serial port handshake type
enum {
	DS_HANDSHAKE_NONE,
	DS_HANDSHAKE_XON_XOFF
};

#define DS_SOCKET_ID             5001
#define DS_SERVERSOCKET_ID       5002
#define DS_ACTIVESERVERSOCKET_ID 5003

#define MAX_RX_MESSSAGE_SIZE  4096
#define RX_BUFFER_SIZE        4096


// A generic position Data structure
struct GenericPosDatEx
{
	double kLat;
	double kLon;
	double kCog;
	double kSog;
	double kVar; // Variation, typically from RMC message
	double kHdm; // Magnetic heading
	double kHdt; // true heading
	time_t FixTime;
	int nSats;
};

extern const wxEventType wxEVT_OCPN_DATASTREAM;
extern const wxEventType EVT_THREADMSG;

//----------------------------------------------------------------------------
// DataStream
//
//      Physical port is specified by a string in the class ctor.
//      Examples strings:
//              Serial:/dev/ttyS0               (Standard serial port)
//              Serial:COM4
//              TCP:192.168.1.1:5200            (TCP source, address and port specified)
//              GPSD:192.168.2.3:5400           (GPSD Wire protocol, address and port specified)
//
//----------------------------------------------------------------------------

class DataStream : public wxEvtHandler
{
	DECLARE_EVENT_TABLE()

public:
	DataStream(wxEvtHandler* input_consumer, const wxString& Port, const wxString& BaudRate,
			   dsPortType io_select, int priority = 0, bool bGarmin = false,
			   int EOS_type = DS_EOS_CRLF, int handshake_type = DS_HANDSHAKE_NONE,
			   void* user_data = NULL);

	~DataStream();

	void Close(void);

	bool IsOk() const;
	wxString GetPort() const;
	dsPortType GetIoSelect() const;
	int GetPriority() const;
	void* GetUserData();
	bool SendSentence(const wxString& sentence);
	int GetLastError();

	// Secondary thread life toggle
	// Used to inform launching object (this) to determine if the thread can
	// be safely called or polled, e.g. wxThread->Destroy();
	void SetSecThreadActive(void);
	void SetSecThreadInActive(void);
	bool IsSecThreadActive() const;
	void SetChecksumCheck(bool check);
	void SetInputFilter(wxArrayString filter);
	void SetInputFilterType(ConnectionParams::ListType filter_type);
	void SetOutputFilter(wxArrayString filter);
	void SetOutputFilterType(ConnectionParams::ListType filter_type);
	bool SentencePassesFilter(const wxString& sentence,
							  ConnectionParams::FilterDirection direction);
	bool ChecksumOK(const wxString& sentence);

	bool GetGarminMode() const;
	wxString GetBaudRate() const;
	dsPortType GetPortType() const;
	wxArrayString GetInputSentenceList();
	wxArrayString GetOutputSentenceList();
	ConnectionParams::ListType GetInputSentenceListType() const;
	ConnectionParams::ListType GetOutputSentenceListType() const;
	bool GetChecksumCheck() const;
	ConnectionParams::ConnectionType GetConnectionType() const;
	int m_Thread_run_flag;

private:
	void Init(void);
	void Open(void);

	void OnSocketEvent(wxSocketEvent& event);
	void OnTimerSocket(wxTimerEvent& event);
	void OnSocketReadWatchdogTimer(wxTimerEvent& event);

	wxMutex m_output_mutex;
	bool m_bok;
	wxEvtHandler* m_consumer;
	wxString m_portstring;
	wxString m_BaudRate;
	dsPortType m_io_select;
	int m_priority;
	int m_handshake;
	void* m_user_data;

	OCP_DataStreamInput_Thread* m_pSecondary_Thread;
	bool m_bsec_thread_active;
	int m_last_error;

	wxIPV4address m_addr;
	wxSocketBase* m_sock;
	wxSocketBase* m_tsock;
	bool m_is_multicast;
	struct ip_mreq m_mrq; // mreq rather than mreqn for windows

	// TCP Server support
	void OnServerSocketEvent(wxSocketEvent& event); // The listener
	void OnActiveServerEvent(wxSocketEvent& event); // The open connection
	wxSocketServer* m_socket_server; //  The listening server
	wxSocketBase* m_socket_server_active; //  The active connection

	wxString m_sock_buffer;
	wxString m_net_addr;
	wxString m_net_port;
	ConnectionParams::NetworkProtocol m_net_protocol;
	ConnectionParams::ConnectionType m_connection_type;

	bool m_bchecksumCheck;
	wxArrayString m_input_filter;
	ConnectionParams::ListType m_input_filter_type;
	wxArrayString m_output_filter;
	ConnectionParams::ListType m_output_filter_type;

	bool m_bGarmin_GRMN_mode;
	garmin::GarminProtocolHandler* m_GarminHandler;
	wxDateTime m_connect_time;
	bool m_brx_connect_event;
	wxTimer m_socket_timer;
	int m_txenter;
	wxTimer m_socketread_watchdog_timer;
	int m_dog_value;
};

#endif
