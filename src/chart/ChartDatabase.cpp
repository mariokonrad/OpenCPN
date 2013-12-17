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

#include <wx/encconv.h>
#include <wx/regex.h>
#include <wx/progdlg.h>

#include "ChartDatabase.h"
#include <ChartPlugInWrapper.h>
#include <chart/ChartBase.h>

#include <geo/BoundingBox.h>

#include <plugin/PlugInManager.h>

extern PlugInManager* g_pi_manager;

namespace chart {

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ChartTable);

using chart::ChartClassDescriptor;

static const int DB_VERSION_CURRENT = 17; // FIXME: duplicate

int s_dbVersion; // Database version currently in use at runtime

bool FindMatchingFile(const wxString& theDir, const wxChar* theRegEx, int nameLength,
					  wxString& theMatch)
{
	wxDir dir(theDir);
	wxRegEx rePattern(theRegEx);
	for (bool fileFound = dir.GetFirst(&theMatch); fileFound; fileFound = dir.GetNext(&theMatch))
		if (theMatch.length() == (unsigned int)nameLength && rePattern.Matches(theMatch))
			return true;
	return false;
}

chart::ChartFamilyEnum GetChartFamily(int charttype)
{
	switch (charttype) {
		case CHART_TYPE_KAP:      return chart::CHART_FAMILY_RASTER;
		case CHART_TYPE_GEO:      return chart::CHART_FAMILY_RASTER;
		case CHART_TYPE_S57:      return chart::CHART_FAMILY_VECTOR;
		case CHART_TYPE_CM93:     return chart::CHART_FAMILY_VECTOR;
		case CHART_TYPE_CM93COMP: return chart::CHART_FAMILY_VECTOR;
		case CHART_TYPE_DUMMY:    return chart::CHART_FAMILY_RASTER;
		case CHART_TYPE_UNKNOWN:  return chart::CHART_FAMILY_UNKNOWN;
		default: break;
	}
	return chart::CHART_FAMILY_UNKNOWN;
}

ChartDatabase::ChartDatabase()
{
	m_ChartTableEntryDummy.Clear();
	UpdateChartClassDescriptorArray();
}

void ChartDatabase::UpdateChartClassDescriptorArray(void)
{
	m_ChartClassDescriptorArray.clear();

	m_ChartClassDescriptorArray.push_back(ChartClassDescriptor(_T("ChartKAP"), _T("*.kap"), BUILTIN_DESCRIPTOR));
	m_ChartClassDescriptorArray.push_back(ChartClassDescriptor(_T("ChartGEO"), _T("*.geo"), BUILTIN_DESCRIPTOR));
	m_ChartClassDescriptorArray.push_back(ChartClassDescriptor(_T("s57chart"), _T("*.000"), BUILTIN_DESCRIPTOR));
	m_ChartClassDescriptorArray.push_back(ChartClassDescriptor(_T("s57chart"), _T("*.s57"), BUILTIN_DESCRIPTOR));
	m_ChartClassDescriptorArray.push_back(ChartClassDescriptor(_T("cm93compchart"), _T("00300000.a"), BUILTIN_DESCRIPTOR));

	// If the PlugIn Manager exists, get the array of dynamically loadable chart class names
	if (g_pi_manager) {
		wxArrayString array = g_pi_manager->GetPlugInChartClassNameArray();
		for (unsigned int j = 0; j < array.size(); j++) {
			// Instantiate a blank chart to retrieve the directory search mask for this chart type
			wxString class_name = array.Item(j);
			ChartPlugInWrapper* cpiw = new ChartPlugInWrapper(class_name);
			if (cpiw) {
				wxString mask = cpiw->GetFileSearchMask();
				m_ChartClassDescriptorArray.push_back(ChartClassDescriptor(class_name, mask, PLUGIN_DESCRIPTOR));
				delete cpiw;
			}
		}
	}
}

wxString ChartDatabase::getChartClassName(int type, const wxString& ext) const
{
	const wxString ext_upper = ext.Upper();
	const wxString ext_lower = ext.Lower();

	for (ArrayOfChartClassDescriptor::const_iterator i = m_ChartClassDescriptorArray.begin();
		 i != m_ChartClassDescriptorArray.end(); ++i) {
		if (i->m_descriptor_type == type) {
			if (i->m_search_mask == ext_upper) {
				return i->m_class_name;
			}
			if (i->m_search_mask == ext_lower) {
				return i->m_class_name;
			}
		}
	}
	return wxString();
}

void ChartDatabase::SetValid(bool valid)
{
	bValid = valid;
}

int ChartDatabase::GetVersion() const
{
	return m_dbversion;
}

bool ChartDatabase::IsValid() const
{
	return bValid;
}

const wxString& ChartDatabase::GetDBFileName() const
{
	return m_DBFileName;
}

const ArrayOfCDI& ChartDatabase::GetChartDirArray() const
{
	return m_dir_array;
}

const wxArrayString& ChartDatabase::GetChartDirArrayString() const
{
	return m_chartDirs;
}

int ChartDatabase::GetChartTableEntries() const
{
	return chartTable.size();
}

const ChartTableEntry& ChartDatabase::GetChartTableEntry(int index) const
{
	if (index < static_cast<int>(chartTable.size()))
		return chartTable[index];
	else
		return m_ChartTableEntryDummy;
}

