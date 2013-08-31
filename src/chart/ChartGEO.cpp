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

#include "ChartGEO.h"

#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/dir.h>
#include <wx/log.h>

ChartGEO::ChartGEO()
{
	m_ChartType = CHART_TYPE_GEO;
}

ChartGEO::~ChartGEO()
{
}

InitReturn ChartGEO::Init( const wxString& name, ChartInitFlag init_flags)
{
#define BUF_LEN_MAX 4000

	PreInit(name, init_flags, GLOBAL_COLOR_SCHEME_DAY);

	char buffer[BUF_LEN_MAX];

	ifs_hdr = new wxFileInputStream(name);          // open the file as a read-only stream

	if(!ifs_hdr->Ok())
		return INIT_FAIL_REMOVE;

	int nPlypoint = 0;
	Plypoint *pPlyTable = (Plypoint *)malloc(sizeof(Plypoint)); // FIXME: use std::vector instead of dynamic memory allocation

	m_FullPath = name;
	m_Description = m_FullPath;

	wxFileName GEOFile(m_FullPath);

	wxString Path;
	Path = GEOFile.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME);


	//    Read the GEO file, extracting useful information

	ifs_hdr->SeekI(0, wxFromStart);                 // rewind

	Size_X = Size_Y = 0;

	wxString bitmap_filepath;
	while( (ReadBSBHdrLine(ifs_hdr, &buffer[0], BUF_LEN_MAX)) != 0 )
	{
		wxString str_buf(buffer, wxConvUTF8);
		if(!strncmp(buffer, "Bitmap", 6))
		{
			wxStringTokenizer tkz(str_buf, _T("="));
			wxString token = tkz.GetNextToken();
			if(token.IsSameAs(_T("Bitmap"), TRUE))
			{
				int i;
				i = tkz.GetPosition();
				bitmap_filepath.Clear();
				while (buffer[i]) {
					bitmap_filepath.Append(buffer[i]);
					i++;
				}
			}
		}


		else if(!strncmp(buffer, "Scale", 5))
		{
			wxStringTokenizer tkz(str_buf, _T("="));
			wxString token = tkz.GetNextToken();
			if(token.IsSameAs(_T("Scale"), TRUE))               // extract Scale
			{
				int i;
				i = tkz.GetPosition();
				m_Chart_Scale = atoi(&buffer[i]);
			}
		}

		else if(!strncmp(buffer, "Depth", 5))
		{
			wxStringTokenizer tkz(str_buf, _T("="));
			wxString token = tkz.GetNextToken();
			if(token.IsSameAs(_T("Depth Units"), FALSE))               // extract Depth Units
			{
				int i;
				i = tkz.GetPosition();
				wxString str(&buffer[i],  wxConvUTF8);
				m_DepthUnits = str.Trim();
			}
		}

		else if (!strncmp(buffer, "Point", 5))                // Extract RefPoints
		{
			// FIXME: why not read Refpoint directly, DUPLICATE CODE
			int i, xr, yr;
			float ltr,lnr;
			sscanf(&buffer[0], "Point%d=%f %f %d %d", &i, &lnr, &ltr, &yr, &xr);
			Refpoint p;
			p.xr = xr;
			p.yr = yr;
			p.latr = ltr;
			p.lonr = lnr;
			p.bXValid = 1;
			p.bYValid = 1;
			reference_points.push_back(p);
		}

		else if (!strncmp(buffer, "Vertex", 6))
		{
			int i;
			float ltp,lnp;
			sscanf(buffer, "Vertex%d=%f %f", &i, &ltp, &lnp);
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

		else if (!strncmp(buffer, "Date Pub", 8))
		{
			char date_string[40];
			char date_buf[10];
			sscanf(buffer, "Date Published=%s\r\n", &date_string[0]);
			wxString date_wxstr(date_string,  wxConvUTF8);
			wxDateTime dt;
			if(dt.ParseDate(date_wxstr))       // successful parse?
			{
				sprintf(date_buf, "%d", dt.GetYear());
			}
			else
			{
				sscanf(date_string, "%s", date_buf);
			}
			m_PubYear = wxString(date_buf, wxConvUTF8);
		}

		else if (!strncmp(buffer, "Skew", 4))
		{
			wxStringTokenizer tkz(str_buf, _T("="));
			wxString token = tkz.GetNextToken();
			if(token.IsSameAs(_T("Skew Angle"), FALSE))               // extract Skew Angle
			{
				int i;
				i = tkz.GetPosition();
				float fcs;
				sscanf(&buffer[i], "%f,", &fcs);
				m_Chart_Skew = fcs;
			}
		}

		else if (!strncmp(buffer, "Latitude Offset", 15))
		{
			wxStringTokenizer tkz(str_buf, _T("="));
			wxString token = tkz.GetNextToken();
			if(token.IsSameAs(_T("Latitude Offset"), FALSE))
			{
				int i;
				i = tkz.GetPosition();
				float lto;
				sscanf(&buffer[i], "%f,", &lto);
				m_dtm_lat = lto;
			}
		}


		else if (!strncmp(buffer, "Longitude Offset", 16))
		{
			wxStringTokenizer tkz(str_buf, _T("="));
			wxString token = tkz.GetNextToken();
			if(token.IsSameAs(_T("Longitude Offset"), FALSE))
			{
				int i;
				i = tkz.GetPosition();
				float lno;
				sscanf(&buffer[i], "%f,", &lno);
				m_dtm_lon = lno;
			}
		}

		else if (!strncmp(buffer, "Datum", 5))
		{
			wxStringTokenizer tkz(str_buf, _T("="));
			wxString token = tkz.GetNextToken();
			if(token.IsSameAs(_T("Datum"), FALSE))
			{
				token = tkz.GetNextToken();
				m_datum_str = token;
			}
		}


		else if (!strncmp(buffer, "Name", 4))
		{
			wxStringTokenizer tkz(str_buf, _T("="));
			wxString token = tkz.GetNextToken();
			if(token.IsSameAs(_T("Name"), FALSE))                         // Name
			{
				int i;
				i = tkz.GetPosition();
				m_Name.Clear();
				while(isprint(buffer[i]) && (i < 80))
					m_Name.Append(buffer[i++]);
			}
		}
	}     //while



	// Extract the remaining data from .NOS Bitmap file
	ifs_bitmap = NULL;

	// Something wrong with the .geo file, there is no Bitmap reference
	// This is where the arbitrarily bad file is caught, such as
	// a file with.GEO extension that is not really a chart
	if (bitmap_filepath.IsEmpty())
		return INIT_FAIL_REMOVE;

	wxString NOS_Name(bitmap_filepath);            // take a copy

	wxDir target_dir(Path);
	wxArrayString file_array;
	int nfiles = wxDir::GetAllFiles(Path, &file_array);
	int ifile;

	bitmap_filepath.Prepend(Path);

	wxFileName NOS_filename(bitmap_filepath);
	if(NOS_filename.FileExists())
	{
		ifss_bitmap = new wxFileInputStream(bitmap_filepath); // open the bitmap file
		ifs_bitmap = new wxBufferedInputStream(*ifss_bitmap);
	}
	//    File as fetched verbatim from the .geo file doesn't exist.
	//    Try all possible upper/lower cases
	else
	{
		//    Extract the filename and extension
		wxString fname(NOS_filename.GetName());
		wxString fext(NOS_filename.GetExt());

		//    Try all four combinations, the hard way
		// case 1
		fname.MakeLower();
		fext.MakeLower();
		NOS_filename.SetName(fname);
		NOS_filename.SetExt(fext);

		if(NOS_filename.FileExists())
			goto found_uclc_file;

		// case 2
		fname.MakeLower();
		fext.MakeUpper();
		NOS_filename.SetName(fname);
		NOS_filename.SetExt(fext);

		if(NOS_filename.FileExists())
			goto found_uclc_file;

		// case 3
		fname.MakeUpper();
		fext.MakeLower();
		NOS_filename.SetName(fname);
		NOS_filename.SetExt(fext);

		if(NOS_filename.FileExists())
			goto found_uclc_file;

		// case 4
		fname.MakeUpper();
		fext.MakeUpper();
		NOS_filename.SetName(fname);
		NOS_filename.SetExt(fext);

		if(NOS_filename.FileExists())
			goto found_uclc_file;


		//      Search harder

		for(ifile = 0 ; ifile < nfiles ; ifile++)
		{
			wxString file_up = file_array.Item(ifile);
			file_up.MakeUpper();

			wxString target_up = bitmap_filepath;
			target_up.MakeUpper();

			if(file_up.IsSameAs( target_up))
			{
				NOS_filename.Clear();
				NOS_filename.Assign(file_array.Item(ifile));
				goto found_uclc_file;
			}

		}

		return INIT_FAIL_REMOVE;                  // not found at all

found_uclc_file:

		bitmap_filepath = NOS_filename.GetFullPath();
		ifss_bitmap = new wxFileInputStream(bitmap_filepath); // open the bitmap file
		ifs_bitmap = new wxBufferedInputStream(*ifss_bitmap);

	}


	if(ifs_bitmap == NULL)
		return INIT_FAIL_REMOVE;

	if(!ifss_bitmap->Ok())
		return INIT_FAIL_REMOVE;


	while( (ReadBSBHdrLine(ifss_bitmap, &buffer[0], BUF_LEN_MAX)) != 0 )
	{
		wxString str_buf(buffer,  wxConvUTF8);

		if(!strncmp(buffer, "NOS", 3))
		{
			wxStringTokenizer tkz(str_buf, _T(",="));
			while ( tkz.HasMoreTokens() )
			{
				wxString token = tkz.GetNextToken();
				if(token.IsSameAs(_T("RA"), TRUE))                  // extract RA=x,y
				{
					int i;
					tkz.GetNextToken();
					tkz.GetNextToken();
					i = tkz.GetPosition();
					Size_X = atoi(&buffer[i]);
					wxString token = tkz.GetNextToken();
					i = tkz.GetPosition();
					Size_Y = atoi(&buffer[i]);
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
	}


	//    Validate some of the header data
	if((Size_X == 0) || (Size_Y == 0))
		return INIT_FAIL_REMOVE;

	if(nPlypoint < 3)
	{
		wxString msg(_("   Chart File contains less than 3 PLY points: "));
		msg.Append(m_FullPath);
		wxLogMessage(msg);

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

	if(!SetMinMax())
		return INIT_FAIL_REMOVE;          // have to bail here

	if(init_flags == HEADER_ONLY)
		return INIT_OK;

	//    Advance to the data
	char c;
	if((c = ifs_bitmap->GetC()) != 0x1a){ return INIT_FAIL_REMOVE;}
	if((c = ifs_bitmap->GetC()) == 0x0d)
	{
		if((c = ifs_bitmap->GetC()) != 0x0a){  return INIT_FAIL_REMOVE;}
		if((c = ifs_bitmap->GetC()) != 0x1a){  return INIT_FAIL_REMOVE;}
		if((c = ifs_bitmap->GetC()) != 0x00){  return INIT_FAIL_REMOVE;}
	}

	else if(c != 0x00){  return INIT_FAIL_REMOVE;}

	//    Read the Color table bit size
	nColorSize = ifs_bitmap->GetC();


	//    Perform common post-init actions in ChartBaseBSB
	InitReturn pi_ret = PostInit();
	if( pi_ret  != INIT_OK)
		return pi_ret;
	else
		return INIT_OK;

}


