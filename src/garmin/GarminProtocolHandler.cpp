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

#include "GarminProtocolHandler.h"

#include <wx/log.h>

#ifdef __WXMSW__
	#include <windows.h>
	#include <setupapi.h>
	#include <dbt.h>
	#include <windows.h>
	#include <initguid.h>
#endif

#include "DataStream.h"
#include <OCPN_DataStreamEvent.h>
#include <garmin/GARMIN_Serial_Thread.h>
#include <garmin/GARMIN_USB_Thread.h>
#include <garmin/jeeps/garmin_wrapper.h>

#define TIMER_GARMIN1 7005

namespace garmin {

//----------------------------------------------------------------------------
// Garmin Device Management
// Handle USB and Serial Port Garmin PVT protocol data interface.
//----------------------------------------------------------------------------

#ifdef __WXMSW__
// Routine Description: This routine returns TRUE if the caller's
// process is a member of the Administrators local group. Caller is NOT
// expected to be impersonating anyone and is expected to be able to
// open its own process and process token.
// Arguments: None.
// Return Value:
// TRUE - Caller has Administrators local group.
// FALSE - Caller does not have Administrators local group. --
BOOL IsUserAdmin(VOID)
{
	BOOL b;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	b = AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
								 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);
	if (b) {
		if (!CheckTokenMembership(NULL, AdministratorsGroup, &b)) {
			b = FALSE;
		}
		FreeSid(AdministratorsGroup);
	}

	return (b);
}

void le_write16(void* addr, const unsigned value)
{
	unsigned char* p = (unsigned char*)addr;
	p[0] = value;
	p[1] = value >> 8;
}

void le_write32(void* addr, const unsigned value)
{
	unsigned char* p = (unsigned char*)addr;
	p[0] = value;
	p[1] = value >> 8;
	p[2] = value >> 16;
	p[3] = value >> 24;
}

signed int le_read16(const void* addr)
{
	const unsigned char* p = (const unsigned char*)addr;
	return p[0] | (p[1] << 8);
}

