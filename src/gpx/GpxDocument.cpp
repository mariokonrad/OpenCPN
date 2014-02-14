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

#include "GpxDocument.h"

#include <gpx/GpxRootElement.h>

#include <wx/regex.h>
#include <wx/ffile.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/utils.h>

namespace gpx {

GpxDocument::GpxDocument()
{
	PopulateEmptyDocument(_T("OpenCPN"));
	AddCustomNamespace(_T("xmlns:opencpn"), _T("http://www.opencpn.org"));
	SeedRandom();
}

GpxDocument::~GpxDocument()
{
}

bool GpxDocument::LoadFile(const wxString& filename)
{
	SeedRandom();

	// We try to fix popularily broken GPX files. Unencoded '&' is an illegal character
	// in XML, but seems higly popular amongst users (and perhaps even vendors not aware
	// of what XML is...)
	wxRegEx re;

	// The same as above is true for '<' but it would be harder to solve - it's illegal just inside
	// a value, not when it starts a tag
	int re_compile_flags = wxRE_ICASE;
#ifdef wxHAS_REGEX_ADVANCED
	re_compile_flags |= wxRE_ADVANCED;
#endif
	// Should find all the non-XML entites to be encoded as text
	bool b = re.Compile(wxT("&(?!amp;|lt;|gt;|apos;|quot;|#[0-9]{1,};|#x[0-f]{1,};)"),
						re_compile_flags);

	wxFFile file(filename);
	wxString s;
	if (file.IsOpened()) {
		file.ReadAll(&s, wxConvUTF8);

		// Fallback for not-well formed (non-UTF8) GPX files
		// the "garbage" characters are lost, but the important part of the information should
		// survive...
		if (s == wxEmptyString) {
			file.Seek(0);
			file.ReadAll(&s, wxConvISO8859_1);
			wxLogMessage(wxString::Format(
				wxT("File %s seems not to be well-formed UTF-8 XML, used fallback ASCII format conversion - some text information might have not been imported."),
				filename.c_str()));
		}

		file.Close();
	}
	if (b) {
		// CDATA handling makes this task way too complex for regular expressions to handle,
		// so we do nothing and just let the possible damage happen...
		if (!s.Contains(wxT("![CDATA["))) {
			int cnt = re.ReplaceAll(&s, wxT("&amp;"));
			if (cnt > 0)
				wxLogMessage(wxString::Format(wxT("File %s seems broken, %i occurences of '&' were replaced with '&amp;' to try to fix it."),
											  filename.c_str(), cnt));
		}
	}

	wxFFile gpxfile;
	wxString gpxfilename = wxFileName::CreateTempFileName(wxT("gpx"), &gpxfile);
	gpxfile.Write(s);
	gpxfile.Close();
	bool res = TiXmlDocument::LoadFile((const char*)gpxfilename.mb_str());

	if (!res) {
		wxLogMessage(_T("Failed to load ") + filename + _T(": ")
					 + wxString(TiXmlDocument::ErrorDesc(), wxConvUTF8));
	}
	::wxRemoveFile(gpxfilename);

	return res;
}

bool GpxDocument::SaveFile(const wxString& filename)
{
	return TiXmlDocument::SaveFile((const char*)filename.mb_str());
}

void GpxDocument::SeedRandom()
{
	// Fill with random. Miliseconds hopefully good enough for our usage, reading /dev/random would
	// be much better on linux and system guid function on Windows as well
	wxDateTime x = wxDateTime::UNow();
	long seed = x.GetMillisecond();
	seed *= x.GetTicks();
	srand(seed);
}

// RFC4122 version 4 compliant random UUIDs generator.
wxString GpxDocument::GetUUID(void)
{
	wxString str;
	struct UUID
	{
		int time_low;
		int time_mid;
		int time_hi_and_version;
		int clock_seq_hi_and_rsv;
		int clock_seq_low;
		int node_hi;
		int node_low;
	};

	UUID uuid;

	// FIXME: the max should be set to something like MAXINT32, but it doesn't compile un gcc...
	uuid.time_low = GetRandomNumber(0, 2147483647);
	uuid.time_mid = GetRandomNumber(0, 65535);
	uuid.time_hi_and_version = GetRandomNumber(0, 65535);
	uuid.clock_seq_hi_and_rsv = GetRandomNumber(0, 255);
	uuid.clock_seq_low = GetRandomNumber(0, 255);
	uuid.node_hi = GetRandomNumber(0, 65535);
	uuid.node_low = GetRandomNumber(0, 2147483647);

	// Set the two most significant bits (bits 6 and 7) of the
	// clock_seq_hi_and_rsv to zero and one, respectively.
	uuid.clock_seq_hi_and_rsv = (uuid.clock_seq_hi_and_rsv & 0x3F) | 0x80;

	// Set the four most significant bits (bits 12 through 15) of the
	// time_hi_and_version field to 4
	uuid.time_hi_and_version = (uuid.time_hi_and_version & 0x0fff) | 0x4000;

	str.Printf(_T("%08x-%04x-%04x-%02x%02x-%04x%08x"), uuid.time_low, uuid.time_mid,
			   uuid.time_hi_and_version, uuid.clock_seq_hi_and_rsv, uuid.clock_seq_low,
			   uuid.node_hi, uuid.node_low);

	return str;
}

int GpxDocument::GetRandomNumber(int range_min, int range_max)
{
	int d = range_max - range_min;

	long u = (long)wxRound(((double)rand() / ((double)(RAND_MAX) + 1) * d) + range_min);
	return (int)u;
}

void GpxDocument::AddCustomNamespace(const wxString& name, const wxString& url)
{
	RootElement()->SetAttribute(name.ToUTF8(), url.ToUTF8());
}

void GpxDocument::PopulateEmptyDocument(const wxString& creator)
{
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "");
	GpxRootElement* gpx_root = new GpxRootElement(creator);

	LinkEndChild(decl);
	LinkEndChild(gpx_root);
}

}

