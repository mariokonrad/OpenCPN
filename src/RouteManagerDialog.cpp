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

#include "RouteManagerDialog.h"
#include "dychart.h"
#include <MessageBox.h>
#include <Select.h>
#include <WayPointman.h>
#include <Track.h>
#include <RouteProp.h>
#include <MarkInfo.h>
#include <Routeman.h>
#include <Layer.h>
#include <SendToGpsDlg.h>
#include <TrackPropDlg.h>
#include <Undo.h>
#include <Config.h>
#include <ChartCanvas.h>
#include <MainFrame.h>
#include <DimeControl.h>
#include <Layer.h>
#include <Units.h>

#include <geo/GeoRef.h>

#include <global/OCPN.h>
#include <global/Navigation.h>

#include <iostream>
#include <algorithm>

#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/progdlg.h>
#include <wx/clipbrd.h>
#include <wx/imaglist.h>

#define DIALOG_MARGIN 3

/* XPM */
static const char *eye[]={
	"20 20 7 1",
	". c none",
	"# c #000000",
	"a c #333333",
	"b c #666666",
	"c c #999999",
	"d c #cccccc",
	"e c #ffffff",
	"....................",
	"....................",
	"....................",
	"....................",
	".......######.......",
	".....#aabccb#a#.....",
	"....#deeeddeebcb#...",
	"..#aeeeec##aceaec#..",
	".#bedaeee####dbcec#.",
	"#aeedbdabc###bcceea#",
	".#bedad######abcec#.",
	"..#be#d######dadb#..",
	"...#abac####abba#...",
	".....##acbaca##.....",
	".......######.......",
	"....................",
	"....................",
	"....................",
	"....................",
	"...................."};

/* XPM */
static const char *eyex[]={
	"20 20 8 1",
	"# c None",
	"a c #000000",
	"b c #333333",
	"c c #666666",
	"d c #999999",
	"f c #cccccc",
	". c #ff0000",
	"e c #ffffff",
	".##################.",
	"..################..",
	"#..##############..#",
	"##..############..##",
	"###..##aaaaaa##..###",
	"####..bbcddcab..####",
	"####a..eeffee..ca###",
	"##abee..daab..beda##",
	"#acefbe..aa..fcdeda#",
	"abeefcfb....acddeeba",
	"#acefbfaa..aabcdeda#",
	"##aceafa....afbfca##",
	"###abcb..aa..ccba###",
	"#####a..dcbd..a#####",
	"#####..aaaaaa..#####",
	"####..########..####",
	"###..##########..###",
	"##..############..##",
	"#..##############..#",
	"..################.."};

enum { rmVISIBLE = 0, rmROUTENAME, rmROUTEDESC };// RMColumns;
enum { colTRKVISIBLE = 0, colTRKNAME, colTRKLENGTH };
enum { colLAYVISIBLE = 0, colLAYNAME, colLAYITEMS };
enum { colWPTICON = 0, colWPTNAME, colWPTDIST };

extern RouteList* pRouteList;
extern LayerList* pLayerList;
extern RouteProp* pRoutePropDialog;
extern TrackPropDlg* pTrackPropDialog;
extern Routeman* g_pRouteMan;
extern Config* pConfig;
extern ChartCanvas* cc1;
extern chart::ChartBase* Current_Ch;
extern Track* g_pActiveTrack;
extern WayPointman* pWayPointMan;
extern MarkInfoImpl* pMarkPropDialog;
extern MainFrame* gFrame;
extern Select* pSelect;
extern bool g_bShowLayers;
extern wxString g_default_wp_icon;

struct SortContext
{
	wxListCtrl* list;
	int direction;
	int column;
};

#if wxCHECK_VERSION(2, 9, 0)
typedef wxInPtr SortPtrType;
#else
typedef long SortPtrType;
#endif

static int wxCALLBACK SortColumnText(long item1, long item2, SortPtrType context)
{
	SortContext* sorting = reinterpret_cast<SortContext*>(context);
	wxListCtrl* lc = sorting->list;

	wxListItem it1;
	wxListItem it2;
	it1.SetId(lc->FindItem(-1, item1));
	it1.SetColumn(sorting->column);
	it1.SetMask(it1.GetMask() | wxLIST_MASK_TEXT);

	it2.SetId(lc->FindItem(-1, item2));
	it2.SetColumn(sorting->column);
	it2.SetMask(it2.GetMask() | wxLIST_MASK_TEXT);

	lc->GetItem(it1);
	lc->GetItem(it2);

	if (sorting->direction & 1)
		return it2.GetText().CmpNoCase(it1.GetText());
	else
		return it1.GetText().CmpNoCase(it2.GetText());
}

static int wxCALLBACK SortColumnDistance(long item1, long item2, SortPtrType context)
{
	SortContext* sorting = reinterpret_cast<SortContext*>(context);
	wxListCtrl* lc = sorting->list;

	wxListItem it1;
	wxListItem it2;
	it1.SetId(lc->FindItem(-1, item1));
	it1.SetColumn(sorting->column);
	it1.SetMask(it1.GetMask() | wxLIST_MASK_TEXT);

	it2.SetId(lc->FindItem(-1, item2));
	it2.SetColumn(sorting->column);
	it2.SetMask(it2.GetMask() | wxLIST_MASK_TEXT);

	lc->GetItem(it1);
	lc->GetItem(it2);

	wxString s1 = wxString::Format(_T("%11s"), it1.GetText().c_str());
	wxString s2 = wxString::Format(_T("%11s"), it2.GetText().c_str());

	double l1;
	double l2;
	s1.ToDouble(&l1);
	s2.ToDouble(&l2);

	if (sorting->direction & 1)
		return (l1 < l2);
	else
		return (l2 < l1);
}

/// Sort callback. Sort by wpt name.
static int wxCALLBACK SortWaypointsOnName(long item1, long item2, SortPtrType context)
{
	RoutePoint* pRP1 = reinterpret_cast<RoutePoint*>(item1);
	RoutePoint* pRP2 = reinterpret_cast<RoutePoint*>(item2);
	SortContext* sorting = reinterpret_cast<SortContext*>(context);

	if (pRP1 && pRP2) {
		if (sorting->direction & 1)
			return pRP2->GetName().CmpNoCase(pRP1->GetName());
		else
			return pRP1->GetName().CmpNoCase(pRP2->GetName());
	} else
		return 0;
}

// event table. Empty, because I find it much easier to see what is connected to what
// using Connect() where possible, so that it is visible in the code.
BEGIN_EVENT_TABLE(RouteManagerDialog, wxDialog)
	EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, RouteManagerDialog::OnTabSwitch) // This should work under Windows :-(
END_EVENT_TABLE()

void RouteManagerDialog::OnTabSwitch(wxNotebookEvent& event)
{
	if (!m_pNotebook)
		return;
	int current_page = m_pNotebook->GetSelection();
	if (current_page == 3) {
		// ?
	} else {
		if (btnImport)
			btnImport->Enable(true);
		if (btnExport)
			btnExport->Enable(true);
		if (btnExportViz)
			btnExportViz->Enable(true);
	}
	event.Skip(); // remove if using event table... why?
}

RouteManagerDialog::RouteManagerDialog(wxWindow* parent)
{
	long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;
#ifdef __WXOSX__
	style |= wxSTAY_ON_TOP;
#endif

	wxDialog::Create(parent, -1, _("Route Manager"), wxDefaultPosition, wxDefaultSize, style);

	m_lastWptItem = -1;
	m_lastTrkItem = -1;
	m_lastRteItem = -1;

	btnImport = NULL;
	btnExport = NULL;
	btnExportViz = NULL;

	Create();
}

void RouteManagerDialog::create_routes_panel()
{
	m_pPanelRte = new wxPanel(m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
							  wxNO_BORDER | wxTAB_TRAVERSAL);
	wxBoxSizer* sbsRoutes = new wxBoxSizer(wxHORIZONTAL);
	m_pPanelRte->SetSizer(sbsRoutes);
	m_pNotebook->AddPage(m_pPanelRte, _("Routes"));

	sort_wp_len_dir = 1;
	sort_wp_name_dir = 0;
	sort_track_len_dir = 1;
	sort_route_to_dir = 0;
	sort_track_name_dir = 0;
	sort_route_name_dir = 0;
	sort_layer_name_dir = 0;
	sort_layer_len_dir = 1;

	// Setup GUI
	m_pRouteListCtrl = new wxListCtrl( m_pPanelRte, -1, wxDefaultPosition, wxSize( 400, -1 ),
			wxLC_REPORT  | wxLC_SORT_ASCENDING | wxLC_HRULES
			| wxBORDER_SUNKEN/*|wxLC_VRULES*/);
	m_pRouteListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED,
			wxListEventHandler(RouteManagerDialog::OnRteSelected), NULL, this );
	m_pRouteListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,
			wxListEventHandler(RouteManagerDialog::OnRteSelected), NULL, this );
	m_pRouteListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
			wxListEventHandler(RouteManagerDialog::OnRteDefaultAction), NULL, this );
	m_pRouteListCtrl->Connect( wxEVT_LEFT_DOWN,
			wxMouseEventHandler(RouteManagerDialog::OnRteToggleVisibility), NULL, this );
	m_pRouteListCtrl->Connect( wxEVT_COMMAND_LIST_COL_CLICK,
			wxListEventHandler(RouteManagerDialog::OnRteColumnClicked), NULL, this );
	sbsRoutes->Add( m_pRouteListCtrl, 1, wxEXPAND | wxALL, DIALOG_MARGIN );

	// Columns: visibility ctrl, name
	// note that under MSW for SetColumnWidth() to work we need to create the
	// items with images initially even if we specify dummy image id

	m_pRouteListCtrl->InsertColumn(rmVISIBLE, _("Show"), wxLIST_FORMAT_LEFT, 40);
	m_pRouteListCtrl->InsertColumn(rmROUTENAME, _("Route Name"), wxLIST_FORMAT_LEFT, 120);
	m_pRouteListCtrl->InsertColumn(rmROUTEDESC, _("To"), wxLIST_FORMAT_LEFT, 230);

	// Buttons: Delete, Properties...
	wxBoxSizer* bsRouteButtons = new wxBoxSizer(wxVERTICAL);
	sbsRoutes->Add(bsRouteButtons, 0, wxALIGN_RIGHT);

	btnRteProperties = new wxButton(m_pPanelRte, -1, _("&Properties..."));
	bsRouteButtons->Add(btnRteProperties, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnRteProperties->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							  wxCommandEventHandler(RouteManagerDialog::OnRtePropertiesClick), NULL,
							  this);

	btnRteActivate = new wxButton(m_pPanelRte, -1, _("&Activate"));
	bsRouteButtons->Add(btnRteActivate, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnRteActivate->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							wxCommandEventHandler(RouteManagerDialog::OnRteActivateClick), NULL,
							this);
	btnRteActivate->Connect(wxEVT_LEFT_DOWN,
							wxMouseEventHandler(RouteManagerDialog::OnRteBtnLeftDown), NULL, this);

	btnRteZoomto = new wxButton(m_pPanelRte, -1, _("&Center View"));
	bsRouteButtons->Add(btnRteZoomto, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnRteZoomto->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						  wxCommandEventHandler(RouteManagerDialog::OnRteZoomtoClick), NULL, this);
	btnRteZoomto->Connect(wxEVT_LEFT_DOWN,
						  wxMouseEventHandler(RouteManagerDialog::OnRteBtnLeftDown), NULL, this);

	btnRteReverse = new wxButton(m_pPanelRte, -1, _("&Reverse"));
	bsRouteButtons->Add(btnRteReverse, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnRteReverse->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						   wxCommandEventHandler(RouteManagerDialog::OnRteReverseClick), NULL,
						   this);

	btnRteDelete = new wxButton(m_pPanelRte, -1, _("&Delete"));
	bsRouteButtons->Add(btnRteDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnRteDelete->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						  wxCommandEventHandler(RouteManagerDialog::OnRteDeleteClick), NULL, this);

	btnRteExport = new wxButton(m_pPanelRte, -1, _("&Export selected..."));
	bsRouteButtons->Add(btnRteExport, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnRteExport->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						  wxCommandEventHandler(RouteManagerDialog::OnRteExportClick), NULL, this);

	btnRteSendToGPS = new wxButton(m_pPanelRte, -1, _("&Send to GPS"));
	bsRouteButtons->Add(btnRteSendToGPS, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnRteSendToGPS->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							 wxCommandEventHandler(RouteManagerDialog::OnRteSendToGPSClick), NULL,
							 this);

	bsRouteButtons->AddSpacer(10);

	btnRteDeleteAll = new wxButton(m_pPanelRte, -1, _("&Delete All"));
	bsRouteButtons->Add(btnRteDeleteAll, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnRteDeleteAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							 wxCommandEventHandler(RouteManagerDialog::OnRteDeleteAllClick), NULL,
							 this);
}

