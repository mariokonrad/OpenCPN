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

#include "OCPNBitmap.h"
#include <wx/image.h>

#ifdef ocpnUSE_OCPNBitmap

#define M_BMPDATA wx_static_cast(wxBitmapRefData*, m_refData)

IMPLEMENT_DYNAMIC_CLASS(OCPNBitmap, wxBitmap)


OCPNBitmap::OCPNBitmap()
{
}

#ifdef  __WXGTK__
#ifdef opcnUSE_GTK_OPTIMIZE
bool OCPNBitmap::CreateFromData( void *pPix, int width, int height, int depth )
{
	Create(width, height, 32);

	if(NULL != pPix)
	{
		GdkPixbuf* pixbuf = GetPixbuf();

		if (!pixbuf)
			return false;

		unsigned char* in = (unsigned char *)pPix;
		unsigned char *out = gdk_pixbuf_get_pixels(pixbuf);

		int rowpad = gdk_pixbuf_get_rowstride(pixbuf) - 4 * width;

		for (int y = 0; y < height; y++, out += rowpad)
		{
			for (int x = 0; x < width; x++, out += 4, in += 4)
			{
				out[0] = in[0];
				out[1] = in[1];
				out[2] = in[2];
				out[3] = in[3];
			}

		}
	}
	return true;

}
#endif
#endif


#ifdef __WXX11__
bool OCPNBitmap::CreateFromocpnXImage( ocpnXImage *poXI, int width, int height, int depth )
{
	//    Do some basic setup in the parent  wxBitmap class
	Create(width, height, -1);

	Display *xdisplay = (Display *)GetDisplay();

	XImage *data_image = poXI->m_img;
	bool bShared = poXI->buse_mit;

	// Blit picture

	Pixmap mypixmap = ((Pixmap )GetPixmap());

	GC gc = XCreateGC( xdisplay, mypixmap, 0, NULL );

	if(bShared)
		XShmPutImage( xdisplay, mypixmap, gc, data_image, 0, 0, 0, 0, width, height, False );
	else
		XPutImage( xdisplay, mypixmap, gc, data_image, 0, 0, 0, 0, width, height );

	XFreeGC( xdisplay, gc );

	return true;
}

bool OCPNBitmap::CreateFromData( void *pPix, int width, int height, int depth )
{

	XImage *img = NULL;

	//    Do some basic setup in the parent wxBitmap class
	Create(width, height, -1);

	Display *xdisplay = (Display *)GetDisplay();

	ocpnXImage *pXI = new ocpnXImage(width, height);
	img = pXI->m_img;

	//    Faster render from a 24 or 32 bit pixel buffer

	if((pPix != NULL ) && (NULL != img))
	{
		unsigned char* data = (unsigned char *)pPix;
		if(depth == 32)                          // special fast case
		{
			for (int y = 0; y < height; y++)
			{
				char *pd = img->data + (y * img->bytes_per_line);
				unsigned char *ps = data + (y * width * 4);
				memcpy(pd, ps, width*4);
			}
		}

		else
		{
			int *pi = (int *)img->data;
			int index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					int ri = *(int *)(&data[index]);
					index++;
					index++;
					index++;

					*pi = ri;
					pi++;

				}
			}
		}
	}

	// Blit picture

	Pixmap mypixmap = ((Pixmap )GetPixmap());
	GC gc = XCreateGC( xdisplay, mypixmap, 0, NULL );

	pXI->PutImage(mypixmap, gc);

	delete pXI;

	XFreeGC( xdisplay, gc );

	return TRUE;
}
#endif //__WXX11__


