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

static const char sig_version[] = "COVR1002";

static void appendOSDirSep(wxString& s) // FIXME: code duplication
{
	wxChar sep = wxFileName::GetPathSeparator();
	if (s.Last() != sep)
		s.Append(sep);
}

covr_set::covr_set(cm93chart * parent)
{
	m_pParent = parent;
}

covr_set::~covr_set()
{
	// Create/Update the cache
	if (m_cachefile.IsEmpty())
		return; // presumably for Z scale charts
	// for which we create no cache

	if (m_covr_array_outlines.size()) {
		wxFFileOutputStream ofs(m_cachefile);
		if (ofs.IsOk()) {
			ofs.Write(sig_version, 8); // write signature

			for (unsigned int i = 0; i < m_covr_array_outlines.size(); i++) {
				int wkbsize = m_covr_array_outlines[i].GetWKBSize();
				if (wkbsize) {
					char* p = (char*)malloc(wkbsize * sizeof(char));
					m_covr_array_outlines[i].WriteWKB(p);
					ofs.Write(p, wkbsize);
					free(p);
				}
			}
			ofs.Close();
		}
	}
}

int covr_set::get_scale(wxChar scale_char) const
{
	switch (scale_char) {
		case 'Z': return 20000000;
		case 'A': return  3000000;
		case 'B': return  1000000;
		case 'C': return   200000;
		case 'D': return   100000;
		case 'E': return    50000;
		case 'F': return    20000;
		case 'G': return     7500;
		default:  return 20000000;
	}
}

bool covr_set::Init(wxChar scale_char, const wxString& prefix)
{
	m_scale_char = scale_char;
	m_scale = get_scale(scale_char);

	// Create the cache file name
	wxString prefix_string = prefix;
	wxString sep(wxFileName::GetPathSeparator());
	prefix_string.Replace(sep, _T("_"));
	prefix_string.Replace(_T(":"), _T("_")); // for Windows

	m_cachefile = global::OCPN::get().sys().data().private_data_dir;
	appendOSDirSep(m_cachefile);

	m_cachefile += _T("cm93");
	appendOSDirSep(m_cachefile);

	m_cachefile += prefix_string; // include the cm93 prefix string in the cache file name
	m_cachefile += _T("_"); // to support multiple cm93 data sets

	wxString cache_old_name = m_cachefile;
	cache_old_name += _T("coverset.");
	cache_old_name += m_scale_char;

	m_cachefile += _T("coverset_sig.");
	m_cachefile += m_scale_char;

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
		char sig_bytes[9];
		// Validate the file signature
		if (!ifs.Read(&sig_bytes, 8).Eof()) {
			if (strncmp(sig_bytes, sig_version, 8)) {
				return false; // bad signature match
			}
		} else
			return false; // short file

		bool b_cont = true;
		while (b_cont) {
			M_COVR_Desc* pmcd = new M_COVR_Desc;
			int length = pmcd->ReadWKB(ifs);

			if (length) {
				m_covr_array_outlines.Add(pmcd);

				if (m_cell_hash.find(pmcd->m_cell_index) == m_cell_hash.end())
					m_cell_hash[pmcd->m_cell_index] = 0; // initialize the element

				m_cell_hash[pmcd->m_cell_index]++; // add this M_COVR to the hash map

			} else {
				delete pmcd;
				b_cont = false;
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
		M_COVR_Desc* pmcd_candidate = &m_covr_array_outlines.Item(i);
		if ((pmcd_candidate->m_cell_index == pmcd->m_cell_index)
			&& (pmcd_candidate->m_object_id == pmcd->m_object_id)
			&& (pmcd_candidate->m_subcell == pmcd->m_subcell)) {
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
		M_COVR_Desc* pmcd_candidate = &m_covr_array_outlines.Item(i);
		if ((pmcd_candidate->m_cell_index == pmcd->m_cell_index)
			&& (pmcd_candidate->m_object_id == pmcd->m_object_id)
			&& (pmcd_candidate->m_subcell == pmcd->m_subcell)) {
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
		M_COVR_Desc* pmcd_candidate = &m_covr_array_outlines.Item(i);
		if ((pmcd_candidate->m_cell_index == cell_index)
			&& (pmcd_candidate->m_object_id == object_id) && (pmcd_candidate->m_subcell == subcell))

			return pmcd_candidate;
	}

	return NULL;
}

