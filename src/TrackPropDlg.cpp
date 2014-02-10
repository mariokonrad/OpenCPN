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

#include "TrackPropDlg.h"

#include <OCPNTrackListCtrl.h>
#include <Track.h>
#include <Routeman.h>
#include <Select.h>
#include <RouteManagerDialog.h>
#include <RoutePrintSelection.h>
#include <LinkPropDlg.h>
#include <ChartCanvas.h>
#include <MainFrame.h>
#include <Config.h>
#include <Units.h>

#include <global/OCPN.h>

#include <navigation/RouteTracker.h>

#include <geo/GeoRef.h>

#include <plugin/PlugInManager.h>

#include <gpx/gpx.h>

extern RouteList* pRouteList;
extern Routeman* g_pRouteMan;
extern Select* pSelect;
extern RouteManagerDialog* pRouteManagerDialog;
extern Config* pConfig;
extern MainFrame* gFrame;
extern ChartCanvas* cc1;
extern PlugInManager* g_pi_manager;

TrackPropDlg::TrackPropDlg(
		wxWindow* parent,
		wxWindowID id,
		const wxString& title,
		const wxPoint& pos,
		const wxSize& size,
		long style)
	: wxDialog(parent, id, title, pos, size, style)
{
	long wstyle = style;
#ifdef __WXOSX__
	wstyle |= wxSTAY_ON_TOP;
#endif

	SetWindowStyleFlag(wstyle);
	this->SetSizeHints(wxSize(670, 440), wxDefaultSize);

	wxBoxSizer* bSizerMain;
	bSizerMain = new wxBoxSizer(wxVERTICAL);

	m_notebook1 = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
	m_panelBasic
		= new wxPanel(m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizerBasic;
	bSizerBasic = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* bSizerName;
	bSizerName = new wxBoxSizer(wxHORIZONTAL);

	m_stName
		= new wxStaticText(m_panelBasic, wxID_ANY, _("Name"), wxDefaultPosition, wxDefaultSize, 0);
	m_stName->Wrap(-1);
	bSizerName->Add(m_stName, 0, wxALL, 5);

	m_tName = new wxTextCtrl(m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition,
							 wxDefaultSize, 0);
	bSizerName->Add(m_tName, 1, 0, 5);

	bSizerBasic->Add(bSizerName, 0, wxALL | wxEXPAND, 5);

	wxBoxSizer* bSizerFromTo;
	bSizerFromTo = new wxBoxSizer(wxHORIZONTAL);

	m_stFrom
		= new wxStaticText(m_panelBasic, wxID_ANY, _("From"), wxDefaultPosition, wxDefaultSize, 0);
	m_stFrom->Wrap(-1);
	bSizerFromTo->Add(m_stFrom, 0, wxALL, 5);

	m_tFrom = new wxTextCtrl(m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition,
							 wxDefaultSize, 0);
	bSizerFromTo->Add(m_tFrom, 1, 0, 5);

	m_stTo = new wxStaticText(m_panelBasic, wxID_ANY, _("To"), wxDefaultPosition, wxDefaultSize, 0);
	m_stTo->Wrap(-1);
	bSizerFromTo->Add(m_stTo, 0, wxALL, 5);

	m_tTo = new wxTextCtrl(m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
						   0);
	bSizerFromTo->Add(m_tTo, 1, 0, 5);

	bSizerBasic->Add(bSizerFromTo, 0, wxALL | wxEXPAND, 5);

	wxStaticBoxSizer* sbSizerParams;
	sbSizerParams = new wxStaticBoxSizer(
		new wxStaticBox(m_panelBasic, wxID_ANY, _("Display parameters")), wxHORIZONTAL);

	m_cbShow = new wxCheckBox(m_panelBasic, wxID_ANY, _("Show on chart"), wxDefaultPosition,
							  wxDefaultSize, 0);
	sbSizerParams->Add(m_cbShow, 0, wxALL, 5);

	m_stColor
		= new wxStaticText(m_panelBasic, wxID_ANY, _("Color"), wxDefaultPosition, wxDefaultSize, 0);
	m_stColor->Wrap(-1);
	sbSizerParams->Add(m_stColor, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxString m_cColorChoices[]
		= { _("Default color"), _("Black"),		_("Dark Red"),	 _("Dark Green"),
			_("Dark Yellow"),   _("Dark Blue"), _("Dark Magenta"), _("Dark Cyan"),
			_("Light Gray"),	_("Dark Gray"), _("Red"),		   _("Green"),
			_("Yellow"),		_("Blue"),		_("Magenta"),	  _("Cyan"),
			_("White") };
	int m_cColorNChoices = sizeof(m_cColorChoices) / sizeof(wxString);
	m_cColor = new wxChoice(m_panelBasic, wxID_ANY, wxDefaultPosition, wxDefaultSize,
							m_cColorNChoices, m_cColorChoices, 0);
	m_cColor->SetSelection(0);
	sbSizerParams->Add(m_cColor, 1, 0, 5);

	m_stStyle
		= new wxStaticText(m_panelBasic, wxID_ANY, _("Style"), wxDefaultPosition, wxDefaultSize, 0);
	m_stStyle->Wrap(-1);
	sbSizerParams->Add(m_stStyle, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxString m_cStyleChoices[]
		= { _("Default"), _("Solid"), _("Dot"), _("Long dash"), _("Short dash"), _("Dot dash") };
	int m_cStyleNChoices = sizeof(m_cStyleChoices) / sizeof(wxString);
	m_cStyle = new wxChoice(m_panelBasic, wxID_ANY, wxDefaultPosition, wxDefaultSize,
							m_cStyleNChoices, m_cStyleChoices, 0);
	m_cStyle->SetSelection(0);
	sbSizerParams->Add(m_cStyle, 1, 0, 5);

	m_stWidth
		= new wxStaticText(m_panelBasic, wxID_ANY, _("Width"), wxDefaultPosition, wxDefaultSize, 0);
	m_stWidth->Wrap(-1);
	sbSizerParams->Add(m_stWidth, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxString m_cWidthChoices[] = { _("Default"),  _("1 pixel"),  _("2 pixels"), _("3 pixels"),
								   _("4 pixels"), _("5 pixels"), _("6 pixels"), _("7 pixels"),
								   _("8 pixels"), _("9 pixels"), _("10 pixels") };
	int m_cWidthNChoices = sizeof(m_cWidthChoices) / sizeof(wxString);
	m_cWidth = new wxChoice(m_panelBasic, wxID_ANY, wxDefaultPosition, wxDefaultSize,
							m_cWidthNChoices, m_cWidthChoices, 0);
	m_cWidth->SetSelection(0);
	sbSizerParams->Add(m_cWidth, 1, 0, 5);

	bSizerBasic->Add(sbSizerParams, 0, wxALL | wxEXPAND, 5);

	wxStaticBoxSizer* sbSizerStats;
	sbSizerStats = new wxStaticBoxSizer(new wxStaticBox(m_panelBasic, wxID_ANY, _("Statistics")),
										wxVERTICAL);

	wxBoxSizer* bSizerStats;
	bSizerStats = new wxBoxSizer(wxHORIZONTAL);

	m_stTotDistance = new wxStaticText(m_panelBasic, wxID_ANY, _("Total distance"),
									   wxDefaultPosition, wxDefaultSize, 0);
	m_stTotDistance->Wrap(-1);
	bSizerStats->Add(m_stTotDistance, 0, wxALL, 5);

	m_tTotDistance = new wxTextCtrl(m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition,
									wxDefaultSize, wxTE_READONLY);
	bSizerStats->Add(m_tTotDistance, 1, 0, 5);

	m_stAvgSpeed = new wxStaticText(m_panelBasic, wxID_ANY, _("Avg. speed"), wxDefaultPosition,
									wxDefaultSize, 0);
	m_stAvgSpeed->Wrap(-1);
	bSizerStats->Add(m_stAvgSpeed, 0, wxALL, 5);

	m_tAvgSpeed = new wxTextCtrl(m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition,
								 wxDefaultSize, wxTE_READONLY);
	bSizerStats->Add(m_tAvgSpeed, 1, 0, 5);

	m_stTimeEnroute = new wxStaticText(m_panelBasic, wxID_ANY, _("Time enroute"), wxDefaultPosition,
									   wxDefaultSize, 0);
	m_stTimeEnroute->Wrap(-1);
	bSizerStats->Add(m_stTimeEnroute, 0, wxALL, 5);

	m_tTimeEnroute = new wxTextCtrl(m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition,
									wxDefaultSize, wxTE_READONLY);
	bSizerStats->Add(m_tTimeEnroute, 2, 0, 5);

	sbSizerStats->Add(bSizerStats, 0, wxEXPAND, 5);

	bSizerBasic->Add(sbSizerStats, 0, wxALL | wxEXPAND, 5);

	wxStaticBoxSizer* sbSizerPoints;
	sbSizerPoints = new wxStaticBoxSizer(
		new wxStaticBox(m_panelBasic, wxID_ANY, _("Recorded points")), wxVERTICAL);

	wxBoxSizer* bSizerShowTime;
	bSizerShowTime = new wxBoxSizer(wxHORIZONTAL);

	m_stShowTime = new wxStaticText(m_panelBasic, wxID_ANY, _("Time shown as"), wxDefaultPosition,
									wxDefaultSize, 0);
	m_stShowTime->Wrap(-1);
	bSizerShowTime->Add(m_stShowTime, 0, wxALL, 5);

	m_rbShowTimeUTC
		= new wxRadioButton(m_panelBasic, wxID_ANY, _("UTC"), wxDefaultPosition, wxDefaultSize, 0);
	bSizerShowTime->Add(m_rbShowTimeUTC, 0, 0, 5);

	m_rbShowTimePC = new wxRadioButton(m_panelBasic, wxID_ANY, _("Local @ PC"), wxDefaultPosition,
									   wxDefaultSize, 0);
	bSizerShowTime->Add(m_rbShowTimePC, 0, 0, 5);

	m_rbShowTimeLocal = new wxRadioButton(m_panelBasic, wxID_ANY, _("LMT @ Track Start"),
										  wxDefaultPosition, wxDefaultSize, 0);
	bSizerShowTime->Add(m_rbShowTimeLocal, 0, 0, 5);

	m_rbShowTimePC->SetValue(true);

	sbSizerPoints->Add(bSizerShowTime, 0, wxEXPAND, 5);

	m_lcPoints = new OCPNTrackListCtrl(m_panelBasic, wxID_ANY, wxDefaultPosition, wxDefaultSize,
									   wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_EDIT_LABELS
									   | wxLC_VIRTUAL);

	m_lcPoints->InsertColumn(0, _("Leg"), wxLIST_FORMAT_LEFT, 45);
	m_lcPoints->InsertColumn(2, _("Distance"), wxLIST_FORMAT_RIGHT, 70);
	m_lcPoints->InsertColumn(3, _("Bearing"), wxLIST_FORMAT_LEFT, 70);
	m_lcPoints->InsertColumn(4, _("Latitude"), wxLIST_FORMAT_LEFT, 85);
	m_lcPoints->InsertColumn(5, _("Longitude"), wxLIST_FORMAT_LEFT, 90);
	m_lcPoints->InsertColumn(6, _("Timestamp"), wxLIST_FORMAT_LEFT, 135);
	m_lcPoints->InsertColumn(7, _("Speed"), wxLIST_FORMAT_CENTER, 100);

	sbSizerPoints->Add(m_lcPoints, 1, wxALL | wxEXPAND, 5);

	bSizerBasic->Add(sbSizerPoints, 1, wxALL | wxEXPAND, 5);

	m_panelBasic->SetSizer(bSizerBasic);
	m_panelBasic->Layout();
	bSizerBasic->Fit(m_panelBasic);
	m_notebook1->AddPage(m_panelBasic, _("Basic"), true);

	m_panelAdvanced
		= new wxPanel(m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	wxBoxSizer* bSizerAdvanced;
	bSizerAdvanced = new wxBoxSizer(wxVERTICAL);

	m_stDescription = new wxStaticText(m_panelAdvanced, wxID_ANY, _("Description"),
									   wxDefaultPosition, wxDefaultSize, 0);
	m_stDescription->Wrap(-1);
	bSizerAdvanced->Add(m_stDescription, 0, wxALL, 5);

	m_tDescription = new wxTextCtrl(m_panelAdvanced, wxID_ANY, wxEmptyString, wxDefaultPosition,
									wxDefaultSize, wxTE_MULTILINE);
	bSizerAdvanced->Add(m_tDescription, 1, wxALL | wxEXPAND, 5);

	sbSizerLinks
		= new wxStaticBoxSizer(new wxStaticBox(m_panelAdvanced, wxID_ANY, _("Links")), wxVERTICAL);

	m_scrolledWindowLinks = new wxScrolledWindow(m_panelAdvanced, wxID_ANY, wxDefaultPosition,
												 wxDefaultSize, wxHSCROLL | wxVSCROLL);
	m_scrolledWindowLinks->SetScrollRate(5, 5);
	bSizerLinks = new wxBoxSizer(wxVERTICAL);

	m_hyperlink1 = new wxHyperlinkCtrl(m_scrolledWindowLinks, wxID_ANY, _("wxFB Website"),
									   wxT("http://www.wxformbuilder.org"), wxDefaultPosition,
									   wxDefaultSize, wxHL_DEFAULT_STYLE);
	m_menuLink = new wxMenu();
	wxMenuItem* m_menuItemEdit;
	m_menuItemEdit
		= new wxMenuItem(m_menuLink, wxID_ANY, wxString(_("Edit")), wxEmptyString, wxITEM_NORMAL);
	m_menuLink->Append(m_menuItemEdit);

	wxMenuItem* m_menuItemAdd;
	m_menuItemAdd = new wxMenuItem(m_menuLink, wxID_ANY, wxString(_("Add new")), wxEmptyString,
								   wxITEM_NORMAL);
	m_menuLink->Append(m_menuItemAdd);

	wxMenuItem* m_menuItemDelete;
	m_menuItemDelete
		= new wxMenuItem(m_menuLink, wxID_ANY, wxString(_("Delete")), wxEmptyString, wxITEM_NORMAL);
	m_menuLink->Append(m_menuItemDelete);

	m_hyperlink1->Connect(wxEVT_RIGHT_DOWN,
						  wxMouseEventHandler(TrackPropDlg::m_hyperlink1OnContextMenu), NULL, this);

	bSizerLinks->Add(m_hyperlink1, 0, wxALL, 5);

	m_scrolledWindowLinks->SetSizer(bSizerLinks);
	m_scrolledWindowLinks->Layout();
	bSizerLinks->Fit(m_scrolledWindowLinks);
	sbSizerLinks->Add(m_scrolledWindowLinks, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer(wxHORIZONTAL);

	m_buttonAddLink = new wxButton(m_panelAdvanced, wxID_ANY, _("Add"), wxDefaultPosition,
								   wxDefaultSize, wxBU_EXACTFIT);
	bSizer27->Add(m_buttonAddLink, 0, wxALL, 5);

	m_toggleBtnEdit = new wxToggleButton(m_panelAdvanced, wxID_ANY, _("Edit"), wxDefaultPosition,
										 wxDefaultSize, 0);
	bSizer27->Add(m_toggleBtnEdit, 0, wxALL, 5);

	m_staticTextEditEnabled
		= new wxStaticText(m_panelAdvanced, wxID_ANY, _("Links are opened in the default browser."),
						   wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextEditEnabled->Wrap(-1);
	bSizer27->Add(m_staticTextEditEnabled, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	sbSizerLinks->Add(bSizer27, 0, wxEXPAND, 5);

	bSizerAdvanced->Add(sbSizerLinks, 1, wxEXPAND, 5);

	m_panelAdvanced->SetSizer(bSizerAdvanced);
	m_panelAdvanced->Layout();
	bSizerAdvanced->Fit(m_panelAdvanced);
	m_notebook1->AddPage(m_panelAdvanced, _("Advanced"), false);

	bSizerMain->Add(m_notebook1, 1, wxEXPAND | wxALL, 5);

	m_sdbBtmBtnsSizer = new wxStdDialogButtonSizer();
	m_sdbBtmBtnsSizerOK = new wxButton(this, wxID_OK);
	m_sdbBtmBtnsSizer->AddButton(m_sdbBtmBtnsSizerOK);
	m_sdbBtmBtnsSizerCancel = new wxButton(this, wxID_CANCEL);
	m_sdbBtmBtnsSizer->AddButton(m_sdbBtmBtnsSizerCancel);

	m_sdbBtmBtnsSizerPrint
		= new wxButton(this, wxID_ANY, _("Print"), wxDefaultPosition, wxDefaultSize);
	m_sdbBtmBtnsSizer->Add(m_sdbBtmBtnsSizerPrint);
	m_sdbBtmBtnsSizerSplit
		= new wxButton(this, wxID_ANY, _("Split"), wxDefaultPosition, wxDefaultSize);
	m_sdbBtmBtnsSizer->Add(m_sdbBtmBtnsSizerSplit);
	m_sdbBtmBtnsSizerExtend
		= new wxButton(this, wxID_ANY, _("Extend"), wxDefaultPosition, wxDefaultSize);
	m_sdbBtmBtnsSizer->Add(m_sdbBtmBtnsSizerExtend);
	m_sdbBtmBtnsSizerToRoute
		= new wxButton(this, wxID_ANY, _("To route"), wxDefaultPosition, wxDefaultSize);
	m_sdbBtmBtnsSizer->Add(m_sdbBtmBtnsSizerToRoute);
	m_sdbBtmBtnsSizerExport
		= new wxButton(this, wxID_ANY, _("Export"), wxDefaultPosition, wxDefaultSize);
	m_sdbBtmBtnsSizer->Add(m_sdbBtmBtnsSizerExport);

	m_sdbBtmBtnsSizer->Realize();

	bSizerMain->Add(m_sdbBtmBtnsSizer, 0, wxALL | wxEXPAND, 5);

	// Make it look nice and add the needed non-standard buttons
	int w1;
	int w2;
	int h;
	((wxWindowBase*)m_stName)->GetSize(&w1, &h);
	((wxWindowBase*)m_stFrom)->GetSize(&w2, &h);
	((wxWindowBase*)m_stName)->SetMinSize(wxSize(wxMax(w1, w2), h));
	((wxWindowBase*)m_stFrom)->SetMinSize(wxSize(wxMax(w1, w2), h));

	this->SetSizer(bSizerMain);
	this->Layout();

	this->Centre(wxBOTH);

	// Connect Events
	m_sdbBtmBtnsSizerCancel->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
									 wxCommandEventHandler(TrackPropDlg::OnCancelBtnClick), NULL,
									 this);
	m_sdbBtmBtnsSizerOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
								 wxCommandEventHandler(TrackPropDlg::OnOKBtnClick), NULL, this);
	m_sdbBtmBtnsSizerPrint->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
									wxCommandEventHandler(TrackPropDlg::OnPrintBtnClick), NULL,
									this);
	m_sdbBtmBtnsSizerSplit->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
									wxCommandEventHandler(TrackPropDlg::OnSplitBtnClick), NULL,
									this);
	m_sdbBtmBtnsSizerExtend->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
									 wxCommandEventHandler(TrackPropDlg::OnExtendBtnClick), NULL,
									 this);
	m_sdbBtmBtnsSizerToRoute->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
									  wxCommandEventHandler(TrackPropDlg::OnToRouteBtnClick), NULL,
									  this);
	m_sdbBtmBtnsSizerExport->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
									 wxCommandEventHandler(TrackPropDlg::OnExportBtnClick), NULL,
									 this);
	m_lcPoints->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
						wxListEventHandler(TrackPropDlg::OnTrackPropListClick), NULL, this);
	Connect(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
			wxListEventHandler(TrackPropDlg::OnTrackPropRightClick), NULL, this);
	Connect(wxEVT_COMMAND_MENU_SELECTED,
			wxCommandEventHandler(TrackPropDlg::OnTrackPropMenuSelected), NULL, this);

	Connect(m_menuItemDelete->GetId(), wxEVT_COMMAND_MENU_SELECTED,
			wxCommandEventHandler(TrackPropDlg::OnDeleteLink));
	Connect(m_menuItemEdit->GetId(), wxEVT_COMMAND_MENU_SELECTED,
			wxCommandEventHandler(TrackPropDlg::OnEditLink));
	Connect(m_menuItemAdd->GetId(), wxEVT_COMMAND_MENU_SELECTED,
			wxCommandEventHandler(TrackPropDlg::OnAddLink));
	m_buttonAddLink->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							 wxCommandEventHandler(TrackPropDlg::OnAddLink), NULL, this);
	m_toggleBtnEdit->Connect(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
							 wxCommandEventHandler(TrackPropDlg::OnEditLinkToggle), NULL, this);

	m_rbShowTimeUTC->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							 wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ), NULL, this);
	m_rbShowTimePC->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ), NULL, this);
	m_rbShowTimeLocal->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							   wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ), NULL, this);

	m_pLinkProp = new LinkPropDialog(this);
}

