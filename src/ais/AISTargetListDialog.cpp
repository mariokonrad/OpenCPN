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

#include "AISTargetListDialog.h"
#include <OCPNListCtrl.h>
#include <Select.h>
#include <RouteManagerDialog.h>
#include <Undo.h>
#include <ChartCanvas.h>
#include <DimeControl.h>
#include <Config.h>
#include <MainFrame.h>

#include <gui/StyleManager.h>
#include <gui/Style.h>

#include <global/OCPN.h>
#include <global/GUI.h>

#include <ais/ais.h>
#include <ais/AIS_Decoder.h>
#include <ais/AIS_Target_Data.h>

#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/tokenzr.h>
#include <wx/button.h>

static ais::AIS_Decoder* s_p_sort_decoder;
extern ais::AISTargetListDialog* g_pAISTargetList;

static int s_AisTargetList_count; // FIXME

extern Config* pConfig;
extern MainFrame* gFrame;
extern ChartCanvas* cc1;
extern Select* pSelect;
extern RouteManagerDialog* pRouteManagerDialog;

namespace ais
{

IMPLEMENT_CLASS(AISTargetListDialog, wxPanel)

BEGIN_EVENT_TABLE(AISTargetListDialog, wxPanel)
	EVT_CLOSE(AISTargetListDialog::OnClose)
END_EVENT_TABLE()

static int ItemCompare(AIS_Target_Data* pAISTarget1, AIS_Target_Data* pAISTarget2)
{
	wxString s1;
	wxString s2;
	double n1 = 0.;
	double n2 = 0.;
	bool b_cmptype_num = false;

	// Don't sort if target list count is too large
	if (s_AisTargetList_count > 1000)
		return 0;

	AIS_Target_Data* t1 = pAISTarget1;
	AIS_Target_Data* t2 = pAISTarget2;

	if (t1->Class == AIS_SART) {
		if (t2->Class == AIS_DSC)
			return 0;
		else
			return -1;
	}

	if (t2->Class == AIS_SART) {
		if (t1->Class == AIS_DSC)
			return 0;
		else
			return 1;
	}

	const global::GUI::AISTargetList& ais_target_list = global::OCPN::get().gui().ais_target_list();

	switch (ais_target_list.sortColumn) {
		case tlTRK:
			n1 = t1->b_show_track;
			n2 = t2->b_show_track;
			b_cmptype_num = true;
			break;

		case tlNAME:
			s1 = trimAISField(t1->ShipName);
			if ((!t1->b_nameValid && (t1->Class == AIS_BASE)) || (t1->Class == AIS_SART))
				s1 = _T("-");

			s2 = trimAISField(t2->ShipName);
			if ((!t2->b_nameValid && (t2->Class == AIS_BASE)) || (t2->Class == AIS_SART))
				s2 = _T("-");
			break;

		case tlCALL:
			s1 = trimAISField(t1->CallSign);
			s2 = trimAISField(t2->CallSign);
			break;

		case tlMMSI:
			n1 = t1->MMSI;
			n2 = t2->MMSI;
			b_cmptype_num = true;
			break;

		case tlCLASS:
			s1 = t1->Get_class_string(true);
			s2 = t2->Get_class_string(true);
			break;

		case tlTYPE:
			s1 = t1->Get_vessel_type_string(false);
			if ((t1->Class == AIS_BASE) || (t1->Class == AIS_SART))
				s1 = _T("-");

			s2 = t2->Get_vessel_type_string(false);
			if ((t1->Class == AIS_BASE) || (t1->Class == AIS_SART))
				s2 = _T("-");
			break;

		case tlNAVSTATUS: {
			if ((t1->NavStatus <= 15) && (t1->NavStatus >= 0)) {
				if (t1->Class == AIS_SART) {
					if (t1->NavStatus == RESERVED_14)
						s1 = _("Active");
					else if (t1->NavStatus == UNDEFINED)
						s1 = _("Testing");
				} else
					s1 = ais_get_status(t1->NavStatus);
			} else
				s1 = _("-");

			if ((t1->Class == AIS_ATON) || (t1->Class == AIS_BASE) || (t1->Class == AIS_CLASS_B))
				s1 = _T("-");

			if ((t2->NavStatus <= 15) && (t2->NavStatus >= 0)) {
				if (t2->Class == AIS_SART) {
					if (t2->NavStatus == RESERVED_14)
						s2 = _("Active");
					else if (t2->NavStatus == UNDEFINED)
						s2 = _("Testing");
				} else
					s2 = ais_get_status(t2->NavStatus);
			} else
				s2 = _("-");

			if ((t2->Class == AIS_ATON) || (t2->Class == AIS_BASE) || (t2->Class == AIS_CLASS_B))
				s2 = _T("-");

			break;
		}

		case tlBRG: {
			int brg1 = wxRound(t1->Brg);
			if (brg1 == 360)
				n1 = 0.0;
			else
				n1 = brg1;

			int brg2 = wxRound(t2->Brg);
			if (brg2 == 360)
				n2 = 0.0;
			else
				n2 = brg2;

			b_cmptype_num = true;
			break;
		}

		case tlCOG: {
			if ((t1->COG >= 360.0) || (t1->Class == AIS_ATON) || (t1->Class == AIS_BASE))
				n1 = -1.0;
			else {
				int crs = wxRound(t1->COG);
				if (crs == 360)
					n1 = 0.;
				else
					n1 = crs;
			}

			if ((t2->COG >= 360.0) || (t2->Class == AIS_ATON) || (t2->Class == AIS_BASE))
				n2 = -1.0;
			else {
				int crs = wxRound(t2->COG);
				if (crs == 360)
					n2 = 0.;
				else
					n2 = crs;
			}

			b_cmptype_num = true;
			break;
		}

		case tlSOG: {
			if ((t1->SOG > 100.) || (t1->Class == AIS_ATON) || (t1->Class == AIS_BASE))
				n1 = -1.0;
			else
				n1 = t1->SOG;

			if ((t2->SOG > 100.) || (t2->Class == AIS_ATON) || (t2->Class == AIS_BASE))
				n2 = -1.0;
			else
				n2 = t2->SOG;

			b_cmptype_num = true;
			break;
		}

		case tlCPA: {
			if ((!t1->bCPA_Valid) || (t1->Class == AIS_ATON) || (t1->Class == AIS_BASE))
				n1 = 99999.0;
			else
				n1 = t1->CPA;

			if ((!t2->bCPA_Valid) || (t2->Class == AIS_ATON) || (t2->Class == AIS_BASE))
				n2 = 99999.0;
			else
				n2 = t2->CPA;

			b_cmptype_num = true;
			break;
		}

		case tlTCPA: {
			if ((!t1->bCPA_Valid) || (t1->Class == AIS_ATON) || (t1->Class == AIS_BASE))
				n1 = 99999.0;
			else
				n1 = t1->TCPA;

			if ((!t2->bCPA_Valid) || (t2->Class == AIS_ATON) || (t2->Class == AIS_BASE))
				n2 = 99999.0;
			else
				n2 = t2->TCPA;

			b_cmptype_num = true;
			break;
		}

		case tlRNG: {
			n1 = t1->Range_NM;
			n2 = t2->Range_NM;
			b_cmptype_num = true;
			break;
		}

		default:
			break;
	}

	if (!b_cmptype_num) {
		if (ais_target_list.sortReverse)
			return s2.Cmp(s1);
		return s1.Cmp(s2);
	} else {
		// If numeric sort values are equal, secondary sort is on Range_NM
		if (ais_target_list.sortReverse) {
			if (n2 > n1)
				return 1;
			else if (n2 < n1)
				return -1;
			else
				return (t1->Range_NM > t2->Range_NM); // 0;
		} else {
			if (n2 > n1)
				return -1;
			else if (n2 < n1)
				return 1;
			else
				return (t1->Range_NM > t2->Range_NM); // 0;
		}
	}
}

static int ArrayItemCompareMMSI(int MMSI1, int MMSI2)
{
	if (s_p_sort_decoder) {
		AIS_Target_Data* pAISTarget1 = s_p_sort_decoder->Get_Target_Data_From_MMSI(MMSI1);
		AIS_Target_Data* pAISTarget2 = s_p_sort_decoder->Get_Target_Data_From_MMSI(MMSI2);

		if (pAISTarget1 && pAISTarget2)
			return ItemCompare(pAISTarget1, pAISTarget2);
		else
			return 0;
	} else
		return 0;
}

AISTargetListDialog::AISTargetListDialog(wxWindow* parent, wxAuiManager* auimgr,
										 AIS_Decoder* pdecoder)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(780, 250), wxBORDER_NONE)
{
	const global::GUI::AISTargetList& ais_target_list = global::OCPN::get().gui().ais_target_list();

	m_pparent = parent;
	m_pAuiManager = auimgr;
	m_pdecoder = pdecoder;

	wxFont* qFont = GetOCPNScaledFont(_("Dialog"), 10);
	SetFont(*qFont);

	// Make an estimate of the default dialog size
	// for the case when the AUI Perspective for this dialog is undefined
	wxSize esize;
	esize.x = 700;
	esize.y = GetCharHeight() * 18;
	SetSize(esize);

	s_p_sort_decoder = pdecoder;
	m_pMMSI_array = new ArrayOfMMSI(ArrayItemCompareMMSI);

	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(topSizer);

	// Parse the global column width string as read from config file
	wxStringTokenizer tkz(ais_target_list.column_spec, _T(";"));
	wxString s_width = tkz.GetNextToken();
	int width;
	long lwidth;

	m_pListCtrlAISTargets = new OCPNListCtrl(
		this, ID_AIS_TARGET_LIST, wxDefaultPosition, wxDefaultSize,
		wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxBORDER_SUNKEN | wxLC_VIRTUAL);
	wxImageList* imglist = new wxImageList(16, 16, true, 2);

	gui::Style& style = global::OCPN::get().styleman().current();
	imglist->Add(style.GetIcon(_T("sort_asc")));
	imglist->Add(style.GetIcon(_T("sort_desc")));

	m_pListCtrlAISTargets->AssignImageList(imglist, wxIMAGE_LIST_SMALL);
	m_pListCtrlAISTargets->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
								   wxListEventHandler(AISTargetListDialog::OnTargetSelected), NULL,
								   this);
	m_pListCtrlAISTargets->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED,
								   wxListEventHandler(AISTargetListDialog::OnTargetSelected), NULL,
								   this);
	m_pListCtrlAISTargets->Connect(wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
								   wxListEventHandler(AISTargetListDialog::OnTargetDefaultAction),
								   NULL, this);
	m_pListCtrlAISTargets->Connect(
		wxEVT_COMMAND_LIST_COL_CLICK,
		wxListEventHandler(AISTargetListDialog::OnTargetListColumnClicked), NULL, this);

	int dx = GetCharWidth();

	width = dx * 4;
	if (s_width.ToLong(&lwidth)) {
		width = wxMax(dx * 2, lwidth);
		width = wxMin(width, dx * 30);
	}
	m_pListCtrlAISTargets->InsertColumn(tlTRK, _("Trk"), wxLIST_FORMAT_LEFT, width);
	s_width = tkz.GetNextToken();

	width = dx * 12;
	if (s_width.ToLong(&lwidth)) {
		width = wxMax(dx * 2, lwidth);
		width = wxMin(width, dx * 30);
	}
	m_pListCtrlAISTargets->InsertColumn(tlNAME, _("Name"), wxLIST_FORMAT_LEFT, width);
	s_width = tkz.GetNextToken();

	width = dx * 7;
	if (s_width.ToLong(&lwidth)) {
		width = wxMax(dx * 2, lwidth);
		width = wxMin(width, dx * 30);
	}
	m_pListCtrlAISTargets->InsertColumn(tlCALL, _("Call"), wxLIST_FORMAT_LEFT, width);
	s_width = tkz.GetNextToken();

	width = dx * 10;
	if (s_width.ToLong(&lwidth)) {
		width = wxMax(dx * 2, lwidth);
		width = wxMin(width, dx * 30);
	}
	m_pListCtrlAISTargets->InsertColumn(tlMMSI, _("MMSI"), wxLIST_FORMAT_LEFT, width);
	s_width = tkz.GetNextToken();

	width = dx * 7;
	if (s_width.ToLong(&lwidth)) {
		width = wxMax(dx * 2, lwidth);
		width = wxMin(width, dx * 30);
	}
	m_pListCtrlAISTargets->InsertColumn(tlCLASS, _("Class"), wxLIST_FORMAT_CENTER, width);
	s_width = tkz.GetNextToken();

	width = dx * 10;
	if (s_width.ToLong(&lwidth)) {
		width = wxMax(dx * 2, lwidth);
		width = wxMin(width, dx * 30);
	}
	m_pListCtrlAISTargets->InsertColumn(tlTYPE, _("Type"), wxLIST_FORMAT_LEFT, width);
	s_width = tkz.GetNextToken();

	width = dx * 12;
	if (s_width.ToLong(&lwidth)) {
		width = wxMax(dx * 2, lwidth);
		width = wxMin(width, dx * 30);
	}
	m_pListCtrlAISTargets->InsertColumn(tlNAVSTATUS, _("Nav Status"), wxLIST_FORMAT_LEFT, width);
	s_width = tkz.GetNextToken();

	width = dx * 6;
	if (s_width.ToLong(&lwidth)) {
		width = wxMax(dx * 2, lwidth);
		width = wxMin(width, dx * 30);
	}
	m_pListCtrlAISTargets->InsertColumn(tlBRG, _("Brg"), wxLIST_FORMAT_RIGHT, width);
	s_width = tkz.GetNextToken();

	width = dx * 8;
	if (s_width.ToLong(&lwidth)) {
		width = wxMax(dx * 2, lwidth);
		width = wxMin(width, dx * 30);
	}
	m_pListCtrlAISTargets->InsertColumn(tlRNG, _("Range"), wxLIST_FORMAT_RIGHT, width);
	s_width = tkz.GetNextToken();

	width = dx * 6;
	if (s_width.ToLong(&lwidth)) {
		width = wxMax(dx * 2, lwidth);
		width = wxMin(width, dx * 30);
	}
	m_pListCtrlAISTargets->InsertColumn(tlCOG, _("CoG"), wxLIST_FORMAT_RIGHT, width);
	s_width = tkz.GetNextToken();

	width = dx * 6;
	if (s_width.ToLong(&lwidth)) {
		width = wxMax(dx * 2, lwidth);
		width = wxMin(width, dx * 30);
	}
	m_pListCtrlAISTargets->InsertColumn(tlSOG, _("SoG"), wxLIST_FORMAT_RIGHT, width);

	width = dx * 7;
	if (s_width.ToLong(&lwidth)) {
		width = wxMax(dx * 2, lwidth);
		width = wxMin(width, dx * 30);
	}
	m_pListCtrlAISTargets->InsertColumn(tlCPA, _("CPA"), wxLIST_FORMAT_RIGHT, width);

	width = dx * 8;
	if (s_width.ToLong(&lwidth)) {
		width = wxMax(dx * 2, lwidth);
		width = wxMin(width, dx * 30);
	}
	m_pListCtrlAISTargets->InsertColumn(tlTCPA, _("TCPA"), wxLIST_FORMAT_RIGHT, width);
	wxListItem item;
	item.SetMask(wxLIST_MASK_IMAGE);
	item.SetImage(ais_target_list.sortReverse ? 1 : 0);
	global::OCPN::get().gui().set_ais_target_list_sortColumn(wxMax(ais_target_list.sortColumn, 0));
	m_pListCtrlAISTargets->SetColumn(ais_target_list.sortColumn, item);

	topSizer->Add(m_pListCtrlAISTargets, 1, wxEXPAND | wxALL, 0);

	wxBoxSizer* boxSizer02 = new wxBoxSizer(wxVERTICAL);
	boxSizer02->AddSpacer(22);

	m_pButtonInfo = new wxButton(this, wxID_ANY, _("Target info"), wxDefaultPosition, wxDefaultSize,
								 wxBU_AUTODRAW);
	m_pButtonInfo->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						   wxCommandEventHandler(AISTargetListDialog::OnTargetQuery), NULL, this);
	boxSizer02->Add(m_pButtonInfo, 0, wxEXPAND | wxALL, 0);
	boxSizer02->AddSpacer(5);

	m_pButtonJumpTo = new wxButton(this, wxID_ANY, _("Center View"), wxDefaultPosition,
								   wxDefaultSize, wxBU_AUTODRAW);
	m_pButtonJumpTo->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							 wxCommandEventHandler(AISTargetListDialog::OnTargetScrollTo), NULL,
							 this);
	boxSizer02->Add(m_pButtonJumpTo, 0, wxEXPAND | wxALL, 0);

	m_pButtonCreateWpt = new wxButton(this, wxID_ANY, _("Create WPT"), wxDefaultPosition,
									  wxDefaultSize, wxBU_AUTODRAW);
	m_pButtonCreateWpt->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
								wxCommandEventHandler(AISTargetListDialog::OnTargetCreateWpt), NULL,
								this);
	boxSizer02->Add(m_pButtonCreateWpt, 0, wxEXPAND | wxALL, 0);

	m_pButtonHideAllTracks = new wxButton(this, wxID_ANY, _("Hide All Tracks"), wxDefaultPosition,
										  wxDefaultSize, wxBU_AUTODRAW);
	m_pButtonHideAllTracks->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
									wxCommandEventHandler(AISTargetListDialog::OnHideAllTracks),
									NULL, this);
	boxSizer02->Add(m_pButtonHideAllTracks, 0, wxEXPAND | wxALL, 0);

	m_pButtonShowAllTracks = new wxButton(this, wxID_ANY, _("Show All Tracks"), wxDefaultPosition,
										  wxDefaultSize, wxBU_AUTODRAW);
	m_pButtonShowAllTracks->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
									wxCommandEventHandler(AISTargetListDialog::OnShowAllTracks),
									NULL, this);
	boxSizer02->Add(m_pButtonShowAllTracks, 0, wxEXPAND | wxALL, 0);

	m_pButtonToggleTrack = new wxButton(this, wxID_ANY, _("Toggle track"), wxDefaultPosition,
										wxDefaultSize, wxBU_AUTODRAW);
	m_pButtonToggleTrack->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
								  wxCommandEventHandler(AISTargetListDialog::OnToggleTrack), NULL,
								  this);
	boxSizer02->Add(m_pButtonToggleTrack, 0, wxEXPAND | wxALL, 0);

	boxSizer02->AddSpacer(10);

	m_pStaticTextRange = new wxStaticText(this, wxID_ANY, _("Limit range: NM"), wxDefaultPosition,
										  wxDefaultSize, 0);
	boxSizer02->Add(m_pStaticTextRange, 0, wxALL, 0);
	boxSizer02->AddSpacer(2);
	m_pSpinCtrlRange
		= new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, -1),
						 wxSP_ARROW_KEYS, 1, 20000, ais_target_list.range);
	m_pSpinCtrlRange->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED,
							  wxCommandEventHandler(AISTargetListDialog::OnLimitRange), NULL, this);
	m_pSpinCtrlRange->Connect(wxEVT_COMMAND_TEXT_UPDATED,
							  wxCommandEventHandler(AISTargetListDialog::OnLimitRange), NULL, this);
	boxSizer02->Add(m_pSpinCtrlRange, 0, wxEXPAND | wxALL, 0);
	topSizer->Add(boxSizer02, 0, wxEXPAND | wxALL, 2);

	boxSizer02->AddSpacer(10);
	m_pStaticTextCount
		= new wxStaticText(this, wxID_ANY, _("Target Count"), wxDefaultPosition, wxDefaultSize, 0);
	boxSizer02->Add(m_pStaticTextCount, 0, wxALL, 0);

	boxSizer02->AddSpacer(2);
	m_pTextTargetCount
		= new wxTextCtrl(this, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	boxSizer02->Add(m_pTextTargetCount, 0, wxALL, 0);

	boxSizer02->AddSpacer(10);
	m_pButtonOK
		= new wxButton(this, wxID_ANY, _("Close"), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	m_pButtonOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						 wxCommandEventHandler(AISTargetListDialog::OnCloseButton), NULL, this);
	boxSizer02->Add(m_pButtonOK, 0, wxEXPAND | wxALL, 0);

	topSizer->Layout();

	// This is silly, but seems to be required for __WXMSW__ build
	// If not done, the SECOND invocation of AISTargetList fails to expand the list to the full
	// wxSizer size....
	SetSize(GetSize().x, GetSize().y - 1);

	SetColorScheme();
	UpdateButtons();

	if (m_pAuiManager) {
		wxAuiPaneInfo pane = wxAuiPaneInfo()
								 .Name(_T("AISTargetList"))
								 .CaptionVisible(true)
								 .DestroyOnClose(true)
								 .Float()
								 .FloatingPosition(50, 50)
								 .TopDockable(false)
								 .BottomDockable(true)
								 .LeftDockable(false)
								 .RightDockable(false)
								 .Show(true);
		m_pAuiManager->LoadPaneInfo(ais_target_list.perspective, pane);

		bool b_reset_pos = false;

		if ((pane.floating_size.x != -1) && (pane.floating_size.y != -1)) {
#ifdef __WXMSW__
		// Support MultiMonitor setups which an allow negative window positions.
		// If the requested window title bar does not intersect any installed monitor,
		// then default to simple primary monitor positioning.
		RECT frame_title_rect;
		frame_title_rect.left = pane.floating_pos.x;
		frame_title_rect.top = pane.floating_pos.y;
		frame_title_rect.right = pane.floating_pos.x + pane.floating_size.x;
		frame_title_rect.bottom = pane.floating_pos.y + 30;

		if (NULL == MonitorFromRect(&frame_title_rect, MONITOR_DEFAULTTONULL))
			b_reset_pos = true;
#else

		// Make sure drag bar (title bar) of window intersects wxClient Area of screen, with a
		// little slop...
		wxRect window_title_rect; // conservative estimate
		window_title_rect.x = pane.floating_pos.x;
		window_title_rect.y = pane.floating_pos.y;
		window_title_rect.width = pane.floating_size.x;
		window_title_rect.height = 30;

		wxRect ClientRect = wxGetClientDisplayRect();
		ClientRect.Deflate(60, 60); // Prevent the new window from being too close to the edge
		if (!ClientRect.Intersects(window_title_rect))
			b_reset_pos = true;

#endif

		if (b_reset_pos)
			pane.FloatingPosition(50, 50);
		}

		// If the list got accidentally dropped on top of the chart bar, move it away....
		if (pane.IsDocked() && (pane.dock_row == 0)) {
			pane.Float();
			pane.Row(1);
			pane.Position(0);

			global::OCPN::get().gui().set_ais_target_list_perspective(
				m_pAuiManager->SavePaneInfo(pane));
			pConfig->UpdateSettings();
		}

		pane.Caption(wxGetTranslation(_("AIS target list")));
		m_pAuiManager->AddPane(this, pane);
		m_pAuiManager->Update();

		m_pAuiManager->Connect(wxEVT_AUI_PANE_CLOSE,
							   wxAuiManagerEventHandler(AISTargetListDialog::OnPaneClose), NULL,
							   this);
	}
}

