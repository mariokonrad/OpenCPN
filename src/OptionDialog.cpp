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

#include <wx/progdlg.h>
#include <wx/sound.h>
#include <wx/radiobox.h>
#include <wx/listbox.h>
#include <wx/imaglist.h>
#include <wx/display.h>
#include <wx/choice.h>
#include <wx/dirdlg.h>

#if defined(__WXX11__)
	#include <linux/X11FontPicker.h>
#else
	#include <wx/fontdlg.h>
#endif

#if wxCHECK_VERSION(2,9,4) /* does this work in 2.8 too.. do we need a test? */
	#include <wx/renderer.h>
#endif

#ifdef __WXGTK__
	#include <wx/colordlg.h>
#endif

#include "OptionDialog.h"
#include <MainFrame.h>
#include <MessageBox.h>
#include <DataStream.h>
#include <Multiplexer.h>
#include <NMEALogWindow.h>
#include <SentenceListDlg.h>
#include <ChartGroupsUI.h>
#include <OptionIDs.h>
#include <LanguageList.h>
#include <Config.h>
#include <SerialPorts.h>
#include <ChartCanvas.h>
#include <DimeControl.h>

#include <gui/FontManager.h>
#include <gui/StyleManager.h>
#include <gui/Style.h>

#include <ais/AIS_Decoder.h>
#include <ais/ais.h>

#include <chart/ChartDB.h>

#include <sound/OCPN_Sound.h>

#include <plugin/PluginListPanel.h>

#include <global/OCPN.h>
#include <global/GUI.h>
#include <global/System.h>
#include <global/AIS.h>
#include <global/Navigation.h>

#ifdef USE_S57
	#include <chart/s52plib.h>
	#include <chart/s52utils.h>
#endif

static wxString GetOCPNKnownLanguage(wxString lang_canonical, wxString *lang_dir);

extern MainFrame* gFrame;
extern ChartCanvas* cc1;

extern ArrayOfConnPrm* g_pConnectionParams;
extern Multiplexer* g_pMUX;

extern PlugInManager* g_pi_manager;

extern wxLocale *plocale_def_lang;
extern sound::OCPN_Sound g_anchorwatch_sound;


#ifdef USE_S57
extern chart::s52plib *ps52plib;
#endif

extern chart::ChartGroupArray* g_pGroupArray;

// Some constants
#define ID_CHOICE_NMEA  wxID_HIGHEST + 1

extern ais::AIS_Decoder* g_pAIS;
extern bool g_bserial_access_checked;

options* g_pOptions;

extern bool g_btouch;
extern bool g_bresponsive;

extern "C" bool CheckSerialAccess(void);

IMPLEMENT_DYNAMIC_CLASS(options, wxDialog)

// sort callback for Connections list  Sort by priority.
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortConnectionOnPriority(long item1, long item2, wxIntPtr list)
#else
int wxCALLBACK SortConnectionOnPriority(long item1, long item2, long list)
#endif
{
	wxListCtrl *lc = (wxListCtrl*)list;

	wxListItem it1, it2;
	it1.SetId(lc->FindItem(-1, item1));
	it1.SetColumn(3);
	it1.SetMask(it1.GetMask() | wxLIST_MASK_TEXT);

	it2.SetId(lc->FindItem(-1, item2));
	it2.SetColumn(3);
	it2.SetMask(it2.GetMask() | wxLIST_MASK_TEXT);

	lc->GetItem(it1);
	lc->GetItem(it2);

#ifdef __WXOSX__
	return it1.GetText().CmpNoCase(it2.GetText());
#else
	return it2.GetText().CmpNoCase(it1.GetText());
#endif
}


BEGIN_EVENT_TABLE(options, wxDialog)
	EVT_CHECKBOX(ID_DEBUGCHECKBOX1, options::OnDebugcheckbox1Click)
	EVT_BUTTON(ID_BUTTONADD, options::OnButtonaddClick)
	EVT_BUTTON(ID_BUTTONDELETE, options::OnButtondeleteClick)
	EVT_BUTTON(ID_TCDATAADD, options::OnInsertTideDataLocation)
	EVT_BUTTON(ID_TCDATADEL, options::OnRemoveTideDataLocation)
	EVT_BUTTON(ID_APPLY, options::OnApplyClick)
	EVT_BUTTON(xID_OK, options::OnXidOkClick)
	EVT_BUTTON(wxID_CANCEL, options::OnCancelClick)
	EVT_BUTTON(ID_BUTTONFONTCHOOSE, options::OnChooseFont)
#ifdef __WXGTK__
	EVT_BUTTON(ID_BUTTONFONTCOLOR, options::OnChooseFontColor)
#endif
	EVT_RADIOBOX(ID_RADARDISTUNIT, options::OnDisplayCategoryRadioButton)
	EVT_BUTTON(ID_CLEARLIST, options::OnButtonClearClick)
	EVT_BUTTON(ID_SELECTLIST, options::OnButtonSelectClick)
	EVT_BUTTON(ID_AISALERTSELECTSOUND, options::OnButtonSelectSound)
	EVT_BUTTON(ID_AISALERTTESTSOUND, options::OnButtonTestSound)
	EVT_CHECKBOX(ID_SHOWGPSWINDOW, options::OnShowGpsWindowCheckboxClick)
	EVT_CHECKBOX(ID_ZTCCHECKBOX, options::OnZTCCheckboxClick)
	EVT_CHOICE(ID_SHIPICONTYPE, options::OnShipTypeSelect)
	EVT_CHOICE(ID_RADARRINGS, options::OnRadarringSelect)
	EVT_CHAR_HOOK(options::OnCharHook)
END_EVENT_TABLE()


options::options()
{
	Init();
}

options::options(MainFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos,
				 const wxSize& size, long WXUNUSED(style))
{
	Init();

	pParent = parent;

	long wstyle = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;
	SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

	wxDialog::Create(parent, id, caption, pos, size, wstyle);

	CreateControls();
	Fit();
	Center();
}

options::~options()
{
	// Disconnect Events
	m_lcSources->Disconnect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
							wxListEventHandler(options::OnSelectDatasource), NULL, this);
	m_buttonAdd->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
							wxCommandEventHandler(options::OnAddDatasourceClick), NULL, this);
	m_buttonRemove->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
							   wxCommandEventHandler(options::OnRemoveDatasourceClick), NULL, this);
	m_tFilterSec->Disconnect(wxEVT_COMMAND_TEXT_UPDATED,
							 wxCommandEventHandler(options::OnValChange), NULL, this);
	m_rbTypeSerial->Disconnect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							   wxCommandEventHandler(options::OnTypeSerialSelected), NULL, this);
	m_rbTypeNet->Disconnect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							wxCommandEventHandler(options::OnTypeNetSelected), NULL, this);
	m_rbNetProtoTCP->Disconnect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
								wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
	m_rbNetProtoUDP->Disconnect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
								wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
	m_rbNetProtoGPSD->Disconnect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
								 wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
	m_tNetAddress->Disconnect(wxEVT_COMMAND_TEXT_UPDATED,
							  wxCommandEventHandler(options::OnValChange), NULL, this);
	m_tNetPort->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(options::OnValChange),
						   NULL, this);
	m_comboPort->Disconnect(wxEVT_COMMAND_COMBOBOX_SELECTED,
							wxCommandEventHandler(options::OnValChange), NULL, this);
	m_comboPort->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(options::OnValChange),
							NULL, this);
	m_choiceBaudRate->Disconnect(wxEVT_COMMAND_CHOICE_SELECTED,
								 wxCommandEventHandler(options::OnBaudrateChoice), NULL, this);
	m_choiceSerialProtocol->Disconnect(wxEVT_COMMAND_CHOICE_SELECTED,
									   wxCommandEventHandler(options::OnProtocolChoice), NULL,
									   this);
	m_choicePriority->Disconnect(wxEVT_COMMAND_CHOICE_SELECTED,
								 wxCommandEventHandler(options::OnValChange), NULL, this);
	m_cbCheckCRC->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED,
							 wxCommandEventHandler(options::OnCrcCheck), NULL, this);
	m_cbGarminHost->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED,
							   wxCommandEventHandler(options::OnUploadFormatChange), NULL, this);
	m_cbGarminUploadHost->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED,
									 wxCommandEventHandler(options::OnUploadFormatChange), NULL,
									 this);
	m_cbFurunoGP3X->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED,
							   wxCommandEventHandler(options::OnUploadFormatChange), NULL, this);
	m_rbIAccept->Disconnect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							wxCommandEventHandler(options::OnRbAcceptInput), NULL, this);
	m_rbIIgnore->Disconnect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							wxCommandEventHandler(options::OnRbIgnoreInput), NULL, this);
	m_tcInputStc->Disconnect(wxEVT_COMMAND_TEXT_UPDATED,
							 wxCommandEventHandler(options::OnValChange), NULL, this);
	m_btnInputStcList->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
								  wxCommandEventHandler(options::OnBtnIStcs), NULL, this);
	m_cbOutput->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED,
						   wxCommandEventHandler(options::OnCbOutput), NULL, this);
	m_rbOAccept->Disconnect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							wxCommandEventHandler(options::OnRbOutput), NULL, this);
	m_rbOIgnore->Disconnect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							wxCommandEventHandler(options::OnRbOutput), NULL, this);
	m_tcOutputStc->Disconnect(wxEVT_COMMAND_TEXT_UPDATED,
							  wxCommandEventHandler(options::OnValChange), NULL, this);
	m_btnOutputStcList->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
								   wxCommandEventHandler(options::OnBtnOStcs), NULL, this);
	m_cbNMEADebug->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED,
							  wxCommandEventHandler(options::OnShowGpsWindowCheckboxClick), NULL,
							  this);

	delete m_pSerialArray;
	groupsPanel->EmptyChartGroupArray(m_pGroupArray);
	delete m_pGroupArray;
	m_pGroupArray = NULL;
	m_groupsPage = NULL;
	g_pOptions = NULL;
	if (m_topImgList)
		delete m_topImgList;
}

void options::Init()
{
	m_pWorkDirList = NULL;

	pDebugShowStat = NULL;
	pSelCtl = NULL;
	pActiveChartsList = NULL;
	ps57CtlListBox = NULL;
	pDispCat = NULL;
	m_pSerialArray = NULL;
	pUpdateCheckBox = NULL;
	k_charts = 0;
	k_vectorcharts = 0;
	k_plugins = 0;
	k_tides = 0;

	activeSizer = NULL;
	itemActiveChartStaticBox = NULL;

	m_bVisitLang = false;
	m_itemFontElementListBox = NULL;
	m_topImgList = NULL;
	m_pSerialArray = EnumerateSerialPorts();

	m_pListbook = NULL;
	m_pGroupArray = NULL;
	m_groups_changed = 0;

	m_pageDisplay = -1;
	m_pageConnections = -1;
	m_pageCharts = -1;
	m_pageShips = -1;
	m_pageUI = -1;
	m_pagePlugins = -1;

	lastPage = -1;

	// This variable is used by plugin callback function AddOptionsPage
	g_pOptions = this;
}

bool options::Create(MainFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos,
					 const wxSize& size, long style)
{
	SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
	wxDialog::Create(parent, id, caption, pos, size, style);

	CreateControls();
	Fit();
	if (lastWindowPos == wxPoint(0, 0)) {
		Centre();
	} else {
		Move(lastWindowPos);
	}
	lastWindowPos = GetPosition();
	return TRUE;
}

wxWindow* options::GetContentWindow() const
{
	return NULL;
}

double options::get_double(const wxTextCtrl* text) const
{
	if (!text)
		return 0.0;

	double value = 0.0;
	text->GetValue().ToDouble(&value);
	return value;
}

size_t options::CreatePanel(const wxString& title)
{
	size_t id = m_pListbook->GetPageCount();
	// This is the default empty content for any top tab.
	// It'll be replaced when we call AddPage
	wxPanel* panel = new wxPanel(m_pListbook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
								 wxTAB_TRAVERSAL, title);
	m_pListbook->AddPage(panel, title, false, id);
	return id;
}

wxScrolledWindow* options::AddPage(size_t parent, const wxString& title)
{
	if (parent > m_pListbook->GetPageCount() - 1) {
		wxLogMessage(wxString::Format(_T("Warning: invalid parent in options::AddPage( %d, "),
									  parent) + title + _T(" )"));
		return NULL;
	}
	wxNotebookPage* page = m_pListbook->GetPage(parent);

	wxScrolledWindow* window;
	int style = wxVSCROLL | wxTAB_TRAVERSAL;
	if (page->IsKindOf(CLASSINFO(wxNotebook))) {
		window = new wxScrolledWindow(page, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);
		window->SetScrollRate(5, 5);
		((wxNotebook*)page)->AddPage(window, title);
	} else if (page->IsKindOf(CLASSINFO(wxScrolledWindow))) {
		wxString toptitle = m_pListbook->GetPageText(parent);
		wxNotebook* nb
			= new wxNotebook(m_pListbook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP);
		/* Only remove the tab from listbook, we still have original content in {page} */
		m_pListbook->RemovePage(parent);
		m_pListbook->InsertPage(parent, nb, toptitle, false, parent);
		wxString previoustitle = page->GetName();
		page->Reparent(nb);
		nb->AddPage(page, previoustitle);
		/* wxNotebookPage is hidden under wxGTK after RemovePage/Reparent
		 * we must explicitely Show() it */
		page->Show();
		window = new wxScrolledWindow(nb, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);
		window->SetScrollRate(5, 5);
		nb->AddPage(window, title);
		nb->ChangeSelection(0);
	} else { // This is the default content, we can replace it now
		window = new wxScrolledWindow(m_pListbook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
									  style, title);
		window->SetScrollRate(5, 5);
		wxString toptitle = m_pListbook->GetPageText(parent);
		m_pListbook->DeletePage(parent);
		m_pListbook->InsertPage(parent, window, toptitle, false, parent);
	}

	return window;
}

bool options::DeletePage(wxScrolledWindow* page)
{
	for (size_t i = 0; i < m_pListbook->GetPageCount(); i++) {
		wxNotebookPage* pg = m_pListbook->GetPage(i);

		if (pg->IsKindOf(CLASSINFO(wxNotebook))) {
			wxNotebook* nb = ((wxNotebook*)pg);
			for (size_t j = 0; j < nb->GetPageCount(); j++) {
				wxNotebookPage* spg = nb->GetPage(j);
				if (spg == page) {
					nb->DeletePage(j);
					if (nb->GetPageCount() == 1) {
						// There's only one page, remove inner notebook
						spg = nb->GetPage(0);
						spg->Reparent(m_pListbook);
						nb->RemovePage(0);
						wxString toptitle = m_pListbook->GetPageText(i);
						m_pListbook->DeletePage(i);
						m_pListbook->InsertPage(i, spg, toptitle, false, i);
					}
					return true;
				}
			}
		} else if (pg->IsKindOf(CLASSINFO(wxScrolledWindow)) && pg == page) {
			// There's only one page, replace it with empty panel
			m_pListbook->DeletePage(i);
			wxPanel* panel = new wxPanel(m_pListbook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
										 wxTAB_TRAVERSAL, _T(""));
			wxString toptitle = m_pListbook->GetPageText(i);
			m_pListbook->InsertPage(i, panel, toptitle, false, i);
			return true;
		}
	}
	return false;
}

void options::CreatePanel_NMEA(size_t parent, int WXUNUSED(border_size),
							   int WXUNUSED(group_item_spacing), wxSize WXUNUSED(small_button_size))
{
	m_pNMEAForm = AddPage(parent, _("NMEA"));

	wxBoxSizer* bSizer4 = new wxBoxSizer(wxVERTICAL);
	m_pNMEAForm->SetSizer(bSizer4);
	m_pNMEAForm->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* bSizerOuterContainer = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer* sbSizerGeneral;
	sbSizerGeneral
		= new wxStaticBoxSizer(new wxStaticBox(m_pNMEAForm, wxID_ANY, _("General")), wxVERTICAL);

	wxBoxSizer* bSizer151;
	bSizer151 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* bSizer161;
	bSizer161 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* bSizer171;
	bSizer171 = new wxBoxSizer(wxHORIZONTAL);

	m_cbFilterSogCog = new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Filter NMEA Course and Speed data"),
									  wxDefaultPosition, wxDefaultSize, 0);
	bSizer171->Add(m_cbFilterSogCog, 0, wxALL, 5);

	m_stFilterSec = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Filter period (sec)"),
									 wxDefaultPosition, wxDefaultSize, 0);
	m_stFilterSec->Wrap(-1);

	int nspace = 5;
#ifdef __WXGTK__
	nspace = 9;
