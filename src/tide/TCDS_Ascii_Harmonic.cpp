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

#include "TCDS_Ascii_Harmonic.h"
#include <tide/IDX_entry.h>

#include <windows/compatibility.h>

#include <cmath>

#include <wx/filename.h>
#include <wx/tokenzr.h>

namespace tide {

#define LINELEN 300

enum {
	IFF_OPEN  = 0,
	IFF_CLOSE = 1,
	IFF_SEEK  = 2,
	IFF_TELL  = 3,
	IFF_READ  = 4
};

typedef struct
{
	void* next;
	short int rec_start;
	char* name;
} harmonic_file_entry;

// Turn a time displacement of the form [-]HH:MM into the number of seconds.
static int hhmm2seconds(const char* hhmm)
{
	int h;
	int m;
	char s;
	if (sscanf(hhmm, "%d:%d", &h, &m) != 2)
		return (0);
	if (sscanf(hhmm, "%c", &s) != 1)
		return (0);
	if (h < 0 || s == '-')
		m = -m;
	return h * 3600 + m * 60;
}

TCDS_Ascii_Harmonic::TCDS_Ascii_Harmonic()
	: m_IndexFile(NULL)
	, num_nodes(0)
	, num_csts(0)
	, num_epochs(0)
{
}

TCDS_Ascii_Harmonic::~TCDS_Ascii_Harmonic()
{
	free_data();

	for (std::vector<IDX_entry*>::iterator i = m_IDX_array.begin(); i != m_IDX_array.end(); ++i) {
		delete *i;
	}
	m_IDX_array.clear();
}

TC_Error_Code TCDS_Ascii_Harmonic::LoadData(const wxString& data_file_path)
{
	if (m_IndexFile)
		IndexFileIO(IFF_CLOSE, 0);

	m_indexfile_name = data_file_path;

	TC_Error_Code error_return = init_index_file();
	if (error_return != TC_NO_ERROR)
		return error_return;

	wxFileName f(data_file_path);
	m_harmfile_name = f.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME);
	m_harmfile_name += f.GetName();
	error_return = LoadHarmonicConstants(m_harmfile_name);

	// Mark the index entries individually with invariant harmonic constants
	unsigned int max_index = GetMaxIndex();
	for (unsigned int i = 0; i < max_index; i++) {
		IDX_entry* pIDX = GetIndexEntry(i);
		if (pIDX) {
			pIDX->num_nodes = num_nodes;
			pIDX->num_csts = num_csts;
			pIDX->num_epochs = num_epochs;
			pIDX->first_year = m_first_year;
			pIDX->m_cst_speeds = &m_cst_speeds;
			pIDX->m_cst_nodes = &m_cst_nodes;
			pIDX->m_cst_epochs = &m_cst_epochs;
			pIDX->m_work_buffer = &m_work_buffer;
		}
	}

	return error_return;
}

IDX_entry* TCDS_Ascii_Harmonic::GetIndexEntry(int n_index)
{
	return m_IDX_array.at(n_index);
}