AISTargetListDialog::~AISTargetListDialog()
{
	Disconnect_decoder();
	g_pAISTargetList = NULL;
}

void AISTargetListDialog::OnClose(wxCloseEvent&)
{
	Disconnect_decoder();
}

void AISTargetListDialog::Disconnect_decoder()
{
	m_pdecoder = NULL;
}

void AISTargetListDialog::SetColorScheme()
{
	DimeControl(this);
}

void AISTargetListDialog::OnPaneClose(wxAuiManagerEvent& event)
{
	if (event.pane->name == _T("AISTargetList")) {
		global::OCPN::get().gui().set_ais_target_list_perspective(
			m_pAuiManager->SavePaneInfo(*event.pane));
	}
	event.Skip();
}

void AISTargetListDialog::OnCloseButton(wxCommandEvent&)
{
	if (m_pAuiManager) {
		wxAuiPaneInfo pane = m_pAuiManager->GetPane(_T("AISTargetList"));
		global::OCPN::get().gui().set_ais_target_list_perspective(
			m_pAuiManager->SavePaneInfo(pane));
		m_pAuiManager->DetachPane(this);
		Destroy();
	}
}

void AISTargetListDialog::UpdateButtons()
{
	long item = m_pListCtrlAISTargets->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	bool enable = (item != -1);

	m_pButtonInfo->Enable(enable);

	if (m_pdecoder && item != -1) {
		AIS_Target_Data* pAISTargetSel
			= m_pdecoder->Get_Target_Data_From_MMSI(m_pMMSI_array->Item(item));
		if (pAISTargetSel && (!pAISTargetSel->b_positionOnceValid))
			enable = false;
	}
	m_pButtonJumpTo->Enable(enable);
	m_pButtonCreateWpt->Enable(enable);
	m_pButtonToggleTrack->Enable(enable);
}