#endif
	bSizer171->Add(m_stFilterSec, 0, wxALL, nspace);

	m_tFilterSec
		= new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer171->Add(m_tFilterSec, 0, wxALL, 4);

	bSizer161->Add(bSizer171, 1, wxEXPAND, 5);

	int cb_space = 2;
	m_cbNMEADebug = new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Show NMEA Debug Window"),
								   wxDefaultPosition, wxDefaultSize, 0);
	bSizer161->Add(m_cbNMEADebug, 0, wxALL, cb_space);

	const global::System::Config& cfg = global::OCPN::get().sys().config();

	m_cbFurunoGP3X = new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Format uploads for Furuno GP3X"),
									wxDefaultPosition, wxDefaultSize, 0);
	m_cbFurunoGP3X->SetValue(cfg.GPS_Ident == _T("FurunoGP3X"));
	bSizer161->Add(m_cbFurunoGP3X, 0, wxALL, cb_space);

	m_cbGarminUploadHost
		= new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Use Garmin GRMN (Host) mode for uploads"),
						 wxDefaultPosition, wxDefaultSize, 0);
	m_cbGarminUploadHost->SetValue(cfg.GarminHostUpload);
	bSizer161->Add(m_cbGarminUploadHost, 0, wxALL, cb_space);

	m_cbAPBMagnetic
		= new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Use magnetic bearings in output sentence ECAPB"),
						 wxDefaultPosition, wxDefaultSize, 0);
	m_cbAPBMagnetic->SetValue(global::OCPN::get().nav().get_data().MagneticAPB);
	bSizer161->Add(m_cbAPBMagnetic, 0, wxALL, cb_space);

	bSizer151->Add(bSizer161, 1, wxEXPAND, 5);
	sbSizerGeneral->Add(bSizer151, 1, wxEXPAND, 5);
	bSizerOuterContainer->Add(sbSizerGeneral, 0, wxALL | wxEXPAND, 5);

	// Connections listbox, etc
	wxStaticBoxSizer* sbSizerLB = new wxStaticBoxSizer(
		new wxStaticBox(m_pNMEAForm, wxID_ANY, _("Data Connections")), wxVERTICAL);

	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer(wxHORIZONTAL);

	m_lcSources = new wxListCtrl(m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxSize(-1, 150),
								 wxLC_REPORT | wxLC_SINGLE_SEL);
	bSizer17->Add(m_lcSources, 1, wxALL | wxEXPAND, 5);

	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer(wxVERTICAL);

	m_buttonAdd = new wxButton(m_pNMEAForm, wxID_ANY, _("Add Connection"), wxDefaultPosition,
							   wxDefaultSize, 0);
	bSizer18->Add(m_buttonAdd, 0, wxALL, 5);

	m_buttonRemove = new wxButton(m_pNMEAForm, wxID_ANY, _("Remove Connection"), wxDefaultPosition,
								  wxDefaultSize, 0);
	m_buttonRemove->Enable(false);
	bSizer18->Add(m_buttonRemove, 0, wxALL, 5);

	bSizer17->Add(bSizer18, 0, wxEXPAND, 5);
	sbSizerLB->Add(bSizer17, 1, wxEXPAND, 5);
	bSizerOuterContainer->Add(sbSizerLB, 0, wxEXPAND, 5);

	// Connections Properties

	sbSizerConnectionProps
		= new wxStaticBoxSizer(new wxStaticBox(m_pNMEAForm, wxID_ANY, _("Properties")), wxVERTICAL);

	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer(wxHORIZONTAL);

	m_rbTypeSerial = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Serial"), wxDefaultPosition,
									   wxDefaultSize, 0);
	m_rbTypeSerial->SetValue(true);
	bSizer15->Add(m_rbTypeSerial, 0, wxALL, 5);

	m_rbTypeNet = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Network"), wxDefaultPosition,
									wxDefaultSize, 0);
	bSizer15->Add(m_rbTypeNet, 0, wxALL, 5);

	sbSizerConnectionProps->Add(bSizer15, 0, wxEXPAND, 0);

	gSizerNetProps = new wxGridSizer(0, 2, 0, 0);

	m_stNetProto = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Protocol"), wxDefaultPosition,
									wxDefaultSize, 0);
	m_stNetProto->Wrap(-1);
	gSizerNetProps->Add(m_stNetProto, 0, wxALL, 5);

	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer(wxHORIZONTAL);

	m_rbNetProtoTCP = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("TCP"), wxDefaultPosition,
										wxDefaultSize, wxRB_GROUP);
	m_rbNetProtoTCP->Enable(true);

	bSizer16->Add(m_rbNetProtoTCP, 0, wxALL, 5);

	m_rbNetProtoUDP
		= new wxRadioButton(m_pNMEAForm, wxID_ANY, _("UDP"), wxDefaultPosition, wxDefaultSize, 0);
	m_rbNetProtoUDP->Enable(true);

	bSizer16->Add(m_rbNetProtoUDP, 0, wxALL, 5);

	m_rbNetProtoGPSD
		= new wxRadioButton(m_pNMEAForm, wxID_ANY, _("GPSD"), wxDefaultPosition, wxDefaultSize, 0);
	m_rbNetProtoGPSD->SetValue(true);
	bSizer16->Add(m_rbNetProtoGPSD, 0, wxALL, 5);

	gSizerNetProps->Add(bSizer16, 1, wxEXPAND, 5);

	m_stNetAddr = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Address"), wxDefaultPosition,
								   wxDefaultSize, 0);
	m_stNetAddr->Wrap(-1);
	gSizerNetProps->Add(m_stNetAddr, 0, wxALL, 5);

	m_tNetAddress
		= new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	gSizerNetProps->Add(m_tNetAddress, 0, wxEXPAND | wxTOP, 5);

	m_stNetPort = new wxStaticText(m_pNMEAForm, wxID_ANY, _("DataPort"), wxDefaultPosition,
								   wxDefaultSize, 0);
	m_stNetPort->Wrap(-1);
	gSizerNetProps->Add(m_stNetPort, 0, wxALL, 5);

	m_tNetPort
		= new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	gSizerNetProps->Add(m_tNetPort, 1, wxEXPAND | wxTOP, 5);

	sbSizerConnectionProps->Add(gSizerNetProps, 0, wxEXPAND, 5);

	gSizerSerProps = new wxGridSizer(0, 1, 0, 0);

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(0, 4, 0, 0);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_stSerPort = new wxStaticText(m_pNMEAForm, wxID_ANY, _("DataPort"), wxDefaultPosition,
								   wxDefaultSize, 0);
	m_stSerPort->Wrap(-1);
	fgSizer1->Add(m_stSerPort, 0, wxALL, 5);

	m_comboPort = new wxComboBox(m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition,
								 wxDefaultSize, 0, NULL, 0);
	fgSizer1->Add(m_comboPort, 0, wxEXPAND | wxTOP, 5);

	m_stSerBaudrate = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Baudrate"), wxDefaultPosition,
									   wxDefaultSize, 0);
	m_stSerBaudrate->Wrap(-1);
	fgSizer1->Add(m_stSerBaudrate, 0, wxALL, 5);

	wxString m_choiceBaudRateChoices[] = { _("150"),	_("300"),   _("600"),	_("1200"),
										   _("2400"),   _("4800"),  _("9600"),   _("19200"),
										   _("38400"),  _("57600"), _("115200"), _("230400"),
										   _("460800"), _("921600") };
	int m_choiceBaudRateNChoices = sizeof(m_choiceBaudRateChoices) / sizeof(wxString);
	m_choiceBaudRate = new wxChoice(m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize,
									m_choiceBaudRateNChoices, m_choiceBaudRateChoices, 0);
	m_choiceBaudRate->SetSelection(0);
	fgSizer1->Add(m_choiceBaudRate, 1, wxEXPAND | wxTOP, 5);

	m_stSerProtocol = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Protocol"), wxDefaultPosition,
									   wxDefaultSize, 0);
	m_stSerProtocol->Wrap(-1);
	fgSizer1->Add(m_stSerProtocol, 0, wxALL, 5);

	wxString m_choiceSerialProtocolChoices[] = { _("NMEA 0183"), _("NMEA 2000"), _("Seatalk") };
	int m_choiceSerialProtocolNChoices = sizeof(m_choiceSerialProtocolChoices) / sizeof(wxString);
	m_choiceSerialProtocol
		= new wxChoice(m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize,
					   m_choiceSerialProtocolNChoices, m_choiceSerialProtocolChoices, 0);
	m_choiceSerialProtocol->SetSelection(0);
	m_choiceSerialProtocol->Enable(false);

	fgSizer1->Add(m_choiceSerialProtocol, 1, wxEXPAND | wxTOP, 5);

	m_stPriority = new wxStaticText(m_pNMEAForm, wxID_ANY, _("Priority"), wxDefaultPosition,
									wxDefaultSize, 0);
	m_stPriority->Wrap(-1);
	fgSizer1->Add(m_stPriority, 0, wxALL, 5);

	wxString m_choicePriorityChoices[]
		= { _("0"), _("1"), _("2"), _("3"), _("4"), _("5"), _("6"), _("7"), _("8"), _("9") };
	int m_choicePriorityNChoices = sizeof(m_choicePriorityChoices) / sizeof(wxString);
	m_choicePriority = new wxChoice(m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize,
									m_choicePriorityNChoices, m_choicePriorityChoices, 0);
	m_choicePriority->SetSelection(9);
	fgSizer1->Add(m_choicePriority, 0, wxEXPAND | wxTOP, 5);

	gSizerSerProps->Add(fgSizer1, 0, wxEXPAND, 5);

	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	fgSizer5->SetFlexibleDirection(wxBOTH);
	fgSizer5->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_cbCheckCRC = new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Control checksum"), wxDefaultPosition,
								  wxDefaultSize, 0);
	m_cbCheckCRC->SetValue(true);
	m_cbCheckCRC->SetToolTip(
		_("If checked, only the sentences with a valid checksum are passed through"));
	fgSizer5->Add(m_cbCheckCRC, 0, wxALL, 5);

	m_cbGarminHost = new wxCheckBox(m_pNMEAForm, wxID_ANY, _("Use Garmin (GRMN) mode for input"),
									wxDefaultPosition, wxDefaultSize, 0);
	m_cbGarminHost->SetValue(false);
	fgSizer5->Add(m_cbGarminHost, 0, wxALL, 5);

	sbSizerConnectionProps->Add(gSizerSerProps, 0, wxEXPAND, 5);
	sbSizerConnectionProps->Add(fgSizer5, 0, wxEXPAND, 5);

	sbSizerInFilter = new wxStaticBoxSizer(
		new wxStaticBox(m_pNMEAForm, wxID_ANY, _("Input filtering")), wxVERTICAL);

	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer(wxHORIZONTAL);

	m_rbIAccept = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Accept only sentences"),
									wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	bSizer9->Add(m_rbIAccept, 0, wxALL, 5);

	m_rbIIgnore = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Ignore sentences"), wxDefaultPosition,
									wxDefaultSize, 0);
	bSizer9->Add(m_rbIIgnore, 0, wxALL, 5);

	sbSizerInFilter->Add(bSizer9, 0, wxEXPAND, 5);

	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer(wxHORIZONTAL);

	m_tcInputStc = new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition,
								  wxDefaultSize, wxTE_READONLY);
	bSizer11->Add(m_tcInputStc, 1, wxALL | wxEXPAND, 5);

	m_btnInputStcList = new wxButton(m_pNMEAForm, wxID_ANY, _("..."), wxDefaultPosition,
									 wxDefaultSize, wxBU_EXACTFIT);
	bSizer11->Add(m_btnInputStcList, 0, wxALL, 5);

	sbSizerInFilter->Add(bSizer11, 0, wxEXPAND, 5);

	sbSizerConnectionProps->Add(sbSizerInFilter, 0, wxEXPAND, 5);

	m_cbOutput = new wxCheckBox(m_pNMEAForm, wxID_ANY,
								_("Output on this port ( as Autopilot or NMEA Repeater)"),
								wxDefaultPosition, wxDefaultSize, 0);
	sbSizerConnectionProps->Add(m_cbOutput, 0, wxALL, 5);

	sbSizerOutFilter = new wxStaticBoxSizer(
		new wxStaticBox(m_pNMEAForm, wxID_ANY, _("Output filtering")), wxVERTICAL);

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer(wxHORIZONTAL);

	m_rbOAccept = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Transmit sentences"),
									wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	bSizer10->Add(m_rbOAccept, 0, wxALL, 5);

	m_rbOIgnore = new wxRadioButton(m_pNMEAForm, wxID_ANY, _("Drop sentences"), wxDefaultPosition,
									wxDefaultSize, 0);
	bSizer10->Add(m_rbOIgnore, 0, wxALL, 5);

	sbSizerOutFilter->Add(bSizer10, 0, wxEXPAND, 5);

	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer(wxHORIZONTAL);

	m_tcOutputStc = new wxTextCtrl(m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition,
								   wxDefaultSize, wxTE_READONLY);
	bSizer12->Add(m_tcOutputStc, 1, wxALL | wxEXPAND, 5);

	m_btnOutputStcList = new wxButton(m_pNMEAForm, wxID_ANY, _("..."), wxDefaultPosition,
									  wxDefaultSize, wxBU_EXACTFIT);
	bSizer12->Add(m_btnOutputStcList, 0, wxALL, 5);

	sbSizerOutFilter->Add(bSizer12, 0, wxEXPAND, 5);
	sbSizerConnectionProps->Add(sbSizerOutFilter, 0, wxEXPAND, 5);

	bSizerOuterContainer->Add(sbSizerConnectionProps, 1, wxALL | wxEXPAND, 5);

	bSizer4->Add(bSizerOuterContainer, 1, wxEXPAND, 5);

	// Connect Events
	m_lcSources->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED,
						 wxListEventHandler(options::OnSelectDatasource), NULL, this);
	m_buttonAdd->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
						 wxCommandEventHandler(options::OnAddDatasourceClick), NULL, this);
	m_buttonRemove->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							wxCommandEventHandler(options::OnRemoveDatasourceClick), NULL, this);
	m_rbTypeSerial->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							wxCommandEventHandler(options::OnTypeSerialSelected), NULL, this);
	m_rbTypeNet->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
						 wxCommandEventHandler(options::OnTypeNetSelected), NULL, this);
	m_rbNetProtoTCP->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							 wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
	m_rbNetProtoUDP->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							 wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
	m_rbNetProtoGPSD->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
							  wxCommandEventHandler(options::OnNetProtocolSelected), NULL, this);
	m_tNetAddress->Connect(wxEVT_COMMAND_TEXT_UPDATED,
						   wxCommandEventHandler(options::OnConnValChange), NULL, this);
	m_tNetPort->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(options::OnConnValChange),
						NULL, this);
	m_comboPort->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED,
						 wxCommandEventHandler(options::OnConnValChange), NULL, this);
	m_comboPort->Connect(wxEVT_COMMAND_TEXT_UPDATED,
						 wxCommandEventHandler(options::OnConnValChange), NULL, this);
	m_choiceBaudRate->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
							  wxCommandEventHandler(options::OnBaudrateChoice), NULL, this);
	m_choiceSerialProtocol->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
									wxCommandEventHandler(options::OnProtocolChoice), NULL, this);
	m_choicePriority->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
							  wxCommandEventHandler(options::OnConnValChange), NULL, this);
	m_cbCheckCRC->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
						  wxCommandEventHandler(options::OnCrcCheck), NULL, this);
	m_cbGarminHost->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
							wxCommandEventHandler(options::OnUploadFormatChange), NULL, this);
	m_cbGarminUploadHost->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
								  wxCommandEventHandler(options::OnUploadFormatChange), NULL, this);
	m_cbFurunoGP3X->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
							wxCommandEventHandler(options::OnUploadFormatChange), NULL, this);
	m_rbIAccept->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
						 wxCommandEventHandler(options::OnRbAcceptInput), NULL, this);
	m_rbIIgnore->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
						 wxCommandEventHandler(options::OnRbIgnoreInput), NULL, this);
	m_tcInputStc->Connect(wxEVT_COMMAND_TEXT_UPDATED,
						  wxCommandEventHandler(options::OnConnValChange), NULL, this);
	m_btnInputStcList->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
							   wxCommandEventHandler(options::OnBtnIStcs), NULL, this);
	m_cbOutput->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(options::OnCbOutput),
						NULL, this);
	m_rbOAccept->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
						 wxCommandEventHandler(options::OnRbOutput), NULL, this);
	m_rbOIgnore->Connect(wxEVT_COMMAND_RADIOBUTTON_SELECTED,
						 wxCommandEventHandler(options::OnRbOutput), NULL, this);
	m_tcOutputStc->Connect(wxEVT_COMMAND_TEXT_UPDATED,
						   wxCommandEventHandler(options::OnConnValChange), NULL, this);
	m_btnOutputStcList->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
								wxCommandEventHandler(options::OnBtnOStcs), NULL, this);

	m_cbNMEADebug->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
						   wxCommandEventHandler(options::OnShowGpsWindowCheckboxClick), NULL,
						   this);
	m_cbFilterSogCog->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
							  wxCommandEventHandler(options::OnValChange), NULL, this);
	m_tFilterSec->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(options::OnValChange),
						  NULL, this);
	m_cbAPBMagnetic->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
							 wxCommandEventHandler(options::OnValChange), NULL, this);
	m_lcSources->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(options::OnConnectionToggleEnable),
						 NULL, this);

	wxListItem col0;
	col0.SetId(0);
	col0.SetText(_("Enable"));
	m_lcSources->InsertColumn(0, col0);

	wxListItem col1;
	col1.SetId(1);
	col1.SetText(_("Type"));
	m_lcSources->InsertColumn(1, col1);

	wxListItem col2;
	col2.SetId(2);
	col2.SetText(_("DataPort"));
	m_lcSources->InsertColumn(2, col2);

	wxListItem col3;
	col3.SetId(3);
	col3.SetText(_("Priority"));
	m_lcSources->InsertColumn(3, col3);

	wxListItem col4;
	col4.SetId(4);
	col4.SetText(_("Parameters"));
	m_lcSources->InsertColumn(4, col4);

	wxListItem col5;
	col5.SetId(5);
	col5.SetText(_("Output"));
	m_lcSources->InsertColumn(5, col5);

	wxListItem col6;
	col6.SetId(6);
	col6.SetText(_("Filters"));
	m_lcSources->InsertColumn(6, col6);

	// Build the image list
	wxBitmap unchecked_bmp(16, 16), checked_bmp(16, 16);

	{
		wxMemoryDC renderer_dc;

		// Unchecked
		renderer_dc.SelectObject(unchecked_bmp);
		renderer_dc.SetBackground(
			*wxTheBrushList->FindOrCreateBrush(GetBackgroundColour(), wxSOLID));
		renderer_dc.Clear();
		wxRendererNative::Get().DrawCheckBox(this, renderer_dc, wxRect(0, 0, 16, 16), 0);

		// Checked
		renderer_dc.SelectObject(checked_bmp);
		renderer_dc.SetBackground(
			*wxTheBrushList->FindOrCreateBrush(GetBackgroundColour(), wxSOLID));
		renderer_dc.Clear();
		wxRendererNative::Get().DrawCheckBox(this, renderer_dc, wxRect(0, 0, 16, 16),
											 wxCONTROL_CHECKED);
	}

	wxImageList* imglist = new wxImageList(16, 16, true, 1);
	imglist->Add(unchecked_bmp);
	imglist->Add(checked_bmp);
	m_lcSources->AssignImageList(imglist, wxIMAGE_LIST_SMALL);

	m_lcSources->Refresh();

	m_stcdialog_in = new SentenceListDlg(ConnectionParams::FILTER_INPUT, this);
	m_stcdialog_out = new SentenceListDlg(ConnectionParams::FILTER_OUTPUT, this);

	FillSourceList();

	for (size_t i = 0; i < m_pSerialArray->Count(); i++) {
		m_comboPort->Append(m_pSerialArray->Item(i));
	}
	ShowNMEACommon(false);
	ShowNMEASerial(false);
	ShowNMEANet(false);
	connectionsaved = true;
}

void options::OnConnectionToggleEnable(wxMouseEvent& event)
{
	wxPoint pos = event.GetPosition();
	int flags = 0;
	long clicked_index = m_lcSources->HitTest(pos, flags);

	// Clicking Enable Checkbox (full column)?
	if (clicked_index > -1 && event.GetX() < m_lcSources->GetColumnWidth(0)) {
		// Process the clicked item
		ConnectionParams& conn = g_pConnectionParams->at(m_lcSources->GetItemData(clicked_index));
		conn.toggleEnabled();
		m_connection_enabled = conn.isEnabled();
		m_lcSources->SetItemImage(clicked_index, conn.isEnabled() ? 1 : 0);

		cc1->Refresh();
	} else if (clicked_index == -1) {
		ClearNMEAForm();
		m_buttonRemove->Enable(false);
	}

	// Allow wx to process...
	event.Skip();
}

void options::CreatePanel_Ownship(size_t parent, int border_size, int group_item_spacing,
								  wxSize WXUNUSED(small_button_size))
{
	itemPanelShip = AddPage(parent, _("Own Ship"));

	ownShip = new wxBoxSizer(wxVERTICAL);
	itemPanelShip->SetSizer(ownShip);

	//      OwnShip Display options
	wxStaticBox* osdBox = new wxStaticBox(itemPanelShip, wxID_ANY, _("Display Options"));
	dispOptions = new wxStaticBoxSizer(osdBox, wxVERTICAL);
	ownShip->Add(dispOptions, 0, wxTOP | wxALL | wxEXPAND, border_size);

	wxFlexGridSizer* dispOptionsGrid
		= new wxFlexGridSizer(2, 2, group_item_spacing, group_item_spacing);
	dispOptionsGrid->AddGrowableCol(1);
	dispOptions->Add(dispOptionsGrid, 0, wxALL | wxEXPAND, border_size);

	wxStaticText* pStatic_OSCOG_Predictor
		= new wxStaticText(itemPanelShip, wxID_ANY, _("COG Predictor Length (min)"));
	dispOptionsGrid->Add(pStatic_OSCOG_Predictor, 0);

	m_pText_OSCOG_Predictor = new wxTextCtrl(itemPanelShip, wxID_ANY);
	dispOptionsGrid->Add(m_pText_OSCOG_Predictor, 0, wxALIGN_RIGHT);

	wxStaticText* iconTypeTxt = new wxStaticText(itemPanelShip, wxID_ANY, _("Ship Icon Type"));
	dispOptionsGrid->Add(iconTypeTxt, 0);

	wxString iconTypes[] = { _("Default"), _("Real Scale Bitmap"), _("Real Scale Vector") };
	m_pShipIconType = new wxChoice(itemPanelShip, ID_SHIPICONTYPE, wxDefaultPosition, wxDefaultSize,
								   3, iconTypes);
	dispOptionsGrid->Add(m_pShipIconType, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT | wxTOP,
						 group_item_spacing);

	realSizes = new wxFlexGridSizer(5, 2, group_item_spacing, group_item_spacing);
	realSizes->AddGrowableCol(1);

	dispOptions->Add(realSizes, 0, wxEXPAND | wxLEFT, 30);

	realSizes->Add(new wxStaticText(itemPanelShip, wxID_ANY, _("Length Over All (m)")), 1,
				   wxALIGN_LEFT);
	m_pOSLength = new wxTextCtrl(itemPanelShip, 1);
	realSizes->Add(m_pOSLength, 1, wxALIGN_RIGHT | wxALL, group_item_spacing);

	realSizes->Add(new wxStaticText(itemPanelShip, wxID_ANY, _("Width Over All (m)")), 1,
				   wxALIGN_LEFT);
	m_pOSWidth = new wxTextCtrl(itemPanelShip, wxID_ANY);
	realSizes->Add(m_pOSWidth, 1, wxALIGN_RIGHT | wxALL, group_item_spacing);

	realSizes->Add(new wxStaticText(itemPanelShip, wxID_ANY, _("GPS Offset from Bow (m)")), 1,
				   wxALIGN_LEFT);
	m_pOSGPSOffsetY = new wxTextCtrl(itemPanelShip, wxID_ANY);
	realSizes->Add(m_pOSGPSOffsetY, 1, wxALIGN_RIGHT | wxALL, group_item_spacing);

	realSizes->Add(new wxStaticText(itemPanelShip, wxID_ANY, _("GPS Offset from Midship (m)")), 1,
				   wxALIGN_LEFT);
	m_pOSGPSOffsetX = new wxTextCtrl(itemPanelShip, wxID_ANY);
	realSizes->Add(m_pOSGPSOffsetX, 1, wxALIGN_RIGHT | wxALL, group_item_spacing);

	realSizes->Add(new wxStaticText(itemPanelShip, wxID_ANY, _("Minimum Screen Size (mm)")), 1,
				   wxALIGN_LEFT);
	m_pOSMinSize = new wxTextCtrl(itemPanelShip, wxID_ANY);
	realSizes->Add(m_pOSMinSize, 1, wxALIGN_RIGHT | wxALL, group_item_spacing);

	// Radar rings

	wxFlexGridSizer* rrSelect = new wxFlexGridSizer(1, 2, group_item_spacing, group_item_spacing);
	rrSelect->AddGrowableCol(1);
	dispOptions->Add(rrSelect, 0, wxLEFT | wxRIGHT | wxEXPAND, border_size);

	wxStaticText* rrTxt = new wxStaticText(itemPanelShip, wxID_ANY, _("Show radar rings"));
	rrSelect->Add(rrTxt, 1, wxEXPAND | wxALL, group_item_spacing);

	wxString rrAlt[] = { _("None"), _T("1"), _T("2"), _T("3"), _T("4"), _T("5"),
						 _T("6"),   _T("7"), _T("8"), _T("9"), _T("10") };
	pNavAidRadarRingsNumberVisible = new wxChoice(itemPanelShip, ID_RADARRINGS, wxDefaultPosition,
												  m_pShipIconType->GetSize(), 11, rrAlt);
	rrSelect->Add(pNavAidRadarRingsNumberVisible, 0, wxALIGN_RIGHT | wxALL, group_item_spacing);

	radarGrid = new wxFlexGridSizer(2, 2, group_item_spacing, group_item_spacing);
	radarGrid->AddGrowableCol(1);
	dispOptions->Add(radarGrid, 0, wxLEFT | wxEXPAND, 30);

	wxStaticText* distanceText
		= new wxStaticText(itemPanelShip, wxID_STATIC, _("Distance Between Rings"));
	radarGrid->Add(distanceText, 1, wxEXPAND | wxALL, group_item_spacing);

	pNavAidRadarRingsStep
		= new wxTextCtrl(itemPanelShip, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(100, -1), 0);
	radarGrid->Add(pNavAidRadarRingsStep, 0, wxALIGN_RIGHT | wxALL, group_item_spacing);

	wxStaticText* unitText = new wxStaticText(itemPanelShip, wxID_STATIC, _("Distance Unit"));
	radarGrid->Add(unitText, 1, wxEXPAND | wxALL, group_item_spacing);

	wxString pDistUnitsStrings[] = { _("Nautical Miles"), _("Kilometers") };
	m_itemRadarRingsUnits = new wxChoice(itemPanelShip, ID_RADARDISTUNIT, wxDefaultPosition,
										 m_pShipIconType->GetSize(), 2, pDistUnitsStrings);
	radarGrid->Add(m_itemRadarRingsUnits, 0, wxALIGN_RIGHT | wxALL, border_size);

	//  Tracks
	wxStaticBox* trackText = new wxStaticBox(itemPanelShip, wxID_ANY, _("Tracks"));
	wxStaticBoxSizer* trackSizer = new wxStaticBoxSizer(trackText, wxVERTICAL);
	ownShip->Add(trackSizer, 0, wxGROW | wxALL, border_size);

	pTrackDaily = new wxCheckBox(itemPanelShip, ID_DAILYCHECKBOX, _("Automatic Daily Tracks"));
	trackSizer->Add(pTrackDaily, 1, wxALL, border_size);

	pTrackHighlite = new wxCheckBox(itemPanelShip, ID_TRACKHILITE, _("Highlight Tracks"));
	trackSizer->Add(pTrackHighlite, 1, wxALL, border_size);

	wxFlexGridSizer* pTrackGrid = new wxFlexGridSizer(1, 2, group_item_spacing, group_item_spacing);
	pTrackGrid->AddGrowableCol(1);
	trackSizer->Add(pTrackGrid, 0, wxALL | wxEXPAND, border_size);

	wxStaticText* tpText = new wxStaticText(itemPanelShip, wxID_STATIC, _("Tracking Precision"));
	pTrackGrid->Add(tpText, 1, wxEXPAND | wxALL, group_item_spacing);

	wxString trackAlt[] = { _("Low"), _("Medium"), _("High") };
	pTrackPrecision = new wxChoice(itemPanelShip, wxID_ANY, wxDefaultPosition,
								   m_pShipIconType->GetSize(), 3, trackAlt);
	pTrackGrid->Add(pTrackPrecision, 0, wxALIGN_RIGHT | wxALL, group_item_spacing);

	//  Routes
	wxStaticBox* routeText = new wxStaticBox(itemPanelShip, wxID_ANY, _("Routes"));
	wxStaticBoxSizer* routeSizer = new wxStaticBoxSizer(routeText, wxVERTICAL);
	ownShip->Add(routeSizer, 0, wxGROW | wxALL, border_size);

	wxFlexGridSizer* pRouteGrid = new wxFlexGridSizer(1, 2, group_item_spacing, group_item_spacing);
	pRouteGrid->AddGrowableCol(1);
	routeSizer->Add(pRouteGrid, 0, wxALL | wxEXPAND, border_size);

	wxStaticText* raText
		= new wxStaticText(itemPanelShip, wxID_STATIC, _("Waypoint Arrival Circle Radius (NMi)"));
	pRouteGrid->Add(raText, 1, wxEXPAND | wxALL, group_item_spacing);

	m_pText_ACRadius = new wxTextCtrl(itemPanelShip, -1);
	pRouteGrid->Add(m_pText_ACRadius, 0, wxALL | wxALIGN_RIGHT, group_item_spacing);

	DimeControl(itemPanelShip);
}