TC_Error_Code TCDS_Ascii_Harmonic::init_index_file()
{
	long int xref_start = 0;

	std::vector<AbbrEntry> abbreviation_array;
	m_IDX_array.clear();
	int have_index = 0;
	int index_in_memory = 0;

	if (!IndexFileIO(IFF_OPEN, 0))
		return TC_NO_ERROR;

	while (IndexFileIO(IFF_READ, 0)) {
		if ((index_line_buffer[0] == '#') || (index_line_buffer[0] <= ' ')) {
			// Skip comment lines
		} else if (!have_index && !xref_start) {
			if (!strncmp(index_line_buffer, "XREF", 4))
				xref_start = IndexFileIO(IFF_TELL, 0);
		} else if (!have_index && !strncmp(index_line_buffer, "*END*", 5)) {
			if (abbreviation_array.empty()) {
				IndexFileIO(IFF_CLOSE, 0);
				return TC_INDEX_FILE_CORRUPT; // missing at least some data so no valid index
			} else {
				// We're done with abbreviation list (and no errors)
				have_index = 1;
			}
		} else if (!have_index && xref_start) {
			wxString line(index_line_buffer, wxConvUTF8);

			AbbrEntry entry;

			wxStringTokenizer tkz(line, _T(" "));
			wxString token = tkz.GetNextToken();
			if (token.IsSameAs(_T("REGION"), FALSE))
				entry.type = REGION;
			else if (token.IsSameAs(_T("COUNTRY"), FALSE))
				entry.type = COUNTRY;
			else if (token.IsSameAs(_T("STATE"), FALSE))
				entry.type = STATE;

			token = tkz.GetNextToken();
			entry.short_s = token;

			entry.long_s = line.Mid(tkz.GetPosition()).Strip();

			abbreviation_array.push_back(entry);

		} else if (have_index && (strchr("TtCcIUu", index_line_buffer[0]))) {
			// Load index file data.
			IDX_entry* pIDX = new IDX_entry;
			pIDX->source_data_type = IDX_entry::SOURCE_TYPE_ASCII_HARMONIC;
			pIDX->pDataSource = NULL;

			index_in_memory = TRUE;
			pIDX->Valid15 = 0;

			build_IDX_entry(pIDX);
			m_IDX_array.push_back(pIDX);
		}
	}
	if (index_in_memory)
		IndexFileIO(IFF_CLOSE, 0); // All done with file

	return TC_NO_ERROR;
}

// Decode an index data line into an IDX_entry
TC_Error_Code TCDS_Ascii_Harmonic::build_IDX_entry(IDX_entry* pIDX)
{
	int TZHr;
	int TZMin;

	char stz[80];

	pIDX->pref_sta_data = NULL; // no reference data yet
	pIDX->IDX_Useable = 1; // but assume data is OK

	pIDX->IDX_tzname.clear();
	if (7 != sscanf(index_line_buffer, "%c%s%lf%lf%d:%d%*c%[^\r\n]", &pIDX->IDX_type,
					&pIDX->IDX_zone[0], &pIDX->IDX_lon, &pIDX->IDX_lat, &TZHr, &TZMin,
					&pIDX->IDX_station_name[0]))
		return (TC_INDEX_ENTRY_BAD);

	pIDX->IDX_time_zone = TZHr * 60 + TZMin;

	if (strchr("tcUu", index_line_buffer[0])) { // Substation so get second line of info
		IndexFileIO(IFF_READ, 0);

		if (index_line_buffer[0] == '^') { // Opencpn special
			if (11 != sscanf(index_line_buffer, "%*c%d %f %f %d %f %f %d %d %d %d%*c%[^\r\n]",
							 &pIDX->IDX_ht_time_off, &pIDX->IDX_ht_mpy, &pIDX->IDX_ht_off,
							 &pIDX->IDX_lt_time_off, &pIDX->IDX_lt_mpy, &pIDX->IDX_lt_off,
							 &pIDX->IDX_sta_num, &pIDX->IDX_flood_dir, &pIDX->IDX_ebb_dir,
							 &pIDX->IDX_ref_file_num, pIDX->IDX_reference_name))
				return (TC_INDEX_ENTRY_BAD);

			if (abs(pIDX->IDX_ht_time_off) > 1000) // useable?
				pIDX->IDX_Useable = 0;

			if (abs(pIDX->IDX_flood_dir) > 360) // useable?
				pIDX->IDX_Useable = 0;
			if (abs(pIDX->IDX_ebb_dir) > 360) // useable?
				pIDX->IDX_Useable = 0;

			// Fix up the secondaries which are identical to masters
			if (pIDX->IDX_ht_mpy == 0.0)
				pIDX->IDX_ht_mpy = 1.0;
			if (pIDX->IDX_lt_mpy == 0.0)
				pIDX->IDX_lt_mpy = 1.0;
		} else {
			if (9
				!= sscanf(index_line_buffer, "%*c%d %f %f %d %f %f %d %d%*c%[^\r\n]",
						  &pIDX->IDX_ht_time_off, &pIDX->IDX_ht_mpy, &pIDX->IDX_ht_off,
						  &pIDX->IDX_lt_time_off, &pIDX->IDX_lt_mpy, &pIDX->IDX_lt_off,
						  &pIDX->IDX_sta_num, &pIDX->IDX_ref_file_num, pIDX->IDX_reference_name)) {
				// Had an error so try alternate with timezone name before ref file number
				if (10 != sscanf(index_line_buffer, "%*c%d %f %f %d %f %f %d %s %d%*c%[^\r\n]",
								 &pIDX->IDX_ht_time_off, &pIDX->IDX_ht_mpy, &pIDX->IDX_ht_off,
								 &pIDX->IDX_lt_time_off, &pIDX->IDX_lt_mpy, &pIDX->IDX_lt_off,
								 &pIDX->IDX_sta_num, stz, &pIDX->IDX_ref_file_num,
								 pIDX->IDX_reference_name))
					return (TC_INDEX_ENTRY_BAD);
			}

			pIDX->IDX_tzname = stz;
		}

		// We only consider 1 reference file per index file
		pIDX->IDX_ref_file_num = 0;
	} else {
		// Reference stations have no offsets
		pIDX->IDX_ht_time_off = pIDX->IDX_lt_time_off = 0;
		pIDX->IDX_ht_mpy = pIDX->IDX_lt_mpy = 1.0;
		pIDX->IDX_ht_off = pIDX->IDX_lt_off = 0.0;
		pIDX->IDX_sta_num = 0;
		strcpy(pIDX->IDX_reference_name, pIDX->IDX_station_name);
	}

	if (pIDX->IDX_ht_time_off || pIDX->IDX_ht_off != 0.0 || pIDX->IDX_lt_off != 0.0
		|| pIDX->IDX_ht_mpy != 1.0 || pIDX->IDX_lt_mpy != 1.0)
		pIDX->have_offsets = 1;

	pIDX->station_tz_offset = 0; // ASCII Harmonic data is (always??) corrected to Ref Station TZ

	return TC_NO_ERROR;
}

