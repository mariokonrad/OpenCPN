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

#ifndef __OPTIONS__H__
#define __OPTIONS__H__

#include <wx/listbook.h>
#include <wx/dirctrl.h>
#include <wx/spinctrl.h>
#include <wx/listctrl.h>
#include <wx/choice.h>
#include <wx/collpane.h>
#include <wx/dialog.h>

#include <plugin/PlugInManager.h>
#include <chart/ChartDatabase.h>
#include <MainFrame.h>
#include <ChartDirectoryInfo.h>
#include <GUI_IDs.h>

class wxGenericDirCtrl;
class Config;
class ChartGroupsUI;
class ConnectionParams;
class SentenceListDlg;
class MainFrame;

#define ID_DIALOG 10001

// Define an int bit field for dialog return value
// To indicate which types of settings have changed
#define GENERIC_CHANGED    1
#define S52_CHANGED        2
#define FONT_CHANGED       4
#define FORCE_UPDATE       8
#define VISIT_CHARTS      16
#define LOCALE_CHANGED    32
#define TOOLBAR_CHANGED   64
#define CHANGE_CHARTS    128
#define SCAN_UPDATE      256
#define GROUPS_CHANGED   512
#define STYLE_CHANGED   1024
#define TIDES_CHANGED   2048
#define GL_CHANGED      4096

