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

#include "AboutDialog.h"
#include <ChartCanvas.h>
#include <DimeControl.h>
#include <OCPN_Version.h>

#include <gui/StyleManager.h>
#include <gui/Style.h>

#include <global/OCPN.h>
#include <global/System.h>

#include <wx/textfile.h>
#include <wx/textctrl.h>
#include <wx/ffile.h>
#include <wx/clipbrd.h>
#include <wx/sizer.h>
#include <wx/log.h>
#include <wx/button.h>
#include <wx/bmpbuttn.h>
#include <wx/dataobj.h>
#include <wx/html/htmlwin.h>

static char AboutText[] =
{
	"\n                                         OpenCPN\n\n"
	"                       (c) 2000-2013 The OpenCPN Authors\n"
};

static char OpenCPNInfo[] = {
	"\n\n"
	"      OpenCPN is a Free Software project, built by sailors.\n"
	"       It is freely available to download and distribute\n"
	"               without charge at Opencpn.org.\n\n"
	"       If you use OpenCPN, please consider contributing\n"
	"                or donating funds to the project.\n\n"
	"      Documentation\n"
	"           http://Opencpn.org\n\n"
};

static char AuthorText[] =
{
	"   David S Register\n"
	"      OpenCPN Lead Developer\n\n"
	"    Jesper Weissglas\n"
	"      Vector Chart Rendering\n"
	"      User Interface\n\n"
	"    Sean D'Epagnier\n"
	"      OpenGL Architecture\n\n"
	"    Kathleen Boswell\n"
	"      Icon design\n\n"
	"    Flavius Bindea\n"
	"      CM93 Offset and AIS enhancements\n\n"
	"    Gunther Pilz\n"
	"      Windows Installer enhancements\n\n"
	"    Alan Bleasby\n"
	"      Garmin jeeps module\n\n"
	"    Jean-Eudes Onfray\n"
	"      Dashboard and Dialog enhancements\n\n"
	"    Pavel Kalian\n"
	"      S52 Rasterization Improvements\n\n"
	"    Piotr Carlson\n"
	"      General usability enhancements\n\n"
	"    Anders Lund\n"
	"      RouteManagerDialog\n\n"
	"    Gordon Mau\n"
	"      OpenCPN Documentation\n\n"
	"    Tim Francis\n"
	"      OpenCPN Documentation\n\n"
	"    Mark A Sikes\n"
	"      OpenCPN CoDeveloper\n\n"
	"    Thomas Haller\n"
	"      GPX Import/Export Implementation\n\n"
	"    Will Kamp\n"
	"      Toolbar Icon design\n\n"
	"    Richard Smith\n"
	"      OpenCPN CoDeveloper, MacOSX\n\n"
	"    David Herring\n"
	"      OpenCPN CoDeveloper, MacOSX\n\n"
	"    Philip Lange\n"
	"      OpenCPN Documentation\n\n"
	"    Ron Kuris\n"
	"      wxWidgets Support\n\n"
	"    Julian Smart, Robert Roebling et al\n"
	"      wxWidgets Authors\n\n"
	"    Sylvain Duclos\n"
	"      S52 Presentation Library code\n\n"
	"    Manish P. Pagey\n"
	"      Serial Port Library\n\n"
	"    David Flater\n"
	"      XTIDE tide and current code\n\n"
	"    Frank Warmerdam\n"
	"      GDAL Class Library\n\n"
	"    Mike Higgins\n"
	"      BSB Chart Format Detail\n\n"
	"    Samuel R. Blackburn\n"
	"      NMEA0183 Class Library\n\n"
	"    Atul Narkhede\n"
	"      Polygon Graphics utilities\n\n"
	"    Jan C. Depner\n"
	"      WVS Chart Library\n\n"
	"    Stuart Cunningham, et al\n"
	"      BSB Chart Georeferencing Algorithms\n\n"
	"    John F. Waers\n"
	"      UTM Conversion Algorithms\n\n"
	"    Carsten Tschach\n"
	"      UTM Conversion Algorithms\n\n"
	"    Ed Williams\n"
	"      Great Circle Formulary\n\n"
	"    Philippe Bekaert\n"
	"      CIE->RGB Color Conversion Matrix\n\n"
	"    Robert Lipe\n"
	"      Garmin USB GPS Interface\n"
};

