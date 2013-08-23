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

#include "ChartTableHeader.h"
#include <wx/stream.h>
#include <wx/log.h>

static const int DB_VERSION_PREVIOUS = 16;
static const int DB_VERSION_CURRENT = 17;

void ChartTableHeader::Read(wxInputStream &is)
{
	is.Read(this, sizeof(ChartTableHeader));
}

void ChartTableHeader::Write(wxOutputStream &os)
{
	char vb[5];
	sprintf(vb, "V%03d", DB_VERSION_CURRENT);

	memcpy(dbVersion, vb, 4);
	os.Write(this, sizeof(ChartTableHeader));
}

bool ChartTableHeader::CheckValid()
{
	char vb[5];
	sprintf(vb, "V%03d", DB_VERSION_CURRENT);
	if (strncmp(vb, dbVersion, sizeof(dbVersion)))
	{
		wxString msg;
		char vbo[5];
		memcpy(vbo, dbVersion, 4);
		vbo[4] = 0;
		msg.Append(wxString(vbo, wxConvUTF8));
		msg.Prepend(wxT("   Warning: found incorrect chart db version: "));
		wxLogMessage(msg);

		return false;       // no match....

		/*
		// Try previous version....
		sprintf(vb, "V%03d", DB_VERSION_PREVIOUS);
		if (strncmp(vb, dbVersion, sizeof(dbVersion)))
		return false;
		else
		{
		wxLogMessage(_T("   Scheduling db upgrade to current db version on Options->Charts page visit..."));
		return true;
		}
		 */

	}
	else
	{
		wxString msg;
		char vbo[5];
		memcpy(vbo, dbVersion, 4);
		vbo[4] = 0;
		msg.Append(wxString(vbo, wxConvUTF8));
		msg.Prepend(wxT("Loading chart db version: "));
		wxLogMessage(msg);
	}

	return true;
}

ChartTableHeader::ChartTableHeader()
{}

ChartTableHeader::ChartTableHeader(int dirEntries, int tableEntries)
	: nTableEntries(tableEntries)
	, nDirEntries(dirEntries)
{}

int ChartTableHeader::GetDirEntries() const
{ return nDirEntries; }

int ChartTableHeader::GetTableEntries() const
{ return nTableEntries; }

char *ChartTableHeader::GetDBVersionString()
{ return dbVersion; }