TrackPropDlg::~TrackPropDlg()
{
	// Disconnect Events
	m_sdbBtmBtnsSizerCancel->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
										wxCommandEventHandler(TrackPropDlg::OnCancelBtnClick), NULL,
										this);
	m_sdbBtmBtnsSizerOK->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
									wxCommandEventHandler(TrackPropDlg::OnOKBtnClick), NULL, this);
	m_sdbBtmBtnsSizerPrint->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
									   wxCommandEventHandler(TrackPropDlg::OnPrintBtnClick), NULL,
									   this);
	m_sdbBtmBtnsSizerSplit->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
									   wxCommandEventHandler(TrackPropDlg::OnSplitBtnClick), NULL,
									   this);
	m_sdbBtmBtnsSizerExtend->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
										wxCommandEventHandler(TrackPropDlg::OnExtendBtnClick), NULL,
										this);
	m_sdbBtmBtnsSizerToRoute->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
										 wxCommandEventHandler(TrackPropDlg::OnToRouteBtnClick),
										 NULL, this);
	m_sdbBtmBtnsSizerExport->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
										wxCommandEventHandler(TrackPropDlg::OnExportBtnClick), NULL,
										this);
	m_lcPoints->Disconnect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
						   wxListEventHandler(TrackPropDlg::OnTrackPropListClick), NULL, this);
	Disconnect(wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
			   wxListEventHandler(TrackPropDlg::OnTrackPropRightClick), NULL, this);
	Disconnect(wxEVT_COMMAND_MENU_SELECTED,
			   wxCommandEventHandler(TrackPropDlg::OnTrackPropMenuSelected), NULL, this);

	Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
			   wxCommandEventHandler(TrackPropDlg::OnDeleteLink));
	Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
			   wxCommandEventHandler(TrackPropDlg::OnEditLink));
	Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
			   wxCommandEventHandler(TrackPropDlg::OnAddLink));
	m_buttonAddLink->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
								wxCommandEventHandler(TrackPropDlg::OnAddLink), NULL, this);
	m_toggleBtnEdit->Disconnect(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
								wxCommandEventHandler(TrackPropDlg::OnEditLinkToggle), NULL, this);

	m_rbShowTimeUTC->Disconnect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
								wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ), NULL, this);
	m_rbShowTimePC->Disconnect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							   wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ), NULL, this);
	m_rbShowTimeLocal->Disconnect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
								  wxCommandEventHandler(TrackPropDlg::OnShowTimeTZ), NULL, this);

	delete m_menuLink;
}