bool ChartDatabase::Read(const wxString& filePath)
{
	ChartTableEntry entry;
	int entries;

	bValid = false;

	wxFileName file(filePath);
	if (!file.FileExists())
		return false;

	m_DBFileName = filePath;

	wxFileInputStream ifs(filePath);
	if (!ifs.Ok())
		return false;

	ChartTableHeader cth;
	cth.Read(ifs);
	if (!cth.CheckValid())
		return false;

	// Capture the version number
	char vbo[5];
	memcpy(vbo, cth.GetDBVersionString(), 4);
	vbo[4] = 0;
	m_dbversion = atoi(&vbo[1]);
	s_dbVersion = m_dbversion; // save the static copy

	wxLogVerbose(wxT("Chartdb:Reading %d directory entries, %d table entries"), cth.GetDirEntries(),
				 cth.GetTableEntries());
	wxLogMessage(_T("Chartdb: Chart directory list follows"));
	if (0 == cth.GetDirEntries())
		wxLogMessage(_T("  Nil"));

	for (int iDir = 0; iDir < cth.GetDirEntries(); iDir++) {
		wxString dir;
		int dirlen;
		ifs.Read(&dirlen, sizeof(int));
		while (dirlen > 0) {
			char dirbuf[1024];
			int alen = dirlen > 1023 ? 1023 : dirlen;
			if (ifs.Read(&dirbuf, alen).Eof())
				goto read_error;
			dirbuf[alen] = 0;
			dirlen -= alen;
			dir.Append(wxString(dirbuf, wxConvUTF8));
		}
		wxString msg;
		msg.Printf(wxT("  Chart directory #%d: "), iDir);
		msg.Append(dir);
		wxLogMessage(msg);
		m_chartDirs.push_back(dir);
	}

	// FIXME: be aware: the allocation and the Read/push_back combo do not caus calls to constructor
	entries = cth.GetTableEntries();
	chartTable.Alloc(entries);
	while (entries-- && entry.Read(this, ifs))
		chartTable.push_back(entry);

	entry.Clear();
	bValid = true;
	return true;

read_error:
	bValid = false;
	return false;
}

bool ChartDatabase::Write(const wxString& filePath)
{
	wxFileName file(filePath);
	wxFileName dir(file.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME, wxPATH_NATIVE));

	if (!dir.DirExists() && !dir.Mkdir())
		return false;

	wxFileOutputStream ofs(filePath);
	if (!ofs.Ok())
		return false;

	ChartTableHeader cth(m_chartDirs.size(), chartTable.size());
	cth.Write(ofs);

	for (int iDir = 0; iDir < cth.GetDirEntries(); iDir++) {
		const wxString& dir = m_chartDirs[iDir];
		int dirlen = dir.length();
		char s[200];
		strncpy(s, dir.mb_str(wxConvUTF8), 199);
		s[199] = 0;
		dirlen = strlen(s);
		ofs.Write(&dirlen, sizeof(int));
		ofs.Write(s, dirlen);
	}

	for (unsigned int iTable = 0; iTable < chartTable.size(); iTable++)
		chartTable[iTable].Write(this, ofs);

	// Explicitly set the version
	m_dbversion = DB_VERSION_CURRENT;

	return true;
}

wxString SplitPath(wxString s, wxString tkd, int nchar, int offset, int* pn_split)
{
	wxString r;
	int ncr = 0;

	int rlen = offset;
	wxStringTokenizer tkz(s, tkd);
	while (tkz.HasMoreTokens()) {
		wxString token = tkz.GetNextToken();
		if ((rlen + (int)token.Len() + 1) < nchar) {
			r += token;
			r += tkd[0];
			rlen += token.Len() + 1;
		} else {
			r += _T("\n");
			ncr++;
			for (int i = 0; i < offset; i++) {
				r += _T(" ");
			}
			r += token;
			r += tkd[0];
			rlen = offset + token.Len() + 1;
		}
	}

	if (pn_split)
		*pn_split = ncr;

	return r.Mid(0, r.Len() - 1); // strip the last separator char
}