class options : public wxDialog
{
	DECLARE_DYNAMIC_CLASS(options)
	DECLARE_EVENT_TABLE()

public:
	options();
	options(MainFrame* parent, wxWindowID id = ID_DIALOG, const wxString& caption = _T("Options"),
			const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(500, 500),
			long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

	virtual ~options();

	bool Create(MainFrame* parent, wxWindowID id = ID_DIALOG,
				const wxString& caption = _T("Options"), const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxSize(500, 500),
				long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

	void Init();

	wxWindow* GetContentWindow() const;

	void CreateControls();
	size_t CreatePanel(const wxString& title);
	wxScrolledWindow* AddPage(size_t parent, const wxString& title);
	bool DeletePage(wxScrolledWindow* page);
	void SetColorScheme(global::ColorScheme cs);

	void SetInitChartDir(const wxString& dir);
	void SetInitialSettings();

	void SetCurrentDirList(const ChartDirectories& p);
	void SetWorkDirListPtr(ChartDirectories* p); // FIXME
	ChartDirectories* GetWorkDirListPtr(); // FIXME
	void UpdateDisplayedChartDirList(const ChartDirectories& p);
	void SetConfigPtr(Config* p);

	size_t get_pageDisplay() const;
	size_t get_pageConnections() const;
	size_t get_pageCharts() const;
	size_t get_pageShips() const;
	size_t get_pageUI() const;
	size_t get_pagePlugins() const;

	void set_last_window_pos(const wxPoint&);

	int get_lastPage() const;
	const wxPoint& get_lastWindowPos() const;
	const wxSize& get_lastWindowSize() const;

	void set_page_selection(int);

private:
	void OnDebugcheckbox1Click(wxCommandEvent& event);
	void OnDirctrlSelChanged(wxTreeEvent& event);
	void OnButtonaddClick(wxCommandEvent& event);
	void OnButtondeleteClick(wxCommandEvent& event);
	void OnRadioboxSelected(wxCommandEvent& event);
	void OnApplyClick(wxCommandEvent& event);
	void OnXidOkClick(wxCommandEvent& event);
	void OnCancelClick(wxCommandEvent& event);
	void OnChooseFont(wxCommandEvent& event);
	void OnCPAWarnClick(wxCommandEvent& event);

#ifdef __WXGTK__
	void OnChooseFontColor(wxCommandEvent& event);
#endif
	void OnDisplayCategoryRadioButton(wxCommandEvent& event);
	void OnButtonClearClick(wxCommandEvent& event);
	void OnButtonSelectClick(wxCommandEvent& event);
	void OnPageChange(wxListbookEvent& event);
	void OnButtonSelectSound(wxCommandEvent& event);
	void OnButtonTestSound(wxCommandEvent& event);
	void OnShowGpsWindowCheckboxClick(wxCommandEvent& event);
	void OnZTCCheckboxClick(wxCommandEvent& event);
	void OnRadarringSelect(wxCommandEvent& event);
	void OnShipTypeSelect(wxCommandEvent& event);
	void OnButtonGroups(wxCommandEvent& event);
	void OnInsertTideDataLocation(wxCommandEvent& event);
	void OnRemoveTideDataLocation(wxCommandEvent& event);
	void OnCharHook(wxKeyEvent& event);
	void OnChartsPageChange(wxListbookEvent& event);

	void UpdateWorkArrayFromTextCtl();

	// Should we show tooltips?
	static bool ShowToolTips();

	wxListbook* m_pListbook;
	size_t m_pageDisplay;
	size_t m_pageConnections;
	size_t m_pageCharts;
	size_t m_pageShips;
	size_t m_pageUI;
	size_t m_pagePlugins;
	int lastPage;
	wxPoint lastWindowPos;
	wxSize lastWindowSize;
	wxButton* m_ApplyButton;
	wxButton* m_OKButton;
	wxButton* m_CancelButton;

	chart::ChartGroupArray* m_pGroupArray;
	int m_groups_changed;

	// For General Options
	wxCheckBox* pDebugShowStat;
	wxCheckBox* pPrintShowIcon;
	wxCheckBox* pCDOOutlines;
	wxCheckBox* pSDepthUnits;
	wxCheckBox* pSDisplayGrid;
	wxCheckBox* pAutoAnchorMark;
	wxCheckBox* pCDOQuilting;
	wxCheckBox* pCBRaster;
	wxCheckBox* pCBVector;
	wxCheckBox* pCBCM93;
	wxCheckBox* pCBCourseUp;
	wxTextCtrl* pCOGUPUpdateSecs;
	wxCheckBox* pCBLookAhead;
	wxTextCtrl* m_pText_OSCOG_Predictor;
	wxChoice* m_pShipIconType;
	wxCheckBox* pSkewComp;
	wxCheckBox* pOpenGL;
	wxCheckBox* pSmoothPanZoom;
	wxCheckBox* pFullScreenQuilt;
	wxChoice* m_pcTCDatasets;
	wxCheckBox* pCBMagShow;
	wxTextCtrl* pMagVar;
	wxCheckBox* pMobile;
	wxCheckBox* pResponsive;

	int k_tides;

	// For GPS Page
	wxListCtrl* m_lcSources;
	wxButton* m_buttonAdd;
	wxButton* m_buttonRemove;
	wxStaticBoxSizer* sbSizerConnectionProps;
	wxRadioButton* m_rbTypeSerial;
	wxRadioButton* m_rbTypeNet;
	wxGridSizer* gSizerNetProps;
	wxStaticText* m_stNetProto;
	wxRadioButton* m_rbNetProtoTCP;
	wxRadioButton* m_rbNetProtoUDP;
	wxRadioButton* m_rbNetProtoGPSD;
	wxStaticText* m_stNetAddr;
	wxTextCtrl* m_tNetAddress;
	wxStaticText* m_stNetPort;
	wxTextCtrl* m_tNetPort;
	wxGridSizer* gSizerSerProps;
	wxStaticText* m_stSerPort;
	wxComboBox* m_comboPort;
	wxStaticText* m_stSerBaudrate;
	wxChoice* m_choiceBaudRate;
	wxStaticText* m_stSerProtocol;
	wxChoice* m_choiceSerialProtocol;
	wxStaticText* m_stPriority;
	wxChoice* m_choicePriority;
	wxCheckBox* m_cbCheckCRC;
	wxCheckBox* m_cbGarminHost;
	wxCheckBox* m_cbGarminUploadHost;
	wxCheckBox* m_cbFurunoGP3X;
	wxCheckBox* m_cbNMEADebug;
	wxCheckBox* m_cbFilterSogCog;
	wxStaticText* m_stFilterSec;
	wxTextCtrl* m_tFilterSec;
	wxRadioButton* m_rbIAccept;
	wxRadioButton* m_rbIIgnore;
	wxTextCtrl* m_tcInputStc;
	wxButton* m_btnInputStcList;
	wxCheckBox* m_cbOutput;
	wxRadioButton* m_rbOAccept;
	wxRadioButton* m_rbOIgnore;
	wxTextCtrl* m_tcOutputStc;
	wxButton* m_btnOutputStcList;
	wxStdDialogButtonSizer* m_sdbSizerDlgButtons;
	wxButton* m_sdbSizerDlgButtonsOK;
	wxButton* m_sdbSizerDlgButtonsApply;
	wxButton* m_sdbSizerDlgButtonsCancel;
	wxStaticBoxSizer* sbSizerInFilter;
	wxStaticBoxSizer* sbSizerOutFilter;
	wxCheckBox* m_cbAPBMagnetic;

	SentenceListDlg* m_stcdialog_in;
	SentenceListDlg* m_stcdialog_out;

	// Virtual event handlers, overide them in your derived class
	void OnSelectDatasource(wxListEvent& event);
	void OnAddDatasourceClick(wxCommandEvent& event);
	void OnRemoveDatasourceClick(wxCommandEvent& event);
	void OnTypeSerialSelected(wxCommandEvent& event);
	void OnTypeNetSelected(wxCommandEvent& event);
	void OnNetProtocolSelected(wxCommandEvent& event);
	void OnBaudrateChoice(wxCommandEvent& event);
	void OnProtocolChoice(wxCommandEvent& event);
	void OnCrcCheck(wxCommandEvent& event);
	void OnRbAcceptInput(wxCommandEvent& event);
	void OnRbIgnoreInput(wxCommandEvent& event);
	void OnBtnIStcs(wxCommandEvent& event);
	void OnCbOutput(wxCommandEvent& event);
	void OnRbOutput(wxCommandEvent& event);
	void OnBtnOStcs(wxCommandEvent& event);
	void OnConnValChange(wxCommandEvent& event);
	void OnValChange(wxCommandEvent& event);
	void OnUploadFormatChange(wxCommandEvent& event);
	void OnConnectionToggleEnable(wxMouseEvent& event);

	bool connectionsaved;
	bool m_connection_enabled;

	// For "S57" page
	wxFlexGridSizer* vectorPanel;
	wxScrolledWindow* ps57Ctl;
	wxCheckListBox* ps57CtlListBox;
	wxRadioBox* pDispCat;
	wxButton* itemButtonClearList;
	wxButton* itemButtonSelectList;
	wxRadioBox* pPointStyle;
	wxRadioBox* pBoundStyle;
	wxRadioBox* p24Color;
	wxCheckBox* pCheck_SOUNDG;
	wxCheckBox* pCheck_META;
	wxCheckBox* pCheck_SHOWIMPTEXT;
	wxCheckBox* pCheck_SCAMIN;
	wxCheckBox* pCheck_ATONTEXT;
	wxCheckBox* pCheck_LDISTEXT;
	wxCheckBox* pCheck_XLSECTTEXT;
	wxCheckBox* pCheck_DECLTEXT;
	wxCheckBox* pCheck_NATIONALTEXT;
	wxTextCtrl* m_ShallowCtl;
	wxTextCtrl* m_SafetyCtl;
	wxTextCtrl* m_DeepCtl;
	wxRadioBox* pDepthUnitSelect;
	wxSlider* m_pSlider_CM93_Zoom;
	wxCheckBox* pSEnableCM93Offset;
	int k_vectorcharts;

	// For "Charts" page
	wxStaticBoxSizer* activeSizer;
	wxBoxSizer* chartPanel;
	wxTextCtrl* pSelCtl;
	wxListBox* pActiveChartsList;
	wxStaticBox* itemActiveChartStaticBox;
	wxCheckBox* pUpdateCheckBox;
	wxCheckBox* pScanCheckBox;
	int k_charts;

	// For "AIS" Page
	wxCheckBox* m_pCheck_CPA_Max;
	wxTextCtrl* m_pText_CPA_Max;
	wxCheckBox* m_pCheck_CPA_Warn;
	wxTextCtrl* m_pText_CPA_Warn;
	wxCheckBox* m_pCheck_CPA_WarnT;
	wxTextCtrl* m_pText_CPA_WarnT;
	wxCheckBox* m_pCheck_Mark_Lost;
	wxTextCtrl* m_pText_Mark_Lost;
	wxCheckBox* m_pCheck_Remove_Lost;
	wxTextCtrl* m_pText_Remove_Lost;
	wxCheckBox* m_pCheck_Show_COG;
	wxTextCtrl* m_pText_COG_Predictor;
	wxCheckBox* m_pCheck_Show_Tracks;
	wxTextCtrl* m_pText_Track_Length;
	wxCheckBox* m_pCheck_Show_Moored;
	wxTextCtrl* m_pText_Moored_Speed;
	wxCheckBox* m_pCheck_AlertDialog;
	wxCheckBox* m_pCheck_AlertAudio;
	wxCheckBox* m_pCheck_Alert_Moored;
	wxCheckBox* m_pCheck_Rollover_Class;
	wxCheckBox* m_pCheck_Rollover_COG;
	wxCheckBox* m_pCheck_Rollover_CPA;
	wxCheckBox* m_pCheck_Ack_Timout;
	wxTextCtrl* m_pText_ACK_Timeout;
	wxCheckBox* m_pCheck_Show_Area_Notices;
	wxCheckBox* m_pCheck_Draw_Target_Size;
	wxCheckBox* m_pCheck_Show_Target_Name;
	wxTextCtrl* m_pText_Show_Target_Name_Scale;
	wxCheckBox* m_pCheck_Wpl_Aprs;
	wxCheckBox* m_pCheck_ShowAllCPA;
	// For Ship page
	wxFlexGridSizer* realSizes;
	wxTextCtrl* m_pOSLength;
	wxTextCtrl* m_pOSWidth;
	wxTextCtrl* m_pOSGPSOffsetX;
	wxTextCtrl* m_pOSGPSOffsetY;
	wxTextCtrl* m_pOSMinSize;
	wxStaticBoxSizer* dispOptions;
	wxScrolledWindow* itemPanelShip;
	wxBoxSizer* ownShip;
	wxTextCtrl* m_pText_ACRadius;

	// For Fonts page
	wxBoxSizer* m_itemBoxSizerFontPanel;
	wxChoice* m_itemFontElementListBox;
	wxChoice* m_itemStyleListBox;
	wxChoice* m_itemLangListBox;
	bool m_bVisitLang;

	// For "AIS Options"
	wxComboBox* m_itemAISListBox;

	// For "PlugIns" Panel
	PluginListPanel* m_pPlugInCtrl;
	int k_plugins;

	wxChoice* pNavAidRadarRingsNumberVisible;
	wxFlexGridSizer* radarGrid;
	wxTextCtrl* pNavAidRadarRingsStep;
	wxChoice* m_itemRadarRingsUnits;
	wxCheckBox* pWayPointPreventDragging;
	wxCheckBox* pConfirmObjectDeletion;
	wxCheckBox* pEnableZoomToCursor;
	wxCheckBox* pPreserveScale;
	wxCheckBox* pPlayShipsBells;
	wxCheckBox* pFullScreenToolbar;
	wxCheckBox* pTransparentToolbar;
	wxChoice* pSDMMFormat;
	wxChoice* pDistanceFormat;
	wxChoice* pSpeedFormat;

	wxCheckBox* pTrackShowIcon;
	wxCheckBox* pTrackDaily;
	wxCheckBox* pTrackHighlite;
	wxChoice* pTrackPrecision;
	wxTextCtrl* m_pText_TP_Secs;
	wxTextCtrl* m_pText_TP_Dist;

	wxCheckBox* pSettingsCB1;

	Config* m_pConfig;

	wxString m_init_chart_dir;
	MainFrame* pParent;

	wxArrayString* m_pSerialArray;

private:
	double get_double(const wxTextCtrl*) const;

	void CreatePanel_AIS(size_t parent, int border_size, int group_item_spacing,
						 wxSize small_button_size);
	void CreatePanel_Ownship(size_t parent, int border_size, int group_item_spacing,
							 wxSize small_button_size);
	void CreatePanel_NMEA(size_t parent, int border_size, int group_item_spacing,
						  wxSize small_button_size);
	void CreatePanel_ChartsLoad(size_t parent, int border_size, int group_item_spacing,
								wxSize small_button_size);
	void CreatePanel_VectorCharts(size_t parent, int border_size, int group_item_spacing,
								  wxSize small_button_size);
	void CreatePanel_TidesCurrents(size_t parent, int border_size, int group_item_spacing,
								   wxSize small_button_size);
	void CreatePanel_ChartGroups(size_t parent, int border_size, int group_item_spacing,
								 wxSize small_button_size);
	void CreatePanel_Display(size_t parent, int border_size, int group_item_spacing,
							 wxSize small_button_size);
	void CreatePanel_UI(size_t parent, int border_size, int group_item_spacing,
						wxSize small_button_size);

	ChartDirectories m_CurrentDirList;
	ChartDirectories* m_pWorkDirList;

	int m_returnChanges;
	wxListBox* tcDataSelected;
	std::vector<int> marinersStdXref;
	ChartGroupsUI* groupsPanel;
	wxImageList* m_topImgList;

	wxScrolledWindow* m_pNMEAForm;
	void ShowNMEACommon(bool visible);
	void ShowNMEASerial(bool visible);
	void ShowNMEANet(bool visible);
	void SetNMEAFormToSerial();
	void SetNMEAFormToNet();
	void ClearNMEAForm();
	bool m_bNMEAParams_shown;

	void SetConnectionParams(const ConnectionParams& cp);
	void SetDefaultConnectionParams(void);
	void SetDSFormRWStates();
	void FillSourceList();

	wxNotebookPage* m_groupsPage;

	bool CreateConnectionParamsFromSelectedItem(ConnectionParams&);
	ConnectionParams createConnectionParams() const;
	ConnectionParams::ConnectionType getConParamConnectionType() const;
	ConnectionParams::NetworkProtocol getConParamNetworkProtocol() const;
	ConnectionParams::ListType getConParamInputListType() const;
	ConnectionParams::ListType getConParamOutputListType() const;
};

#endif