void options::CreatePanel_ChartsLoad(size_t WXUNUSED(parent), int border_size,
									 int group_item_spacing, wxSize WXUNUSED(small_button_size))
{
	wxScrolledWindow* chartPanelWin = AddPage(m_pageCharts, _("Loaded Charts"));

	chartPanel = new wxBoxSizer(wxVERTICAL);
	chartPanelWin->SetSizer(chartPanel);

	wxStaticBox* loadedBox = new wxStaticBox(chartPanelWin, wxID_ANY, _("Directories"));
	activeSizer = new wxStaticBoxSizer(loadedBox, wxHORIZONTAL);
	chartPanel->Add(activeSizer, 1, wxALL | wxEXPAND, border_size);

	wxString* pListBoxStrings = NULL;
	pActiveChartsList = new wxListBox(chartPanelWin, ID_LISTBOX, wxDefaultPosition, wxDefaultSize,
									  0, pListBoxStrings, wxLB_MULTIPLE);

	activeSizer->Add(pActiveChartsList, 1, wxALL | wxEXPAND, border_size);

	wxBoxSizer* cmdButtonSizer = new wxBoxSizer(wxVERTICAL);
	activeSizer->Add(cmdButtonSizer, 0, wxALL, border_size);

	// Currently loaded chart dirs
	if (pActiveChartsList) {
		pActiveChartsList->Clear();
		for (ChartDirectories::const_iterator i = m_CurrentDirList.begin(); i != m_CurrentDirList.end();
			 ++i) {
			if (!i->fullpath.IsEmpty())
				pActiveChartsList->Append(i->fullpath);
		}
	}

	wxButton* addBtn = new wxButton(chartPanelWin, ID_BUTTONADD, _("Add Directory..."));
	cmdButtonSizer->Add(addBtn, 1, wxALL | wxEXPAND, group_item_spacing);

	wxButton* removeBtn = new wxButton(chartPanelWin, ID_BUTTONDELETE, _("Remove Selected"));
	cmdButtonSizer->Add(removeBtn, 1, wxALL | wxEXPAND, group_item_spacing);

	wxStaticBox* itemStaticBoxUpdateStatic
		= new wxStaticBox(chartPanelWin, wxID_ANY, _("Update Control"));
	wxStaticBoxSizer* itemStaticBoxSizerUpdate
		= new wxStaticBoxSizer(itemStaticBoxUpdateStatic, wxVERTICAL);
	chartPanel->Add(itemStaticBoxSizerUpdate, 0, wxGROW | wxALL, 5);

	pScanCheckBox
		= new wxCheckBox(chartPanelWin, ID_SCANCHECKBOX, _("Scan Charts and Update Database"));
	itemStaticBoxSizerUpdate->Add(pScanCheckBox, 1, wxALL, 5);

	pUpdateCheckBox
		= new wxCheckBox(chartPanelWin, ID_UPDCHECKBOX, _("Force Full Database Rebuild"));
	itemStaticBoxSizerUpdate->Add(pUpdateCheckBox, 1, wxALL, 5);

	chartPanel->Layout();
}

void options::CreatePanel_VectorCharts(size_t parent, int border_size, int group_item_spacing,
									   wxSize WXUNUSED(small_button_size))
{
	ps57Ctl = AddPage(parent, _("Vector Charts"));

	vectorPanel = new wxFlexGridSizer(2, 3, border_size, border_size);
	vectorPanel->AddGrowableCol(0, 1);

	ps57Ctl->SetSizer(vectorPanel);

	wxStaticBox* marinersBox = new wxStaticBox(ps57Ctl, wxID_ANY, _("Mariner's Standard"));
	wxStaticBoxSizer* marinersSizer = new wxStaticBoxSizer(marinersBox, wxVERTICAL);
	vectorPanel->Add(marinersSizer, 1, wxALL | wxEXPAND, border_size);

	wxString* ps57CtlListBoxStrings = NULL;
	ps57CtlListBox
		= new wxCheckListBox(ps57Ctl, ID_CHECKLISTBOX, wxDefaultPosition, wxSize(200, 250), 0,
							 ps57CtlListBoxStrings, wxLB_SINGLE | wxLB_HSCROLL | wxLB_SORT);
	marinersSizer->Add(ps57CtlListBox, 1, wxALL | wxEXPAND, group_item_spacing);

	wxBoxSizer* btnRow = new wxBoxSizer(wxHORIZONTAL);
	itemButtonSelectList = new wxButton(ps57Ctl, ID_SELECTLIST, _("Select All"));
	btnRow->Add(itemButtonSelectList, 0, wxALL, group_item_spacing);
	itemButtonClearList = new wxButton(ps57Ctl, ID_CLEARLIST, _("Clear All"));
	btnRow->Add(itemButtonClearList, 0, wxALL, group_item_spacing);
	marinersSizer->Add(btnRow);

	wxBoxSizer* catSizer = new wxBoxSizer(wxVERTICAL);
	vectorPanel->Add(catSizer, 1, wxALL | wxEXPAND, group_item_spacing);

	wxString pDispCatStrings[] = { _("Base"), _("Standard"), _("All"), _("Mariners Standard") };
	pDispCat = new wxRadioBox(ps57Ctl, ID_RADARDISTUNIT, _("Display Category"), wxDefaultPosition,
							  wxDefaultSize, 4, pDispCatStrings, 1, wxRA_SPECIFY_COLS);
	catSizer->Add(pDispCat, 0, wxALL | wxEXPAND, 2);

	pCheck_SOUNDG = new wxCheckBox(ps57Ctl, ID_SOUNDGCHECKBOX, _("Depth Soundings"));
	pCheck_SOUNDG->SetValue(FALSE);
	catSizer->Add(pCheck_SOUNDG, 1, wxALL | wxEXPAND, group_item_spacing);

	pCheck_META = new wxCheckBox(ps57Ctl, ID_METACHECKBOX, _("Chart Information Objects"));
	pCheck_META->SetValue(FALSE);
	catSizer->Add(pCheck_META, 1, wxALL | wxEXPAND, group_item_spacing);

	pCheck_SHOWIMPTEXT = new wxCheckBox(ps57Ctl, ID_IMPTEXTCHECKBOX, _("Important Text Only"));
	pCheck_SHOWIMPTEXT->SetValue(FALSE);
	catSizer->Add(pCheck_SHOWIMPTEXT, 1, wxALL | wxEXPAND, group_item_spacing);

	pCheck_SCAMIN = new wxCheckBox(ps57Ctl, ID_SCAMINCHECKBOX, _("Reduced Detail at Small Scale"));
	pCheck_SCAMIN->SetValue(FALSE);
	catSizer->Add(pCheck_SCAMIN, 1, wxALL | wxEXPAND, group_item_spacing);

	pCheck_ATONTEXT = new wxCheckBox(ps57Ctl, ID_ATONTEXTCHECKBOX, _("Buoy/Light Labels"));
	pCheck_SCAMIN->SetValue(FALSE);
	catSizer->Add(pCheck_ATONTEXT, 1, wxALL | wxEXPAND, group_item_spacing);

	pCheck_LDISTEXT = new wxCheckBox(ps57Ctl, ID_LDISTEXTCHECKBOX, _("Light Descriptions"));
	pCheck_LDISTEXT->SetValue(FALSE);
	catSizer->Add(pCheck_LDISTEXT, 1, wxALL | wxEXPAND, group_item_spacing);

	pCheck_XLSECTTEXT = new wxCheckBox(ps57Ctl, ID_LDISTEXTCHECKBOX, _("Extended Light Sectors"));
	pCheck_XLSECTTEXT->SetValue(FALSE);
	catSizer->Add(pCheck_XLSECTTEXT, 1, wxALL | wxEXPAND, group_item_spacing);

	pCheck_DECLTEXT = new wxCheckBox(ps57Ctl, ID_DECLTEXTCHECKBOX, _("De-Cluttered Text"));
	pCheck_DECLTEXT->SetValue(FALSE);
	catSizer->Add(pCheck_DECLTEXT, 1, wxALL | wxEXPAND, group_item_spacing);

	pCheck_NATIONALTEXT
		= new wxCheckBox(ps57Ctl, ID_NATIONALTEXTCHECKBOX, _("National text on chart"));
	pCheck_NATIONALTEXT->SetValue(FALSE);
	catSizer->Add(pCheck_NATIONALTEXT, 1, wxALL | wxEXPAND, group_item_spacing);

	wxBoxSizer* styleSizer = new wxBoxSizer(wxVERTICAL);
	vectorPanel->Add(styleSizer, 1, wxALL | wxEXPAND, 0);

	wxString pPointStyleStrings[] = { _("Paper Chart"), _("Simplified"), };
	pPointStyle = new wxRadioBox(ps57Ctl, ID_RADARDISTUNIT, _("Points"), wxDefaultPosition,
								 wxDefaultSize, 2, pPointStyleStrings, 1, wxRA_SPECIFY_COLS);
	styleSizer->Add(pPointStyle, 0, wxALL | wxEXPAND, group_item_spacing);

	wxString pBoundStyleStrings[] = { _("Plain"), _("Symbolized"), };
	pBoundStyle = new wxRadioBox(ps57Ctl, ID_RADARDISTUNIT, _("Boundaries"), wxDefaultPosition,
								 wxDefaultSize, 2, pBoundStyleStrings, 1, wxRA_SPECIFY_COLS);
	styleSizer->Add(pBoundStyle, 0, wxALL | wxEXPAND, group_item_spacing);

	wxString pColorNumStrings[] = { _("2 Color"), _("4 Color"), };
	p24Color = new wxRadioBox(ps57Ctl, ID_RADARDISTUNIT, _("Colors"), wxDefaultPosition,
							  wxDefaultSize, 2, pColorNumStrings, 1, wxRA_SPECIFY_COLS);
	styleSizer->Add(p24Color, 0, wxALL | wxEXPAND, group_item_spacing);

	wxStaticBox* depthBox = new wxStaticBox(ps57Ctl, wxID_ANY, _("Depth Settings(m)"));
	wxStaticBoxSizer* depthsSizer = new wxStaticBoxSizer(depthBox, wxVERTICAL);
	vectorPanel->Add(depthsSizer, 0, wxALL | wxEXPAND, border_size);

	wxStaticText* itemStaticText4 = new wxStaticText(ps57Ctl, wxID_STATIC, _("Shallow Depth"));
	depthsSizer->Add(itemStaticText4, 0, wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE,
					 group_item_spacing);

	m_ShallowCtl
		= new wxTextCtrl(ps57Ctl, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(120, -1), 0);
	depthsSizer->Add(m_ShallowCtl, 0, wxLEFT | wxRIGHT | wxBOTTOM, group_item_spacing);

	wxStaticText* itemStaticText5 = new wxStaticText(ps57Ctl, wxID_STATIC, _("Safety Depth"));
	depthsSizer->Add(itemStaticText5, 0, wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE,
					 group_item_spacing);

	m_SafetyCtl
		= new wxTextCtrl(ps57Ctl, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(120, -1), 0);
	depthsSizer->Add(m_SafetyCtl, 0, wxLEFT | wxRIGHT | wxBOTTOM, group_item_spacing);

	wxStaticText* itemStaticText6 = new wxStaticText(ps57Ctl, wxID_STATIC, _("Deep Depth"));
	depthsSizer->Add(itemStaticText6, 0, wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE,
					 group_item_spacing);

	m_DeepCtl = new wxTextCtrl(ps57Ctl, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(120, -1), 0);
	depthsSizer->Add(m_DeepCtl, 0, wxLEFT | wxRIGHT | wxBOTTOM, group_item_spacing);

	wxString pDepthUnitStrings[] = { _("Feet"), _("Meters"), _("Fathoms"), };

	pDepthUnitSelect
		= new wxRadioBox(ps57Ctl, ID_RADARDISTUNIT, _("Chart Depth Units"), wxDefaultPosition,
						 wxDefaultSize, 3, pDepthUnitStrings, 1, wxRA_SPECIFY_COLS);
	vectorPanel->Add(pDepthUnitSelect, 1, wxALL | wxEXPAND, border_size);

#ifdef USE_S57
#define CM93_ZOOM_FACTOR_MAX_RANGE 5 // FIXME: better solution (maybe over global infrastructure)
	wxStaticBox* cm93DetailBox = new wxStaticBox(ps57Ctl, wxID_ANY, _("CM93 Detail Level"));
	wxStaticBoxSizer* cm93Sizer = new wxStaticBoxSizer(cm93DetailBox, wxVERTICAL);
	m_pSlider_CM93_Zoom = new wxSlider(
		ps57Ctl, ID_CM93ZOOM, 0, -CM93_ZOOM_FACTOR_MAX_RANGE, CM93_ZOOM_FACTOR_MAX_RANGE,
		wxDefaultPosition, wxSize(140, 50), wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);
	cm93Sizer->Add(m_pSlider_CM93_Zoom, 0, wxALL | wxEXPAND, border_size);
	cm93Sizer->SetSizeHints(cm93DetailBox);
	vectorPanel->Add(cm93Sizer, 1, wxALL | wxEXPAND, border_size);
#endif
}

void options::CreatePanel_TidesCurrents(size_t parent, int border_size, int group_item_spacing,
										wxSize WXUNUSED(small_button_size))
{
	wxScrolledWindow* tcPanel = AddPage(parent, _("Tides && Currents"));

	wxBoxSizer* mainHBoxSizer = new wxBoxSizer(wxVERTICAL);
	tcPanel->SetSizer(mainHBoxSizer);

	wxStaticBox* tcBox = new wxStaticBox(tcPanel, wxID_ANY, _("Active Datasets"));
	wxStaticBoxSizer* tcSizer = new wxStaticBoxSizer(tcBox, wxHORIZONTAL);
	mainHBoxSizer->Add(tcSizer, 1, wxALL | wxEXPAND, border_size);

	tcDataSelected = new wxListBox(tcPanel, ID_TIDESELECTED, wxDefaultPosition, wxDefaultSize);

	tcSizer->Add(tcDataSelected, 1, wxALL | wxEXPAND, border_size);

	// Populate Selection List Control with the contents
	// of the Global static array
	const std::vector<wxString>& tide_dataset = global::OCPN::get().sys().data().current_tide_dataset;
	for (unsigned int id = 0; id < tide_dataset.size(); id++) {
		tcDataSelected->Append(tide_dataset.at(id));
	}

	// Add the "Insert/Remove" buttons
	wxButton* insertButton = new wxButton(tcPanel, ID_TCDATAADD, _("Add Dataset..."));
	wxButton* removeButton = new wxButton(tcPanel, ID_TCDATADEL, _("Remove Selected"));

	wxBoxSizer* btnSizer = new wxBoxSizer(wxVERTICAL);
	tcSizer->Add(btnSizer);

	btnSizer->Add(insertButton, 1, wxALL | wxEXPAND, group_item_spacing);
	btnSizer->Add(removeButton, 1, wxALL | wxEXPAND, group_item_spacing);
}

void options::CreatePanel_ChartGroups(size_t parent, int border_size, int group_item_spacing,
									  wxSize small_button_size)
{
	// Special case for adding the tab here. We know this page has multiple tabs,
	// and we have the actual widgets in a separate class (because of its complexity)

	m_groupsPage = m_pListbook->GetPage(parent);
	groupsPanel = new ChartGroupsUI(m_groupsPage);

	groupsPanel->CreatePanel(parent, border_size, group_item_spacing, small_button_size);
	((wxNotebook*)m_groupsPage)->AddPage(groupsPanel, _("Chart Groups"));

	m_groupsPage->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
						  wxListbookEventHandler(options::OnChartsPageChange), NULL, this);
}

