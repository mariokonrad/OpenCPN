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

#ifndef __TIDE__TCDS_ASCII_HARMONIC_H__
#define __TIDE__TCDS_ASCII_HARMONIC_H__

#include <wx/string.h>

#include <vector>
#include <cstdio>

#include <tide/TCDataFactory.h>
#include <tide/Station_Data.h>

namespace tide {

class IDX_entry;

class TCDS_Ascii_Harmonic : public TCDataFactory
{
public:
	TCDS_Ascii_Harmonic();
	virtual ~TCDS_Ascii_Harmonic();

	virtual TC_Error_Code LoadData(const wxString& data_file_path);
	virtual int GetMaxIndex(void) const;
	virtual IDX_entry* GetIndexEntry(int n_index);
	virtual TC_Error_Code LoadHarmonicData(IDX_entry* pIDX);

private:
	class AbbrEntry
	{
	public:
		int type;
		wxString short_s;
		wxString long_s;
	};

	long IndexFileIO(int func, long value);
	TC_Error_Code init_index_file();
	TC_Error_Code build_IDX_entry(IDX_entry* pIDX);
	TC_Error_Code LoadHarmonicConstants(const wxString& data_file_path);
	int read_next_line(FILE* fp, char* linrec, int end_ok);
	int skipnl(FILE* fp);
	char* nojunk(char* line);
	int slackcmp(const char* a, const char* b);

	void free_nodes();
	void free_epochs();
	void free_data();

	std::vector<Station_Data> m_msd_array;

	wxString m_indexfile_name;
	wxString m_harmfile_name;
	wxString m_last_reference_not_found;

	char index_line_buffer[1024];
	FILE* m_IndexFile;
	std::vector<IDX_entry*> m_IDX_array;

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