IMPLEMENT_DYNAMIC_CLASS(AboutDialog, wxDialog)

BEGIN_EVENT_TABLE(AboutDialog, wxDialog)
	EVT_BUTTON(xID_OK, AboutDialog::OnXidOkClick)
	EVT_NOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK_HELP, AboutDialog::OnPageChange)
	EVT_BUTTON(ID_DONATE, AboutDialog::OnDonateClick)
	EVT_BUTTON(ID_COPYINI, AboutDialog::OnCopyClick)
	EVT_BUTTON(ID_COPYLOG, AboutDialog::OnCopyClick)
END_EVENT_TABLE()

AboutDialog::AboutDialog()
	: m_parent(NULL)
	, m_dataLocn(wxEmptyString)
	, m_ptips_window(NULL)
	, m_btips_loaded(false)
	, itemPanelAbout(NULL)
	, itemPanelAuthors(NULL)
	, itemPanelLicense(NULL)
	, itemPanelTips(NULL)
	, pAboutTextCtl(NULL)
	, pAuthorTextCtl(NULL)
	, pLicenseTextCtl(NULL)
	, pNotebook(NULL)
{}

AboutDialog::AboutDialog(wxWindow* parent, const wxString& license_data_Locn, wxWindowID id,
						 const wxString& caption, const wxPoint& pos, const wxSize& size,
						 long style)
	: m_parent(parent)
	, m_dataLocn(license_data_Locn)
	, m_ptips_window(NULL)
	, m_btips_loaded(false)
	, itemPanelAbout(NULL)
	, itemPanelAuthors(NULL)
	, itemPanelLicense(NULL)
	, itemPanelTips(NULL)
	, pAboutTextCtl(NULL)
	, pAuthorTextCtl(NULL)
	, pLicenseTextCtl(NULL)
	, pNotebook(NULL)
{
#ifdef __WXOSX__
	style |= wxSTAY_ON_TOP;
#endif
	Create(parent, id, caption, pos, size, style);
}

bool AboutDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption,
						 const wxPoint& pos, const wxSize& size, long style)
{
	SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
	wxDialog::Create(parent, id, caption, pos, size, style);

	m_parent = parent;

	m_btips_loaded = false;

	CreateControls();
	Update();

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();

	return true;
}

void AboutDialog::Update()
{
	pAboutTextCtl->Clear();
	wxString* pAboutString = new wxString(AboutText, wxConvUTF8);

	pAboutString->Append(ocpn::Version().get_version());
	pAboutString->Append(ocpn::Version().get_git_info());
	pAboutString->Append(wxString(OpenCPNInfo, wxConvUTF8));

	pAboutTextCtl->WriteText(*pAboutString);
	delete pAboutString;

	const global::System::Data& sys = global::OCPN::get().sys().data();

	// Show the user where the log file is going to be
	wxString log = _T("    Logfile location: ");
	log.Append(sys.log_file);
	pAboutTextCtl->WriteText(log);

	// Show the user where the config file is going to be
	wxString conf = _T("\n    Config file location: ");
	conf.Append(sys.config_file);
	pAboutTextCtl->WriteText(conf);

	pAuthorTextCtl->Clear();
	wxString* pAuthorsString = new wxString(AuthorText, wxConvUTF8);
	pAuthorTextCtl->WriteText(*pAuthorsString);
	delete pAuthorsString;

	pLicenseTextCtl->Clear();
	wxString license_loc(m_dataLocn);
	license_loc.Append(_T("license.txt"));

	wxTextFile license_file(license_loc);

	if (license_file.Open()) {
		wxString str = license_file.GetFirstLine();
		pLicenseTextCtl->WriteText(str);

		while (!license_file.Eof()) {
			str = license_file.GetNextLine();
			str.Append(_T("\n"));
			pLicenseTextCtl->AppendText(str);
		}
		license_file.Close();
	} else {
		wxString msg(_T("Could not open License file: "));
		msg.Append(license_loc);
		wxLogMessage(msg);
	}
	pLicenseTextCtl->SetInsertionPoint(0);

	DimeControl(this);
}