wxString ChartDatabase::GetFullChartInfo(ChartBase* pc, int dbIndex, int* char_width,
										 int* line_count) const
{
	wxString r;
	int lc = 0;
	unsigned int max_width = 0;
	int ncr;
	unsigned int target_width = 60;

	const ChartTableEntry& cte = GetChartTableEntry(dbIndex);
	if (1) // TODO why can't this be cte.GetbValid()?
	{
		wxString line;
		line = _(" ChartFile:  ");
		wxString longline(cte.GetpFullPath(), wxConvUTF8);
		if (longline.Len() > target_width) {
			line += SplitPath(wxString(cte.GetpFullPath(), wxConvUTF8), _T("/,\\"), target_width,
							  15, &ncr);
			max_width = wxMax(max_width, target_width + 4);
			lc += ncr;
		} else {
			line += longline;
			max_width = wxMax(max_width, line.Len() + 4);
		}

		r += line;
		r += _T("\n");
		lc++;

		line.Empty();
		if (pc) {
			line = _(" Name:  ");
			wxString longline = pc->GetName();

			wxString tkz;
			if (longline.Find(' ') != wxNOT_FOUND) // assume a proper name
				tkz = _T(" ");
			else
				tkz = _T("/,\\"); // else a file name

			if (longline.Len() > target_width) {
				line += SplitPath(pc->GetName(), tkz, target_width, 12, &ncr);
				max_width = wxMax(max_width, target_width + 4);
				lc += ncr;
			} else {
				line += longline;
				max_width = wxMax(max_width, line.Len() + 4);
			}
		}

		line += _T("\n");
		r += line;
		lc++;

		if (pc) // chart is loaded and available
			line.Printf(_(" Scale:  1:%d"), pc->GetNativeScale());
		else
			line.Printf(_(" Scale:  1:%d"), cte.GetScale());

		line += _T("\n");
		max_width = wxMax(max_width, line.Len());
		r += line;
		lc++;

		line.Empty();
		if (pc) {
			line = _(" ID:  ");
			line += pc->GetID();
		}
		line += _T("\n");
		max_width = wxMax(max_width, line.Len());
		r += line;
		lc++;

		line.Empty();
		if (pc) {
			line = _(" Depth Units:  ");
			line += pc->GetDepthUnits();
		}
		line += _T("\n");
		max_width = wxMax(max_width, line.Len());
		r += line;
		lc++;

		line.Empty();
		if (pc) {
			line = _(" Soundings:  ");
			line += pc->GetSoundingsDatum();
		}
		line += _T("\n");
		max_width = wxMax(max_width, line.Len());
		r += line;
		lc++;

		line.Empty();
		if (pc) {
			line = _(" Datum:  ");
			line += pc->GetDatumString();
		}
		line += _T("\n");
		max_width = wxMax(max_width, line.Len());
		r += line;
		lc++;

		line = _(" Projection:  ");
		if(PROJECTION_UNKNOWN == cte.GetChartProjectionType())
			line += _("Unknown");
		else if (PROJECTION_MERCATOR == cte.GetChartProjectionType())
			line += _("Mercator");
		else if (PROJECTION_TRANSVERSE_MERCATOR == cte.GetChartProjectionType())
			line += _("Transverse Mercator");
		else if (PROJECTION_POLYCONIC == cte.GetChartProjectionType())
			line += _("Polyconic");
		line += _T("\n");
		max_width = wxMax(max_width, line.Len());
		r += line;
		lc++;

		line.Empty();
		if (pc) {
			line = _(" Source Edition:  ");
			line += pc->GetSE();
		}
		line += _T("\n");
		max_width = wxMax(max_width, line.Len());
		r += line;
		lc++;

		line.Empty();
		if (pc) {
			line = _(" Updated:  ");
			wxDateTime ed = pc->GetEditionDate();
			line += ed.FormatISODate();
		}
		line += _T("\n");
		max_width = wxMax(max_width, line.Len());
		r += line;
		lc++;

		line.Empty();
		if (pc && pc->GetExtraInfo().Len()) {
			line += pc->GetExtraInfo();
			line += _T("\n");
			max_width = wxMax(max_width, line.Len());
			r += line;
			lc++;
		}
	}

	if (line_count)
		*line_count = lc;

	if (char_width)
		*char_width = max_width;

	return r;
}

// Create Chart Table Database by directory search
// resulting in valid pChartTable in (this)
bool ChartDatabase::Create(ArrayOfCDI& dir_array, wxProgressDialog* pprog)
{
	m_dir_array = dir_array;

	bValid = false;

	m_chartDirs.Clear();
	chartTable.Clear();
	Update(dir_array, true, pprog); // force the update the reload everything

	bValid = true;

	// Explicitly set the version
	m_dbversion = DB_VERSION_CURRENT;

	return true;
}

// Update existing ChartTable Database by directory search
// resulting in valid pChartTable in (this)
bool ChartDatabase::Update(ArrayOfCDI& dir_array, bool bForce, wxProgressDialog* pprog)
{
	m_dir_array = dir_array;

	bValid = false; // database is not useable right now...

	// Mark all charts provisionally invalid
	for (unsigned int i = 0; i < chartTable.size(); i++)
		chartTable[i].SetValid(false);

	m_chartDirs.Clear();

	if (bForce)
		chartTable.Clear();

	bool lbForce = bForce;

	// Do a dB Version upgrade if the current one is obsolete
	if (s_dbVersion != DB_VERSION_CURRENT) {

		chartTable.Clear();
		lbForce = true;
		s_dbVersion = DB_VERSION_CURRENT; // Update the static indicator
		m_dbversion = DB_VERSION_CURRENT; // and the member
	}

	// Get the new charts

	for (ArrayOfCDI::iterator i = dir_array.begin(); i != dir_array.end(); ++i) {
		wxString dir_magic;
		TraverseDirAndAddCharts(*i, pprog, dir_magic, lbForce);
		i->magic_number = dir_magic;
		m_chartDirs.push_back(i->fullpath);
	}

	for (unsigned int i = 0 ; i < chartTable.size(); ++i) {
		if (!chartTable[i].GetbValid()) {
			chartTable.RemoveAt(i);
			i--; // entry is gone, recheck this index for next entry
		}
	}

	// And once more, setting the Entry index field
	for(unsigned int i=0 ; i<chartTable.size() ; i++)
		chartTable[i].SetEntryOffset( i );

	bValid = true;
	return true;
}

// Find Chart dbIndex
int ChartDatabase::FinddbIndex(const wxString& PathToFind) const
{
	// Find the chart
	for (unsigned int i = 0; i < chartTable.size(); i++) {
		if (PathToFind.IsSameAs(wxString(chartTable[i].GetpFullPath(), wxConvUTF8))) {
			return i;
		}
	}
	return -1;
}

// Disable Chart
int ChartDatabase::DisableChart(const wxString& PathToDisable)
{
	// Find the chart
	for (unsigned int i = 0; i < chartTable.size(); i++) {
		if (PathToDisable.IsSameAs(wxString(chartTable[i].GetpFullPath(), wxConvUTF8))) {
			ChartTableEntry* pentry = &chartTable[i];
			pentry->Disable();
			return 1;
		}
	}
	return 0;
}

