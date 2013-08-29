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

#ifndef __CHART__FINDCM93DICTIONARY__H__
#define __CHART__FINDCM93DICTIONARY__H__

// Case-insensitive cm93 directory tree depth-first traversal to find the dictionary...
// This could be made simpler, but matches the old code better as is
class FindCM93Dictionary : public wxDirTraverser
{
	public:
		FindCM93Dictionary(wxString& path)
			: m_path (path)
		{}

		virtual wxDirTraverseResult OnFile ( const wxString& filename )
		{
			wxString name = filename.AfterLast ( wxFileName::GetPathSeparator() ).Lower();
			if ( name == wxT ( "cm93obj.dic" ) )
			{
				m_path = filename;
				return wxDIR_STOP;
			}

			return wxDIR_CONTINUE;
		}

		virtual wxDirTraverseResult OnDir ( const wxString& WXUNUSED ( dirname ) )
		{
			return wxDIR_CONTINUE;
		}

	private:
		wxString & m_path;
};

#endif