void TrackPropDlg::SetTrackAndUpdate(Route* pR)
{
	m_pRoute = pR;
	m_lcPoints->DeleteAllItems();
	InitializeList();
	UpdateProperties();
}

void TrackPropDlg::InitializeList()
{
	if (NULL == m_pRoute)
		return;

	m_lcPoints->set_route(m_pRoute);
	m_lcPoints->set_lmt_offset((m_pRoute->routepoints().front()->latitude()) * 3600.0 / 15.0);
	m_lcPoints->SetItemCount(m_pRoute->GetnPoints());
}

bool TrackPropDlg::UpdateProperties()
{
	if (NULL == m_pRoute)
		return false;

	::wxBeginBusyCursor();

	wxWindowList kids = m_scrolledWindowLinks->GetChildren();
	for (unsigned int i = 0; i < kids.size(); i++) {
		wxWindowListNode* node = kids.Item(i);
		wxWindow* win = node->GetData();

		if (win->IsKindOf(CLASSINFO(wxHyperlinkCtrl))) {
			((wxHyperlinkCtrl*)win)->Disconnect(
				wxEVT_COMMAND_HYPERLINK, wxHyperlinkEventHandler(TrackPropDlg::OnHyperLinkClick));
			((wxHyperlinkCtrl*)win)->Disconnect(
				wxEVT_RIGHT_DOWN, wxMouseEventHandler(TrackPropDlg::m_hyperlinkContextMenu));
		}
	}
	m_scrolledWindowLinks->DestroyChildren();

	// FIXME: code duplication of MarkInfoImpl::add_hyperlink
	const Hyperlinks& linklist = m_pRoute->m_HyperlinkList;
	for (Hyperlinks::const_iterator i = linklist.begin(); i != linklist.end(); ++i) {
		// FIXME: maybe here got some code lost (ctrl->Connect(wxEVT_COMMAND_HYPERLINK...)???
		add_hyperlink(i->desc(), i->url(), m_pRoute->m_bIsInLayer);
	}
	bSizerLinks->Fit(m_scrolledWindowLinks);

	m_tName->SetValue(m_pRoute->get_name());
	m_tFrom->SetValue(m_pRoute->get_startString());
	m_tTo->SetValue(m_pRoute->get_endString());
	m_tDescription->SetValue(m_pRoute->get_description());

	m_tTotDistance->SetValue(_T(""));
	m_tTimeEnroute->SetValue(_T(""));

	m_sdbBtmBtnsSizerSplit->Enable(false);
	m_sdbBtmBtnsSizerExtend->Enable(false);

	m_pRoute->UpdateSegmentDistances(); // get segment and total distance
	// but ignore leg speed calcs

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
			m_avgspeed = m_pRoute->m_route_length / total_seconds * 3600;
		} else {
			m_avgspeed = 0;
		}
		m_tAvgSpeed->SetValue(wxString::Format(_T("%5.2f"), toUsrSpeed(m_avgspeed)));
	} else {
		wxString s(_T("--"));
		m_tAvgSpeed->SetValue(s);
	}

	// Total length
	m_tTotDistance->SetValue(wxString::Format(wxT("%5.2f ") + getUsrDistanceUnit(), toUsrDistance(m_pRoute->m_route_length)));

	// Time
	wxString time_form;
	wxTimeSpan time(0, 0, (int)total_seconds, 0);
	if (total_seconds > 3600.0 * 24.0)
		time_form = time.Format(_("%D Days, %H:%M"));
	else if (total_seconds > 0.)
		time_form = time.Format(_("%H:%M"));
	else
		time_form = _T("--");
	m_tTimeEnroute->SetValue(time_form);

	m_cbShow->SetValue(m_pRoute->IsVisible());

	if (m_pRoute->m_Colour == wxEmptyString) {
		m_cColor->Select(0);
	} else {
		for (unsigned int i = 0; i < sizeof(::GpxxColorNames) / sizeof(wxString); i++) {
			if (m_pRoute->m_Colour == ::GpxxColorNames[i]) {
				m_cColor->Select(i + 1);
				break;
			}
		}
	}

	for (unsigned int i = 0; i < sizeof(::StyleValues) / sizeof(int); i++) {
		if (m_pRoute->m_style == ::StyleValues[i]) {
			m_cStyle->Select(i);
			break;
		}
	}

	for (unsigned int i = 0; i < sizeof(::WidthValues) / sizeof(int); i++) {
		if (m_pRoute->m_width == ::WidthValues[i]) {
			m_cWidth->Select(i);
			break;
		}
	}

	if (m_pRoute->m_bIsInLayer) {
		m_tName->SetEditable(false);
		m_tFrom->SetEditable(false);
		m_tTo->SetEditable(false);
		m_tDescription->SetEditable(false);
		m_cbShow->Enable(false);
		m_cColor->Enable(false);
		m_cStyle->Enable(false);
		m_cWidth->Enable(false);
		m_sdbBtmBtnsSizerExtend->Enable(false);
		m_sdbBtmBtnsSizerSplit->Enable(false);
		SetTitle(wxString::Format(_("Track Properties, Layer: %d"), m_pRoute->m_LayerID));
	} else {
		m_tName->SetEditable(true);
		m_tFrom->SetEditable(true);
		m_tTo->SetEditable(true);
		m_tDescription->SetEditable(true);
		m_cbShow->Enable(true);
		m_cColor->Enable(true);
		m_cStyle->Enable(true);
		m_cWidth->Enable(true);

		m_sdbBtmBtnsSizerExtend->Enable(IsThisTrackExtendable());
		SetTitle(_("Track Properties"));
	}

	::wxEndBusyCursor();

	return true;
}