// Traverse the given directory looking for charts
// If bupdate is true, also search the existing database for a name match.
// If target chart is already in database, mark the entry valid and skip additional processing
int ChartDatabase::TraverseDirAndAddCharts(
		const ChartDirInfo& dir_info,
		wxProgressDialog* pprog,
		wxString& dir_magic,
		bool bForce)
{
	// Extract the true dir name and magic number from the compound string
	wxString dir_path = dir_info.fullpath;
	wxString old_magic = dir_info.magic_number;
	wxString new_magic = old_magic;
	dir_magic = old_magic; // provisionally the same

	int nAdd = 0;

	bool b_skipDetectDirChange = false;
	bool b_dirchange = false;

	// Does this directory actually exist?
	if (!wxDir::Exists(dir_path))
		return 0;

	// Check to see if this is a cm93 directory root
	// If so, skip the DetectDirChange since it may be very slow
	// and give no information
	// Assume a change has happened, and process accordingly
	bool b_cm93 = Check_CM93_Structure(dir_path);
	if (b_cm93) {
		b_skipDetectDirChange = true;
		b_dirchange = true;
	}

	// Quick scan the directory to see if it has changed
	// If not, there is no need to scan again.....
	if (!b_skipDetectDirChange)
		b_dirchange = DetectDirChange(dir_path, old_magic, new_magic, pprog);

	if (!bForce && !b_dirchange) {
		wxLogMessage(_T("   No change detected on directory ") + dir_path);

		// Traverse the database, and mark as valid all charts coming from this dir,
		// or anywhere in its tree

		wxFileName fn_dir(dir_path, _T("stuff"));
		unsigned int dir_path_count = fn_dir.GetDirCount();

		if (pprog)
			pprog->SetTitle(_("OpenCPN Chart Scan...."));

		int nEntries = chartTable.size();

		for (int ic = 0; ic < nEntries; ic++) {
			wxFileName fn(wxString(chartTable[ic].GetpFullPath(), wxConvUTF8));
			wxString t = fn.GetPath();

			while (fn.GetDirCount() >= dir_path_count) {
				t = fn.GetPath();
				if (fn.GetPath() == dir_path) {
					chartTable[ic].SetValid(true);
					break;
				}
				fn.RemoveLastDir();
			}
		}

		return 0;
	}

	// There presumably was a change in the directory contents.  Return the new magic number
	dir_magic = new_magic;

	// Look for all possible defined chart classes
	for (ArrayOfChartClassDescriptor::const_iterator i = m_ChartClassDescriptorArray.begin();
		 i != m_ChartClassDescriptorArray.end(); ++i) {
		nAdd += SearchDirAndAddCharts(dir_path, *i, pprog);
	}

	return nAdd;
}

bool ChartDatabase::DetectDirChange(const wxString& dir_path, const wxString& magic,
									wxString& new_magic, wxProgressDialog* pprog) const
{
	if (pprog)
		pprog->SetTitle(_("OpenCPN Directory Scan...."));

	// parse the magic number
	long long unsigned int nmagic;
	wxULongLong nacc = 0;

	magic.ToULongLong(&nmagic, 10);

	// Get an arraystring of all files
	wxArrayString FileList;
	wxDir dir(dir_path);
	int n_files = dir.GetAllFiles(dir_path, &FileList);

	// Arbitrarily, we decide if the dir has more than a specified number of files
	// then don't scan it.  Takes too long....

	if (n_files > 10000) {
		new_magic = _T("");
		return true;
	}

	// Traverse the list of files, getting their interesting stuff to add to accumulator
	for (int ifile = 0; ifile < n_files; ifile++) {
		if (pprog && ((((ifile * 100) / n_files) % 20) == 0))
			pprog->Update(wxMin((ifile * 100) / n_files, 100), dir_path);

		wxFileName file(FileList.Item(ifile));

		// File Size;
		wxULongLong size = file.GetSize();
		if (wxInvalidSize != size)
			nacc = nacc + size;

		// Mod time, in ticks
		wxDateTime t = file.GetModificationTime();
		nacc += t.GetTicks();

		// File name
		wxString n = file.GetFullName();
		for (unsigned int in = 0; in < n.Len(); in++) {
			nacc += (unsigned char)n[in];
		}
	}

	// Return the calculated magic number
	new_magic = nacc.ToString();

	// And do the test
	if (new_magic != magic)
		return true;
	else
		return false;
}

bool ChartDatabase::IsChartDirUsed(const wxString& theDir) const
{
	wxString dir(theDir);
	if (dir.Last() == '/' || dir.Last() == wxFileName::GetPathSeparator())
		dir.RemoveLast();

	dir.Append(wxT("*"));
	for (unsigned int i = 0; i < chartTable.size(); i++) {
		wxString chartPath(chartTable[i].GetpFullPath(), wxConvUTF8);
		if (chartPath.Matches(dir))
			return true;
	}
	return false;
}

// Validate a given directory as a cm93 root database
// If it appears to be a cm93 database, then return true
bool ChartDatabase::Check_CM93_Structure(wxString dir_name) const
{
	wxString filespec;

	wxRegEx test(_T("[0-9]+"));

	wxDir dirt(dir_name);
	wxString candidate;

	bool b_maybe_found_cm93 = false;
	bool b_cont = dirt.GetFirst(&candidate);

	while (b_cont) {
		if (test.Matches(candidate) && (candidate.Len() == 8)) {
			b_maybe_found_cm93 = true;
			break;
		}

		b_cont = dirt.GetNext(&candidate);
	}

	if (b_maybe_found_cm93) {
		wxString dir_next = dir_name;
		dir_next += _T("/");
		dir_next += candidate;
		if (wxDir::Exists(dir_next)) {
			wxDir dir_n(dir_next);
			wxString candidate_n;

			wxRegEx test_n(_T("^[A-Ga-g]"));
			bool b_probably_found_cm93 = false;
			bool b_cont_n = dir_n.GetFirst(&candidate_n);
			while (b_cont_n) {
				if (test_n.Matches(candidate_n) && (candidate_n.Len() == 1)) {
					b_probably_found_cm93 = true;
					break;
				}
				b_cont_n = dir_n.GetNext(&candidate_n);
			}

			if (b_probably_found_cm93) {
				// found a directory that looks like {dir_name}/12345678/A
				// probably cm93
				// make sure the dir exists
				wxString dir_luk = dir_next;
				dir_luk += _T("/");
				dir_luk += candidate_n;
				if (wxDir::Exists(dir_luk))
					return true;
			}
		}
	}

	return false;
}

