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

#include "CM93Manager.h"
#include <chart/CM93Dictionary.h>
#include <wx/log.h>
#include <wx/filename.h>


cm93manager::cm93manager ( void )
{
	m_pcm93Dict = NULL;

	m_bfoundA = false;
	m_bfoundB = false;
	m_bfoundC = false;
	m_bfoundD = false;
	m_bfoundE = false;
	m_bfoundF = false;
	m_bfoundG = false;
	m_bfoundZ = false;
}

cm93manager::~cm93manager ( void )
{
	delete m_pcm93Dict;
}

bool cm93manager::Loadcm93Dictionary(const wxString & name)
{

	//  Find and load cm93_dictionary
	if ( !m_pcm93Dict )
	{
		m_pcm93Dict = FindAndLoadDict ( name );

		if ( !m_pcm93Dict )
		{
			wxLogMessage ( _T ( "   Cannot load CM93 Dictionary." ) );
			return false;
		}


		if ( !m_pcm93Dict->IsOk() )
		{
			wxLogMessage ( _T ( "   Error in loading CM93 Dictionary." ) );
			delete m_pcm93Dict;
			m_pcm93Dict = NULL;
			return false;;
		}
	}
	else if ( !m_pcm93Dict->IsOk() )
	{
		wxLogMessage ( _T ( "   CM93 Dictionary is not OK." ) );
		return false;
	}

	return true;
}

cm93_dictionary *cm93manager::FindAndLoadDict ( const wxString &file )
{
	cm93_dictionary *retval = NULL;
	cm93_dictionary *pdict = new cm93_dictionary();

	//    Search for the dictionary files all along the path of the passed parameter filename

	wxFileName fn ( file );
	wxString path = fn.GetPath ( ( int ) ( wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME ) );
	wxString target;
	unsigned int i = 0;

	while ( i < path.Len() )
	{
		target.Append ( path[i] );
		if ( path[i] == fn.GetPathSeparator() )
		{
			if ( pdict->LoadDictionary ( target ) )
			{
				retval = pdict;
				break;
			}
			if ( pdict->LoadDictionary ( target + _T ( "CM93ATTR" ) ) )
			{
				retval = pdict;
				break;
			}
		}
		i++;
	}

	char t[100];
	strncpy ( t, target.mb_str(), 99 );

	if ( retval == NULL )
		delete pdict;

	return retval;
}