void RouteManagerDialog::create_tracks_panel()
{
	m_pPanelTrk = new wxPanel(m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
							  wxNO_BORDER | wxTAB_TRAVERSAL);
	wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_pPanelTrk->SetSizer(itemBoxSizer3);
	m_pNotebook->AddPage(m_pPanelTrk, _("Tracks"));

	m_pTrkListCtrl = new wxListCtrl(m_pPanelTrk, -1, wxDefaultPosition, wxSize(400, -1),
									wxLC_REPORT | wxLC_SORT_ASCENDING | wxLC_HRULES
									| wxBORDER_SUNKEN /*|wxLC_VRULES*/);
	m_pTrkListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
							wxListEventHandler(RouteManagerDialog::OnTrkSelected), NULL, this);
	m_pTrkListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED,
							wxListEventHandler(RouteManagerDialog::OnTrkSelected), NULL, this);
	m_pTrkListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
							wxListEventHandler(RouteManagerDialog::OnTrkDefaultAction), NULL, this);
	m_pTrkListCtrl->Connect(wxEVT_LEFT_DOWN,
							wxMouseEventHandler(RouteManagerDialog::OnTrkToggleVisibility), NULL,
							this);
	m_pTrkListCtrl->Connect(wxEVT_COMMAND_LIST_COL_CLICK,
							wxListEventHandler(RouteManagerDialog::OnTrkColumnClicked), NULL, this);
	m_pTrkListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
							wxListEventHandler(RouteManagerDialog::OnTrkRightClick), NULL, this);
	this->Connect(wxEVT_COMMAND_MENU_SELECTED,
				  wxCommandEventHandler(RouteManagerDialog::OnTrkMenuSelected), NULL, this);

	itemBoxSizer3->Add(m_pTrkListCtrl, 1, wxEXPAND | wxALL, DIALOG_MARGIN);

	m_pTrkListCtrl->InsertColumn(colTRKVISIBLE, _("Show"), wxLIST_FORMAT_LEFT, 40);
	m_pTrkListCtrl->InsertColumn(colTRKNAME, _("Track Name"), wxLIST_FORMAT_LEFT, 250);
	m_pTrkListCtrl->InsertColumn(colTRKLENGTH, _("Length"), wxLIST_FORMAT_LEFT, 100);

	wxBoxSizer* bsTrkButtons = new wxBoxSizer(wxVERTICAL);
	itemBoxSizer3->Add(bsTrkButtons, 0, wxALIGN_RIGHT);

	btnTrkNew = new wxButton(m_pPanelTrk, -1, _("&Start Track"));
	bsTrkButtons->Add(btnTrkNew, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnTrkNew->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
					   wxCommandEventHandler(RouteManagerDialog::OnTrkNewClick), NULL, this);

	btnTrkProperties = new wxButton(m_pPanelTrk, -1, _("&Properties"));
	bsTrkButtons->Add(btnTrkProperties, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnTrkProperties->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							  wxCommandEventHandler(RouteManagerDialog::OnTrkPropertiesClick), NULL,
							  this);

	btnTrkDelete = new wxButton(m_pPanelTrk, -1, _("&Delete"));
	bsTrkButtons->Add(btnTrkDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnTrkDelete->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						  wxCommandEventHandler(RouteManagerDialog::OnTrkDeleteClick), NULL, this);

	btnTrkExport = new wxButton(m_pPanelTrk, -1, _("&Export selected..."));
	bsTrkButtons->Add(btnTrkExport, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnTrkExport->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						  wxCommandEventHandler(RouteManagerDialog::OnTrkExportClick), NULL, this);

	btnTrkRouteFromTrack = new wxButton(m_pPanelTrk, -1, _("Route from Track"));
	bsTrkButtons->Add(btnTrkRouteFromTrack, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnTrkRouteFromTrack->Connect(
		wxEVT_COMMAND_BUTTON_CLICKED,
		wxCommandEventHandler(RouteManagerDialog::OnTrkRouteFromTrackClick), NULL, this);

	bsTrkButtons->AddSpacer(10);

	btnTrkDeleteAll = new wxButton(m_pPanelTrk, -1, _("&Delete All"));
	bsTrkButtons->Add(btnTrkDeleteAll, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnTrkDeleteAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							 wxCommandEventHandler(RouteManagerDialog::OnTrkDeleteAllClick), NULL,
							 this);
}

void RouteManagerDialog::create_waypoints_panel(wxBoxSizer* itemBoxSizer1)
{
	m_pPanelWpt = new wxPanel(m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
							  wxNO_BORDER | wxTAB_TRAVERSAL);
	wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
	m_pPanelWpt->SetSizer(itemBoxSizer4);
	m_pNotebook->AddPage(m_pPanelWpt, _("Waypoints"));

	m_pWptListCtrl = new wxListCtrl(m_pPanelWpt, -1, wxDefaultPosition, wxSize(400, -1),
									wxLC_REPORT | wxLC_SORT_ASCENDING | wxLC_HRULES
									| wxBORDER_SUNKEN /*|wxLC_VRULES*/);
	m_pWptListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
							wxListEventHandler(RouteManagerDialog::OnWptSelected), NULL, this);
	m_pWptListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED,
							wxListEventHandler(RouteManagerDialog::OnWptSelected), NULL, this);
	m_pWptListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
							wxListEventHandler(RouteManagerDialog::OnWptDefaultAction), NULL, this);
	m_pWptListCtrl->Connect(wxEVT_LEFT_DOWN,
							wxMouseEventHandler(RouteManagerDialog::OnWptToggleVisibility), NULL,
							this);
	m_pWptListCtrl->Connect(wxEVT_COMMAND_LIST_COL_CLICK,
							wxListEventHandler(RouteManagerDialog::OnWptColumnClicked), NULL, this);
	itemBoxSizer4->Add(m_pWptListCtrl, 1, wxEXPAND | wxALL, DIALOG_MARGIN);

	m_pWptListCtrl->InsertColumn(colWPTICON, _("Icon"), wxLIST_FORMAT_LEFT, 44);
	m_pWptListCtrl->InsertColumn(colWPTNAME, _("Waypoint Name"), wxLIST_FORMAT_LEFT, 180);
	m_pWptListCtrl->InsertColumn(colWPTDIST, _("Distance from Ownship"), wxLIST_FORMAT_LEFT, 180);

	wxBoxSizer* bsWptButtons = new wxBoxSizer(wxVERTICAL);
	itemBoxSizer4->Add(bsWptButtons, 0, wxALIGN_RIGHT);

	btnWptNew = new wxButton(m_pPanelWpt, -1, _("&New"));
	bsWptButtons->Add(btnWptNew, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnWptNew->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
					   wxCommandEventHandler(RouteManagerDialog::OnWptNewClick), NULL, this);

	btnWptProperties = new wxButton(m_pPanelWpt, -1, _("&Properties"));
	bsWptButtons->Add(btnWptProperties, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnWptProperties->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							  wxCommandEventHandler(RouteManagerDialog::OnWptPropertiesClick), NULL,
							  this);

	btnWptZoomto = new wxButton(m_pPanelWpt, -1, _("&Center View"));
	bsWptButtons->Add(btnWptZoomto, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnWptZoomto->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						  wxCommandEventHandler(RouteManagerDialog::OnWptZoomtoClick), NULL, this);

	btnWptDelete = new wxButton(m_pPanelWpt, -1, _("&Delete"));
	bsWptButtons->Add(btnWptDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnWptDelete->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						  wxCommandEventHandler(RouteManagerDialog::OnWptDeleteClick), NULL, this);

	btnWptGoTo = new wxButton(m_pPanelWpt, -1, _("&Go To"));
	bsWptButtons->Add(btnWptGoTo, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnWptGoTo->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						wxCommandEventHandler(RouteManagerDialog::OnWptGoToClick), NULL, this);

	btnWptExport = new wxButton(m_pPanelWpt, -1, _("&Export selected..."));
	bsWptButtons->Add(btnWptExport, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnWptExport->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						  wxCommandEventHandler(RouteManagerDialog::OnWptExportClick), NULL, this);

	btnWptSendToGPS = new wxButton(m_pPanelWpt, -1, _("&Send to GPS"));
	bsWptButtons->Add(btnWptSendToGPS, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnWptSendToGPS->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							 wxCommandEventHandler(RouteManagerDialog::OnWptSendToGPSClick), NULL,
							 this);

	bsWptButtons->AddSpacer(10);

	btnWptDeleteAll = new wxButton(m_pPanelWpt, -1, _("Delete All"));
	bsWptButtons->Add(btnWptDeleteAll, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnWptDeleteAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							 wxCommandEventHandler(RouteManagerDialog::OnWptDeleteAllClick), NULL,
							 this);

	wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
	itemBoxSizer1->Add(itemBoxSizer5, 0, wxALL | wxEXPAND);

	wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
	itemBoxSizer5->Add(itemBoxSizer6, 1, wxALL | wxEXPAND | wxALIGN_LEFT);

	btnImport = new wxButton(this, -1, _("I&mport GPX..."));
	itemBoxSizer6->Add(btnImport, 0, wxALL | wxALIGN_LEFT, DIALOG_MARGIN);
	btnImport->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
					   wxCommandEventHandler(RouteManagerDialog::OnImportClick), NULL, this);

	btnExportViz = new wxButton(this, -1, _("Export All Visible..."));
	itemBoxSizer6->Add(btnExportViz, 0, wxALL | wxALIGN_LEFT, DIALOG_MARGIN);
	btnExportViz->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						  wxCommandEventHandler(RouteManagerDialog::OnExportVizClick), NULL, this);

	// Dialog buttons
	wxSizer* szButtons = CreateButtonSizer(wxOK);
	itemBoxSizer5->Add(szButtons, 0, wxALL | wxALIGN_RIGHT, DIALOG_MARGIN);
}