#ifdef __WXMSW__
bool OCPNBitmap::CreateFromData( void *pPix, int width, int height, int depth )
{
	m_refData = CreateData();                 // found in wxBitmap
	//    int width = image.GetWidth();
	//    int height0 = image.GetHeight();

	int height0 = height;
	int sizeLimit = 1280*1024 *3;

	int bmpHeight = height0;
	//    int height = bmpHeight;

	// calc the number of bytes per scanline and padding
	int bytePerLine = width*3;
	int sizeDWORD = sizeof( DWORD );
	int lineBoundary = bytePerLine % sizeDWORD;
	int padding = 0;
	if( lineBoundary > 0 )
	{
		padding = sizeDWORD - lineBoundary;
		bytePerLine += padding;
	}

	// set bitmap parameters
	SetWidth( width );
	SetHeight( bmpHeight );
	if (depth == -1) depth = wxDisplayDepth();
	SetDepth( depth );


	// create a DIB header
	int headersize = sizeof(BITMAPINFOHEADER);
	BITMAPINFO *lpDIBh = (BITMAPINFO *) malloc( headersize );
	wxCHECK_MSG( lpDIBh, FALSE, wxT("could not allocate memory for DIB header") );
	// Fill in the DIB header
	lpDIBh->bmiHeader.biSize = headersize;
	lpDIBh->bmiHeader.biWidth = (DWORD)width;
	lpDIBh->bmiHeader.biHeight = (DWORD)(-height);
	lpDIBh->bmiHeader.biSizeImage = bytePerLine*height;
	//   the general formula for biSizeImage:
	//      ( ( ( ((DWORD)width*24) +31 ) & ~31 ) >> 3 ) * height;
	lpDIBh->bmiHeader.biPlanes = 1;
	lpDIBh->bmiHeader.biBitCount = 24;
	lpDIBh->bmiHeader.biCompression = BI_RGB;
	lpDIBh->bmiHeader.biClrUsed = 0;
	// These seem not really needed for our purpose here.
	lpDIBh->bmiHeader.biClrImportant = 0;
	lpDIBh->bmiHeader.biXPelsPerMeter = 0;
	lpDIBh->bmiHeader.biYPelsPerMeter = 0;
	// memory for DIB data
	unsigned char *lpBits;
	lpBits = (unsigned char *)malloc( lpDIBh->bmiHeader.biSizeImage );
	if( !lpBits )
	{
		wxFAIL_MSG( wxT("could not allocate memory for DIB") );
		free( lpDIBh );
		return FALSE;
	}

	// create and set the device-dependent bitmap
	HDC hdc = ::GetDC(NULL);
	HDC memdc = ::CreateCompatibleDC( hdc );
	HBITMAP hbitmap;
	//    hbitmap = ::CreateCompatibleBitmap( hdc, width, bmpHeight );

	//    if(hbitmap == NULL)
	//          int cop =0;

	//    ::SelectObject( memdc, hbitmap);


	// copy image data into DIB data
	unsigned char *data = (unsigned char *)pPix;
	int i, j;
	unsigned char *ptdata = data;
	unsigned char *ptbits;

	ptbits = lpBits;

	if(pPix)
	{
		for( j=0; j<height; j++ )
		{

			memcpy(ptbits, ptdata, width * 3);
			ptbits += width * 3;
			ptdata += width * 3;

			for( i=0; i< padding; i++ )   *(ptbits++) = 0;
		}
	}

	else
	{
		for( j=0; j<height; j++ )
		{

			memset(ptbits, 0,  width * 3);
			ptbits += width * 3;

			for( i=0; i< padding; i++ )   *(ptbits++) = 0;
		}
	}




	hbitmap = CreateDIBitmap( hdc, &(lpDIBh->bmiHeader), CBM_INIT, lpBits, lpDIBh, DIB_RGB_COLORS );
	// The above line is equivalent to the following two lines.
	//    hbitmap = ::CreateCompatibleBitmap( hdc, width, height );
	//    ::SetDIBits( hdc, hbitmap, 0, height, lpBits, lpDIBh, DIB_RGB_COLORS);
	// or the following lines
	//    hbitmap = ::CreateCompatibleBitmap( hdc, width, height );
	//    HDC memdc = ::CreateCompatibleDC( hdc );
	//    ::SelectObject( memdc, hbitmap);
	//    ::SetDIBitsToDevice( memdc, 0, 0, width, height,
	//              0, 0, 0, height, (void *)lpBits, lpDIBh, DIB_RGB_COLORS);
	//    ::SelectObject( memdc, 0 );
	//    ::DeleteDC( memdc );
	SetHBITMAP( (WXHBITMAP) hbitmap );


	// free allocated resources
	::DeleteDC( memdc );
	::ReleaseDC(NULL, hdc);
	free(lpDIBh);
	free(lpBits);

	return TRUE;
}

