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

#ifndef __ROUTEPRINTOUT__H__
#define __ROUTEPRINTOUT__H__

#include <print/PrintTable.h>
#include <MyPrintout.h>

class Route;

class RoutePrintout : public MyPrintout
{
public:
	RoutePrintout(std::vector<bool> _toPrintOut, Route* route,
				  const wxChar* title = _T("My Route printout"));

	virtual bool OnPrintPage(int page);
	virtual void OnPreparePrinting();
	void DrawPage(wxDC* dc);

	virtual bool HasPage(int num) const;
	virtual void GetPageInfo( // FIXME: bad interface of method
		int* minPage, int* maxPage, int* selPageFrom, int* selPageTo);

protected:
	static const int pN = 5; // number of fields sofar

	wxDC* myDC;
	print::PrintTable table;
	Route* myRoute;
	std::vector<bool> toPrintOut; // list of fields of bool, if certain element should be print out.
	int pageToPrint;
	int numberOfPages;
	int marginX;
	int marginY;
	int textOffsetX;
	int textOffsetY;
};

#endif
