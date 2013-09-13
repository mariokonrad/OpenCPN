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
#include "CDI.h"
#include <MainFrame.h>

class wxTreeCtrl;
class wxButton;
class wxNotebook;
class wxNotebookEvent;
class wxGenericDirCtrl;
class ChartGroupArray;
class ChartGroup;

WX_DECLARE_OBJARRAY(wxGenericDirCtrl *, ArrayOfDirCtrls);

class ChartGroupsUI : public wxScrolledWindow
{
		DECLARE_EVENT_TABLE()

	public:
		ChartGroupsUI(wxWindow * parent);
		virtual ~ChartGroupsUI();

		void CreatePanel(
				size_t parent,
				int border_size,
				int group_item_spacing,
				wxSize small_button_size);

		void CompletePanel(void);
		void SetDBDirs(ArrayOfCDI & array);
		void SetGroupArray(ChartGroupArray * pGroupArray);
		void SetInitialSettings();
		void CompleteInitialSettings();
		void PopulateTrees();

		void PopulateTreeCtrl(
				wxTreeCtrl * ptc,
				const wxArrayString & dir_array,
				const wxColour & col,
				wxFont * pFont = NULL);

		wxTreeCtrl * AddEmptyGroupPage(const wxString & label);

		void BuildNotebookPages(ChartGroupArray * pGroupArray);
		ChartGroupArray * CloneChartGroupArray(ChartGroupArray * s);
		void EmptyChartGroupArray(ChartGroupArray * s);

		void OnNodeExpanded(wxTreeEvent& event );
		void OnAvailableSelection(wxTreeEvent & event);
		void OnInsertChartItem(wxCommandEvent & event);
		void OnRemoveChartItem(wxCommandEvent & event);
		void OnGroupPageChange(wxNotebookEvent & event);
		void OnNewGroup(wxCommandEvent & event);
		void OnDeleteGroup(wxCommandEvent & event);

		bool modified;
		bool m_UIcomplete;
		bool m_settingscomplete;
		bool m_treespopulated;

	private:
		int FindGroupBranch(
				ChartGroup * pGroup,
				wxTreeCtrl * ptree,
				wxTreeItemId item,
				wxString * pbranch_adder);

		wxWindow * pParent;

		wxFlexGridSizer * groupsSizer;
		wxButton * m_pAddButton;
		wxButton * m_pRemoveButton;
		wxButton * m_pNewGroupButton;
		wxButton * m_pDeleteGroupButton;
		int m_border_size;
		int m_group_item_spacing;

		wxGenericDirCtrl * allAvailableCtl;
		wxGenericDirCtrl * defaultAllCtl;
		wxTreeCtrl * m_pActiveChartsTree;
		wxTreeCtrl * lastSelectedCtl;
		wxTreeItemId lastDeletedItem;
		wxNotebook * m_GroupNB;
		ArrayOfCDI m_db_dirs;
		int m_GroupSelectedPage;
		wxFont * iFont;

		ArrayOfDirCtrls m_DirCtrlArray;
		ChartGroupArray * m_pGroupArray;
};

#endif
