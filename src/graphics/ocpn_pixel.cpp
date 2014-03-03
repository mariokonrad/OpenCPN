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

/*
 * Example of how to use the X Shared Memory extension: MIT_SHM.
 * This code was lifted from my Mesa library.  It hasn't been tested
 * in this form but should be close enough for you to get it working.
 * Beware that this extension isn't available on all systems.  Your
 * application code should use #ifdef's around this code so it can be
 * omitted on systems that don't have it, then fallback to using a regular
 * XImage.
 *
 * Brian Paul  Sep, 20, 1995  brianp@ssec.wisc.edu
 */

#include "ocpn_pixel.h"
#include <graphics/OCPNMemDC.h>

#ifndef WX_PRECOMP
#include <stdio.h>
#include <wx/list.h>
#include <wx/utils.h>
#include <wx/app.h>
#include <wx/palette.h>
#include <wx/dcmemory.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#endif


#ifdef __WXMSW__
#include <wx/msw/private.h>
#include <wx/log.h>
#include <wx/msw/dib.h>
#endif

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/log.h>
#include <wx/image.h>
#include <wx/app.h>
#include <wx/math.h>
#include <wx/gdicmn.h>
#include <wx/palette.h>


// missing from mingw32 header
#ifndef CLR_INVALID
#define CLR_INVALID ((COLORREF)-1)
#endif // no CLR_INVALID


#ifdef ocpnUSE_ocpnBitmap
#ifdef __WXX11__
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif
#endif

extern unsigned int malloc_max;