signed int le_read32(const void* addr)
{
	const unsigned char* p = (const unsigned char*)addr;
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

#endif


BEGIN_EVENT_TABLE(GarminProtocolHandler, wxEvtHandler)
	EVT_TIMER(TIMER_GARMIN1, GarminProtocolHandler::OnTimerGarmin1)
END_EVENT_TABLE()

GarminProtocolHandler::GarminProtocolHandler(DataStream* parent, wxEvtHandler* MessageTarget,
											 bool bsel_usb)
{
	m_pparent = parent;
	m_pMainEventHandler = MessageTarget;
	m_garmin_serial_thread = NULL;
	m_garmin_usb_thread = NULL;
	m_bOK = false;
	m_busb = bsel_usb;

	char pvt_on[14] = { 20, 0, 0, 0, 10, 0, 0, 0, 2, 0, 0, 0, 49, 0 };

	char pvt_off[14] = { 20, 0, 0, 0, 10, 0, 0, 0, 2, 0, 0, 0, 50, 0 };

#ifdef __WXMSW__
	if (m_busb) {
		m_usb_handle = INVALID_HANDLE_VALUE;

		m_bneed_int_reset = true;
		m_receive_state = rs_fromintr;
		m_ndelay = 0;

		wxLogMessage(_T("Searching for Garmin DeviceInterface and Device..."));

		if (!FindGarminDeviceInterface()) {
			wxLogMessage(_T("   Find:Is the Garmin USB driver installed?"));
		} else {
			if (!ResetGarminUSBDriver())
				wxLogMessage(_T("   Reset:Is the Garmin USB Device plugged in?"));
		}
	}
#endif

	// Not using USB, so try a Garmin port open and device ident
	if (!m_busb) {
		m_port = m_pparent->GetPort().AfterFirst(':'); // strip "Serial:"

		// Start handler thread
		m_garmin_serial_thread
			= new GARMIN_Serial_Thread(this, m_pparent, m_pMainEventHandler, m_port);
		m_Thread_run_flag = 1;
		m_garmin_serial_thread->Run();
	}

	TimerGarmin1.SetOwner(this, TIMER_GARMIN1);
	TimerGarmin1.Start(100);
}

GarminProtocolHandler::~GarminProtocolHandler()
{
}

void GarminProtocolHandler::Close(void)
{
	TimerGarmin1.Stop();

	StopIOThread(true);
	StopSerialThread();
}

void GarminProtocolHandler::StopSerialThread(void)
{
	if (m_garmin_serial_thread) {
		wxLogMessage(_T("Stopping Garmin Serial thread"));
		m_Thread_run_flag = 0;

		int tsec = 5;
		while ((m_Thread_run_flag >= 0) && (tsec--)) {
			wxSleep(1);
		}

		wxString msg;
		if (m_Thread_run_flag < 0)
			msg.Printf(_T("Stopped in %d sec."), 5 - tsec);
		else
			msg.Printf(_T("Not Stopped after 5 sec."));
		wxLogMessage(msg);
	}
	m_garmin_serial_thread = NULL;
}

void GarminProtocolHandler::StopIOThread(bool b_pause)
{
	if (b_pause)
		TimerGarmin1.Stop();

	if (m_garmin_usb_thread) {
		wxLogMessage(_T("Stopping Garmin USB thread"));
		m_Thread_run_flag = 0;

		int tsec = 5;
		while ((m_Thread_run_flag >= 0) && (tsec--)) {
			wxSleep(1);
		}

		wxString msg;
		if (m_Thread_run_flag < 0)
			msg.Printf(_T("Stopped in %d sec."), 5 - tsec);
		else
			msg.Printf(_T("Not Stopped after 5 sec."));
		wxLogMessage(msg);
	}

	m_garmin_usb_thread = NULL;

#ifdef __WXMSW__
	if (m_busb && (m_usb_handle != INVALID_HANDLE_VALUE))
		CloseHandle(m_usb_handle);
	m_usb_handle = INVALID_HANDLE_VALUE;
#endif

	m_ndelay = 30; // Fix delay for next restart
}

void GarminProtocolHandler::RestartIOThread(void)
{
	wxLogMessage(_T("Restarting Garmin I/O thread"));
	TimerGarmin1.Start(1000);
}

void GarminProtocolHandler::OnTimerGarmin1(wxTimerEvent&)
{
	char pvt_on[14] = { 20, 0, 0, 0, 10, 0, 0, 0, 2, 0, 0, 0, 49, 0 };

	TimerGarmin1.Stop();

	if (m_busb) {
#ifdef __WXMSW__
		// Try to open the Garmin USB device
		if (INVALID_HANDLE_VALUE == m_usb_handle) {
			if (INVALID_HANDLE_VALUE != garmin_usb_start()) {
				// Send out a request for Garmin PVT data
				m_receive_state = rs_fromintr;
				gusb_cmd_send((const garmin_usb_packet*)pvt_on, sizeof(pvt_on));

				// Start the pump
				m_garmin_usb_thread = new GARMIN_USB_Thread(this, m_pparent, m_pMainEventHandler,
															(int)m_usb_handle, m_max_tx_size);
				m_Thread_run_flag = 1;
				m_garmin_usb_thread->Run();
			}
		}
#endif
	}

	TimerGarmin1.Start(1000);
}

#ifdef __WXMSW__
bool GarminProtocolHandler::ResetGarminUSBDriver()
{
	OSVERSIONINFO version_info;
	version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (GetVersionEx(&version_info)) {
		if (version_info.dwMajorVersion > 5) {
			if (!IsUserAdmin()) {
				wxLogMessage(_T("    GarminUSBDriver Reset skipped, requires elevated privileges ")
							 _T("on Vista and later...."));
				return true;
			}
		}
	}

	HDEVINFO devs;
	SP_DEVINFO_DATA devInfo;
	SP_PROPCHANGE_PARAMS pchange;

	devs = SetupDiGetClassDevs((GUID*)&GARMIN_GUID, NULL, NULL,
							   DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	if (devs == INVALID_HANDLE_VALUE)
		return false;

	devInfo.cbSize = sizeof(devInfo);
	if (!SetupDiEnumDeviceInfo(devs, 0, &devInfo)) {
		wxLogMessage(_T("   GarminUSBDriver Reset0 failed..."));
		return false;
	}

	pchange.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
	pchange.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
	pchange.StateChange = DICS_PROPCHANGE;
	pchange.Scope = DICS_FLAG_CONFIGSPECIFIC;
	pchange.HwProfile = 0;

	if (!SetupDiSetClassInstallParams(devs, &devInfo, &pchange.ClassInstallHeader,
									  sizeof(pchange))) {
		wxLogMessage(_T("   GarminUSBDriver Reset1 failed..."));
		return false;
	}

	if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, devs, &devInfo)) {
		wxLogMessage(_T("   GarminUSBDriver Reset2 failed..."));
		return false;
	}

	wxLogMessage(_T("GarminUSBDriver Reset succeeded."));

	return true;
}

