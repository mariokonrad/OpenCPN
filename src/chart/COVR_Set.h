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

#ifndef __CHART__COVR_SET__H__
#define __CHART__COVR_SET__H__

#include <wx/hashmap.h>
#include <vector>

WX_DECLARE_HASH_MAP(int, int, wxIntegerHash, wxIntegerEqual, cm93cell_hash); // FIXME: use std::unordered_map<int,int>

class cm93chart;
class M_COVR_Desc;

/// This is a helper class which holds all the known information
/// relating to cm93 cell MCOVR objects of a particular scale
class covr_set
{
public:
	covr_set(cm93chart* parent);
	~covr_set();

	bool Init(wxChar scale_char, const wxString& prefix);

	unsigned int GetCoverCount() const;
	M_COVR_Desc* GetCover(unsigned int im);
	void Add_MCD(M_COVR_Desc* pmcd);
	bool Add_Update_MCD(M_COVR_Desc* pmcd);
	bool IsCovrLoaded(int cell_index);
	M_COVR_Desc* Find_MCD(int cell_index, int object_id, int sbcell);

private:
	void write_cachefile();
	wxString cache_name(wxString& name, wxString& old_name, wxChar scale_char,
						const wxString& prefix) const;

	cm93chart* m_pParent;
	wxString m_cachefile;

	typedef std::vector<M_COVR_Desc*> Outlines;
	Outlines outlines; // chart outline rendering, has ownership over stored objects

	// This is a hash, indexed by cell index, elements contain the number of M_COVRs
	// found on this particular cell
	cm93cell_hash m_cell_hash;
};

#endif