void AISTargetListDialog::OnTargetSelected(wxListEvent&)
{
	UpdateButtons();
}

void AISTargetListDialog::DoTargetQuery(int mmsi)
{
	ShowAISTargetQueryDialog(m_pparent, mmsi);
}

// When an item is activated in AIS TArget List then opens the AIS Target Query Dialog
void AISTargetListDialog::OnTargetDefaultAction(wxListEvent& event)
{
	long mmsi_no;
	if ((mmsi_no = event.GetData()))
		DoTargetQuery(mmsi_no);
}

void AISTargetListDialog::OnTargetQuery(wxCommandEvent&)
{
	long selItemID = m_pListCtrlAISTargets->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (selItemID == -1)
		return;

	if (m_pdecoder) {
		AIS_Target_Data* pAISTarget
			= m_pdecoder->Get_Target_Data_From_MMSI(m_pMMSI_array->Item(selItemID));
		if (pAISTarget)
			DoTargetQuery(pAISTarget->MMSI);
	}
}

void AISTargetListDialog::OnTargetListColumnClicked(wxListEvent& event)
{
	global::GUI& gui = global::OCPN::get().gui();
	const global::GUI::AISTargetList& ais_target_list = gui.ais_target_list();

	int key = event.GetColumn();
	wxListItem item;
	item.SetMask(wxLIST_MASK_IMAGE);
	if (key == ais_target_list.sortColumn)
		gui.set_ais_target_list_sortReverse(!ais_target_list.sortReverse);
	else {
		item.SetImage(-1);
		m_pListCtrlAISTargets->SetColumn(ais_target_list.sortColumn, item);
		gui.set_ais_target_list_sortReverse(false);
		gui.set_ais_target_list_sortColumn(key);
	}
	item.SetImage(ais_target_list.sortReverse ? 1 : 0);
	if (ais_target_list.sortColumn >= 0) {
		m_pListCtrlAISTargets->SetColumn(ais_target_list.sortColumn, item);
		UpdateAISTargetList();
	}
}