void TrackPropDlg::add_hyperlink(const wxString& desc, const wxString& link, bool on_layer)
{
	wxHyperlinkCtrl* ctrl
		= new wxHyperlinkCtrl(m_scrolledWindowLinks, wxID_ANY, desc, link, wxDefaultPosition,
							  wxDefaultSize, wxHL_DEFAULT_STYLE);
	ctrl->Connect(wxEVT_COMMAND_HYPERLINK, wxHyperlinkEventHandler(TrackPropDlg::OnHyperLinkClick),
				  NULL, this);
	if (!on_layer)
		ctrl->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(TrackPropDlg::m_hyperlinkContextMenu),
					  NULL, this);

	bSizerLinks->Add(ctrl, 0, wxALL, 5);
}

bool TrackPropDlg::IsThisTrackExtendable()
{
	m_pExtendRoute = NULL;
	m_pExtendPoint = NULL;

	if (global::OCPN::get().tracker().is_active_track(m_pRoute) || m_pRoute->m_bIsInLayer)
		return false;

	RoutePoint* pLastPoint = m_pRoute->GetPoint(1);
	if (!pLastPoint->GetCreateTime().IsValid())
		return false;

	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route* route = *i;
		if (route->m_bIsTrack && route->IsVisible() && (route->guid() != m_pRoute->guid())) {
			RoutePoint* track_node = route->GetLastPoint();
			if (track_node->GetCreateTime().IsValid()) {
				if (track_node->GetCreateTime() <= pLastPoint->GetCreateTime())
					if (!m_pExtendPoint || track_node->GetCreateTime()
										   > m_pExtendPoint->GetCreateTime()) {
						m_pExtendPoint = track_node;
						m_pExtendRoute = route;
					}
			}
		}
	}

	if (m_pExtendRoute)
		return !m_pExtendRoute->m_bIsInLayer;
	else
		return false;
}