void AboutDialog::CreateControls()
{
	AboutDialog* itemDialog1 = this;

	wxBoxSizer* aboutSizer = new wxBoxSizer(wxVERTICAL);
	itemDialog1->SetSizer(aboutSizer);

	wxStaticText* pST1 = new wxStaticText(this, -1, _T("Label"), wxDefaultPosition, wxSize(500, 30),
										  wxALIGN_CENTRE | wxALIGN_CENTER_VERTICAL);
	pST1->SetLabel(_("The Open Source Chart Plotter/Navigator"));
	wxFont* headerFont = wxTheFontList->FindOrCreateFont(14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
														 wxFONTWEIGHT_BOLD);
	pST1->SetFont(*headerFont);
	aboutSizer->Add(pST1, 1, wxALL | wxEXPAND, 8);

	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton* copyIni = new wxButton(itemDialog1, ID_COPYINI, _("Copy Settings File to Clipboard"));
	buttonSizer->Add(copyIni, 1, wxALL | wxEXPAND, 3);

	wxButton* copyLog = new wxButton(itemDialog1, ID_COPYLOG, _("Copy Log File to Clipboard"));
	buttonSizer->Add(copyLog, 1, wxALL | wxEXPAND, 3);

	wxBitmap donate_bmp = global::OCPN::get().styleman().current().GetIcon(_T("donate"));

	wxButton* donateButton = new wxBitmapButton(itemDialog1, ID_DONATE, donate_bmp,
												wxDefaultPosition, wxDefaultSize, 0);

	buttonSizer->Add(donateButton, 1, wxALL | wxEXPAND | wxALIGN_RIGHT, 3);

	// Main Notebook
	pNotebook = new wxNotebook(itemDialog1, ID_NOTEBOOK_HELP, wxDefaultPosition, wxSize(-1, -1),
							   wxNB_TOP);
	pNotebook->InheritAttributes();
	aboutSizer->Add(pNotebook, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 5);

	aboutSizer->Add(buttonSizer, 0, wxALL, 0);

	// About Panel
	itemPanelAbout = new wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize,
								 wxSUNKEN_BORDER | wxTAB_TRAVERSAL);
	itemPanelAbout->InheritAttributes();
	pNotebook->AddPage(itemPanelAbout, _("About"));

	wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
	itemPanelAbout->SetSizer(itemBoxSizer6);

	pAboutTextCtl = new wxTextCtrl(itemPanelAbout, -1, _T(""), wxDefaultPosition, wxSize(-1, 300),
								   wxTE_MULTILINE | wxTE_READONLY);
	pAboutTextCtl->InheritAttributes();
	itemBoxSizer6->Add(pAboutTextCtl, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL, 5);

	// Authors Panel
	itemPanelAuthors = new wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize,
								   wxSUNKEN_BORDER | wxTAB_TRAVERSAL);
	itemPanelAuthors->InheritAttributes();
	pNotebook->AddPage(itemPanelAuthors, _("Authors"));

	wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
	itemPanelAuthors->SetSizer(itemBoxSizer7);

	pAuthorTextCtl = new wxTextCtrl(itemPanelAuthors, -1, _T(""), wxDefaultPosition,
									wxSize(-1, 300), wxTE_MULTILINE | wxTE_READONLY);
	pAuthorTextCtl->InheritAttributes();
	itemBoxSizer7->Add(pAuthorTextCtl, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL, 5);

	// License Panel
	itemPanelLicense = new wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize,
								   wxSUNKEN_BORDER | wxTAB_TRAVERSAL);
	itemPanelLicense->InheritAttributes();
	pNotebook->AddPage(itemPanelLicense, _("License"));

	wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
	itemPanelLicense->SetSizer(itemBoxSizer8);

	int tcflags = wxTE_MULTILINE | wxTE_READONLY;

	// wxX11 TextCtrl is broken in many ways.
	// Here, the wxTE_DONTWRAP flag creates a horizontal scroll bar
	// which fails in wxX11 2.8.2....
#ifndef __WXX11__
	tcflags |= wxTE_DONTWRAP;
