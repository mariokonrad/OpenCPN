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

#include "CM93_attr_block.h"
#include <chart/CM93Dictionary.h>

namespace chart {

cm93_attr_block::cm93_attr_block(void * block, cm93_dictionary * pdict)
{
	m_cptr = 0;
	m_block = ( unsigned char * ) block;
	m_pDict = pdict;
}

unsigned char *cm93_attr_block::GetNextAttr()
{
	//    return current pointer
	unsigned char *ret_val = m_block + m_cptr;

	//    Advance the pointer

	unsigned char iattr = * ( m_block + m_cptr );
	m_cptr++;

	//      char vtype = m_pDict->m_ValTypeArray[iattr];
	char vtype = m_pDict->GetAttrType ( iattr );

	switch ( vtype )
	{
		case 'I':                           // never seen?
			m_cptr += 2;
			break;
		case 'B':
			m_cptr += 1;
			//                  pb = (unsigned char *)aval;
			//                  sprintf(val, "%d", *pb);
			//                  pvtype = 'I';                 // override
			break;
		case 'S':
			while ( * ( m_block + m_cptr ) )
				m_cptr++;
			m_cptr++;                           // skip terminator
			//                  sprintf(val, "%s", aval);
			break;
		case 'R':
			m_cptr += 4;
			//                  pf = (float *)aval;
			//                  sprintf(val, "%g", *pf);
			break;
		case 'W':
			m_cptr += 2;
			break;
		case 'G':
			m_cptr += 4;
			break;
		case 'C':
			m_cptr += 3;
			while ( * ( m_block + m_cptr ) )
				m_cptr++;
			m_cptr++;                           // skip terminator
			//                  sprintf(val, "%s", &aval[3]);
			//                  pvtype = 'S';                 // override
			break;
		case 'L':
			{
				unsigned char nl = * ( m_block + m_cptr );
				m_cptr++;
				m_cptr += nl;

				//                  pb = (unsigned char *)aval;
				//                  unsigned char nl = *pb++;
				//                  char vi[20];
				//                  val[0] = 0;
				//                  for(int i=0 ; i<nl ; i++)
				//                  {
				//                        sprintf(vi, "%d,", *pb++);
				//                        strcat(val, vi);
				//                  }
				//                  if(strlen(val))
				//                        val[strlen(val)-1] = 0;         // strip last ","
				//                  pvtype = 'S';                 // override
				break;
			}
		default:
			//                  sprintf(val, "Unknown Value Type");
			break;
	}

	return ret_val;

}

}

