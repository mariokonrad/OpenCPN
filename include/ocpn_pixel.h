/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Optimized wxBitmap Object
 * Author:   David Register
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

#ifndef _OCPN_PIXEL_H_
#define _OCPN_PIXEL_H_

#include "dychart.h"                // for configuration stuff

wxImage Image_Rotate(wxImage &base_image, double angle, const wxPoint & centre_of_rotation, bool interpolating, wxPoint * offset_after_rotation);

//--------------------------------------------------------------------------
//      Set the desired compile time conditionals related to display optimization
//--------------------------------------------------------------------------



//          Specify the Pixel Cache type
//          Only one of the following must be selected
//          with due regard for the system type

//#define __PIX_CACHE_WXIMAGE__                               // a safe default
//#define __PIX_CACHE_DIBSECTION__                            // for MSW
//#define __PIX_CACHE_X11IMAGE__                              // for X11/Universal, requires ocpnUSE_ocpnBitmap

//  I use these shortcuts....
#ifdef __WXX11__
#define __PIX_CACHE_WXIMAGE__
//#define     __PIX_CACHE_X11IMAGE__
#endif

#ifdef __WXGTK__
#define __PIX_CACHE_WXIMAGE__
//#define     __PIX_CACHE_X11IMAGE__
//#define __PIX_CACHE_PIXBUF__
#endif

#ifdef __WXMSW__
#define __PIX_CACHE_DIBSECTION__
#define     ocpnUSE_DIBSECTION
#define     ocpnUSE_ocpnBitmap
#endif

#ifdef __WXOSX__
#define __PIX_CACHE_WXIMAGE__
#endif

//    Some configuration sanity checks

//          Use OCPNBitmap (Optimized wxBitmap)
//          Required for X11 native systems, optional on MSW
//          Also required for GTK PixBuf optimized configuration

#ifdef      __PIX_CACHE_X11IMAGE__
#define     ocpnUSE_ocpnBitmap
#endif

#ifdef      __PIX_CACHE_PIXBUF__
#define     ocpnUSE_ocpnBitmap
#define     opcnUSE_GTK_OPTIMIZE
#endif


//          For Optimized X11 systems, use MIT shared memory XImage, requires ocpnUSE_ocpnBitmap
#ifdef __PIX_CACHE_X11IMAGE__
#define ocpUSE_MITSHM
#endif


//          The BitsPerPixel value for chart data storage
//          Todo get this during pixcache ctor
#ifdef __PIX_CACHE_WXIMAGE__                               // a safe default
#define BPP 24
#endif
#ifdef __PIX_CACHE_DIBSECTION__                            // for MSW
#define BPP 24
#endif
#ifdef __PIX_CACHE_X11IMAGE__                              // for X11/Universal
#define BPP 32
#endif
#ifdef __PIX_CACHE_PIXBUF__                                // for GTK Optimized
#define BPP 32
#endif

//    A fall back position is smart....
#ifndef BPP
#define BPP 24
#endif

//      Extended includes
#ifdef __PIX_CACHE_X11IMAGE__
#include "wx/x11/private.h"

//    For MIT-SHM Extensions
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

#ifdef __WXMSW__
#include "wx/msw/dib.h"                     // for ocpnMemDC
#endif

// ============================================================================
// Declarations
// ============================================================================

 typedef enum RGBO
{
    RGB = 0,
    BGR
}_RGBO;

class OCPNBitmap;

#ifdef __PIX_CACHE_X11IMAGE__
//----------------------------------------------------------------------
//      ocpnXImage Definition
//----------------------------------------------------------------------
class ocpnXImage
{
    public:
        ocpnXImage(int width, int height);
        ~ocpnXImage();
        bool PutImage(Pixmap pixmap, GC gc);

        bool            buse_mit;
        XShmSegmentInfo shminfo;
        XImage          *m_img;
        Display         *xdisplay;
        int             xscreen;
        Visual          *xvisual;
        int             bpp;
        int             m_width, m_height;
};
#endif




// ============================================================================
// PixelCache Definition
// ============================================================================
class PixelCache
{
    public:

      //    Constructors

        PixelCache(int width, int height, int depth);
        ~PixelCache();

        void SelectIntoDC(wxMemoryDC &dc);
        void Update(void);
        RGBO GetRGBO(){return m_rgbo;}
        unsigned char *GetpData() const;
        int GetLinePitch() const { return line_pitch_bytes; }
        int GetWidth(void){ return m_width; }
        int GetHeight(void){ return m_height; }

      //    Data storage
    private:
        int               m_width;
        int               m_height;
        int               m_depth;
        int               line_pitch_bytes;
        int               bytes_per_pixel;
        RGBO               m_rgbo;
        unsigned char     *pData;

#ifdef ocpnUSE_ocpnBitmap
      OCPNBitmap         *m_pbm;
#else
      wxBitmap          *m_pbm;
#endif

      wxImage           *m_pimage;

#ifdef __PIX_CACHE_DIBSECTION__
      wxDIB             *m_pDS;
#endif

#ifdef __PIX_CACHE_X11IMAGE__
      XImage            *m_pxim;
      Display           *xdisplay;
      ocpnXImage        *m_pocpnXI;

#endif

#ifdef ocpUSE_MITSHM
      XShmSegmentInfo   *pshminfo;
#endif

#ifdef __PIX_CACHE_PIXBUF__
      unsigned char     *m_pdata;
      GdkPixbuf         *m_pixbuf;
#endif


};


#ifdef ocpnUSE_ocpnBitmap

#ifdef __WXMSW__
	#include "wx/msw/gdiimage.h"
	#include "wx/msw/dib.h"
#endif

#ifdef __WXX11__
	#include "wx/x11/private.h"
#endif

#endif

#endif