void options::CreatePanel_Display(size_t parent, int border_size, int WXUNUSED(group_item_spacing),
								  wxSize WXUNUSED(small_button_size))
{
	wxScrolledWindow* itemPanelUI = AddPage(parent, _("Display"));

	wxBoxSizer* itemBoxSizerUI = new wxBoxSizer(wxVERTICAL);
	itemPanelUI->SetSizer(itemBoxSizerUI);

	// Chart Display Options Box
	wxStaticBox* itemStaticBoxSizerCDOStatic
		= new wxStaticBox(itemPanelUI, wxID_ANY, _("Chart Display Options"));
	wxStaticBoxSizer* itemStaticBoxSizerCDO
		= new wxStaticBoxSizer(itemStaticBoxSizerCDOStatic, wxVERTICAL);
	itemBoxSizerUI->Add(itemStaticBoxSizerCDO, 0, wxEXPAND | wxALL, border_size);

	// "Course Up" checkbox
	pCBCourseUp = new wxCheckBox(itemPanelUI, ID_COURSEUPCHECKBOX, _("Course UP Mode"));
	itemStaticBoxSizerCDO->Add(pCBCourseUp, 0, wxALL, border_size);

	// Course Up display update period
	wxFlexGridSizer* pCOGUPFilterGrid = new wxFlexGridSizer(2);
	pCOGUPFilterGrid->AddGrowableCol(1);
	itemStaticBoxSizerCDO->Add(pCOGUPFilterGrid, 0, wxALL | wxEXPAND, border_size);

	wxStaticText* itemStaticTextCOGUPFilterSecs = new wxStaticText(
		itemPanelUI, wxID_STATIC, _("Course-Up Mode Display Update Period (sec)"));
	pCOGUPFilterGrid->Add(itemStaticTextCOGUPFilterSecs, 0, wxADJUST_MINSIZE, border_size);

	pCOGUPUpdateSecs
		= new wxTextCtrl(itemPanelUI, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxDefaultSize);
	pCOGUPFilterGrid->Add(pCOGUPUpdateSecs, 0, wxALIGN_RIGHT | wxALL, border_size);

	// "LookAhead" checkbox
	pCBLookAhead = new wxCheckBox(itemPanelUI, ID_CHECK_LOOKAHEAD, _("Look Ahead Mode"));
	itemStaticBoxSizerCDO->Add(pCBLookAhead, 0, wxALL, border_size);

	// Grid display  checkbox
	pSDisplayGrid = new wxCheckBox(itemPanelUI, ID_CHECK_DISPLAYGRID, _("Show Grid"));
	itemStaticBoxSizerCDO->Add(pSDisplayGrid, 1, wxALL, border_size);

	// Depth Unit checkbox
	pSDepthUnits = new wxCheckBox(itemPanelUI, ID_SHOWDEPTHUNITSBOX1, _("Show Depth Units"));
	itemStaticBoxSizerCDO->Add(pSDepthUnits, 1, wxALL, border_size);

	// OpenGL Render checkbox
	pOpenGL = new wxCheckBox(itemPanelUI, ID_OPENGLBOX, _("Use Accelerated Graphics (OpenGL)"));
	itemStaticBoxSizerCDO->Add(pOpenGL, 1, wxALL, border_size);
	pOpenGL->Enable(!global::OCPN::get().gui().view().disable_opengl);

	// Smooth Pan/Zoom checkbox
	pSmoothPanZoom
		= new wxCheckBox(itemPanelUI, ID_SMOOTHPANZOOMBOX, _("Smooth Panning / Zooming"));
	itemStaticBoxSizerCDO->Add(pSmoothPanZoom, 1, wxALL, border_size);

	pEnableZoomToCursor = new wxCheckBox(itemPanelUI, ID_ZTCCHECKBOX, _("Zoom to Cursor"));
	pEnableZoomToCursor->SetValue(FALSE);
	itemStaticBoxSizerCDO->Add(pEnableZoomToCursor, 1, wxALL, border_size);

	pPreserveScale = new wxCheckBox(itemPanelUI, ID_PRESERVECHECKBOX,
									_("Preserve Scale when Switching Charts"));
	itemStaticBoxSizerCDO->Add(pPreserveScale, 1, wxALL, border_size);

	// Quilting checkbox
	pCDOQuilting = new wxCheckBox(itemPanelUI, ID_QUILTCHECKBOX1, _("Enable Chart Quilting"));
	itemStaticBoxSizerCDO->Add(pCDOQuilting, 1, wxALL, border_size);

	// Full Screen Quilting Disable checkbox
	pFullScreenQuilt
		= new wxCheckBox(itemPanelUI, ID_FULLSCREENQUILT, _("Disable Full Screen Quilting"));
	itemStaticBoxSizerCDO->Add(pFullScreenQuilt, 1, wxALL, border_size);

	// Chart Outlines checkbox
	pCDOOutlines = new wxCheckBox(itemPanelUI, ID_OUTLINECHECKBOX1, _("Show Chart Outlines"));
	itemStaticBoxSizerCDO->Add(pCDOOutlines, 1, wxALL, border_size);

	// Skewed Raster compenstation checkbox
	pSkewComp
		= new wxCheckBox(itemPanelUI, ID_SKEWCOMPBOX, _("Show Skewed Raster Charts as North-Up"));
	itemStaticBoxSizerCDO->Add(pSkewComp, 1, wxALL, border_size);

	// Mobile/Tochscreen checkbox
	pMobile = new wxCheckBox(itemPanelUI, ID_MOBILEBOX, _("Enable Touchscreen/Tablet interface"));
	itemStaticBoxSizerCDO->Add(pMobile, 1, wxALL, border_size);

	pResponsive
		= new wxCheckBox(itemPanelUI, ID_REPONSIVEBOX, _("Enable Responsive graphics interface"));
	itemStaticBoxSizerCDO->Add(pResponsive, 1, wxALL, border_size);

	// "Mag Heading" checkbox
	pCBMagShow
		= new wxCheckBox(itemPanelUI, ID_MAGSHOWCHECKBOX, _("Show Magnetic bearings and headings"));
	itemStaticBoxSizerCDO->Add(pCBMagShow, 0, wxALL, border_size);

	// Mag Heading user variation
	wxFlexGridSizer* pUserVarGrid = new wxFlexGridSizer(2);
	pUserVarGrid->AddGrowableCol(1);
	itemStaticBoxSizerCDO->Add(pUserVarGrid, 0, wxALL | wxEXPAND, border_size);

	wxStaticText* itemStaticTextUserVar
		= new wxStaticText(itemPanelUI, wxID_STATIC, _("Assumed Magnetic Variation, deg."));
	pUserVarGrid->Add(itemStaticTextUserVar, 0, wxADJUST_MINSIZE, border_size);

	pMagVar = new wxTextCtrl(itemPanelUI, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxDefaultSize);
	pUserVarGrid->Add(pMagVar, 0, wxALIGN_RIGHT | wxALL, border_size);
}

void options::CreatePanel_AIS(size_t parent, int border_size, int group_item_spacing,
							  wxSize small_button_size)
{
	wxScrolledWindow* panelAIS = AddPage(parent, _("AIS Targets"));

	wxBoxSizer* aisSizer = new wxBoxSizer(wxVERTICAL);
	panelAIS->SetSizer(aisSizer);

	//      CPA Box
	wxStaticBox* itemStaticBoxCPA = new wxStaticBox(panelAIS, wxID_ANY, _("CPA Calculation"));
	wxStaticBoxSizer* itemStaticBoxSizerCPA = new wxStaticBoxSizer(itemStaticBoxCPA, wxVERTICAL);
	aisSizer->Add(itemStaticBoxSizerCPA, 0, wxALL | wxEXPAND, border_size);

	wxFlexGridSizer* pCPAGrid = new wxFlexGridSizer(2);
	pCPAGrid->AddGrowableCol(1);
	itemStaticBoxSizerCPA->Add(pCPAGrid, 0, wxALL | wxEXPAND, border_size);

	m_pCheck_CPA_Max = new wxCheckBox(
		panelAIS, -1, _("No CPA Calculation if target range is greater than (NMi)"));
	pCPAGrid->Add(m_pCheck_CPA_Max, 0, wxALL, group_item_spacing);

	m_pText_CPA_Max = new wxTextCtrl(panelAIS, -1);
	pCPAGrid->Add(m_pText_CPA_Max, 0, wxALL | wxALIGN_RIGHT, group_item_spacing);

	m_pCheck_CPA_Warn = new wxCheckBox(panelAIS, -1, _("Warn if CPA less than (NMi)"));
	pCPAGrid->Add(m_pCheck_CPA_Warn, 0, wxALL, group_item_spacing);

	m_pText_CPA_Warn = new wxTextCtrl(panelAIS, -1, _T(""), wxDefaultPosition, wxSize(-1, -1));
	pCPAGrid->Add(m_pText_CPA_Warn, 0, wxALL | wxALIGN_RIGHT, group_item_spacing);

	m_pCheck_CPA_Warn->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
							   wxCommandEventHandler(options::OnCPAWarnClick), NULL, this);

	m_pCheck_CPA_WarnT = new wxCheckBox(panelAIS, -1, _("...and TCPA is less than (min)"));
	pCPAGrid->Add(m_pCheck_CPA_WarnT, 0, wxALL, group_item_spacing);

	m_pText_CPA_WarnT = new wxTextCtrl(panelAIS, -1);
	pCPAGrid->Add(m_pText_CPA_WarnT, 0, wxALL | wxALIGN_RIGHT, group_item_spacing);

	//      Lost Targets
	wxStaticBox* lostBox = new wxStaticBox(panelAIS, wxID_ANY, _("Lost Targets"));
	wxStaticBoxSizer* lostSizer = new wxStaticBoxSizer(lostBox, wxVERTICAL);
	aisSizer->Add(lostSizer, 0, wxALL | wxEXPAND, 3);

	wxFlexGridSizer* pLostGrid = new wxFlexGridSizer(2);
	pLostGrid->AddGrowableCol(1);
	lostSizer->Add(pLostGrid, 0, wxALL | wxEXPAND, border_size);

	m_pCheck_Mark_Lost = new wxCheckBox(panelAIS, -1, _("Mark targets as lost after (min)"));
	pLostGrid->Add(m_pCheck_Mark_Lost, 1, wxALL, group_item_spacing);

	m_pText_Mark_Lost = new wxTextCtrl(panelAIS, -1);
	pLostGrid->Add(m_pText_Mark_Lost, 1, wxALL | wxALIGN_RIGHT, group_item_spacing);

	m_pCheck_Remove_Lost = new wxCheckBox(panelAIS, -1, _("Remove lost targets after (min)"));
	pLostGrid->Add(m_pCheck_Remove_Lost, 1, wxALL, group_item_spacing);

	m_pText_Remove_Lost = new wxTextCtrl(panelAIS, -1);
	pLostGrid->Add(m_pText_Remove_Lost, 1, wxALL | wxALIGN_RIGHT, group_item_spacing);

	//      Display
	wxStaticBox* displBox = new wxStaticBox(panelAIS, wxID_ANY, _("Display"));
	wxStaticBoxSizer* displSizer = new wxStaticBoxSizer(displBox, wxHORIZONTAL);
	aisSizer->Add(displSizer, 0, wxALL | wxEXPAND, border_size);

	wxFlexGridSizer* pDisplayGrid = new wxFlexGridSizer(2);
	pDisplayGrid->AddGrowableCol(1);
	displSizer->Add(pDisplayGrid, 1, wxALL | wxEXPAND, border_size);

	m_pCheck_Show_COG
		= new wxCheckBox(panelAIS, -1, _("Show target COG predictor arrow, length (min)"));
	pDisplayGrid->Add(m_pCheck_Show_COG, 1, wxALL | wxEXPAND, group_item_spacing);

	m_pText_COG_Predictor = new wxTextCtrl(panelAIS, -1);
	pDisplayGrid->Add(m_pText_COG_Predictor, 1, wxALL | wxALIGN_RIGHT, group_item_spacing);

	m_pCheck_Show_Tracks = new wxCheckBox(panelAIS, -1, _("Show target tracks, length (min)"));
	pDisplayGrid->Add(m_pCheck_Show_Tracks, 1, wxALL, group_item_spacing);

	m_pText_Track_Length = new wxTextCtrl(panelAIS, -1);
	pDisplayGrid->Add(m_pText_Track_Length, 1, wxALL | wxALIGN_RIGHT, group_item_spacing);

	m_pCheck_Show_Moored
		= new wxCheckBox(panelAIS, -1, _("Hide anchored/moored targets, speed max (kn)"));
	pDisplayGrid->Add(m_pCheck_Show_Moored, 1, wxALL, group_item_spacing);

	m_pText_Moored_Speed = new wxTextCtrl(panelAIS, -1);
	pDisplayGrid->Add(m_pText_Moored_Speed, 1, wxALL | wxALIGN_RIGHT, group_item_spacing);

	m_pCheck_Show_Area_Notices
		= new wxCheckBox(panelAIS, -1, _("Show area notices (from AIS binary messages)"));
	pDisplayGrid->Add(m_pCheck_Show_Area_Notices, 1, wxALL, group_item_spacing);

	wxStaticText* pStatic_Dummy5 = new wxStaticText(panelAIS, -1, _T(""));
	pDisplayGrid->Add(pStatic_Dummy5, 1, wxALL, group_item_spacing);

	m_pCheck_Draw_Target_Size = new wxCheckBox(panelAIS, -1, _("Show AIS targets real size"));
	pDisplayGrid->Add(m_pCheck_Draw_Target_Size, 1, wxALL, group_item_spacing);

	wxStaticText* pStatic_Dummy6 = new wxStaticText(panelAIS, -1, _T(""));
	pDisplayGrid->Add(pStatic_Dummy6, 1, wxALL, group_item_spacing);

	m_pCheck_Show_Target_Name
		= new wxCheckBox(panelAIS, -1, _("Show names with AIS targets at scale greater than 1:"));
	pDisplayGrid->Add(m_pCheck_Show_Target_Name, 1, wxALL, group_item_spacing);

	m_pText_Show_Target_Name_Scale = new wxTextCtrl(panelAIS, -1);
	pDisplayGrid->Add(m_pText_Show_Target_Name_Scale, 1, wxALL | wxALIGN_RIGHT, group_item_spacing);

	m_pCheck_Wpl_Aprs
		= new wxCheckBox(panelAIS, -1, _("Treat WPL sentences as APRS position reports"));
	pDisplayGrid->Add(m_pCheck_Wpl_Aprs, 1, wxALL, group_item_spacing);

	wxStaticText* pStatic_Dummy7 = new wxStaticText(panelAIS, -1, _T(""));
	pDisplayGrid->Add(pStatic_Dummy7, 1, wxALL, group_item_spacing);

	wxStaticText* pStatic_Dummy5a = new wxStaticText(panelAIS, -1, _T(""));
	pDisplayGrid->Add(pStatic_Dummy5a, 1, wxALL, group_item_spacing);

	// Rollover
	wxStaticBox* rolloverBox = new wxStaticBox(panelAIS, wxID_ANY, _("Rollover"));
	wxStaticBoxSizer* rolloverSizer = new wxStaticBoxSizer(rolloverBox, wxVERTICAL);
	aisSizer->Add(rolloverSizer, 0, wxALL | wxEXPAND, border_size);

	wxStaticText* pStatic_Dummy4
		= new wxStaticText(panelAIS, -1, _("\"Ship Name\" MMSI (Call Sign)"));
	rolloverSizer->Add(pStatic_Dummy4, 1, wxALL, 2 * group_item_spacing);

	m_pCheck_Rollover_Class = new wxCheckBox(panelAIS, -1, _("[Class] Type (Status)"));
	rolloverSizer->Add(m_pCheck_Rollover_Class, 1, wxALL, 2 * group_item_spacing);

	m_pCheck_Rollover_COG = new wxCheckBox(panelAIS, -1, _("SOG COG"));
	rolloverSizer->Add(m_pCheck_Rollover_COG, 1, wxALL, 2 * group_item_spacing);

	m_pCheck_Rollover_CPA = new wxCheckBox(panelAIS, -1, _("CPA TCPA"));
	rolloverSizer->Add(m_pCheck_Rollover_CPA, 1, wxALL, 2 * group_item_spacing);

	//      Alert Box
	wxStaticBox* alertBox = new wxStaticBox(panelAIS, wxID_ANY, _("CPA/TCPA Alerts"));
	wxStaticBoxSizer* alertSizer = new wxStaticBoxSizer(alertBox, wxVERTICAL);
	aisSizer->Add(alertSizer, 0, wxALL | wxEXPAND, group_item_spacing);

	wxFlexGridSizer* pAlertGrid = new wxFlexGridSizer(2);
	pAlertGrid->AddGrowableCol(1);
	alertSizer->Add(pAlertGrid, 0, wxALL | wxEXPAND, group_item_spacing);

	m_pCheck_AlertDialog
		= new wxCheckBox(panelAIS, ID_AISALERTDIALOG, _("Show CPA/TCPA Alert Dialog"));
	pAlertGrid->Add(m_pCheck_AlertDialog, 0, wxALL, group_item_spacing);

	wxButton* m_SelSound = new wxButton(panelAIS, ID_AISALERTSELECTSOUND, _("Select Alert Sound"),
										wxDefaultPosition, small_button_size, 0);
	pAlertGrid->Add(m_SelSound, 0, wxALL | wxALIGN_RIGHT, group_item_spacing);

	m_pCheck_AlertAudio = new wxCheckBox(
		panelAIS, ID_AISALERTAUDIO, _("Play Sound on CPA/TCPA Alerts and DSC/SART emergencies."));
	pAlertGrid->Add(m_pCheck_AlertAudio, 0, wxALL, group_item_spacing);

	wxButton* m_pPlay_Sound = new wxButton(panelAIS, ID_AISALERTTESTSOUND, _("Test Alert Sound"),
										   wxDefaultPosition, small_button_size, 0);
	pAlertGrid->Add(m_pPlay_Sound, 0, wxALL | wxALIGN_RIGHT, group_item_spacing);

	m_pCheck_Alert_Moored
		= new wxCheckBox(panelAIS, -1, _("Supress Alerts for anchored/moored targets"));
	pAlertGrid->Add(m_pCheck_Alert_Moored, 1, wxALL, group_item_spacing);

	wxStaticText* pStatic_Dummy2 = new wxStaticText(panelAIS, -1, _T(""));
	pAlertGrid->Add(pStatic_Dummy2, 1, wxALL, group_item_spacing);

	m_pCheck_Ack_Timout
		= new wxCheckBox(panelAIS, -1, _("Enable Target Alert Acknowledge timeout (min)"));
	pAlertGrid->Add(m_pCheck_Ack_Timout, 1, wxALL, group_item_spacing);

	m_pText_ACK_Timeout = new wxTextCtrl(panelAIS, -1);
	pAlertGrid->Add(m_pText_ACK_Timeout, 1, wxALL | wxALIGN_RIGHT, group_item_spacing);

	panelAIS->Layout();
}

void options::CreatePanel_UI(size_t parent, int border_size, int WXUNUSED(group_item_spacing),
							 wxSize WXUNUSED(small_button_size))
{
	wxScrolledWindow* itemPanelFont = AddPage(parent, _("General Options"));

	m_itemBoxSizerFontPanel = new wxBoxSizer(wxVERTICAL);
	itemPanelFont->SetSizer(m_itemBoxSizerFontPanel);

	wxBoxSizer* langStyleBox = new wxBoxSizer(wxHORIZONTAL);
	m_itemBoxSizerFontPanel->Add(langStyleBox, 0, wxEXPAND | wxALL, border_size);

	wxStaticBox* itemLangStaticBox = new wxStaticBox(itemPanelFont, wxID_ANY, _("Language"));
	wxStaticBoxSizer* itemLangStaticBoxSizer = new wxStaticBoxSizer(itemLangStaticBox, wxVERTICAL);

	langStyleBox->Add(itemLangStaticBoxSizer, 1, wxEXPAND | wxALL, border_size);

	m_itemLangListBox = new wxChoice(itemPanelFont, ID_CHOICE_LANG);

	itemLangStaticBoxSizer->Add(m_itemLangListBox, 0, wxEXPAND | wxALL, border_size);

	wxStaticBox* itemFontStaticBox = new wxStaticBox(itemPanelFont, wxID_ANY, _("Fonts"));
	wxStaticBoxSizer* itemFontStaticBoxSizer
		= new wxStaticBoxSizer(itemFontStaticBox, wxHORIZONTAL);
	m_itemBoxSizerFontPanel->Add(itemFontStaticBoxSizer, 0, wxEXPAND | wxALL, border_size);

	m_itemFontElementListBox = new wxChoice(itemPanelFont, ID_CHOICE_FONTELEMENT);

	const gui::FontManager& fonts = global::OCPN::get().font();
	const global::System::Data& sys = global::OCPN::get().sys().data();
	int nFonts = fonts.GetNumFonts();
	for (int it = 0; it < nFonts; it++) {
		const wxString& t = fonts.GetDialogString(it);

		if (fonts.GetConfigString(it).StartsWith(sys.locale)) {
			m_itemFontElementListBox->Append(t);
		}
	}

	if (nFonts)
		m_itemFontElementListBox->SetSelection(0);

	itemFontStaticBoxSizer->Add(m_itemFontElementListBox, 0, wxALL, border_size);

	wxButton* itemFontChooseButton
		= new wxButton(itemPanelFont, ID_BUTTONFONTCHOOSE, _("Choose Font..."), wxDefaultPosition,
					   wxDefaultSize, 0);
	itemFontStaticBoxSizer->Add(itemFontChooseButton, 0, wxALL, border_size);
#ifdef __WXGTK__
	wxButton* itemFontColorButton
		= new wxButton(itemPanelFont, ID_BUTTONFONTCOLOR, _("Choose Font Color..."),
					   wxDefaultPosition, wxDefaultSize, 0);
	itemFontStaticBoxSizer->Add(itemFontColorButton, 0, wxALL, border_size);
#endif
	wxStaticBox* itemStyleStaticBox
		= new wxStaticBox(itemPanelFont, wxID_ANY, _("Toolbar and Window Style"));
	wxStaticBoxSizer* itemStyleStaticBoxSizer
		= new wxStaticBoxSizer(itemStyleStaticBox, wxVERTICAL);
	langStyleBox->Add(itemStyleStaticBoxSizer, 1, wxEXPAND | wxALL, border_size);

	m_itemStyleListBox = new wxChoice(itemPanelFont, ID_STYLESCOMBOBOX);

	const gui::StyleManager& styleman = global::OCPN::get().styleman();

	gui::StyleManager::StyleNames style_names = styleman.GetStyleNames();
	for (gui::StyleManager::StyleNames::const_iterator i = style_names.begin();
		 i != style_names.end(); ++i) {
		m_itemStyleListBox->Append(*i);
	}
	m_itemStyleListBox->SetStringSelection(styleman.current().getName());
	itemStyleStaticBoxSizer->Add(m_itemStyleListBox, 1, wxEXPAND | wxALL, border_size);

	wxStaticBox* miscOptionsBox
		= new wxStaticBox(itemPanelFont, wxID_ANY, _("Miscellaneous Options"));
	wxStaticBoxSizer* miscOptions = new wxStaticBoxSizer(miscOptionsBox, wxVERTICAL);
	m_itemBoxSizerFontPanel->Add(miscOptions, 0, wxALL | wxEXPAND, border_size);

	pDebugShowStat = new wxCheckBox(itemPanelFont, ID_DEBUGCHECKBOX1, _("Show Status Bar"));
	pDebugShowStat->SetValue(FALSE);
	miscOptions->Add(pDebugShowStat, 0, wxALL, border_size);

	pFullScreenToolbar
		= new wxCheckBox(itemPanelFont, ID_FSTOOLBARCHECKBOX, _("Show Toolbar in Fullscreen Mode"));
	miscOptions->Add(pFullScreenToolbar, 0, wxALL, border_size);

	pTransparentToolbar
		= new wxCheckBox(itemPanelFont, ID_TRANSTOOLBARCHECKBOX, _("Enable Transparent Toolbar"));
	miscOptions->Add(pTransparentToolbar, 0, wxALL, border_size);
	if (global::OCPN::get().gui().view().opengl)
		pTransparentToolbar->Disable();

	wxFlexGridSizer* pFormatGrid = new wxFlexGridSizer(2);
	pFormatGrid->AddGrowableCol(1);
	miscOptions->Add(pFormatGrid, 0, wxALL | wxEXPAND, border_size);

	wxStaticText* itemStaticTextSDMMFormat
		= new wxStaticText(itemPanelFont, wxID_STATIC, _("Show Lat/Long as"));
	pFormatGrid->Add(itemStaticTextSDMMFormat, 0, wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE,
					 border_size);

	wxString pSDMMFormats[]
		= { _("Degrees, Decimal Minutes"), _("Decimal Degrees"), _("Degrees, Minutes, Seconds") };
	int m_SDMMFormatsNChoices = sizeof(pSDMMFormats) / sizeof(wxString);
	pSDMMFormat = new wxChoice(itemPanelFont, ID_SDMMFORMATCHOICE, wxDefaultPosition, wxDefaultSize,
							   m_SDMMFormatsNChoices, pSDMMFormats);
	pFormatGrid->Add(pSDMMFormat, 0, wxALIGN_RIGHT, 2);

	wxStaticText* itemStaticTextDistanceFormat
		= new wxStaticText(itemPanelFont, wxID_STATIC, _("Show distance as"));
	pFormatGrid->Add(itemStaticTextDistanceFormat, 0, wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE,
					 border_size);

	wxString pDistanceFormats[]
		= { _("Nautical miles"), _("Statute miles"), _("Kilometers"), _("Meters") };
	int m_DistanceFormatsNChoices = sizeof(pDistanceFormats) / sizeof(wxString);
	pDistanceFormat = new wxChoice(itemPanelFont, ID_DISTANCEFORMATCHOICE, wxDefaultPosition,
								   wxDefaultSize, m_DistanceFormatsNChoices, pDistanceFormats);
	pFormatGrid->Add(pDistanceFormat, 0, wxALIGN_RIGHT, 2);

	wxStaticText* itemStaticTextSpeedFormat
		= new wxStaticText(itemPanelFont, wxID_STATIC, _("Show speed as"));
	pFormatGrid->Add(itemStaticTextSpeedFormat, 0, wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE,
					 border_size);

	wxString pSpeedFormats[] = { _("Knots"), _("Mph"), _("km/h"), _("m/s") };
	int m_SpeedFormatsNChoices = sizeof(pSpeedFormats) / sizeof(wxString);
	pSpeedFormat = new wxChoice(itemPanelFont, ID_SPEEDFORMATCHOICE, wxDefaultPosition,
								wxDefaultSize, m_SpeedFormatsNChoices, pSpeedFormats);
	pFormatGrid->Add(pSpeedFormat, 0, wxALIGN_RIGHT, 2);

	pPlayShipsBells = new wxCheckBox(itemPanelFont, ID_BELLSCHECKBOX, _("Play Ships Bells"));
	miscOptions->Add(pPlayShipsBells, 0, wxALIGN_LEFT | wxALL, border_size);

	pWayPointPreventDragging
		= new wxCheckBox(itemPanelFont, ID_DRAGGINGCHECKBOX,
						 _("Lock Waypoints (Unless waypoint property dialog visible)"));
	pWayPointPreventDragging->SetValue(FALSE);
	miscOptions->Add(pWayPointPreventDragging, 0, wxALL, border_size);

	pConfirmObjectDeletion = new wxCheckBox(itemPanelFont, ID_DELETECHECKBOX,
											_("Confirm deletion of tracks and routes"));
	pConfirmObjectDeletion->SetValue(FALSE);
	miscOptions->Add(pConfirmObjectDeletion, 0, wxALL, border_size);
}