#endif
	pLicenseTextCtl
		= new wxTextCtrl(itemPanelLicense, -1, _T(""), wxDefaultPosition, wxSize(-1, 300), tcflags);

	pLicenseTextCtl->InheritAttributes();
	itemBoxSizer8->Add(pLicenseTextCtl, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL, 5);

	// Help Panel
	itemPanelTips = new wxPanel(pNotebook, -1, wxDefaultPosition, wxDefaultSize,
								wxSUNKEN_BORDER | wxTAB_TRAVERSAL);
	itemPanelTips->InheritAttributes();
	pNotebook->AddPage(itemPanelTips, _("Help"));

	wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
	itemPanelTips->SetSizer(itemBoxSizer9);

	// Close Button

	wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
	aboutSizer->Add(itemBoxSizer28, 0, wxALIGN_RIGHT | wxALL, 5);

	wxButton* itemButton29
		= new wxButton(itemDialog1, xID_OK, _("Close"), wxDefaultPosition, wxDefaultSize, 0);
	itemButton29->SetDefault();
	itemButton29->InheritAttributes();
	itemBoxSizer28->Add(itemButton29, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
}

void AboutDialog::OnXidOkClick(wxCommandEvent&)
{
	Close();
}

void AboutDialog::OnDonateClick(wxCommandEvent&)
{
	wxLaunchDefaultBrowser(_T("https://sourceforge.net/donate/index.php?group_id=180842"));
}

void AboutDialog::OnCopyClick(wxCommandEvent& event)
{
	const global::System::Data& sys = global::OCPN::get().sys().data();

	wxString filename = sys.config_file;
	if (event.GetId() == ID_COPYLOG)
		filename = sys.log_file;

	wxFFile file(filename);

	if (!file.IsOpened()) {
		wxLogMessage(_T("Failed to open file for Copy to Clipboard."));
		return;
	}

	wxString fileContent;
	char buf[1024];
	while (!file.Eof()) {
		int c = file.Read(&buf, 1024);
		if (c)
			fileContent += wxString(buf, wxConvUTF8, c);
	}

	file.Close();

	if (event.GetId() == ID_COPYLOG) {
		wxString lastLogs = fileContent;
		int pos = lastLogs.Find(_T("________"));
		while (pos != wxNOT_FOUND && lastLogs.Length() > 65000) {
			lastLogs = lastLogs.Right(lastLogs.Length() - pos - 8);
			pos = lastLogs.Find(_T("________"));
		}
		fileContent = lastLogs;
	}

	::wxBeginBusyCursor();

	if (wxTheClipboard->Open()) {
		wxTextDataObject* data = new wxTextDataObject;
		data->SetText(fileContent);
		if (!wxTheClipboard->SetData(data)) {
			wxLogMessage(_T("wxTheClipboard->Open() failed."));
		}
		wxTheClipboard->Close();
	} else {
		wxLogMessage(_T("wxTheClipboard->Open() failed."));
	}
	::wxEndBusyCursor();
}

void AboutDialog::OnPageChange(wxNotebookEvent& event)
{
	int i = event.GetSelection();

	if (3 == i) { // 3 is the index of "Help" page
		wxString def_lang_canonical = wxLocale::GetLanguageInfo(wxLANGUAGE_DEFAULT)->CanonicalName;

		wxString help_locn = _T("doc/help_");
		help_locn.Prepend(m_dataLocn);

		wxString help_try = help_locn;
		help_try += def_lang_canonical;
		help_try += _T(".html");

		if (::wxFileExists(help_try)) {
			wxLaunchDefaultBrowser(wxString(_T("file:///")) + help_try);
			pNotebook->ChangeSelection(0);
		} else {
			help_try = help_locn;
			help_try += _T("en_US");
			help_try += _T(".html");

			if (::wxFileExists(help_try)) {
				pNotebook->ChangeSelection(0);
				wxLaunchDefaultBrowser(wxString(_T("file:///")) + help_try);
			} else {
				help_try = _T("doc/help_web.html");
				help_try.Prepend(m_dataLocn);
				if (::wxFileExists(help_try)) {
					pNotebook->ChangeSelection(0);
					wxLaunchDefaultBrowser(wxString(_T("file:///")) + help_try);
				}
			}
		}
	}
}