void AISTargetListDialog::OnTargetScrollTo(wxCommandEvent&)
{
	long selItemID = -1;
	selItemID
		= m_pListCtrlAISTargets->GetNextItem(selItemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (selItemID == -1)
		return;

	AIS_Target_Data* pAISTarget = NULL;
	if (m_pdecoder)
		pAISTarget = m_pdecoder->Get_Target_Data_From_MMSI(m_pMMSI_array->Item(selItemID));

	if (pAISTarget)
		gFrame->JumpToPosition(geo::Position(pAISTarget->Lat, pAISTarget->Lon), cc1->GetVPScale());
}

void AISTargetListDialog::OnTargetCreateWpt(wxCommandEvent&)
{
	long selItemID = -1;
	selItemID
		= m_pListCtrlAISTargets->GetNextItem(selItemID, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (selItemID == -1)
		return;

	AIS_Target_Data* pAISTarget = NULL;
	if (m_pdecoder)
		pAISTarget = m_pdecoder->Get_Target_Data_From_MMSI(m_pMMSI_array->Item(selItemID));

	if (pAISTarget) {
		geo::Position pos(pAISTarget->Lat, pAISTarget->Lon);
		RoutePoint* pWP
			= new RoutePoint(pos, global::OCPN::get().gui().view().default_wp_icon, wxEmptyString);
		pWP->m_bIsolatedMark = true; // This is an isolated mark
		pSelect->AddSelectableRoutePoint(pos, pWP);
		pConfig->AddNewWayPoint(pWP, -1); // use auto next num

		if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
			pRouteManagerDialog->UpdateWptListCtrl();
		cc1->get_undo().BeforeUndoableAction(UndoAction::Undo_CreateWaypoint, pWP,
											 UndoAction::Undo_HasParent, NULL);
		cc1->get_undo().AfterUndoableAction(NULL);
		Refresh(false);
	}
}

void AISTargetListDialog::OnShowAllTracks(wxCommandEvent& WXUNUSED(event))
{
	AIS_Target_Hash::iterator it;
	AIS_Target_Hash* current_targets = m_pdecoder->GetTargetList();
	for (it = (*current_targets).begin(); it != (*current_targets).end(); ++it) {
		AIS_Target_Data* pAISTarget = it->second;
		if (NULL != pAISTarget) {
			pAISTarget->b_show_track = true;
		}
	}
	UpdateAISTargetList();
}

void AISTargetListDialog::OnHideAllTracks(wxCommandEvent& WXUNUSED(event))
{
	AIS_Target_Hash::iterator it;
	AIS_Target_Hash* current_targets = m_pdecoder->GetTargetList();
	for (it = (*current_targets).begin(); it != (*current_targets).end(); ++it) {
		AIS_Target_Data* pAISTarget = it->second;
		if (NULL != pAISTarget) {
			pAISTarget->b_show_track = false;
		}
	}
	UpdateAISTargetList();
}

void AISTargetListDialog::OnToggleTrack(wxCommandEvent& WXUNUSED(event))
{
	long selItemID = m_pListCtrlAISTargets->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (selItemID == -1)
		return;

	AIS_Target_Data* pAISTarget = NULL;
	if (m_pdecoder)
		pAISTarget = m_pdecoder->Get_Target_Data_From_MMSI(m_pMMSI_array->Item(selItemID));

	if (pAISTarget) {
		pAISTarget->b_show_track = !pAISTarget->b_show_track;
		UpdateAISTargetList();
	}
}

void AISTargetListDialog::OnLimitRange(wxCommandEvent&)
{
	global::OCPN::get().gui().set_ais_target_list_range(m_pSpinCtrlRange->GetValue());
	UpdateAISTargetList();
}

AIS_Target_Data* AISTargetListDialog::GetpTarget(unsigned int list_item)
{
	return m_pdecoder->Get_Target_Data_From_MMSI(m_pMMSI_array->Item(list_item));
}

void AISTargetListDialog::UpdateAISTargetList(void)
{
	if (!m_pdecoder)
		return;

	int sb_position = m_pListCtrlAISTargets->GetScrollPos(wxVERTICAL);

	// Capture the MMSI of the curently selected list item
	long selItemID = m_pListCtrlAISTargets->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	int selMMSI = -1;
	if (selItemID != -1)
		selMMSI = m_pMMSI_array->Item(selItemID);

	AIS_Target_Hash::iterator it;
	AIS_Target_Hash* current_targets = m_pdecoder->GetTargetList();
	wxListItem item;

	int index = 0;
	m_pMMSI_array->Clear();

	const global::GUI::AISTargetList& ais_target_list = global::OCPN::get().gui().ais_target_list();

	for (it = current_targets->begin(); it != current_targets->end(); ++it, ++index) {
		AIS_Target_Data* pAISTarget = it->second;
		item.SetId(index);

		if (NULL != pAISTarget) {
			if ((pAISTarget->b_positionOnceValid)
				&& (pAISTarget->Range_NM <= ais_target_list.range))
				m_pMMSI_array->Add(pAISTarget->MMSI);
			else if (!pAISTarget->b_positionOnceValid)
				m_pMMSI_array->Add(pAISTarget->MMSI);
		}
	}

	m_pListCtrlAISTargets->SetItemCount(m_pMMSI_array->size());

	s_AisTargetList_count = m_pMMSI_array->size();

	m_pListCtrlAISTargets->SetScrollPos(wxVERTICAL, sb_position, false);

	// Restore selected item
	long item_sel = 0;
	if ((selItemID != -1) && (selMMSI != -1)) {
		for (unsigned int i = 0; i < m_pMMSI_array->size(); i++) {
			if (m_pMMSI_array->Item(i) == selMMSI) {
				item_sel = i;
				break;
			}
		}
	}

	if (m_pMMSI_array->size())
		m_pListCtrlAISTargets->SetItemState(item_sel, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
											wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
	else
		m_pListCtrlAISTargets->DeleteAllItems();

	wxString count;
	count.Printf(_T("%d"), m_pMMSI_array->size());
	m_pTextTargetCount->ChangeValue(count);

#ifdef __WXMSW__
	m_pListCtrlAISTargets->Refresh(false);
#endif
}

}