bool OCPNBitmap::CreateFromImage( const wxImage& image, int depth )
{
	//    wxCHECK_MSG( image.Ok(), FALSE, wxT("invalid image") )

	m_refData = CreateData();                 // found in wxBitmap

	// sizeLimit is the MS upper limit for the DIB size
	int sizeLimit = 1280*1024 *3;

	// width and height of the device-dependent bitmap
	int width = image.GetWidth();
	int bmpHeight = image.GetHeight();

	// calc the number of bytes per scanline and padding
	int bytePerLine = width*3;
	int sizeDWORD = sizeof( DWORD );
	int lineBoundary = bytePerLine % sizeDWORD;
	int padding = 0;
	if( lineBoundary > 0 )
	{
		padding = sizeDWORD - lineBoundary;
		bytePerLine += padding;
	}
	// calc the number of DIBs and heights of DIBs
	int numDIB = 1;
	int hRemain = 0;
	int height = sizeLimit/bytePerLine;
	if( height >= bmpHeight )
		height = bmpHeight;
	else
	{
		numDIB =  bmpHeight / height;
		hRemain = bmpHeight % height;
		if( hRemain >0 )  numDIB++;
	}

	// set bitmap parameters
	wxCHECK_MSG( image.Ok(), FALSE, wxT("invalid image") );
	SetWidth( width );
	SetHeight( bmpHeight );
	if (depth == -1) depth = wxDisplayDepth();
	SetDepth( depth );

#if wxUSE_PALETTE
	// Copy the palette from the source image
	SetPalette(image.GetPalette());
#endif // wxUSE_PALETTE

	// create a DIB header
	int headersize = sizeof(BITMAPINFOHEADER);
	BITMAPINFO *lpDIBh = (BITMAPINFO *) malloc( headersize );
	wxCHECK_MSG( lpDIBh, FALSE, wxT("could not allocate memory for DIB header") );
	// Fill in the DIB header
	lpDIBh->bmiHeader.biSize = headersize;
	lpDIBh->bmiHeader.biWidth = (DWORD)width;
	lpDIBh->bmiHeader.biHeight = (DWORD)(-height);
	lpDIBh->bmiHeader.biSizeImage = bytePerLine*height;
	//   the general formula for biSizeImage:
	//      ( ( ( ((DWORD)width*24) +31 ) & ~31 ) >> 3 ) * height;
	lpDIBh->bmiHeader.biPlanes = 1;
	lpDIBh->bmiHeader.biBitCount = 24;
	lpDIBh->bmiHeader.biCompression = BI_RGB;
	lpDIBh->bmiHeader.biClrUsed = 0;
	// These seem not really needed for our purpose here.
	lpDIBh->bmiHeader.biClrImportant = 0;
	lpDIBh->bmiHeader.biXPelsPerMeter = 0;
	lpDIBh->bmiHeader.biYPelsPerMeter = 0;
	// memory for DIB data
	unsigned char *lpBits;
	lpBits = (unsigned char *)malloc( lpDIBh->bmiHeader.biSizeImage );
	if( !lpBits )
	{
		wxFAIL_MSG( wxT("could not allocate memory for DIB") );
		free( lpDIBh );
		return FALSE;
	}

	// create and set the device-dependent bitmap
	HDC hdc = ::GetDC(NULL);
	HDC memdc = ::CreateCompatibleDC( hdc );
	HBITMAP hbitmap;
	//    hbitmap = ::CreateCompatibleBitmap( hdc, width, bmpHeight );

	//    if(hbitmap == NULL)
	//          int cop =0;

	//    ::SelectObject( memdc, hbitmap);

#if wxUSE_PALETTE
	HPALETTE hOldPalette = 0;
	if (image.GetPalette().Ok())
	{
		hOldPalette = ::SelectPalette(memdc, (HPALETTE) image.GetPalette().GetHPALETTE(), FALSE);
		::RealizePalette(memdc);
	}
#endif // wxUSE_PALETTE

	// copy image data into DIB data and then into DDB (in a loop)
	unsigned char *data = image.GetData();
	int i, j, n;
	int origin = 0;
	unsigned char *ptdata = data;
	unsigned char *ptbits;

	for( n=0; n<numDIB; n++ )
	{
		if( numDIB > 1 && n == numDIB-1 && hRemain > 0 )
		{
			// redefine height and size of the (possibly) last smaller DIB
			// memory is not reallocated
			height = hRemain;
			lpDIBh->bmiHeader.biHeight = (DWORD)(-height);
			lpDIBh->bmiHeader.biSizeImage = bytePerLine*height;
		}
		ptbits = lpBits;

		for( j=0; j<height; j++ )
		{

			memcpy(ptbits, ptdata, width * 3);
			ptbits += width * 3;
			ptdata += width * 3;

			/*
			   for( i=0; i<width; i++ )
			   {
			 *(ptbits++) = *(ptdata+2);
			 *(ptbits++) = *(ptdata+1);
			 *(ptbits++) = *(ptdata  );
			 ptdata += 3;
			 }
			 */
			for( i=0; i< padding; i++ )   *(ptbits++) = 0;
		}
		//        ::StretchDIBits( memdc, 0, origin, width, height,
		//            0, 0, width, height, lpBits, lpDIBh, DIB_RGB_COLORS, SRCCOPY);
		origin += height;
		// if numDIB = 1,  lines below can also be used
		hbitmap = CreateDIBitmap( hdc, &(lpDIBh->bmiHeader), CBM_INIT, lpBits, lpDIBh, DIB_RGB_COLORS );
		//      if(hbitmap == NULL)
		//            int cop =0;

		// The above line is equivalent to the following two lines.
		//    hbitmap = ::CreateCompatibleBitmap( hdc, width, height );
		//    ::SetDIBits( hdc, hbitmap, 0, height, lpBits, lpDIBh, DIB_RGB_COLORS);
		// or the following lines
		//    hbitmap = ::CreateCompatibleBitmap( hdc, width, height );
		//    HDC memdc = ::CreateCompatibleDC( hdc );
		//    ::SelectObject( memdc, hbitmap);
		//    ::SetDIBitsToDevice( memdc, 0, 0, width, height,
		//              0, 0, 0, height, (void *)lpBits, lpDIBh, DIB_RGB_COLORS);
		//    ::SelectObject( memdc, 0 );
		//    ::DeleteDC( memdc );
	}
	SetHBITMAP( (WXHBITMAP) hbitmap );

	//      if(!this->Ok())
	//         int cop = this->Ok();

#if wxUSE_PALETTE
	if (hOldPalette)
		SelectPalette(memdc, hOldPalette, FALSE);
#endif // wxUSE_PALETTE

	// similarly, created an mono-bitmap for the possible mask
	if( image.HasMask() )
	{
		hbitmap = ::CreateBitmap( (WORD)width, (WORD)bmpHeight, 1, 1, NULL );
		HGDIOBJ hbmpOld = ::SelectObject( memdc, hbitmap);
		if( numDIB == 1 )   height = bmpHeight;
		else                height = sizeLimit/bytePerLine;
		lpDIBh->bmiHeader.biHeight = (DWORD)(-height);
		lpDIBh->bmiHeader.biSizeImage = bytePerLine*height;
		origin = 0;
		unsigned char r = image.GetMaskRed();
		unsigned char g = image.GetMaskGreen();
		unsigned char b = image.GetMaskBlue();
		unsigned char zero = 0, one = 255;
		ptdata = data;
		for( n=0; n<numDIB; n++ )
		{
			if( numDIB > 1 && n == numDIB - 1 && hRemain > 0 )
			{
				// redefine height and size of the (possibly) last smaller DIB
				// memory is not reallocated
				height = hRemain;
				lpDIBh->bmiHeader.biHeight = (DWORD)(-height);
				lpDIBh->bmiHeader.biSizeImage = bytePerLine*height;
			}
			ptbits = lpBits;
			for( int j=0; j<height; j++ )
			{
				for(i=0; i<width; i++ )
				{
					// was causing a code gen bug in cw : if( ( cr !=r) || (cg!=g) || (cb!=b) )
					unsigned char cr = (*(ptdata++)) ;
					unsigned char cg = (*(ptdata++)) ;
					unsigned char cb = (*(ptdata++)) ;

					if( ( cr !=r) || (cg!=g) || (cb!=b) )
					{
						*(ptbits++) = one;
						*(ptbits++) = one;
						*(ptbits++) = one;
					}
					else
					{
						*(ptbits++) = zero;
						*(ptbits++) = zero;
						*(ptbits++) = zero;
					}
				}
				for( i=0; i< padding; i++ )   *(ptbits++) = zero;
			}
			::StretchDIBits( memdc, 0, origin, width, height,\
					0, 0, width, height, lpBits, lpDIBh, DIB_RGB_COLORS, SRCCOPY);
			origin += height;
		}
		// create a wxMask object
		wxMask *mask = new wxMask();
		mask->SetMaskBitmap( (WXHBITMAP) hbitmap );
		SetMask( mask );
		// It will be deleted when the wxBitmap object is deleted (as of 01/1999)
		/* The following can also be used but is slow to run
		   wxColour colour( GetMaskRed(), GetMaskGreen(), GetMaskBlue());
		   wxMask *mask = new wxMask( *this, colour );
		   SetMask( mask );
		 */

		::SelectObject( memdc, hbmpOld );
	}

	// free allocated resources
	::DeleteDC( memdc );
	::ReleaseDC(NULL, hdc);
	free(lpDIBh);
	free(lpBits);

#if WXWIN_COMPATIBILITY_2
	// check the wxBitmap object
	GetBitmapData()->SetOk();
#endif // WXWIN_COMPATIBILITY_2

	return TRUE;
}
#endif // __WXMSW__

#endif

