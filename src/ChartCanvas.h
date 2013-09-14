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

#ifndef __CHARTCANVAS__H__
#define __CHARTCANVAS__H__

#include <wx/datetime.h>
#include <wx/treectrl.h>
#include <wx/dirctrl.h>
#include <wx/sound.h>
#include <wx/grid.h>
#include <wx/wxhtml.h>
#include <wx/dcmemory.h>
#include <wx/image.h>

#include <vector>

#include "CM93DSlide.h"
#include "RolloverWin.h"
#include <ais/AISTargetQueryDialog.h>
#include "glChartCanvas.h"
#include "timers.h"
#include <chart/S57Sector.h>

class LatLonBoundingBox;
class ocpnDC;
class GSHHSChart;

void ShowAISTargetQueryDialog(wxWindow *parent, int mmsi);
int RestoreScreenBrightness(void);

class EmbossData;
class Undo;
class Route;
class TCWin;
class IDX_entry;
class RoutePoint;
class SelectItem;
class ocpnBitmap;
class WVSChart;
class MainFrame;
class ChartBaseBSB;
class ChartBase;
class AIS_Target_Data;
class S57ObjectTree;
class S57ObjectDesc;
class RolloverWin;
class Quilt;
class PixelCache;
class ChInfoWin;
class glChartCanvas;
class TimedPopupWin;

enum //  specify the render behaviour of SetViewPoint()
{
	CURRENT_RENDER,  // use the current render type
	FORCE_SUBSAMPLE  // force sub-sampled render, with re-render timer
};

// Cursor region enumerator
enum
{
	CENTER,
	MID_RIGHT,
	MID_LEFT,
	MID_TOP,
	MID_BOT,
};

enum ownship_state_t
{
	SHIP_NORMAL = 0,
	SHIP_LOWACCURACY,
	SHIP_INVALID
};

enum
{
	ID_S57QUERYTREECTRL = 10000,
	ID_AISDIALOGOK
};

class ChartCanvas : public wxWindow
{
		DECLARE_EVENT_TABLE()
		friend class glChartCanvas;

	public:
		ChartCanvas(wxFrame *frame);
		virtual ~ChartCanvas();

		void OnKeyDown(wxKeyEvent &event);
		void OnKeyUp(wxKeyEvent &event);
		void OnPaint(wxPaintEvent& event);
		void PaintCleanup();
		void Scroll(int dx, int dy);
		void CanvasPopupMenu(int x, int y, int seltype);

		void PopupMenuHandler(wxCommandEvent& event);

		void EnablePaint(bool b_enable);
		virtual bool SetCursor(const wxCursor &c);
		virtual void Refresh(bool eraseBackground = true, const wxRect *rect = NULL);
		virtual void Update();

		void LostMouseCapture(wxMouseCaptureLostEvent& event);

		void CancelMouseRoute();

		void Do_Pankeys(wxTimerEvent& event);
		void EnableAutoPan( bool b_enable );

		bool SetViewPoint(
				double lat,
				double lon,
				double scale_ppm,
				double skew,
				double rotation,
				bool b_adjust = true);

		bool SetVPScale(double sc);
		bool SetViewPoint(double lat, double lon);
		void ReloadVP(bool b_adjust = true);
		void LoadVP(ViewPort &vp, bool b_adjust = true );
		void SetVPRotation(double angle);
		double GetVPRotation(void) const;
		double GetVPSkew(void) const;
		void ClearbFollow(void);

		void GetCanvasPointPix(double rlat, double rlon, wxPoint *r);
		void GetCanvasPixPoint(int x, int y, double &lat, double &lon);
		void WarpPointerDeferred(int x, int y);
		void UpdateShips();
		void UpdateAIS();
		void UpdateAlerts();

		void SetQuiltMode(bool b_quilt);
		bool GetQuiltMode(void);
		std::vector<int> GetQuiltIndexArray(void);
		bool IsQuiltDelta(void);
		void SetQuiltChartHiLiteIndex(int dbIndex);
		int GetQuiltReferenceChartIndex(void);

		int GetNextContextMenuId();

		void SetColorScheme(ColorScheme cs);
		ColorScheme GetColorScheme() const;

		wxString FormatDistanceAdaptive(double distance);

		int GetCanvasWidth() const;
		int GetCanvasHeight() const;
		float GetVPScale() const;
		float GetVPChartScale() const;
		double GetCanvasScaleFactor() const;
		double GetCanvasTrueScale() const;
		double GetAbsoluteMinScalePpm() const;
		ViewPort & GetVP();
		const ViewPort & GetVP() const;
		ChartBase* GetChartAtCursor();
		ChartBase* GetOverlayChartAtCursor();

