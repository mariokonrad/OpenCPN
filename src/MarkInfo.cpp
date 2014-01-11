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

#include "MarkInfo.h"
#include <RouteProp.h>
#include <LinkPropDlg.h>
#include <WayPointman.h>
#include <Select.h>
#include <Routeman.h>
#include <PositionParser.h>
#include <RouteManagerDialog.h>
#include <Config.h>
#include <ChartCanvas.h>
#include <DimeControl.h>

#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/bmpcbox.h>
#include <wx/hyperlink.h>
#include <wx/tglbtn.h>
#include <wx/clipbrd.h>

extern WayPointman* pWayPointMan;
extern Select* pSelect;
extern Config* pConfig;
extern ChartCanvas* cc1;
extern RouteManagerDialog* pRouteManagerDialog;
extern Routeman* g_pRouteMan;

MarkInfoDef::MarkInfoDef(
		wxWindow * parent,
		wxWindowID id,
		const wxString & title,
		const wxPoint & pos,
		const wxSize & size,
		long style)
{
	long wstyle = style;
#ifdef __WXOSX__
	wstyle |= wxSTAY_ON_TOP;
#endif

	Create(parent, id, title, pos, size, wstyle);

	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	m_notebookProperties = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
	m_panelBasicProperties = new wxPanel(m_notebookProperties, wxID_ANY, wxDefaultPosition,
										 wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizerBasicProperties;
	bSizerBasicProperties = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer* sbSizerProperties;
	sbSizerProperties = new wxStaticBoxSizer(
		new wxStaticBox(m_panelBasicProperties, wxID_ANY, _("Properties")), wxVERTICAL);

	wxBoxSizer* bSizerInnerProperties;
	bSizerInnerProperties = new wxBoxSizer(wxHORIZONTAL);

	m_bitmapIcon = new wxStaticBitmap(m_panelBasicProperties, wxID_ANY, wxNullBitmap,
									  wxDefaultPosition, wxDefaultSize, 0);
	bSizerInnerProperties->Add(m_bitmapIcon, 0, wxALL, 5);

	wxBoxSizer* bSizerTextProperties;
	bSizerTextProperties = new wxBoxSizer(wxVERTICAL);

	m_staticTextLayer = new wxStaticText(m_panelBasicProperties, wxID_ANY,
										 _("This waypoint is part of a layer and can't be edited"),
										 wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextLayer->Wrap(-1);
	m_staticTextLayer->Enable(false);

	bSizerTextProperties->Add(m_staticTextLayer, 0, wxALL, 5);

	wxBoxSizer* bSizerName;
	bSizerName = new wxBoxSizer(wxHORIZONTAL);

	m_staticTextName = new wxStaticText(m_panelBasicProperties, wxID_ANY, _("Name"),
										wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextName->Wrap(-1);
	bSizerName->Add(m_staticTextName, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	wxBoxSizer* bSizerNameValue;
	bSizerNameValue = new wxBoxSizer(wxVERTICAL);

	m_textName = new wxTextCtrl(m_panelBasicProperties, wxID_ANY, wxEmptyString, wxDefaultPosition,
								wxDefaultSize, 0);
	bSizerNameValue->Add(m_textName, 0, wxALL | wxEXPAND, 5);

	bSizerName->Add(bSizerNameValue, 1, wxEXPAND, 5);

	bSizerTextProperties->Add(bSizerName, 0, wxEXPAND, 5);

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer(wxHORIZONTAL);

	m_checkBoxShowName = new wxCheckBox(m_panelBasicProperties, wxID_ANY, _("Show name"),
										wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	bSizer8->Add(m_checkBoxShowName, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	m_staticTextIcon = new wxStaticText(m_panelBasicProperties, wxID_ANY, _("Icon"),
										wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextIcon->Wrap(-1);
	bSizer8->Add(m_staticTextIcon, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	m_bcomboBoxIcon
		= new wxBitmapComboBox(m_panelBasicProperties, wxID_ANY, _("Combo!"), wxDefaultPosition,
							   wxDefaultSize, 0, NULL, wxCB_READONLY);
	bSizer8->Add(m_bcomboBoxIcon, 1, wxALL, 5);

	bSizerTextProperties->Add(bSizer8, 0, wxEXPAND, 5);

	wxBoxSizer* bSizerLatLon;
	bSizerLatLon = new wxBoxSizer(wxHORIZONTAL);

	m_staticTextLatitude = new wxStaticText(m_panelBasicProperties, wxID_ANY, _("Latitude"),
											wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextLatitude->Wrap(-1);
	bSizerLatLon->Add(m_staticTextLatitude, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	m_textLatitude = new wxTextCtrl(m_panelBasicProperties, wxID_ANY, wxEmptyString,
									wxDefaultPosition, wxDefaultSize, 0);
	bSizerLatLon->Add(m_textLatitude, 1, wxALL, 5);

	m_staticTextLongitude = new wxStaticText(m_panelBasicProperties, wxID_ANY, _("Longitude"),
											 wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextLongitude->Wrap(-1);
	bSizerLatLon->Add(m_staticTextLongitude, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	m_textLongitude = new wxTextCtrl(m_panelBasicProperties, wxID_ANY, wxEmptyString,
									 wxDefaultPosition, wxDefaultSize, 0);
	bSizerLatLon->Add(m_textLongitude, 1, wxALL, 5);

	bSizerTextProperties->Add(bSizerLatLon, 0, wxEXPAND, 5);

	m_staticTextDescription = new wxStaticText(m_panelBasicProperties, wxID_ANY, _("Description"),
											   wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextDescription->Wrap(-1);
	bSizerTextProperties->Add(m_staticTextDescription, 0, wxALL, 5);

	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer(wxHORIZONTAL);

	m_textDescription = new wxTextCtrl(m_panelBasicProperties, wxID_ANY, wxEmptyString,
									   wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	m_textDescription->SetMinSize(wxSize(-1, 60));

	bSizer14->Add(m_textDescription, 1, wxALL | wxEXPAND, 5);

	m_buttonExtDescription = new wxButton(m_panelBasicProperties, wxID_ANY, _("..."),
										  wxDefaultPosition, wxSize(20, -1), 0);
	bSizer14->Add(m_buttonExtDescription, 0, wxALL | wxEXPAND, 5);

	bSizerTextProperties->Add(bSizer14, 1, wxEXPAND, 5);

	bSizerInnerProperties->Add(bSizerTextProperties, 1, wxEXPAND, 5);

	sbSizerProperties->Add(bSizerInnerProperties, 1, wxEXPAND, 5);

	bSizerBasicProperties->Add(sbSizerProperties, 3, wxALL | wxEXPAND, 5);

	sbSizerLinks = new wxStaticBoxSizer(
		new wxStaticBox(m_panelBasicProperties, wxID_ANY, _("Links")), wxVERTICAL);

	m_scrolledWindowLinks = new wxScrolledWindow(
		m_panelBasicProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
	m_scrolledWindowLinks->SetScrollRate(5, 5);
	bSizerLinks = new wxBoxSizer(wxVERTICAL);

	m_hyperlink17 = new wxHyperlinkCtrl(m_scrolledWindowLinks, wxID_ANY, _("wxFB Website"),
										wxT("file:///C:\\ProgramData\\opencpn\\opencpn.log"),
										wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
	m_menuLink = new wxMenu();
	wxMenuItem* m_menuItemDelete;
	m_menuItemDelete
		= new wxMenuItem(m_menuLink, wxID_ANY, wxString(_("Delete")), wxEmptyString, wxITEM_NORMAL);
	m_menuLink->Append(m_menuItemDelete);

	wxMenuItem* m_menuItemEdit;
	m_menuItemEdit
		= new wxMenuItem(m_menuLink, wxID_ANY, wxString(_("Edit")), wxEmptyString, wxITEM_NORMAL);
	m_menuLink->Append(m_menuItemEdit);

	wxMenuItem* m_menuItemAdd;
	m_menuItemAdd = new wxMenuItem(m_menuLink, wxID_ANY, wxString(_("Add new")), wxEmptyString,
								   wxITEM_NORMAL);
	m_menuLink->Append(m_menuItemAdd);

	m_hyperlink17->Connect(wxEVT_RIGHT_DOWN,
						   wxMouseEventHandler(MarkInfoDef::hyperlink17OnContextMenu), NULL, this);

	bSizerLinks->Add(m_hyperlink17, 0, wxALL, 5);

	m_scrolledWindowLinks->SetSizer(bSizerLinks);
	m_scrolledWindowLinks->Layout();
	bSizerLinks->Fit(m_scrolledWindowLinks);
	sbSizerLinks->Add(m_scrolledWindowLinks, 1, wxEXPAND | wxALL, 5);

	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer(wxHORIZONTAL);

	m_buttonAddLink = new wxButton(m_panelBasicProperties, wxID_ANY, _("Add"), wxDefaultPosition,
								   wxDefaultSize, wxBU_EXACTFIT);
	bSizer9->Add(m_buttonAddLink, 0, wxALL, 5);

	m_toggleBtnEdit = new wxToggleButton(m_panelBasicProperties, wxID_ANY, _("Edit"),
										 wxDefaultPosition, wxDefaultSize, 0);
	bSizer9->Add(m_toggleBtnEdit, 0, wxALL, 5);

	m_staticTextEditEnabled = new wxStaticText(m_panelBasicProperties, wxID_ANY,
											   _("Links are opened in the default browser."),
											   wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextEditEnabled->Wrap(-1);
	bSizer9->Add(m_staticTextEditEnabled, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	sbSizerLinks->Add(bSizer9, 0, wxEXPAND, 5);

	bSizerBasicProperties->Add(sbSizerLinks, 2, wxALL | wxEXPAND, 5);

	m_panelBasicProperties->SetSizer(bSizerBasicProperties);
	m_panelBasicProperties->Layout();
	bSizerBasicProperties->Fit(m_panelBasicProperties);
	m_notebookProperties->AddPage(m_panelBasicProperties, _("Basic"), true);
	m_panelDescription = new wxPanel(m_notebookProperties, wxID_ANY, wxDefaultPosition,
									 wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer(wxVERTICAL);

	m_textCtrlExtDescription = new wxTextCtrl(m_panelDescription, wxID_ANY, wxEmptyString,
											  wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	bSizer15->Add(m_textCtrlExtDescription, 1, wxALL | wxEXPAND, 5);

	m_panelDescription->SetSizer(bSizer15);
	m_panelDescription->Layout();
	bSizer15->Fit(m_panelDescription);
	m_notebookProperties->AddPage(m_panelDescription, _("Description"), false);
	m_panelExtendedProperties = new wxPanel(m_notebookProperties, wxID_ANY, wxDefaultPosition,
											wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizerExtendedProperties;
	bSizerExtendedProperties = new wxBoxSizer(wxVERTICAL);

	m_checkBoxVisible = new wxCheckBox(m_panelExtendedProperties, wxID_ANY, _("Show on chart"),
									   wxDefaultPosition, wxDefaultSize, 0);
	bSizerExtendedProperties->Add(m_checkBoxVisible, 0, wxALL, 5);

	wxBoxSizer* bSizerGuid;
	bSizerGuid = new wxBoxSizer(wxHORIZONTAL);

	m_staticTextGuid = new wxStaticText(m_panelExtendedProperties, wxID_ANY, _("GUID"),
										wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextGuid->Wrap(-1);
	bSizerGuid->Add(m_staticTextGuid, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	m_textCtrlGuid = new wxTextCtrl(m_panelExtendedProperties, wxID_ANY, wxEmptyString,
									wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	bSizerGuid->Add(m_textCtrlGuid, 1, wxALL | wxEXPAND, 5);

	bSizerExtendedProperties->Add(bSizerGuid, 0, wxEXPAND, 5);

	m_staticTextGpx = new wxStaticText(m_panelExtendedProperties, wxID_ANY, _("GPX"),
									   wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextGpx->Wrap(-1);
	m_staticTextGpx->Enable(false);

	bSizerExtendedProperties->Add(m_staticTextGpx, 0, wxALL, 5);

	m_textCtrlGpx
		= new wxTextCtrl(m_panelExtendedProperties, wxID_ANY, wxEmptyString, wxDefaultPosition,
						 wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
	m_textCtrlGpx->Enable(false);

	bSizerExtendedProperties->Add(m_textCtrlGpx, 1, wxALL | wxEXPAND, 5);

	m_panelExtendedProperties->SetSizer(bSizerExtendedProperties);
	m_panelExtendedProperties->Layout();
	bSizerExtendedProperties->Fit(m_panelExtendedProperties);
	m_notebookProperties->AddPage(m_panelExtendedProperties, _("Extended"), false);

	bSizer1->Add(m_notebookProperties, 1, wxEXPAND | wxALL, 5);

	m_sdbSizerButtons = new wxStdDialogButtonSizer();
	m_sdbSizerButtonsOK = new wxButton(this, wxID_OK);
	m_sdbSizerButtons->AddButton(m_sdbSizerButtonsOK);
	m_sdbSizerButtonsCancel = new wxButton(this, wxID_CANCEL);
	m_sdbSizerButtons->AddButton(m_sdbSizerButtonsCancel);
	m_sdbSizerButtons->Realize();

	bSizer1->Add(m_sdbSizerButtons, 0, wxALL | wxEXPAND, 5);

	this->SetSizer(bSizer1);
	this->Layout();

	this->Centre(wxBOTH);

	// Connect Events
	m_textLatitude->Connect(wxEVT_COMMAND_TEXT_ENTER,
							wxCommandEventHandler(MarkInfoDef::OnPositionCtlUpdated), NULL, this);
	m_textLongitude->Connect(wxEVT_COMMAND_TEXT_ENTER,
							 wxCommandEventHandler(MarkInfoDef::OnPositionCtlUpdated), NULL, this);

	m_textLatitude->Connect(wxEVT_CONTEXT_MENU, wxCommandEventHandler(MarkInfoImpl::OnRightClick),
							NULL, this);
	m_textLongitude->Connect(wxEVT_CONTEXT_MENU, wxCommandEventHandler(MarkInfoImpl::OnRightClick),
							 NULL, this);

	m_textDescription->Connect(wxEVT_COMMAND_TEXT_UPDATED,
							   wxCommandEventHandler(MarkInfoDef::OnDescChangedBasic), NULL, this);
	m_buttonExtDescription->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
									wxCommandEventHandler(MarkInfoDef::OnExtDescriptionClick), NULL,
									this);

	this->Connect(m_menuItemDelete->GetId(), wxEVT_COMMAND_MENU_SELECTED,
				  wxCommandEventHandler(MarkInfoDef::OnDeleteLink));
	this->Connect(m_menuItemEdit->GetId(), wxEVT_COMMAND_MENU_SELECTED,
				  wxCommandEventHandler(MarkInfoDef::OnEditLink));
	this->Connect(m_menuItemAdd->GetId(), wxEVT_COMMAND_MENU_SELECTED,
				  wxCommandEventHandler(MarkInfoDef::OnAddLink));
	this->Connect(ID_RCLK_MENU_COPY, wxEVT_COMMAND_MENU_SELECTED,
				  wxCommandEventHandler(MarkInfoDef::OnCopyPasteLatLon));
	this->Connect(ID_RCLK_MENU_COPY_LL, wxEVT_COMMAND_MENU_SELECTED,
				  wxCommandEventHandler(MarkInfoDef::OnCopyPasteLatLon));
	this->Connect(ID_RCLK_MENU_PASTE, wxEVT_COMMAND_MENU_SELECTED,
				  wxCommandEventHandler(MarkInfoDef::OnCopyPasteLatLon));
	this->Connect(ID_RCLK_MENU_PASTE_LL, wxEVT_COMMAND_MENU_SELECTED,
				  wxCommandEventHandler(MarkInfoDef::OnCopyPasteLatLon));

	m_buttonAddLink->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							 wxCommandEventHandler(MarkInfoDef::OnAddLink), NULL, this);
	m_toggleBtnEdit->Connect(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
							 wxCommandEventHandler(MarkInfoDef::OnEditLinkToggle), NULL, this);
	m_textCtrlExtDescription->Connect(wxEVT_COMMAND_TEXT_UPDATED,
									  wxCommandEventHandler(MarkInfoDef::OnDescChangedExt), NULL,
									  this);
	m_sdbSizerButtonsCancel->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
									 wxCommandEventHandler(MarkInfoDef::OnMarkInfoCancelClick),
									 NULL, this);
	m_sdbSizerButtonsOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
								 wxCommandEventHandler(MarkInfoDef::OnMarkInfoOKClick), NULL, this);
}

MarkInfoDef::~MarkInfoDef()
{
	// Disconnect Events
	m_textLatitude->Disconnect(wxEVT_COMMAND_TEXT_ENTER,
							   wxCommandEventHandler(MarkInfoDef::OnPositionCtlUpdated), NULL,
							   this);
	m_textLongitude->Disconnect(wxEVT_COMMAND_TEXT_ENTER,
								wxCommandEventHandler(MarkInfoDef::OnPositionCtlUpdated), NULL,
								this);
	m_textDescription->Disconnect(wxEVT_COMMAND_TEXT_UPDATED,
								  wxCommandEventHandler(MarkInfoDef::OnDescChangedBasic), NULL,
								  this);
	m_buttonExtDescription->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
									   wxCommandEventHandler(MarkInfoDef::OnExtDescriptionClick),
									   NULL, this);
	this->Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
					 wxCommandEventHandler(MarkInfoDef::OnDeleteLink));
	this->Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
					 wxCommandEventHandler(MarkInfoDef::OnEditLink));
	this->Disconnect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
					 wxCommandEventHandler(MarkInfoDef::OnAddLink));
	m_buttonAddLink->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
								wxCommandEventHandler(MarkInfoDef::OnAddLink), NULL, this);
	m_toggleBtnEdit->Disconnect(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
								wxCommandEventHandler(MarkInfoDef::OnEditLinkToggle), NULL, this);
	m_textCtrlExtDescription->Disconnect(wxEVT_COMMAND_TEXT_UPDATED,
										 wxCommandEventHandler(MarkInfoDef::OnDescChangedExt), NULL,
										 this);
	m_sdbSizerButtonsCancel->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
										wxCommandEventHandler(MarkInfoDef::OnMarkInfoCancelClick),
										NULL, this);
	m_sdbSizerButtonsOK->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
									wxCommandEventHandler(MarkInfoDef::OnMarkInfoOKClick), NULL,
									this);

	delete m_menuLink;
}

void MarkInfoDef::hyperlink17OnContextMenu(wxMouseEvent & event)
{
	m_hyperlink17->PopupMenu(m_menuLink, event.GetPosition());
}

MarkInfoImpl::MarkInfoImpl(wxWindow* parent, wxWindowID id, const wxString& title,
						   const wxPoint& pos, const wxSize& size, long style)
	: MarkInfoDef(parent, id, title, pos, size, style)
{
	m_pLinkProp = new LinkPropDialog(this);
	m_staticTextGpx->Show(false);
	m_textCtrlGpx->Show(false);
	SetColorScheme(static_cast<global::ColorScheme>(0));
}

MarkInfoImpl::~MarkInfoImpl()
{
	m_bcomboBoxIcon->Clear();
}

void MarkInfoImpl::InitialFocus(void)
{
	m_textName->SetFocus();
	m_textName->SetInsertionPointEnd();
}

void MarkInfoImpl::SetColorScheme(global::ColorScheme)
{
	DimeControl(this);
	DimeControl(m_pLinkProp);
}

bool MarkInfoImpl::UpdateProperties(bool positionOnly)
{
	if (!m_pRoutePoint)
		return true;

	m_textLatitude->SetValue(::toSDMM(1, m_pRoutePoint->latitude()));
	m_textLongitude->SetValue(::toSDMM(2, m_pRoutePoint->longitude()));
	m_lat_save = m_pRoutePoint->latitude();
	m_lon_save = m_pRoutePoint->longitude();

	if (positionOnly)
		return true;

	// Layer or not?
	if (m_pRoutePoint->m_bIsInLayer) {
		m_staticTextLayer->Enable();
		m_staticTextLayer->Show(true);
		m_textName->SetEditable(false);
		m_textDescription->SetEditable(false);
		m_textCtrlExtDescription->SetEditable(false);
		m_textLatitude->SetEditable(false);
		m_textLongitude->SetEditable(false);
		m_bcomboBoxIcon->Enable(false);
		m_buttonAddLink->Enable(false);
		m_toggleBtnEdit->Enable(false);
		m_toggleBtnEdit->SetValue(false);
		m_checkBoxShowName->Enable(false);
		m_checkBoxVisible->Enable(false);
	} else {
		m_staticTextLayer->Enable(false);
		m_staticTextLayer->Show(false);
		m_textName->SetEditable(true);
		m_textDescription->SetEditable(true);
		m_textCtrlExtDescription->SetEditable(true);
		m_textLatitude->SetEditable(true);
		m_textLongitude->SetEditable(true);
		m_bcomboBoxIcon->Enable(true);
		m_buttonAddLink->Enable(true);
		m_toggleBtnEdit->Enable(true);
		m_checkBoxShowName->Enable(true);
		m_checkBoxVisible->Enable(true);
	}
	m_textName->SetValue(m_pRoutePoint->GetName());
	m_textDescription->SetValue(m_pRoutePoint->m_MarkDescription);
	m_textCtrlExtDescription->SetValue(m_pRoutePoint->m_MarkDescription);
	m_bitmapIcon->SetBitmap(*m_pRoutePoint->m_pbmIcon);
	wxWindowList kids = m_scrolledWindowLinks->GetChildren();
	for (unsigned int i = 0; i < kids.size(); i++) {
		wxWindowListNode* node = kids.Item(i);
		wxWindow* win = node->GetData();

		if (win->IsKindOf(CLASSINFO(wxHyperlinkCtrl))) {
			((wxHyperlinkCtrl*)win)->Disconnect(
				wxEVT_COMMAND_HYPERLINK, wxHyperlinkEventHandler(MarkInfoImpl::OnHyperLinkClick));
			((wxHyperlinkCtrl*)win)->Disconnect(
				wxEVT_RIGHT_DOWN, wxMouseEventHandler(MarkInfoImpl::hyperlinkContextMenu));
		}
	}
	m_scrolledWindowLinks->DestroyChildren();
	m_checkBoxShowName->SetValue(m_pRoutePoint->m_bShowName);
	m_checkBoxVisible->SetValue(m_pRoutePoint->m_bIsVisible);
	m_textCtrlGuid->SetValue(m_pRoutePoint->m_GUID);

	build_hyperlink_list();
	bSizerLinks->Fit(m_scrolledWindowLinks);

	// Iterate on the Icon Descriptions, filling in the control
	int iconToSelect = 0;
	bool fillCombo = m_bcomboBoxIcon->GetCount() == 0;
	wxImageList* icons = NULL;
	if (fillCombo)
		icons = pWayPointMan->Getpmarkicon_image_list();
	for (int i = 0; i < pWayPointMan->GetNumIcons(); ++i) {
		const wxString ps = pWayPointMan->GetIconDescription(i);
		if (pWayPointMan->GetIconKey(i) == m_pRoutePoint->m_IconName)
			iconToSelect = i;
		if (fillCombo && icons)
			m_bcomboBoxIcon->Append(ps, icons->GetBitmap(i));
	}
	m_bcomboBoxIcon->Select(iconToSelect);
	this->Fit();
	sbSizerLinks->Layout();
	icons = NULL;

	return true;
}

void MarkInfoImpl::build_hyperlink_list()
{
	if (!m_pRoutePoint)
		return;

	const Hyperlinks& linklist = m_pRoutePoint->m_HyperlinkList;
	for (Hyperlinks::const_iterator i = linklist.begin(); i != linklist.end(); ++i) {
		add_hyperlink(i->DescrText, i->Link, m_pRoutePoint->m_bIsInLayer);
	}
}

void MarkInfoImpl::add_hyperlink(const wxString& desc, const wxString& link, bool on_layer)
{
	wxHyperlinkCtrl* ctrl
		= new wxHyperlinkCtrl(m_scrolledWindowLinks, wxID_ANY, desc, link, wxDefaultPosition,
							  wxDefaultSize, wxHL_DEFAULT_STYLE);
	ctrl->Connect(wxEVT_COMMAND_HYPERLINK, wxHyperlinkEventHandler(MarkInfoImpl::OnHyperLinkClick),
				  NULL, this);

	if (!on_layer)
		ctrl->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(MarkInfoImpl::hyperlinkContextMenu),
					  NULL, this);

	bSizerLinks->Add(ctrl, 0, wxALL, 5);
}

void MarkInfoImpl::SetRoutePoint(RoutePoint* pRP)
{
	m_pRoutePoint = pRP;
	if (!m_pRoutePoint)
		return;

	m_lat_save = m_pRoutePoint->latitude();
	m_lon_save = m_pRoutePoint->longitude();
	m_IconName_save = m_pRoutePoint->m_IconName;
	m_bShowName_save = m_pRoutePoint->m_bShowName;
	m_bIsVisible_save = m_pRoutePoint->m_bIsVisible;

	m_pMyLinkList = m_pRoutePoint->m_HyperlinkList;
}

void MarkInfoImpl::hyperlinkContextMenu(wxMouseEvent& event)
{
	m_pEditedLink = (wxHyperlinkCtrl*)event.GetEventObject();
	m_scrolledWindowLinks->PopupMenu(m_menuLink,
									 m_pEditedLink->GetPosition().x + event.GetPosition().x,
									 m_pEditedLink->GetPosition().y + event.GetPosition().y);
}

void MarkInfoImpl::OnDeleteLink(wxCommandEvent& event)
{
	const wxString findurl = m_pEditedLink->GetURL();
	const wxString findlabel = m_pEditedLink->GetLabel();

	// remove all links, re-insert all non-deleted in the loop below
	m_scrolledWindowLinks->DestroyChildren();

	// FIXME: use find_if
	Hyperlinks& linklist = m_pRoutePoint->m_HyperlinkList;
	for (Hyperlinks::iterator i = linklist.begin(); i != linklist.end(); ++i) {
		wxString Link = i->Link;
		wxString Descr = i->DescrText;
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

void MarkInfoImpl::OnEditLink(wxCommandEvent& event)
{
	wxString findurl = m_pEditedLink->GetURL();
	wxString findlabel = m_pEditedLink->GetLabel();
	m_pLinkProp->m_textCtrlLinkDescription->SetValue(findlabel);
	m_pLinkProp->m_textCtrlLinkUrl->SetValue(findurl);
	if (m_pLinkProp->ShowModal() != wxID_OK) {
		event.Skip();
		return;
	}

	Hyperlinks& linklist = m_pRoutePoint->m_HyperlinkList;
	// FIXME: use find_if
	for (Hyperlinks::iterator i = linklist.begin(); i != linklist.end(); ++i) {
		if ((i->Link == findurl)
			&& ((i->DescrText == findlabel)
				|| ((i->Link == findlabel) && (i->DescrText == wxEmptyString)))) {
			i->Link = m_pLinkProp->m_textCtrlLinkUrl->GetValue();
			i->DescrText = m_pLinkProp->m_textCtrlLinkDescription->GetValue();
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

void MarkInfoImpl::OnAddLink(wxCommandEvent& event)
{
	m_pLinkProp->m_textCtrlLinkDescription->SetValue(wxEmptyString);
	m_pLinkProp->m_textCtrlLinkUrl->SetValue(wxEmptyString);
	if (m_pLinkProp->ShowModal() == wxID_OK) {
		wxString desc = m_pLinkProp->m_textCtrlLinkDescription->GetValue();
		if (desc == wxEmptyString)
			desc = m_pLinkProp->m_textCtrlLinkUrl->GetValue();

		add_hyperlink(desc, m_pLinkProp->m_textCtrlLinkUrl->GetValue());
		bSizerLinks->Fit(m_scrolledWindowLinks);
		this->Fit();

		m_pRoutePoint->m_HyperlinkList.push_back(
			Hyperlink(m_pLinkProp->m_textCtrlLinkDescription->GetValue(),
					  m_pLinkProp->m_textCtrlLinkUrl->GetValue(), wxEmptyString));
	}
	sbSizerLinks->Layout();
	event.Skip();
}

void MarkInfoImpl::OnEditLinkToggle(wxCommandEvent& event)
{
	if (m_toggleBtnEdit->GetValue())
		m_staticTextEditEnabled->SetLabel(_("Links are opened for editing."));
	else
		m_staticTextEditEnabled->SetLabel(_("Links are opened in the default browser."));
	event.Skip();
}

void MarkInfoImpl::OnDescChangedBasic(wxCommandEvent& event)
{
	if (m_panelBasicProperties->IsShownOnScreen())
		m_textCtrlExtDescription->ChangeValue(m_textDescription->GetValue());
	event.Skip();
}

void MarkInfoImpl::OnDescChangedExt(wxCommandEvent& event)
{
	if (m_panelDescription->IsShownOnScreen())
		m_textDescription->ChangeValue(m_textCtrlExtDescription->GetValue());
	event.Skip();
}

void MarkInfoImpl::OnExtDescriptionClick(wxCommandEvent& event)
{
	m_notebookProperties->SetSelection(1);
	event.Skip();
}

bool MarkInfoImpl::SaveChanges()
{
	if (!m_pRoutePoint)
		return true;

	if (m_pRoutePoint->m_bIsInLayer)
		return true;

	// Get User input Text Fields
	m_pRoutePoint->SetName(m_textName->GetValue());
	m_pRoutePoint->m_MarkDescription = m_textDescription->GetValue();
	m_pRoutePoint->SetVisible(m_checkBoxVisible->GetValue());
	m_pRoutePoint->SetNameShown(m_checkBoxShowName->GetValue());
	m_pRoutePoint->set_position(
		Position(fromDMM(m_textLatitude->GetValue()), fromDMM(m_textLongitude->GetValue())));
	m_pRoutePoint->m_IconName = pWayPointMan->GetIconKey(m_bcomboBoxIcon->GetSelection());
	m_pRoutePoint->ReLoadIcon();

	// Here is some logic....
	// If the Markname is completely numeric, and is part of a route,
	// Then declare it to be of attribute m_bDynamicName = true
	// This is later used for re-numbering points on actions like
	// Insert Point, Delete Point, Append Point, etc

	if (m_pRoutePoint->m_bIsInRoute) {
		bool b_name_is_numeric = true;
		for (unsigned int i = 0; i < m_pRoutePoint->GetName().Len(); i++) {
			if (wxChar('0') > m_pRoutePoint->GetName()[i])
				b_name_is_numeric = false;
			if (wxChar('9') < m_pRoutePoint->GetName()[i])
				b_name_is_numeric = false;
		}

		m_pRoutePoint->m_bDynamicName = b_name_is_numeric;
	} else
		m_pRoutePoint->m_bDynamicName = false;

	if (m_pRoutePoint->m_bIsInRoute) {
		// Update the route segment selectables
		pSelect->UpdateSelectableRouteSegments(m_pRoutePoint);

		// Get an array of all routes using this point
		Routeman::RouteArray* pEditRouteArray = g_pRouteMan->GetRouteArrayContaining(m_pRoutePoint);
		if (pEditRouteArray) {
			for (Routeman::RouteArray::iterator i = pEditRouteArray->begin();
				 i != pEditRouteArray->end(); ++i) {
				Route* route = static_cast<Route*>(*i);
				route->CalculateBBox();
				route->UpdateSegmentDistances();
				pConfig->UpdateRoute(route);
			}
			delete pEditRouteArray;
		}
	} else
		pConfig->UpdateWayPoint(m_pRoutePoint);

	// No general settings need be saved pConfig->UpdateSettings();
	return true;
}

void MarkInfoImpl::OnMarkInfoOKClick(wxCommandEvent& event)
{
	if (m_pRoutePoint) {
		OnPositionCtlUpdated(event);
		SaveChanges(); // write changes to globals and update config
		cc1->RefreshRect(m_pRoutePoint->CurrentRect_in_DC.Inflate(1000, 100), false);
	}
	Show(false);
	m_pMyLinkList.clear();

	if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
		pRouteManagerDialog->UpdateWptListCtrl();

	event.Skip();
}

void MarkInfoImpl::OnMarkInfoCancelClick(wxCommandEvent& event)
{
	if (m_pRoutePoint) {
		m_pRoutePoint->SetVisible(m_bIsVisible_save);
		m_pRoutePoint->SetNameShown(m_bShowName_save);
		m_pRoutePoint->set_position(Position(m_lat_save, m_lon_save));
		m_pRoutePoint->m_IconName = m_IconName_save;
		m_pRoutePoint->ReLoadIcon();

		m_pRoutePoint->m_HyperlinkList = m_pMyLinkList;
	}

	Show(false);
	m_pMyLinkList.clear();
	event.Skip();
}

void MarkInfoImpl::OnPositionCtlUpdated(wxCommandEvent&)
{
	if (!m_pRoutePoint->m_bIsInLayer) {
		// Fetch the control values, convert to degrees
		Position pos(fromDMM(m_textLatitude->GetValue()), fromDMM(m_textLongitude->GetValue()));
		m_pRoutePoint->set_position(pos);
		pSelect->ModifySelectablePoint(pos, (void*)m_pRoutePoint, SelectItem::TYPE_ROUTEPOINT);
	}

	// Update the mark position dynamically
	cc1->Refresh();
}

void MarkInfoImpl::OnRightClick(wxCommandEvent& event)
{
	wxMenu* popup = new wxMenu();
	popup->Append(ID_RCLK_MENU_COPY, _T("Copy"));
	popup->Append(ID_RCLK_MENU_COPY_LL, _T("Copy lat/long"));
	popup->Append(ID_RCLK_MENU_PASTE, _T("Paste"));
	popup->Append(ID_RCLK_MENU_PASTE_LL, _T("Paste lat/long"));
	m_contextObject = event.GetEventObject();
	PopupMenu(popup);
	delete popup;
}

void MarkInfoDef::OnCopyPasteLatLon(wxCommandEvent& event)
{
	// Fetch the control values, convert to degrees
	double lat = fromDMM(m_textLatitude->GetValue());
	double lon = fromDMM(m_textLongitude->GetValue());

	wxString result;

	switch (event.GetId()) {
		case ID_RCLK_MENU_PASTE:
			if (wxTheClipboard->Open()) {
				wxTextDataObject data;
				wxTheClipboard->GetData(data);
				result = data.GetText();
				((wxTextCtrl*)m_contextObject)->SetValue(result);
				wxTheClipboard->Close();
			}
			return;

		case ID_RCLK_MENU_PASTE_LL:
			if (wxTheClipboard->Open()) {
				wxTextDataObject data;
				wxTheClipboard->GetData(data);
				result = data.GetText();

				PositionParser pparse(result);

				if (pparse.IsOk()) {
					m_textLatitude->SetValue(pparse.GetLatitudeString());
					m_textLongitude->SetValue(pparse.GetLongitudeString());
				}
				wxTheClipboard->Close();
			}
			return;

		case ID_RCLK_MENU_COPY:
			result = ((wxTextCtrl*)m_contextObject)->GetValue();
			break;

		case ID_RCLK_MENU_COPY_LL:
			result << toSDMM(1, lat, true) << _T('\t');
			result << toSDMM(2, lon, true);
			break;
	}

	if (wxTheClipboard->Open()) {
		wxTextDataObject* data = new wxTextDataObject;
		data->SetText(result);
		wxTheClipboard->SetData(data);
		wxTheClipboard->Close();
	}
}

void MarkInfoImpl::OnHyperLinkClick(wxHyperlinkEvent& event)
{
	if (m_toggleBtnEdit->GetValue()) {
		m_pEditedLink = (wxHyperlinkCtrl*)event.GetEventObject();
		OnEditLink(event);
		event.Skip(false);
		return;
	}

#ifdef __WXMSW__
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

void MarkInfoImpl::ValidateMark(void)
{
	// Look in the master list of Waypoints to see if the currently selected waypoint is still valid
	// It may have been deleted as part of a route

	if (!pWayPointMan->contains(m_pRoutePoint))
		m_pRoutePoint = NULL;
}

