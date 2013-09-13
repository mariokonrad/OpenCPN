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

#ifndef _ROUTEPROP_H_
#define _ROUTEPROP_H_

#include <wx/listctrl.h>
#include <wx/hyperlink.h>
#include <wx/choice.h>
#include <wx/tglbtn.h>
#include <wx/bmpcbox.h>
#include <wx/notebook.h>
#include <wx/filesys.h>

#include "LinkPropDlg.h"

#if wxCHECK_VERSION(2, 9, 0)
	#include <wx/dialog.h>
#else
	#include "scrollingdialog.h"
#endif

class wxListCtrl;
class Route;
class RoutePoint;

// FIXME: there seem to exist two definitions of (some) constants:

////@begin control identifiers
#define ID_ROUTEPROP 7000
#define SYMBOL_ROUTEPROP_TITLE _("Route Properties")
#define SYMBOL_ROUTEPROP_SIZE wxSize(450, 300)

#define ID_TEXTCTRL            7001
#define ID_TEXTCTRL2           7002
#define ID_TEXTCTRL1           7003
#define ID_TEXTCTRL3           7005
#define ID_LISTCTRL            7004
#define ID_ROUTEPROP_CANCEL    7006
#define ID_ROUTEPROP_OK        7007
#define ID_ROUTEPROP_SPLIT     7107
#define ID_ROUTEPROP_EXTEND    7207
#define ID_ROUTEPROP_COPYTXT   7307
#define ID_ROUTEPROP_PRINT     7407
#define ID_PLANSPEEDCTL        7008
#define ID_TEXTCTRL4           7009
#define ID_TEXTCTRLDESC        7010
#define ID_STARTTIMECTL        7011
#define ID_TIMEZONESEL         7012
#define ID_TRACKLISTCTRL       7013
#define ID_RCLK_MENU_COPY_TEXT 7014
#define ID_RCLK_MENU_EDIT_WP   7015
#define ID_RCLK_MENU_DELETE    7016
#define ID_RCLK_MENU_COPY      7017
#define ID_RCLK_MENU_COPY_LL   7018
#define ID_RCLK_MENU_PASTE     7019
#define ID_RCLK_MENU_PASTE_LL  7020

#define ID_MARKPROP 8000
#define SYMBOL_MARKPROP_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_MARKPROP_TITLE _("Waypoint Properties")
#define SYMBOL_MARKPROP_IDNAME ID_MARKPROP
#define SYMBOL_MARKPROP_SIZE wxSize(200, 300)
#define SYMBOL_MARKPROP_POSITION wxDefaultPosition
#define ID_MARKPROP_CANCEL 8001
#define ID_MARKPROP_OK 8002
#define ID_ICONCTRL 8003
#define ID_LATCTRL 8004
#define ID_LONCTRL 8005
#define ID_SHOWNAMECHECKBOX1 8006
////@end control identifiers

#ifndef wxCLOSE_BOX
	#define wxCLOSE_BOX 0x1000
#endif

#ifndef wxFIXED_MINSIZE
	#define wxFIXED_MINSIZE 0
#endif


class RouteProp: public wxDialog
{
	DECLARE_DYNAMIC_CLASS( RouteProp )
		DECLARE_EVENT_TABLE()

	public:
		/// Constructors
		RouteProp( );
		RouteProp(wxWindow* parent, wxWindowID id = ID_ROUTEPROP,
				const wxString& caption = SYMBOL_ROUTEPROP_TITLE,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = SYMBOL_ROUTEPROP_SIZE,
				long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

		virtual ~RouteProp();

		/// Creation
		bool Create( wxWindow* parent, wxWindowID id = ID_ROUTEPROP,
				const wxString& caption = SYMBOL_ROUTEPROP_TITLE,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = SYMBOL_ROUTEPROP_SIZE,
				long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

		void CreateControls();

		void SetColorScheme(ColorScheme cs);
		void SetDialogTitle(const wxString & title);
		void OnRoutepropCancelClick( wxCommandEvent& event );
		void OnRoutepropOkClick( wxCommandEvent& event );
		void OnPlanSpeedCtlUpdated( wxCommandEvent& event );
		void OnStartTimeCtlUpdated( wxCommandEvent& event );
		void OnTimeZoneSelected( wxCommandEvent& event );
		void OnRoutepropListClick( wxListEvent& event );
		void OnRoutepropSplitClick( wxCommandEvent& event );
		void OnRoutepropExtendClick( wxCommandEvent& event );
		void OnRoutepropPrintClick( wxCommandEvent& event );
		void OnRoutepropCopyTxtClick( wxCommandEvent& event );
		void OnRoutePropRightClick( wxListEvent &event );
		void OnRoutePropMenuSelected( wxCommandEvent &event );
		bool IsThisRouteExtendable();
		void OnEvtColDragEnd(wxListEvent& event);
		void InitializeList();


		/// Should we show tooltips?
		static bool ShowToolTips();

		void SetRouteAndUpdate(Route *pR);
		Route *GetRoute(void){return m_pRoute;}

		bool UpdateProperties(void);
		wxString MakeTideInfo(int jx, time_t tm, int tz_selection, long LMT_Offset);
		bool SaveChanges(void);

		wxTextCtrl  *m_TotalDistCtl;
		wxTextCtrl  *m_PlanSpeedCtl;
		wxTextCtrl	*m_StartTimeCtl;
		wxTextCtrl  *m_TimeEnrouteCtl;

		wxStaticText *m_PlanSpeedLabel;
		wxStaticText *m_StartTimeLabel;

		wxTextCtrl  *m_RouteNameCtl;
		wxTextCtrl  *m_RouteStartCtl;
		wxTextCtrl  *m_RouteDestCtl;

		wxListCtrl        *m_wpList;

		wxButton*     m_CancelButton;
		wxButton*     m_OKButton;
		wxButton*     m_CopyTxtButton;
		wxButton*     m_PrintButton;
		wxButton*     m_ExtendButton;
		wxButton*     m_SplitButton;

		Route       *m_pRoute;
		Route       *m_pHead; // for route splitting
		Route       *m_pTail;
		RoutePoint *m_pExtendPoint;
		Route *m_pExtendRoute;
		RoutePoint    *m_pEnroutePoint;
		bool          m_bStartNow;

		double      m_planspeed;
		double      m_avgspeed;

		int         m_nSelected; // index of point selected in Properties dialog row
		int         m_tz_selection;

		wxDateTime	 m_starttime; // kept as UTC
		wxRadioBox	*pDispTz;
		wxStaticText  *m_staticText1;
		wxStaticText  *m_staticText2;
		wxStaticText  *m_staticText3;
		wxChoice      *m_chColor;
		wxChoice      *m_chStyle;
		wxChoice      *m_chWidth;

		wxStaticBoxSizer* m_pListSizer;
};

#endif