// Populate Chart Table by directory search for specified file type
// If bupdate flag is true, search the Chart Table for matching chart.
// if target chart is already in table, mark it valid and skip chart processing
int ChartDatabase::SearchDirAndAddCharts(wxString& dir_name_base,
										 const chart::ChartClassDescriptor& chart_desc,
										 wxProgressDialog* pprog)
{
	wxString msg(_T("Searching directory: "));
	msg += dir_name_base;
	msg += _T(" for ");
	msg += chart_desc.m_search_mask;
	wxLogMessage(msg);

	if (!wxDir::Exists(dir_name_base))
		return 0;

	wxString dir_name = dir_name_base;
	wxString filespec = chart_desc.m_search_mask.Upper();
	wxString lowerFileSpec = chart_desc.m_search_mask.Lower();
	wxString filename;

	// Count the files
	wxArrayString FileList;
	int gaf_flags = wxDIR_DEFAULT; // as default, recurse into subdirs

	// Here is an optimization for MSW/cm93 especially
	// If this directory seems to be a cm93, and we are not explicitely looking for cm93, then
	// abort Otherwise, we will be looking thru entire cm93 tree for non-existent .KAP files, etc.
	bool b_found_cm93 = false;
	bool b_cm93 = Check_CM93_Structure(dir_name);
	if (b_cm93) {
		if (filespec != _T("00300000.A")) {
			return false;
		} else {
			filespec = dir_name;
			b_found_cm93 = true;
		}
	}

	if (!b_found_cm93) {
		wxDir dir(dir_name);
		dir.GetAllFiles(dir_name, &FileList, filespec, gaf_flags);
#ifndef __WXMSW__
		if (filespec != lowerFileSpec) {
			// add lowercase filespec files too
			wxArrayString lowerFileList;
			dir.GetAllFiles(dir_name, &lowerFileList, lowerFileSpec, gaf_flags);
			for (wxArrayString::const_iterator item = lowerFileList.begin();
				 item != lowerFileList.end(); item++)
				FileList.Add(*item);
		}
#endif
	} else { // This is a cm93 dataset, specified as yada/yada/cm93
		wxString dir_plus = dir_name;
#ifdef __WXMSW__
		dir_plus += wxFileName::GetPathSeparator();
#endif
		FileList.Add(dir_plus);
	}

	int nFile = FileList.size();

	if (!nFile)
		return false;

	int nDirEntry = 0;

	// Check to see if there are any charts in the DB which refer to this directory
	// If none at all, there is no need to scan the DB for fullpath match of each potential addition
	// and bthis_dir_in_dB is false.
	bool bthis_dir_in_dB = IsChartDirUsed(dir_name);

	int isearch
		= 0; // create a smarter search index indexing the DB starting from the last found item

	if (pprog)
		pprog->SetTitle(_("OpenCPN Chart Add...."));

	for (int ifile = 0; ifile < nFile; ifile++) {
		wxFileName file(FileList.Item(ifile));
		wxString full_name = file.GetFullPath();
		wxString file_name = file.GetFullName();

		// Validate the file name again, considering MSW's semi-random treatment of case....
		// TODO...something fishy here - may need to normalize saved name?
		if (!file_name.Matches(lowerFileSpec) && !file_name.Matches(filespec) && !b_found_cm93)
			continue;

		if (pprog)
			pprog->Update(wxMin((ifile * 100) / nFile, 100), full_name);

		ChartTableEntry* pnewChart = NULL;
		bool bAddFinal = true;
		int b_add_msg = 0;

		pnewChart = CreateChartTableEntry(full_name, chart_desc);
		if (!pnewChart) {
			bAddFinal = false;
			wxLogMessage(_T("   CreateChartTableEntry() failed for file: ") + full_name);
		} else { // traverse the existing database looking for duplicates, and choosing the right
			// one
			int nEntry = chartTable.size();
			for (int i = 0; i < nEntry; i++) {
				wxString table_file_name(chartTable[isearch].GetpFullPath(), wxConvUTF8);

				// If the chart full file paths are exactly the same, select the newer one
				if (bthis_dir_in_dB && full_name.IsSameAs(table_file_name)) {
					b_add_msg++;

					// Check the file modification time
					time_t t_oldFile = chartTable[isearch].GetFileTime();
					time_t t_newFile = file.GetModificationTime().GetTicks();

					if (t_newFile <= t_oldFile) {
						bAddFinal = false;
						chartTable[isearch].SetValid(true);
					} else {
						bAddFinal = true;
						chartTable[isearch].SetValid(false);
						wxLogMessage(_T("   Replacing older chart file of same path: ")
									 + full_name);
					}

					break;
				}

				// Look at the chart file name (without directory prefix) for a further check for
				// duplicates
				// This catches the case in which the "same" chart is in different locations,
				// and one may be newer than the other.
				wxFileName table_file(table_file_name);

				if (table_file.GetFullName() == file_name) {
					b_add_msg++;

					if (pnewChart->IsEarlierThan(chartTable[isearch])) {
						// Make sure the compare file actually exists
						if (table_file.IsFileReadable()) {
							chartTable[isearch].SetValid(true);
							bAddFinal = false;
							wxLogMessage(_T("   Retaining newer chart file of same name: ")
										 + full_name);
						}
					} else if (pnewChart->IsEqualTo(chartTable[isearch])) {
						// The file names (without dir prefix) are identical,
						// and the mod times are identical
						// Prsume that this is intentional, in order to facilitate
						// having the same chart in multiple groups.
						// So, add this chart.
						bAddFinal = true;
					} else {
						chartTable[isearch].SetValid(false);
						bAddFinal = true;
						wxLogMessage(_T("   Replacing older chart file of same name: ")
									 + full_name);
					}

					break;
				}

				// TODO: Look at the chart ID as a further check against duplicates

				isearch++;
				if (nEntry == isearch)
					isearch = 0;
			}
		}

		if (bAddFinal) {
			if (0 == b_add_msg) {
				wxLogMessage(_T("   Adding chart file: ") + full_name);
			}
			chartTable.push_back(pnewChart);
			nDirEntry++;
		}
	}

	return nDirEntry;
}