void options::CreateControls()
{
	int border_size = 4;
	int check_spacing = 4;
	int group_item_spacing = 2; // use for items within one group, with Add(...wxALL)

	wxFont* qFont = GetOCPNScaledFont(_("Dialog"), 10);
	SetFont(*qFont);

	int font_size_y, font_descent, font_lead;
	GetTextExtent(_T("0"), NULL, &font_size_y, &font_descent, &font_lead);
	wxSize small_button_size(-1, (int)(1.4 * (font_size_y + font_descent + font_lead)));

	// Some members (pointers to controls) need to initialized
	pEnableZoomToCursor = NULL;
	pSmoothPanZoom = NULL;

	// Check the display size.
	// If "small", adjust some factors to squish out some more white space
	int width, height;
	::wxDisplaySize(&width, &height);

	if (!g_bresponsive) {
		if (height <= 800) {
			border_size = 2;
			check_spacing = 2;
			group_item_spacing = 1;

			wxFont* sFont = wxTheFontList->FindOrCreateFont(
				8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
			SetFont(*sFont);

			int font_size_y, font_descent, font_lead;
			GetTextExtent(_T("0"), NULL, &font_size_y, &font_descent, &font_lead);
			small_button_size = wxSize(-1, (int)(1.5 * (font_size_y + font_descent + font_lead)));
		}
	}

	options* itemDialog1 = this;

	wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
	itemDialog1->SetSizer(itemBoxSizer2);

	m_pListbook
		= new wxListbook(itemDialog1, ID_NOTEBOOK, wxDefaultPosition, wxSize(-1, -1), wxLB_TOP);

	// Reduce the Font size on ListBook(ListView) selectors to allow single line layout
	if (g_bresponsive) {
		wxListView* lv = m_pListbook->GetListView();
		wxFont* sFont = wxTheFontList->FindOrCreateFont(10, wxFONTFAMILY_DEFAULT,
														wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		lv->SetFont(*sFont);
	}

	m_topImgList = new wxImageList(40, 40, true, 1);
	gui::Style& style = global::OCPN::get().styleman().current();

#if wxCHECK_VERSION(2, 8, 12)
	m_topImgList->Add(style.GetIcon(_T("Display")));
	m_topImgList->Add(style.GetIcon(_T("Connections")));
	m_topImgList->Add(style.GetIcon(_T("Charts")));
	m_topImgList->Add(style.GetIcon(_T("Ship")));
	m_topImgList->Add(style.GetIcon(_T("UI")));
	m_topImgList->Add(style.GetIcon(_T("Plugins")));
#else
	wxBitmap bmp;
	wxImage img;
	bmp = style.GetIcon(_T("Display"));
	img = bmp.ConvertToImage();
	img.ConvertAlphaToMask(128);
	bmp = wxBitmap(img);
	m_topImgList->Add(bmp);
	bmp = style.GetIcon(_T("Connections"));
	img = bmp.ConvertToImage();
	img.ConvertAlphaToMask(128);
	bmp = wxBitmap(img);
	m_topImgList->Add(bmp);
	bmp = style.GetIcon(_T("Charts"));
	img = bmp.ConvertToImage();
	img.ConvertAlphaToMask(128);
	bmp = wxBitmap(img);
	m_topImgList->Add(bmp);
	bmp = style.GetIcon(_T("Ship"));
	img = bmp.ConvertToImage();
	img.ConvertAlphaToMask(128);
	bmp = wxBitmap(img);
	m_topImgList->Add(bmp);
	bmp = style.GetIcon(_T("UI"));
	img = bmp.ConvertToImage();
	img.ConvertAlphaToMask(128);
	bmp = wxBitmap(img);
	m_topImgList->Add(bmp);
	bmp = style.GetIcon(_T("Plugins"));
	img = bmp.ConvertToImage();
	img.ConvertAlphaToMask(128);
	bmp = wxBitmap(img);
	m_topImgList->Add(bmp);
#endif

	m_pListbook->SetImageList(m_topImgList);
	itemBoxSizer2->Add(m_pListbook, 1,
					   wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND,
					   border_size);

	wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
	itemBoxSizer2->Add(buttons, 0, wxALIGN_RIGHT | wxALL, border_size);

	m_OKButton = new wxButton(itemDialog1, xID_OK, _("Ok"));
	m_OKButton->SetDefault();
	buttons->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, border_size);

	m_CancelButton = new wxButton(itemDialog1, wxID_CANCEL, _("&Cancel"));
	buttons->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, border_size);

	m_ApplyButton = new wxButton(itemDialog1, ID_APPLY, _("Apply"));
	buttons->Add(m_ApplyButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, border_size);

	m_pageDisplay = CreatePanel(_("Display"));
	CreatePanel_Display(m_pageDisplay, border_size, group_item_spacing, small_button_size);

	m_pageConnections = CreatePanel(_("Connections"));
	CreatePanel_NMEA(m_pageConnections, border_size, group_item_spacing, small_button_size);

	m_pageCharts = CreatePanel(_("Charts"));
	CreatePanel_ChartsLoad(m_pageCharts, border_size, group_item_spacing, small_button_size);
	CreatePanel_VectorCharts(m_pageCharts, border_size, group_item_spacing, small_button_size);
	// ChartGroups must be created after ChartsLoad
	CreatePanel_ChartGroups(m_pageCharts, border_size, group_item_spacing, small_button_size);
	CreatePanel_TidesCurrents(m_pageCharts, border_size, group_item_spacing, small_button_size);

	m_pageShips = CreatePanel(_("Ships"));
	CreatePanel_Ownship(m_pageShips, border_size, group_item_spacing, small_button_size);
	CreatePanel_AIS(m_pageShips, border_size, group_item_spacing, small_button_size);

	m_pageUI = CreatePanel(_("User Interface"));
	CreatePanel_UI(m_pageUI, border_size, group_item_spacing, small_button_size);

	m_pagePlugins = CreatePanel(_("Plugins"));
	wxScrolledWindow* itemPanelPlugins = AddPage(m_pagePlugins, _("Plugins"));

	wxBoxSizer* itemBoxSizerPanelPlugins = new wxBoxSizer(wxVERTICAL);
	itemPanelPlugins->SetSizer(itemBoxSizerPanelPlugins);

	//      Build the PlugIn Manager Panel
	m_pPlugInCtrl = new PluginListPanel(itemPanelPlugins, ID_PANELPIM, wxDefaultPosition,
										wxDefaultSize, g_pi_manager->GetPlugInArray());
	m_pPlugInCtrl->SetScrollRate(15, 15);

	itemBoxSizerPanelPlugins->Add(m_pPlugInCtrl, 1, wxEXPAND | wxALL, border_size);

	//      PlugIns can add panels, too
	if (g_pi_manager)
		g_pi_manager->NotifySetupOptions();

	pSettingsCB1 = pDebugShowStat;

	SetColorScheme(static_cast<global::ColorScheme>(0));

	//  Update the PlugIn page to reflect the state of individual selections
	m_pPlugInCtrl->UpdateSelections();

	if (height < 768) {
		SetSizeHints(width - 200, height - 200, -1, -1);
	} else {
		vectorPanel->SetSizeHints(ps57Ctl);
	}

	m_pListbook->Connect(wxEVT_COMMAND_LISTBOOK_PAGE_CHANGED,
						 wxListbookEventHandler(options::OnPageChange), NULL, this);
}

void options::SetColorScheme(global::ColorScheme)
{
	DimeControl(this);
}

void options::SetInitialSettings()
{
	// ChartsLoad

	const global::System::Config& cfg = global::OCPN::get().sys().config();
	const global::GUI& gui = global::OCPN::get().gui();
	const global::GUI::View& view = global::OCPN::get().gui().view();
	const global::GUI::OwnShip& ownship = global::OCPN::get().gui().ownship();
	const global::AIS::Data& ais = global::OCPN::get().ais().get_data();
	const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
	const global::Navigation::Route& route = global::OCPN::get().nav().route();
	const global::Navigation::Track& track = global::OCPN::get().nav().get_track();

	for (ChartDirectories::const_iterator i = m_CurrentDirList.begin(); i != m_CurrentDirList.end();
		 ++i) {
		if (!i->fullpath.IsEmpty()) {
			if (pActiveChartsList) {
				pActiveChartsList->Append(i->fullpath);
			}
		}
	}

	// ChartGroups

	if (pActiveChartsList) {
		UpdateWorkArrayFromTextCtl();
		groupsPanel->SetDBDirs(*m_pWorkDirList);

		// Make a deep copy of the current global Group Array
		groupsPanel->EmptyChartGroupArray(m_pGroupArray);
		delete m_pGroupArray;
		m_pGroupArray = groupsPanel->CloneChartGroupArray(g_pGroupArray);
		groupsPanel->SetGroupArray(m_pGroupArray);
		groupsPanel->SetInitialSettings();
	}

	if (m_pConfig)
		pSettingsCB1->SetValue(m_pConfig->show_debug_windows());

	m_cbNMEADebug->SetValue(NMEALogWindow::Get().Active());
	m_cbFilterSogCog->SetValue(cfg.filter_cogsog);

	m_tFilterSec->SetValue(wxString::Format(_T("%d"), cfg.COGFilterSec));

	pCOGUPUpdateSecs->SetValue(wxString::Format(_T("%d"), nav.COGAvgSec));

	pCDOOutlines->SetValue(view.show_outlines);
	pCDOQuilting->SetValue(view.quilt_enable);
	pFullScreenQuilt->SetValue(!view.fullscreen_quilt);
	pSDepthUnits->SetValue(view.show_depth_units);
	pSkewComp->SetValue(view.skew_comp);
	pMobile->SetValue(g_btouch);
	pResponsive->SetValue(g_bresponsive);
	pOpenGL->SetValue(view.opengl);
	pSmoothPanZoom->SetValue(view.smooth_pan_zoom);
	if (view.enable_zoom_to_cursor || pEnableZoomToCursor->GetValue()) {
		pSmoothPanZoom->SetValue(false);
		pSmoothPanZoom->Disable();
	}

	m_cbAPBMagnetic->SetValue(nav.MagneticAPB);
	pCBMagShow->SetValue(view.ShowMag);

	pMagVar->SetValue(wxString::Format(_T("%4.1f"), nav.user_var));

	pSDisplayGrid->SetValue(view.display_grid);

	pCBCourseUp->SetValue(nav.CourseUp);
	pCBLookAhead->SetValue(view.lookahead_mode);

	if (fabs(wxRound(ownship.predictor_minutes) - ownship.predictor_minutes) > 1e-4)
		m_pText_OSCOG_Predictor->SetValue(
			wxString::Format(_T("%6.2f"), ownship.predictor_minutes));
	else
		m_pText_OSCOG_Predictor->SetValue(
			wxString::Format(_T("%4.0f"), ownship.predictor_minutes));

	m_pShipIconType->SetSelection(ownship.icon_type);
	wxCommandEvent eDummy;
	OnShipTypeSelect(eDummy);
	m_pOSLength->SetValue(wxString::Format(_T("%.1f"), ownship.length_meters));
	m_pOSWidth->SetValue(wxString::Format(_T("%.1f"), ownship.beam_meters));
	m_pOSGPSOffsetX->SetValue(wxString::Format(_T("%.1f"), ownship.gps_antenna_offset_x));
	m_pOSGPSOffsetY->SetValue(wxString::Format(_T("%.1f"), ownship.gps_antenna_offset_y));
	m_pOSMinSize->SetValue(wxString::Format(_T("%d"), ownship.min_mm));
	m_pText_ACRadius->SetValue(wxString::Format(_T("%.2f"), route.arrival_circle_radius));

	if (view.NavAidRadarRingsNumberVisible > 10) // FIXME: this is the wrong point to ensure range
		global::OCPN::get().gui().set_NavAidRadarRingsNumberVisible(10);
	pNavAidRadarRingsNumberVisible->SetSelection(view.NavAidRadarRingsNumberVisible);
	pNavAidRadarRingsStep->SetValue(wxString::Format(_T("%.3f"), view.NavAidRadarRingsStep));
	m_itemRadarRingsUnits->SetSelection(view.NavAidRadarRingsStepUnits);
	OnRadarringSelect(eDummy);

	pWayPointPreventDragging->SetValue(view.WayPointPreventDragging);
	pConfirmObjectDeletion->SetValue(view.ConfirmObjectDelete);

	pEnableZoomToCursor->SetValue(view.enable_zoom_to_cursor);
	if (pEnableZoomToCursor->GetValue()) {
		pSmoothPanZoom->Disable();
	} else {
		pSmoothPanZoom->Enable();
	}

	pPreserveScale->SetValue(view.preserve_scale_on_x);
	pPlayShipsBells->SetValue(cfg.PlayShipsBells);
	pFullScreenToolbar->SetValue(gui.toolbar().full_screen);
	pTransparentToolbar->SetValue(gui.toolbar().transparent);
	pSDMMFormat->Select(cfg.SDMMFormat);
	pDistanceFormat->Select(cfg.DistanceFormat);
	pSpeedFormat->Select(cfg.SpeedFormat);

	pTrackDaily->SetValue(track.TrackDaily);
	pTrackHighlite->SetValue(track.HighliteTracks);
	pTrackPrecision->SetSelection(track.TrackPrecision);

	// AIS Parameters
	// CPA Box
	m_pCheck_CPA_Max->SetValue(ais.CPAMax);

	m_pText_CPA_Max->SetValue(wxString::Format(_T("%4.1f"), ais.CPAMax_NM));
	m_pCheck_CPA_Warn->SetValue(ais.CPAWarn);
	m_pText_CPA_Warn->SetValue(wxString::Format(_T("%4.1f"), ais.CPAWarn_NM));
	m_pText_CPA_WarnT->SetValue(wxString::Format(_T("%4.0f"), ais.TCPA_Max_min));

	if (m_pCheck_CPA_Warn->GetValue()) {
		m_pCheck_CPA_WarnT->Enable();
		m_pCheck_CPA_WarnT->SetValue(ais.TCPA_Max);
	} else {
		m_pCheck_CPA_WarnT->Disable();
	}

	// Lost Targets
	m_pCheck_Mark_Lost->SetValue(ais.MarkLost);
	m_pText_Mark_Lost->SetValue(wxString::Format(_T("%4.0f"), ais.MarkLost_Mins));
	m_pCheck_Remove_Lost->SetValue(ais.RemoveLost);
	m_pText_Remove_Lost->SetValue(wxString::Format(_T("%4.0f"), ais.RemoveLost_Mins));

	// Display
	m_pCheck_Show_COG->SetValue(ais.ShowCOG);
	m_pText_COG_Predictor->SetValue(wxString::Format(_T("%4.0f"), ais.ShowCOG_Mins));
	m_pCheck_Show_Tracks->SetValue(ais.AISShowTracks);
	m_pText_Track_Length->SetValue(wxString::Format(_T("%4.0f"), ais.AISShowTracks_Mins));
	m_pCheck_Show_Moored->SetValue(!ais.ShowMoored);
	m_pText_Moored_Speed->SetValue(wxString::Format(_T("%4.1f"), ais.ShowMoored_Kts));
	m_pCheck_Show_Area_Notices->SetValue(ais.ShowAreaNotices);
	m_pCheck_Draw_Target_Size->SetValue(view.DrawAISSize);
	m_pCheck_Show_Target_Name->SetValue(view.ShowAISName);
	m_pText_Show_Target_Name_Scale->SetValue(wxString::Format(_T("%d"), view.Show_Target_Name_Scale));
	m_pCheck_Wpl_Aprs->SetValue(ais.WplIsAprsPosition);

	// Alerts
	m_pCheck_AlertDialog->SetValue(ais.AIS_CPA_Alert);
	m_pCheck_AlertAudio->SetValue(ais.AIS_CPA_Alert_Audio);
	m_pCheck_Alert_Moored->SetValue(ais.AIS_CPA_Alert_Suppress_Moored);
	m_pCheck_Ack_Timout->SetValue(ais.AIS_ACK_Timeout);
	m_pText_ACK_Timeout->SetValue(wxString::Format(_T("%4.0f"), ais.AckTimeout_Mins));

	// Rollover
	m_pCheck_Rollover_Class->SetValue(ais.AISRolloverShowClass);
	m_pCheck_Rollover_COG->SetValue(ais.AISRolloverShowCOG);
	m_pCheck_Rollover_CPA->SetValue(ais.AISRolloverShowCPA);

#ifdef USE_S57
	m_pSlider_CM93_Zoom->SetValue(gui.cm93().zoom_factor);

	// Diplay Category
	if (ps52plib) {

		// S52 Primary Filters
		ps57CtlListBox->Clear();
		marinersStdXref.clear();

		for (unsigned int iPtr = 0; iPtr < ps52plib->OBJLArray.size(); ++iPtr) {
			chart::OBJLElement* pOLE = ps52plib->OBJLArray.at(iPtr);
			wxString item;
			if (iPtr < ps52plib->OBJLDescriptions.size()) {
				item = ps52plib->OBJLDescriptions[iPtr];
			} else {
				item = wxString(pOLE->OBJLName, wxConvUTF8);
			}

			// The ListBox control will insert entries in sorted order, which means we need to
			// keep track of already inseted items that gets pushed down the line.
			int newpos = ps57CtlListBox->Append(item);
			marinersStdXref.push_back(newpos);
			for (size_t i = 0; i < iPtr; i++) {
				if (marinersStdXref[i] >= newpos)
					marinersStdXref[i]++;
			}

			ps57CtlListBox->Check(newpos, !(pOLE->nViz == 0));
		}

		int nset = 2; // default OTHER
		switch (ps52plib->m_nDisplayCategory) {
			case chart::DISPLAYBASE:
				nset = 0;
				break;
			case chart::STANDARD:
				nset = 1;
				break;
			case chart::OTHER:
				nset = 2;
				break;
			case chart::MARINERS_STANDARD:
				nset = 3;
				break;
			default:
				nset = 3;
				break;
		}

		pDispCat->SetSelection(nset);

		ps57CtlListBox->Enable(chart::MARINERS_STANDARD == ps52plib->m_nDisplayCategory);
		itemButtonClearList->Enable(chart::MARINERS_STANDARD == ps52plib->m_nDisplayCategory);
		itemButtonSelectList->Enable(chart::MARINERS_STANDARD == ps52plib->m_nDisplayCategory);

		// Other Display Filters
		pCheck_SOUNDG->SetValue(ps52plib->m_bShowSoundg);
		pCheck_META->SetValue(ps52plib->m_bShowMeta);
		pCheck_SHOWIMPTEXT->SetValue(ps52plib->m_bShowS57ImportantTextOnly);
		pCheck_SCAMIN->SetValue(ps52plib->m_bUseSCAMIN);
		pCheck_ATONTEXT->SetValue(ps52plib->m_bShowAtonText);
		pCheck_LDISTEXT->SetValue(ps52plib->m_bShowLdisText);
		pCheck_XLSECTTEXT->SetValue(ps52plib->m_bExtendLightSectors);
		pCheck_DECLTEXT->SetValue(ps52plib->m_bDeClutterText);
		pCheck_NATIONALTEXT->SetValue(ps52plib->m_bShowNationalTexts);

		// Chart Display Style
		if (ps52plib->m_nSymbolStyle == chart::PAPER_CHART)
			pPointStyle->SetSelection(0);
		else
			pPointStyle->SetSelection(1);

		if (ps52plib->m_nBoundaryStyle == chart::PLAIN_BOUNDARIES)
			pBoundStyle->SetSelection(0);
		else
			pBoundStyle->SetSelection(1);

		if (S52_getMarinerParam(chart::S52_MAR_TWO_SHADES) == 1.0)
			p24Color->SetSelection(0);
		else
			p24Color->SetSelection(1);

		m_SafetyCtl->SetValue(wxString::Format(
			_T("%6.2f"), chart::S52_getMarinerParam(chart::S52_MAR_SAFETY_CONTOUR)));
		m_ShallowCtl->SetValue(wxString::Format(
			_T("%6.2f"), chart::S52_getMarinerParam(chart::S52_MAR_SHALLOW_CONTOUR)));
		m_DeepCtl->SetValue(
			wxString::Format(_T("%6.2f"), chart::S52_getMarinerParam(chart::S52_MAR_DEEP_CONTOUR)));

		pDepthUnitSelect->SetSelection(ps52plib->m_nDepthUnitDisplay);
	}
#endif
}