// Load the Harmonic Constant Invariants
TC_Error_Code TCDS_Ascii_Harmonic::LoadHarmonicConstants(const wxString& data_file_path)
{
	char linrec[LINELEN];
	char junk[80];
	int a;
	int b;

	free_data();

	FILE* fp = fopen(data_file_path.mb_str(), "r");
	if (NULL == fp)
		return TC_FILE_NOT_FOUND;

	read_next_line(fp, linrec, 0);
	sscanf(linrec, "%d", &num_csts);

	m_cst_speeds.clear();
	m_cst_speeds.resize(num_csts);
	m_work_buffer.clear();
	m_work_buffer.resize(num_csts);

	// Load constituent speeds
	for (a = 0; a < num_csts; a++) {
		read_next_line(fp, linrec, 0);
		double value = 0.0;
		sscanf(linrec, "%s %lf", junk, &value);
		m_cst_speeds[a] = value * M_PI / 648000; // Convert to radians per second
	}

	// Get first year for nodes and epochs
	read_next_line(fp, linrec, 0);
	sscanf(linrec, "%d", &m_first_year);

	// Load epoch table
	read_next_line(fp, linrec, 0);
	sscanf(linrec, "%d", &num_epochs);

	m_cst_epochs.clear();
	m_cst_epochs.resize(num_csts, std::vector<double>(num_epochs));
	for (int i = 0; i < num_csts; i++) {
		if (EOF == fscanf(fp, "%s", linrec))
			return TC_HARM_FILE_CORRUPT;
		for (int b = 0; b < num_epochs; b++) {
			if (EOF == fscanf(fp, "%lf", &(m_cst_epochs[i][b])))
				return TC_HARM_FILE_CORRUPT;
			m_cst_epochs[i][b] *= M_PI / 180.0;
		}
	}

	// Sanity check
	if (EOF == fscanf(fp, "%s", linrec))
		return TC_HARM_FILE_CORRUPT;
	skipnl(fp);

	// Load node factor table
	read_next_line(fp, linrec, 0);
	sscanf(linrec, "%d", &num_nodes);

	m_cst_nodes.clear();
	m_cst_nodes.resize(num_csts, std::vector<double>(num_nodes));
	for (int a = 0; a < num_csts; a++) {
		fscanf(fp, "%s", linrec);
		for (b = 0; b < num_nodes; b++)
			fscanf(fp, "%lf", &(m_cst_nodes[a][b]));
	}

	fclose(fp);

	return TC_NO_ERROR;
}