bool ChartDatabase::AddChart(wxString& chartfilename, ChartClassDescriptor& chart_desc,
							 wxProgressDialog* pprog, int isearch, bool bthis_dir_in_dB)
{
	bool rv = false;
	wxFileName file(chartfilename);
	wxString full_name = file.GetFullPath();
	wxString file_name = file.GetFullName();

	//    Validate the file name again, considering MSW's semi-random treatment of case....
	// TODO...something fishy here - may need to normalize saved name?
	//    if(!file_name.Matches(lowerFileSpec) && !file_name.Matches(filespec) && !b_found_cm93)
	//        continue;

	if (pprog)
		pprog->Update(wxMin((m_pdifile * 100) / m_pdnFile, 100), full_name);

	ChartTableEntry* pnewChart = NULL;
	bool bAddFinal = true;
	int b_add_msg = 0;

	pnewChart = CreateChartTableEntry(full_name, chart_desc);
	if (!pnewChart) {
		bAddFinal = false;
		wxString msg = _T("   CreateChartTableEntry() failed for file: ");
		msg.Append(full_name);
		wxLogMessage(msg);
		return false;
	} else // traverse the existing database looking for duplicates, and choosing the right one
	{
		int nEntry = chartTable.GetCount();
		for (int i = 0; i < nEntry; i++) {
			wxString table_file_name(chartTable[isearch].GetpFullPath(), wxConvUTF8);

			//    If the chart full file paths are exactly the same, select the newer one
			if (bthis_dir_in_dB && full_name.IsSameAs(table_file_name)) {
				b_add_msg++;

				//    Check the file modification time
				time_t t_oldFile = chartTable[isearch].GetFileTime();
				time_t t_newFile = file.GetModificationTime().GetTicks();

				if (t_newFile <= t_oldFile) {
					bAddFinal = false;
					chartTable[isearch].SetValid(true);
				} else {
					bAddFinal = true;
					chartTable[isearch].SetValid(false);
					wxString msg = _T("   Replacing older chart file of same path: ");
					msg.Append(full_name);
					wxLogMessage(msg);
				}

				break;
			}

			//  Look at the chart file name (without directory prefix) for a further check for
			// duplicates
			//  This catches the case in which the "same" chart is in different locations,
			//  and one may be newer than the other.
			wxFileName table_file(table_file_name);

			if (table_file.GetFullName() == file_name) {
				b_add_msg++;

				if (pnewChart->IsEarlierThan(chartTable[isearch])) {
					//    Make sure the compare file actually exists
					if (table_file.IsFileReadable()) {
						chartTable[isearch].SetValid(true);
						bAddFinal = false;
						wxString msg = _T("   Retaining newer chart file of same name: ");
						msg.Append(full_name);
						wxLogMessage(msg);
					}
				} else if (pnewChart->IsEqualTo(chartTable[isearch])) {
					//    The file names (without dir prefix) are identical,
					//    and the mod times are identical
					//    Prsume that this is intentional, in order to facilitate
					//    having the same chart in multiple groups.
					//    So, add this chart.
					bAddFinal = true;
				} else {
					chartTable[isearch].SetValid(false);
					bAddFinal = true;
					wxString msg = _T("   Replacing older chart file of same name: ");
					msg.Append(full_name);
					wxLogMessage(msg);
				}

				break;
			}

			// TODO    Look at the chart ID as a further check against duplicates

			isearch++;
			if (nEntry == isearch)
				isearch = 0;
		} // for
	}

	if (bAddFinal) {
		if (0 == b_add_msg) {
			wxString msg = _T("   Adding chart file: ");
			msg.Append(full_name);
			wxLogMessage(msg);
		}

		chartTable.Add(pnewChart);

		rv = true;
	} else {
		//                  wxString msg = _T("   Not adding chart file: ");
		//                  msg.Append(full_name);
		//                  wxLogMessage(msg);
		rv = false;
	}

	return rv;
}