bool GarminProtocolHandler::FindGarminDeviceInterface()
{
	// Search for a useable Garmin Device Interface Class

	HDEVINFO hdevinfo;
	SP_DEVINFO_DATA devInfo;

	hdevinfo = SetupDiGetClassDevs((GUID*)&GARMIN_GUID, NULL, NULL,
								   DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

	if (hdevinfo != INVALID_HANDLE_VALUE) {
		devInfo.cbSize = sizeof(devInfo);
		if (!SetupDiEnumDeviceInfo(hdevinfo, 0, &devInfo)) {
			return false;
		}
	}

	return true;
}

bool GarminProtocolHandler::IsGarminPlugged()
{
	DWORD size = 0;

	HDEVINFO hdevinfo;
	SP_DEVICE_INTERFACE_DATA infodata;

	// Search for the Garmin Device Interface Class
	hdevinfo = SetupDiGetClassDevs((GUID*)&GARMIN_GUID, NULL, NULL,
								   DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

	if (hdevinfo == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;

	infodata.cbSize = sizeof(infodata);

	bool bgarmin_unit_found
		= (SetupDiEnumDeviceInterfaces(hdevinfo, NULL, (GUID*)&GARMIN_GUID, 0, &infodata) != 0);

	if (!bgarmin_unit_found)
		return false;

	PSP_INTERFACE_DEVICE_DETAIL_DATA pdd = NULL;
	SP_DEVINFO_DATA devinfo;

	SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata, NULL, 0, &size, NULL);

	pdd = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(size);
	pdd->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

	devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
	if (!SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata, pdd, size, NULL, &devinfo)) {
		free(pdd);
		return false;
	}

	free(pdd);

	return true;
}

HANDLE GarminProtocolHandler::garmin_usb_start()
{
	DWORD size = 0;

	HDEVINFO hdevinfo;
	SP_DEVICE_INTERFACE_DATA infodata;

	// Search for the Garmin Device Interface Class
	hdevinfo = SetupDiGetClassDevs((GUID*)&GARMIN_GUID, NULL, NULL,
								   DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

	if (hdevinfo == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;

	infodata.cbSize = sizeof(infodata);

	bool bgarmin_unit_found
		= (SetupDiEnumDeviceInterfaces(hdevinfo, NULL, (GUID*)&GARMIN_GUID, 0, &infodata) != 0);

	if (!bgarmin_unit_found)
		return INVALID_HANDLE_VALUE;

	wxLogMessage(_T("Garmin USB Device Found"));

	if ((m_usb_handle == INVALID_HANDLE_VALUE) || (m_usb_handle == 0)) {
		PSP_INTERFACE_DEVICE_DETAIL_DATA pdd = NULL;
		SP_DEVINFO_DATA devinfo;

		SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata, NULL, 0, &size, NULL);

		pdd = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(size);
		pdd->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

		devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
		if (!SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata, pdd, size, NULL, &devinfo)) {
			wxLogMessage(_T("   SetupDiGetDeviceInterfaceDetail failed for Garmin Device..."));
			free(pdd);
			return INVALID_HANDLE_VALUE;
		}

		if (m_bneed_int_reset) {
			ResetGarminUSBDriver();
			m_bneed_int_reset = false;
		}

		m_usb_handle = CreateFile(pdd->DevicePath, GENERIC_READ | GENERIC_WRITE, 0, NULL,
								  OPEN_EXISTING, 0, NULL);

		if (m_usb_handle == INVALID_HANDLE_VALUE) {
			wxString msg;
			msg.Printf(_T("   (usb) CreateFile on '%s' failed"), pdd->DevicePath);
			wxLogMessage(msg);
		}

		free(pdd);
	}

	m_max_tx_size = 0;

	if (!DeviceIoControl(m_usb_handle, IOCTL_GARMIN_USB_BULK_OUT_PACKET_SIZE, NULL, 0,
						 &m_max_tx_size, GARMIN_USB_INTERRUPT_DATA_SIZE, &size, NULL)) {
		wxLogMessage(_T("   Couldn't get Garmin USB packet size."));
		CloseHandle(m_usb_handle);
		m_usb_handle = INVALID_HANDLE_VALUE;
		return INVALID_HANDLE_VALUE;
	}

	if (!gusb_syncup()) {
		CloseHandle(m_usb_handle);
		m_usb_handle = INVALID_HANDLE_VALUE;
	}

	return m_usb_handle;
}

bool GarminProtocolHandler::gusb_syncup(void)
{
	static int unit_number;
	static const char oinit[12] = { 0, 0, 0, 0, GUSB_SESSION_START, 0, 0, 0, 0, 0, 0, 0 };
	garmin_usb_packet iresp;
	int i;

	// This is our first communication with the unit.

	m_receive_state = rs_fromintr;

	for (i = 0; i < 25; i++) {
		le_write16(&iresp.gusb_pkt.pkt_id[0], 0);
		le_write32(&iresp.gusb_pkt.datasz[0], 0);
		le_write32(&iresp.gusb_pkt.databuf[0], 0);

		if (gusb_cmd_send((const garmin_usb_packet*)oinit, sizeof(oinit))) {
			gusb_cmd_get(&iresp, sizeof(iresp));

			if ((le_read16(iresp.gusb_pkt.pkt_id) == GUSB_SESSION_ACK)
				&& (le_read32(iresp.gusb_pkt.datasz) == 4)) {
				unit_number++;

				wxLogMessage(_T("Successful Garmin USB syncup."));
				return true;
			}
		}
	}
	wxLogMessage(_T("   Unable to establish Garmin USB syncup."));
	return false;
}

int GarminProtocolHandler::gusb_cmd_send(const garmin_usb_packet* opkt, size_t sz)
{
	unsigned int rv;
	unsigned char* obuf = (unsigned char*)&opkt->dbuf[0];

	rv = gusb_win_send(opkt, sz);

	// Recursion, when used in a disciplined way, can be our friend.
	//
	// The Garmin protocol requires that packets that are exactly
	// a multiple of the max tx size be followed by a zero length
	// packet.  Do that here so we can see it in debugging traces.

	if (sz && !(sz % m_max_tx_size)) {
		wxLogMessage(_T("win_send_call1"));
		gusb_win_send(opkt, 0);
		wxLogMessage(_T("win_send_ret1"));
	}

	return rv;
}

int GarminProtocolHandler::gusb_cmd_get(garmin_usb_packet* ibuf, size_t sz)
{
	int rv;
	unsigned char* buf = (unsigned char*)&ibuf->dbuf[0];
	int orig_receive_state;
top: // TODO: no, just no
	orig_receive_state = m_receive_state;
	switch (m_receive_state) {
		case rs_fromintr:
			rv = gusb_win_get(ibuf, sz);
			break;
		case rs_frombulk:
			rv = gusb_win_get_bulk(ibuf, sz);
			break;
	}

	// Adjust internal state and retry the read
	if ((rv > 0) && (ibuf->gusb_pkt.pkt_id[0] == GUSB_REQUEST_BULK)) {
		m_receive_state = rs_frombulk;
		goto top;
	}

	// If we were reading from the bulk pipe and we just got
	// a zero request, adjust our internal state.
	// It's tempting to retry the read here to hide this "stray"
	// packet from our callers, but that only works when you know
	// there's another packet coming.   That works in every case
	// except the A000 discovery sequence.
	if ((m_receive_state == rs_frombulk) && (rv <= 0)) {
		m_receive_state = rs_fromintr;
	}

	return rv;
}

int GarminProtocolHandler::gusb_win_get(garmin_usb_packet* ibuf, size_t sz)
{
	DWORD rxed = GARMIN_USB_INTERRUPT_DATA_SIZE;
	unsigned char* buf = (unsigned char*)&ibuf->dbuf[0];
	int tsz = 0;

	while (sz) {
		// The driver wrongly (IMO) rejects reads smaller than
		// GARMIN_USB_INTERRUPT_DATA_SIZE
		if (!DeviceIoControl(m_usb_handle, IOCTL_GARMIN_USB_INTERRUPT_IN, NULL, 0, buf,
							 GARMIN_USB_INTERRUPT_DATA_SIZE, &rxed, NULL)) {
		}

		buf += rxed;
		sz -= rxed;
		tsz += rxed;
		if (rxed < GARMIN_USB_INTERRUPT_DATA_SIZE)
			break;
	}
	return tsz;
}

int GarminProtocolHandler::gusb_win_get_bulk(garmin_usb_packet* ibuf, size_t sz)
{
	int n;
	DWORD rsz;
	unsigned char* buf = (unsigned char*)&ibuf->dbuf[0];

	n = ReadFile(m_usb_handle, buf, sz, &rsz, NULL);

	return rsz;
}

int GarminProtocolHandler::gusb_win_send(const garmin_usb_packet* opkt, size_t sz)
{
	DWORD rsz;
	unsigned char* obuf = (unsigned char*)&opkt->dbuf[0];

	// The spec warns us about making writes an exact multiple
	// of the packet size, but isn't clear whether we can issue
	// data in a single call to WriteFile if it spans buffers.
	WriteFile(m_usb_handle, obuf, sz, &rsz, NULL);
	int err = GetLastError();

	return rsz;
}
#endif

}
