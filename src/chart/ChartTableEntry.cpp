/***************************************************************************
 *
 * Project:  ChartManager
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

#include "ChartTableEntry.h"

#include <chart/ChartBase.h>
#include <chart/ChartDatabase.h>
#include <chart/PlyPoint.h>

#include <wx/filename.h>
#include <wx/log.h>
#include <wx/stream.h>

using chart::Plypoint;

extern int s_dbVersion;

struct ChartTableEntry_onDisk_17
{
	int EntryOffset;
	int ChartType;
	float LatMax;
	float LatMin;
	float LonMax;
	float LonMin;

	int Scale;
	int edition_date;
	int file_date;

	int nPlyEntries;
	int nAuxPlyEntries;

	float skew;
	int ProjectionType;
	bool bValid;

	int nNoCovrPlyEntries;
};

struct ChartTableEntry_onDisk_16
{
	int EntryOffset;
	int ChartType;
	float LatMax;
	float LatMin;
	float LonMax;
	float LonMin;

	int Scale;
	int edition_date;
	int file_date;

	int nPlyEntries;
	int nAuxPlyEntries;

	float skew;
	int ProjectionType;
	bool bValid;
};

struct ChartTableEntry_onDisk_15
{
	int EntryOffset;
	int ChartType;
	float LatMax;
	float LatMin;
	float LonMax;
	float LonMin;

	int Scale;
	time_t edition_date;
	time_t file_date;

	int nPlyEntries;
	int nAuxPlyEntries;

	bool bValid;
};

struct ChartTableEntry_onDisk_14
{
	int EntryOffset;
	int ChartType;
	char ChartID[16];
	float LatMax;
	float LatMin;
	float LonMax;
	float LonMin;
	char* pFullPath;
	int Scale;
	time_t edition_date;
	float* pPlyTable;
	int nPlyEntries;
	int nAuxPlyEntries;
	float** pAuxPlyTable;
	int* pAuxCntTable;
	bool bValid;
};

ChartTableEntry::ChartTableEntry()
{
	Clear();
}

void ChartTableEntry::SetValid(bool valid)
{
	bValid = valid;
}

time_t ChartTableEntry::GetFileTime() const
{
	return file_date;
}

int ChartTableEntry::GetnPlyEntries() const
{
	return nPlyEntries;
}

float *ChartTableEntry::GetpPlyTable() const
{
	return pPlyTable;
}

int ChartTableEntry::GetnAuxPlyEntries() const
{
	return nAuxPlyEntries;
}

float *ChartTableEntry::GetpAuxPlyTableEntry(int index) const
{
	return pAuxPlyTable[index];
}

int ChartTableEntry::GetAuxCntTableEntry(int index) const
{
	return pAuxCntTable[index];
}

int ChartTableEntry::GetnNoCovrPlyEntries() const
{
	return nNoCovrPlyEntries;
}

float *ChartTableEntry::GetpNoCovrPlyTableEntry(int index) const
{
	return pNoCovrPlyTable[index];
}

int ChartTableEntry::GetNoCovrCntTableEntry(int index) const
{
	return pNoCovrCntTable[index];
}

char * ChartTableEntry::GetpFullPath() const
{
	return pFullPath;
}

float ChartTableEntry::GetLonMax() const
{
	return LonMax;
}

float ChartTableEntry::GetLonMin() const
{
	return LonMin;
}

float ChartTableEntry::GetLatMax() const
{
	return LatMax;
}

float ChartTableEntry::GetLatMin() const
{
	return LatMin;
}

int ChartTableEntry::GetScale() const
{
	return Scale;
}

int ChartTableEntry::GetChartProjectionType() const
{
	return ProjectionType;
}

float ChartTableEntry::GetChartSkew() const
{
	return Skew;
}

bool ChartTableEntry::GetbValid() const
{
	return bValid;
}

void ChartTableEntry::SetEntryOffset(int n)
{
	EntryOffset = n;
}

std::vector<int> &ChartTableEntry::GetGroupArray(void)
{
	return m_GroupArray;
}

const wxString & ChartTableEntry::GetFileName(void) const
{
	return m_filename;
}

ChartTableEntry::ChartTableEntry(ChartBase& theChart)
{
	Clear();

	// FIXME: use string
	char* pt = (char*)malloc(strlen(theChart.GetFullPath().mb_str(wxConvUTF8)) + 1);
	strcpy(pt, theChart.GetFullPath().mb_str(wxConvUTF8));
	pFullPath = pt;

	ChartType = theChart.GetChartType();
	Scale = theChart.GetNativeScale();

	Skew = theChart.GetChartSkew();
	ProjectionType = theChart.GetChartProjectionType();

	wxDateTime ed = theChart.GetEditionDate();
	if (theChart.GetEditionDate().IsValid())
		edition_date = theChart.GetEditionDate().GetTicks();

	wxFileName fn(theChart.GetFullPath());
	if (fn.GetModificationTime().IsValid())
		file_date = fn.GetModificationTime().GetTicks();

	m_filename = fn.GetFullName();

	Extent ext;
	theChart.GetChartExtent(&ext);
	LatMax = ext.NLAT;
	LatMin = ext.SLAT;
	LonMin = ext.WLON;
	LonMax = ext.ELON;

	// Fill in the PLY information

	// If COVR table has only one entry, us it for the primary Ply Table
	if (theChart.GetCOVREntries() == 1) {
		nPlyEntries = theChart.GetCOVRTablePoints(0);
		float* pf = (float*)malloc(2 * nPlyEntries * sizeof(float));
		pPlyTable = pf;
		float* pfe = pf;
		Plypoint* ppp = reinterpret_cast<Plypoint*>(theChart.GetCOVRTableHead(0)); // FIXME

		for (int i = 0; i < nPlyEntries; i++) {
			*pfe++ = ppp->ltp;
			*pfe++ = ppp->lnp;
			ppp++;
		}
	} else {
		// Else create a rectangular primary Ply Table from the chart extents
		// and create AuxPly table from the COVR tables
		// Create new artificial Ply table from chart extents
		nPlyEntries = 4;
		float* pf1 = (float*)malloc(2 * 4 * sizeof(float));
		pPlyTable = pf1;
		float* pfe = pf1;
		Extent fext;
		theChart.GetChartExtent(&fext);

		*pfe++ = fext.NLAT; // LatMax;
		*pfe++ = fext.WLON; // LonMin;

		*pfe++ = fext.NLAT; // LatMax;
		*pfe++ = fext.ELON; // LonMax;

		*pfe++ = fext.SLAT; // LatMin;
		*pfe++ = fext.ELON; // LonMax;

		*pfe++ = fext.SLAT; // LatMin;
		*pfe++ = fext.WLON; // LonMin;

		// Fill in the structure for pAuxPlyTable

		nAuxPlyEntries = theChart.GetCOVREntries();
		wxASSERT(nAuxPlyEntries);
		float** pfp = (float**)malloc(nAuxPlyEntries * sizeof(float*));
		float** pft0 = pfp;
		int* pip = (int*)malloc(nAuxPlyEntries * sizeof(int));

		for (int j = 0; j < nAuxPlyEntries; j++) {
			float* pf_entry = (float*)malloc(theChart.GetCOVRTablePoints(j) * 2 * sizeof(float));
			memcpy(pf_entry, theChart.GetCOVRTableHead(j),
				   theChart.GetCOVRTablePoints(j) * 2 * sizeof(float));
			pft0[j] = pf_entry;
			pip[j] = theChart.GetCOVRTablePoints(j);
		}

		pAuxPlyTable = pfp;
		pAuxCntTable = pip;
	}

	// Get and populate the NoCovr tables

	nNoCovrPlyEntries = theChart.GetNoCOVREntries();
	float** pfpnc = (float**)malloc(nNoCovrPlyEntries * sizeof(float*));
	float** pft0nc = pfpnc;
	int* pipnc = (int*)malloc(nNoCovrPlyEntries * sizeof(int));

	for (int j = 0; j < nNoCovrPlyEntries; j++) {
		float* pf_entry = (float*)malloc(theChart.GetNoCOVRTablePoints(j) * 2 * sizeof(float));
		memcpy(pf_entry, theChart.GetNoCOVRTableHead(j),
			   theChart.GetNoCOVRTablePoints(j) * 2 * sizeof(float));
		pft0nc[j] = pf_entry;
		pipnc[j] = theChart.GetNoCOVRTablePoints(j);
	}

	pNoCovrPlyTable = pfpnc;
	pNoCovrCntTable = pipnc;
}

ChartTableEntry::~ChartTableEntry()
{
	free(pFullPath);
	free(pPlyTable);

	for (int i = 0; i < nAuxPlyEntries; i++)
		free(pAuxPlyTable[i]);
	free(pAuxPlyTable);
	free(pAuxCntTable);

	if (nNoCovrPlyEntries) {
		for (int i = 0; i < nNoCovrPlyEntries; i++)
			free(pNoCovrPlyTable[i]);
		free(pNoCovrPlyTable);
		free(pNoCovrCntTable);
	}
}

bool ChartTableEntry::IsEarlierThan(const ChartTableEntry &cte) const
{
	wxDateTime mine(edition_date);
	wxDateTime theirs(cte.edition_date);
	return mine.IsEarlierThan(theirs);
}

bool ChartTableEntry::IsEqualTo(const ChartTableEntry &cte) const
{
	wxDateTime mine(edition_date);
	wxDateTime theirs(cte.edition_date);
	return mine.IsEqualTo(theirs);
}

int ChartTableEntry::GetChartType() const
{
	// Hackeroo here....
	// dB version 14 had different ChartType Enum, patch it here
	// FIXME: yes this is a hack, plugin adaptations should be done in the plugin stuff, not here
	if (s_dbVersion == 14) {
		switch (ChartType) {
			case 0:
				return CHART_TYPE_KAP;
			case 1:
				return CHART_TYPE_GEO;
			case 2:
				return CHART_TYPE_S57;
			case 3:
				return CHART_TYPE_CM93;
			case 4:
				return CHART_TYPE_CM93COMP;
			case 5:
				return CHART_TYPE_UNKNOWN;
			case 6:
				return CHART_TYPE_DONTCARE;
			case 7:
				return CHART_TYPE_DUMMY;
			default:
				return CHART_TYPE_UNKNOWN;
		}
	} else
		return ChartType;
}

int ChartTableEntry::GetChartFamily() const
{
	switch (ChartType) {
		case CHART_TYPE_KAP:
		case CHART_TYPE_GEO:
			return CHART_FAMILY_RASTER;

		case CHART_TYPE_S57:
		case CHART_TYPE_CM93:
		case CHART_TYPE_CM93COMP:
			return CHART_FAMILY_VECTOR;

		default:
			return CHART_FAMILY_UNKNOWN;
	}
}

void ChartTableEntry::read_17(wxInputStream & is)
{
	char path[4096]; // FIXME: potential buffer overflow, replace dynamic memory allocation with std::string
	char *cp;

	// Read the path first
	for (cp = path; (*cp = (char)is.GetC()) != 0; cp++); // FIXME: potential buffer overflow
	pFullPath = (char *)malloc(cp - path + 1); // FIXME: use string
	strncpy(pFullPath, path, cp - path + 1);
	wxLogVerbose(_T("  Chart %s"), pFullPath);

	//  Create and populate the helper members
	m_filename = wxFileName(wxString(pFullPath, wxConvUTF8)).GetFullName();

	// Read the table entry
	ChartTableEntry_onDisk_17 cte;
	is.Read(&cte, sizeof(ChartTableEntry_onDisk_17));

	//    Transcribe the elements....
	EntryOffset = cte.EntryOffset;
	ChartType = cte.ChartType;
	LatMax = cte.LatMax;
	LatMin = cte.LatMin;
	LonMax = cte.LonMax;
	LonMin = cte.LonMin;

	Skew = cte.skew;
	ProjectionType = cte.ProjectionType;

	Scale = cte.Scale;
	edition_date = cte.edition_date;
	file_date = cte.file_date;

	nPlyEntries = cte.nPlyEntries;
	nAuxPlyEntries = cte.nAuxPlyEntries;

	nNoCovrPlyEntries = cte.nNoCovrPlyEntries;

	bValid = cte.bValid;

	if (nPlyEntries) {
		int npeSize = nPlyEntries * 2 * sizeof(float);
		pPlyTable = (float *)malloc(npeSize);
		is.Read(pPlyTable, npeSize);
	}

	if (nAuxPlyEntries) {
		int napeSize = nAuxPlyEntries * sizeof(int);
		pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
		pAuxCntTable = (int *)malloc(napeSize);
		is.Read(pAuxCntTable, napeSize);

		for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
			int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
			pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
			is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
		}
	}

	if (nNoCovrPlyEntries) {
		int napeSize = nNoCovrPlyEntries * sizeof(int);
		pNoCovrCntTable = (int *)malloc(napeSize);
		is.Read(pNoCovrCntTable, napeSize);

		pNoCovrPlyTable = (float **)malloc(nNoCovrPlyEntries * sizeof(float *));
		for (int i = 0; i < nNoCovrPlyEntries; i++) {
			int nfSize = pNoCovrCntTable[i] * 2 * sizeof(float);
			pNoCovrPlyTable[i] = (float *)malloc(nfSize);
			is.Read(pNoCovrPlyTable[i], nfSize);
		}
	}
}

void ChartTableEntry::read_16(wxInputStream & is)
{
	char path[4096]; // FIXME: potential buffer overflow, replace dynamic memory allocation with std::string
	char *cp;

	// Read the path first
	for (cp = path; (*cp = (char)is.GetC()) != 0; cp++);
	// TODO: optimize prepended dir
	pFullPath = (char *)malloc(cp - path + 1); // FIXME: use string
	strncpy(pFullPath, path, cp - path + 1);
	wxLogVerbose(_T("  Chart %s"), pFullPath);

	// Create and populate the helper members
	m_filename = wxFileName(wxString(pFullPath, wxConvUTF8)).GetFullName();

	// Read the table entry
	ChartTableEntry_onDisk_16 cte;
	is.Read(&cte, sizeof(ChartTableEntry_onDisk_16));

	//    Transcribe the elements....
	EntryOffset = cte.EntryOffset;
	ChartType = cte.ChartType;
	LatMax = cte.LatMax;
	LatMin = cte.LatMin;
	LonMax = cte.LonMax;
	LonMin = cte.LonMin;

	Skew = cte.skew;
	ProjectionType = cte.ProjectionType;

	Scale = cte.Scale;
	edition_date = cte.edition_date;
	file_date = cte.file_date;

	nPlyEntries = cte.nPlyEntries;
	nAuxPlyEntries = cte.nAuxPlyEntries;

	bValid = cte.bValid;

	if (nPlyEntries) {
		int npeSize = nPlyEntries * 2 * sizeof(float);
		pPlyTable = (float *)malloc(npeSize);
		is.Read(pPlyTable, npeSize);
	}

	if (nAuxPlyEntries) {
		int napeSize = nAuxPlyEntries * sizeof(int);
		pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
		pAuxCntTable = (int *)malloc(napeSize);
		is.Read(pAuxCntTable, napeSize);

		for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
			int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
			pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
			is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
		}
	}
}

void ChartTableEntry::read_15(wxInputStream & is)
{
	char path[4096]; // FIXME: potential buffer overflow, replace dynamic memory allocation with std::string
	char *cp;

	// Read the path first
	for (cp = path; (*cp = (char)is.GetC()) != 0; cp++);
	// TODO: optimize prepended dir
	pFullPath = (char *)malloc(cp - path + 1); // FIXME: use string
	strncpy(pFullPath, path, cp - path + 1);
	wxLogVerbose(_T("  Chart %s"), pFullPath);

	// Read the table entry
	ChartTableEntry_onDisk_15 cte;
	is.Read(&cte, sizeof(ChartTableEntry_onDisk_15));

	//    Transcribe the elements....
	EntryOffset = cte.EntryOffset;
	ChartType = cte.ChartType;
	LatMax = cte.LatMax;
	LatMin = cte.LatMin;
	LonMax = cte.LonMax;
	LonMin = cte.LonMin;

	Scale = cte.Scale;
	edition_date = cte.edition_date;
	file_date = cte.file_date;

	nPlyEntries = cte.nPlyEntries;
	nAuxPlyEntries = cte.nAuxPlyEntries;

	bValid = cte.bValid;

	if (nPlyEntries) {
		int npeSize = nPlyEntries * 2 * sizeof(float);
		pPlyTable = (float *)malloc(npeSize);
		is.Read(pPlyTable, npeSize);
	}

	if (nAuxPlyEntries) {
		int napeSize = nAuxPlyEntries * sizeof(int);
		pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
		pAuxCntTable = (int *)malloc(napeSize);
		is.Read(pAuxCntTable, napeSize);

		for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
			int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
			pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
			is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
		}
	}
}

void ChartTableEntry::read_14(wxInputStream & is)
{
	char path[4096]; // FIXME: potential buffer overflow, replace dynamic memory allocation with std::string
	char *cp;

	// Read the path first
	for (cp = path; (*cp = (char)is.GetC()) != 0; cp++);
	pFullPath = (char *)malloc(cp - path + 1); // FIXME: use string
	strncpy(pFullPath, path, cp - path + 1);
	wxLogVerbose(_T("  Chart %s"), pFullPath);

	// Read the table entry
	ChartTableEntry_onDisk_14 cte;
	is.Read(&cte, sizeof(ChartTableEntry_onDisk_14));

	//    Transcribe the elements....
	EntryOffset = cte.EntryOffset;
	ChartType = cte.ChartType;
	LatMax = cte.LatMax;
	LatMin = cte.LatMin;
	LonMax = cte.LonMax;
	LonMin = cte.LonMin;
	Scale = cte.Scale;
	edition_date = cte.edition_date;
	file_date = 0;                        //  file_date does not exist in V14;
	nPlyEntries = cte.nPlyEntries;
	nAuxPlyEntries = cte.nAuxPlyEntries;
	bValid = cte.bValid;

	if (nPlyEntries) {
		int npeSize = nPlyEntries * 2 * sizeof(float);
		pPlyTable = (float *)malloc(npeSize);
		is.Read(pPlyTable, npeSize);
	}

	if (nAuxPlyEntries) {
		int napeSize = nAuxPlyEntries * sizeof(int);
		pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
		pAuxCntTable = (int *)malloc(napeSize);
		is.Read(pAuxCntTable, napeSize);

		for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
			int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
			pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
			is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
		}
	}
}

bool ChartTableEntry::Read(const ChartDatabase * pDb, wxInputStream & is)
{
	// TODO: exception handling

	Clear(); // FIXME: potential memory leak

	// Allow reading of current db format, and maybe others
	switch (pDb->GetVersion()) {
		case 17: read_17(is); break;
		case 16: read_16(is); break;
		case 15: read_15(is); break;
		case 14: read_14(is); break;
	}

	return true;
}

bool ChartTableEntry::Write(const ChartDatabase * WXUNUSED(pDb), wxOutputStream &os)
{
	os.Write(pFullPath, strlen(pFullPath) + 1);

	// Write the current version type only
	// Create an on_disk table entry
	ChartTableEntry_onDisk_17 cte;

	// Transcribe the elements....
	cte.EntryOffset = EntryOffset;
	cte.ChartType = ChartType;
	cte.LatMax = LatMax;
	cte.LatMin = LatMin;
	cte.LonMax = LonMax;
	cte.LonMin = LonMin;

	cte.Scale = Scale;
	cte.edition_date = edition_date;
	cte.file_date = file_date;

	cte.nPlyEntries = nPlyEntries;
	cte.nAuxPlyEntries = nAuxPlyEntries;

	cte.skew = Skew;
	cte.ProjectionType = ProjectionType;

	cte.bValid = bValid;

	cte.nNoCovrPlyEntries = nNoCovrPlyEntries;

	os.Write(&cte, sizeof(ChartTableEntry_onDisk_17));
	wxLogVerbose(_T("  Wrote Chart %s"), pFullPath);

	//      Write out the tables
	if (nPlyEntries) {
		int npeSize = nPlyEntries * 2 * sizeof(float);
		os.Write(pPlyTable, npeSize);
	}

	if (nAuxPlyEntries) {
		int napeSize = nAuxPlyEntries * sizeof(int);
		os.Write(pAuxCntTable, napeSize);

		for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
			int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
			os.Write(pAuxPlyTable[nAuxPlyEntry], nfSize);
		}
	}

	if (nNoCovrPlyEntries ) {
		int ncSize = nNoCovrPlyEntries * sizeof(int);
		os.Write(pNoCovrCntTable, ncSize);

		for (int i = 0; i < nNoCovrPlyEntries; i++) {
			int nctSize = pNoCovrCntTable[i] * 2 * sizeof(float);
			os.Write(pNoCovrPlyTable[i], nctSize);
		}
	}

	return true;
}

void ChartTableEntry::Clear()
{
	EntryOffset = 0;
	ChartType = 0;
	LatMax = 0.0f;
	LatMin = 0.0f;
	LonMax = 0.0f;
	LonMin = 0.0f;
	pFullPath = NULL;// FIXME: memory leak?
	Scale = 0;
	edition_date;
	file_date;
	pPlyTable = NULL;// FIXME: memory leak?
	nPlyEntries = 0;
	nAuxPlyEntries = 0;
	pAuxPlyTable = NULL; // FIXME: memory leak?
	pAuxCntTable = NULL; // FIXME: memory leak?
	Skew = 0.0f;
	ProjectionType = 0;
	bValid = false;
	nNoCovrPlyEntries = 0;
	pNoCovrCntTable = NULL; // FIXME: memory leak?
	pNoCovrPlyTable = NULL; // FIXME: memroy leak?

	m_GroupArray.clear();
	m_filename = wxString();
}

void ChartTableEntry::Disable()
{
	// Mark this chart in the database, so that it will not be seen during this run
	// How?  By setting the chart bounding box to an absurd value
	// TODO... Fix this heinous hack
	LatMax = 100.0f;
	LatMin = 91.0f;
}

