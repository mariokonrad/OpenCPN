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

#ifndef __PRINTTABLE_H__
#define __PRINTTABLE_H__

#include <Table.h>
#include <PrintCell.h>

#include <iostream>
#include <vector>

#ifdef __WXMSW__
	#include <wx/msw/private.h>
#endif


///\brief Extension of a class Table with printing into dc.
///
/// It takes all elements, takes DC as a printing device, takes  a maximal
/// possible table width,  calculate width of every column.
/// For printing of every cell it modifies its content so, that it fits into cell by inserting
/// new lines.
class PrintTable : public Table
{
	public:
		typedef std::vector<PrintCell> ContentRow;
		typedef std::vector<ContentRow> Content;
	protected:
		Content contents;
		ContentRow header_content;
		std::vector<int> rows_heights;
		int header_height;
		int number_of_pages; // stores the number of pages for printing of this table. It is set by AdjustCells

	public:
		PrintTable();

		// creates internally vector of PrintCell's, to calculate columns widths and row sizes
		void AdjustCells(wxDC * _dc, int marginX, int marginY);

		const Content & GetContent() const;
		const ContentRow & GetHeader() const;
		int GetNumberPages() const;
		int GetHeaderHeight() const;
};

#endif
