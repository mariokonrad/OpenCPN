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

#ifndef __PRINT__TABLE__H__
#define __PRINT__TABLE__H__

#include <vector>
#include <string>
#include <ostream>
#include <wx/string.h>

namespace print {

/// \brief
/// Enumeration is used to notice the state of the table.
///
/// Different states are used to signalize different semanic of the data in
/// the operator << of the class Table.
/// If the state is "setup columns widths" -> then the data is used to store
/// the width of the columns.
/// If the state is "fill with data" -> then the data is the cell content.
enum TableState
{
	TABLE_SETUP_WIDTHS = 0,
	TABLE_FILL_DATA,
	TABLE_FILL_HEADER
};


/// \brief Represents a NxM simple table with captions.
///
/// Input operator is "<<"
/// Number of columns and rows are given dynamically by the input data.
/// Captions are given by first input line.
/// Every cell is given column by column.
/// Next row is given by "<< '\n'" (or << endl)
class Table
{
public:
	typedef std::vector<wxString> Row;
	typedef std::vector<Row> Data;

protected:
	int nrows;
	int ncols;

	bool create_next_row;

	Data data;
	std::vector<double> widths;
	Row header;
	TableState state;

	void Start();
	void NewRow();

public:
	Table();
	~Table();

	Table& operator<<(const std::string&);
	Table& operator<<(const int&);
	Table& operator<<(const double&);

	const Data& GetData() const;
	void StartFillData();
	void StartFillHeader();
	void StartFillWidths();
	int GetRowHeight(int i) const;

	friend std::ostream& operator<<(std::ostream&, const Table&);
};

}

#endif