		glChartCanvas *GetglCanvas();
		GSHHSChart* GetWorldBackgroundChart();

		void SetbTCUpdate(bool f);
		bool GetbTCUpdate() const;
		void SetbShowCurrent(bool f);
		bool GetbShowCurrent() const;
		void SetbShowTide(bool f);
		bool GetbShowTide() const;
		double GetPixPerMM() const;

		void SetOwnShipState(ownship_state_t state);
		void GetCursorLatLon(double *lat, double *lon);

		bool ZoomCanvasIn(double factor);
		bool ZoomCanvasOut(double factor);
		bool DoZoomCanvasIn(double factor);
		bool DoZoomCanvasOut(double factor);

		bool PanCanvas(int dx, int dy);
		void StopAutoPan(void);

		void ShowAISTargetList(void);

		void ShowGoToPosition(void);

		ChartBase *GetLargestScaleQuiltChart();
		ChartBase *GetFirstQuiltChart();
		ChartBase *GetNextQuiltChart();
		int GetQuiltChartCount();
		void InvalidateAllQuiltPatchs(void);
		void SetQuiltRefChart(int dbIndex);
		std::vector<int> GetQuiltCandidatedbIndexArray(bool flag1 = true, bool flag2 = true);
		std::vector<int> GetQuiltExtendedStackdbIndexArray();
		std::vector<int> GetQuiltEclipsedStackdbIndexArray();
		int GetQuiltRefChartdbIndex(void);
		void InvalidateQuilt(void);
		double GetQuiltMaxErrorFactor();
		bool IsChartQuiltableRef(int db_index);
		bool IsChartLargeEnoughToRender( ChartBase* chart, ViewPort& vp );
		int GetCanvasChartNativeScale();
		int FindClosestCanvasChartdbIndex(int scale);
		void UpdateCanvasOnGroupChange(void);


		void ShowChartInfoWindow(int x, int y, int dbIndex);
		void HideChartInfoWindow(void);
		void CancelMeasureRoute();

		//Todo build more accessors
		bool        m_bFollow;
		wxCursor    *pCursorPencil;
		wxCursor    *pCursorArrow;
		wxCursor    *pCursorCross;
		TCWin       *pCwin;
		wxBitmap    *pscratch_bm;
		double      m_cursor_lon, m_cursor_lat;
		Undo        * undo;
		wxPoint     r_rband;
		double      m_prev_rlat;
		double      m_prev_rlon;
		RoutePoint  *m_prev_pMousePoint;

		bool PurgeGLCanvasChartCache(ChartBase *pc);

		void RemovePointFromRoute(RoutePoint * point, Route * route);


	private:
		ViewPort VPoint;
		void PositionConsole(void);
		void FinishRoute(void);
		wxString FindValidUploadPort();

		ChInfoWin * m_pCIWin;

		bool m_bShowCurrent;
		bool m_bShowTide;
		int cursor_region;
		bool m_bTCupdate;

		wxRect bbRect;

		wxPoint LastShipPoint;
		wxPoint LastPredPoint;
		bool m_bDrawingRoute;
		bool m_bRouteEditing;
		bool m_bMarkEditing;
		RoutePoint * m_pRoutePointEditTarget;
		SelectItem * m_pFoundPoint;
		bool m_bChartDragging;
		wxString m_active_upload_port;
		Route * m_pMouseRoute;
		Route * m_pSelectedRoute;
		Route * m_pSelectedTrack;
		wxArrayPtrVoid * m_pEditRouteArray;
		RoutePoint * m_pFoundRoutePoint;
		RoutePoint * m_pFoundRoutePointSecond;

		int m_FoundAIS_MMSI;

		wxCursor * pCursorLeft;
		wxCursor * pCursorRight;
		wxCursor * pCursorUp;
		wxCursor * pCursorDown;

		wxCursor * pCursorUpLeft;
		wxCursor * pCursorUpRight;
		wxCursor * pCursorDownLeft;
		wxCursor * pCursorDownRight;

		int popx;
		int popy;
		bool m_bAppendingRoute;

		wxBitmap * pThumbDIBShow;
		wxBitmap * pThumbShowing;

		bool bShowingCurrent;
		bool bShowingTide;

		double m_canvas_scale_factor;    // converter....
		// useage....
		// true_chart_scale_on_display = m_canvas_scale_factor / pixels_per_meter of displayed chart
		// also may be considered as the "pixels-per-meter" of the canvas on-screen
		double m_absolute_min_scale_ppm;