TC_Error_Code TCDS_Ascii_Harmonic::LoadHarmonicData(IDX_entry* pIDX)
{
	Station_Data* psd = NULL;

	// Look in the index first
	if (pIDX->pref_sta_data)
		return TC_NO_ERROR; // easy

	// Try the member array of "already-looked-at" master stations
	for (unsigned int i = 0; i < m_msd_array.size(); i++) {
		psd = &m_msd_array.at(i);
		//    In the following comparison, it is allowed that the sub-station reference_name may be
		//          a pre-subset of the master station name.
		//          e.g  IDX_refence_name:  The Narrows midchannel New York
		//                            as found in HARMONIC.IDX
		//                 psd_station_name:      The Narrows, Midchannel, New York Harbor, New York
		// Current
		//                            as found in HARMONIC
		if ((!slackcmp(psd->station_name.c_str(), pIDX->IDX_reference_name))
			&& (toupper(pIDX->IDX_type) == psd->station_type)) {
			pIDX->pref_sta_data = psd; // save for later
			return TC_NO_ERROR;
		}
	}

	// OK, have to read and create from the raw file
	psd = NULL;

	// If reference station was recently sought, and not found, don't bother
	if (m_last_reference_not_found.IsSameAs(wxString(pIDX->IDX_reference_name, wxConvUTF8)))
		return TC_MASTER_HARMONICS_NOT_FOUND;

	// Clear for this looking
	m_last_reference_not_found.Clear();

	// Find and load appropriate constituents
	char linrec[LINELEN];
	FILE* fp = fopen(m_harmfile_name.mb_str(), "r");

	while (read_next_line(fp, linrec, 1)) {
		nojunk(linrec);
		int curonly = 0;
		if (curonly)
			if (!strstr(linrec, "Current"))
				continue;

		// See the note above about station names

		if (slackcmp(linrec, pIDX->IDX_reference_name))
			continue;

		// Got the right location, so load the data

		psd = new Station_Data;

		psd->amplitude.clear();
		psd->amplitude.resize(num_csts);
		psd->epoch.clear();
		psd->epoch.resize(num_csts);
		psd->station_name = linrec;

		// Establish Station Type
		wxString caplin(linrec, wxConvUTF8);
		caplin.MakeUpper();
		if (caplin.Contains(_T("CURRENT")))
			psd->station_type = 'C';
		else
			psd->station_type = 'T';

		// Get meridian
		read_next_line(fp, linrec, 0);
		psd->meridian = hhmm2seconds(linrec);
		psd->zone_offset = 0;

		// Get tzfile, if present
		char junk[80];
		if (sscanf(nojunk(linrec), "%s %s", junk, psd->tzfile) < 2)
			strcpy(psd->tzfile, "UTC0");

		// Get DATUM and units
		read_next_line(fp, linrec, 0);
		if (sscanf(nojunk(linrec), "%lf %s", &(psd->DATUM), psd->unit) < 2)
			strcpy(psd->unit, "unknown");

		int unit_index = findunit(psd->unit);
		psd->have_BOGUS = (unit_index != -1) && (get_unit(unit_index).type == BOGUS);

		int unit_c = -1;
		if (psd->have_BOGUS)
			unit_c = findunit("knots");
		else
			unit_c = findunit(psd->unit);

		if (unit_c != -1) {
			strncpy(psd->units_conv, get_unit(unit_c).name.c_str(), sizeof(psd->units_conv) - 1);
			strncpy(psd->units_abbrv, get_unit(unit_c).abbrv.c_str(), sizeof(psd->units_abbrv) - 1);
		}

		// Get constituents
		double loca;
		double loce;
		for (int a = 0; a < num_csts; a++) {
			read_next_line(fp, linrec, 0);
			sscanf(linrec, "%s %lf %lf", junk, &loca, &loce);
			psd->amplitude[a] = loca;
			psd->epoch[a] = loce * M_PI / 180.0;
		}
		fclose(fp);

		break;
	}

	if (!psd) {
		m_last_reference_not_found = wxString(pIDX->IDX_reference_name, wxConvUTF8);
		return TC_MASTER_HARMONICS_NOT_FOUND;
	} else {
		m_msd_array.push_back(*psd); // add it to the member array
		pIDX->pref_sta_data = psd;
		return TC_NO_ERROR;
	}
}

