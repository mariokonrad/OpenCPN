/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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

#include "Table.h"
#include <sstream>

namespace print {

Table::Table()
	: nrows(0)
	, ncols(0)
	, create_next_row(true)
	, state(TABLE_SETUP_WIDTHS)
{
	data.clear();
}

Table::~Table()
{
	for (Data::iterator i = data.begin(); i != data.end(); ++i) {
		i->clear();
	}
	data.clear();
}

void Table::Start()
{
	if (create_next_row) {
		NewRow();
		create_next_row = false;
	}
}

void Table::NewRow()
{
	data.push_back(Row());
}

Table& Table::operator<<(const double& cellcontent)
{
	if (state == TABLE_SETUP_WIDTHS) {
		widths.push_back(cellcontent);
		return *this;
	}
	if (state == TABLE_FILL_DATA) {
		std::stringstream sstr;
		sstr << cellcontent;
		std::string _cellcontent = sstr.str();
		Start();
		wxString _str(_cellcontent.c_str(), wxConvUTF8);
		data[data.size() - 1].push_back(_str);
	}
	return *this;
}

Table& Table::operator<<(const std::string& cellcontent)
{
	Start();
	if (state == TABLE_FILL_HEADER) { // if we start to fill with string data, we change state
									  // automatically.
		wxString _str(cellcontent.c_str(), wxConvUTF8);
		header.push_back(_str);
		return *this;
	}
	if (state == TABLE_SETUP_WIDTHS) { // if we start to fill with string data, we change state
									   // automatically.
		state = TABLE_FILL_DATA;
	}

	if ((cellcontent.compare("\n") == 0)) {
		create_next_row = true;
		return *this;
	}
	wxString _str(cellcontent.c_str(), wxConvUTF8);
	data[data.size() - 1].push_back(_str);
	return *this;
}

Table& Table::operator<<(const int& cellcontent)
{
	using namespace std;

	if (state == TABLE_SETUP_WIDTHS) {
		widths.push_back((double)cellcontent);
		return *this;
	}
	if (state == TABLE_FILL_DATA) {
		stringstream sstr;
		sstr << cellcontent;
		string _cellcontent = sstr.str();
		Start();
		wxString _str(_cellcontent.c_str(), wxConvUTF8);
		data[data.size() - 1].push_back(_str);
	}
	return *this;
}

const Table::Data& Table::GetData() const
{
	return data;
}

void Table::StartFillData()
{
	state = TABLE_FILL_DATA;
}

void Table::StartFillHeader()
{
	state = TABLE_FILL_HEADER;
}

void Table::StartFillWidths()
{
	state = TABLE_SETUP_WIDTHS;
}

int Table::GetRowHeight(int i) const
{
	return widths[i];
}

std::ostream& operator<<(std::ostream& out, const Table& table)
{
	const Table::Data& data = table.GetData();

	for (Table::Data::const_iterator i = data.begin(); i != data.end(); ++i) {
		for (Table::Row::const_iterator row = i->begin(); row != i->end(); ++row) {
			out << row->fn_str() << " ";
		}
		out << std::endl;
	}
	return out;
}

}

