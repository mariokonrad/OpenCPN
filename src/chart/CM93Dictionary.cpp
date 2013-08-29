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

#include "CM93Dictionary.h"
#include <wx/filename.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/wfstream.h>
#include <wx/log.h>

cm93_dictionary::cm93_dictionary()
{
	m_S57ClassArray   = NULL;
	m_AttrArray       = NULL;
	m_GeomTypeArray   = NULL;;
	m_ValTypeArray    = NULL;
	m_max_class       = 0;
	m_ok = false;

}

bool cm93_dictionary::IsOk(void) const
{
	return m_ok;
}

wxString cm93_dictionary::GetDictDir(void) const
{
	return m_dict_dir;
}

bool cm93_dictionary::LoadDictionary(const wxString & dictionary_dir)
{
	int i, nline;
	wxString line;
	wxString dir ( dictionary_dir );    // a copy
	bool  ret_val = false;

	wxChar sep = wxFileName::GetPathSeparator();
	if ( dir.Last() != sep )
		dir.Append ( sep );

	m_dict_dir = dir;


	//    Build some array strings for Feature decoding

	wxString sf ( dir );
	sf.Append ( _T ( "CM93OBJ.DIC" ) );

	if ( !wxFileName::FileExists ( sf ) )
	{
		sf = dir;
		sf.Append ( _T ( "cm93obj.dic" ) );
		if ( !wxFileName::FileExists ( sf ) )
			return false;
	}

	wxTextFile file;
	if ( !file.Open ( sf ) )
		return false;


	nline = file.GetLineCount();

	if ( !nline )
		return false;

	//    Read the file once to get the max class number
	int iclass_max = 0;

	for ( i=0 ; i<nline ; i++ )
	{
		line = file.GetLine ( i );

		wxStringTokenizer tkz ( line, wxT ( "|" ) );
		//            while ( tkz.HasMoreTokens() )
		{
			//  6 char class name
			wxString class_name = tkz.GetNextToken();

			//  class number, ascii
			wxString token = tkz.GetNextToken();
			long liclass;
			token.ToLong ( &liclass );
			int iclass = liclass;
			if ( iclass > iclass_max )
				iclass_max = iclass;

			//  geom type, ascii
			wxString geo_type = tkz.GetNextToken();

		}

	}

	m_max_class = iclass_max;

	//    Create the class name array
	m_S57ClassArray = new wxArrayString;
	m_S57ClassArray->Add ( _T ( "NULLNM" ), iclass_max+1 );

	//    And an array of ints describing the geometry type per class
	m_GeomTypeArray = ( int * ) malloc ( ( iclass_max + 1 ) * sizeof ( int ) );

	//    Iterate over the file, filling in the values
	for ( i=0 ; i<nline ; i++ )
	{
		line = file.GetLine ( i );

		wxStringTokenizer tkz ( line, wxT ( "|" ) );
		//           while ( tkz.HasMoreTokens() )
		{
			//  6 char class name
			wxString class_name = tkz.GetNextToken();

			//  class number, ascii
			wxString token = tkz.GetNextToken();
			long liclass;
			token.ToLong ( &liclass );
			int iclass = liclass;

			//  geom type, ascii
			wxString geo_type = tkz.GetNextToken();

			m_S57ClassArray->Insert ( class_name, iclass );
			m_S57ClassArray->RemoveAt ( iclass + 1 );

			int igeom_type = -1;                            // default unknown
			wxChar geo_type_primary = geo_type[0];

			if ( geo_type_primary == 'A' )
				igeom_type = 3;
			else if ( geo_type_primary == 'L' )
				igeom_type = 2;
			else if ( geo_type_primary == 'P' )
				igeom_type = 1;

			//    Note:  there are other types in the file, e.g. 'C'.  Dunno what this is
			//    Also, some object classes want multiple geometries, like PA, PLA, etc.
			//    Take only primary, ignore the rest

			m_GeomTypeArray[iclass] = igeom_type;

		}
	}
	file.Close();

	//    Build some array strings for Attribute decoding

	wxString sfa ( dir );
	sfa.Append ( _T ( "ATTRLUT.DIC" ) );

	if ( !wxFileName::FileExists ( sfa ) )
	{
		sfa = dir;
		sfa.Append ( _T ( "attrlut.dic" ) );
	}

	if ( wxFileName::FileExists ( sfa ) )
	{
		wxFileInputStream filea ( sfa );

		if ( filea.IsOk() )
		{
			//    Read the file once to get the max attr number
			int iattr_max = 0;


			while ( !filea.Eof() )
			{

				//read a line
				line.Empty();
				while ( 1 )
				{
					char a = filea.GetC();
					if ( filea.Eof() )
						break;
					line.Append ( a );
					if ( a == 0x0a )
						break;
				}


				if ( !line.StartsWith ( ( const wxChar * ) wxT ( ";" ) ) )
				{
					wxStringTokenizer tkz ( line, wxT ( "|" ) );
					{
						//  6 attribute label
						wxString class_name = tkz.GetNextToken();

						//  attribute number, ascii
						wxString token = tkz.GetNextToken();
						long liattr;
						token.ToLong ( &liattr );
						int iattr = liattr;
						if ( iattr > iattr_max )
							iattr_max = iattr;
					}
				}
			}

			m_max_attr = iattr_max;

			filea.SeekI ( 0 );


			//    Create the attribute label array

			m_AttrArray = new wxArrayString;
			m_AttrArray->Add ( _T ( "NULLNM" ), iattr_max+1 );

			//    And an array of chars describing the attribute value type
			m_ValTypeArray = ( char * ) malloc ( ( iattr_max + 1 ) * sizeof ( char ) );


			//    Iterate over the file, filling in the values
			while ( !filea.Eof() )
			{
				//read a line
				line.Empty();
				while ( 1 )
				{
					char a = filea.GetC();
					if ( filea.Eof() )
						break;
					line.Append ( a );
					if ( a == 0x0a )
						break;
				}


				if ( !line.StartsWith ( ( const wxChar * ) wxT ( ";" ) ) )
				{
					wxStringTokenizer tkz ( line, wxT ( "|" ) );
					{
						//  6 char class name
						wxString attr_name = tkz.GetNextToken();

						//  class number, ascii
						wxString token = tkz.GetNextToken();
						long liattr;
						token.ToLong ( &liattr );
						int iattr = liattr;

						m_AttrArray->Insert ( attr_name, iattr );
						m_AttrArray->RemoveAt ( iattr + 1 );

						//    Skip some
						token = tkz.GetNextToken();
						token = tkz.GetNextToken();
						token = tkz.GetNextToken();
						token = tkz.GetNextToken().Trim();

						char atype = '?';
						if ( token.IsSameAs ( _T ( "aFLOAT" ) ) )
							atype = 'R';
						else if ( token.IsSameAs ( _T ( "aBYTE" ) ) )
							atype = 'B';
						else if ( token.IsSameAs ( _T ( "aSTRING" ) ) )
							atype = 'S';
						else if ( token.IsSameAs ( _T ( "aCMPLX" ) ) )
							atype = 'C';
						else if ( token.IsSameAs ( _T ( "aLIST" ) ) )
							atype = 'L';
						else if ( token.IsSameAs ( _T ( "aWORD10" ) ) )
							atype = 'W';
						else if ( token.IsSameAs ( _T ( "aLONG" ) ) )
							atype = 'G';

						m_ValTypeArray[iattr] = atype;

					}
				}
			}
			ret_val = true;
		}
		else              // stream IsOK
		{
			ret_val = false;
		}
	}


	else                    //    Look for alternate file
	{
		sfa = dir;
		sfa.Append ( _T ( "CM93ATTR.DIC" ) );

		if ( !wxFileName::FileExists ( sfa ) )
		{
			sfa = dir;
			sfa.Append ( _T ( "cm93attr.dic" ) );
		}

		if ( wxFileName::FileExists ( sfa ) )
		{
			wxFileInputStream filea ( sfa );

			if ( filea.IsOk() )
			{
				//    Read the file once to get the max attr number
				int iattr_max = 0;


				while ( !filea.Eof() )
				{

					//read a line
					line.Empty();
					while ( 1 )
					{
						char a = filea.GetC();
						if ( filea.Eof() )
							break;
						line.Append ( a );
						if ( a == 0x0a )
							break;
					}


					if ( !line.StartsWith ( ( const wxChar * ) wxT ( ";" ) ) )
					{
						wxStringTokenizer tkz ( line, wxT ( "|" ) );
						if ( tkz.CountTokens() )
						{
							//  6 attribute label
							wxString class_name = tkz.GetNextToken();

							//  attribute number, ascii
							wxString token = tkz.GetNextToken();
							long liattr;
							token.ToLong ( &liattr );
							int iattr = liattr;
							if ( iattr > iattr_max )
								iattr_max = iattr;
						}
					}
				}

				m_max_attr = iattr_max;

				filea.SeekI ( 0 );


				//    Create the attribute label array

				m_AttrArray = new wxArrayString;
				m_AttrArray->Add ( _T ( "NULLNM" ), iattr_max+1 );

				//    And an array of chars describing the attribute value type
				m_ValTypeArray = ( char * ) malloc ( ( iattr_max + 1 ) * sizeof ( char ) );
				for ( int iat=0 ; iat < iattr_max + 1 ; iat++ )
					m_ValTypeArray[iat] = '?';


				//    Iterate over the file, filling in the values
				while ( !filea.Eof() )
				{
					//read a line
					line.Empty();
					while ( 1 )
					{
						char a = filea.GetC();
						if ( filea.Eof() )
							break;
						line.Append ( a );
						if ( a == 0x0a )
							break;
					}


					if ( !line.StartsWith ( ( const wxChar * ) wxT ( ";" ) ) )
					{
						wxStringTokenizer tkz ( line, wxT ( "|\r\n" ) );
						if ( tkz.CountTokens() >= 3 )
						{
							//  6 char class name
							wxString attr_name = tkz.GetNextToken();

							//  class number, ascii
							wxString token = tkz.GetNextToken();
							long liattr;
							token.ToLong ( &liattr );
							int iattr = liattr;

							m_AttrArray->Insert ( attr_name, iattr );
							m_AttrArray->RemoveAt ( iattr + 1 );

							token = tkz.GetNextToken().Trim();

							char atype = '?';
							if ( token.IsSameAs ( _T ( "aFLOAT" ) ) )
								atype = 'R';
							else if ( token.IsSameAs ( _T ( "aBYTE" ) ) )
								atype = 'B';
							else if ( token.IsSameAs ( _T ( "aSTRING" ) ) )
								atype = 'S';
							else if ( token.IsSameAs ( _T ( "aCMPLX" ) ) )
								atype = 'C';
							else if ( token.IsSameAs ( _T ( "aLIST" ) ) )
								atype = 'L';
							else if ( token.IsSameAs ( _T ( "aWORD10" ) ) )
								atype = 'W';
							else if ( token.IsSameAs ( _T ( "aLONG" ) ) )
								atype = 'G';

							m_ValTypeArray[iattr] = atype;

						}
					}
				}
				ret_val = true;
			}
			else              // stream IsOK
				ret_val = false;
		}
	}

	if ( ret_val )
	{
		m_ok = true;

		wxString msg ( _T ( "Loaded CM93 Dictionary from " ) );
		msg.Append ( dir );
		wxLogMessage ( msg );
	}

	return ret_val;
}

wxString cm93_dictionary::GetClassName ( int iclass )
{
	if ( ( iclass > m_max_class ) || ( iclass < 0 ) )
		return ( _T ( "Unknown" ) );
	else
		return ( m_S57ClassArray->Item ( iclass ) );
}

wxString cm93_dictionary::GetAttrName ( int iattr )
{
	if ( ( iattr > m_max_attr ) || ( iattr < 0 ) )
		return ( _T ( "UnknownAttr" ) );
	else
		return ( m_AttrArray->Item ( iattr ) );
}

//      char vtype = m_pDict->m_ValTypeArray[iattr];
char cm93_dictionary::GetAttrType ( int iattr )
{
	if ( ( iattr > m_max_attr ) || ( iattr < 0 ) )
		return ( '?' );
	else
		return ( m_ValTypeArray[iattr] );
}




cm93_dictionary::~cm93_dictionary()
{
	delete m_S57ClassArray;
	free ( m_GeomTypeArray );
	delete m_AttrArray;
	free ( m_ValTypeArray );
}


