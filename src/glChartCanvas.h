/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __GLCHARTCANVAS_H__
#define __GLCHARTCANVAS_H__

#include <wx/glcanvas.h>
#include <OCPNRegion.h>
#include <ViewPort.h>

class glTextureDescriptor;

namespace chart { class ChartBase; }

WX_DECLARE_HASH_MAP(int, glTextureDescriptor*, wxIntegerHash, wxIntegerEqual, ChartTextureHashType);
WX_DECLARE_HASH_MAP(void *, ChartTextureHashType*, wxPointerHash, wxPointerEqual, ChartPointerHashType);

class glChartCanvas : public wxGLCanvas
{
		DECLARE_EVENT_TABLE()

	public:
		glChartCanvas(wxWindow * parent);
		virtual ~glChartCanvas();

		void SetContext(wxGLContext *pcontext);
		void OnPaint(wxPaintEvent& event);
		void OnEraseBG(wxEraseEvent& evt);
		void render();
		void OnActivate(wxActivateEvent & event);
		void OnSize(wxSizeEvent & event);
		void MouseEvent(wxMouseEvent& event);
		wxString GetRendererString() const;
		void EnablePaint(bool b_enable);
		void Invalidate();
		void RenderRasterChartRegionGL(chart::ChartBase *chart, ViewPort & vp, OCPNRegion &region);
		bool PurgeChartTextures(chart::ChartBase *pc);
		void ClearAllRasterTextures(void);
		void DrawGLOverLayObjects(void);

	protected:
		void RenderQuiltViewGL(const ViewPort & vp, OCPNRegion Region, bool b_clear = true);
		void BuildFBO(void);
		void SetClipRegion(const ViewPort & vp, OCPNRegion &region, bool b_clear);
		void ComputeRenderQuiltViewGLRegion(const ViewPort & vp, OCPNRegion Region);

		wxGLContext * m_pcontext;

		int m_cacheinvalid;
		int max_texture_dimension;

		unsigned char * m_data;
		int m_datasize;

		bool m_bsetup;

		wxString m_renderer;

		void GrowData(int size);

		// This is a hash table
		// key is ChartBaseBSB pointer
		// Value is ChartTextureHashType*
		ChartPointerHashType m_chart_hash;

		ViewPort m_gl_cache_vp;

		bool m_bGenMM;
		bool m_bGL_GEN_MM;
		int m_ntex;
		int m_tex_max_res;
		int m_tex_max_res_initial;
		bool m_b_mem_crunch;
		bool m_b_paint_enable;

		//    For FBO(s)
		bool m_b_useFBO;
		bool m_b_useFBOStencil;
		GLuint m_fb0;
		GLuint m_depth_rb;

		GLenum m_TEX_TYPE;
		GLuint m_cache_tex;
		GLuint m_blit_tex;
		int m_cache_tex_x;
		int m_cache_tex_y;
		OCPNRegion m_gl_rendered_region;
};

#endif
