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

#ifndef __TIDE__TCDS_BINARY_HARMONIC_H__
#define __TIDE__TCDS_BINARY_HARMONIC_H__

#include <wx/string.h>

#include <vector>

#include <tide/TCDataFactory.h>
#include <tide/Station_Data.h>

namespace tide {

class IDX_entry;

class TCDS_Binary_Harmonic : public TCDataFactory
{
public:
	TCDS_Binary_Harmonic();
	virtual ~TCDS_Binary_Harmonic();

	virtual TC_Error_Code LoadData(const wxString& data_file_path);
	virtual int GetMaxIndex(void) const;
	virtual IDX_entry* GetIndexEntry(int n_index);
	virtual TC_Error_Code LoadHarmonicData(IDX_entry* pIDX);

private:
	std::vector<Station_Data> m_msd_array;

	wxString m_last_reference_not_found;

	std::vector<IDX_entry*> m_IDX_array;

	int num_IDX;
	int num_nodes;
	int num_csts;
	int num_epochs;
	std::vector<double> m_cst_speeds;
	std::vector<std::vector<double> > m_cst_nodes;
	std::vector<std::vector<double> > m_cst_epochs;
	std::vector<double> m_work_buffer;
	int m_first_year;
};

}

#endif