void options::OnCPAWarnClick(wxCommandEvent&)
{
	if (m_pCheck_CPA_Warn->GetValue()) {
		m_pCheck_CPA_WarnT->Enable();
	} else {
		m_pCheck_CPA_WarnT->SetValue(false);
		m_pCheck_CPA_WarnT->Disable();
	}
}

void options::OnShowGpsWindowCheckboxClick(wxCommandEvent&)
{
	if (!m_cbNMEADebug->GetValue()) {
		NMEALogWindow::Get().DestroyWindow();
	} else {
		NMEALogWindow::Get().Create(pParent, 35);
		Raise();
	}
}

void options::OnZTCCheckboxClick(wxCommandEvent&)
{
	if (pEnableZoomToCursor->GetValue()) {
		pSmoothPanZoom->Disable();
	} else {
		pSmoothPanZoom->Enable();
	}
}

void options::OnShipTypeSelect(wxCommandEvent& event)
{
	if (m_pShipIconType->GetSelection() == 0) {
		realSizes->ShowItems(false);
	} else {
		realSizes->ShowItems(true);
	}
	dispOptions->Layout();
	ownShip->Layout();
	itemPanelShip->Layout();
	itemPanelShip->Refresh();
	event.Skip();
}

void options::OnRadarringSelect(wxCommandEvent& event)
{
	if (pNavAidRadarRingsNumberVisible->GetSelection() == 0) {
		radarGrid->ShowItems(false);
	} else {
		radarGrid->ShowItems(true);
	}
	dispOptions->Layout();
	ownShip->Layout();
	itemPanelShip->Layout();
	itemPanelShip->Refresh();
	event.Skip();
}

void options::OnDisplayCategoryRadioButton(wxCommandEvent& event)
{
	int select = pDispCat->GetSelection();

	if (3 == select) {
		ps57CtlListBox->Enable();
		itemButtonClearList->Enable();
		itemButtonSelectList->Enable();
	} else {
		ps57CtlListBox->Disable();
		itemButtonClearList->Disable();
		itemButtonSelectList->Disable();
	}

	event.Skip();
}

void options::OnButtonClearClick(wxCommandEvent& event)
{
	int nOBJL = ps57CtlListBox->GetCount();
	for (int iPtr = 0; iPtr < nOBJL; iPtr++)
		ps57CtlListBox->Check(iPtr, false);

	event.Skip();
}

void options::OnButtonSelectClick(wxCommandEvent& event)
{
	int nOBJL = ps57CtlListBox->GetCount();
	for (int iPtr = 0; iPtr < nOBJL; iPtr++)
		ps57CtlListBox->Check(iPtr, true);

	event.Skip();
}

bool options::ShowToolTips()
{
	return TRUE;
}

void options::OnCharHook(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_RETURN) {
		if (event.GetModifiers() == wxMOD_CONTROL) {
			wxCommandEvent okEvent;
			okEvent.SetId(xID_OK);
			okEvent.SetEventType(wxEVT_COMMAND_BUTTON_CLICKED);
			GetEventHandler()->AddPendingEvent(okEvent);
		}
	}
	event.Skip();
}