bool ChartDatabase::AddSingleChart(wxString& ChartFullPath)
{
	//  Find a relevant chart class descriptor
	wxFileName fn(ChartFullPath);
	wxString ext = fn.GetExt();
	ext.Prepend(_T("*."));
	wxString ext_upper = ext.MakeUpper();
	wxString ext_lower = ext.MakeLower();
	wxString dir_name = fn.GetPath();

	// Search the array of chart class descriptors to find a match
	// bewteen the search mask and the the chart file extension

	ChartClassDescriptor desc;
	for (unsigned int i = 0; i < m_ChartClassDescriptorArray.size(); i++) {
		if (m_ChartClassDescriptorArray.at(i).m_descriptor_type == PLUGIN_DESCRIPTOR) {
			if (m_ChartClassDescriptorArray.at(i).m_search_mask == ext_upper) {
				desc = m_ChartClassDescriptorArray.at(i);
				break;
			}
			if (m_ChartClassDescriptorArray.at(i).m_search_mask == ext_lower) {
				desc = m_ChartClassDescriptorArray.at(i);
				break;
			}
		}
	}

	bool rv = AddChart(ChartFullPath, desc, NULL, 0, IsChartDirUsed(dir_name));

	// remove duplicates marked in AddChart()

	for (unsigned int i = 0; i < chartTable.GetCount(); i++) {
		if (!chartTable[i].GetbValid()) {
			chartTable.RemoveAt(i);
			i--; // entry is gone, recheck this index for next entry
		}
	}

	// Update the Entry index fields
	for (unsigned int i = 0; i < chartTable.GetCount(); i++)
		chartTable[i].SetEntryOffset(i);

	// Update (clone) the CDI array
	bool bcfound = false;
	ArrayOfCDI NewChartDirArray;

	ArrayOfCDI ChartDirArray = GetChartDirArray();
	for (unsigned int i = 0; i < ChartDirArray.size(); i++) {
		ChartDirInfo cdi = ChartDirArray.at(i);

		ChartDirInfo newcdi = cdi;

		// If entry is found that matches this cell, clear the magic number.
		if (newcdi.fullpath == dir_name) {
			newcdi.magic_number = _T("");
			bcfound = true;
		}

		NewChartDirArray.push_back(newcdi);
	}

	if (!bcfound) {
		ChartDirInfo cdi;
		cdi.fullpath = dir_name;
		cdi.magic_number = _T("");
		NewChartDirArray.push_back(cdi);
	}

	// Update the database master copy of the CDI array
	SetChartDirArray(NewChartDirArray);

	//  Update the list of chart dirs.
	m_chartDirs.Clear();

	for (unsigned int i = 0; i < GetChartDirArray().size(); i++) {
		ChartDirInfo cdi = GetChartDirArray().at(i);
		m_chartDirs.Add(cdi.fullpath);
	}

	return rv;
}

ChartTableEntry& ChartDatabase::GetWritableChartTableEntry(int index)
{
	if (index < static_cast<int>(chartTable.size()))
		return chartTable[index];
	else
		return m_ChartTableEntryDummy;
}

bool ChartDatabase::RemoveSingleChart(wxString& ChartFullPath)
{
	int rv = false;

	//  Walk the chart table, looking for the target
	for (unsigned int i = 0; i < chartTable.GetCount(); i++) {
		if (!strcmp(ChartFullPath.mb_str(), GetChartTableEntry(i).GetpFullPath())) {
			chartTable.RemoveAt(i);
			break;
		}
	}

	// Update the EntryOffset fields for the array
	for (unsigned int i = 0; i < chartTable.GetCount(); i++) {
		ChartTableEntry& pcte = GetWritableChartTableEntry(i);
		pcte.SetEntryOffset(i);
	}

	//  Check and update the dir array
	wxFileName fn(ChartFullPath);
	wxString fd = fn.GetPath();
	if (!IsChartDirUsed(fd)) {

		// Clone a new array, removing the unused directory,
		ArrayOfCDI NewChartDirArray;

		ArrayOfCDI ChartDirArray = GetChartDirArray();
		for (unsigned int i = 0; i < ChartDirArray.size(); i++) {
			ChartDirInfo cdi = ChartDirArray.at(i);

			ChartDirInfo newcdi = cdi;

			if (newcdi.fullpath != fd)
				NewChartDirArray.push_back(newcdi);
		}

		SetChartDirArray(NewChartDirArray);
	}

	//  Update the list of chart dirs.
	m_chartDirs.Clear();
	for (unsigned int i = 0; i < GetChartDirArray().size(); i++) {
		ChartDirInfo cdi = GetChartDirArray().at(i);
		m_chartDirs.Add(cdi.fullpath);
	}

	return rv;
}

// Create a Chart object
ChartBase* ChartDatabase::GetChart(const wxChar* WXUNUSED(theFilePath),
								   const chart::ChartClassDescriptor& WXUNUSED(chart_desc)) const
{
	// TODO: support non-UI chart factory
	return NULL;
}

// Create Chart Table entry by reading chart header info, etc.
ChartTableEntry* ChartDatabase::CreateChartTableEntry(const wxString& filePath,
													  const chart::ChartClassDescriptor& chart_desc)
{
	wxLogMessage(wxT("Loading chart data for ") + filePath);

	ChartBase* pch = GetChart(filePath, chart_desc);

	if (pch == NULL) {
		wxLogMessage(wxT("   ...creation failed for ") + filePath);
		return NULL;
	}

	InitReturn rc = pch->Init(filePath, HEADER_ONLY);
	if (rc != INIT_OK) {
		delete pch;
		wxLogMessage(wxT("   ...initialization failed for ") + filePath);
		return NULL;
	}

	ChartTableEntry* ret_val = new ChartTableEntry(*pch);
	ret_val->SetValid(true);

	delete pch;

	return ret_val;
}

bool ChartDatabase::GetCentroidOfLargestScaleChart(double* clat, double* clon,
												   chart::ChartFamilyEnum family) const
{
	int cur_max_i = -1;
	int cur_max_scale = 0;

	const int nEntry = chartTable.size();

	for (int i = 0; i < nEntry; ++i) {
		if (GetChartFamily(chartTable[i].GetChartType()) == family) {
			if (chartTable[i].GetScale() > cur_max_scale) {
				cur_max_scale = chartTable[i].GetScale();
				cur_max_i = i;
			}
		}
	}

	if (cur_max_i == -1)
		return false; // nothing found

	*clat = (chartTable[cur_max_i].GetLatMax() + chartTable[cur_max_i].GetLatMin()) / 2.0;
	*clon = (chartTable[cur_max_i].GetLonMin() + chartTable[cur_max_i].GetLonMax()) / 2.0;
	return true;
}

