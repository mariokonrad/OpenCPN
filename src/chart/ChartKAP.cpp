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

#include "ChartKAP.h"

#include <wx/wfstream.h>
#include <wx/tokenzr.h>

ChartKAP::ChartKAP()
{
     m_ChartType = CHART_TYPE_KAP;
}

ChartKAP::~ChartKAP()
{
}

InitReturn ChartKAP::Init( const wxString& name, ChartInitFlag init_flags )
{
      #define BUF_LEN_MAX 4000

      int nPlypoint = 0;
      Plypoint *pPlyTable = (Plypoint *)malloc(sizeof(Plypoint));

      PreInit(name, init_flags, GLOBAL_COLOR_SCHEME_DAY);

      char buffer[BUF_LEN_MAX];

      ifs_hdr = new wxFileInputStream(name);          // open the Header file as a read-only stream

      if(!ifs_hdr->Ok())
	  {
            free(pPlyTable);
            return INIT_FAIL_REMOVE;
      }

      m_FullPath = name;
      m_Description = m_FullPath;

      ifss_bitmap = new wxFileInputStream(name); // Open again, as the bitmap
      ifs_bitmap = new wxBufferedInputStream(*ifss_bitmap);

      //    Clear georeferencing coefficients
      for(int icl=0 ; icl< 12 ; icl++)
      {
          wpx[icl] = 0;
          wpy[icl] = 0;
          pwx[icl] = 0;
          pwy[icl] = 0;
      }


//    Validate the BSB header
//    by reading some characters into a buffer and looking for BSB\ keyword


      unsigned int TestBlockSize = 1999;
      ifs_hdr->Read(buffer, TestBlockSize);

      if(ifs_hdr->LastRead() != TestBlockSize)
      {
          wxString msg;
          msg.Printf(_("   Could not read first %d bytes of header for chart file: "), TestBlockSize);
          msg.Append(name);
          wxLogMessage(msg);
          free(pPlyTable);
          return INIT_FAIL_REMOVE;
      }

      unsigned int i;
      for( i = 0; i < TestBlockSize - 4; i++ )
      {
        //Test for "BSB/"
            if( buffer[i+0] == 'B' && buffer[i+1] == 'S'
                          && buffer[i+2] == 'B' && buffer[i+3] == '/' )
                  break;

        // Test for "NOS/"
            if( buffer[i+0] == 'N' && buffer[i+1] == 'O'
                          && buffer[i+2] == 'S' && buffer[i+3] == '/' )
                  break;

      }
      if( i == TestBlockSize - 4 )
      {
          wxString msg(_("   Chart file has no BSB header, cannot Init."));
          msg.Append(name);
          wxLogMessage(msg);
          free(pPlyTable);
          return INIT_FAIL_REMOVE;
      }



//    Read and Parse Chart Header, line by line
      ifs_hdr->SeekI(0, wxFromStart);                                         // rewind

      Size_X = Size_Y = 0;

      int done_header_parse = 0;

      while(done_header_parse == 0)
      {
            if(ReadBSBHdrLine(ifs_hdr, buffer, BUF_LEN_MAX) == 0)
            {
                  unsigned char c;
                  c = ifs_hdr->GetC();
                  ifs_hdr->Ungetch(c);

                  if(0x1a == c)
                      done_header_parse = 1;
                  else
                  {
                      free(pPlyTable);
                      return INIT_FAIL_REMOVE;
                  }

                  continue;
            }


            wxCSConv iso_conv(wxT("ISO-8859-1"));                 // we will need a converter

            wxString str_buf(buffer,  wxConvUTF8);
            if(!str_buf.Len())                                    // failed conversion
                  str_buf = wxString(buffer, iso_conv);

            if(str_buf.Find(_T("SHOM")) != wxNOT_FOUND)
                  m_b_SHOM = true;

            if(!strncmp(buffer, "BSB", 3))
            {
                  wxString clip_str_buf(&buffer[0],  iso_conv);  // for single byte French encodings of NAme field
                  wxStringTokenizer tkz(clip_str_buf, _T("/,="));
                  while ( tkz.HasMoreTokens() )
                  {
                        wxString token = tkz.GetNextToken();
                        if(token.IsSameAs(_T("RA"), TRUE))                  // extract RA=x,y
                        {
                              int i;
                              i = tkz.GetPosition();
                              Size_X = atoi(&buffer[i]);
                              wxString token = tkz.GetNextToken();
                              i = tkz.GetPosition();
                              Size_Y = atoi(&buffer[i]);
                        }
                        else if(token.IsSameAs(_T("NA"), TRUE))                  // extract NA=str
                        {
                          int i = tkz.GetPosition();
                          char nbuf[81];
                          int j=0;
                          while((buffer[i] != ',') && (i < 80))
                                nbuf[j++] = buffer[i++];
                          nbuf[j] = 0;
                          wxString n_str(nbuf,  iso_conv);
                          m_Name = n_str;
                        }
                        else if(token.IsSameAs(_T("NU"), TRUE))                  // extract NU=str
                        {
                              int i = tkz.GetPosition();
                              char nbuf[81];
                              int j=0;
                              while((buffer[i] != ',') && (i < 80))
                                    nbuf[j++] = buffer[i++];
                              nbuf[j] = 0;
                              wxString n_str(nbuf,  iso_conv);
                              m_ID = n_str;
                        }
                        else if(token.IsSameAs(_T("DU"), TRUE))                  // extract DU=n
                        {
                          token = tkz.GetNextToken();
                          long temp_du;
                          if(token.ToLong(&temp_du))
                                m_Chart_DU = temp_du;
                        }

                  }
            }

            else if(!strncmp(buffer, "KNP", 3))
            {
                  wxString conv_buf(buffer,  iso_conv);
                  wxStringTokenizer tkz(conv_buf, _T("/,="));
                  while ( tkz.HasMoreTokens() )
                  {
                        wxString token = tkz.GetNextToken();
                        if(token.IsSameAs(_T("SC"), TRUE))                  // extract Scale
                        {
                              int i;
                              i = tkz.GetPosition();
                              m_Chart_Scale = atoi(&buffer[i]);
                              if(0 == m_Chart_Scale)
                                    m_Chart_Scale = 100000000;
                        }
                        else if(token.IsSameAs(_T("SK"), TRUE))                  // extract Skew
                        {
                              int i;
                              i = tkz.GetPosition();
                              float fcs;
                              sscanf(&buffer[i], "%f,", &fcs);
                              m_Chart_Skew = fcs;
                        }
                        else if(token.IsSameAs(_T("UN"), TRUE))                  // extract Depth Units
                        {
                            int i;
                            i = tkz.GetPosition();
                            wxString str(&buffer[i], iso_conv);
                            m_DepthUnits = str.BeforeFirst(',');
                        }
                        else if(token.IsSameAs(_T("GD"), TRUE))                  // extract Datum
                        {
                              int i;
                              i = tkz.GetPosition();
                              wxString str(&buffer[i], iso_conv);
                              m_datum_str = str.BeforeFirst(',').Trim();
                        }
                        else if(token.IsSameAs(_T("SD"), TRUE))                  // extract Soundings Datum
                        {
                              int i;
                              i = tkz.GetPosition();
                              wxString str(&buffer[i], iso_conv);
                              m_SoundingsDatum = str.BeforeFirst(',').Trim();
                        }
                        else if(token.IsSameAs(_T("PP"), TRUE))                  // extract Projection Parameter
                        {
                              int i;
                              i = tkz.GetPosition();
                              double fcs;
                              wxString str(&buffer[i], iso_conv);
                              wxString str1 = str.BeforeFirst(',').Trim();
                              if(str1.ToDouble(&fcs))
                                    m_proj_parameter = fcs;
                        }
                        else if(token.IsSameAs(_T("PR"), TRUE))                  // extract Projection Type
                        {
                              int i;
                              i = tkz.GetPosition();
                              wxString str(&buffer[i], iso_conv);
                              wxString stru = str.MakeUpper();
                              bool bp_set = false;;

                              if(stru.Matches(_T("*MERCATOR*")))
                              {
                                    m_projection = PROJECTION_MERCATOR;
                                    bp_set = true;
                              }

                              if(stru.Matches(_T("*TRANSVERSE*")))
                              {
                                    m_projection = PROJECTION_TRANSVERSE_MERCATOR;
                                    bp_set = true;
                              }

                              if(stru.Matches(_T("*POLYCONIC*")))
                              {
                                    m_projection = PROJECTION_POLYCONIC;
                                    bp_set = true;
                              }

                              if(stru.Matches(_T("*TM*")))
                              {
                                    m_projection = PROJECTION_TRANSVERSE_MERCATOR;
                                    bp_set = true;
                              }

                              if(!bp_set)
                              {
                                  m_projection = PROJECTION_UNKNOWN;
                                  wxString msg(_T("   Chart projection is "));
                                  msg += tkz.GetNextToken();
                                  msg += _T(" which is unsupported.  Disabling chart ");
                                  msg += m_FullPath;
                                  wxLogMessage(msg);

                                  return INIT_FAIL_REMOVE;
                              }

                        }
                        else if(token.IsSameAs(_T("DX"), TRUE))                  // extract Pixel scale parameter, if present
                        {
                              int i;
                              i = tkz.GetPosition();
                              float x;
                              sscanf(&buffer[i], "%f,", &x);
                              m_dx = x;
                        }
                        else if(token.IsSameAs(_T("DY"), TRUE))                  // extract Pixel scale parameter, if present
                        {
                              int i;
                              i = tkz.GetPosition();
                              float x;
                              sscanf(&buffer[i], "%f,", &x);
                              m_dy = x;
                        }


                 }
            }


            else if (!strncmp(buffer, "RGB", 3))
                  CreatePaletteEntry(buffer, COLOR_RGB_DEFAULT);

            else if (!strncmp(buffer, "DAY", 3))
                  CreatePaletteEntry(buffer, DAY);

            else if (!strncmp(buffer, "DSK", 3))
                  CreatePaletteEntry(buffer, DUSK);

            else if (!strncmp(buffer, "NGT", 3))
                  CreatePaletteEntry(buffer, NIGHT);

            else if (!strncmp(buffer, "NGR", 3))
                  CreatePaletteEntry(buffer, NIGHTRED);

            else if (!strncmp(buffer, "GRY", 3))
                  CreatePaletteEntry(buffer, GRAY);

            else if (!strncmp(buffer, "PRC", 3))
                  CreatePaletteEntry(buffer, PRC);

            else if (!strncmp(buffer, "PRG", 3))
                  CreatePaletteEntry(buffer, PRG);


            else if (!strncmp(buffer, "REF", 3))
            {
                  int i, xr, yr;
                  float ltr,lnr;
                  sscanf(&buffer[4], "%d,%d,%d,%f,%f", &i, &xr, &yr, &ltr, &lnr);
                  pRefTable = (Refpoint *)realloc(pRefTable, sizeof(Refpoint) * (nRefpoint+1));
                  pRefTable[nRefpoint].xr = xr;
                  pRefTable[nRefpoint].yr = yr;
                  pRefTable[nRefpoint].latr = ltr;
                  pRefTable[nRefpoint].lonr = lnr;
                  pRefTable[nRefpoint].bXValid = 1;
                  pRefTable[nRefpoint].bYValid = 1;

                  nRefpoint++;

            }

            else if (!strncmp(buffer, "WPX", 3))
            {
                int idx = 0;
                double d;
                wxStringTokenizer tkz(str_buf.Mid(4), _T(","));
                wxString token = tkz.GetNextToken();

                if(token.ToLong((long int *)&wpx_type))
                {
                    while ( tkz.HasMoreTokens() && (idx < 12) )
                    {
                        token = tkz.GetNextToken();
                        if(token.ToDouble(&d))
                        {
                            wpx[idx] = d;
                            idx++;
                        }
                    }
                }
                n_wpx = idx;
            }

            else if (!strncmp(buffer, "WPY", 3))
            {
                int idx = 0;
                double d;
                wxStringTokenizer tkz(str_buf.Mid(4), _T(","));
                wxString token = tkz.GetNextToken();

                if(token.ToLong((long int *)&wpy_type))
                {
                    while ( tkz.HasMoreTokens() && (idx < 12) )
                    {
                        token = tkz.GetNextToken();
                        if(token.ToDouble(&d))
                        {
                            wpy[idx] = d;
                            idx++;
                        }
                    }
                }
                n_wpy = idx;
            }

            else if (!strncmp(buffer, "PWX", 3))
            {
                int idx = 0;
                double d;
                wxStringTokenizer tkz(str_buf.Mid(4), _T(","));
                wxString token = tkz.GetNextToken();

                if(token.ToLong((long int *)&pwx_type))
                {
                    while ( tkz.HasMoreTokens() && (idx < 12) )
                    {
                        token = tkz.GetNextToken();
                        if(token.ToDouble(&d))
                        {
                            pwx[idx] = d;
                            idx++;
                        }
                    }
                }
                n_pwx = idx;
            }

            else if (!strncmp(buffer, "PWY", 3))
            {
                int idx = 0;
                double d;
                wxStringTokenizer tkz(str_buf.Mid(4), _T(","));
                wxString token = tkz.GetNextToken();

                if(token.ToLong((long int *)&pwy_type))
                {
                    while ( tkz.HasMoreTokens() && (idx < 12) )
                    {
                        token = tkz.GetNextToken();
                        if(token.ToDouble(&d))
                        {
                            pwy[idx] = d;
                            idx++;
                        }
                    }
                }
                n_pwy = idx;
            }


            else if (!strncmp(buffer, "CPH", 3))
            {
                float float_cph;
                sscanf(&buffer[4], "%f", &float_cph);
                m_cph = float_cph;
            }

            else if (!strncmp(buffer, "VER", 3))
            {
                  wxStringTokenizer tkz(str_buf, _T("/,="));
                  wxString token = tkz.GetNextToken();

                  m_bsb_ver = tkz.GetNextToken();
            }

            else if (!strncmp(buffer, "DTM", 3))
            {
                  double val;
                  wxStringTokenizer tkz(str_buf, _T("/,="));
                  wxString token = tkz.GetNextToken();

                  token = tkz.GetNextToken();
                  if(token.ToDouble(&val))
                        m_dtm_lat = val;

                  token = tkz.GetNextToken();
                  if(token.ToDouble(&val))
                        m_dtm_lon = val;


//                  float fdtmlat, fdtmlon;
//                  sscanf(&buffer[4], "%f,%f", &fdtmlat, &fdtmlon);
//                  m_dtm_lat = fdtmlat;
//                  m_dtm_lon = fdtmlon;
            }


            else if (!strncmp(buffer, "PLY", 3))
            {
                  int i;
                  float ltp,lnp;
                  sscanf(&buffer[4], "%d,%f,%f", &i, &ltp, &lnp);
                  Plypoint *tmp = pPlyTable;
                  pPlyTable = (Plypoint *)realloc(pPlyTable, sizeof(Plypoint) * (nPlypoint+1));
                  if (NULL == pPlyTable)
                  {
                      free(tmp);
                      tmp = NULL;
                  } else
                  {
                      pPlyTable[nPlypoint].ltp = ltp;
                      pPlyTable[nPlypoint].lnp = lnp;
                      nPlypoint++;
                  }
            }

            else if(!strncmp(buffer, "CED", 3))
            {
                wxStringTokenizer tkz(str_buf, _T("/,="));
                  while ( tkz.HasMoreTokens() )
                  {
                        wxString token = tkz.GetNextToken();
                        if(token.IsSameAs(_T("ED"), TRUE))                  // extract Edition Date
                        {

                              int i;
                              i = tkz.GetPosition();

                              char date_string[40];
                              char date_buf[10];
                              sscanf(&buffer[i], "%s\r\n", date_string);
                              wxString date_wxstr(date_string,  wxConvUTF8);

                              wxDateTime dt;
                              if(dt.ParseDate(date_wxstr))       // successful parse?
                              {
                                  int iyear = dt.GetYear(); // GetYear() fails on W98, DMC compiler, wx2.8.3
//    BSB charts typically list publish date as xx/yy/zz, we want 19zz.
                                  if(iyear < 100)
                                  {
                                      iyear += 1900;
                                      dt.SetYear(iyear);
                                  }
                                  sprintf(date_buf, "%d", iyear);

                              //    Initialize the wxDateTime menber for Edition Date
                                  m_EdDate = dt;
                              }
                              else
                              {
                                 sscanf(date_string, "%s", date_buf);
                                 m_EdDate.Set(1, wxDateTime::Jan, 2000);                    //Todo this could be smarter
                              }

                              m_PubYear = wxString(date_buf,  wxConvUTF8);
                        }
                        else if(token.IsSameAs(_T("SE"), TRUE))                  // extract Source Edition
                        {
                              int i;
                              i = tkz.GetPosition();
                              wxString str(&buffer[i], iso_conv);
                              m_SE = str.BeforeFirst(',');
                        }

                  }
            }

      }

      //    Some charts improperly encode the DTM parameters.
      //    Identify them as necessary, for further processing
      if(m_b_SHOM && (m_bsb_ver == _T("1.1")))
            m_b_apply_dtm = false;

      //    If imbedded coefficients are found,
      //    then use the polynomial georeferencing algorithms
      if(n_pwx && n_pwy && n_pwx && n_pwy)
          bHaveEmbeddedGeoref = true;


      //    Set up the projection point according to the projection parameter
      if(m_projection == PROJECTION_MERCATOR)
            m_proj_lat = m_proj_parameter;
      else if(m_projection == PROJECTION_TRANSVERSE_MERCATOR)
            m_proj_lon = m_proj_parameter;
      else if(m_projection == PROJECTION_POLYCONIC)
            m_proj_lon = m_proj_parameter;


//    Validate some of the header data
      if((Size_X == 0) || (Size_Y == 0))
      {
          free(pPlyTable);
          return INIT_FAIL_REMOVE;
      }

      if(nPlypoint < 3)
      {
            wxString msg(_("   Chart File contains less than 3 PLY points: "));
            msg.Append(m_FullPath);
            wxLogMessage(msg);
            free(pPlyTable);
            return INIT_FAIL_REMOVE;
      }


//    Convert captured plypoint information into chart COVR structures
      m_nCOVREntries = 1;
      m_pCOVRTablePoints = (int *)malloc(sizeof(int));
      *m_pCOVRTablePoints = nPlypoint;
      m_pCOVRTable = (float **)malloc(sizeof(float *));
      *m_pCOVRTable = (float *)malloc(nPlypoint * 2 * sizeof(float));
      memcpy(*m_pCOVRTable, pPlyTable, nPlypoint * 2 * sizeof(float));
      free(pPlyTable);


      //    Adjust the PLY points to WGS84 datum
      Plypoint *ppp = (Plypoint *)GetCOVRTableHead(0);
      int cnPlypoint = GetCOVRTablenPoints(0);

      //  n.b. this is not precisely right for non-wgs84 charts.
      //  should use molodensky transform, and then consider SHOM Ver 1.1 charts

      for(int u=0 ; u<cnPlypoint ; u++)
      {
            ppp->lnp += m_dtm_lon / 3600;
            ppp->ltp += m_dtm_lat / 3600;
            ppp++;
      }


      if(!SetMinMax())
            return INIT_FAIL_REMOVE;          // have to bail here

      if(init_flags == HEADER_ONLY)
            return INIT_OK;

//    Advance to the data
      unsigned char c;
      bool bcorrupt = false;

      if((c = ifs_hdr->GetC()) != 0x1a){ bcorrupt = true; }
      if((c = ifs_hdr->GetC()) == 0x0d)
      {
            if((c = ifs_hdr->GetC()) != 0x0a){ bcorrupt = true; }
            if((c = ifs_hdr->GetC()) != 0x1a){ bcorrupt = true; }
            if((c = ifs_hdr->GetC()) != 0x00){ bcorrupt = true; }
      }

      else if(c != 0x00){ bcorrupt = true; }


      if(bcorrupt)
      {
            wxString msg(_("   Chart File RLL data corrupt on chart "));
            msg.Append(m_FullPath);
            wxLogMessage(msg);

            return INIT_FAIL_REMOVE;
      }


//    Read the Color table bit size
      nColorSize = ifs_hdr->GetC();

      nFileOffsetDataStart = ifs_hdr->TellI();

//    Perform common post-init actions in ChartBaseBSB
      InitReturn pi_ret = PostInit();
      if( pi_ret  != INIT_OK)
            return pi_ret;
      else
            return INIT_OK;
}