void TrackPropDlg::OnExtendBtnClick(wxCommandEvent&)
{
	RoutePoint* pLastPoint = m_pRoute->GetPoint(1);

	if (IsThisTrackExtendable()) {
		int begin = 1;
		if (pLastPoint->GetCreateTime() == m_pExtendPoint->GetCreateTime())
			begin = 2;
		pSelect->DeleteAllSelectableTrackSegments(m_pExtendRoute);
		m_pExtendRoute->CloneTrack(m_pRoute, begin, m_pRoute->GetnPoints(), _("_plus"));
		pSelect->AddAllSelectableTrackSegments(m_pExtendRoute);
		pSelect->DeleteAllSelectableTrackSegments(m_pRoute);
		m_pRoute->ClearHighlights();
		g_pRouteMan->DeleteTrack(m_pRoute);

		SetTrackAndUpdate(m_pExtendRoute);
		UpdateProperties();

		if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
			pRouteManagerDialog->UpdateTrkListCtrl();
	}
}

void TrackPropDlg::OnSplitBtnClick(wxCommandEvent&)
{
	m_sdbBtmBtnsSizerSplit->Enable(false);

	if (m_pRoute->m_bIsInLayer)
		return;

	if ((m_nSelected > 1) && (m_nSelected < m_pRoute->GetnPoints())) {
		m_pHead = new Track();
		m_pTail = new Track();
		m_pHead->CloneTrack(m_pRoute, 1, m_nSelected, _("_A"));
		m_pTail->CloneTrack(m_pRoute, m_nSelected, m_pRoute->GetnPoints(), _("_B"));
		pRouteList->push_back(m_pHead);
		pConfig->AddNewRoute(m_pHead, -1);
		m_pHead->RebuildGUIDList();

		pRouteList->push_back(m_pTail);
		pConfig->AddNewRoute(m_pTail, -1);
		m_pTail->RebuildGUIDList();

		pConfig->DeleteConfigRoute(m_pRoute);

		pSelect->DeleteAllSelectableRoutePoints(m_pRoute);
		pSelect->DeleteAllSelectableRouteSegments(m_pRoute);
		g_pRouteMan->DeleteRoute(m_pRoute);
		pSelect->AddAllSelectableRouteSegments(m_pTail);
		pSelect->AddAllSelectableRoutePoints(m_pTail);
		pSelect->AddAllSelectableRouteSegments(m_pHead);
		pSelect->AddAllSelectableRoutePoints(m_pHead);

		SetTrackAndUpdate(m_pTail);
		UpdateProperties();

		if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
			pRouteManagerDialog->UpdateTrkListCtrl();
	}
}

