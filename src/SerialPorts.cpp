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

#include "SerialPorts.h"

#include <MessageBox.h>

#include <global/OCPN.h>
#include <global/System.h>

#include <wx/log.h>
#include <wx/utils.h>
#include <wx/intl.h>

#ifdef __WXGTK__
	extern "C" int wait(int *); // POSIX wait() for process
	#include <termios.h>
	#include <sys/ioctl.h>
	#include <linux/serial.h>
	#include <dirent.h>
	#include <sys/types.h>
	#include <fcntl.h>
	#include <unistd.h>
#endif

#ifdef __WXMSW__
	#include <windows.h>
	#include <setupapi.h>
	#include <guiddef.h>
	#include <initguid.h>

	// FIXME: thid guid is defined multiple times
	// {2C9C45C2-8E7D-4C08-A12D-816BBAE722C0}
	DEFINE_GUID(GARMIN_DETECT_GUID, 0x2c9c45c2L, 0x8e7d, 0x4c08, 0xa1, 0x2d, 0x81, 0x6b, 0xba, 0xe7, 0x22, 0xc0);
#endif

#ifdef __WXOSX__
	#include <macutils.h>
#endif

// reserve 4 pattern for plugins
char* devPatern[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, (char*)-1 };

// This function allow external plugin to search for a special device name
static int paternAdd(const char* patern)
{
	int ind = 0;

	// snan table for a free slot inside devpatern table
	for (ind = 0; devPatern[ind] != (char*)-1; ind++)
		if (devPatern[ind] == NULL)
			break;

	// table if full
	if (devPatern[ind] == (char*)-1)
		return -1;

	// store a copy of the patern in case calling routine had it on its stack
	devPatern[ind] = strdup(patern);
	return 0;
}

#ifdef __WXGTK__
// This filter verify is device is withing searched patern and verify it is openable
static int paternFilter(const struct dirent* dir)
{
	char* res = NULL;

	// search if devname fits with searched paterns
	for (int ind = 0; devPatern[ind] != (char*)-1; ind++) {
		if (devPatern[ind] != NULL)
			res = (char*)strcasestr(dir->d_name, devPatern[ind]);
		if (res != NULL)
			break;
	}

	// File does not fit researched patern
	if (res == NULL)
		return 0;

	// Check if we may open this file
	char devname[255];
	snprintf(devname, sizeof(devname), "/dev/%s", dir->d_name);
	int fd = open(devname, O_RDWR | O_NDELAY | O_NOCTTY);

	// device name is pointing to a real device
	if (fd > 0) {
		close(fd);
		return 1;
	}

	// file is not valid
	perror(devname);
	return 0;
}

static int isTTYreal(const char* dev)
{
	struct serial_struct serinfo;
	int ret = 0;

	int fd = open(dev, O_RDWR | O_NONBLOCK | O_NOCTTY);

	// device name is pointing to a real device
	if (fd > 0) {
		if (ioctl(fd, TIOCGSERIAL, &serinfo) == 0) {
			// If device type is no PORT_UNKNOWN we accept the port
			if (serinfo.type != PORT_UNKNOWN)
				ret = 1;
		}
		close(fd);
	}

	return ret;
}
#endif