void RouteManagerDialog::create_layers_panel()
{
	m_pPanelLay = new wxPanel(m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
							  wxNO_BORDER | wxTAB_TRAVERSAL);
	wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
	m_pPanelLay->SetSizer(itemBoxSizer7);
	m_pNotebook->AddPage(m_pPanelLay, _("Layers"));

	m_pLayListCtrl = new wxListCtrl(m_pPanelLay, -1, wxDefaultPosition, wxSize(400, -1),
									wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING
									| wxLC_HRULES | wxBORDER_SUNKEN /*|wxLC_VRULES*/);
	m_pLayListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
							wxListEventHandler(RouteManagerDialog::OnLaySelected), NULL, this);
	m_pLayListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED,
							wxListEventHandler(RouteManagerDialog::OnLaySelected), NULL, this);
	m_pLayListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
							wxListEventHandler(RouteManagerDialog::OnLayDefaultAction), NULL, this);
	m_pLayListCtrl->Connect(wxEVT_LEFT_DOWN,
							wxMouseEventHandler(RouteManagerDialog::OnLayToggleVisibility), NULL,
							this);
	m_pLayListCtrl->Connect(wxEVT_COMMAND_LIST_COL_CLICK,
							wxListEventHandler(RouteManagerDialog::OnLayColumnClicked), NULL, this);
	itemBoxSizer7->Add(m_pLayListCtrl, 1, wxEXPAND | wxALL, DIALOG_MARGIN);

	m_pLayListCtrl->InsertColumn(colLAYVISIBLE, _T(""), wxLIST_FORMAT_LEFT, 28);
	m_pLayListCtrl->InsertColumn(colLAYNAME, _("Layer Name"), wxLIST_FORMAT_LEFT, 250);
	m_pLayListCtrl->InsertColumn(colLAYITEMS, _("No. of items"), wxLIST_FORMAT_LEFT, 100);

	wxBoxSizer* bsLayButtons = new wxBoxSizer(wxVERTICAL);
	itemBoxSizer7->Add(bsLayButtons, 0, wxALIGN_RIGHT);

	btnLayNew = new wxButton(m_pPanelLay, -1, _("Temporary layer"));
	bsLayButtons->Add(btnLayNew, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnLayNew->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
					   wxCommandEventHandler(RouteManagerDialog::OnLayNewClick), NULL, this);

	btnLayDelete = new wxButton(m_pPanelLay, -1, _("&Delete"));
	bsLayButtons->Add(btnLayDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnLayDelete->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						  wxCommandEventHandler(RouteManagerDialog::OnLayDeleteClick), NULL, this);

	btnLayToggleChart = new wxButton(m_pPanelLay, -1, _("Show on chart"));
	bsLayButtons->Add(btnLayToggleChart, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnLayToggleChart->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							   wxCommandEventHandler(RouteManagerDialog::OnLayToggleChartClick),
							   NULL, this);

	btnLayToggleNames = new wxButton(m_pPanelLay, -1, _("Show WPT names"));
	bsLayButtons->Add(btnLayToggleNames, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnLayToggleNames->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							   wxCommandEventHandler(RouteManagerDialog::OnLayToggleNamesClick),
							   NULL, this);

	btnLayToggleListing = new wxButton(m_pPanelLay, -1, _("List contents"));
	bsLayButtons->Add(btnLayToggleListing, 0, wxALL | wxEXPAND, DIALOG_MARGIN);
	btnLayToggleListing->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
								 wxCommandEventHandler(RouteManagerDialog::OnLayToggleListingClick),
								 NULL, this);
}

void RouteManagerDialog::Create()
{
	wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
	SetSizer(itemBoxSizer1);

	m_pNotebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(-1, -1), wxNB_TOP);
	itemBoxSizer1->Add(m_pNotebook, 1,
					   wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);

	create_routes_panel();
	create_tracks_panel();
	create_waypoints_panel(itemBoxSizer1);
	create_layers_panel();

	Fit();
	SetMinSize(GetBestSize());

	// create a image list for the list with just the eye icon
	wxImageList* imglist = new wxImageList(20, 20, true, 1);
	imglist->Add(wxBitmap(eye));
	imglist->Add(wxBitmap(eyex));
	m_pRouteListCtrl->AssignImageList(imglist, wxIMAGE_LIST_SMALL);
	// Assign will handle destroy, Set will not. It's OK, that's what we want
	m_pTrkListCtrl->SetImageList(imglist, wxIMAGE_LIST_SMALL);
	m_pWptListCtrl->SetImageList(pWayPointMan->Getpmarkicon_image_list(), wxIMAGE_LIST_SMALL);
	m_pLayListCtrl->SetImageList(imglist, wxIMAGE_LIST_SMALL);

	SetColorScheme();

	UpdateRouteListCtrl();
	UpdateTrkListCtrl();
	UpdateWptListCtrl();
	UpdateLayListCtrl();

	// This should work under Linux :-(
	// m_pNotebook->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
	// wxNotebookEventHandler(RouteManagerDialog::OnTabSwitch), NULL, this);

	m_bNeedConfigFlush = false;
}

RouteManagerDialog::~RouteManagerDialog()
{
	delete m_pRouteListCtrl;
	delete m_pTrkListCtrl;
	delete m_pWptListCtrl;
	delete m_pLayListCtrl;

	delete btnRteDelete;
	delete btnRteExport;
	delete btnRteZoomto;
	delete btnRteProperties;
	delete btnRteActivate;
	delete btnRteReverse;
	delete btnRteSendToGPS;
	delete btnRteDeleteAll;
	delete btnTrkNew;
	delete btnTrkProperties;
	delete btnTrkDelete;
	delete btnTrkExport;
	delete btnTrkRouteFromTrack;
	delete btnTrkDeleteAll;
	delete btnWptNew;
	delete btnWptProperties;
	delete btnWptZoomto;
	delete btnWptDelete;
	delete btnWptGoTo;
	delete btnWptExport;
	delete btnWptSendToGPS;
	delete btnWptDeleteAll;
	delete btnLayNew;
	delete btnLayToggleChart;
	delete btnLayToggleListing;
	delete btnLayToggleNames;
	delete btnLayDelete;
	delete btnImport;
	delete btnExport;
	delete btnExportViz;
	btnImport = NULL;
	btnExport = NULL;
	btnExportViz = NULL;

	delete m_pNotebook;

	// Does not need to be done here at all, since this dialog is autommatically deleted as a child
	// of the frame.
	// By that time, the config has already been updated for shutdown.
}

void RouteManagerDialog::SetColorScheme()
{
	DimeControl(this);
}

void RouteManagerDialog::UpdateRouteListCtrl()
{
	// if an item was selected, make it selected again if it still exist
	long item = m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	long selected_id = -1;
	if (item != -1)
		selected_id = m_pRouteListCtrl->GetItemData(item);

	// Delete existing items
	m_pRouteListCtrl->DeleteAllItems();

	// then add routes to the listctrl
	int index = 0;
	for (RouteList::iterator it = pRouteList->begin(); it != pRouteList->end(); ++it, ++index) {
		if ((*it)->m_bIsTrack || !(*it)->IsListed())
			continue;

		wxListItem li;
		li.SetId(index);
		li.SetImage((*it)->IsVisible() ? 0 : 1);
		li.SetData(index);
		li.SetText(_T(""));

		if ((*it)->m_bRtIsActive) {
			wxFont font = *wxNORMAL_FONT;
			font.SetWeight(wxFONTWEIGHT_BOLD);
			li.SetFont(font);
		}

		long idx = m_pRouteListCtrl->InsertItem(li);

		wxString name = (*it)->m_RouteNameString;
		if (name.IsEmpty())
			name = _("(Unnamed Route)");
		m_pRouteListCtrl->SetItem(idx, rmROUTENAME, name);

		wxString startend = (*it)->m_RouteStartString;
		if (!(*it)->m_RouteEndString.IsEmpty())
			startend.append(_(" - ") + (*it)->m_RouteEndString);
		m_pRouteListCtrl->SetItem(idx, rmROUTEDESC, startend);
	}

	SortContext sorting = { m_pRouteListCtrl, sort_route_name_dir, 1 };
	m_pRouteListCtrl->SortItems(SortColumnText, (long)&sorting);

	// restore selection if possible
	// NOTE this will select a different item, if one is deleted
	// (the next route will get that index).
	if (selected_id > -1) {
		item = m_pRouteListCtrl->FindItem(-1, selected_id);
		m_pRouteListCtrl->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}

	if ((m_lastRteItem >= 0) && (m_pRouteListCtrl->GetItemCount()))
		m_pRouteListCtrl->EnsureVisible(m_lastRteItem);
	UpdateRteButtons();
}

void RouteManagerDialog::UpdateRteButtons()
{
	// enable/disable buttons
	long selected_index_index
		= m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	bool enable1 = m_pRouteListCtrl->GetSelectedItemCount() == 1;
	bool enablemultiple = m_pRouteListCtrl->GetSelectedItemCount() >= 1;

	m_lastRteItem = selected_index_index;

	btnRteDelete->Enable(m_pRouteListCtrl->GetSelectedItemCount() > 0);
	btnRteZoomto->Enable(enable1);
	btnRteProperties->Enable(enable1);
	btnRteReverse->Enable(enable1);
	btnRteExport->Enable(enablemultiple);
	btnRteSendToGPS->Enable(enable1);
	btnRteDeleteAll->Enable(enablemultiple);

	// set activate button text
	Route* route = NULL;
	if (enable1) {
		long item_index = m_pRouteListCtrl->GetItemData(selected_index_index);
		route = pRouteList->at(item_index);
	}

	if (!g_pRouteMan->IsAnyRouteActive()) {
		btnRteActivate->Enable(enable1);
		if (enable1)
			btnRteActivate->SetLabel(_("Activate"));
	} else {
		if (enable1) {
			if (route && route->m_bRtIsActive) {
				btnRteActivate->Enable(enable1);
				btnRteActivate->SetLabel(_("Deactivate"));
			} else
				btnRteActivate->Enable(false);
		} else
			btnRteActivate->Enable(false);
	}
}

