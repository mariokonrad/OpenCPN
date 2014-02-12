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

#ifndef __CHARTGROUPSUI__H__
#define __CHARTGROUPSUI__H__

#include <wx/scrolwin.h>
#include <wx/treebase.h>

#include <chart/ChartGroup.h>

#include <CDI.h>
#include <ChartDirectoryInfo.h>

#include <vector>

class wxTreeCtrl;
class wxButton;
class wxNotebook;
class wxNotebookEvent;
class wxGenericDirCtrl;
class wxFlexGridSizer;

class ChartGroupsUI : public wxScrolledWindow
{
	DECLARE_EVENT_TABLE()
	friend class options;

public:
	ChartGroupsUI(wxWindow* parent);
	virtual ~ChartGroupsUI();

	void CreatePanel(size_t parent, int border_size, int group_item_spacing,
					 wxSize small_button_size);

	void CompletePanel(void);
	void SetDBDirs(const ChartDirectories& array);
	void SetGroupArray(chart::ChartGroupArray* pGroupArray);
	void SetInitialSettings();
	void CompleteInitialSettings();
	void PopulateTrees();

	void PopulateTreeCtrl(wxTreeCtrl* ptc, const wxArrayString& dir_array, const wxColour& col,
						  wxFont* pFont = NULL);

	wxTreeCtrl* AddEmptyGroupPage(const wxString& label);

	void BuildNotebookPages(chart::ChartGroupArray* pGroupArray);
	chart::ChartGroupArray* CloneChartGroupArray(chart::ChartGroupArray* s);
	void EmptyChartGroupArray(chart::ChartGroupArray* s);

	void OnNodeExpanded(wxTreeEvent& event);
	void OnAvailableSelection(wxTreeEvent& event);
	void OnInsertChartItem(wxCommandEvent& event);
	void OnRemoveChartItem(wxCommandEvent& event);
	void OnGroupPageChange(wxNotebookEvent& event);
	void OnNewGroup(wxCommandEvent& event);
	void OnDeleteGroup(wxCommandEvent& event);

private:
	int FindGroupBranch(chart::ChartGroup* pGroup, wxTreeCtrl* ptree, wxTreeItemId item,
						wxString* pbranch_adder);

	bool modified;
	bool m_UIcomplete;
	bool m_settingscomplete;
	bool m_treespopulated;

	wxFlexGridSizer* groupsSizer;
	wxButton* m_pAddButton;
	wxButton* m_pRemoveButton;
	wxButton* m_pNewGroupButton;
	wxButton* m_pDeleteGroupButton;
	int m_border_size;
	int m_group_item_spacing;

	wxGenericDirCtrl* allAvailableCtl;
	wxGenericDirCtrl* defaultAllCtl;
	wxTreeCtrl* m_pActiveChartsTree;
	wxTreeCtrl* lastSelectedCtl;
	wxTreeItemId lastDeletedItem;
	wxNotebook* m_GroupNB;
	ChartDirectories m_db_dirs;
	int m_GroupSelectedPage;
	wxFont* iFont;

	typedef std::vector<wxGenericDirCtrl*> DirectoryControls;

	DirectoryControls m_DirCtrlArray;
	chart::ChartGroupArray* m_pGroupArray;
};

#endif