void TrackPropDlg::OnTrackPropCopyTxtClick(wxCommandEvent&)
{
	wxString tab("\t", wxConvUTF8);
	wxString eol("\n", wxConvUTF8);
	wxString csvString;

	csvString << this->GetTitle() << eol << _("Name") << tab << m_pRoute->get_name() << eol
			  << _("Depart From") << tab << m_pRoute->get_startString() << eol << _("Destination")
			  << tab << m_pRoute->get_endString() << eol << _("Total Distance") << tab
			  << m_tTotDistance->GetValue() << eol << _("Speed") << tab << m_tAvgSpeed->GetValue()
			  << eol << _("Departure Time (m/d/y h:m)") << tab
			  << m_pRoute->GetPoint(1)->GetCreateTime().Format() << eol << _("Time Enroute") << tab
			  << m_tTimeEnroute->GetValue() << eol << eol;

	int noCols;
	int noRows;
	noCols = m_lcPoints->GetColumnCount();
	noRows = m_lcPoints->GetItemCount();
	wxListItem item;
	item.SetMask(wxLIST_MASK_TEXT);

	for (int i = 0; i < noCols; i++) {
		m_lcPoints->GetColumn(i, item);
		csvString << item.GetText() << tab;
	}
	csvString << eol;

	for (int j = 0; j < noRows; j++) {
		item.SetId(j);
		for (int i = 0; i < noCols; i++) {
			item.SetColumn(i);
			m_lcPoints->GetItem(item);
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

void TrackPropDlg::OnPrintBtnClick(wxCommandEvent&)
{
	RoutePrintSelection* pTrackPrintSelection = new RoutePrintSelection(this, m_pRoute);
	pTrackPrintSelection->ShowModal();
	delete pTrackPrintSelection;
}

void TrackPropDlg::OnTrackPropRightClick(wxListEvent&)
{
	wxMenu menu;
	menu.Append(ID_RCLK_MENU_COPY_TEXT, _("&Copy all as text"));
	PopupMenu(&menu);
}

void TrackPropDlg::OnTrackPropListClick(wxListEvent&)
{
	long itemno = -1;
	m_nSelected = 0;

	int selected_no;
	itemno = m_lcPoints->GetNextItem(itemno, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (itemno == -1)
		selected_no = 0;
	else
		selected_no = itemno;

	m_pRoute->ClearHighlights();

	RoutePointList::iterator i = m_pRoute->routepoints().begin();
	while ((i != m_pRoute->routepoints().end()) && itemno--) { // FIXME: this is basically an indexed access
		++i;
	}
	if (i != m_pRoute->routepoints().end()) {
		RoutePoint* prp = *i;
		if (prp) {
			prp->m_bPtIsSelected = true; // highlight the routepoint

			if (!(m_pRoute->m_bIsInLayer) && !global::OCPN::get().tracker().is_active_track(m_pRoute)
				&& !(m_pRoute->m_bRtIsActive)) {
				m_nSelected = selected_no + 1;
				m_sdbBtmBtnsSizerSplit->Enable(true);
			}
			gFrame->JumpToPosition(prp->get_position(), cc1->GetVPScale());
		}
	}
	if (selected_no == 0 || selected_no == m_pRoute->GetnPoints() - 1)
		m_sdbBtmBtnsSizerSplit->Enable(false);
}

void TrackPropDlg::OnTrackPropMenuSelected(wxCommandEvent& event)
{
	switch (event.GetId()) {
		case ID_RCLK_MENU_COPY_TEXT:
			OnTrackPropCopyTxtClick(event);
			break;
	}
}

void TrackPropDlg::OnToRouteBtnClick(wxCommandEvent&)
{
	pRouteManagerDialog->TrackToRoute((Track*)m_pRoute);
	if (NULL != pRouteManagerDialog && pRouteManagerDialog->IsVisible())
		pRouteManagerDialog->UpdateRouteListCtrl();
}

void TrackPropDlg::OnExportBtnClick(wxCommandEvent&)
{
	wxString suggested_name = _("track");
	RouteList list;
	list.push_back(m_pRoute);
	if (m_pRoute->get_name() != wxEmptyString)
		suggested_name = m_pRoute->get_name();
	pConfig->ExportGPXRoutes(this, &list, suggested_name);
}

void TrackPropDlg::m_hyperlinkContextMenu(wxMouseEvent& event)
{
	m_pEditedLink = (wxHyperlinkCtrl*)event.GetEventObject();
	m_scrolledWindowLinks->PopupMenu(m_menuLink,
									 m_pEditedLink->GetPosition().x + event.GetPosition().x,
									 m_pEditedLink->GetPosition().y + event.GetPosition().y);
}

void TrackPropDlg::OnDeleteLink(wxCommandEvent& event)
{
	wxString findurl = m_pEditedLink->GetURL();
	wxString findlabel = m_pEditedLink->GetLabel();

	// remove all links, re-insert all non-deleted in the loop below
	m_scrolledWindowLinks->DestroyChildren();

	// FIXME: use find_if
	// FIXME: code duplication of MarkInfoImpl::OnDeleteLink
	Hyperlinks& linklist = m_pRoute->m_HyperlinkList;
	for (Hyperlinks::iterator i = linklist.begin(); i != linklist.end(); ++i) {
		wxString Link = i->url();
		wxString Descr = i->desc();
		if (Link == findurl
			&& (Descr == findlabel || (Link == findlabel && Descr == wxEmptyString))) {

			// found hyperlink to delete, repopulate GUI list
			linklist.erase(i);
			build_hyperlink_list();
			break;
		}
	}

	m_scrolledWindowLinks->InvalidateBestSize();
	m_scrolledWindowLinks->Layout();
	sbSizerLinks->Layout();
	event.Skip();
}

void TrackPropDlg::build_hyperlink_list()
{
	if (!m_pRoute)
		return;

	const Hyperlinks& linklist = m_pRoute->m_HyperlinkList;
	for (Hyperlinks::const_iterator i = linklist.begin(); i != linklist.end(); ++i) {
		add_hyperlink(i->desc(), i->url(), m_pRoute->m_bIsInLayer);
	}
}

void TrackPropDlg::OnEditLink(wxCommandEvent& event)
{
	wxString findurl = m_pEditedLink->GetURL();
	wxString findlabel = m_pEditedLink->GetLabel();
	m_pLinkProp->m_textCtrlLinkDescription->SetValue(findlabel);
	m_pLinkProp->m_textCtrlLinkUrl->SetValue(findurl);
	if (m_pLinkProp->ShowModal() != wxID_OK) {
		event.Skip();
		return;
	}

	Hyperlinks& linklist = m_pRoute->m_HyperlinkList;
	// FIXME: use find_if
	for (Hyperlinks::iterator i = linklist.begin(); i != linklist.end(); ++i) {
		if ((i->url() == findurl)
			&& (i->desc() == findlabel
				|| ((i->url() == findlabel) && (i->desc() == wxEmptyString)))) {
			*i = Hyperlink(m_pLinkProp->m_textCtrlLinkDescription->GetValue(),
						   m_pLinkProp->m_textCtrlLinkUrl->GetValue(), i->type());
			wxHyperlinkCtrl* h
				= static_cast<wxHyperlinkCtrl*>(m_scrolledWindowLinks->FindWindowByLabel(findlabel));
			if (h) {
				h->SetLabel(m_pLinkProp->m_textCtrlLinkDescription->GetValue());
				h->SetURL(m_pLinkProp->m_textCtrlLinkUrl->GetValue());
			}
		}
	}

	m_scrolledWindowLinks->InvalidateBestSize();
	m_scrolledWindowLinks->Layout();
	sbSizerLinks->Layout();
	event.Skip();
}

void TrackPropDlg::OnAddLink(wxCommandEvent& event)
{
	m_pLinkProp->m_textCtrlLinkDescription->SetValue(wxEmptyString);
	m_pLinkProp->m_textCtrlLinkUrl->SetValue(wxEmptyString);
	if (m_pLinkProp->ShowModal() == wxID_OK) {
		wxString desc = m_pLinkProp->m_textCtrlLinkDescription->GetValue();
		if (desc == wxEmptyString)
			desc = m_pLinkProp->m_textCtrlLinkUrl->GetValue();

		// FIXME: code duplication
		wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl(
			m_scrolledWindowLinks, wxID_ANY, desc, m_pLinkProp->m_textCtrlLinkUrl->GetValue(),
			wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
		ctrl->Connect(wxEVT_COMMAND_HYPERLINK,
					  wxHyperlinkEventHandler(TrackPropDlg::OnHyperLinkClick), NULL, this);
		ctrl->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(TrackPropDlg::m_hyperlinkContextMenu),
					  NULL, this);

		bSizerLinks->Add(ctrl, 0, wxALL, 5);
		bSizerLinks->Fit(m_scrolledWindowLinks);
		this->Fit();

		m_pRoute->m_HyperlinkList.push_back(
			Hyperlink(m_pLinkProp->m_textCtrlLinkDescription->GetValue(),
					  m_pLinkProp->m_textCtrlLinkUrl->GetValue(), wxEmptyString));
	}

	sbSizerLinks->Layout();
	event.Skip();
}

void TrackPropDlg::OnEditLinkToggle(wxCommandEvent& event)
{
	if (m_toggleBtnEdit->GetValue())
		m_staticTextEditEnabled->SetLabel(_("Links are opened for editing."));
	else
		m_staticTextEditEnabled->SetLabel(_("Links are opened in the default browser."));
	event.Skip();
}

void TrackPropDlg::OnHyperLinkClick(wxHyperlinkEvent& event)
{
	if (m_toggleBtnEdit->GetValue()) {
		m_pEditedLink = (wxHyperlinkCtrl*)event.GetEventObject();
		OnEditLink(event);
		event.Skip(false);
		return;
	}

#ifdef __WXMSW__
	// FIXME: this smells like code duplication, see MarkInfoImpl::OnHyperLinkClick

	// Windows has trouble handling local file URLs with embedded anchor points, e.g
	// file://testfile.html#point1
	// The trouble is with the wxLaunchDefaultBrowser with verb "open"
	// Workaround is to probe the registry to get the default browser, and open directly
	//
	// But, we will do this only if the URL contains the anchor point charater '#'
	// What a hack......

	wxString cc = event.GetURL();
	if (cc.Find(_T("#")) != wxNOT_FOUND) {
		wxRegKey RegKey(wxString(_T("HKEY_CLASSES_ROOT\\HTTP\\shell\\open\\command")));
		if (RegKey.Exists()) {
			wxString command_line;
			RegKey.QueryValue(wxString(_T("")), command_line);

			//  Remove "
			command_line.Replace(wxString(_T("\"")), wxString(_T("")));

			//  Strip arguments
			int l = command_line.Find(_T(".exe"));
			if (wxNOT_FOUND == l)
				l = command_line.Find(_T(".EXE"));

			if (wxNOT_FOUND != l) {
				wxString cl = command_line.Mid(0, l + 4);
				cl += _T(" ");
				cc.Prepend(_T("\""));
				cc.Append(_T("\""));
				cl += cc;
				wxExecute(cl); // Async, so Fire and Forget...
			}
		}
	} else {
		event.Skip();
	}
#else
	wxString url = event.GetURL();
	url.Replace(_T(" "), _T("%20"));
	::wxLaunchDefaultBrowser(url);
#endif
}

void TrackPropDlg::OnShowTimeTZ(wxCommandEvent& WXUNUSED(event))
{
	if (m_rbShowTimeUTC->GetValue())
		m_lcPoints->set_tz_selection(OCPNTrackListCtrl::UTCINPUT);
	else if (m_rbShowTimePC->GetValue())
		m_lcPoints->set_tz_selection(OCPNTrackListCtrl::LTINPUT);
	else
		m_lcPoints->set_tz_selection(OCPNTrackListCtrl::LMTINPUT);
	m_lcPoints->DeleteAllItems();
	InitializeList();
}

bool TrackPropDlg::SaveChanges(void)
{
	if (m_pRoute && !m_pRoute->m_bIsInLayer) {
		// Get User input Text Fields
		m_pRoute->set_name(m_tName->GetValue());
		m_pRoute->set_startString(m_tFrom->GetValue());
		m_pRoute->set_endString(m_tTo->GetValue());
		m_pRoute->set_description(m_tDescription->GetValue());
		m_pRoute->SetVisible(m_cbShow->GetValue());
		if (m_cColor->GetSelection() == 0)
			m_pRoute->m_Colour = wxEmptyString;
		else
			m_pRoute->m_Colour = ::GpxxColorNames[m_cColor->GetSelection() - 1];
		m_pRoute->m_style = ::StyleValues[m_cStyle->GetSelection()];
		m_pRoute->m_width = ::WidthValues[m_cWidth->GetSelection()];

		pConfig->UpdateRoute(m_pRoute);
		pConfig->UpdateSettings();
	}

	if (static_cast<Track*>(m_pRoute)->IsRunning()) {
		wxJSONValue v;
		v[_T("Name")] = m_pRoute->get_name();
		v[_T("GUID")] = m_pRoute->guid();
		wxString msg_id(_T("OCPN_TRK_ACTIVATED"));
		g_pi_manager->SendJSONMessageToAllPlugins(msg_id, v);
	}

	return true;
}

void TrackPropDlg::OnOKBtnClick(wxCommandEvent& event)
{
	// Look in the route list to be sure the route is still available
	// (May have been deleted by RouteManagerDialog...)

	if (g_pRouteMan->RouteExists(m_pRoute)) {
		SaveChanges(); // write changes to globals and update config
		m_pRoute->ClearHighlights();
	}

	m_pEnroutePoint = NULL;
	m_bStartNow = false;

	if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
		pRouteManagerDialog->UpdateTrkListCtrl();

	Hide();
	cc1->Refresh(false);

	event.Skip();
}

void TrackPropDlg::OnCancelBtnClick(wxCommandEvent& event)
{
	// Look in the route list to be sure the raoute is still available
	// (May have been deleted by RouteMangerDialog...)

	if (g_pRouteMan->RouteExists(m_pRoute))
		m_pRoute->ClearHighlights();

	Hide();
	cc1->Refresh(false);

	event.Skip();
}

void TrackPropDlg::m_hyperlink1OnContextMenu(wxMouseEvent& event)
{
	m_hyperlink1->PopupMenu(m_menuLink, event.GetPosition());
}

Route* TrackPropDlg::GetTrack(void)
{
	return m_pRoute;
}