		int m_panx;
		int m_pany;
		int m_panspeed;
		int m_modkeys;
		bool m_bmouse_key_mod;

		bool singleClickEventIsValid;
		wxMouseEvent singleClickEvent;

		std::vector<s57Sector_t> extendedSectorLegs;

		void OnActivate(wxActivateEvent& event);
		void OnSize(wxSizeEvent& event);
		void MouseTimedEvent(wxTimerEvent& event);
		void MouseEvent(wxMouseEvent& event);
		void ShipDraw(ocpnDC& dc);
		void DrawArrow(ocpnDC& dc, int x, int y, double rot_angle, double scale);
		void OnRolloverPopupTimerEvent(wxTimerEvent & event);
		void FindRoutePointsAtCursor(float selectRadius, bool setBeingEdited);

		void RotateTimerEvent(wxTimerEvent& event);
		void PanTimerEvent(wxTimerEvent& event);
		bool CheckEdgePan(int x, int y, bool bdragging);
		void OnCursorTrackTimerEvent(wxTimerEvent& event);
		void OnZoomTimerEvent(wxTimerEvent& event);

		void DrawAllRoutesInBBox(
				ocpnDC & dc,
				LatLonBoundingBox & BltBBox,
				const wxRegion& clipregion);

		void DrawAllWaypointsInBBox(
				ocpnDC & dc,
				LatLonBoundingBox & BltBBox,
				const wxRegion & clipregion,
				bool bDrawMarksOnly);

		double GetAnchorWatchRadiusPixels(RoutePoint *pAnchorWatchPoint);

		void DrawAllTidesInBBox(
				ocpnDC & dc,
				LatLonBoundingBox & BBox,
				bool bRebuildSelList,
				bool bforce_redraw_tides,
				bool bdraw_mono = false);

		void DrawAllCurrentsInBBox(
				ocpnDC & dc,
				LatLonBoundingBox & BBox,
				bool bRebuildSelList,
				bool bforce_redraw_currents,
				bool bdraw_mono = false);

		void DrawTCWindow(int x, int y, void *pIDX);

		void RenderAllChartOutlines(ocpnDC &dc, ViewPort& vp);
		void RenderChartOutline(ocpnDC &dc, int dbIndex, ViewPort& vp);
		void RenderRouteLegs( ocpnDC &dc );

		wxBitmap *DrawTCCBitmap( wxDC *pbackground_dc, bool bAddNewSelpoints = true);

		void AISDraw(ocpnDC& dc);
		void AISDrawAreaNotices(ocpnDC& dc );
		void AISDrawTarget(AIS_Target_Data *td, ocpnDC& dc );

		void AlertDraw(ocpnDC & dc);

		void TargetFrame(ocpnDC & dc, wxPen pen, int x, int y, int radius);
		void AtoN_Diamond(ocpnDC & dc, wxPen pen, int x, int y, int radius, bool b_virtual);
		void Base_Square(ocpnDC & dc, wxPen pen, int x, int y, int radius);
		void SART_Render(ocpnDC & dc, wxPen pen, int x, int y, int radius);

		void GridDraw(ocpnDC & dc);
		void ScaleBarDraw(ocpnDC & dc);

		void DrawOverlayObjects(ocpnDC & dc, const wxRegion & ru);

		void EmbossDepthScale(ocpnDC & dc);
		EmbossData * CreateEmbossMapData(wxFont & font, int width, int height, const wxChar *str, ColorScheme cs);
		void CreateDepthUnitEmbossMaps(ColorScheme cs);
		wxBitmap CreateDimBitmap(wxBitmap &Bitmap, double factor);

		void CreateOZEmbossMapData(ColorScheme cs);
		void EmbossOverzoomIndicator ( ocpnDC &dc);

		void EmbossCanvas(ocpnDC & dc, EmbossData *pemboss, int x, int y);

		void JaggyCircle(ocpnDC &dc, wxPen pen, int x, int y, int radius);
		void ShowObjectQueryWindow( int x, int y, float zlat, float zlon);
		void ShowMarkPropertiesDialog( RoutePoint* markPoint );
		void ShowRoutePropertiesDialog(wxString title, Route* selected);
		void ShowTrackPropertiesDialog( Route* selected );

		void ShowBrightnessLevelTimedPopup( int brightness, int min, int max );

		int m_canvas_width;
		int m_canvas_height;

