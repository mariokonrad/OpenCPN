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

#include "MemoryStatus.h"

#include <wx/string.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/utils.h>

#ifdef __WXMSW__
	#include <windows.h>
	#include <psapi.h>
#endif

// Memory monitor support
bool GetMemoryStatus(int & mem_total, int & mem_used)
{
#ifdef __LINUX__
	// Use filesystem /proc/pid/status to determine memory status

	unsigned long processID = wxGetProcessId();
	wxTextFile file;
	wxString file_name;

	mem_used = 0;
	file_name.Printf(_T("/proc/%d/status"), (int)processID);
	if(file.Open(file_name))
	{
		bool b_found = false;
		wxString str;
		for ( str = file.GetFirstLine(); !file.Eof(); str = file.GetNextLine() )
		{
			wxStringTokenizer tk(str, _T(" :"));
			while ( tk.HasMoreTokens() )
			{
				wxString token = tk.GetNextToken();
				if(token == _T("VmSize"))
				{
					wxStringTokenizer tkm(str, _T(" "));
					wxString mem = tkm.GetNextToken();
					long mem_extract = 0;
					while(mem.Len())
					{
						mem.ToLong(&mem_extract);
						if(mem_extract)
							break;
						mem = tkm.GetNextToken();
					}

					mem_used = mem_extract;
					b_found = true;
					break;
				}
				else
					break;
			}
			if(b_found)
				break;
		}
	}

	mem_total = 0;
	wxTextFile file_info;
	file_name = _T("/proc/meminfo");
	if(file_info.Open(file_name))
	{
		bool b_found = false;
		wxString str;
		for ( str = file_info.GetFirstLine(); !file_info.Eof(); str = file_info.GetNextLine() )
		{
			wxStringTokenizer tk(str, _T(" :"));
			while ( tk.HasMoreTokens() )
			{
				wxString token = tk.GetNextToken();
				if(token == _T("MemTotal"))
				{
					wxStringTokenizer tkm(str, _T(" "));
					wxString mem = tkm.GetNextToken();
					long mem_extract = 0;
					while(mem.Len())
					{
						mem.ToLong(&mem_extract);
						if(mem_extract)
							break;
						mem = tkm.GetNextToken();
					}

					mem_total = mem_extract;
					b_found = true;
					break;
				}
				else
					break;
			}
			if(b_found)
				break;
		}
	}

#endif

#ifdef __WXMSW__
	HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;

	unsigned long processID = wxGetProcessId();

	hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );

	if( hProcess && GetProcessMemoryInfo( hProcess, &pmc, sizeof( pmc ) ) ) {
		mem_used = pmc.WorkingSetSize / 1024;
	}

	CloseHandle( hProcess );

	MEMORYSTATUSEX statex;

	statex.dwLength = sizeof( statex );
	GlobalMemoryStatusEx( &statex );
	mem_total = statex.ullTotalPhys / 1024;
#endif

	return true;
}

