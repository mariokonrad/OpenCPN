/**************************************************************************
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

#include "RouteProp.h"
#include <Select.h>
#include <Track.h>
#include <Routeman.h>
#include <WayPointman.h>
#include <RouteManagerDialog.h>
#include <MessageBox.h>
#include <RoutePrintSelection.h>
#include <PositionParser.h>
#include <Config.h>
#include <ChartCanvas.h>
#include <MainFrame.h>
#include <DimeControl.h>
#include <Units.h>
#include <MagneticVariation.h>
#include <Daylight.h>

#include <geo/GeoRef.h>

#include <tide/IDX_entry.h>
#include <tide/TCMgr.h>

#include <plugin/PlugInManager.h>

#include <global/OCPN.h>
#include <global/Navigation.h>

#include <gpx/gpx.h>

#include <wx/datetime.h>
#include <wx/clipbrd.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/stattext.h>

extern double g_PlanSpeed;
extern wxDateTime g_StartTime;
extern int g_StartTimeTZ;
extern tide::IDX_entry* gpIDX;
extern tide::TCMgr* ptcmgr;
extern long gStart_LMT_Offset;
extern Config* pConfig;
extern WayPointman* pWayPointMan;
extern ChartCanvas* cc1;
extern Select* pSelect;
extern Routeman* g_pRouteMan;
extern RouteManagerDialog* pRouteManagerDialog;
extern Track* g_pActiveTrack;
extern RouteList* pRouteList;
extern PlugInManager* g_pi_manager;
extern bool g_bShowMag;
extern MainFrame* gFrame;

// Global print route selection dialog
RoutePrintSelection* pRoutePrintSelection = NULL;

#define UTCINPUT         0
#define LTINPUT          1 // i.e. this PC local time
#define LMTINPUT         2 // i.e. the remote location LMT time
#define INPUT_FORMAT     1
#define DISPLAY_FORMAT   2
#define TIMESTAMP_FORMAT 3

wxString ts2s(wxDateTime ts, int tz_selection, long LMT_offset, int format)
{
	wxString s = _T("");
	wxString f;
	if (format == INPUT_FORMAT)
		f = _T("%m/%d/%Y %H:%M");
	else if (format == TIMESTAMP_FORMAT)
		f = _T("%m/%d/%Y %H:%M:%S");
	else
		f = _T(" %m/%d %H:%M");
	switch (tz_selection) {
		case 0:
			s.Append(ts.Format(f));
			if (format != INPUT_FORMAT)
				s.Append(_T(" UT"));
			break;
		case 1:
			s.Append(ts.FromUTC().Format(f));
			break;
		case 2:
			wxTimeSpan lmt(0, 0, (int)LMT_offset, 0);
			s.Append(ts.Add(lmt).Format(f));
			if (format != INPUT_FORMAT)
				s.Append(_T(" LMT"));
	}
	return s;
}

IMPLEMENT_DYNAMIC_CLASS( RouteProp, wxDialog )

BEGIN_EVENT_TABLE( RouteProp, wxDialog )
	EVT_TEXT( ID_PLANSPEEDCTL, RouteProp::OnPlanSpeedCtlUpdated )
	EVT_TEXT_ENTER( ID_STARTTIMECTL, RouteProp::OnStartTimeCtlUpdated )
	EVT_RADIOBOX ( ID_TIMEZONESEL, RouteProp::OnTimeZoneSelected )
	EVT_BUTTON( ID_ROUTEPROP_CANCEL, RouteProp::OnRoutepropCancelClick )
	EVT_BUTTON( ID_ROUTEPROP_OK, RouteProp::OnRoutepropOkClick )
	EVT_LIST_ITEM_SELECTED( ID_LISTCTRL, RouteProp::OnRoutepropListClick )
	EVT_LIST_ITEM_SELECTED( ID_TRACKLISTCTRL, RouteProp::OnRoutepropListClick )
	EVT_BUTTON( ID_ROUTEPROP_SPLIT, RouteProp::OnRoutepropSplitClick )
	EVT_BUTTON( ID_ROUTEPROP_EXTEND, RouteProp::OnRoutepropExtendClick )
	EVT_BUTTON( ID_ROUTEPROP_PRINT, RouteProp::OnRoutepropPrintClick )
END_EVENT_TABLE()

RouteProp::RouteProp()
{
}

RouteProp::RouteProp(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos,
					 const wxSize& size, long style)
	: m_TotalDistCtl(NULL)
	, m_wpList(NULL)
	, m_pRoute(NULL)
	, m_pHead(NULL)
	, m_pTail(NULL)
	, m_pEnroutePoint(NULL)
	, m_bStartNow(false)
	, m_nSelected(0)
{
	long wstyle = style;
#ifdef __WXOSX__
	wstyle |= wxSTAY_ON_TOP;
#endif

	Create(parent, id, caption, pos, size, wstyle);
	GetSizer()->SetSizeHints(this);
	Centre();
}

void RouteProp::OnRoutePropRightClick(wxListEvent&)
{
	wxMenu menu;

	if (m_pRoute->m_bIsTrack) {
		// No track specific items so far.
	} else {
		if (!m_pRoute->m_bIsInLayer) {
			wxMenuItem* editItem = menu.Append(ID_RCLK_MENU_EDIT_WP, _("&Waypoint Properties..."));
			editItem->Enable(m_wpList->GetSelectedItemCount() == 1);

			wxMenuItem* delItem = menu.Append(ID_RCLK_MENU_DELETE, _("&Remove Selected"));
			delItem->Enable(m_wpList->GetSelectedItemCount() > 0 && m_wpList->GetItemCount() > 2);
		}
	}

	menu.Append(ID_RCLK_MENU_COPY_TEXT, _("&Copy all as text"));
	PopupMenu(&menu);
}

const Route* RouteProp::getRoute() const
{
	return m_pRoute;
}

const RoutePoint* RouteProp::getEnroutePoint() const
{
	return m_pEnroutePoint;
}

void RouteProp::setEnroutePoint(RoutePoint* point)
{
	m_pEnroutePoint = point;
}

double RouteProp::getPlanSpeed() const
{
	return m_planspeed;
}

void RouteProp::OnRoutepropSplitClick(wxCommandEvent&)
{
	m_SplitButton->Enable(false);

	if (m_pRoute->m_bIsInLayer)
		return;

	if ((m_nSelected > 1) && (m_nSelected < m_pRoute->GetnPoints())) {
		if (m_pRoute->m_bIsTrack) {
			m_pHead = new Track();
			m_pTail = new Track();
			m_pHead->CloneTrack(m_pRoute, 1, m_nSelected, _("_A"));
			m_pTail->CloneTrack(m_pRoute, m_nSelected, m_pRoute->GetnPoints(), _("_B"));
		} else {
			m_pHead = new Route();
			m_pTail = new Route();
			m_pHead->CloneRoute(m_pRoute, 1, m_nSelected, _("_A"));
			m_pTail->CloneRoute(m_pRoute, m_nSelected, m_pRoute->GetnPoints(), _("_B"));
		}
		pRouteList->push_back(m_pHead);
		pConfig->AddNewRoute(m_pHead, -1);
		m_pHead->RebuildGUIDList();

		pRouteList->push_back(m_pTail);
		pConfig->AddNewRoute(m_pTail, -1);
		m_pTail->RebuildGUIDList();

		pConfig->DeleteConfigRoute(m_pRoute);

		if (!m_pTail->m_bIsTrack) {
			pSelect->DeleteAllSelectableRoutePoints(m_pRoute);
			pSelect->DeleteAllSelectableRouteSegments(m_pRoute);
			g_pRouteMan->DeleteRoute(m_pRoute);
			pSelect->AddAllSelectableRouteSegments(m_pTail);
			pSelect->AddAllSelectableRoutePoints(m_pTail);
			pSelect->AddAllSelectableRouteSegments(m_pHead);
			pSelect->AddAllSelectableRoutePoints(m_pHead);
		} else {
			pSelect->DeleteAllSelectableTrackSegments(m_pRoute);
			m_pRoute->ClearHighlights();
			g_pRouteMan->DeleteTrack(m_pRoute);
			pSelect->AddAllSelectableTrackSegments(m_pTail);
			pSelect->AddAllSelectableTrackSegments(m_pHead);
		}

		SetRouteAndUpdate(m_pTail);
		UpdateProperties();

		if (pRouteManagerDialog && pRouteManagerDialog->IsShown()) {
			if (!m_pTail->m_bIsTrack)
				pRouteManagerDialog->UpdateRouteListCtrl();
			else
				pRouteManagerDialog->UpdateTrkListCtrl();
		}
	}
}

// slot on pressed button "Print Route" with selection of the route properties to print
void RouteProp::OnRoutepropPrintClick(wxCommandEvent&)
{
	if (!pRoutePrintSelection)
		pRoutePrintSelection = new RoutePrintSelection(GetParent(), m_pRoute);

	if (!pRoutePrintSelection->IsShown())
		pRoutePrintSelection->ShowModal();
	delete pRoutePrintSelection;
	pRoutePrintSelection = NULL;
}

void RouteProp::OnRoutepropExtendClick(wxCommandEvent&)
{
	m_ExtendButton->Enable(false);

	if (m_pRoute->m_bIsTrack)
		return;

	if (IsThisRouteExtendable()) {
		int fm = m_pExtendRoute->GetIndexOf(m_pExtendPoint) + 1;
		int to = m_pExtendRoute->GetnPoints();
		if (fm <= to) {
			pSelect->DeleteAllSelectableRouteSegments(m_pRoute);
			m_pRoute->CloneRoute(m_pExtendRoute, fm, to, _("_plus"));
			pSelect->AddAllSelectableRouteSegments(m_pRoute);
			SetRouteAndUpdate(m_pRoute);
			UpdateProperties();
		}
	}
}

void RouteProp::OnRoutepropCopyTxtClick(wxCommandEvent&)
{
	wxString tab("\t", wxConvUTF8);
	wxString eol("\n", wxConvUTF8);
	wxString csvString;

	csvString << this->GetTitle() << eol << _("Name") << tab << m_pRoute->m_RouteNameString << eol
			  << _("Depart From") << tab << m_pRoute->m_RouteStartString << eol << _("Destination")
			  << tab << m_pRoute->m_RouteEndString << eol << _("Total Distance") << tab
			  << m_TotalDistCtl->GetValue() << eol << _("Speed (Kts)") << tab
			  << m_PlanSpeedCtl->GetValue() << eol << _("Departure Time (m/d/y h:m)") << tab
			  << m_StartTimeCtl->GetValue() << eol << _("Time Enroute") << tab
			  << m_TimeEnrouteCtl->GetValue() << eol << eol;

	int noCols = m_wpList->GetColumnCount();
	int noRows = m_wpList->GetItemCount();
	wxListItem item;
	item.SetMask(wxLIST_MASK_TEXT);

	for (int i = 0; i < noCols; i++) {
		m_wpList->GetColumn(i, item);
		csvString << item.GetText() << tab;
	}
	csvString << eol;

	for (int j = 0; j < noRows; j++) {
		item.SetId(j);
		for (int i = 0; i < noCols; i++) {
			item.SetColumn(i);
			m_wpList->GetItem(item);
			csvString << item.GetText() << tab;
		}
		csvString << eol;
	}

	if (wxTheClipboard->Open()) {
		wxTextDataObject* data = new wxTextDataObject;
		data->SetText(csvString);
		wxTheClipboard->SetData(data);
		wxTheClipboard->Close();
	}
}

bool RouteProp::IsThisRouteExtendable()
{
	m_pExtendRoute = NULL;
	m_pExtendPoint = NULL;

	if (m_pRoute->m_bRtIsActive || m_pRoute->m_bIsInLayer)
		return false;

	if (!m_pRoute->m_bIsTrack) {
		RoutePoint* pLastPoint = m_pRoute->GetLastPoint();
		Routeman::RouteArray* pEditRouteArray = g_pRouteMan->GetRouteArrayContaining(pLastPoint);

		// remove invisible & own routes from choices
		for (int i = pEditRouteArray->size(); i > 0; --i) {
			Route* route = static_cast<Route*>(pEditRouteArray->at(i - 1));
			if (!route->IsVisible() || (route->m_GUID == m_pRoute->m_GUID))
				pEditRouteArray->erase(pEditRouteArray->begin() + i - 1); // FIXME: altering container while iterating
		}

		if (pEditRouteArray->size() == 1) {
			m_pExtendPoint = pLastPoint;
		} else {
			if (pEditRouteArray->size() == 0) {

				int nearby_radius_meters = static_cast<int>(8.0 / cc1->GetCanvasTrueScale());
				Position rpos = pLastPoint->get_position();

				m_pExtendPoint = pWayPointMan->GetOtherNearbyWaypoint(rpos, nearby_radius_meters,
																	  pLastPoint->m_GUID);
				if (m_pExtendPoint && !m_pExtendPoint->m_bIsInTrack) {
					Routeman::RouteArray* pCloseWPRouteArray
						= g_pRouteMan->GetRouteArrayContaining(m_pExtendPoint);
					if (pCloseWPRouteArray) {
						pEditRouteArray = pCloseWPRouteArray; // FIXME: the original pEditRouteArray
															  // ist lost, MEMORY LEAK

						// remove invisible & own routes from choices
						for (int i = pEditRouteArray->size(); i > 0; --i) {
							Route* route = static_cast<Route*>(pEditRouteArray->at(i - 1));
							// FIXME: altering container while iterating
							if (!route->IsVisible() || (route->m_GUID == m_pRoute->m_GUID))
								pEditRouteArray->erase(pEditRouteArray->begin() + i - 1);
						}
					}
				}
			}
		}

		if (pEditRouteArray->size() == 1) {
			Route* route = static_cast<Route*>(pEditRouteArray->at(0));
			int fm = route->GetIndexOf(m_pExtendPoint) + 1;
			int to = route->GetnPoints();
			if (fm <= to) {
				m_pExtendRoute = route;
				delete pEditRouteArray;
				return true;
			}
		}
		delete pEditRouteArray;
	}
	return false;
}

RouteProp::~RouteProp()
{
	delete m_TotalDistCtl;
	delete m_PlanSpeedCtl;
	delete m_TimeEnrouteCtl;
	delete m_RouteNameCtl;
	delete m_RouteStartCtl;
	delete m_RouteDestCtl;
	delete m_StartTimeCtl;
	delete m_wpList;

	// delete global print route selection dialog
	delete pRoutePrintSelection;
	pRoutePrintSelection = NULL;
}

bool RouteProp::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos,
					   const wxSize& size, long style)
{
	SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
	wxDialog::Create(parent, id, caption, pos, size, style);
	CreateControls();
	return true;
}

void RouteProp::CreateControls()
{
	////@begin RouteProp content construction

	RouteProp* itemDialog1 = this;

	wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
	itemDialog1->SetSizer(itemBoxSizer2);

	wxStaticBox* itemStaticBoxSizer3Static
		= new wxStaticBox(itemDialog1, wxID_ANY, _("Properties"));
	wxStaticBoxSizer* itemStaticBoxSizer3
		= new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
	itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxEXPAND | wxALL, 5);

	wxStaticText* itemStaticText4 = new wxStaticText(itemDialog1, wxID_STATIC, _("Name"),
													 wxDefaultPosition, wxDefaultSize, 0);
	itemStaticBoxSizer3->Add(itemStaticText4, 0,
							 wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE, 5);

	m_RouteNameCtl
		= new wxTextCtrl(itemDialog1, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(710, -1), 0);
	itemStaticBoxSizer3->Add(m_RouteNameCtl, 0,
							 wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

	wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(2, 2, 0, 0);
	itemStaticBoxSizer3->Add(itemFlexGridSizer6, 1, wxALIGN_LEFT | wxALL, 5);

	wxStaticText* itemStaticText7 = new wxStaticText(itemDialog1, wxID_STATIC, _("Depart From"),
													 wxDefaultPosition, wxDefaultSize, 0);
	itemFlexGridSizer6->Add(itemStaticText7, 0,
							wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);

	wxStaticText* itemStaticText8 = new wxStaticText(itemDialog1, wxID_STATIC, _("Destination"),
													 wxDefaultPosition, wxDefaultSize, 0);
	itemFlexGridSizer6->Add(itemStaticText8, 0,
							wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);

	m_RouteStartCtl
		= new wxTextCtrl(itemDialog1, ID_TEXTCTRL2, _T(""), wxDefaultPosition, wxSize(300, -1), 0);
	itemFlexGridSizer6->Add(m_RouteStartCtl, 0,
							wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
							5);

	m_RouteDestCtl
		= new wxTextCtrl(itemDialog1, ID_TEXTCTRL1, _T(""), wxDefaultPosition, wxSize(300, -1), 0);
	itemFlexGridSizer6->Add(m_RouteDestCtl, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL
											   | wxLEFT | wxRIGHT | wxBOTTOM,
							5);

	wxFlexGridSizer* itemFlexGridSizer6a = new wxFlexGridSizer(2, 4, 0, 0);
	itemStaticBoxSizer3->Add(itemFlexGridSizer6a, 1, wxALIGN_LEFT | wxALL, 5);

	wxStaticText* itemStaticText11 = new wxStaticText(itemDialog1, wxID_STATIC, _("Total Distance"),
													  wxDefaultPosition, wxDefaultSize, 0);
	itemFlexGridSizer6a->Add(itemStaticText11, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT
												  | wxRIGHT | wxTOP | wxADJUST_MINSIZE,
							 5);

	m_PlanSpeedLabel = new wxStaticText(itemDialog1, wxID_STATIC, _("Plan Speed"),
										wxDefaultPosition, wxDefaultSize, 0);
	itemFlexGridSizer6a->Add(m_PlanSpeedLabel, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT
												  | wxRIGHT | wxTOP | wxADJUST_MINSIZE,
							 5);

	wxStaticText* itemStaticText12a = new wxStaticText(itemDialog1, wxID_STATIC, _("Time Enroute"),
													   wxDefaultPosition, wxDefaultSize, 0);
	itemFlexGridSizer6a->Add(itemStaticText12a, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT
												   | wxRIGHT | wxTOP | wxADJUST_MINSIZE,
							 5);

	m_StartTimeLabel = new wxStaticText(itemDialog1, wxID_STATIC, _("Departure Time (m/d/y h:m)"),
										wxDefaultPosition, wxDefaultSize, 0);
	itemFlexGridSizer6a->Add(m_StartTimeLabel, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT
												  | wxRIGHT | wxTOP | wxADJUST_MINSIZE,
							 5);

	m_TotalDistCtl = new wxTextCtrl(itemDialog1, ID_TEXTCTRL3, _T(""), wxDefaultPosition,
									wxDefaultSize, wxTE_READONLY);
	itemFlexGridSizer6a->Add(
		m_TotalDistCtl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	m_PlanSpeedCtl = new wxTextCtrl(itemDialog1, ID_PLANSPEEDCTL, _T(""), wxDefaultPosition,
									wxSize(100, -1), wxTE_PROCESS_ENTER);
	itemFlexGridSizer6a->Add(
		m_PlanSpeedCtl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	m_TimeEnrouteCtl = new wxTextCtrl(itemDialog1, ID_TEXTCTRL4, _T(""), wxDefaultPosition,
									  wxSize(200, -1), wxTE_READONLY);
	itemFlexGridSizer6a->Add(m_TimeEnrouteCtl, 0,
							 wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
							 5);

	m_StartTimeCtl = new wxTextCtrl(itemDialog1, ID_STARTTIMECTL, _T(""), wxDefaultPosition,
									wxSize(150, -1), wxTE_PROCESS_ENTER);
	itemFlexGridSizer6a->Add(
		m_StartTimeCtl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	wxString pDispTimeZone[] = { _("UTC"), _("Local @ PC"), _("LMT @ Location") };
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);
	pDispTz = new wxRadioBox(itemDialog1, ID_TIMEZONESEL, _("Times shown as"), wxDefaultPosition,
							 wxDefaultSize, 3, pDispTimeZone, 3, wxRA_SPECIFY_COLS);
	bSizer2->Add(pDispTz, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
				 5);

	m_staticText1
		= new wxStaticText(this, wxID_ANY, _("Color:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText1->Wrap(-1);
	bSizer2->Add(m_staticText1, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	const wxString m_chColorChoices[]
		= { _("Default color"), _("Black"),		_("Dark Red"),	 _("Dark Green"),
			_("Dark Yellow"),   _("Dark Blue"), _("Dark Magenta"), _("Dark Cyan"),
			_("Light Gray"),	_("Dark Gray"), _("Red"),		   _("Green"),
			_("Yellow"),		_("Blue"),		_("Magenta"),	  _("Cyan"),
			_("White") };
	int m_chColorNChoices = sizeof(m_chColorChoices) / sizeof(wxString);
	m_chColor = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chColorNChoices,
							 m_chColorChoices, 0);
	m_chColor->SetSelection(0);
	bSizer2->Add(m_chColor, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	m_staticText2
		= new wxStaticText(this, wxID_ANY, _("Style:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText2->Wrap(-1);
	bSizer2->Add(m_staticText2, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxString m_chStyleChoices[]
		= { _("Default"), _("Solid"), _("Dot"), _("Long dash"), _("Short dash"), _("Dot dash") };
	int m_chStyleNChoices = sizeof(m_chStyleChoices) / sizeof(wxString);
	m_chStyle = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chStyleNChoices,
							 m_chStyleChoices, 0);
	m_chStyle->SetSelection(0);
	bSizer2->Add(m_chStyle, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	m_staticText2
		= new wxStaticText(this, wxID_ANY, _("Width:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText2->Wrap(-1);
	bSizer2->Add(m_staticText2, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxString m_chWidthChoices[] = { _("Default"),  _("1 pixel"),  _("2 pixels"), _("3 pixels"),
									_("4 pixels"), _("5 pixels"), _("6 pixels"), _("7 pixels"),
									_("8 pixels"), _("9 pixels"), _("10 pixels") };
	int m_chWidthNChoices = sizeof(m_chWidthChoices) / sizeof(wxString);
	m_chWidth = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chWidthNChoices,
							 m_chWidthChoices, 0);
	m_chWidth->SetSelection(0);
	bSizer2->Add(m_chWidth, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	itemStaticBoxSizer3->Add(bSizer2, 1, wxEXPAND, 0);

	wxStaticBox* itemStaticBoxSizer14Static
		= new wxStaticBox(itemDialog1, wxID_ANY, _("Waypoints"));
	m_pListSizer = new wxStaticBoxSizer(itemStaticBoxSizer14Static, wxVERTICAL);
	itemBoxSizer2->Add(m_pListSizer, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
	itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

	m_PrintButton = new wxButton(itemDialog1, ID_ROUTEPROP_PRINT, _("Print Route"),
								 wxDefaultPosition, wxDefaultSize, 0);
	itemBoxSizer16->Add(m_PrintButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
	m_PrintButton->Enable(true);

	m_ExtendButton = new wxButton(itemDialog1, ID_ROUTEPROP_EXTEND, _("Extend Route"),
								  wxDefaultPosition, wxDefaultSize, 0);
	itemBoxSizer16->Add(m_ExtendButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
	m_ExtendButton->Enable(false);

	m_SplitButton = new wxButton(itemDialog1, ID_ROUTEPROP_SPLIT, _("Split Route"),
								 wxDefaultPosition, wxDefaultSize, 0);
	itemBoxSizer16->Add(m_SplitButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
	m_SplitButton->Enable(false);

	m_CancelButton = new wxButton(itemDialog1, ID_ROUTEPROP_CANCEL, _("Cancel"), wxDefaultPosition,
								  wxDefaultSize, 0);
	itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

	m_OKButton
		= new wxButton(itemDialog1, ID_ROUTEPROP_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0);
	itemBoxSizer16->Add(m_OKButton, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);
	m_OKButton->SetDefault();

	// To correct a bug in MSW commctl32, we need to catch column width drag events, and do a
	// Refresh()
	// Otherwise, the column heading disappear.....
	// Does no harm for GTK builds, so no need for conditional
	Connect(wxEVT_COMMAND_LIST_COL_END_DRAG,
			(wxObjectEventFunction)(wxEventFunction) & RouteProp::OnEvtColDragEnd);

	// Create the two list controls
	m_wpList = new wxListCtrl(itemDialog1, ID_LISTCTRL, wxDefaultPosition, wxSize(800, 200),
							  wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_EDIT_LABELS);

	m_wpList->InsertColumn(0, _("Leg"), wxLIST_FORMAT_LEFT, 45);
	m_wpList->InsertColumn(1, _("To Waypoint"), wxLIST_FORMAT_LEFT, 120);
	m_wpList->InsertColumn(2, _("Distance"), wxLIST_FORMAT_RIGHT, 70);

	if (g_bShowMag)
		m_wpList->InsertColumn(3, _("Bearing (M)"), wxLIST_FORMAT_LEFT, 80);
	else
		m_wpList->InsertColumn(3, _("Bearing"), wxLIST_FORMAT_LEFT, 80);

	m_wpList->InsertColumn(4, _("Latitude"), wxLIST_FORMAT_LEFT, 85);
	m_wpList->InsertColumn(5, _("Longitude"), wxLIST_FORMAT_LEFT, 90);
	m_wpList->InsertColumn(6, _("ETE/ETD"), wxLIST_FORMAT_LEFT, 135);
	m_wpList->InsertColumn(7, _("Speed"), wxLIST_FORMAT_CENTER, 72);
	m_wpList->InsertColumn(8, _("Next tide event"), wxLIST_FORMAT_LEFT, 90);
	m_wpList->InsertColumn(9, _("Description"), wxLIST_FORMAT_LEFT,
						   90); // additional columt with WP description
	if (g_bShowMag)
		m_wpList->InsertColumn(
			10, _("Course (M)"), wxLIST_FORMAT_LEFT,
			80); // additional columt with WP new course. Is it same like "bearing" of the next WP.
	else
		m_wpList->InsertColumn(10, _("Course"), wxLIST_FORMAT_LEFT, 80); // additional columt with
																		 // WP new course. Is it
																		 // same like "bearing" of
																		 // the next WP.
	m_wpList->Hide();

	Connect(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
			wxListEventHandler(RouteProp::OnRoutePropRightClick), NULL, this);
	Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(RouteProp::OnRoutePropMenuSelected),
			NULL, this);

	//  Fetch any config file values
	m_planspeed = g_PlanSpeed;

	pDispTz->SetSelection(g_StartTimeTZ);

	SetColorScheme((ColorScheme)0);
}

void RouteProp::OnRoutepropListClick(wxListEvent& event)
{
	long itemno = 0;
	m_nSelected = 0;

	// We use different methods to determine the selected point,
	// depending on whether this is a Route or a Track.
	int selected_no;
	const wxListItem& item = event.GetItem();
	item.GetText().ToLong(&itemno);
	selected_no = itemno;

	m_pRoute->ClearHighlights();

	RoutePointList::iterator i = m_pRoute->pRoutePointList->begin();
	while ((i != m_pRoute->pRoutePointList->end()) && itemno--) { // FIXME: this is basically an indexed access
		++i;
	}
	if (i != m_pRoute->pRoutePointList->end()) {
		RoutePoint* prp = *i;
		if (prp) {
			prp->m_bPtIsSelected = true; // highlight the routepoint

			if (!(m_pRoute->m_bIsInLayer) && !(m_pRoute == g_pActiveTrack)
				&& !(m_pRoute->m_bRtIsActive)) {
				m_nSelected = selected_no + 1;
				m_SplitButton->Enable(true);
			}

			gFrame->JumpToPosition(prp->get_position(), cc1->GetVPScale());
		}
	}
}

void RouteProp::OnRoutePropMenuSelected(wxCommandEvent& event)
{
	switch (event.GetId()) {
		case ID_RCLK_MENU_COPY_TEXT:
			OnRoutepropCopyTxtClick(event);
			break;

		case ID_RCLK_MENU_DELETE: {
			int dlg_return = OCPNMessageBox(
				this, _("Are you sure you want to remove this waypoint?"),
				_("OpenCPN Remove Waypoint"), (long)wxYES_NO | wxCANCEL | wxYES_DEFAULT);
			if (dlg_return == wxID_YES) {
				long item = -1;
				item = m_wpList->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
				if (item == -1)
					break;
				RoutePoint* wp = reinterpret_cast<RoutePoint*>(m_wpList->GetItemData(item));
				cc1->RemovePointFromRoute(wp, m_pRoute);
				SetRouteAndUpdate(m_pRoute);
			}
			break;
		}

		case ID_RCLK_MENU_EDIT_WP: {
			long item = m_wpList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if (item == -1)
				break;
			RoutePoint* wp = reinterpret_cast<RoutePoint*>(m_wpList->GetItemData(item));
			if (!wp)
				break;
			RouteManagerDialog::WptShowPropertiesDialog(wp, this);
			UpdateProperties();
			break;
		}
	}
}

void RouteProp::SetColorScheme(ColorScheme)
{
	DimeControl(this);
}

bool RouteProp::ShowToolTips()
{
	return true;
}

void RouteProp::SetDialogTitle(const wxString& title)
{
	SetTitle(title);
}

void RouteProp::SetRouteAndUpdate(Route* pR)
{
	// Fetch any config file values

	m_tz_selection = 1;

	if (pR == m_pRoute) {
		gStart_LMT_Offset = 0;
		if (!pR->m_PlannedDeparture.IsValid())
			m_tz_selection = 0;
	} else {
		g_StartTime = wxInvalidDateTime;
		g_StartTimeTZ = 1;
		if (pR->m_PlannedDeparture.IsValid())
			m_starttime = pR->m_PlannedDeparture;
		else
			m_starttime = g_StartTime;
		if (pR->m_TimeDisplayFormat == RTE_TIME_DISP_UTC)
			m_tz_selection = 0;
		else if (pR->m_TimeDisplayFormat == RTE_TIME_DISP_LOCAL)
			m_tz_selection = 2;
		else
			m_tz_selection = g_StartTimeTZ;
		gStart_LMT_Offset = 0;
		m_pEnroutePoint = NULL;
		m_bStartNow = false;
		m_planspeed = pR->m_PlannedSpeed;
	}

	m_pRoute = pR;

	pDispTz->SetSelection(m_tz_selection);

	if (m_pRoute) {
		// Calculate  LMT offset from the first point in the route
		if (m_pEnroutePoint && m_bStartNow)
			gStart_LMT_Offset = long((m_pEnroutePoint->longitude()) * 3600.0 / 15.0);
		else
			gStart_LMT_Offset = long((m_pRoute->pRoutePointList->front()->longitude()) * 3600.0 / 15.0);
	}

	// Reorganize dialog for route or track display
	if (m_pRoute) {
		if (m_pRoute->m_bIsTrack) {
			m_PlanSpeedLabel->SetLabel(_("Avg. speed"));
			m_PlanSpeedCtl->SetEditable(false);
			m_ExtendButton->SetLabel(_("Extend Track"));
			m_SplitButton->SetLabel(_("Split Track"));
		} else {
			m_PlanSpeedLabel->SetLabel(_("Plan speed"));
			m_PlanSpeedCtl->SetEditable(true);
			m_ExtendButton->SetLabel(_("Extend Route"));
			m_SplitButton->SetLabel(_("Split Route"));
		}

		// Fill in some top pane properties from the Route member elements
		m_RouteNameCtl->SetValue(m_pRoute->m_RouteNameString);
		m_RouteStartCtl->SetValue(m_pRoute->m_RouteStartString);
		m_RouteDestCtl->SetValue(m_pRoute->m_RouteEndString);
		m_RouteNameCtl->SetFocus();
	} else {
		m_RouteNameCtl->Clear();
		m_RouteStartCtl->Clear();
		m_RouteDestCtl->Clear();
		m_PlanSpeedCtl->Clear();
		m_StartTimeCtl->Clear();
	}

	m_wpList->DeleteAllItems();

	// Select the proper list control, and add it to List sizer
	m_pListSizer->Clear();

	if (m_pRoute) {
		m_wpList->Show();
		m_pListSizer->Add(m_wpList, 2, wxEXPAND | wxALL, 5);
	}
	GetSizer()->Fit(this);
	GetSizer()->Layout();

	InitializeList();
	UpdateProperties();
}

void RouteProp::InitializeList()
{
	if (!m_pRoute)
		return;

	if (!m_pRoute->m_bIsTrack) {
		m_pRoute->UpdateSegmentDistances(m_planspeed); // to fix ETD properties

		// Iterate on Route Points, inserting blank fields starting with index 0
		int in = 0;
		for (RoutePointList::iterator route_point = m_pRoute->pRoutePointList->begin();
			 route_point != m_pRoute->pRoutePointList->end(); ++route_point) {
			m_wpList->InsertItem(in, _T(""), 0);
			m_wpList->SetItemPtrData(in, (wxUIntPtr)(*route_point));
			in++;
			if ((*route_point)->m_seg_etd.IsValid()) {
				m_wpList->InsertItem(in, _T(""), 0);
				in++;
			}
		}

		// Update the plan speed and route start time controls
		m_PlanSpeedCtl->SetValue(wxString::Format(_T("%5.2f"), m_planspeed));

		if (m_starttime.IsValid()) {
			m_StartTimeCtl->SetValue(ts2s(m_starttime, m_tz_selection, (int)gStart_LMT_Offset, INPUT_FORMAT));
		} else
			m_StartTimeCtl->Clear();
	}
}

void RouteProp::update_track_properties()
{
	m_pRoute->UpdateSegmentDistances(); // get segment and total distance but ignore leg speed calcs

	// Calculate AVG speed if we are showing a track and total time
	RoutePoint* last_point = m_pRoute->GetLastPoint();
	RoutePoint* first_point = m_pRoute->GetPoint(1);
	double total_seconds = 0.0;

	if (last_point->GetCreateTime().IsValid() && first_point->GetCreateTime().IsValid()) {
		total_seconds = last_point->GetCreateTime()
							.Subtract(first_point->GetCreateTime())
							.GetSeconds()
							.ToDouble();
		if (total_seconds != 0.0) {
			m_avgspeed = m_pRoute->m_route_length / total_seconds * 3600.0;
		} else {
			m_avgspeed = 0;
		}
		m_PlanSpeedCtl->SetValue(wxString::Format(_T("%5.2f"), toUsrSpeed(m_avgspeed)));
	} else {
		m_PlanSpeedCtl->SetValue(_T("--"));
	}

	// Total length
	m_TotalDistCtl->SetValue(wxString::Format(wxT("%5.2f ") + getUsrDistanceUnit(), toUsrDistance(m_pRoute->m_route_length)));

	// Time
	wxString time_form;
	wxTimeSpan time(0, 0, (int)total_seconds, 0);
	if (total_seconds > 3600.0 * 24.0)
		time_form = time.Format(_(" %D Days  %H Hours  %M Minutes"));
	else if (total_seconds > 0.0)
		time_form = time.Format(_(" %H Hours  %M Minutes"));
	else
		time_form = _T("--");
	m_TimeEnrouteCtl->SetValue(time_form);
}

void RouteProp::update_route_properties()
{
	// Update the "tides event" column header
	wxListItem column_info;
	if (m_wpList->GetColumn(8, column_info)) {
		wxString c = _("Next tide event");
		if (gpIDX && m_starttime.IsValid()) {
			c = _T("@~~");
			c.Append(wxString(gpIDX->IDX_station_name, wxConvUTF8));
			int i = c.Find(',');
			if (i != wxNOT_FOUND)
				c.Remove(i);
		}
		column_info.SetText(c);
		m_wpList->SetColumn(8, column_info);
	}

	// Update the "ETE/Timestamp" column header

	if (m_wpList->GetColumn(6, column_info)) {
		if (m_starttime.IsValid())
			column_info.SetText(_("ETA"));
		else
			column_info.SetText(_("ETE"));
		m_wpList->SetColumn(6, column_info);
	}

	m_pRoute->UpdateSegmentDistances(m_planspeed); // get segment and total distance
	double leg_speed = m_planspeed;
	wxTimeSpan stopover_time(0); // time spent waiting for ETD
	wxTimeSpan joining_time(0); // time spent before reaching first waypoint

	double total_seconds = 0.;

	if (m_pRoute) {
		total_seconds = m_pRoute->m_route_time;
		if (m_bStartNow) {
			if (m_pEnroutePoint)
				gStart_LMT_Offset = static_cast<long>(m_pEnroutePoint->longitude() * 3600.0 / 15.0);
			else
				gStart_LMT_Offset
					= static_cast<long>(m_pRoute->pRoutePointList->front()->longitude() * 3600.0 / 15.0);
		}
	}

	const int tz_selection = pDispTz->GetSelection();

	if (m_starttime.IsValid()) {
		wxString s;
		if (m_bStartNow) {
			wxDateTime d = wxDateTime::Now();
			if (tz_selection == 1) {
				m_starttime = d.ToUTC();
				s = _T(">");
			}
		}
		s += ts2s(m_starttime, tz_selection, static_cast<int>(gStart_LMT_Offset), INPUT_FORMAT);
		m_StartTimeCtl->SetValue(s);
	} else
		m_StartTimeCtl->Clear();

	if (IsThisRouteExtendable())
		m_ExtendButton->Enable(true);

	// Total length
	if (!m_pEnroutePoint) {
		m_TotalDistCtl->SetValue(wxString::Format(wxT("%5.2f ") + getUsrDistanceUnit(), toUsrDistance(m_pRoute->m_route_length)));
	} else {
		m_TotalDistCtl->Clear();
	}

	wxString time_form;
	wxString tide_form;

	// Time

	wxTimeSpan time(0, 0, static_cast<int>(total_seconds), 0);
	if (total_seconds > 3600.0 * 24.0)
		time_form = time.Format(_(" %D Days  %H Hours  %M Minutes"));
	else
		time_form = time.Format(_(" %H Hours  %M Minutes"));

	if (!m_pEnroutePoint)
		m_TimeEnrouteCtl->SetValue(time_form);
	else
		m_TimeEnrouteCtl->Clear();

	// Iterate on Route Points

	const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
	int i = 0;
	double slat = nav.pos.lat();
	double slon = nav.pos.lon();
	double tdis = 0.0;
	double tsec = 0.0; // total time in seconds

	int stopover_count = 0;
	bool arrival = true; // marks which pass over the wpt we do - 1. arrival 2. departure
	bool enroute = true; // for active route, skip all points up to the active point

	if (m_pRoute->m_bRtIsActive) {
		if (m_pEnroutePoint && m_bStartNow)
			enroute = (m_pRoute->GetPoint(1)->m_GUID == m_pEnroutePoint->m_GUID);
	}

	wxString nullify = _T("----");

	RoutePointList::iterator node = m_pRoute->pRoutePointList->begin();
	while (node != m_pRoute->pRoutePointList->end()) {
		RoutePoint* prp = *node;
		long item_line_index = i + stopover_count;

		// Leg
		wxString t = wxString::Format(_T("%d"), i);
		if (i == 0)
			t = _T("---");
		if (arrival)
			m_wpList->SetItem(item_line_index, 0, t);

		// Mark Name
		if (arrival)
			m_wpList->SetItem(item_line_index, 1, prp->GetName());
		// Store Dewcription
		if (arrival)
			m_wpList->SetItem(item_line_index, 9, prp->GetDescription());

		// Distance
		// Note that Distance/Bearing for Leg 000 is as from current position

		double brg;
		double leg_dist;
		bool starting_point = false;

		starting_point = (i == 0) && enroute;
		if (m_pEnroutePoint && !starting_point)
			starting_point = (prp->m_GUID == m_pEnroutePoint->m_GUID);

		if (starting_point) {
			const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();

			slat = nav.pos.lat();
			slon = nav.pos.lon();
			if (nav.sog > 0.0)
				leg_speed = nav.sog; // should be VMG
			else
				leg_speed = m_planspeed;
			if (m_bStartNow) {
				geo::DistanceBearingMercator(prp->latitude(), prp->longitude(), slat, slon, &brg,
											 &leg_dist);
				if (i == 0)
					joining_time
						= wxTimeSpan::Seconds((long)wxRound((leg_dist * 3600.0) / leg_speed));
			}
			enroute = true;
		} else {
			if (prp->m_seg_vmg > 0.0)
				leg_speed = prp->m_seg_vmg;
			else
				leg_speed = m_planspeed;
		}

		geo::DistanceBearingMercator(prp->latitude(), prp->longitude(), slat, slon, &brg, &leg_dist);

		// calculation of course at current WayPoint.
		double course = 10;
		double tmp_leg_dist = 23;
		RoutePointList::iterator next_node = node + 1;
		RoutePoint* _next_prp = (next_node != m_pRoute->pRoutePointList->end()) ? *next_node : NULL;
		if (_next_prp) {
			geo::DistanceBearingMercator(_next_prp->latitude(), _next_prp->longitude(),
										 prp->latitude(), prp->longitude(), &course, &tmp_leg_dist);
		} else {
			course = 0.0;
			tmp_leg_dist = 0.0;
		}

		prp->SetCourse(course); // save the course to the next waypoint for printing.
		// end of calculation

		t.Printf(_T("%6.2f ") + getUsrDistanceUnit(), toUsrDistance(leg_dist));
		if (arrival)
			m_wpList->SetItem(item_line_index, 2, t);
		if (!enroute)
			m_wpList->SetItem(item_line_index, 2, nullify);
		prp->SetDistance(leg_dist); // save the course to the next waypoint for printing.

		//  Bearing
		if (g_bShowMag)
			t.Printf(_T("%03.0f Deg. M"), navigation::GetTrueOrMag(brg));
		else
			t.Printf(_T("%03.0f Deg. T"), navigation::GetTrueOrMag(brg));
		if (arrival)
			m_wpList->SetItem(item_line_index, 3, t);
		if (!enroute)
			m_wpList->SetItem(item_line_index, 3, nullify);

		// Course (bearing of next)
		if (_next_prp) {
			if (g_bShowMag)
				t.Printf(_T("%03.0f Deg. M"), navigation::GetTrueOrMag(course));
			else
				t.Printf(_T("%03.0f Deg. T"), navigation::GetTrueOrMag(course));
			if (arrival)
				m_wpList->SetItem(item_line_index, 10, t);
		} else {
			m_wpList->SetItem(item_line_index, 10, nullify);
		}

		// Lat/Lon
		wxString tlat = toSDMM(1, prp->latitude(), prp->m_bIsInTrack); // low precision for routes
		if (arrival)
			m_wpList->SetItem(item_line_index, 4, tlat);

		wxString tlon = toSDMM(2, prp->longitude(), prp->m_bIsInTrack);
		if (arrival)
			m_wpList->SetItem(item_line_index, 5, tlon);

		tide_form = _T("");

		long LMT_Offset = static_cast<long>(prp->longitude() * 3600.0 / 15.0); // offset in seconds from UTC for given location (-1 hr / 15 deg W)

		// Time to each waypoint or creation date for tracks
		if (i == 0 && enroute) {
			time_form.Printf(_("Start"));
			if (m_starttime.IsValid()) {
				wxDateTime act_starttime = m_starttime + joining_time;
				time_form.Append(_T(": "));

				if (!arrival) {
					wxDateTime etd = prp->m_seg_etd;
					if (etd.IsValid() && etd.IsLaterThan(m_starttime)) {
						stopover_time += etd.Subtract(m_starttime);
						act_starttime = prp->m_seg_etd;
					}
				}

				wxString s = ts2s(act_starttime, tz_selection, (int)LMT_Offset, DISPLAY_FORMAT);
				time_form.Append(s);
				time_form.Append(_T("   ("));
				time_form.Append(GetDaylightString(getDaylightStatus(prp->get_position(), act_starttime)));
				time_form.Append(_T(")"));

				if (ptcmgr) {
					int jx = 0;
					if (prp->GetName().Find(_T("@~~")) != wxNOT_FOUND) {
						tide_form = prp->GetName().Mid(prp->GetName().Find(_T("@~~")) + 3);
						jx = ptcmgr->GetStationIDXbyName(tide_form, prp->latitude(), prp->longitude());
					}
					if (gpIDX || jx) {
						time_t tm = act_starttime.GetTicks();
						tide_form = MakeTideInfo(jx, tm, tz_selection, LMT_Offset);
					}
				}
			}
			tdis = 0;
			tsec = 0.0;
		} else {
			if (arrival && enroute)
				tdis += leg_dist;
			if (leg_speed) {
				if (arrival && enroute)
					tsec += 3600 * leg_dist / leg_speed; // time in seconds to arrive here
				wxTimeSpan time(0, 0, (int)tsec, 0);

				if (m_starttime.IsValid()) {

					wxDateTime ueta = m_starttime;
					ueta.Add(time + stopover_time + joining_time);

					if (!arrival) {
						wxDateTime etd = prp->m_seg_etd;
						if (etd.IsValid() && etd.IsLaterThan(ueta)) {
							stopover_time += etd.Subtract(ueta);
							ueta = prp->m_seg_etd;
						}
					}

					time_form = ts2s(ueta, tz_selection, LMT_Offset, DISPLAY_FORMAT);
					time_form.Append(_T("   ("));
					time_form.Append(GetDaylightString(getDaylightStatus(prp->get_position(), ueta)));
					time_form.Append(_T(")"));

					if (ptcmgr) {
						int jx = 0;
						if (prp->GetName().Find(_T("@~~")) != wxNOT_FOUND) {
							tide_form = prp->GetName().Mid(prp->GetName().Find(_T("@~~")) + 3);
							jx = ptcmgr->GetStationIDXbyName(tide_form, prp->latitude(), prp->longitude());
						}
						if (gpIDX || jx) {
							time_t tm = ueta.GetTicks();
							tide_form = MakeTideInfo(jx, tm, tz_selection, LMT_Offset);
						}
					}
				} else {
					if (tsec > 3600.0 * 24.0)
						time_form = time.Format(_T(" %D D  %H H  %M M"));
					else
						time_form = time.Format(_T(" %H H  %M M"));
				}
			}
		}

		if (enroute && (arrival || m_starttime.IsValid()))
			m_wpList->SetItem(item_line_index, 6, time_form);
		else
			m_wpList->SetItem(item_line_index, 6, _T("----"));

		// Leg speed
		if (arrival) {
			m_wpList->SetItem(item_line_index, 7, wxString::Format(_T("%5.2f"), toUsrSpeed(leg_speed)));
		}

		if (enroute) {
			m_wpList->SetItem(item_line_index, 8, tide_form);
		} else {
			m_wpList->SetItem(item_line_index, 7, nullify);
			m_wpList->SetItem(item_line_index, 8, nullify);
		}

		// Save for iterating distance/bearing calculation
		slat = prp->latitude();
		slon = prp->longitude();

		// if stopover (ETD) found, loop for next output line for the same point
		// with departure time & tide information

		if (arrival && (prp->m_seg_etd.IsValid())) {
			stopover_count++;
			arrival = false;
		} else {
			arrival = true;
			i++;
			++node;
		}
	}
}

bool RouteProp::UpdateProperties()
{
	if (!m_pRoute)
		return false;

	::wxBeginBusyCursor();

	m_TotalDistCtl->SetValue(_T(""));
	m_TimeEnrouteCtl->SetValue(_T(""));

	m_SplitButton->Enable(false);
	m_ExtendButton->Enable(false);

	if (m_pRoute->m_bIsTrack) {
		update_track_properties();
	} else {
		update_route_properties();
	}

	if (m_pRoute->m_Colour == wxEmptyString) {
		m_chColor->Select(0);
	} else {
		for (unsigned int i = 0; i < sizeof(::GpxxColorNames) / sizeof(::GpxxColorNames[0]); i++) {
			if (m_pRoute->m_Colour == ::GpxxColorNames[i]) {
				m_chColor->Select(i + 1);
				break;
			}
		}
	}

	for (unsigned int i = 0; i < sizeof(::StyleValues) / sizeof(::StyleValues[0]); i++) {
		if (m_pRoute->m_style == ::StyleValues[i]) {
			m_chStyle->Select(i);
			break;
		}
	}

	for (unsigned int i = 0; i < sizeof(::WidthValues) / sizeof(::WidthValues[0]); i++) {
		if (m_pRoute->m_width == ::WidthValues[i]) {
			m_chWidth->Select(i);
			break;
		}
	}

	::wxEndBusyCursor();

	return true;
}

wxString RouteProp::MakeTideInfo(int jx, time_t tm, int tz_selection, long LMT_Offset)
{
	int ev = 0;
	wxString tide_form;

	if (gpIDX) {
		ev = ptcmgr->GetNextBigEvent(
			&tm, ptcmgr->GetStationIDXbyName(wxString(gpIDX->IDX_station_name, wxConvUTF8),
											 gpIDX->IDX_lat, gpIDX->IDX_lon));
	} else
		ev = ptcmgr->GetNextBigEvent(&tm, jx);

	wxDateTime dtm;
	dtm.Set(tm).MakeUTC(); // apparently Set works as from LT
	if (ev == 1)
		tide_form.Printf(_T("LW: "));
	if (ev == 2)
		tide_form.Printf(_T("HW: "));
	tide_form.Append(ts2s(dtm, tz_selection, LMT_Offset, DISPLAY_FORMAT));
	if (!gpIDX) {
		wxString locn(ptcmgr->GetIDX_entry(jx)->IDX_station_name, wxConvUTF8);
		tide_form.Append(_T(" @~~"));
		tide_form.Append(locn);
	}
	return tide_form;
}

bool RouteProp::SaveChanges(void)
{
	//  Save the current planning speed
	g_PlanSpeed = m_planspeed;
	g_StartTime = m_starttime; // both always UTC
	g_StartTimeTZ = pDispTz->GetSelection();
	m_StartTimeCtl->Clear();

	if (m_pRoute && !m_pRoute->m_bIsInLayer) {
		//  Get User input Text Fields
		m_pRoute->m_RouteNameString = m_RouteNameCtl->GetValue();
		m_pRoute->m_RouteStartString = m_RouteStartCtl->GetValue();
		m_pRoute->m_RouteEndString = m_RouteDestCtl->GetValue();
		if (m_chColor->GetSelection() == 0)
			m_pRoute->m_Colour = wxEmptyString;
		else
			m_pRoute->m_Colour = ::GpxxColorNames[m_chColor->GetSelection() - 1];
		m_pRoute->m_style = ::StyleValues[m_chStyle->GetSelection()];
		m_pRoute->m_width = ::WidthValues[m_chWidth->GetSelection()];
		m_pRoute->m_PlannedDeparture = g_StartTime;
		m_pRoute->m_PlannedSpeed = m_planspeed;
		switch (g_StartTimeTZ) {
			case 1:
				m_pRoute->m_TimeDisplayFormat = RTE_TIME_DISP_PC;
				break;
			case 2:
				m_pRoute->m_TimeDisplayFormat = RTE_TIME_DISP_LOCAL;
				break;
			default:
				m_pRoute->m_TimeDisplayFormat = RTE_TIME_DISP_UTC;
		}

		pConfig->UpdateRoute(m_pRoute);
		pConfig->UpdateSettings();
	}

	if (m_pRoute->IsActive() || ((Track*)m_pRoute)->IsRunning()) {
		wxJSONValue v;
		v[_T("Name")] = m_pRoute->m_RouteNameString;
		v[_T("GUID")] = m_pRoute->m_GUID;
		wxString msg_id(_T("OCPN_TRK_ACTIVATED"));
		g_pi_manager->SendJSONMessageToAllPlugins(msg_id, v);
	}

	return true;
}

void RouteProp::OnPlanSpeedCtlUpdated(wxCommandEvent& event)
{
	// Fetch the value, and see if it is a "reasonable" speed
	wxString spd = m_PlanSpeedCtl->GetValue();
	double s;
	spd.ToDouble(&s);
	if ((0.1 < s) && (s < 1000.0) && !m_pRoute->m_bIsTrack) {
		m_planspeed = fromUsrSpeed(s);
		UpdateProperties();
	}

	event.Skip();
}

void RouteProp::OnStartTimeCtlUpdated(wxCommandEvent&)
{
	//  Fetch the value, and see if it is a "reasonable" time
	wxString stime = m_StartTimeCtl->GetValue();
	int tz_selection = pDispTz->GetSelection();

	wxDateTime d;
	if (stime.StartsWith(_T(">"))) {
		if (m_pRoute->m_bRtIsActive) {
			m_pEnroutePoint = g_pRouteMan->GetpActivePoint();
		}
		m_bStartNow = true;
		d = wxDateTime::Now();
		if (tz_selection == 1)
			m_starttime = d.ToUTC();
		else
			m_starttime = wxInvalidDateTime; // can't get it to work otherwise
	} else {
		m_pEnroutePoint = NULL;
		m_bStartNow = false;
		if (!d.ParseDateTime(stime)) // only specific times accepted
			d = wxInvalidDateTime;

		m_starttime = d;

		if (m_starttime.IsValid()) {
			if (tz_selection == 1)
				m_starttime = d.ToUTC();
			if (tz_selection == 2) {
				wxTimeSpan glmt(0, 0, (int)gStart_LMT_Offset, 0);
				m_starttime -= glmt;
			}
		}
	}

	UpdateProperties();
}

void RouteProp::OnTimeZoneSelected(wxCommandEvent& event)
{
	UpdateProperties();
	event.Skip();
}

void RouteProp::OnRoutepropCancelClick(wxCommandEvent& event)
{
	// Look in the route list to be sure the raoute is still available
	// (May have been deleted by RouteMangerDialog...)

	if (g_pRouteMan->RouteExists(m_pRoute))
		m_pRoute->ClearHighlights();

	Hide();
	cc1->Refresh(false);

	event.Skip();
}

void RouteProp::OnRoutepropOkClick(wxCommandEvent& event)
{
	// Look in the route list to be sure the route is still available
	// (May have been deleted by RouteManagerDialog...)

	if (g_pRouteMan->RouteExists(m_pRoute)) {
		SaveChanges(); // write changes to globals and update config
		m_pRoute->ClearHighlights();
	}

	m_pEnroutePoint = NULL;
	m_bStartNow = false;

	if (pRouteManagerDialog && pRouteManagerDialog->IsShown()) {
		if (!m_pRoute->m_bIsTrack)
			pRouteManagerDialog->UpdateRouteListCtrl();
		else
			pRouteManagerDialog->UpdateTrkListCtrl();
	}

	Hide();
	cc1->Refresh(false);

	event.Skip();
}

void RouteProp::OnEvtColDragEnd(wxListEvent&)
{
	m_wpList->Refresh();
}