// Low level Index file I/O
long TCDS_Ascii_Harmonic::IndexFileIO(int func, long value)
{
	switch (func) {
		// Close either/both if open
		case IFF_CLOSE:
			if (m_IndexFile)
				fclose(m_IndexFile);
			m_IndexFile = NULL;
			return 0;

		// Open
		case IFF_OPEN:
			m_IndexFile = fopen(m_indexfile_name.mb_str(), "rt");
			if (m_IndexFile == NULL)
				return 0;
			return 1;

		// Return file pointer only happens with master file
		case IFF_TELL:
			return (ftell(m_IndexFile));

		// Seek
		case IFF_SEEK:
			return (fseek(m_IndexFile, value, SEEK_SET));

		// Read until EOF.
		case IFF_READ:
			if (fgets(index_line_buffer, sizeof(index_line_buffer), m_IndexFile) != NULL)
				return 1;
			else
				return 0;
	}
	return 0;
}

// Read a line from the harmonics file, skipping comment lines
int TCDS_Ascii_Harmonic::read_next_line(FILE* fp, char* linrec, int end_ok)
{
	do {
		if (!fgets(linrec, LINELEN, fp)) {
			if (end_ok)
				return 0;
			else {
				exit(-1);
			}
		}
	} while (linrec[0] == '#' || linrec[0] == '\r' || linrec[0] == '\n');
	return 1;
}

// Remove lingering carriage return, but do nothing else
int TCDS_Ascii_Harmonic::skipnl(FILE* fp)
{
	char linrec[LINELEN];
	if (NULL == fgets(linrec, LINELEN, fp))
		return 0;
	return 1;
}

// Get rid of trailing garbage in buffer
char* TCDS_Ascii_Harmonic::nojunk(char* line)
{
	char* a = &(line[strlen(line)]);
	while (a > line)
		if (*(a - 1) == '\n' || *(a - 1) == '\r' || *(a - 1) == ' ')
			*(--a) = '\0';
		else
			break;
	return line;
}

// Slackful strcmp; 0 = match.  It's case-insensitive and accepts a
// prefix instead of the entire string.  The second argument is the
// one that can be shorter. Second argument can contain '?' as wild
// card character.
int TCDS_Ascii_Harmonic::slackcmp(const char* a, const char* b)
{
	int n = strlen(b);
	if ((int)(strlen(a)) < n)
		return 1;
	for (int c = 0; c < n; c++) {
		if (b[c] == '?')
			continue;

		int cmp = ((a[c] >= 'A' && a[c] <= 'Z') ? a[c] - 'A' + 'a' : a[c])
			  - ((b[c] >= 'A' && b[c] <= 'Z') ? b[c] - 'A' + 'a' : b[c]);
		if (cmp)
			return cmp;
	}
	return 0;
}

void TCDS_Ascii_Harmonic::free_nodes()
{
	std::vector<std::vector<double> > tmp;
	m_cst_nodes.swap(tmp);
}

void TCDS_Ascii_Harmonic::free_epochs()
{
	std::vector<std::vector<double> > tmp;
	m_cst_epochs.swap(tmp);
}

int TCDS_Ascii_Harmonic::GetMaxIndex(void) const
{
	return m_IDX_array.size();
}

// free harmonics data
void TCDS_Ascii_Harmonic::free_data()
{
	free_nodes();
	free_epochs();
}

}