void RouteManagerDialog::MakeAllRoutesInvisible()
{
	long index = 0;
	for (RouteList::iterator it = pRouteList->begin(); it != pRouteList->end(); ++it, ++index) {
		if ((*it)->IsVisible()) { // avoid config updating as much as possible!
			(*it)->SetVisible(false);

			// Likely not same order
			m_pRouteListCtrl->SetItemImage(m_pRouteListCtrl->FindItem(-1, index), 1);
			pConfig->UpdateRoute(*it); // flushes config to disk. FIXME
		}
	}
}

void RouteManagerDialog::ZoomtoRoute(Route* route)
{
	// Calculate bbox center
	double clat = route->RBBox.GetMinY() + (route->RBBox.GetHeight() / 2);
	double clon = route->RBBox.GetMinX() + (route->RBBox.GetWidth() / 2);

	if (clon > 180.0)
		clon -= 360.0;
	else if (clon < -180.0)
		clon += 360.0;

	// Calculate ppm
	double rw, rh, ppm; // route width, height, final ppm scale to use
	int ww, wh; // chart window width, height
	// route bbox width in nm
	geo::DistanceBearingMercator(route->RBBox.GetMinY(), route->RBBox.GetMinX(),
								 route->RBBox.GetMinY(), route->RBBox.GetMaxX(), NULL, &rw);
	// route bbox height in nm
	geo::DistanceBearingMercator(route->RBBox.GetMinY(), route->RBBox.GetMinX(),
								 route->RBBox.GetMaxY(), route->RBBox.GetMinX(), NULL, &rh);

	cc1->GetSize(&ww, &wh);
	ppm = wxMin(ww / (rw * 1852.0), wh / (rh * 1852.0)) * (100 - fabs(clat)) / 90;
	ppm = wxMin(ppm, 1.0);
	gFrame->JumpToPosition(Position(clat, clon), ppm);
	m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteDeleteClick(wxCommandEvent&)
{
	RouteList list;

	int answer = OCPNMessageBox(this, _("Are you sure you want to delete the selected object(s)"),
								wxString(_("OpenCPN Alert")), wxYES_NO);

	if (answer != wxID_YES)
		return;

	bool busy = false;
	if (m_pRouteListCtrl->GetSelectedItemCount()) {
		::wxBeginBusyCursor();
		cc1->CancelMouseRoute();
		m_bNeedConfigFlush = true;
		busy = true;
	}

	long item = -1;
	for (;;) {
		item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;

		long list_index = m_pRouteListCtrl->GetItemData(item);
		Route* proute_to_delete = pRouteList->at(list_index);

		if (proute_to_delete)
			list.push_back(proute_to_delete);
	}

	if (busy) {
		for (unsigned int i = 0; i < list.size(); ++i) {
			Route* route = list.at(i);
			if (route) {
				pConfig->DeleteConfigRoute(route);
				g_pRouteMan->DeleteRoute(route);
			}
		}

		m_lastRteItem = -1;
		UpdateRouteListCtrl();
		UpdateTrkListCtrl();

		cc1->undo->InvalidateUndo();
		cc1->Refresh();
		::wxEndBusyCursor();
	}
}

void RouteManagerDialog::OnRteDeleteAllClick(wxCommandEvent&)
{
	int dialog_ret = OCPNMessageBox(this, _("Are you sure you want to delete <ALL> routes?"),
									wxString(_("OpenCPN Alert")), wxYES_NO);

	if (dialog_ret == wxID_YES) {
		if (g_pRouteMan->GetpActiveRoute())
			g_pRouteMan->DeactivateRoute();

		cc1->CancelMouseRoute();

		g_pRouteMan->DeleteAllRoutes();

		m_lastRteItem = -1;
		UpdateRouteListCtrl();

		// Also need to update the track list control, since routes and tracks share a common global
		// route list
		UpdateTrkListCtrl();

		if (pRoutePropDialog)
			pRoutePropDialog->Hide();
		cc1->undo->InvalidateUndo();
		cc1->Refresh();

		m_bNeedConfigFlush = true;
	}
}

void RouteManagerDialog::OnRtePropertiesClick(wxCommandEvent&)
{
	// Show routeproperties dialog for selected route
	long item = m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	long item_index = m_pRouteListCtrl->GetItemData(item);
	Route* route = pRouteList->at(item_index);

	if (!route)
		return;

	if (!route->m_bIsTrack) { // TODO: It's a route, we still need the new implementation here
		if (NULL == pRoutePropDialog) // There is one global instance of the RouteProp Dialog
			pRoutePropDialog = new RouteProp(GetParent());

		pRoutePropDialog->SetRouteAndUpdate(route);
		pRoutePropDialog->UpdateProperties();
		if (!route->m_bIsInLayer)
			pRoutePropDialog->SetDialogTitle(_("Route Properties"));
		else {
			wxString caption(_T("Route Properties, Layer: "));
			caption.Append(GetLayerName(route->m_LayerID));
			pRoutePropDialog->SetDialogTitle(caption);
		}

		if (!pRoutePropDialog->IsShown())
			pRoutePropDialog->Show();
	}
	m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteZoomtoClick(wxCommandEvent&)
{
	// Zoom into the bounding box of the selected route
	long item = -1;
	item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	// optionally make this route exclusively visible
	if (m_bCtrlDown)
		MakeAllRoutesInvisible();

	long item_index = m_pRouteListCtrl->GetItemData(item);
	Route* route = pRouteList->at(item_index);

	if (!route)
		return;

	// Ensure route is visible
	if (!route->IsVisible()) {
		route->SetVisible(true);
		m_pRouteListCtrl->SetItemImage(item, route->IsVisible() ? 0 : 1);
		pConfig->UpdateRoute(route);
	}

	ZoomtoRoute(route);
}

void RouteManagerDialog::OnRteReverseClick(wxCommandEvent&)
{
	// Reverse selected route
	long item = -1;
	item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	long item_index = m_pRouteListCtrl->GetItemData(item);
	Route* route = pRouteList->at(item_index);

	if (!route)
		return;
	if (route->m_bIsInLayer)
		return;

	int ask_return = OCPNMessageBox(this, g_pRouteMan->GetRouteReverseMessage(),
									_("Rename Waypoints?"), wxYES_NO);
	bool rename = (ask_return == wxID_YES);

	pSelect->DeleteAllSelectableRouteSegments(route);
	route->Reverse(rename);
	pSelect->AddAllSelectableRouteSegments(route);

	// update column 2 - create a UpdateRouteItem(index) instead?
	wxString startend = route->m_RouteStartString;
	if (!route->m_RouteEndString.IsEmpty())
		startend.append(_(" - ") + route->m_RouteEndString);
	m_pRouteListCtrl->SetItem(item, 2, startend);

	pConfig->UpdateRoute(route);
	cc1->Refresh();

	m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteExportClick(wxCommandEvent&)
{
	RouteList list;

	wxString suggested_name = _T("routes");

	long item = -1;
	for (;;) {
		item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;

		long item_index = m_pRouteListCtrl->GetItemData(item);
		Route* route = pRouteList->at(item_index);

		if (route) {
			list.push_back(route);
			if (route->m_RouteNameString != wxEmptyString)
				suggested_name = route->m_RouteNameString;
		}
	}

	pConfig->ExportGPXRoutes(this, &list, suggested_name);
}

void RouteManagerDialog::OnRteActivateClick(wxCommandEvent&)
{
	// Activate the selected route, unless it already is
	long item = m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	if (m_bCtrlDown)
		MakeAllRoutesInvisible();

	long item_index = m_pRouteListCtrl->GetItemData(item);
	Route* route = pRouteList->at(item_index);

	if (!route)
		return;

	if (!route->m_bRtIsActive) {
		if (!route->IsVisible()) {
			route->SetVisible(true);
			m_pRouteListCtrl->SetItemImage(item, 0, 0);
		}

		ZoomtoRoute(route);

		const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
		RoutePoint* best_point
			= g_pRouteMan->FindBestActivatePoint(route, Position(nav.lat, nav.lon), nav.cog, nav.sog);
		g_pRouteMan->ActivateRoute(route, best_point);
	} else {
		g_pRouteMan->DeactivateRoute();
	}

	UpdateRouteListCtrl();
	pConfig->UpdateRoute(route);
	cc1->Refresh();
	m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteToggleVisibility(wxMouseEvent& event)
{
	wxPoint pos = event.GetPosition();
	int flags = 0;
	long clicked_index = m_pRouteListCtrl->HitTest(pos, flags);

	//    Clicking Visibility column?
	if (clicked_index > -1 && event.GetX() < m_pRouteListCtrl->GetColumnWidth(rmVISIBLE)) {
		// Process the clicked item
		long list_index = m_pRouteListCtrl->GetItemData(clicked_index);
		Route* route = pRouteList->at(list_index);

		int wpts_set_viz = wxID_YES;
		bool togglesharedwpts = true;
		bool has_shared_wpts = g_pRouteMan->DoesRouteContainSharedPoints(route);

		if (has_shared_wpts && route->IsVisible()) {
			wpts_set_viz = OCPNMessageBox(this, _("Do you also want to make the shared waypoints being part of this route invisible?"),
										  _("Question"), wxYES_NO);
			togglesharedwpts = (wpts_set_viz == wxID_YES);
		}
		route->SetVisible(!route->IsVisible(), togglesharedwpts);
		m_pRouteListCtrl->SetItemImage(clicked_index, route->IsVisible() ? 0 : 1);

		::wxBeginBusyCursor();

		pConfig->UpdateRoute(route);
		cc1->Refresh();

		//   We need to update the waypoint list control only if the visibility of shared waypoints
		// might have changed.
		if (has_shared_wpts)
			UpdateWptListCtrlViz();

		::wxEndBusyCursor();
	}

	// Allow wx to process...
	event.Skip();
}

// FIXME add/remove route segments/waypoints from selectable items, so there are no
// hidden selectables! This should probably be done outside this class!
// The problem is that the current waypoint class does not provide good support
// for this, there is a "visible" property, but no means for proper management.
// Jan. 28 2010: Ideas:
// - Calculate on the fly how many visible routes use a waypoint.
//   This requires a semidouble loop (routes, waypoints in visible routes). It could
//   be done by the function getting the selection. Potentially somewhat slow?
// - OR keep a property in waypoints telling that
//   (A number, increased/decreased for each waypoint by Route::SetVisible()).
//   Immediate result when detecting the selectable object, small overhead in
//   Route::SetVisible(). I prefer this.
// - We also need to know if the waypoint should otherwise be visible,
//   ie it is a "normal" waypoint used in the route (then it should be visible
//   in all cases). Is this possible with current code?
// - Get rid of the Select objects, they do no good! They should be replaced with a function
//   in the application, the search would reqire equal amount of looping, but less
//   dereferencing pointers, and it would remove the overhead of keeping and maintaining
//   the extra pointer lists.

void RouteManagerDialog::OnRteBtnLeftDown(wxMouseEvent& event)
{
	m_bCtrlDown = event.ControlDown();
	event.Skip();
}

void RouteManagerDialog::OnRteSelected(wxListEvent& event)
{
	long clicked_index = event.m_itemIndex;
	// Process the clicked item
	long list_index = m_pRouteListCtrl->GetItemData(clicked_index);
	Route* route = pRouteList->at(list_index);
	m_pRouteListCtrl->SetItemImage(clicked_index, route->IsVisible() ? 0 : 1);

	if (cc1)
		cc1->Refresh();

	UpdateRteButtons();
}

void RouteManagerDialog::OnRteColumnClicked(wxListEvent& event)
{
	if (event.m_col == 1) {
		sort_route_name_dir++;
		SortContext sorting = { m_pRouteListCtrl, sort_route_name_dir, 1 };
		m_pRouteListCtrl->SortItems(SortColumnText, (long)&sorting);
	} else {
		if (event.m_col == 2) {
			sort_route_to_dir++;
			SortContext sorting = { m_pRouteListCtrl, sort_route_to_dir, 2 };
			m_pRouteListCtrl->SortItems(SortColumnText, (long)&sorting);
		}
	}
}

void RouteManagerDialog::OnRteSendToGPSClick(wxCommandEvent&)
{
	long item = m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	long item_index = m_pRouteListCtrl->GetItemData(item);
	Route* route = pRouteList->at(item_index);

	if (!route)
		return;

	SendToGpsDlg* pdlg = new SendToGpsDlg();
	pdlg->SetRoute(route);

	wxString source;
	pdlg->Create(NULL, -1, _("Send To GPS..."), source);
	pdlg->ShowModal();

	delete pdlg;
}

void RouteManagerDialog::OnRteDefaultAction(wxListEvent&)
{
	wxCommandEvent evt;
	OnRtePropertiesClick(evt);
}

void RouteManagerDialog::OnTrkDefaultAction(wxListEvent&)
{
	wxCommandEvent evt;
	OnTrkPropertiesClick(evt);
}

void RouteManagerDialog::OnTrkRightClick(wxListEvent&)
{
	wxMenu menu;
	wxMenuItem* mergeItem = menu.Append(TRACK_MERGE, _("&Merge Selected Tracks"));
	mergeItem->Enable(m_pTrkListCtrl->GetSelectedItemCount() > 1);
	wxMenuItem* cleanItem = menu.Append(TRACK_CLEAN, _("Reduce Data..."));
	cleanItem->Enable(m_pTrkListCtrl->GetSelectedItemCount() == 1);
	wxMenuItem* copyItem = menu.Append(TRACK_COPY_TEXT, _("&Copy as text"));
	copyItem->Enable(m_pTrkListCtrl->GetSelectedItemCount() > 0);
	PopupMenu(&menu);
}

struct TrackCompareCreateTime
{
	bool operator()(Track* a, Track* b) const // FIXME: RoutePoint::GetCreateTime is a lazy initialization mess
	{
		RoutePoint* start1 = a->pRoutePointList->front();
		RoutePoint* start2 = b->pRoutePointList->front();
		if (start1->GetCreateTime() > start2->GetCreateTime())
			return true;
		return false;
	}
};

void RouteManagerDialog::OnTrkMenuSelected(wxCommandEvent& event)
{
	int item = -1;

	switch (event.GetId()) {

		case TRACK_CLEAN: {
			item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if (item == -1)
				break;
			long list_index = m_pTrkListCtrl->GetItemData(item);
			Track* track = dynamic_cast<Track*>(pRouteList->at(list_index));
			if (track->IsRunning()) {
				wxBell();
				break;
			}

			wxString choices[] = { _T("5.0"), _T("10.0"), _T("20.0"), _T("50.0"), _T("100.0") };
			wxSingleChoiceDialog* precisionDlg = new wxSingleChoiceDialog(
				this, _("Select the maximum error allowed (in meters)\nafter data reduction:"),
				_("Reduce Data Precision"), 5, choices);

			int result = precisionDlg->ShowModal();
			if (result == wxID_CANCEL)
				break;
			double precision = 5.0;
			switch (precisionDlg->GetSelection()) {
				case 0:
					precision = 5.0;
					break;
				case 1:
					precision = 10.0;
					break;
				case 2:
					precision = 20.0;
					break;
				case 3:
					precision = 50.0;
					break;
				case 4:
					precision = 100.0;
					break;
			}

			int pointsBefore = track->GetnPoints();

			int reduction = track->Simplify(precision);
			gFrame->Refresh(false);

			reduction = 100 * reduction / pointsBefore;
			wxString msg = wxString::Format(
				_("The amount of data used by the track\n was reduced by %d%%."), reduction);
			OCPNMessageBox(this, msg, _("OpenCPN info"), wxICON_INFORMATION | wxOK);

			UpdateTrkListCtrl();
			UpdateRouteListCtrl();

			break;
		}

		case TRACK_COPY_TEXT: {
			wxString csvString;
			while (true) {
				item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
				if (item == -1)
					break;
				long list_index = m_pTrkListCtrl->GetItemData(item);
				const Track* track = dynamic_cast<const Track*>(pRouteList->at(list_index));
				csvString << track->m_RouteNameString << _T("\t")
						  << wxString::Format(_T("%.1f"), track->m_route_length) << _T("\t")
						  << _T("\n");
			}

			if (wxTheClipboard->Open()) {
				wxTextDataObject* data = new wxTextDataObject;
				data->SetText(csvString);
				wxTheClipboard->SetData(data);
				wxTheClipboard->Close();
			}

			break;
		}

		case TRACK_MERGE: {
			std::vector<Track*> mergeList;
			std::vector<Track*> deleteList;
			bool runningSkipped = false;

			::wxBeginBusyCursor();

			while (true) {
				item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
				if (item == -1)
					break;
				long list_index = m_pTrkListCtrl->GetItemData(item);
				Track* track = dynamic_cast<Track*>(pRouteList->at(list_index));
				mergeList.push_back(track);
			}

			std::sort(mergeList.begin(), mergeList.end(), TrackCompareCreateTime()); // TODO: is this sorted the right way, how the fuck does wxFuck::Sort sort. documentation wouldn't hurt.

			Track* targetTrack = mergeList.at(0);
			RoutePoint* lastPoint = targetTrack->GetLastPoint();

			for (unsigned int t = 1; t < mergeList.size(); t++) {

				Track* mergeTrack = mergeList.at(t);

				if (mergeTrack->IsRunning()) {
					runningSkipped = true;
					continue;
				}

				for (RoutePointList::iterator route_point = mergeTrack->pRoutePointList->begin();
					 route_point != mergeTrack->pRoutePointList->end(); ++route_point) {
					RoutePoint* rPoint = *route_point;
					RoutePoint* newPoint = new RoutePoint(rPoint->m_lat, rPoint->m_lon, _T("empty"), _T(""));
					newPoint->m_bShowName = false;
					newPoint->m_bIsVisible = true;
					newPoint->m_GPXTrkSegNo = 1;

					newPoint->SetCreateTime(rPoint->GetCreateTime());

					targetTrack->AddPoint(newPoint);

					newPoint->m_bIsInRoute = false;
					newPoint->m_bIsInTrack = true;

					pSelect->AddSelectableTrackSegment(lastPoint->m_lat, lastPoint->m_lon,
													   newPoint->m_lat, newPoint->m_lon, lastPoint,
													   newPoint, targetTrack);

					lastPoint = newPoint;
				}
				deleteList.push_back(mergeTrack);
			}

			for (unsigned int i = 0; i < deleteList.size(); i++) {
				Track* deleteTrack = deleteList.at(i);
				pConfig->DeleteConfigRoute(deleteTrack);
				g_pRouteMan->DeleteTrack(deleteTrack);
			}

			mergeList.clear();
			deleteList.clear();

			::wxEndBusyCursor();

			UpdateTrkListCtrl();
			UpdateRouteListCtrl();
			cc1->Refresh();

			if (runningSkipped) {
				wxMessageDialog skipWarning(this,
											_("The currently running Track was not merged.\nYou can merge it later when it is completed."),
											_T("Warning"), wxCANCEL | wxICON_WARNING);
				skipWarning.ShowModal();
			}

			break;
		}
	}
}

void RouteManagerDialog::UpdateTrkListCtrl()
{
	// if an item was selected, make it selected again if it still exist
	long item = -1;
	item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	long selected_id = -1;
	if (item != -1)
		selected_id = m_pTrkListCtrl->GetItemData(item);

	// Delete existing items
	m_pTrkListCtrl->DeleteAllItems();

	// then add routes to the listctrl
	int index = 0;
	for (RouteList::iterator it = pRouteList->begin(); it != pRouteList->end(); ++it, ++index) {
		Route* trk = (Route*)*it;
		if (!trk->m_bIsTrack || !trk->IsListed())
			continue;

		wxListItem li;
		li.SetId(index);
		li.SetImage(trk->IsVisible() ? 0 : 1);
		li.SetData(index);
		li.SetText(_T(""));

		if (g_pActiveTrack == trk) {
			wxFont font = *wxNORMAL_FONT;
			font.SetWeight(wxFONTWEIGHT_BOLD);
			li.SetFont(font);
		}
		long idx = m_pTrkListCtrl->InsertItem(li);

		wxString name = trk->m_RouteNameString;
		if (name.IsEmpty()) {
			RoutePoint* rp = trk->GetPoint(1);
			if (rp && rp->GetCreateTime().IsValid())
				name = rp->GetCreateTime().FormatISODate() + _T(" ")
					   + rp->GetCreateTime().FormatISOTime();
			else
				name = _("(Unnamed Track)");
		}
		m_pTrkListCtrl->SetItem(idx, colTRKNAME, name);
		m_pTrkListCtrl->SetItem(idx, colTRKLENGTH, wxString::Format(wxT("%5.2f"), trk->m_route_length));
	}

	SortContext sorting = { m_pTrkListCtrl, sort_track_name_dir, 1 };
	m_pTrkListCtrl->SortItems(SortColumnText, (long)&sorting);

	// restore selection if possible
	// NOTE this will select a different item, if one is deleted
	// (the next route will get that index).
	if (selected_id > -1) {
		item = m_pTrkListCtrl->FindItem(-1, selected_id);
		m_pTrkListCtrl->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}

	if ((m_lastTrkItem >= 0) && (m_pTrkListCtrl->GetItemCount()))
		m_pTrkListCtrl->EnsureVisible(m_lastTrkItem);
	UpdateTrkButtons();
}

void RouteManagerDialog::OnTrkSelected(wxListEvent&)
{
	UpdateTrkButtons();
}

void RouteManagerDialog::OnTrkColumnClicked(wxListEvent& event)
{
	if (event.m_col == 1) {
		sort_track_name_dir++;
		SortContext sorting = { m_pTrkListCtrl, sort_track_name_dir, 1 };
		m_pTrkListCtrl->SortItems(SortColumnText, (long)&sorting);
	} else if (event.m_col == 2) {
		sort_track_len_dir++;
		SortContext sorting = { m_pTrkListCtrl, sort_track_len_dir, 2 };
		m_pTrkListCtrl->SortItems(SortColumnDistance, (long)&sorting);
	}
}

void RouteManagerDialog::UpdateTrkButtons()
{
	long item = -1;
	item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	int items = m_pTrkListCtrl->GetSelectedItemCount();

	m_lastTrkItem = item;

	btnTrkProperties->Enable(items == 1);
	btnTrkDelete->Enable(items >= 1);
	btnTrkExport->Enable(items >= 1);
	btnTrkRouteFromTrack->Enable(items == 1);
	btnTrkDeleteAll->Enable(items >= 1);
}

void RouteManagerDialog::OnTrkToggleVisibility(wxMouseEvent& event)
{
	wxPoint pos = event.GetPosition();
	int flags = 0;
	long clicked_index = m_pTrkListCtrl->HitTest(pos, flags);

	// Clicking Visibility column?
	if (clicked_index > -1 && event.GetX() < m_pTrkListCtrl->GetColumnWidth(colTRKVISIBLE)) {
		// Process the clicked item
		long list_index = m_pTrkListCtrl->GetItemData(clicked_index);
		Route* route = pRouteList->at(list_index);
		route->SetVisible(!route->IsVisible());
		m_pTrkListCtrl->SetItemImage(clicked_index, route->IsVisible() ? 0 : 1);

		cc1->Refresh();
	}

	// Allow wx to process...
	event.Skip();
}

void RouteManagerDialog::OnTrkNewClick(wxCommandEvent&)
{
	gFrame->TrackOff();
	gFrame->TrackOn();

	UpdateTrkListCtrl();
}

void RouteManagerDialog::OnTrkPropertiesClick(wxCommandEvent&)
{
	// Show routeproperties dialog for selected route
	long item = m_pTrkListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	long list_index = m_pTrkListCtrl->GetItemData(item);
	Route* route = pRouteList->at(list_index);

	if (!route)
		return;

	if (NULL == pTrackPropDialog) // There is one global instance of the RouteProp Dialog
		pTrackPropDialog = new TrackPropDlg(GetParent());
	pTrackPropDialog->SetTrackAndUpdate(route);

	if (!pTrackPropDialog->IsShown())
		pTrackPropDialog->Show();
	UpdateTrkListCtrl();

	m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnTrkDeleteClick(wxCommandEvent&)
{
	RouteList list;

	int answer = OCPNMessageBox(this, _("Are you sure you want to delete the selected object(s)"),
								wxString(_("OpenCPN Alert")), wxYES_NO);
	if (answer != wxID_YES)
		return;

	bool busy = false;
	if (m_pTrkListCtrl->GetSelectedItemCount()) {
		::wxBeginBusyCursor();
		m_bNeedConfigFlush = true;
		busy = true;
	}

	long item = -1;
	for (;;) {
		item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;

		long list_index = m_pTrkListCtrl->GetItemData(item);
		Route* ptrack_to_delete = pRouteList->at(list_index);

		if (ptrack_to_delete)
			list.push_back(ptrack_to_delete);
	}

	if (busy) {
		for (unsigned int i = 0; i < list.size(); i++) {
			Track* track = dynamic_cast<Track*>(list.at(i));
			if (track) {
				pConfig->DeleteConfigRoute(track);
				g_pRouteMan->DeleteTrack(track);
			}
		}

		m_lastTrkItem = -1;
		UpdateRouteListCtrl();
		UpdateTrkListCtrl();

		cc1->undo->InvalidateUndo();
		cc1->Refresh();
		::wxEndBusyCursor();
	}
}

void RouteManagerDialog::OnTrkExportClick(wxCommandEvent&)
{
	RouteList list;
	wxString suggested_name = _T("tracks");

	long item = -1;
	for (;;) {
		item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;

		long list_index = m_pTrkListCtrl->GetItemData(item);
		Route* proute_to_export = pRouteList->at(list_index);

		if (proute_to_export) {
			list.push_back(proute_to_export);
			if (proute_to_export->m_RouteNameString != wxEmptyString)
				suggested_name = proute_to_export->m_RouteNameString;
		}
	}

	pConfig->ExportGPXRoutes(this, &list, suggested_name);
}

void RouteManagerDialog::TrackToRoute(Track* track)
{
	if (!track)
		return;
	if (track->m_bIsInLayer)
		return;

	wxProgressDialog* pprog = new wxProgressDialog(_("OpenCPN Converting Track to Route...."),
												   _("Processing Waypoints..."), 101, NULL,
												   wxPD_AUTO_HIDE | wxPD_SMOOTH | wxPD_ELAPSED_TIME
												   | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);

	::wxBeginBusyCursor();

	Route* route = track->RouteFromTrack(pprog);

	pRouteList->push_back(route);

	pprog->Update(101, _("Done."));
	delete pprog;

	cc1->Refresh();

	::wxEndBusyCursor();
}

void RouteManagerDialog::OnTrkRouteFromTrackClick(wxCommandEvent&)
{
	long item = m_pTrkListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	long list_index = m_pTrkListCtrl->GetItemData(item);
	Track* track = static_cast<Track*>(pRouteList->at(list_index));

	TrackToRoute(track);

	UpdateRouteListCtrl();
}

void RouteManagerDialog::OnTrkDeleteAllClick(wxCommandEvent&)
{
	int dialog_ret = OCPNMessageBox(this, _("Are you sure you want to delete <ALL> tracks?"),
									wxString(_("OpenCPN Alert")), wxYES_NO);

	if (dialog_ret == wxID_YES) {
		g_pRouteMan->DeleteAllTracks();
	}

	m_lastTrkItem = -1;
	m_lastRteItem = -1;

	UpdateTrkListCtrl();

	// Also need to update the route list control, since routes and tracks share a common global
	// route list
	UpdateRouteListCtrl();

	if (pRoutePropDialog)
		pRoutePropDialog->Hide();

	cc1->Refresh();

	m_bNeedConfigFlush = true;
}

void RouteManagerDialog::UpdateWptListCtrl(RoutePoint* rp_select, bool b_retain_sort)
{
	long selected_id = -1;
	long item = -1;

	if (NULL == rp_select) {
		// if an item was selected, make it selected again if it still exists
		item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		if (item != -1)
			selected_id = m_pWptListCtrl->GetItemData(item);
	}

	//  Freshen the image list
	m_pWptListCtrl->SetImageList(pWayPointMan->Getpmarkicon_image_list(), wxIMAGE_LIST_SMALL);

	m_pWptListCtrl->DeleteAllItems();

	const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
	int index = 0;
	const RoutePointList& waypoints = pWayPointMan->waypoints();
	for (RoutePointList::const_iterator i = waypoints.begin(); i != waypoints.end(); ++i) {
		const RoutePoint* rp = *i;
		if (rp && rp->IsListed()) {
			if (rp->m_bIsInTrack || rp->m_bIsInRoute) {
				if (!rp->m_bKeepXRoute) {
					continue;
				}
			}

			wxListItem li;
			li.SetId(index);
			li.SetImage(rp->IsVisible() ? pWayPointMan->GetIconIndex(rp->m_pbmIcon)
										: pWayPointMan->GetXIconIndex(rp->m_pbmIcon));
			li.SetData(const_cast<RoutePoint*>(rp));
			li.SetText(_T(""));
			long idx = m_pWptListCtrl->InsertItem(li);

			wxString name = rp->GetName();
			if (name.IsEmpty())
				name = _("(Unnamed Waypoint)");
			m_pWptListCtrl->SetItem(idx, colWPTNAME, name);

			double dst;
			geo::DistanceBearingMercator(rp->m_lat, rp->m_lon, nav.lat, nav.lon, NULL, &dst);
			m_pWptListCtrl->SetItem(idx, colWPTDIST, wxString::Format(_T("%5.2f ") + getUsrDistanceUnit(), toUsrDistance(dst)));

			if (rp == rp_select)
				selected_id = (long)rp_select;

			index++;
		}
	}

	if (!b_retain_sort) {
		SortContext sorting = { m_pWptListCtrl, sort_wp_name_dir, 0 };
		m_pWptListCtrl->SortItems(SortWaypointsOnName, (long)&sorting);
		sort_wp_key = SORT_ON_NAME;
	} else {
		switch (sort_wp_key) {
			case SORT_ON_NAME: {
				SortContext sorting = { m_pWptListCtrl, sort_wp_name_dir, 0 };
				m_pWptListCtrl->SortItems(SortWaypointsOnName, (long)&sorting);
			} break;

			case SORT_ON_DISTANCE: {
				SortContext sorting = { m_pWptListCtrl, sort_wp_len_dir, 2 };
				m_pWptListCtrl->SortItems(SortColumnDistance, (long)&sorting);
			} break;
		}
	}

	if (selected_id > -1) {
		item = m_pWptListCtrl->FindItem(-1, selected_id);
		m_pWptListCtrl->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}

	if ((m_lastWptItem >= 0) && (m_pWptListCtrl->GetItemCount()))
		m_pWptListCtrl->EnsureVisible(m_lastWptItem);
	UpdateWptButtons();
}

void RouteManagerDialog::UpdateWptListCtrlViz()
{
	long item = -1;
	for (;;) {
		item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
		if (item == -1)
			break;

		RoutePoint* pRP = (RoutePoint*)m_pWptListCtrl->GetItemData(item);
		int image = pRP->IsVisible() ? pWayPointMan->GetIconIndex(pRP->m_pbmIcon)
									 : pWayPointMan->GetXIconIndex(pRP->m_pbmIcon);

		m_pWptListCtrl->SetItemImage(item, image);
	}
}

void RouteManagerDialog::OnWptDefaultAction(wxListEvent&)
{
	wxCommandEvent evt;
	OnWptPropertiesClick(evt);
}

void RouteManagerDialog::OnWptSelected(wxListEvent&)
{
	UpdateWptButtons();
}

void RouteManagerDialog::OnWptColumnClicked(wxListEvent& event)
{
	if (event.m_col == 1) {
		sort_wp_name_dir++;
		SortContext sorting = { m_pWptListCtrl, sort_wp_name_dir, 0 };
		m_pWptListCtrl->SortItems(SortWaypointsOnName, (long)&sorting);
		sort_wp_key = SORT_ON_NAME;
	} else if (event.m_col == 2) {
		sort_wp_len_dir++;
		SortContext sorting = { m_pWptListCtrl, sort_wp_len_dir, 02};
		m_pWptListCtrl->SortItems(SortColumnDistance, (long)&sorting);
		sort_wp_key = SORT_ON_DISTANCE;
	}
}

void RouteManagerDialog::UpdateWptButtons()
{
	long item = -1;
	item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	bool enable1 = (m_pWptListCtrl->GetSelectedItemCount() == 1);
	bool enablemultiple = (m_pWptListCtrl->GetSelectedItemCount() >= 1);

	if (enable1)
		m_lastWptItem = item;
	else
		m_lastWptItem = -1;

	// Check selection to see if it is in a layer
	// If so, disable the "delete" button
	bool b_delete_enable = true;
	item = -1;
	for (;;) {
		item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;

		RoutePoint* wp = (RoutePoint*)m_pWptListCtrl->GetItemData(item);

		if (wp && wp->m_bIsInLayer) {
			b_delete_enable = false;
			break;
		}
	}

	btnWptProperties->Enable(enable1);
	btnWptZoomto->Enable(enable1);
	btnWptDeleteAll->Enable(enablemultiple);
	btnWptDelete->Enable(b_delete_enable && enablemultiple);
	btnWptGoTo->Enable(enable1);
	btnWptExport->Enable(enablemultiple);
	btnWptSendToGPS->Enable(enable1);
}

void RouteManagerDialog::OnWptToggleVisibility(wxMouseEvent& event)
{
	wxPoint pos = event.GetPosition();
	int flags = 0;
	long clicked_index = m_pWptListCtrl->HitTest(pos, flags);

	//    Clicking Visibility column?
	if (clicked_index > -1 && event.GetX() < m_pWptListCtrl->GetColumnWidth(colTRKVISIBLE)) {
		// Process the clicked item
		RoutePoint* wp = (RoutePoint*)m_pWptListCtrl->GetItemData(clicked_index);

		wp->SetVisible(!wp->IsVisible());
		m_pWptListCtrl->SetItemImage(clicked_index,
									 wp->IsVisible() ? pWayPointMan->GetIconIndex(wp->m_pbmIcon)
													 : pWayPointMan->GetXIconIndex(wp->m_pbmIcon));

		pConfig->UpdateWayPoint(wp);

		cc1->Refresh();
	}

	// Allow wx to process...
	event.Skip();
}

void RouteManagerDialog::OnWptNewClick(wxCommandEvent&)
{
	const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();

	RoutePoint* pWP = new RoutePoint(nav.lat, nav.lon, g_default_wp_icon, wxEmptyString);
	pWP->m_bIsolatedMark = true; // This is an isolated mark
	pSelect->AddSelectableRoutePoint(nav.lat, nav.lon, pWP);
	pConfig->AddNewWayPoint(pWP, -1); // use auto next num
	cc1->Refresh(false); // Needed for MSW, why not GTK??

	if (NULL == pMarkPropDialog) // There is one global instance of the MarkProp Dialog
		pMarkPropDialog = new MarkInfoImpl(GetParent());

	pMarkPropDialog->SetRoutePoint(pWP);
	pMarkPropDialog->UpdateProperties();

	WptShowPropertiesDialog(pWP, GetParent());
}

void RouteManagerDialog::OnWptPropertiesClick(wxCommandEvent&)
{
	long item = -1;
	item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	RoutePoint* wp = (RoutePoint*)m_pWptListCtrl->GetItemData(item);

	if (!wp)
		return;

	WptShowPropertiesDialog(wp, GetParent());

	UpdateWptListCtrl();
	m_bNeedConfigFlush = true;
}

void RouteManagerDialog::WptShowPropertiesDialog(RoutePoint* wp, wxWindow* parent)
{
	// There is one global instance of the MarkProp Dialog
	if (NULL == pMarkPropDialog)
		pMarkPropDialog = new MarkInfoImpl(parent);

	pMarkPropDialog->SetRoutePoint(wp);
	pMarkPropDialog->UpdateProperties();
	if (wp->m_bIsInLayer) {
		wxString caption(_("Waypoint Properties, Layer: "));
		caption.Append(GetLayerName(wp->get_layer_ID()));
		pMarkPropDialog->SetDialogTitle(caption);
	} else
		pMarkPropDialog->SetDialogTitle(_("Waypoint Properties"));

	if (!pMarkPropDialog->IsShown())
		pMarkPropDialog->Show();
}

void RouteManagerDialog::OnWptZoomtoClick(wxCommandEvent&)
{
	long item = -1;
	item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	RoutePoint* wp = (RoutePoint*)m_pWptListCtrl->GetItemData(item);

	if (!wp)
		return;

	gFrame->JumpToPosition(Position(wp->m_lat, wp->m_lon), cc1->GetVPScale());
}

void RouteManagerDialog::OnWptDeleteClick(wxCommandEvent&)
{
	RoutePointList list;

	int answer = OCPNMessageBox(this, _("Are you sure you want to delete the selected object(s)"),
								wxString(_("OpenCPN Alert")), wxYES_NO);
	if (answer != wxID_YES)
		return;

	bool busy = false;
	if (m_pWptListCtrl->GetSelectedItemCount()) {
		::wxBeginBusyCursor();
		m_bNeedConfigFlush = true;
		busy = true;
	}

	long item = -1;
	long item_last_selected = -1;
	for (;;) {
		item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;

		item_last_selected = item;
		RoutePoint* wp = (RoutePoint*)m_pWptListCtrl->GetItemData(item);

		if (wp && !wp->m_bIsInLayer)
			list.push_back(wp);
	}

	if (busy) {
		for (unsigned int i = 0; i < list.size(); i++) {
			RoutePoint* wp = list.at(i);
			if (wp) {
				if (wp->m_bIsInRoute || wp->m_bIsInTrack) {
					if (wxYES
						== OCPNMessageBox(this, _("The waypoint you want to delete is used in a route, do you really want to delete it?"),
										  _("OpenCPN Alert"), wxYES_NO))
						pWayPointMan->DestroyWaypoint(wp);
				} else {
					pWayPointMan->DestroyWaypoint(wp);
				}
			}
		}

		long item_next = m_pWptListCtrl->GetNextItem(item_last_selected); // next in list
		RoutePoint* wp_next = NULL;
		if (item_next > -1)
			wp_next = (RoutePoint*)m_pWptListCtrl->GetItemData(item_next);

		m_lastWptItem = item_next;

		UpdateRouteListCtrl();
		UpdateTrkListCtrl();
		UpdateWptListCtrl(wp_next, true);

		if (pMarkPropDialog) {
			pMarkPropDialog->SetRoutePoint(NULL);
			pMarkPropDialog->UpdateProperties();
		}

		cc1->undo->InvalidateUndo();
		cc1->Refresh();
		::wxEndBusyCursor();
	}
}

void RouteManagerDialog::OnWptGoToClick(wxCommandEvent&)
{
	long item = -1;
	item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	RoutePoint* wp = (RoutePoint*)m_pWptListCtrl->GetItemData(item);

	if (!wp)
		return;

	const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();

	RoutePoint* pWP_src = new RoutePoint(nav.lat, nav.lon, g_default_wp_icon, wxEmptyString);
	pSelect->AddSelectableRoutePoint(nav.lat, nav.lon, pWP_src);

	Route* temp_route = new Route();
	pRouteList->push_back(temp_route);

	temp_route->AddPoint(pWP_src);
	temp_route->AddPoint(wp);

	pSelect->AddSelectableRouteSegment(nav.lat, nav.lon, wp->m_lat, wp->m_lon, pWP_src, wp,
									   temp_route);

	wxString name = wp->GetName();
	if (name.IsEmpty())
		name = _("(Unnamed Waypoint)");
	wxString rteName = _("Go to ");
	rteName.Append(name);
	temp_route->m_RouteNameString = rteName;
	temp_route->m_RouteStartString = _("Here");

	temp_route->m_RouteEndString = name;
	temp_route->m_bDeleteOnArrival = true;

	if (g_pRouteMan->GetpActiveRoute())
		g_pRouteMan->DeactivateRoute();

	g_pRouteMan->ActivateRoute(temp_route, wp);

	UpdateRouteListCtrl();
}

void RouteManagerDialog::OnWptExportClick(wxCommandEvent&)
{
	RoutePointList list;

	wxString suggested_name = _T("waypoints");

	long item = -1;
	for (;;) {
		item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;

		RoutePoint* wp = (RoutePoint*)m_pWptListCtrl->GetItemData(item);

		if (wp && !wp->m_bIsInLayer) {
			list.push_back(wp);
			if (wp->GetName() != wxEmptyString)
				suggested_name = wp->GetName();
		}
	}

	pConfig->ExportGPXWaypoints(this, &list, suggested_name);
}

void RouteManagerDialog::OnWptSendToGPSClick(wxCommandEvent&)
{
	long item = -1;
	item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	RoutePoint* wp = (RoutePoint*)m_pWptListCtrl->GetItemData(item);

	if (!wp)
		return;

	SendToGpsDlg* pdlg = new SendToGpsDlg();
	pdlg->SetWaypoint(wp);

	wxString source;
	pdlg->Create(NULL, -1, _("Send To GPS..."), source);
	pdlg->ShowModal();

	delete pdlg;
}

void RouteManagerDialog::OnWptDeleteAllClick(wxCommandEvent&)
{
	wxString prompt;
	int buttons, type;
	if (!pWayPointMan->SharedWptsExist()) {
		prompt = _("Are you sure you want to delete <ALL> waypoints?");
		buttons = wxYES_NO;
		type = 1;
	} else {
		prompt = _("There are some waypoints used in routes or anchor alarms. Do you want to delete them as well? This will change the routes and disable the anchor alarms. Answering No keeps the waypoints used in routes or alarms.");
		buttons = wxYES_NO | wxCANCEL;
		type = 2;
	}
	int answer = OCPNMessageBox(this, prompt, wxString(_("OpenCPN Alert")), buttons);
	if (answer == wxID_YES)
		pWayPointMan->DeleteAllWaypoints(true);
	if (answer == wxID_NO && type == 2)
		pWayPointMan->DeleteAllWaypoints(false); // only delete unused waypoints

	if (pMarkPropDialog) {
		pMarkPropDialog->SetRoutePoint(NULL);
		pMarkPropDialog->UpdateProperties();
	}

	m_lastWptItem = -1;
	UpdateRouteListCtrl();
	UpdateWptListCtrl();
	cc1->undo->InvalidateUndo();
	cc1->Refresh();
}

void RouteManagerDialog::OnLaySelected(wxListEvent&)
{
	UpdateLayButtons();
}

void RouteManagerDialog::OnLayColumnClicked(wxListEvent& event)
{
	if (event.m_col == 1) {
		sort_layer_name_dir++;
		SortContext sorting = { m_pLayListCtrl, sort_layer_name_dir, 1 };
		m_pLayListCtrl->SortItems(SortColumnText, (long)&sorting);
	} else if (event.m_col == 2) {
		sort_layer_len_dir++;
		SortContext sorting = { m_pLayListCtrl, sort_layer_len_dir, 2 };
		m_pLayListCtrl->SortItems(SortColumnDistance, (long)&sorting);
	}
}

void RouteManagerDialog::UpdateLayButtons()
{
	long item = m_pLayListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	bool enable = (item != -1);

	btnLayDelete->Enable(enable);
	btnLayToggleChart->Enable(enable);
	btnLayToggleListing->Enable(enable);
	btnLayToggleNames->Enable(enable);

	if (item >= 0) {
		long index = m_pLayListCtrl->GetItemData(item);
		const Layer* layer = getLayerAtIndex(index);

		if (layer->IsVisibleOnChart())
			btnLayToggleChart->SetLabel(_("Hide from chart"));
		else
			btnLayToggleChart->SetLabel(_("Show on chart"));

		if (layer->HasVisibleNames())
			btnLayToggleNames->SetLabel(_("Hide WPT names"));
		else
			btnLayToggleNames->SetLabel(_("Show WPT names"));

		if (layer->IsVisibleOnListing())
			btnLayToggleListing->SetLabel(_("Unlist contents"));
		else
			btnLayToggleListing->SetLabel(_("List contents"));
	} else {
		btnLayToggleChart->SetLabel(_("Show on chart"));
		btnLayToggleNames->SetLabel(_("Show WPT names"));
		btnLayToggleListing->SetLabel(_("List contents"));
	}
}

void RouteManagerDialog::OnLayToggleVisibility(wxMouseEvent& event)
{
	wxPoint pos = event.GetPosition();
	int flags = 0;
	long clicked_index = m_pLayListCtrl->HitTest(pos, flags);

	//    Clicking Visibility column?
	if ((clicked_index > -1) && (event.GetX() < m_pLayListCtrl->GetColumnWidth(colLAYVISIBLE))) {
		// Process the clicked item
		long index = m_pLayListCtrl->GetItemData(clicked_index);
		Layer* layer = getLayerAtIndex(index);

		layer->SetVisibleOnChart(!layer->IsVisibleOnChart());
		m_pLayListCtrl->SetItemImage(clicked_index, layer->IsVisibleOnChart() ? 0 : 1);
		ToggleLayerContentsOnChart(layer);
	}

	// Allow wx to process...
	event.Skip();
}

void RouteManagerDialog::OnLayNewClick(wxCommandEvent&)
{
	bool show_flag = g_bShowLayers;
	g_bShowLayers = true;
	pConfig->UI_ImportGPX(this, true, _T(""));
	g_bShowLayers = show_flag;

	UpdateRouteListCtrl();
	UpdateTrkListCtrl();
	UpdateWptListCtrl();
	UpdateLayListCtrl();
	cc1->Refresh();
}

void RouteManagerDialog::OnLayPropertiesClick(wxCommandEvent&)
{
	// Show layer properties dialog for selected layer - todo
	long item = m_pLayListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;
}

void RouteManagerDialog::OnLayDeleteClick(wxCommandEvent&)
{
	long item = m_pLayListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	long list_index = m_pLayListCtrl->GetItemData(item);
	Layer* layer = getLayerAtIndex(list_index);

	if (!layer)
		return;

	// Process Tracks and Routes in this layer
	// FIXME: container altering iterating, iterate through copy of list, only elements are
	// interesting
	RouteList::iterator node = pRouteList->begin();
	while (node != pRouteList->end()) {
		Route* pRoute = *node;
		RouteList::iterator next = node;
		if (pRoute->m_bIsInLayer && (pRoute->m_LayerID == layer->getID())) {
			pRoute->m_bIsInLayer = false;
			pRoute->m_LayerID = 0;
			if (!pRoute->m_bIsTrack) {
				g_pRouteMan->DeleteRoute(pRoute);
			} else {
				g_pRouteMan->DeleteTrack(pRoute);
			}
		}
		node = next;
	}

	// Process waypoints in this layer
	pWayPointMan->deleteWayPointOnLayer(layer->getID());

	if (pMarkPropDialog) {
		pMarkPropDialog->SetRoutePoint(NULL);
		pMarkPropDialog->UpdateProperties();
	}

	pLayerList->remove(layer); // FIXME: there is probably a memory leak here

	UpdateRouteListCtrl();
	UpdateTrkListCtrl();
	UpdateWptListCtrl();
	UpdateLayListCtrl();

	cc1->Refresh();

	m_bNeedConfigFlush = false;
}

void RouteManagerDialog::OnLayToggleChartClick(wxCommandEvent&)
{
	// Toggle  visibility on chart for selected layer
	long item = m_pLayListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	long index = m_pLayListCtrl->GetItemData(item);
	Layer* layer = getLayerAtIndex(index);

	if (!layer)
		return;

	layer->SetVisibleOnChart(!layer->IsVisibleOnChart());
	m_pLayListCtrl->SetItemImage(item, layer->IsVisibleOnChart() ? 0 : 1);
	ToggleLayerContentsOnChart(layer);
}

void RouteManagerDialog::ToggleLayerContentsOnChart(Layer* layer)
{
	// Process Tracks and Routes in this layer
	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route* route = *i;
		if (route->m_bIsInLayer && (route->m_LayerID == layer->getID())) {
			if (!route->m_bIsTrack) {
				route->SetVisible(layer->IsVisibleOnChart());
				pConfig->UpdateRoute(route);
			} else {
				route->SetVisible(layer->IsVisibleOnChart());
			}
		}
	}

	// Process waypoints in this layer
	pWayPointMan->setWayPointVisibilityOnLayer(layer->getID(), layer->IsVisibleOnChart());

	UpdateRouteListCtrl();
	UpdateTrkListCtrl();
	UpdateWptListCtrl();
	UpdateLayListCtrl();
	UpdateLayButtons();

	cc1->Refresh();
}

void RouteManagerDialog::OnLayToggleNamesClick(wxCommandEvent&)
{
	// Toggle WPT names visibility on chart for selected layer
	long item = m_pLayListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	long index = m_pLayListCtrl->GetItemData(item);
	Layer* layer = getLayerAtIndex(index);

	if (!layer)
		return;

	layer->SetVisibleNames(!layer->HasVisibleNames());
	ToggleLayerContentsNames(layer);
}

void RouteManagerDialog::ToggleLayerContentsNames(Layer* layer)
{
	// Process Tracks and Routes in this layer
	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route* route = *i;
		if (route->m_bIsInLayer && (route->m_LayerID == layer->getID())) {
			for (RoutePointList::iterator j = route->pRoutePointList->begin();
				 j != route->pRoutePointList->end(); ++j) {
				(*j)->m_bShowName = layer->HasVisibleNames();
			}
		}
	}

	// Process waypoints in this layer
	pWayPointMan->setWayPointNameVisibilityOnLayer(layer->getID(), layer->HasVisibleNames());
	UpdateLayButtons();
	cc1->Refresh();
}

void RouteManagerDialog::OnLayToggleListingClick(wxCommandEvent&)
{
	// Toggle  visibility on listing for selected layer
	long item = m_pLayListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item == -1)
		return;

	long index = m_pLayListCtrl->GetItemData(item);
	Layer* layer = getLayerAtIndex(index);

	if (!layer)
		return;

	layer->SetVisibleOnListing(!layer->IsVisibleOnListing());
	ToggleLayerContentsOnListing(layer);
}

void RouteManagerDialog::ToggleLayerContentsOnListing(Layer* layer)
{
	::wxBeginBusyCursor();

	// Process Tracks and Routes in this layer
	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route* route = *i;
		if (route->m_bIsInLayer && (route->m_LayerID == layer->getID())) {
			// FIXME: both paths of the condition do the same, bug or obsolete?
			if (!route->m_bIsTrack) {
				route->SetListed(layer->IsVisibleOnListing());
				// pConfig->UpdateRoute(route); // FIXME: if not needed, condition is obsolete,
				// SetListed is invariant
			} else {
				route->SetListed(layer->IsVisibleOnListing());
			}
		}
	}

	// Process waypoints in this layer
	// n.b.  If the waypoint belongs to a track, and is not shared, then do not list it.
	// This is a performance optimization, allowing large track support.

	pWayPointMan->setWayPointListingVisibilityOnLayer(layer->getID(), layer->IsVisibleOnListing());

	UpdateRouteListCtrl();
	UpdateTrkListCtrl();
	UpdateWptListCtrl();
	UpdateLayListCtrl();

	::wxEndBusyCursor();

	cc1->Refresh();
}

void RouteManagerDialog::OnLayDefaultAction(wxListEvent &)
{
	wxCommandEvent evt;
	OnLayPropertiesClick(evt);
}

void RouteManagerDialog::UpdateLayListCtrl()
{
	// if an item was selected, make it selected again if it still exist
	long item = m_pLayListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	long selected_id = -1;
	if (item != -1)
		selected_id = m_pLayListCtrl->GetItemData(item);

	// Delete existing items
	m_pLayListCtrl->DeleteAllItems();

	// then add routes to the listctrl
	int index = 0;
	for (LayerList::const_iterator it = pLayerList->begin(); it != pLayerList->end();
		 ++it, ++index) {
		const Layer* lay = *it;

		wxListItem li;
		li.SetId(index);
		li.SetImage(lay->IsVisibleOnChart() ? 0 : 1);
		li.SetData(index);
		li.SetText(_T(""));

		long idx = m_pLayListCtrl->InsertItem(li);

		wxString name = lay->getName();
		if (name.IsEmpty()) {
			name = _("(Unnamed Layer)");
		}
		m_pLayListCtrl->SetItem(idx, colLAYNAME, name);
		m_pLayListCtrl->SetItem(idx, colLAYITEMS, wxString::Format(wxT("%d"), (int)lay->getNoOfItems()));
	}

	SortContext sorting = { m_pLayListCtrl, sort_layer_name_dir, 1 };
	m_pLayListCtrl->SortItems(SortColumnText, (long)&sorting);

	// restore selection if possible
	// NOTE this will select a different item, if one is deleted
	// (the next route will get that index).
	if (selected_id > -1) {
		item = m_pLayListCtrl->FindItem(-1, selected_id);
		m_pLayListCtrl->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
	UpdateLayButtons();
}

void RouteManagerDialog::OnImportClick(wxCommandEvent&)
{
	// Import routes
	// FIXME there is no way to instruct this function about what to import.
	// Suggest to add that!
	pConfig->UI_ImportGPX(this);

	UpdateRouteListCtrl();
	UpdateTrkListCtrl();
	UpdateWptListCtrl();
	UpdateLayListCtrl();

	cc1->Refresh();
}

void RouteManagerDialog::OnExportClick(wxCommandEvent&)
{
	pConfig->ExportGPX(this);
}

void RouteManagerDialog::OnExportVizClick(wxCommandEvent&)
{
	pConfig->ExportGPX(this, true, true); // only visible objects, layers included
}