wxArrayString* EnumerateSerialPorts(void)
{
	wxArrayString* preturn = new wxArrayString;

#ifdef __WXGTK__

	// Initialize the pattern table
	if (devPatern[0] == NULL) {
		paternAdd("ttyUSB");
		paternAdd("ttyACM");
		paternAdd("ttyGPS");
		paternAdd("refcom");
	}

	// Looking for user privilege openable devices in /dev
	// Fulup use scandir to improve user experience and support new generation of AIS devices.

	wxString sdev;
	int ind;
	int fcount;
	struct dirent** filelist = { 0 };

	// scan directory filter is applied automatically by this call
	fcount = scandir("/dev", &filelist, paternFilter, alphasort);

	for (ind = 0; ind < fcount; ind++) {
		wxString sdev(filelist[ind]->d_name, wxConvUTF8);
		sdev.Prepend(_T("/dev/"));

		preturn->Add(sdev);
		free(filelist[ind]);
	}

	// We try to add a few more, arbitrarily, for those systems that have fixed, traditional COM ports

	if (isTTYreal("/dev/ttyS0"))
		preturn->Add(_T("/dev/ttyS0"));

	if (isTTYreal("/dev/ttyS1"))
		preturn->Add(_T("/dev/ttyS1"));
#endif

#ifdef PROBE_PORTS__WITH_HELPER

	// For modern Linux/(Posix??) systems, we may use
	// the system files /proc/tty/driver/serial
	// and /proc/tty/driver/usbserial to identify
	// available serial ports.
	// A complicating factor is that most (all??) linux
	// systems require root privileges to access these files.
	// We will use a helper program method here, despite implied vulnerability.

	char buf[256]; // enough to hold one line from serial devices list
	char left_digit;
	char right_digit;
	int port_num;

	pid_t pID = vfork();

	if (pID == 0) { // child
		// Temporarily gain root privileges
		seteuid(geteuid());

		// Execute the helper program
		execlp("ocpnhelper", "ocpnhelper", "-SB", NULL);

		// Return to user privileges
		seteuid(getuid());

		wxLogMessage(_T("Warning: ocpnhelper failed...."));
		_exit(0); // If exec fails then exit forked process.
	}

	wait(NULL); // for the child to quit

	// Read and parse the files

	// see if we have any traditional ttySx ports available
	FILE* f = fopen("/var/tmp/serial", "r");

	if (f != NULL) {
		wxLogMessage(_T("Parsing copy of /proc/tty/driver/serial..."));

		// read in each line of the file
		while (fgets(buf, sizeof(buf), f) != NULL) {
			wxString sm(buf, wxConvUTF8);
			sm.Prepend(_T("   "));
			sm.Replace(_T("\n"), _T(" "));
			wxLogMessage(sm);

			// if the line doesn't start with a number get the next line
			if (buf[0] < '0' || buf[0] > '9')
				continue;

			// convert digits to an int
			left_digit = buf[0];
			right_digit = buf[1];
			if (right_digit < '0' || right_digit > '9')
				port_num = left_digit - '0';
			else
				port_num = (left_digit - '0') * 10 + right_digit - '0';

			// skip if "unknown" in the string
			if (strstr(buf, "unknown") != NULL)
				continue;

			// upper limit of 15
			if (port_num > 15)
				continue;

			// create string from port_num

			wxString s;
			s.Printf(_T("/dev/ttyS%d"), port_num);

			// add to the output array
			preturn->Add(wxString(s));
		}

		fclose(f);
	}

	// Same for USB ports
	f = fopen("/var/tmp/usbserial", "r");

	if (f != NULL) {
		wxLogMessage(_T("Parsing copy of /proc/tty/driver/usbserial..."));

		// read in each line of the file
		while (fgets(buf, sizeof(buf), f) != NULL) {

			wxString sm(buf, wxConvUTF8);
			sm.Prepend(_T("   "));
			sm.Replace(_T("\n"), _T(" "));
			wxLogMessage(sm);

			// if the line doesn't start with a number get the next line
			if (buf[0] < '0' || buf[0] > '9')
				continue;

			// convert digits to an int
			left_digit = buf[0];
			right_digit = buf[1];
			if (right_digit < '0' || right_digit > '9')
				port_num = left_digit - '0';
			else
				port_num = (left_digit - '0') * 10 + right_digit - '0';

			// skip if "unknown" in the string
			if (strstr(buf, "unknown") != NULL)
				continue;

			// upper limit of 15
			if (port_num > 15)
				continue;

			// create string from port_num

			wxString s;
			s.Printf(_T("/dev/ttyUSB%d"), port_num);

			// add to the output array
			preturn->Add(wxString(s));
		}

		fclose(f);
	}

	// As a fallback, in case seteuid doesn't work....
	// provide some defaults
	// This is currently the case for GTK+, which
	// refuses to run suid.  sigh...

	if (preturn->IsEmpty()) {
		preturn->Add(_T("/dev/ttyS0"));
		preturn->Add(_T("/dev/ttyS1"));
		preturn->Add(_T("/dev/ttyS2"));
		preturn->Add(_T("/dev/ttyS3"));
		preturn->Add(_T("/dev/ttyUSB0"));
		preturn->Add(_T("/dev/ttyUSB1"));
		preturn->Add(_T("/dev/ttyUSB2"));
		preturn->Add(_T("/dev/ttyUSB3"));
	}

	// Clean up the temporary files created by helper.
	pid_t cpID = vfork();

	if (cpID == 0) { // child
		// Temporarily gain root privileges
		seteuid(geteuid());

		// Execute the helper program
		execlp("ocpnhelper", "ocpnhelper", "-U", NULL);

		// Return to user privileges
		seteuid(getuid());
		_exit(0); // If exec fails then exit forked process.
	}

#endif

#ifdef __WXOSX__
	char* paPortNames[MAX_SERIAL_PORTS];
	int iPortNameCount;

	memset(paPortNames, 0x00, sizeof(paPortNames));
	iPortNameCount = FindSerialPortNames(&paPortNames[0], MAX_SERIAL_PORTS);
	for (int iPortIndex = 0; iPortIndex < iPortNameCount; iPortIndex++) {
		wxString sm(paPortNames[iPortIndex], wxConvUTF8);
		preturn->Add(sm);
		free(paPortNames[iPortIndex]);
	}
#endif

#ifdef __WXMSW__
	// *************************************************************************
	// Windows provides no system level enumeration of available serial ports
	// There are several ways of doing this.
	// *************************************************************************

	// Method 1:  Use GetDefaultCommConfig()
	// Try first a number of possible COM ports, check for a default configuration
	// This method will not find some Bluetooth SPP ports
	const int N_COM_PORTS = global::OCPN::get().sys().config().COMPortCheck;
	for (int i = 1; i < N_COM_PORTS; i++) {
		wxString s = wxString::Format(_T("COM%d"), i);

		COMMCONFIG cc;
		DWORD dwSize = sizeof(COMMCONFIG);
		if (GetDefaultCommConfig(s.fn_str(), &cc, &dwSize))
			preturn->Add(s);
	}

	// Method 3:  WDM-Setupapi
	// This method may not find XPort virtual ports,
	// but does find Bluetooth SPP ports

	GUID* guidDev = (GUID*)&GUID_CLASS_COMPORT;
	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	hDevInfo = SetupDiGetClassDevs(guidDev, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (hDevInfo != INVALID_HANDLE_VALUE) {
		BOOL bOk = TRUE;
		SP_DEVICE_INTERFACE_DATA ifcData;

		ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		for (DWORD ii = 0; bOk; ii++) {
			bOk = SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guidDev, ii, &ifcData);
			if (bOk) {
				// Got a device. Get the details.

				SP_DEVINFO_DATA devdata = { sizeof(SP_DEVINFO_DATA) };
				bOk = SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, NULL, 0, NULL, &devdata);

				// We really only need devdata
				if (!bOk) {
					if (GetLastError() == 122) // ERROR_INSUFFICIENT_BUFFER, OK in this case
						bOk = true;
				}

				// We could get friendly name and/or description here
				TCHAR desc[256] = { 0 };
				if (bOk) {
					TCHAR fname[256] = { 0 };
					BOOL bSuccess
						= SetupDiGetDeviceRegistryProperty(hDevInfo, &devdata, SPDRP_FRIENDLYNAME,
														   NULL, (PBYTE)fname, sizeof(fname), NULL);

					bSuccess = bSuccess && SetupDiGetDeviceRegistryProperty(
											   hDevInfo, &devdata, SPDRP_DEVICEDESC, NULL,
											   (PBYTE)desc, sizeof(desc), NULL);
				}

				// Get the "COMn string from the registry key
				if (bOk) {
					bool bFoundCom = false;
					TCHAR dname[256];
					HKEY hDeviceRegistryKey = SetupDiOpenDevRegKey(
						hDevInfo, &devdata, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
					if (INVALID_HANDLE_VALUE != hDeviceRegistryKey) {
						DWORD RegKeyType;
						wchar_t wport[80];
						LPCWSTR cstr = wport;
						MultiByteToWideChar(0, 0, "PortName", -1, wport, 80);
						DWORD len = sizeof(dname);

						int result = RegQueryValueEx(hDeviceRegistryKey, cstr, 0, &RegKeyType,
													 (PBYTE)dname, &len);
						if (result == 0)
							bFoundCom = true;
					}

					if (bFoundCom) {
						wxString port(dname, wxConvUTF8);

						// If the port has already been found, remove the prior entry
						// in favor of this entry, which will have descriptive information appended
						for (unsigned int n = 0; n < preturn->size(); n++) {
							if ((preturn->Item(n)).IsSameAs(port)) {
								preturn->RemoveAt(n);
								break;
							}
						}
						wxString desc_name(desc, wxConvUTF8); // append "description"
						port += _T(" ");
						port += desc_name;

						preturn->Add(port);
					}
				}
			}
		}
	}

	// Search for Garmin device driver on Windows platforms
	HDEVINFO hdeviceinfo = INVALID_HANDLE_VALUE;
	hdeviceinfo = SetupDiGetClassDevs((GUID*)&GARMIN_DETECT_GUID, NULL, NULL,
									  DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

	if (hdeviceinfo != INVALID_HANDLE_VALUE) {
		wxLogMessage(_T("EnumerateSerialPorts() Found Garmin USB Driver."));
		preturn->Add(_T("Garmin-USB")); // Add generic Garmin selectable device
	}

#endif
	return preturn;
}

bool CheckSerialAccess(void)
{
	bool bret = true;

#ifdef __WXGTK__

	// Who owns /dev/ttyS0?

	wxArrayString result1;
	wxExecute(_T("stat -c %G /dev/ttyS0"), result1);

	wxString group = result1[0];

	//  Is the current user in this group?
	wxString user = wxGetUserId();
	wxArrayString result2;
	wxExecute(_T("groups ") + user, result2);

	wxString user_groups = result2[0];

	if (user_groups.Find(group) == wxNOT_FOUND)
		bret = false;

	if (!bret) {
		wxString msg = _("OpenCPN requires access to serial ports to use serial NMEA data.\n\
You currently do not have permission to access the serial ports on this system.\n\n\
It is suggested that you exit OpenCPN now,\n\
and add yourself to the correct group to enable serial port access.\n\n\
You may do so by executing the following command from the linux command line:\n\n\
                sudo usermod -a -G ");

		msg += group;
		msg += _T(" ");
		msg += user;
		msg += _T("\n");

		OCPNMessageBox(NULL, msg, wxString(_("OpenCPN Info")), wxICON_INFORMATION | wxOK, 30);
	}

#endif

	return bret;
}