// Get DBChart Projection
int ChartDatabase::GetDBChartProj(int dbIndex) const
{
	if (bValid && (dbIndex >= 0) && (dbIndex < static_cast<int>(chartTable.size())))
		return chartTable[dbIndex].GetChartProjectionType();
	else
		return PROJECTION_UNKNOWN;
}

// Get DBChart Family
int ChartDatabase::GetDBChartFamily(int dbIndex) const
{
	if (bValid && (dbIndex >= 0) && (dbIndex < static_cast<int>(chartTable.size())))
		return chartTable[dbIndex].GetChartFamily();
	else
		return chart::CHART_FAMILY_UNKNOWN;
}

// Get DBChart FullFileName
wxString ChartDatabase::GetDBChartFileName(int dbIndex) const
{
	if (bValid && (dbIndex >= 0) && (dbIndex < static_cast<int>(chartTable.size())))
		return wxString(chartTable[dbIndex].GetpFullPath(), wxConvUTF8);
	else
		return _T("");
}

// Get DBChart Type
int ChartDatabase::GetDBChartType(int dbIndex) const
{
	if ((bValid) && (dbIndex >= 0) && (dbIndex < static_cast<int>(chartTable.size())))
		return chartTable[dbIndex].GetChartType();
	else
		return 0;
}

// Get DBChart Skew
float ChartDatabase::GetDBChartSkew(int dbIndex) const
{
	if ((bValid) && (dbIndex >= 0) && (dbIndex < static_cast<int>(chartTable.size())))
		return chartTable[dbIndex].GetChartSkew();
	else
		return 0.;
}

// Get DBChart Scale
int ChartDatabase::GetDBChartScale(int dbIndex) const
{
	if (bValid && (dbIndex >= 0) && (dbIndex < static_cast<int>(chartTable.size())))
		return chartTable[dbIndex].GetScale();
	else
		return 1;
}

// Get Lat/Lon Bounding Box from db
bool ChartDatabase::GetDBBoundingBox(int dbIndex, geo::BoundingBox* box) const
{
	if (bValid && (dbIndex >= 0) && (dbIndex < static_cast<int>(chartTable.size()))) {
		const ChartTableEntry& entry = GetChartTableEntry(dbIndex);
		box->SetMax(entry.GetLonMax(), entry.GetLatMax());
		box->SetMin(entry.GetLonMin(), entry.GetLatMin());
	}

	return true;
}

// Get PlyPoint from Database
int ChartDatabase::GetDBPlyPoint(int dbIndex, int plyindex, float* lat, float* lon) const
{
	if (bValid && (dbIndex >= 0) && (dbIndex < static_cast<int>(chartTable.size()))) {
		const ChartTableEntry& entry = GetChartTableEntry(dbIndex);
		if (entry.GetnPlyEntries()) {
			const float* fp = entry.GetpPlyTable();
			fp += plyindex * 2;
			if (lat)
				*lat = *fp;
			fp++;
			if (lon)
				*lon = *fp;
		}
		return entry.GetnPlyEntries();
	}
	return 0;
}

// Get AuxPlyPoint from Database
int ChartDatabase::GetDBAuxPlyPoint(int dbIndex, int plyindex, int ply, float* lat, float* lon) const
{
	if (bValid && (dbIndex >= 0) && (dbIndex < static_cast<int>(chartTable.size()))) {
		const ChartTableEntry& entry = GetChartTableEntry(dbIndex);
		if (entry.GetnAuxPlyEntries()) {
			const float* fp = entry.GetpAuxPlyTableEntry(ply);

			fp += plyindex * 2;
			if (lat)
				*lat = *fp;
			fp++;
			if (lon)
				*lon = *fp;
		}

		return entry.GetAuxCntTableEntry(ply);
	}
	return 0;
}

int ChartDatabase::GetnAuxPlyEntries(int dbIndex) const
{
	if (bValid && (dbIndex >= 0) && (dbIndex < static_cast<int>(chartTable.size()))) {
		const ChartTableEntry& entry = GetChartTableEntry(dbIndex);
		return entry.GetnAuxPlyEntries();
	}
	return 0;
}

void ChartDatabase::ApplyGroupArray(chart::ChartGroupArray* pGroupArray) // FIXME: nested depth
{
	for (unsigned int ic = 0; ic < chartTable.size(); ic++) {
		ChartTableEntry* pcte = &chartTable[ic];
		pcte->GetGroupArray().clear();

		wxString chart_full_path(pcte->GetpFullPath(), wxConvUTF8);

		for (unsigned int igroup = 0; igroup < pGroupArray->size(); igroup++) {
			chart::ChartGroup* pGroup = pGroupArray->at(igroup);
			for (unsigned int j = 0; j < pGroup->m_element_array.size(); j++) {
				wxString element_root = pGroup->m_element_array.at(j)->m_element_name;
				if (chart_full_path.StartsWith(element_root)) {
					bool b_add = true;
					for (unsigned int k = 0;
						 k < pGroup->m_element_array.at(j)->missing_names.size(); k++) {
						wxString missing_item
							= pGroup->m_element_array.at(j)->missing_names.at(k);
						if (chart_full_path.StartsWith(missing_item)) {
							if (chart_full_path == missing_item) {
								// missing item is full chart name
								b_add = false;
								break;
							} else {
								if (wxDir::Exists(missing_item)) { // missing item is a dir
									b_add = false;
									break;
								}
							}
						}
					}

					if (b_add)
						pcte->GetGroupArray().push_back(igroup + 1);
				}
			}
		}
	}
}

}

