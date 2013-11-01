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

#include "COVR_Set.h"
#include <global/OCPN.h> // FIXME: this class should not have the need to access global data
#include <global/System.h>
#include <wx/wfstream.h>
#include <wx/filename.h>

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(Array_Of_M_COVR_Desc); // FIXME

static const char SIG_VERSION[] = "COVR1002";

static void appendOSDirSep(wxString& s) // FIXME: code duplication
{
	wxChar sep = wxFileName::GetPathSeparator();
	if (s.Last() != sep)
		s.Append(sep);
}

covr_set::covr_set(cm93chart* parent)
{
	m_pParent = parent;
}

covr_set::~covr_set()
{
	write_cachefile();
}

void covr_set::write_cachefile()
{
	// Create/Update the cache
	if (m_cachefile.IsEmpty())
		return; // presumably for Z scale charts for which we create no cache

	if (m_covr_array_outlines.size() == 0)
		return;

	wxFFileOutputStream ofs(m_cachefile);
	if (!ofs.IsOk())
		return;

	ofs.Write(SIG_VERSION, sizeof(SIG_VERSION)); // write signature

	for (unsigned int i = 0; i < m_covr_array_outlines.size(); i++) {
		int wkbsize = m_covr_array_outlines[i].GetWKBSize();
		if (wkbsize) {
			char* p = new char[wkbsize];
			m_covr_array_outlines[i].WriteWKB(p);
			ofs.Write(p, wkbsize);
			delete [] p;
		}
	}
	ofs.Close();
}

wxString covr_set::cache_name(wxString& name, wxString& old_name, wxChar scale_char,
							  const wxString& prefix) const
{
	// Create the cache file name
	wxString prefix_string = prefix;
	wxString sep(wxFileName::GetPathSeparator());
	prefix_string.Replace(sep, _T("_"));
	prefix_string.Replace(_T(":"), _T("_")); // for Windows

	name = global::OCPN::get().sys().data().private_data_dir;
	appendOSDirSep(name);

	name += _T("cm93");
	appendOSDirSep(name);

	name += prefix_string; // include the cm93 prefix string in the cache file name
	name += _T("_"); // to support multiple cm93 data sets

	old_name = name;
	old_name += _T("coverset.");
	old_name += scale_char;

	name += _T("coverset_sig.");
	name += scale_char;

	return name;
}

bool covr_set::Init(wxChar scale_char, const wxString& prefix)
{
	wxString cache_old_name;
	cache_name(m_cachefile, cache_old_name, scale_char, prefix);

	wxFileName fn(m_cachefile);
	if (!fn.DirExists())
		wxFileName::Mkdir(fn.GetPath(), 0777, wxPATH_MKDIR_FULL);

	// Preload the cache
	if (!wxFileName::FileExists(m_cachefile)) {
		// The signed file does not exist
		// Check for an old style file, and delete if found.
		if (wxFileName::FileExists(cache_old_name))
			::wxRemoveFile(cache_old_name);
		return false;
	}

	wxFFileInputStream ifs(m_cachefile);
	if (ifs.IsOk()) {
		char sig_bytes[sizeof(SIG_VERSION) + 1];
		// Validate the file signature
		if (!ifs.Read(&sig_bytes, sizeof(SIG_VERSION)).Eof()) {
			if (strncmp(sig_bytes, SIG_VERSION, sizeof(SIG_VERSION))) {
				return false; // bad signature match
			}
		} else
			return false; // short file

		while (true) {
			M_COVR_Desc* pmcd = new M_COVR_Desc;
			int length = pmcd->ReadWKB(ifs);

			if (length) {
				m_covr_array_outlines.Add(pmcd);

				if (m_cell_hash.find(pmcd->m_cell_index) == m_cell_hash.end())
					m_cell_hash[pmcd->m_cell_index] = 0; // initialize the element

				m_cell_hash[pmcd->m_cell_index]++; // add this M_COVR to the hash map

			} else {
				delete pmcd;
				break;
			}
		}
	}

	return true;
}

unsigned int covr_set::GetCoverCount() const
{
	return m_covr_array_outlines.size();
}

M_COVR_Desc* covr_set::GetCover(unsigned int im)
{
	return &m_covr_array_outlines.Item(im);
}

void covr_set::Add_MCD(M_COVR_Desc* pmcd)
{
	m_covr_array_outlines.Add(pmcd);

	if (m_cell_hash.find(pmcd->m_cell_index) == m_cell_hash.end()) // not present yet?
		m_cell_hash[pmcd->m_cell_index] = 0; // initialize

	m_cell_hash[pmcd->m_cell_index]++; // add this M_COVR to the hash map
}

bool covr_set::IsCovrLoaded(int cell_index)
{
	return (m_cell_hash.find(cell_index) != m_cell_hash.end());
}

bool covr_set::Add_Update_MCD(M_COVR_Desc* pmcd)
{
	if (m_cell_hash.find(pmcd->m_cell_index) == m_cell_hash.end()) {
		m_covr_array_outlines.Add(pmcd);
		m_cell_hash[pmcd->m_cell_index] = 1; // initialize
		return true;
	}

	// There is at least one MCD already in place for this cell index
	// We need to search the entire table to see if any of those MCD's
	// correspond to this MCD's object identifier and subcell, as well as cell index
	bool b_found = false;
	for (unsigned int i = 0; i < m_covr_array_outlines.size(); i++) {
		M_COVR_Desc* candidate = &m_covr_array_outlines.Item(i);
		if ((candidate->m_cell_index == pmcd->m_cell_index)
			&& (candidate->m_object_id == pmcd->m_object_id)
			&& (candidate->m_subcell == pmcd->m_subcell)) {
			b_found = true;
			break;
		}
	}

	if (!b_found) {
		m_covr_array_outlines.Add(pmcd);
		m_cell_hash[pmcd->m_cell_index]++; // add this M_COVR to the hash map
		return true;
	}
	return false;
}

int covr_set::Find_MCD(M_COVR_Desc* pmcd)
{
	if (m_cell_hash.find(pmcd->m_cell_index) == m_cell_hash.end())
		return -1;

	// There is at least one MCD already in place for this cell index
	// We need to search the entire table to see if any of those MCD's
	// correspond to this MCD's object identifier as well as cell index
	for (unsigned int i = 0; i < m_covr_array_outlines.size(); i++) {
		M_COVR_Desc* candidate = &m_covr_array_outlines.Item(i);
		if ((candidate->m_cell_index == pmcd->m_cell_index)
			&& (candidate->m_object_id == pmcd->m_object_id)
			&& (candidate->m_subcell == pmcd->m_subcell)) {
			return (int)i;
		}
	}

	return -1;
}

M_COVR_Desc* covr_set::Find_MCD(int cell_index, int object_id, int subcell)
{
	if (m_cell_hash.find(cell_index) == m_cell_hash.end()) // not present?
		return NULL;

	for (unsigned int i = 0; i < m_covr_array_outlines.size(); i++) {
		M_COVR_Desc* candidate = &m_covr_array_outlines.Item(i);
		if ((candidate->m_cell_index == cell_index) && (candidate->m_object_id == object_id)
			&& (candidate->m_subcell == subcell))

			return candidate;
	}

	return NULL;
}