		int xr_margin; // chart scroll margins, control cursor, etc.
		int xl_margin;
		int yt_margin;
		int yb_margin;

		MainFrame * parent_frame;

		wxPoint last_drag;

		wxMemoryDC * pmemdc;

		int warp_x;
		int warp_y;
		bool warp_flag;

		float current_draw_scaler;

		wxTimer * pPanTimer;       // This timer used for auto panning on route creation and edit
		wxTimer * pCurTrackTimer;  // This timer used to update the status window on mouse idle
		wxTimer * pRotDefTimer;    // This timer used to control rotaion rendering on mouse moves
		wxTimer * pPanKeyTimer;    // This timer used to update pan key actions
		wxTimer * m_DoubleClickTimer;

		wxTimer m_MouseWheelTimer;
		wxTimer m_RolloverPopupTimer;

		int m_mouse_wheel_oneshot;
		int m_last_wheel_dir;

		int m_curtrack_timer_msec;
		int m_rollover_popup_timer_msec;

		GSHHSChart * pWorldBackgroundChart;

		ChartBaseBSB * pCBSB;
		wxBitmap * pss_overlay_bmp;
		wxMask * pss_overlay_mask;

		wxRect ship_draw_rect;
		wxRect ship_draw_last_rect;
		wxRect ais_draw_rect;
		wxRect alert_draw_rect;          // pjotrc 2010.02.22

		wxBitmap * proute_bm;          // a bitmap and dc used to calculate route bounding box
		wxMemoryDC m_dc_route;         // seen in mouse->edit->route


		EmbossData * m_pEM_Feet;                // maps for depth unit emboss pattern
		EmbossData * m_pEM_Meters;
		EmbossData * m_pEM_Fathoms;

		EmbossData * m_pEM_OverZoom;

		double m_pix_per_mm;     // pixels per millimeter on the screen

		double m_true_scale_ppm;

		ownship_state_t m_ownship_state;

		ColorScheme m_cs;
		bool m_bMeasure_Active;
		int m_nMeasureState;
		Route * m_pMeasureRoute;

		wxBitmap m_bmTideDay;
		wxBitmap m_bmTideDusk;
		wxBitmap m_bmTideNight;
		wxBitmap m_bmCurrentDay;
		wxBitmap m_bmCurrentDusk;
		wxBitmap m_bmCurrentNight;

		RolloverWin * m_pRouteRolloverWin;
		RolloverWin * m_pAISRolloverWin;

		TimedPopupWin * m_pBrightPopup;

		wxImage m_os_image_red_day;
		wxImage m_os_image_red_dusk;
		wxImage m_os_image_red_night;
		wxImage m_os_image_grey_day;
		wxImage m_os_image_grey_dusk;
		wxImage m_os_image_grey_night;

		wxImage * m_pos_image_red;
		wxImage * m_pos_image_grey;

		wxImage * m_pos_image_user;
		wxImage * m_pos_image_user_grey;

		wxImage * m_pos_image_user_day;
		wxImage * m_pos_image_user_dusk;
		wxImage * m_pos_image_user_night;
		wxImage * m_pos_image_user_grey_day;
		wxImage * m_pos_image_user_grey_dusk;
		wxImage * m_pos_image_user_grey_night;

		wxImage m_ship_pix_image;             //cached ship draw image for high overzoom
		int m_cur_ship_pix;
		bool m_cur_ship_pix_isgrey;
		ColorScheme m_ship_cs;

		Quilt * m_pQuilt;

		ViewPort m_cache_vp;
		wxBitmap * m_prot_bm;
		wxPoint m_roffset;

		bool m_b_rot_hidef;

		SelectItem * m_pRolloverRouteSeg;

		double m_wheel_lat, m_wheel_lon;
		int m_wheel_x,m_wheel_y;

		ViewPort m_bm_cache_vp;
		wxBitmap m_working_bm;           // Used to build quilt in OnPaint()
		wxBitmap m_cached_chart_bm;      // A cached copy of the fully drawn quilt

		bool m_bbrightdir;
		int m_brightmod;

		bool m_bzooming;
		IDX_entry * m_pIDXCandidate;

		glChartCanvas * m_glcc;
		wxGLContext * m_pGLcontext;

		//Smooth zoom member variables
		wxTimer m_zoom_timer;
		bool m_bzooming_in;
		bool m_bzooming_out;
		int m_zoomt;                // zoom timer constant, msec
		double m_zoom_target_factor;
		double m_zoom_current_factor;

		bool m_benable_autopan;
		bool m_b_paint_enable;
};

#endif