void options::OnButtonaddClick(wxCommandEvent& event)
{
	const global::System::Data& sys = global::OCPN::get().sys().data();
	const global::System::Config& cfg = global::OCPN::get().sys().config();

	wxDirDialog* dirSelector
		= new wxDirDialog(this, _("Add a directory containing chart files"), sys.init_chart_dir,
						  wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

	if (dirSelector->ShowModal() != wxID_CANCEL) {
		wxString selDir = dirSelector->GetPath();
		wxFileName dirname = wxFileName(selDir);

		if (!cfg.portable)
			global::OCPN::get().sys().set_init_chart_dir(dirname.GetPath());

		if (cfg.portable) {
			wxFileName f(selDir);
			f.MakeRelativeTo(sys.home_location);
			pActiveChartsList->Append(f.GetFullPath());
		} else {
			pActiveChartsList->Append(selDir);
		}

		k_charts |= CHANGE_CHARTS;

		pScanCheckBox->Disable();
	}

	delete dirSelector;
	event.Skip();
}

void options::UpdateDisplayedChartDirList(const ChartDirectories& p)
{
	if (pActiveChartsList) {
		pActiveChartsList->Clear();
		unsigned int nDir = p.size();
		for (unsigned int i = 0; i < nDir; i++) {
			wxString dirname = p.at(i).fullpath;
			if (!dirname.IsEmpty())
				pActiveChartsList->Append(dirname);
		}
	}
}

void options::UpdateWorkArrayFromTextCtl()
{
	wxString dirname;

	if (!m_pWorkDirList)
		return;

	m_pWorkDirList->clear();
	int n = pActiveChartsList->GetCount();
	for (int i = 0; i < n; i++) {
		dirname = pActiveChartsList->GetString(i);
		if (!dirname.IsEmpty()) {
			// This is a fix for OSX, which appends EOL to results of GetLineText()
			while ((dirname.Last() == wxChar(_T('\n'))) || (dirname.Last() == wxChar(_T('\r'))))
				dirname.RemoveLast();

			// scan the current array to find a match
			// if found, add the info to the work list, preserving the magic number
			// If not found, make a new chart directory information, and add it
			bool b_added = false;
			for (ChartDirectories::const_iterator dir = m_CurrentDirList.begin();
				 dir != m_CurrentDirList.end(); ++dir) {
				if (dir->fullpath == dirname) {
					m_pWorkDirList->push_back(*dir);
					b_added = true;
					break;
				}
			}
			if (!b_added) {
				m_pWorkDirList->push_back(ChartDirectoryInfo(dirname));
			}
		}
	}
}

ConnectionParams::ConnectionType options::getConParamConnectionType() const
{
	if (m_rbTypeSerial->GetValue())
		return ConnectionParams::SERIAL;
	return ConnectionParams::NETWORK;
}

ConnectionParams::NetworkProtocol options::getConParamNetworkProtocol() const
{
	if (m_rbNetProtoTCP->GetValue())
		return ConnectionParams::TCP;
	if (m_rbNetProtoUDP->GetValue())
		return ConnectionParams::UDP;
	return ConnectionParams::GPSD;
}

ConnectionParams::ListType options::getConParamInputListType() const
{
	if (m_rbIAccept->GetValue())
		return ConnectionParams::WHITELIST;
	return ConnectionParams::BLACKLIST;
}

ConnectionParams::ListType options::getConParamOutputListType() const
{
	if (m_rbOAccept->GetValue())
		return ConnectionParams::WHITELIST;
	return ConnectionParams::BLACKLIST;
}

ConnectionParams options::createConnectionParams() const
{
	return ConnectionParams(
		getConParamConnectionType(), getConParamNetworkProtocol(), m_tNetAddress->GetValue(),
		wxAtoi(m_tNetPort->GetValue()), wxAtoi(m_choiceBaudRate->GetStringSelection()),
		wxAtoi(m_choicePriority->GetStringSelection()), m_cbCheckCRC->GetValue(),
		m_cbGarminHost->GetValue(), m_tcInputStc->GetValue(), getConParamInputListType(),
		m_cbOutput->GetValue(), m_tcOutputStc->GetValue(), getConParamOutputListType(),
		m_comboPort->GetValue().BeforeFirst(' '), m_connection_enabled);
}

bool options::CreateConnectionParamsFromSelectedItem(ConnectionParams& prm)
{
	if (!m_bNMEAParams_shown)
		return false;

	// Special encoding for deleted connection
	if (m_rbTypeSerial->GetValue() && m_comboPort->GetValue() == _T("Deleted" ))
		return false;

	if (m_rbTypeSerial->GetValue() && m_comboPort->GetValue() == wxEmptyString) {
		wxMessageBox(_("You must select or enter the port..."), _("Error!"));
		return false;
	} else if (m_rbTypeNet->GetValue() && m_tNetAddress->GetValue() == wxEmptyString) {
		// TCP (I/O), GPSD (Input) and UDP (Output) ports require address field to be set
		if (m_rbNetProtoTCP->GetValue() || m_rbNetProtoGPSD->GetValue()
			|| (m_rbNetProtoUDP->GetValue() && m_cbOutput->GetValue())) {
			wxMessageBox(_("You must enter the address..."), _("Error!"));
			return false;
		}
	}

	prm = createConnectionParams();
	return true;
}

void options::OnApplyClick(wxCommandEvent& event)
{
	global::AIS& ais = global::OCPN::get().ais();
	global::GUI& gui = global::OCPN::get().gui();
	global::Navigation& nav = global::OCPN::get().nav();
	global::System& sys = global::OCPN::get().sys();

	::wxBeginBusyCursor();

	m_returnChanges = 0;

	// Start with the stuff that requires intelligent validation.

	if (m_pShipIconType->GetSelection() > 0) {
		double n_ownship_length_meters;
		double n_ownship_beam_meters;
		double n_gps_antenna_offset_y;
		double n_gps_antenna_offset_x;
		long n_ownship_min_mm;
		m_pOSLength->GetValue().ToDouble(&n_ownship_length_meters);
		m_pOSWidth->GetValue().ToDouble(&n_ownship_beam_meters);
		m_pOSGPSOffsetX->GetValue().ToDouble(&n_gps_antenna_offset_x);
		m_pOSGPSOffsetY->GetValue().ToDouble(&n_gps_antenna_offset_y);
		m_pOSMinSize->GetValue().ToLong(&n_ownship_min_mm);
		wxString msg;
		if (n_ownship_length_meters <= 0)
			msg += _("\n - your ship's length must be > 0");
		if (n_ownship_beam_meters <= 0)
			msg += _("\n - your ship's beam must be > 0");
		if (fabs(n_gps_antenna_offset_x) > n_ownship_beam_meters / 2.0)
			msg += _("\n - your GPS offset from midship must be within your ship's beam");
		if (n_gps_antenna_offset_y < 0 || n_gps_antenna_offset_y > n_ownship_length_meters)
			msg += _("\n - your GPS offset from bow must be within your ship's length");
		if (n_ownship_min_mm <= 0 || n_ownship_min_mm > 100)
			msg += _("\n - your minimum ship icon size must be between 1 and 100 mm");
		if (!msg.IsEmpty()) {
			msg.Prepend(_("The settings for own ship real size are not correct:"));
			OCPNMessageBox(this, msg, _("OpenCPN info"), wxICON_ERROR);
			::wxEndBusyCursor();
			event.SetInt(wxID_STOP);
			return;
		}
		gui.set_ownship_length_meters(n_ownship_length_meters);
		gui.set_ownship_beam_meters(n_ownship_beam_meters);
		gui.set_gps_antenna_offset_x(n_gps_antenna_offset_x);
		gui.set_gps_antenna_offset_y(n_gps_antenna_offset_y);
		gui.set_ownship_min_mm(n_ownship_min_mm);
	}
	gui.set_ownship_icon_type(m_pShipIconType->GetSelection());

	nav.set_route_arrival_circle_radius(get_double(m_pText_ACRadius));

	// Handle Chart Tab
	wxString dirname;

	if (pActiveChartsList) {
		UpdateWorkArrayFromTextCtl();
	} else {
		m_pWorkDirList->clear();
		for (ChartDirectories::const_iterator dir = m_CurrentDirList.begin();
			 dir != m_CurrentDirList.end(); ++dir) { // FIXME: use std::copy
			m_pWorkDirList->push_back(*dir);
		}
	}
	groupsPanel->SetDBDirs(*m_pWorkDirList); // update the Groups tab
	groupsPanel->m_treespopulated = false;

	int k_force = FORCE_UPDATE;
	if (pUpdateCheckBox) {
		if (!pUpdateCheckBox->GetValue())
			k_force = 0;
		pUpdateCheckBox->Enable();
		pUpdateCheckBox->SetValue(false);
	} else {
		k_force = 0;
	}

	m_returnChanges |= k_force;

	int k_scan = SCAN_UPDATE;
	if (pScanCheckBox) {
		if (!pScanCheckBox->GetValue())
			k_scan = 0;
		pScanCheckBox->Enable();
		pScanCheckBox->SetValue(false);
	} else {
		k_scan = 0;
	}

	m_returnChanges |= k_scan;

	// Chart Groups

	if (groupsPanel->modified) {
		groupsPanel->EmptyChartGroupArray(g_pGroupArray);
		delete g_pGroupArray;
		g_pGroupArray = groupsPanel->CloneChartGroupArray(m_pGroupArray);
		m_returnChanges |= GROUPS_CHANGED;
	}

	// Handle Settings Tab

	if (m_pConfig)
		m_pConfig->show_debug_windows(pSettingsCB1->GetValue());

	gui.set_view_show_outlines(pCDOOutlines->GetValue());
	gui.set_view_display_grid(pSDisplayGrid->GetValue());

	gui.set_view_quilt_enable(pCDOQuilting->GetValue());
	gui.set_view_fullscreen_quilt(!pFullScreenQuilt->GetValue());

	gui.set_view_show_depth_units(pSDepthUnits->GetValue());
	gui.set_skew_comp(pSkewComp->GetValue());
	g_btouch = pMobile->GetValue();
	g_bresponsive = pResponsive->GetValue();
	bool temp_bopengl = pOpenGL->GetValue();
	gui.set_smooth_pan_zoom(pSmoothPanZoom->GetValue());

	sys.set_config_filter_cogsog(m_cbFilterSogCog->GetValue());

	long filter_val = 1;
	m_tFilterSec->GetValue().ToLong(&filter_val);
	filter_val = wxMin(filter_val, MAX_COGSOG_FILTER_SECONDS);
	filter_val = wxMax(filter_val, 1);
	sys.set_config_COGFilterSec(filter_val);
	sys.set_config_SOGFilterSec(filter_val);

	long update_val = 1;
	pCOGUPUpdateSecs->GetValue().ToLong(&update_val);
	nav.set_COGAvgSec(wxMin(static_cast<int>(update_val), MAX_COG_AVERAGE_SECONDS));

	nav.set_CourseUp(pCBCourseUp->GetValue());
	gui.set_view_lookahead_mode(pCBLookAhead->GetValue());

	gui.set_ShowMag(pCBMagShow->GetValue());
	nav.set_user_var(get_double(pMagVar));
	nav.set_MagneticAPB(m_cbAPBMagnetic->GetValue());

	gui.set_ownship_predictor_minutes(get_double(m_pText_OSCOG_Predictor));

	gui.set_NavAidRadarRingsNumberVisible(pNavAidRadarRingsNumberVisible->GetSelection());
	gui.set_NavAidRadarRingsStep(atof(pNavAidRadarRingsStep->GetValue().mb_str()));
	gui.set_NavAidRadarRingsStepUnits(m_itemRadarRingsUnits->GetSelection());
	gui.set_WayPointPreventDragging(pWayPointPreventDragging->GetValue());
	gui.set_ConfirmObjectDelete(pConfirmObjectDeletion->GetValue());

	gui.set_view_preserve_scale_on_x(pPreserveScale->GetValue());

	sys.set_config_PlayShipsBells(pPlayShipsBells->GetValue());
	gui.set_toolbar_full_screen(pFullScreenToolbar->GetValue());
	gui.set_toolbar_transparent(pTransparentToolbar->GetValue());
	sys.set_config_SDMMFormat(pSDMMFormat->GetSelection());
	sys.set_config_DistanceFormat(pDistanceFormat->GetSelection());
	sys.set_config_SpeedFormat(pSpeedFormat->GetSelection());

	nav.set_TrackPrecision(pTrackPrecision->GetSelection());

	nav.set_TrackDaily(pTrackDaily->GetValue());
	nav.set_HighliteTracks(pTrackHighlite->GetValue());

	gui.set_enable_zoom_to_cursor(pEnableZoomToCursor->GetValue());

	// AIS Parameters

	// CPA Box
	ais.set_CPAMax(m_pCheck_CPA_Max->GetValue());
	ais.set_CPAMax_NM(get_double(m_pText_CPA_Max));
	ais.set_CPAWarn(m_pCheck_CPA_Warn->GetValue());
	ais.set_CPAWarn_NM(get_double(m_pText_CPA_Warn));
	ais.set_TCPA_Max(m_pCheck_CPA_WarnT->GetValue());
	ais.set_TCPA_Max_min(get_double(m_pText_CPA_WarnT));

	// Lost Targets
	ais.set_MarkLost(m_pCheck_Mark_Lost->GetValue());
	ais.set_MarkLost_Mins(get_double(m_pText_Mark_Lost));
	ais.set_RemoveLost(m_pCheck_Remove_Lost->GetValue());
	ais.set_RemoveLost_Mins(get_double(m_pText_Remove_Lost));

	// Display
	ais.set_ShowCOG(m_pCheck_Show_COG->GetValue());
	ais.set_ShowCOG_Mins(get_double(m_pText_COG_Predictor));
	ais.set_AISShowTracks(m_pCheck_Show_Tracks->GetValue());
	ais.set_AISShowTracks_Mins(get_double(m_pText_Track_Length));
	ais.set_ShowMoored(!m_pCheck_Show_Moored->GetValue());
	ais.set_ShowMoored_Kts(get_double(m_pText_Moored_Speed));
	ais.set_ShowAreaNotices(m_pCheck_Show_Area_Notices->GetValue());

	// Update all the current targets
	ais::AIS_Target_Hash::iterator it;
	ais::AIS_Target_Hash* current_targets = g_pAIS->GetTargetList();
	for (it = (*current_targets).begin(); it != (*current_targets).end(); ++it) {
		ais::AIS_Target_Data* pAISTarget = it->second;
		if (NULL != pAISTarget) {
			pAISTarget->b_show_track = ais.get_data().AISShowTracks;
		}
	}

	gui.set_DrawAISSize(m_pCheck_Draw_Target_Size->GetValue());
	gui.set_ShowAISName(m_pCheck_Show_Target_Name->GetValue());
	long ais_name_scale = 5000;
	m_pText_Show_Target_Name_Scale->GetValue().ToLong(&ais_name_scale);
	gui.set_Show_Target_Name_Scale(static_cast<int>(wxMax(5000, ais_name_scale)));

	ais.set_WplIsAprsPosition(m_pCheck_Wpl_Aprs->GetValue());

	// Alert
	ais.set_AIS_CPA_Alert(m_pCheck_AlertDialog->GetValue());
	ais.set_AIS_CPA_Alert_Audio(m_pCheck_AlertAudio->GetValue());
	ais.set_AIS_CPA_Alert_Suppress_Moored(m_pCheck_Alert_Moored->GetValue());
	ais.set_AIS_ACK_Timeout(m_pCheck_Ack_Timout->GetValue());
	ais.set_AckTimeout_Mins(get_double(m_pText_ACK_Timeout));

	// Rollover
	ais.set_AISRolloverShowClass(m_pCheck_Rollover_Class->GetValue());
	ais.set_AISRolloverShowCOG(m_pCheck_Rollover_COG->GetValue());
	ais.set_AISRolloverShowCPA(m_pCheck_Rollover_CPA->GetValue());

	// NMEA Source
	long itemIndex = m_lcSources->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	// If the stream selected exists, capture some of its existing parameters
	// to facility identification and allow stop and restart of the stream
	wxString lastAddr;
	int lastPort = 0;
	if (itemIndex >= 0) {
		int params_index = m_lcSources->GetItemData(itemIndex);
		const ConnectionParams& cpo = g_pConnectionParams->at(params_index);
		lastAddr = cpo.getNetworkAddress();
		lastPort = cpo.getNetworkPort();
	}

	if (!connectionsaved) {
		ConnectionParams cp;
		if (CreateConnectionParamsFromSelectedItem(cp)) {
			if (itemIndex >= 0) {
				int params_index = m_lcSources->GetItemData(itemIndex);
				g_pConnectionParams->at(params_index) = cp;
			} else {
				g_pConnectionParams->push_back(cp);
				itemIndex = g_pConnectionParams->size() - 1;
			}

			// Record the previous parameters, if any
			cp.set_last(lastAddr, lastPort);

			FillSourceList();
			m_lcSources->SetItemState(itemIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
			m_lcSources->Refresh();
			connectionsaved = true;
		}
	}

	// Recreate datastreams that are new, or have been edited
	for (size_t i = 0; i < g_pConnectionParams->size(); i++) {
		ConnectionParams& cp = g_pConnectionParams->at(i);
		if (!cp.isSetup()) { // Stream is new, or edited

			// Terminate and remove any existing stream with the same port name
			DataStream* pds_existing = g_pMUX->FindStream(cp.GetDSPort());
			if (pds_existing)
				g_pMUX->StopAndRemoveStream(pds_existing);

			// Try to stop any previous stream to avoid orphans
			pds_existing = g_pMUX->FindStream(cp.GetLastDSPort());
			if (pds_existing)
				g_pMUX->StopAndRemoveStream(pds_existing);

			if (cp.isEnabled()) {
				dsPortType port_type;
				if (cp.isOutput())
					port_type = DS_TYPE_INPUT_OUTPUT;
				else
					port_type = DS_TYPE_INPUT;
				DataStream* dstr = new DataStream(g_pMUX, cp.GetDSPort(),
												  wxString::Format(wxT("%i"), cp.getBaudrate()),
												  port_type, cp.getPriority(), cp.isGarmin());
				dstr->SetInputFilter(cp.getInputSentenceList());
				dstr->SetInputFilterType(cp.getInputSentenceListType());
				dstr->SetOutputFilter(cp.getOutputSentenceList());
				dstr->SetOutputFilterType(cp.getOutputSentenceListType());
				dstr->SetChecksumCheck(cp.isChecksumCheck());

				g_pMUX->AddStream(dstr);

				cp.set_setup(true);
			}
		}
	}

	sys.set_config_GarminHostUpload(m_cbGarminUploadHost->GetValue());
	if (m_cbFurunoGP3X->GetValue())
		sys.set_config_GPS_Ident(_T("FurunoGP3X"));
	else
		sys.set_config_GPS_Ident(_T("Generic"));

#ifdef USE_S57
	// Handle Vector Charts Tab

	gui.set_cm93_zoom_factor(m_pSlider_CM93_Zoom->GetValue());

	int nOBJL = ps57CtlListBox->GetCount();

	for (int iPtr = 0; iPtr < nOBJL; iPtr++) {
		int itemIndex = -1;
		for (size_t i = 0; i < marinersStdXref.size(); i++) {
			if (marinersStdXref[i] == iPtr) {
				itemIndex = i;
				break;
			}
		}
		chart::OBJLElement* pOLE = ps52plib->OBJLArray.at(itemIndex);
		pOLE->nViz = ps57CtlListBox->IsChecked(iPtr);
	}

	if (ps52plib) {
		if (temp_bopengl != gui.view().opengl) {
			// We need to do this now to handle the screen refresh that
			// is automatically generated on Windows at closure of the options dialog...
			ps52plib->FlushSymbolCaches();
			ps52plib->ClearCNSYLUPArray(); // some CNSY depends on renderer (e.g. CARC)
			ps52plib->GenerateStateHash();

			gui.set_opengl(temp_bopengl);
			m_returnChanges |= GL_CHANGED;
		}

		enum chart::DisCat nset = chart::OTHER;
		switch (pDispCat->GetSelection()) {
			case 0:
				nset = chart::DISPLAYBASE;
				break;
			case 1:
				nset = chart::STANDARD;
				break;
			case 2:
				nset = chart::OTHER;
				break;
			case 3:
				nset = chart::MARINERS_STANDARD;
				break;
		}
		ps52plib->m_nDisplayCategory = static_cast<chart::DisCat>(nset);

		ps52plib->m_bShowSoundg = pCheck_SOUNDG->GetValue();
		ps52plib->m_bShowMeta = pCheck_META->GetValue();
		ps52plib->m_bShowS57ImportantTextOnly = pCheck_SHOWIMPTEXT->GetValue();
		ps52plib->m_bUseSCAMIN = pCheck_SCAMIN->GetValue();
		ps52plib->m_bShowAtonText = pCheck_ATONTEXT->GetValue();
		ps52plib->m_bShowLdisText = pCheck_LDISTEXT->GetValue();
		ps52plib->m_bExtendLightSectors = pCheck_XLSECTTEXT->GetValue();
		ps52plib->m_bDeClutterText = pCheck_DECLTEXT->GetValue();
		ps52plib->m_bShowNationalTexts = pCheck_NATIONALTEXT->GetValue();

		if (0 == pPointStyle->GetSelection())
			ps52plib->m_nSymbolStyle = chart::PAPER_CHART;
		else
			ps52plib->m_nSymbolStyle = chart::SIMPLIFIED;

		if (0 == pBoundStyle->GetSelection())
			ps52plib->m_nBoundaryStyle = chart::PLAIN_BOUNDARIES;
		else
			ps52plib->m_nBoundaryStyle = chart::SYMBOLIZED_BOUNDARIES;

		if (0 == p24Color->GetSelection())
			S52_setMarinerParam(chart::S52_MAR_TWO_SHADES, 1.0);
		else
			S52_setMarinerParam(chart::S52_MAR_TWO_SHADES, 0.0);

		double dval;

		if ((m_SafetyCtl->GetValue()).ToDouble(&dval)) {
			// controls sounding display
			chart::S52_setMarinerParam(chart::S52_MAR_SAFETY_DEPTH, dval);
			chart::S52_setMarinerParam(chart::S52_MAR_SAFETY_CONTOUR, dval); // controls colour
		}

		if ((m_ShallowCtl->GetValue()).ToDouble(&dval))
			chart::S52_setMarinerParam(chart::S52_MAR_SHALLOW_CONTOUR, dval);

		if ((m_DeepCtl->GetValue()).ToDouble(&dval))
			chart::S52_setMarinerParam(chart::S52_MAR_DEEP_CONTOUR, dval);

		ps52plib->UpdateMarinerParams();
		ps52plib->m_nDepthUnitDisplay = pDepthUnitSelect->GetSelection();
		ps52plib->GenerateStateHash();
	}
#endif

	// User Interface Panel
	if (m_bVisitLang) {
		wxString new_canon = _T("en_US");
		wxString lang_sel = m_itemLangListBox->GetStringSelection();

		const int nLang = sizeof(LANGUAGE_LIST) / sizeof(LANGUAGE_LIST[0]);
		for (int it = 0; it < nLang; it++) {
			wxString lang_canonical = wxLocale::GetLanguageInfo(LANGUAGE_LIST[it])->CanonicalName;
			wxString test_string = GetOCPNKnownLanguage(lang_canonical, NULL);
			if (lang_sel == test_string) {
				new_canon = lang_canonical;
				break;
			}
		}

		global::System& sys = global::OCPN::get().sys();
		wxString locale_old = sys.data().locale;
		sys.set_locale(new_canon);

		if (sys.data().locale != locale_old)
			m_returnChanges |= LOCALE_CHANGED;

		gui::StyleManager& styleman = global::OCPN::get().styleman();

		wxString oldStyle = styleman.current().getName();
		styleman.SetStyleNextInvocation(m_itemStyleListBox->GetStringSelection());
		if (styleman.GetStyleNextInvocation() != oldStyle) {
			m_returnChanges |= STYLE_CHANGED;
		}
		wxSizeEvent nullEvent;
		gFrame->OnSize(nullEvent);
	}

	// PlugIn Manager Panel

	// Pick up any changes to selections
	bool bnew_settings = g_pi_manager->UpdatePlugIns();
	if (bnew_settings)
		m_returnChanges |= TOOLBAR_CHANGED;

	// And keep config in sync
	g_pi_manager->UpdateConfig();

	// PlugIns may have added panels
	if (g_pi_manager)
		g_pi_manager->CloseAllPlugInPanels((int)wxOK);

	// Could be a lot smarter here
	m_returnChanges |= GENERIC_CHANGED;
	m_returnChanges |= k_vectorcharts;
	m_returnChanges |= k_charts;
	m_returnChanges |= m_groups_changed;
	m_returnChanges |= k_plugins;
	m_returnChanges |= k_tides;

	// Pick up all the entries in the DataSelected control and update the global static array
	const int nEntry = tcDataSelected->GetCount();
	std::vector<wxString> tide_dataset;
	for (int i = 0; i < nEntry; i++) {
		tide_dataset.push_back(tcDataSelected->GetString(i));
	}
	global::OCPN::get().sys().set_current_tide_dataset(tide_dataset);

	if (event.GetId() == ID_APPLY) {
		gFrame->ProcessOptionsDialog(m_returnChanges, this);
		cc1->ReloadVP();
	}

	k_charts = k_charts & VISIT_CHARTS;
	::wxEndBusyCursor();
}

void options::OnXidOkClick(wxCommandEvent& event)
{
	// When closing the form with Ctrl-Enter sometimes we get double events, the second is empty??
	if (event.GetEventObject() == NULL)
		return;

	OnApplyClick(event);
	if (event.GetInt() == wxID_STOP)
		return;

	//  Required to avoid intermittent crash on wxGTK
	m_pListbook->ChangeSelection(0);
	for (size_t i = 0; i < m_pListbook->GetPageCount(); i++) {
		wxNotebookPage* pg = m_pListbook->GetPage(i);

		if (pg->IsKindOf(CLASSINFO(wxNotebook))) {
			wxNotebook* nb = ((wxNotebook*)pg);
			nb->ChangeSelection(0);
		}
	}

	delete pActiveChartsList;
	delete ps57CtlListBox;
	delete tcDataSelected;

	lastWindowPos = GetPosition();
	lastWindowSize = GetSize();
	EndModal(m_returnChanges);
}

void options::OnButtondeleteClick(wxCommandEvent& event)
{
	wxArrayInt pListBoxSelections;
	pActiveChartsList->GetSelections(pListBoxSelections);
	int nSelections = pListBoxSelections.GetCount();
	for (int i = 0; i < nSelections; i++) {
		pActiveChartsList->Delete(pListBoxSelections.Item((nSelections - i) - 1));
	}

	UpdateWorkArrayFromTextCtl();

	if (m_pWorkDirList) {
		pActiveChartsList->Clear();
		for (ChartDirectories::const_iterator dir = m_pWorkDirList->begin();
			 dir != m_pWorkDirList->end(); ++dir) {
			if (!dir->fullpath.IsEmpty()) {
				pActiveChartsList->Append(dir->fullpath);
			}
		}
	}

	k_charts |= CHANGE_CHARTS;
	pScanCheckBox->Disable();
	event.Skip();
}

void options::OnDebugcheckbox1Click(wxCommandEvent& event)
{
	event.Skip();
}

void options::OnCancelClick(wxCommandEvent&)
{
	// Required to avoid intermittent crash on wxGTK
	m_pListbook->ChangeSelection(0);
	delete pActiveChartsList;
	delete ps57CtlListBox;

	lastWindowPos = GetPosition();
	lastWindowSize = GetSize();
	EndModal(0);
}

void options::OnChooseFont(wxCommandEvent& event)
{
	wxString sel_text_element = m_itemFontElementListBox->GetStringSelection();

	wxFont* psfont;
	wxFontData font_data;

	gui::FontManager& fonts = global::OCPN::get().font();

	wxFont* pif = fonts.GetFont(sel_text_element);
	wxColour init_color = fonts.GetFontColor(sel_text_element);

	wxFontData init_font_data;
	if (pif)
		init_font_data.SetInitialFont(*pif);
	init_font_data.SetColour(init_color);

#ifdef __WXX11__
	X11FontPicker dg(pParent, init_font_data);
#else
	wxFontDialog dg(pParent, init_font_data);
#endif
	int retval = dg.ShowModal();
	if (wxID_CANCEL != retval) {
		font_data = dg.GetFontData();
		wxFont font = font_data.GetChosenFont();
		psfont = new wxFont(font);
		wxColor color = font_data.GetColour();
		fonts.SetFont(sel_text_element, psfont, color);

		pParent->UpdateAllFonts();
	}

	event.Skip();
}

#ifdef __WXGTK__
void options::OnChooseFontColor(wxCommandEvent& event)
{
	wxString sel_text_element = m_itemFontElementListBox->GetStringSelection();

	wxColourData colour_data;

	gui::FontManager& fonts = global::OCPN::get().font();

	wxFont* pif = fonts.GetFont(sel_text_element);
	wxColour init_color = fonts.GetFontColor(sel_text_element);

	wxColourData init_colour_data;
	init_colour_data.SetColour(init_color);

	wxColourDialog dg(pParent, &init_colour_data);

	int retval = dg.ShowModal();
	if (wxID_CANCEL != retval) {
		colour_data = dg.GetColourData();

		wxColor color = colour_data.GetColour();
		fonts.SetFont(sel_text_element, pif, color);

		pParent->UpdateAllFonts();
	}

	event.Skip();
}
#endif

void options::OnChartsPageChange(wxListbookEvent& event)
{
	unsigned int i = event.GetSelection();

	// User selected Chart Groups Page?
	// If so, build the remaining UI elements
	if (2 == i) { // 2 is the index of "Chart Groups" page
		if (!groupsPanel->m_UIcomplete)
			groupsPanel->CompletePanel();

		if (!groupsPanel->m_settingscomplete) {
			::wxBeginBusyCursor();
			groupsPanel->CompleteInitialSettings();
			::wxEndBusyCursor();
		} else if (!groupsPanel->m_treespopulated) {
			groupsPanel->PopulateTrees();
			groupsPanel->m_treespopulated = true;
		}
	}

	event.Skip(); // Allow continued event processing
}

void options::OnPageChange(wxListbookEvent& event)
{
	unsigned int i = event.GetSelection();
	lastPage = i;

	//    User selected Chart Page?
	//    If so, build the "Charts" page variants
	if (2 == i) { // 2 is the index of "Charts" page
		k_charts = VISIT_CHARTS;
	} else if (3 == i) { // 3 is the index of "VectorCharts" page
		//    User selected Vector Chart Page?
		k_vectorcharts = S52_CHANGED;
	} else if (m_pageUI == i) { // 5 is the index of "User Interface" page
		if (!m_bVisitLang) {
			::wxBeginBusyCursor();

			int current_language = plocale_def_lang->GetLanguage();
			wxString current_sel = wxLocale::GetLanguageName(current_language);

			current_sel = GetOCPNKnownLanguage(global::OCPN::get().sys().data().locale, NULL);

			const int nLang = sizeof(LANGUAGE_LIST) / sizeof(LANGUAGE_LIST[0]);

#ifdef __WXMSW__
			// always add us english
			m_itemLangListBox->Append(_T("English (U.S.)"));

			wxString lang_dir = global::OCPN::get().sys().data().sound_data_location
								+ _T("share/locale/");
			for (int it = 1; it < nLang; it++) {
				if (wxLocale::IsAvailable(LANGUAGE_LIST[it])) {
					wxLocale ltest(LANGUAGE_LIST[it], 0);
					ltest.AddCatalog(_T("opencpn"));
					if (!ltest.IsLoaded(_T("opencpn")))
						continue;

					// Defaults
					wxString loc_lang_name = wxLocale::GetLanguageName(LANGUAGE_LIST[it]);
					wxString widgets_lang_name = loc_lang_name;
					wxString lang_canonical
						= wxLocale::GetLanguageInfo(LANGUAGE_LIST[it])->CanonicalName;

					// Make opencpn substitutions
					wxString lang_suffix;
					loc_lang_name = GetOCPNKnownLanguage(lang_canonical, &lang_suffix);

					// Look explicitely to see if .mo is available
					wxString test_dir = lang_dir + lang_suffix;
					if (!wxDir::Exists(test_dir))
						continue;

					m_itemLangListBox->Append(loc_lang_name);
				}
			}
#else
			wxArrayString lang_array;

			// always add us english
			lang_array.Add(_T("en_US"));

			for (int it = 0; it < nLang; it++) {
				{
					wxLog::EnableLogging(false); // avoid "Cannot set locale to..." log message

					wxLocale ltest(LANGUAGE_LIST[it], 0);
					ltest.AddCatalog(_T("opencpn"));

					wxLog::EnableLogging(true);

					if (ltest.IsLoaded(_T("opencpn"))) {
						wxString s0 = wxLocale::GetLanguageInfo(LANGUAGE_LIST[it])->CanonicalName;
						wxString sl = wxLocale::GetLanguageName(LANGUAGE_LIST[it]);
						if (wxNOT_FOUND == lang_array.Index(s0))
							lang_array.Add(s0);
					}
				}
			}

			for (unsigned int i = 0; i < lang_array.GetCount(); i++) {
				//  Make opencpn substitutions
				wxString loc_lang_name = GetOCPNKnownLanguage(lang_array[i], NULL);
				m_itemLangListBox->Append(loc_lang_name);
			}
#endif

			// BUGBUG
			// Remember that wxLocale ctor has the effect of changing the system locale, including
			// the "C" libraries.
			// It should then also happen that the locale should be switched back to ocpn initial
			// load setting
			// upon the dtor of the above wxLocale instantiations....
			// wxWidgets may do so internally, but there seems to be no effect upon the system
			// libraries, so that
			// functions like strftime() do not revert to the correct locale setting.
			// Also, the catalog for the application is not reloaded by the ctor, so we must reload
			// them directly
			// So as workaround, we reset the locale explicitely.

			delete plocale_def_lang;
			plocale_def_lang = new wxLocale(current_language);

			setlocale(LC_NUMERIC, "C");
			plocale_def_lang->AddCatalog(_T("opencpn"));

			m_itemLangListBox->SetStringSelection(current_sel);

			// Initialize Language tab
			const wxLanguageInfo* pli
				= wxLocale::FindLanguageInfo(global::OCPN::get().sys().data().locale);
			if (pli) {
				wxString clang = pli->Description;
			}

			m_bVisitLang = true;

			::wxEndBusyCursor();
		}
	} else if (m_pagePlugins == i) { // 7 is the index of "Plugins" page
		k_plugins = TOOLBAR_CHANGED;
	}
}

void options::OnButtonSelectSound(wxCommandEvent&)
{
	const global::System::Data& sys = global::OCPN::get().sys().data();
	const global::System::Config& cfg = global::OCPN::get().sys().config();

	wxString sound_dir = sys.sound_data_location;
	sound_dir.Append(_T("sounds"));

	// FIXME: memory leak
	wxFileDialog* openDialog
		= new wxFileDialog(this, _("Select Sound File"), sound_dir, wxT(""),
						   _("WAV files (*.wav)|*.wav|All files (*.*)|*.*"), wxFD_OPEN);

	if (openDialog->ShowModal() != wxID_OK)
		return;

	global::AIS& ais = global::OCPN::get().ais();
	if (cfg.portable) {
		wxFileName f(openDialog->GetPath());
		f.MakeRelativeTo(sys.home_location);
		ais.set_AIS_Alert_Sound_File(f.GetFullPath());
	} else {
		ais.set_AIS_Alert_Sound_File(openDialog->GetPath());
	}

	g_anchorwatch_sound.UnLoad();
}

void options::OnButtonTestSound(wxCommandEvent&)
{
	sound::OCPN_Sound AIS_Sound;
	AIS_Sound.Create(global::OCPN::get().ais().get_data().AIS_Alert_Sound_File);

	if (AIS_Sound.IsOk()) {

#ifndef __WXMSW__
		AIS_Sound.Play();
		int t = 0;
		while (AIS_Sound.IsPlaying() && (t < 10)) {
			wxSleep(1);
			t++;
		}
		if (AIS_Sound.IsPlaying())
			AIS_Sound.Stop();

#else
		AIS_Sound.Play(wxSOUND_SYNC);
#endif
	}
}

static wxString GetOCPNKnownLanguage(wxString lang_canonical, wxString* lang_dir)
{
	wxString return_string;
	wxString dir_suffix;

	if (lang_canonical == _T("en_US")) {
		dir_suffix = _T("en");
		return_string = wxString("English (U.S.)", wxConvUTF8);
	} else if (lang_canonical == _T("cs_CZ")) {
		dir_suffix = _T("cs");
		return_string = wxString("Čeština", wxConvUTF8);
	} else if (lang_canonical == _T("da_DK")) {
		dir_suffix = _T("da");
		return_string = wxString("Dansk", wxConvUTF8);
	} else if (lang_canonical == _T("de_DE")) {
		dir_suffix = _T("de");
		return_string = wxString("Deutsch", wxConvUTF8);
	} else if (lang_canonical == _T("et_EE")) {
		dir_suffix = _T("et");
		return_string = wxString("Eesti", wxConvUTF8);
	} else if (lang_canonical == _T("es_ES")) {
		dir_suffix = _T("es");
		return_string = wxString("Español", wxConvUTF8);
	} else if (lang_canonical == _T("fr_FR")) {
		dir_suffix = _T("fr");
		return_string = wxString("Français", wxConvUTF8);
	} else if (lang_canonical == _T("it_IT")) {
		dir_suffix = _T("it");
		return_string = wxString("Italiano", wxConvUTF8);
	} else if (lang_canonical == _T("nl_NL")) {
		dir_suffix = _T("nl");
		return_string = wxString("Nederlands", wxConvUTF8);
	} else if (lang_canonical == _T("pl_PL")) {
		dir_suffix = _T("pl");
		return_string = wxString("Polski", wxConvUTF8);
	} else if (lang_canonical == _T("pt_PT")) {
		dir_suffix = _T("pt_PT");
		return_string = wxString("Português", wxConvUTF8);
	} else if (lang_canonical == _T("pt_BR")) {
		dir_suffix = _T("pt_BR");
		return_string = wxString("Português Brasileiro", wxConvUTF8);
	} else if (lang_canonical == _T("ru_RU")) {
		dir_suffix = _T("ru");
		return_string = wxString("Русский", wxConvUTF8);
	} else if (lang_canonical == _T("sv_SE")) {
		dir_suffix = _T("sv");
		return_string = wxString("Svenska", wxConvUTF8);
	} else if (lang_canonical == _T("fi_FI")) {
		dir_suffix = _T("fi_FI");
		return_string = wxString("Suomi", wxConvUTF8);
	} else if (lang_canonical == _T("nb_NO")) {
		dir_suffix = _T("nb_NO");
		return_string = wxString("Norsk", wxConvUTF8);
	} else if (lang_canonical == _T("tr_TR")) {
		dir_suffix = _T("tr_TR");
		return_string = wxString("Türkçe", wxConvUTF8);
	} else if (lang_canonical == _T("el_GR")) {
		dir_suffix = _T("el_GR");
		return_string = wxString("Ελληνικά", wxConvUTF8);
	} else if (lang_canonical == _T("hu_HU")) {
		dir_suffix = _T("hu_HU");
		return_string = wxString("Magyar", wxConvUTF8);
	} else if (lang_canonical == _T("zh_TW")) {
		dir_suffix = _T("zh_TW");
		return_string = wxString("正體字", wxConvUTF8);
	} else if (lang_canonical == _T("ca_ES")) {
		dir_suffix = _T("ca_ES");
		return_string = wxString("Catalan", wxConvUTF8);
	} else if (lang_canonical == _T("gl_ES")) {
		dir_suffix = _T("gl_ES");
		return_string = wxString("Galician", wxConvUTF8);
	} else {
		dir_suffix = lang_canonical;
		const wxLanguageInfo* info = wxLocale::FindLanguageInfo(lang_canonical);
		return_string = info->Description;
	}

	if (NULL != lang_dir)
		*lang_dir = dir_suffix;

	return return_string;
}

void options::OnInsertTideDataLocation(wxCommandEvent&)
{
	const global::System::Config& cfg = global::OCPN::get().sys().config();
	global::System& sys = global::OCPN::get().sys();

	wxFileDialog openDialog(
		this, _("Select Tide/Current Data"), sys.data().tc_data_dir, wxT(""),
		wxT("Tide/Current Data files (*.IDX; *.TCD)|*.IDX;*.idx;*.TCD;*.tcd|All files (*.*)|*.*"),
		wxFD_OPEN);
	int response = openDialog.ShowModal();
	if (response == wxID_OK) {
		wxString sel_file = openDialog.GetPath();

		if (cfg.portable) {
			wxFileName f(sel_file);
			f.MakeRelativeTo(sys.data().home_location);
			tcDataSelected->Append(f.GetFullPath());
		} else {
			tcDataSelected->Append(sel_file);
		}

		// Record the currently selected directory for later use
		wxFileName fn(sel_file);
		wxString data_dir = fn.GetPath();
		if (cfg.portable) {
			wxFileName f(data_dir);
			f.MakeRelativeTo(sys.data().home_location);
			sys.set_tc_data_dir(f.GetFullPath());
		} else {
			sys.set_tc_data_dir(data_dir);
		}
	}
}

void options::OnRemoveTideDataLocation(wxCommandEvent&)
{
	wxArrayInt sels;
	int nSel = tcDataSelected->GetSelections(sels);
	wxArrayString a;
	for (int i = 0; i < nSel; i++) {
		a.Add(tcDataSelected->GetString(sels.Item(i)));
	}

	for (unsigned int i = 0; i < a.Count(); i++) {
		int b = tcDataSelected->FindString(a.Item(i));
		tcDataSelected->Delete(b);
	}
}

void options::OnValChange(wxCommandEvent& event)
{
	event.Skip();
}

void options::OnConnValChange(wxCommandEvent& event)
{
	connectionsaved = false;
	event.Skip();
}

void options::OnTypeSerialSelected(wxCommandEvent& event)
{
#ifdef __WXGTK__
	if (!g_bserial_access_checked) {
		if (!CheckSerialAccess()) {
			// FIXME: empty?
		}
		g_bserial_access_checked = true;
	}
#endif

	OnConnValChange(event);
	SetNMEAFormToSerial();
}

void options::OnTypeNetSelected(wxCommandEvent& event)
{
	OnConnValChange(event);
	SetNMEAFormToNet();
}

void options::OnUploadFormatChange(wxCommandEvent& event)
{
	if (event.GetEventObject() == m_cbGarminUploadHost && event.IsChecked())
		m_cbFurunoGP3X->SetValue(false);
	else if (event.GetEventObject() == m_cbFurunoGP3X && event.IsChecked())
		m_cbGarminUploadHost->SetValue(false);
	event.Skip();
}

void options::ShowNMEACommon(bool visible)
{
	if (visible) {
		m_rbTypeSerial->Show();
		m_rbTypeNet->Show();
		m_rbIAccept->Show();
		m_rbIIgnore->Show();
		m_rbOAccept->Show();
		m_rbOIgnore->Show();
		m_tcInputStc->Show();
		m_btnInputStcList->Show();
		m_tcOutputStc->Show();
		m_btnOutputStcList->Show();
		m_cbOutput->Show();
		m_choicePriority->Show();
		m_stPriority->Show();
		m_cbCheckCRC->Show();
	} else {
		m_rbTypeSerial->Hide();
		m_rbTypeNet->Hide();
		m_rbIAccept->Hide();
		m_rbIIgnore->Hide();
		m_rbOAccept->Hide();
		m_rbOIgnore->Hide();
		m_tcInputStc->Hide();
		m_btnInputStcList->Hide();
		m_tcOutputStc->Hide();
		m_btnOutputStcList->Hide();
		m_cbOutput->Hide();
		m_choicePriority->Hide();
		m_stPriority->Hide();
		m_cbCheckCRC->Hide();
		sbSizerOutFilter->SetDimension(0, 0, 0, 0);
		sbSizerInFilter->SetDimension(0, 0, 0, 0);
		sbSizerConnectionProps->SetDimension(0, 0, 0, 0);
	}

	m_bNMEAParams_shown = visible;
}

void options::ShowNMEANet(bool visible)
{
	if (visible) {
		m_stNetAddr->Show();
		m_tNetAddress->Show();
		m_stNetPort->Show();
		m_tNetPort->Show();
		m_stNetProto->Show();
		m_rbNetProtoGPSD->Show();
		m_rbNetProtoTCP->Show();
		m_rbNetProtoUDP->Show();
	} else {
		m_stNetAddr->Hide();
		m_tNetAddress->Hide();
		m_stNetPort->Hide();
		m_tNetPort->Hide();
		m_stNetProto->Hide();
		m_rbNetProtoGPSD->Hide();
		m_rbNetProtoTCP->Hide();
		m_rbNetProtoUDP->Hide();
	}
}

void options::ShowNMEASerial(bool visible)
{
	if (visible) {
		m_stSerBaudrate->Show();
		m_choiceBaudRate->Show();
		m_stSerPort->Show();
		m_comboPort->Show();
		m_stSerProtocol->Show();
		m_choiceSerialProtocol->Show();
		m_cbGarminHost->Show();
		gSizerNetProps->SetDimension(0, 0, 0, 0);
	} else {
		m_stSerBaudrate->Hide();
		m_choiceBaudRate->Hide();
		m_stSerPort->Hide();
		m_comboPort->Hide();
		m_stSerProtocol->Hide();
		m_choiceSerialProtocol->Hide();
		m_cbGarminHost->Hide();
		gSizerSerProps->SetDimension(0, 0, 0, 0);
	}
}

void options::SetNMEAFormToSerial()
{
	ShowNMEACommon(true);
	ShowNMEANet(false);
	ShowNMEASerial(true);
	m_pNMEAForm->FitInside();
	m_pNMEAForm->Layout();
	Fit();
	Layout();
	SetDSFormRWStates();
}

void options::SetNMEAFormToNet()
{
	ShowNMEACommon(true);
	ShowNMEANet(true);
	ShowNMEASerial(false);
	m_pNMEAForm->FitInside();
	m_pNMEAForm->Layout();
	Fit();
	Layout();
	SetDSFormRWStates();
}

void options::ClearNMEAForm()
{
	ShowNMEACommon(false);
	ShowNMEANet(false);
	ShowNMEASerial(false);
	m_pNMEAForm->FitInside();
	m_pNMEAForm->Layout();
	Fit();
	Layout();
}

wxString StringArrayToString(wxArrayString arr)
{
	wxString ret = wxEmptyString;
	for (size_t i = 0; i < arr.Count(); i++) {
		if (i > 0)
			ret.Append(_T(","));
		ret.Append(arr[i]);
	}
	return ret;
}

void options::SetDSFormRWStates()
{
	if (m_rbNetProtoGPSD->GetValue() && !m_rbTypeSerial->GetValue()) {
		if (m_tNetPort->GetValue() == wxEmptyString)
			m_tNetPort->SetValue(_T("2947"));
		m_cbOutput->SetValue(false);
		m_cbOutput->Enable(false);
		m_rbOAccept->Enable(false);
		m_rbOIgnore->Enable(false);
		m_btnOutputStcList->Enable(false);
	} else if (m_rbNetProtoUDP->GetValue() && !m_rbTypeSerial->GetValue()) {
		if (m_tNetPort->GetValue() == wxEmptyString)
			m_tNetPort->SetValue(_T("10110"));
		m_cbOutput->Enable(true);
		m_rbOAccept->Enable(true);
		m_rbOIgnore->Enable(true);
		m_btnOutputStcList->Enable(true);
	} else {
		m_cbOutput->Enable(true);
		m_rbOAccept->Enable(true);
		m_rbOIgnore->Enable(true);
		m_btnOutputStcList->Enable(true);
	}
}

void options::SetConnectionParams(const ConnectionParams& cp)
{
	m_comboPort->Select(m_comboPort->FindString(cp.getPort()));
	m_comboPort->SetValue(cp.getPort());
	m_cbCheckCRC->SetValue(cp.isChecksumCheck());
	m_cbGarminHost->SetValue(cp.isGarmin());
	m_cbOutput->SetValue(cp.isOutput());
	if (cp.getInputSentenceListType() == ConnectionParams::WHITELIST)
		m_rbIAccept->SetValue(true);
	else
		m_rbIIgnore->SetValue(true);
	if (cp.getOutputSentenceListType() == ConnectionParams::WHITELIST)
		m_rbOAccept->SetValue(true);
	else
		m_rbOIgnore->SetValue(true);
	m_tcInputStc->SetValue(StringArrayToString(cp.getInputSentenceList()));
	m_tcOutputStc->SetValue(StringArrayToString(cp.getOutputSentenceList()));
	m_choiceBaudRate->Select(m_choiceBaudRate->FindString(wxString::Format(_T("%d"), cp.getBaudrate())));
	m_choiceSerialProtocol->Select(cp.getProtocol()); // TODO
	m_choicePriority->Select(m_choicePriority->FindString(wxString::Format(_T("%d"), cp.getPriority())));

	m_tNetAddress->SetValue(cp.getNetworkAddress());

	if (cp.getNetworkPort() == 0)
		m_tNetPort->SetValue(_T(""));
	else
		m_tNetPort->SetValue(wxString::Format(wxT("%i"), cp.getNetworkPort()));

	if (cp.getNetProtocol() == ConnectionParams::TCP)
		m_rbNetProtoTCP->SetValue(true);
	else if (cp.getNetProtocol() == ConnectionParams::UDP)
		m_rbNetProtoUDP->SetValue(true);
	else
		m_rbNetProtoGPSD->SetValue(true);

	if (cp.getType() == ConnectionParams::SERIAL) {
		m_rbTypeSerial->SetValue(true);
		SetNMEAFormToSerial();
	} else {
		m_rbTypeNet->SetValue(true);
		SetNMEAFormToNet();
	}

	m_connection_enabled = cp.isEnabled();
}

void options::SetDefaultConnectionParams()
{
	m_comboPort->Select(0);
	m_comboPort->SetValue(_T(""));
	m_cbCheckCRC->SetValue(true);
	m_cbGarminHost->SetValue(false);
	m_cbOutput->SetValue(false);
	m_rbIAccept->SetValue(true);
	m_rbOAccept->SetValue(true);
	m_tcInputStc->SetValue(_T(""));
	m_tcOutputStc->SetValue(_T(""));
	m_choiceBaudRate->Select(m_choiceBaudRate->FindString(_T("4800")));
	m_choicePriority->Select(m_choicePriority->FindString(_T("1")));

	bool bserial = true;
#ifdef __WXGTK__
	if (!g_bserial_access_checked)
		bserial = false;
#endif
	m_rbTypeSerial->SetValue(bserial);
	m_rbTypeNet->SetValue(!bserial);

	if (bserial)
		SetNMEAFormToSerial();
	else
		SetNMEAFormToNet();

	m_connection_enabled = true;
}

void options::OnAddDatasourceClick(wxCommandEvent&)
{
	connectionsaved = false;
	SetDefaultConnectionParams();

	long itemIndex = -1;
	for (;;) {
		itemIndex = m_lcSources->GetNextItem(itemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (itemIndex == -1)
			break;
		m_lcSources->SetItemState(itemIndex, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
	}
	m_buttonRemove->Enable(false);
}

void options::FillSourceList()
{
	m_buttonRemove->Enable(false);
	m_lcSources->DeleteAllItems();
	for (size_t i = 0; i < g_pConnectionParams->size(); i++) {
		wxListItem li;
		li.SetId(i);
		li.SetImage(g_pConnectionParams->at(i).isEnabled() ? 1 : 0);
		li.SetData(i);
		li.SetText(_T(""));

		long itemIndex = m_lcSources->InsertItem(li);

		m_lcSources->SetItem(itemIndex, 1, g_pConnectionParams->at(i).GetSourceTypeStr());
		m_lcSources->SetItem(itemIndex, 2, g_pConnectionParams->at(i).GetAddressStr());
		wxString prio_str;
		prio_str.Printf(_T("%d"), g_pConnectionParams->at(i).getPriority());
		m_lcSources->SetItem(itemIndex, 3, prio_str);
		m_lcSources->SetItem(itemIndex, 4, g_pConnectionParams->at(i).GetParametersStr());
		m_lcSources->SetItem(itemIndex, 5, g_pConnectionParams->at(i).GetOutputValueStr());
		m_lcSources->SetItem(itemIndex, 6, g_pConnectionParams->at(i).GetFiltersStr());
	}

#ifdef __WXOSX__
	m_lcSources->SetColumnWidth(0, wxLIST_AUTOSIZE);
	m_lcSources->SetColumnWidth(1, wxLIST_AUTOSIZE);
	m_lcSources->SetColumnWidth(2, wxLIST_AUTOSIZE);
	m_lcSources->SetColumnWidth(3, wxLIST_AUTOSIZE);
	m_lcSources->SetColumnWidth(4, wxLIST_AUTOSIZE);
	m_lcSources->SetColumnWidth(5, wxLIST_AUTOSIZE);
	m_lcSources->SetColumnWidth(6, wxLIST_AUTOSIZE);
#else
	m_lcSources->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	m_lcSources->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
	m_lcSources->SetColumnWidth(2, wxLIST_AUTOSIZE);
	m_lcSources->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
	m_lcSources->SetColumnWidth(4, wxLIST_AUTOSIZE_USEHEADER);
	m_lcSources->SetColumnWidth(5, wxLIST_AUTOSIZE_USEHEADER);
	m_lcSources->SetColumnWidth(6, wxLIST_AUTOSIZE);
#endif

	m_lcSources->SortItems(SortConnectionOnPriority, (long)m_lcSources);
}

void options::OnRemoveDatasourceClick(wxCommandEvent&)
{
	long itemIndex = -1;
	for (;;) {
		itemIndex = m_lcSources->GetNextItem(itemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (itemIndex == -1)
			break;

		int params_index = m_lcSources->GetItemData(itemIndex);
		if (params_index != -1) {
			const ConnectionParams cp = g_pConnectionParams->at(params_index);
			g_pConnectionParams->erase(g_pConnectionParams->begin() + params_index);

			DataStream* pds_existing = g_pMUX->FindStream(cp.GetDSPort());
			if (pds_existing)
				g_pMUX->StopAndRemoveStream(pds_existing);
		}

		//  Mark connection deleted
		m_rbTypeSerial->SetValue(true);
		m_comboPort->SetValue(_T("Deleted"));
	}
	FillSourceList();
	ShowNMEACommon(false);
	ShowNMEANet(false);
	ShowNMEASerial(false);
}

void options::OnSelectDatasource(wxListEvent& event)
{
	connectionsaved = false;
	int params_index = event.GetData();
	SetConnectionParams(g_pConnectionParams->at(params_index));
	m_buttonRemove->Enable();
	event.Skip();
}

void options::OnBtnIStcs(wxCommandEvent&)
{
	m_stcdialog_in->SetSentenceList(wxStringTokenize(m_tcInputStc->GetValue(), _T(",")));
	m_stcdialog_in->SetType(0, (m_rbIAccept->GetValue() == true) ? ConnectionParams::WHITELIST
																 : ConnectionParams::BLACKLIST);

	if (m_stcdialog_in->ShowModal() == wxID_OK)
		m_tcInputStc->SetValue(m_stcdialog_in->GetSentencesAsText());
}

void options::OnBtnOStcs(wxCommandEvent&)
{
	m_stcdialog_out->SetSentenceList(wxStringTokenize(m_tcOutputStc->GetValue(), _T(",")));
	m_stcdialog_out->SetType(1, (m_rbOAccept->GetValue() == true) ? ConnectionParams::WHITELIST
																  : ConnectionParams::BLACKLIST);

	if (m_stcdialog_out->ShowModal() == wxID_OK)
		m_tcOutputStc->SetValue(m_stcdialog_out->GetSentencesAsText());
}

void options::OnNetProtocolSelected(wxCommandEvent& event)
{
	if (m_rbNetProtoGPSD->GetValue()) {
		if (m_tNetPort->GetValue() == wxEmptyString)
			m_tNetPort->SetValue(_T("2947"));
	} else if (m_rbNetProtoUDP->GetValue()) {
		if (m_tNetPort->GetValue() == wxEmptyString)
			m_tNetPort->SetValue(_T("10110"));
	}

	SetDSFormRWStates();
	OnConnValChange(event);
}

void options::OnRbAcceptInput(wxCommandEvent& event)
{
	OnConnValChange(event);
}

void options::OnRbIgnoreInput(wxCommandEvent& event)
{
	OnConnValChange(event);
}

void options::OnRbOutput(wxCommandEvent& event)
{
	OnConnValChange(event);
}

void options::SetInitChartDir(const wxString& dir)
{
	m_init_chart_dir = dir;
}

void options::SetCurrentDirList(const ChartDirectories& p)
{
	m_CurrentDirList = p;
}

void options::SetWorkDirListPtr(ChartDirectories* p) // FIXME
{
	m_pWorkDirList = p;
}

ChartDirectories* options::GetWorkDirListPtr() // FIXME
{
	return m_pWorkDirList;
}

void options::SetConfigPtr(Config* p)
{
	m_pConfig = p;
}

void options::OnBaudrateChoice(wxCommandEvent& event)
{
	OnConnValChange(event);
}

void options::OnProtocolChoice(wxCommandEvent& event)
{
	OnConnValChange(event);
}

void options::OnCrcCheck(wxCommandEvent& event)
{
	OnValChange(event);
}

void options::OnCbOutput(wxCommandEvent& event)
{
	OnConnValChange(event);
}

size_t options::get_pageDisplay() const
{
	return m_pageDisplay;
}

size_t options::get_pageConnections() const
{
	return m_pageConnections;
}

size_t options::get_pageCharts() const
{
	return m_pageCharts;
}

size_t options::get_pageShips() const
{
	return m_pageShips;
}

size_t options::get_pageUI() const
{
	return m_pageUI;
}

size_t options::get_pagePlugins() const
{
	return m_pagePlugins;
}

void options::set_last_window_pos(const wxPoint& pos)
{
	lastWindowPos = pos;
}

int options::get_lastPage() const
{
	return lastPage;
}

const wxPoint& options::get_lastWindowPos() const
{
	return lastWindowPos;
}

const wxSize& options::get_lastWindowSize() const
{
	return lastWindowSize;
}

void options::set_page_selection(int page)
{
	m_pListbook->SetSelection(page);
}