namespace graphics {

#ifdef  __WXX11__


#ifdef ocpUSE_MITSHM
/*
 * MIT-SHM Test Error handling.
 */
static int MITErrorFlag = 0;
static int HandleXError( Display *dpy, XErrorEvent *event )
{
	MITErrorFlag = 1;
	return 0;
}
#endif

static void* x_malloc(size_t t)
{
	void* pr = malloc(t);

	// malloc fails
	if (NULL == pr) {
		wxLogMessage(_T("x_malloc...malloc fails with request of %d bytes."), t);

		// Cat the /proc/meminfo file

		char* p;
		char buf[2000];
		int len;

		int fd = open("/proc/meminfo", O_RDONLY);

		if (fd == -1)
			exit(1);

		len = read(fd, buf, sizeof(buf) - 1);
		if (len <= 0) {
			close(fd);
			exit(1);
		}
		close(fd);
		buf[len] = 0;

		p = buf;
		while (*p) {
			// printf("%c", *p++);
		}

		exit(0);
		return NULL; // for MSVC
	} else {
		if (t > malloc_max) {
			malloc_max = t;
		}

		return pr; // good return
	}
}

//----------------------------------------------------------------------
//      ocpnXImage Implementation
//----------------------------------------------------------------------

ocpnXImage::ocpnXImage(int width, int height)
{
	m_width = width;
	m_height = height;
	buse_mit = false;
	m_img = NULL;

	xdisplay = (Display*)wxGlobalDisplay();
	xscreen = DefaultScreen(xdisplay);
	xvisual = DefaultVisual(xdisplay, xscreen);
	int bpp = wxTheApp->GetVisualInfo(xdisplay)->m_visualDepth;

#ifdef ocpUSE_MITSHM

	// Check to see if the basic extension is supported
	int ignore;
	bool bMIT_SHM = XQueryExtension(xdisplay, "MIT-SHM", &ignore, &ignore, &ignore);

	if (bMIT_SHM) {
		m_img = XShmCreateImage(xdisplay, xvisual, bpp, ZPixmap, NULL, &shminfo, width, height);
		if (m_img == NULL) {
			wxLogError(_T("XShmCreateImage failed!"));
			goto after_check;
		}

		// Identify and allocate the shared memory buffer
		shminfo.shmid
			= shmget(IPC_PRIVATE, m_img->bytes_per_line * m_img->height, IPC_CREAT | 0777);
		if (shminfo.shmid < 0) {
			XDestroyImage(m_img);
			m_img = NULL;
			wxLogMessage(_T("alloc_back_buffer: Shared memory error (shmget), disabling."));
			goto after_check;
		}

		shminfo.shmaddr = m_img->data = (char*)shmat(shminfo.shmid, 0, 0);

		if (shminfo.shmaddr == (char*)-1) {
			XDestroyImage(m_img);
			m_img = NULL;
			wxLogMessage(_T("shmat failed"));
			goto after_check;
		}

		// Make some further checks
		shminfo.readOnly = False;
		MITErrorFlag = 0;

		XSetErrorHandler(HandleXError);
		// This may trigger the X protocol error we're ready to catch:
		XShmAttach(xdisplay, &shminfo);
		XSync(xdisplay, False);

		if (MITErrorFlag) {
			// we are on a remote display, this error is normal, don't print it
			XFlush(xdisplay);
			MITErrorFlag = 0;
			XDestroyImage(m_img);
			m_img = NULL;
			shmdt(shminfo.shmaddr);
			shmctl(shminfo.shmid, IPC_RMID, 0);
			goto after_check;
		}

		shmctl(shminfo.shmid, IPC_RMID, 0); /* nobody else needs it */

		buse_mit = true; // passed all tests
	}

after_check:
#endif
	if (NULL == m_img) {
		m_img = XCreateImage(xdisplay, xvisual, bpp, ZPixmap, 0, 0, width, height, 32, 0);
		m_img->data = (char*)x_malloc(m_img->bytes_per_line * m_img->height);

		if (m_img->data == NULL) {
			XDestroyImage(m_img);
			m_img = NULL;
			wxLogError(wxT("ocpn_Bitmap:Cannot malloc for data image."));
		}
	}
}

ocpnXImage::~ocpnXImage()
{
#ifdef ocpUSE_MITSHM
	if (buse_mit) {
		XShmDetach(xdisplay, &shminfo);
		XDestroyImage(m_img);
		shmdt(shminfo.shmaddr);
	} else {
		XDestroyImage(m_img);
	}
#else
	XDestroyImage(m_img);
#endif
}

bool ocpnXImage::PutImage(Pixmap pixmap, GC gc)
{
#ifdef ocpUSE_MITSHM
	if (buse_mit)
		XShmPutImage(xdisplay, pixmap, gc, m_img, 0, 0, 0, 0, m_width, m_height, False);
	else
		XPutImage(xdisplay, pixmap, gc, m_img, 0, 0, 0, 0, m_width, m_height);

#else
	XPutImage(xdisplay, pixmap, gc, m_img, 0, 0, 0, 0, m_width, m_height);
#endif

	return true;
}

#endif //  __WXX11__




/*  Class : PixelCache
	Why a specific class for what is, in effect, a simple unsigned char[] ?
Answer: Allow performance optimization for specific platforms,
such as MSW dibSections, and X11 Pixmaps
 */

// ============================================================================
// PixelCache Implementation
// ============================================================================
PixelCache::PixelCache(int width, int height, int depth)
{
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_pbm = NULL;
	m_rgbo = RGB; // default value;
	pData = NULL;

	line_pitch_bytes = bytes_per_pixel = BPP / 8;
	line_pitch_bytes = bytes_per_pixel * width;

#ifdef ocpnUSE_ocpnBitmap
	m_rgbo = BGR;
#endif

#ifdef __PIX_CACHE_PIXBUF__
	m_rgbo = RGB;
#endif

#ifdef __PIX_CACHE_DIBSECTION__
	m_pDS = new wxDIB(width, -height, BPP);
	pData = m_pDS->GetData();
#endif

#ifdef __PIX_CACHE_WXIMAGE__
	m_pimage = new wxImage(m_width, m_height, (bool)FALSE);
	pData = m_pimage->GetData();
#endif

#ifdef __PIX_CACHE_X11IMAGE__
	m_pocpnXI = new ocpnXImage(width, height);
	pData = (unsigned char*)m_pocpnXI->m_img->data;
#endif //__PIX_CACHE_X11IMAGE__

#ifdef __PIX_CACHE_PIXBUF__
	//      m_pbm = new OCPNBitmap((unsigned char *)NULL, m_width, m_height, m_depth);

	///      m_pbm = new OCPNBitmap();
	///      m_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
	///                                           1,
	///                                           8, m_width, m_height);

	///      m_pixbuf = m_pbm->GetPixbuf();
	///      pData = gdk_pixbuf_get_pixels(m_pixbuf);
	///      m_pbm->SetPixbuf(m_pixbuf, 32);

	pData = (unsigned char*)malloc(m_width * m_height * 4);
	///      memset(pData, 255, m_width * m_height * 4);       // set alpha channel to 1
#endif
}

PixelCache::~PixelCache()
{
#ifdef __PIX_CACHE_WXIMAGE__
	delete m_pimage;
	delete m_pbm;
#endif

#ifdef __PIX_CACHE_DIBSECTION__
	delete m_pDS;
#endif

#ifdef __PIX_CACHE_X11IMAGE__
	delete m_pbm;
	delete m_pocpnXI;
#endif

#ifdef __PIX_CACHE_PIXBUF__
	free(pData);
	delete m_pbm;
#endif
}

void PixelCache::Update(void)
{
#ifdef __PIX_CACHE_WXIMAGE__
	delete m_pbm;                       // kill the old one
	m_pbm = NULL;
#endif
}

void PixelCache::SelectIntoDC(wxMemoryDC& dc)
{
#ifdef __PIX_CACHE_DIBSECTION__
	OCPNMemDC* pmdc = dynamic_cast<OCPNMemDC*>(&dc);
	pmdc->SelectObject(*m_pDS);

#endif //__PIX_CACHE_DIBSECTION__

#ifdef __PIX_CACHE_WXIMAGE__
	// delete m_pbm;                       // kill the old one

	// Convert image to bitmap
#ifdef ocpnUSE_ocpnBitmap
	if (!m_pbm)
		m_pbm = new OCPNBitmap(*m_pimage, m_depth);
#else
	if (!m_pbm)
		m_pbm = new wxBitmap(*m_pimage, -1);
#endif

	if (m_pbm)
		dc.SelectObject(*m_pbm);
#endif // __PIX_CACHE_WXIMAGE__

#ifdef __PIX_CACHE_X11IMAGE__
	if (!m_pbm)
		m_pbm = new OCPNBitmap(m_pocpnXI, m_width, m_height, m_depth);
	dc.SelectObject(*m_pbm);
#endif //__PIX_CACHE_X11IMAGE__

#ifdef __PIX_CACHE_PIXBUF__
	if (!m_pbm)
		m_pbm = new OCPNBitmap(pData, m_width, m_height, m_depth);
	if (m_pbm) {
		dc.SelectObject(*m_pbm);
	}
#endif //__PIX_CACHE_PIXBUF__
}

unsigned char* PixelCache::GetpData(void) const
{
	return pData;
}

// Rotation code by Carlos Moreno
// Adapted to static and modified for improved performance by dsr

static const double wxROTATE_EPSILON = 1e-10;

// Auxiliary function to rotate a point (x,y) with respect to point p0
// make it inline and use a straight return to facilitate optimization
// also, the function receives the sine and cosine of the angle to avoid
// repeating the time-consuming calls to these functions -- sin/cos can
// be computed and stored in the calling function.

static inline wxRealPoint wxRotatePoint(const wxRealPoint& p, double cos_angle, double sin_angle,
										const wxRealPoint& p0)
{
	return wxRealPoint(p0.x + (p.x - p0.x) * cos_angle - (p.y - p0.y) * sin_angle,
					   p0.y + (p.y - p0.y) * cos_angle + (p.x - p0.x) * sin_angle);
}

static inline wxRealPoint wxRotatePoint(double x, double y, double cos_angle, double sin_angle,
										const wxRealPoint& p0)
{
	return wxRotatePoint(wxRealPoint(x, y), cos_angle, sin_angle, p0);
}

wxImage Image_Rotate(wxImage& base_image, double angle, const wxPoint& centre_of_rotation,
					 bool interpolating, wxPoint* offset_after_rotation)
{
	int i;
	angle = -angle; // screen coordinates are a mirror image of "real" coordinates

	bool has_alpha = base_image.HasAlpha();

	const int w = base_image.GetWidth(), h = base_image.GetHeight();

	// Create pointer-based array to accelerate access to wxImage's data
	unsigned char** data = new unsigned char* [h];
	data[0] = base_image.GetData();
	for (i = 1; i < h; i++)
		data[i] = data[i - 1] + (3 * w);

	// Same for alpha channel
	unsigned char** alpha = NULL;
	if (has_alpha) {
		alpha = new unsigned char* [h];
		alpha[0] = base_image.GetAlpha();
		for (i = 1; i < h; i++)
			alpha[i] = alpha[i - 1] + w;
	}

	// precompute coefficients for rotation formula
	// (sine and cosine of the angle)
	const double cos_angle = cos(angle);
	const double sin_angle = sin(angle);

	// Create new Image to store the result
	// First, find rectangle that covers the rotated image;  to do that,
	// rotate the four corners

	const wxRealPoint p0(centre_of_rotation.x, centre_of_rotation.y);

	wxRealPoint p1 = wxRotatePoint(0, 0, cos_angle, sin_angle, p0);
	wxRealPoint p2 = wxRotatePoint(0, h, cos_angle, sin_angle, p0);
	wxRealPoint p3 = wxRotatePoint(w, 0, cos_angle, sin_angle, p0);
	wxRealPoint p4 = wxRotatePoint(w, h, cos_angle, sin_angle, p0);

	int x1a = (int)floor(wxMin(wxMin(p1.x, p2.x), wxMin(p3.x, p4.x)));
	int y1a = (int)floor(wxMin(wxMin(p1.y, p2.y), wxMin(p3.y, p4.y)));
	int x2a = (int)ceil(wxMax(wxMax(p1.x, p2.x), wxMax(p3.x, p4.x)));
	int y2a = (int)ceil(wxMax(wxMax(p1.y, p2.y), wxMax(p3.y, p4.y)));

	// Create rotated image
	wxImage rotated(x2a - x1a + 1, y2a - y1a + 1, false);
	// With alpha channel
	if (has_alpha)
		rotated.SetAlpha();

	if (offset_after_rotation != NULL) {
		*offset_after_rotation = wxPoint(x1a, y1a);
	}

	// GRG: The rotated (destination) image is always accessed
	//      sequentially, so there is no need for a pointer-based
	//      array here (and in fact it would be slower).
	//
	unsigned char* dst = rotated.GetData();

	unsigned char* alpha_dst = NULL;
	if (has_alpha)
		alpha_dst = rotated.GetAlpha();

	// GRG: if the original image has a mask, use its RGB values
	//      as the blank pixel, else, fall back to default (black).
	//
	unsigned char blank_r = 0;
	unsigned char blank_g = 0;
	unsigned char blank_b = 0;

	if (base_image.HasMask()) {
		blank_r = base_image.GetMaskRed();
		blank_g = base_image.GetMaskGreen();
		blank_b = base_image.GetMaskBlue();
		rotated.SetMaskColour(blank_r, blank_g, blank_b);
	}

	// Now, for each point of the rotated image, find where it came from, by
	// performing an inverse rotation (a rotation of -angle) and getting the
	// pixel at those coordinates

	const int rH = rotated.GetHeight();
	const int rW = rotated.GetWidth();

	// GRG: I've taken the (interpolating) test out of the loops, so that
	//      it is done only once, instead of repeating it for each pixel.

	if (interpolating) {
		for (int y = 0; y < rH; y++) {
			for (int x = 0; x < rW; x++) {
				wxRealPoint src = wxRotatePoint(x + x1a, y + y1a, cos_angle, -sin_angle, p0);

				if (-0.25 < src.x && src.x < w - 0.75 && -0.25 < src.y && src.y < h - 0.75) {
					// interpolate using the 4 enclosing grid-points.  Those
					// points can be obtained using floor and ceiling of the
					// exact coordinates of the point
					int x1, y1, x2, y2;

					if (0 < src.x && src.x < w - 1) {
						x1 = wxRound(floor(src.x));
						x2 = wxRound(ceil(src.x));
					} else {
						// else means that x is near one of the borders (0 or width-1)
						x1 = x2 = wxRound(src.x);
					}

					if (0 < src.y && src.y < h - 1) {
						y1 = wxRound(floor(src.y));
						y2 = wxRound(ceil(src.y));
					} else {
						y1 = y2 = wxRound(src.y);
					}

					// get four points and the distances (square of the distance,
					// for efficiency reasons) for the interpolation formula

					// GRG: Do not calculate the points until they are
					//      really needed -- this way we can calculate
					//      just one, instead of four, if d1, d2, d3
					//      or d4 are < wxROTATE_EPSILON

					const double d1 = (src.x - x1) * (src.x - x1) + (src.y - y1) * (src.y - y1);
					const double d2 = (src.x - x2) * (src.x - x2) + (src.y - y1) * (src.y - y1);
					const double d3 = (src.x - x2) * (src.x - x2) + (src.y - y2) * (src.y - y2);
					const double d4 = (src.x - x1) * (src.x - x1) + (src.y - y2) * (src.y - y2);

					// Now interpolate as a weighted average of the four surrounding
					// points, where the weights are the distances to each of those points

					// If the point is exactly at one point of the grid of the source
					// image, then don't interpolate -- just assign the pixel

					// d1,d2,d3,d4 are positive -- no need for abs()
					if (d1 < wxROTATE_EPSILON) {
						unsigned char* p = data[y1] + (3 * x1);
						*(dst++) = *(p++);
						*(dst++) = *(p++);
						*(dst++) = *p;

						if (has_alpha)
							*(alpha_dst++) = *(alpha[y1] + x1);
					} else if (d2 < wxROTATE_EPSILON) {
						unsigned char* p = data[y1] + (3 * x2);
						*(dst++) = *(p++);
						*(dst++) = *(p++);
						*(dst++) = *p;

						if (has_alpha)
							*(alpha_dst++) = *(alpha[y1] + x2);
					} else if (d3 < wxROTATE_EPSILON) {
						unsigned char* p = data[y2] + (3 * x2);
						*(dst++) = *(p++);
						*(dst++) = *(p++);
						*(dst++) = *p;

						if (has_alpha)
							*(alpha_dst++) = *(alpha[y2] + x2);
					} else if (d4 < wxROTATE_EPSILON) {
						unsigned char* p = data[y2] + (3 * x1);
						*(dst++) = *(p++);
						*(dst++) = *(p++);
						*(dst++) = *p;

						if (has_alpha)
							*(alpha_dst++) = *(alpha[y2] + x1);
					} else {
						// weights for the weighted average are proportional to the inverse of the
						// distance
						unsigned char* v1 = data[y1] + (3 * x1);
						unsigned char* v2 = data[y1] + (3 * x2);
						unsigned char* v3 = data[y2] + (3 * x2);
						unsigned char* v4 = data[y2] + (3 * x1);

						const double w1 = 1 / d1, w2 = 1 / d2, w3 = 1 / d3, w4 = 1 / d4;

						// GRG: Unrolled.

						*(dst++) = (unsigned char)((w1 * *(v1++) + w2 * *(v2++) + w3 * *(v3++)
													+ w4 * *(v4++)) / (w1 + w2 + w3 + w4));
						*(dst++) = (unsigned char)((w1 * *(v1++) + w2 * *(v2++) + w3 * *(v3++)
													+ w4 * *(v4++)) / (w1 + w2 + w3 + w4));
						*(dst++) = (unsigned char)((w1 * *v1 + w2 * *v2 + w3 * *v3 + w4 * *v4)
												   / (w1 + w2 + w3 + w4));

						if (has_alpha) {
							v1 = alpha[y1] + (x1);
							v2 = alpha[y1] + (x2);
							v3 = alpha[y2] + (x2);
							v4 = alpha[y2] + (x1);

							*(alpha_dst++) = (unsigned char)((w1 * *v1 + w2 * *v2 + w3 * *v3
															  + w4 * *v4) / (w1 + w2 + w3 + w4));
						}
					}
				} else {
					*(dst++) = blank_r;
					*(dst++) = blank_g;
					*(dst++) = blank_b;

					if (has_alpha)
						*(alpha_dst++) = 0;
				}
			}
		}
	} else {
		// not interpolating
		double x0 = p0.x;
		double y0 = p0.y;
		double x1b = x1a - p0.x;
		double y1b = y1a - p0.y;
		double msa = -sin_angle;

		for (int y = 0; y < rH; y++) {
			for (int x = 0; x < rW; x++) {
				// wxRealPoint src = wxRotatePoint (x + x1a, y + y1a, cos_angle, -sin_angle, p0);

				// double sx = p0.x + (x + x1a - p0.x) * cos_angle - (y + y1a - p0.y) * -sin_angle;
				// double sy=  p0.y + (y + y1a - p0.y) * cos_angle + (x + x1a - p0.x) * -sin_angle;

				double sx = x0 + (x + x1b) * cos_angle - (y + y1b) * msa;
				double sy = y0 + (y + y1b) * cos_angle + (x + x1b) * msa;

				const int xs = (int)sx;
				const int ys = (int)sy;

				// return wxRealPoint(p0.x + (p.x - p0.x) * cos_angle - (p.y - p0.y) * sin_angle,
				// p0.y + (p.y - p0.y) * cos_angle + (p.x - p0.x) * sin_angle);

				// const int xs = /*wxRound*/ (src.x);      // wxRound rounds to the
				// const int ys = /*wxRound*/ (src.y);      // closest integer

				if (0 <= xs && xs < w && 0 <= ys && ys < h) {
					unsigned char* p = data[ys] + (3 * xs);
					*(dst++) = *(p++);
					*(dst++) = *(p++);
					*(dst++) = *p;

					if (has_alpha)
						*(alpha_dst++) = *(alpha[ys] + (xs));
				} else {
					*(dst++) = blank_r;
					*(dst++) = blank_g;
					*(dst++) = blank_b;

					if (has_alpha)
						*(alpha_dst++) = 255;
				}
			}
		}
	}

	delete[] data;

	if (has_alpha)
		delete[] alpha;

	return rotated;
}

}

