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

#include "ChartGroupsUI.h"
#include <OptionIDs.h>

#include <chart/ChartDatabase.h>

#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/dirctrl.h>

BEGIN_EVENT_TABLE(ChartGroupsUI, wxScrolledWindow)
	EVT_TREE_ITEM_EXPANDED(wxID_TREECTRL, ChartGroupsUI::OnNodeExpanded)
	EVT_BUTTON(ID_GROUPINSERTDIR, ChartGroupsUI::OnInsertChartItem)
	EVT_BUTTON(ID_GROUPREMOVEDIR, ChartGroupsUI::OnRemoveChartItem)
	EVT_NOTEBOOK_PAGE_CHANGED(ID_GROUPNOTEBOOK, ChartGroupsUI::OnGroupPageChange)
	EVT_BUTTON(ID_GROUPNEWGROUP, ChartGroupsUI::OnNewGroup)
	EVT_BUTTON(ID_GROUPDELETEGROUP, ChartGroupsUI::OnDeleteGroup)
END_EVENT_TABLE()

ChartGroupsUI::ChartGroupsUI(wxWindow* parent)
	: modified(false)
	, m_UIcomplete(false)
	, m_settingscomplete(false)
	, m_treespopulated(false)
	, m_pAddButton(NULL)
	, m_pRemoveButton(NULL)
	, m_pNewGroupButton(NULL)
	, m_pDeleteGroupButton(NULL)
	, allAvailableCtl(NULL)
	, defaultAllCtl(NULL)
	, m_pActiveChartsTree(0)
	, lastSelectedCtl(NULL)
	, m_GroupNB(NULL)
	, m_GroupSelectedPage(-1)
	, iFont(NULL)
	, m_pGroupArray(NULL)
{
	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _("Chart Groups"));
}

ChartGroupsUI::~ChartGroupsUI()
{
	m_DirCtrlArray.clear();
}

void ChartGroupsUI::SetDBDirs(ArrayOfCDI& array)
{
	m_db_dirs = array;
}

void ChartGroupsUI::SetGroupArray(chart::ChartGroupArray* pGroupArray)
{
	m_pGroupArray = pGroupArray;
}

void ChartGroupsUI::SetInitialSettings()
{
	m_settingscomplete = false;
	m_treespopulated = false;
}

void ChartGroupsUI::PopulateTrees()
{
	// Fill in the "Active chart" tree control
	// from the options dialog "Active Chart Directories" list
	wxArrayString dir_array;
	for (ArrayOfCDI::const_iterator i = m_db_dirs.begin(); i != m_db_dirs.end(); ++i) {
		if (!i->fullpath.IsEmpty())
			dir_array.push_back(i->fullpath);
	}

	PopulateTreeCtrl(allAvailableCtl->GetTreeCtrl(), dir_array, wxColour(0, 0, 0));
	m_pActiveChartsTree = allAvailableCtl->GetTreeCtrl();

	// Fill in the Page 0 tree control
	// from the options dialog "Active Chart Directories" list
	wxArrayString dir_array0;
	for (ArrayOfCDI::const_iterator i = m_db_dirs.begin(); i != m_db_dirs.end(); ++i) {
		if (!i->fullpath.IsEmpty())
			dir_array0.push_back(i->fullpath);
	}
	PopulateTreeCtrl(defaultAllCtl->GetTreeCtrl(), dir_array0, wxColour(128, 128, 128), iFont);
}

void ChartGroupsUI::CompleteInitialSettings()
{
	PopulateTrees();

	BuildNotebookPages(m_pGroupArray);

	groupsSizer->Layout();

	m_settingscomplete = true;
	m_treespopulated = true;
}

void ChartGroupsUI::PopulateTreeCtrl(
		wxTreeCtrl * ptc,
		const wxArrayString & dir_array,
		const wxColour & col,
		wxFont * pFont)
{
	ptc->DeleteAllItems();

	wxDirItemData* rootData = new wxDirItemData(_T("Dummy"), _T("Dummy"), true);
	wxString rootName;
	rootName = _T("Dummy");
	wxTreeItemId m_rootId = ptc->AddRoot(rootName, 3, -1, rootData);
	ptc->SetItemHasChildren(m_rootId);

	wxString dirname;
	int nDir = dir_array.size();
	for (int i = 0; i < nDir; i++) {
		wxString dirname = dir_array.Item(i);
		if (!dirname.IsEmpty()) {
			wxDirItemData* dir_item = new wxDirItemData(dirname, dirname, true);
			wxTreeItemId id = ptc->AppendItem(m_rootId, dirname, 0, -1, dir_item);

			// wxWidgets bug workaraound (Ticket #10085)
			ptc->SetItemText(id, dirname);
			if (pFont)
				ptc->SetItemFont(id, *pFont);
			ptc->SetItemTextColour(id, col);
			ptc->SetItemHasChildren(id);
		}
	}
}

void ChartGroupsUI::OnInsertChartItem(wxCommandEvent&)
{
	wxString insert_candidate = allAvailableCtl->GetPath();
	if (!insert_candidate.IsEmpty()) {
		if (m_DirCtrlArray.size()) {
			const wxGenericDirCtrl* pDirCtrl = m_DirCtrlArray.at(m_GroupSelectedPage);
			chart::ChartGroup* pGroup = m_pGroupArray->at(m_GroupSelectedPage - 1);
			if (pDirCtrl) {
				wxTreeCtrl* ptree = pDirCtrl->GetTreeCtrl();
				if (ptree) {
					if (ptree->IsEmpty()) {
						wxDirItemData* rootData
							= new wxDirItemData(wxEmptyString, wxEmptyString, true);
						wxString rootName = _T("Dummy");
						wxTreeItemId rootId = ptree->AddRoot(rootName, 3, -1, rootData);

						ptree->SetItemHasChildren(rootId);
					}

					wxTreeItemId root_Id = ptree->GetRootItem();
					wxDirItemData* dir_item
						= new wxDirItemData(insert_candidate, insert_candidate, true);
					wxTreeItemId id = ptree->AppendItem(root_Id, insert_candidate, 0, -1, dir_item);
					if (wxDir::Exists(insert_candidate))
						ptree->SetItemHasChildren(id);
				}

				chart::ChartGroupElement* pnew_element = new chart::ChartGroupElement;
				pnew_element->m_element_name = insert_candidate;
				pGroup->m_element_array.push_back(pnew_element);
			}
		}
	}
	modified = true;
	allAvailableCtl->GetTreeCtrl()->UnselectAll();
	m_pAddButton->Disable();
}

void ChartGroupsUI::OnRemoveChartItem(wxCommandEvent& event)
{
	if (m_DirCtrlArray.size()) {
		const wxGenericDirCtrl* pDirCtrl = m_DirCtrlArray.at(m_GroupSelectedPage);
		chart::ChartGroup* pGroup = m_pGroupArray->at(m_GroupSelectedPage - 1);

		if (pDirCtrl) {
			wxString sel_item = pDirCtrl->GetPath();

			wxTreeCtrl* ptree = pDirCtrl->GetTreeCtrl();
			if (ptree && ptree->GetCount()) {
				wxTreeItemId id = ptree->GetSelection();
				lastDeletedItem = id;
				if (id.IsOk()) {
					wxString branch_adder;
					int group_item_index = FindGroupBranch(pGroup, ptree, id, &branch_adder);
					if (group_item_index >= 0) {
						chart::ChartGroupElement* pelement
							= pGroup->m_element_array.at(group_item_index);
						bool b_duplicate = false;
						for (unsigned int k = 0; k < pelement->missing_names.size(); k++) {
							if (pelement->missing_names.at(k) == sel_item) {
								b_duplicate = true;
								break;
							}
						}
						if (!b_duplicate) {
							pelement->missing_names.push_back(sel_item);
						}

						// Special case...
						// If the selection is a branch itself,
						// Then delete from the tree, and delete from the group
						if (branch_adder == _T("")) {
							ptree->Delete(id);
							pGroup->m_element_array.erase(pGroup->m_element_array.begin() + group_item_index);
						} else {
							ptree->SetItemTextColour(id, wxColour(128, 128, 128));
							//   what about toggle back?
						}
					}
				}
				modified = true;
				lastSelectedCtl->Unselect();
				m_pRemoveButton->Disable();
				wxLogMessage(_T("Disable"));
			}
		}
	}
	event.Skip();
}

void ChartGroupsUI::OnGroupPageChange(wxNotebookEvent& event)
{
	m_GroupSelectedPage = event.GetSelection();
	allAvailableCtl->GetTreeCtrl()->UnselectAll();
	if (lastSelectedCtl)
		lastSelectedCtl->UnselectAll();
	m_pRemoveButton->Disable();
	m_pAddButton->Disable();
}

void ChartGroupsUI::OnAvailableSelection(wxTreeEvent& event)
{
	if (allAvailableCtl && (event.GetEventObject() == allAvailableCtl->GetTreeCtrl())) {
		wxTreeItemId item = allAvailableCtl->GetTreeCtrl()->GetSelection();
		if (item && item.IsOk() && m_GroupSelectedPage > 0) {
			m_pAddButton->Enable();
		} else {
			m_pAddButton->Disable();
		}
	} else {
		lastSelectedCtl = (wxTreeCtrl*)event.GetEventObject();
		wxTreeItemId item = lastSelectedCtl->GetSelection();
		if (item && item.IsOk() && m_GroupSelectedPage > 0) {
			// We need a trick for wxGTK here, since it gives us a Selection event with
			// the just deleted empty element after OnRemoveChartItem()
			wxString itemPath = ((wxGenericDirCtrl*)lastSelectedCtl->GetParent())->GetPath();
			if (itemPath.Length())
				m_pRemoveButton->Enable();
		} else {
			m_pRemoveButton->Disable();
		}
	}
	event.Skip();
}

void ChartGroupsUI::OnNewGroup(wxCommandEvent&)
{
	wxTextEntryDialog* pd
		= new wxTextEntryDialog(this, _("Enter Group Name"), _("New Chart Group"));

	if (pd->ShowModal() == wxID_OK) {
		AddEmptyGroupPage(pd->GetValue());
		chart::ChartGroup* pGroup = new chart::ChartGroup;
		pGroup->m_group_name = pd->GetValue();
		if (m_pGroupArray)
			m_pGroupArray->push_back(pGroup);

		m_GroupSelectedPage = m_GroupNB->GetPageCount() - 1; // select the new page
		m_GroupNB->ChangeSelection(m_GroupSelectedPage);
		modified = true;
	}
	delete pd;
}

void ChartGroupsUI::OnDeleteGroup(wxCommandEvent&)
{
	if (0 != m_GroupSelectedPage) {
		m_DirCtrlArray.erase(m_DirCtrlArray.begin() + m_GroupSelectedPage);
		if (m_pGroupArray)
			m_pGroupArray->erase(m_pGroupArray->begin() + m_GroupSelectedPage - 1);
		m_GroupNB->DeletePage(m_GroupSelectedPage);
		modified = true;
	}
}

int ChartGroupsUI::FindGroupBranch(chart::ChartGroup* pGroup, wxTreeCtrl* ptree, wxTreeItemId item,
								   wxString* pbranch_adder)
{
	wxString branch_name;
	wxString branch_adder;

	wxTreeItemId current_node = item;
	while (current_node.IsOk()) {

		wxTreeItemId parent_node = ptree->GetItemParent(current_node);
		if (!parent_node)
			break;

		if (parent_node == ptree->GetRootItem()) {
			branch_name = ptree->GetItemText(current_node);
			break;
		}

		branch_adder.Prepend(ptree->GetItemText(current_node));
		branch_adder.Prepend(wxString(wxFILE_SEP_PATH));

		current_node = ptree->GetItemParent(current_node);
	}

	// Find the index and element pointer of the target branch in the Group
	unsigned int target_item_index = -1;
	for (unsigned int i = 0; i < pGroup->m_element_array.size(); i++) {
		wxString target = pGroup->m_element_array.at(i)->m_element_name;
		if (branch_name == target) {
			target_item_index = i;
			break;
		}
	}

	if (pbranch_adder)
		*pbranch_adder = branch_adder;

	return target_item_index;
}

void ChartGroupsUI::OnNodeExpanded(wxTreeEvent& event)
{
	wxTreeItemId node = event.GetItem();

	if (m_GroupSelectedPage > 0) {
		const wxGenericDirCtrl* pDirCtrl = m_DirCtrlArray.at(m_GroupSelectedPage);
		chart::ChartGroup* pGroup = m_pGroupArray->at(m_GroupSelectedPage - 1);
		if (pDirCtrl) {
			wxTreeCtrl* ptree = pDirCtrl->GetTreeCtrl();

			wxString branch_adder;
			int target_item_index = FindGroupBranch(pGroup, ptree, node, &branch_adder);
			if (target_item_index >= 0) {
				chart::ChartGroupElement* target_element = pGroup->m_element_array.at(target_item_index);
				wxString branch_name = target_element->m_element_name;

				// Walk the children of the expanded node, marking any items which appear in
				// the "missing" list
				if ((target_element->missing_names.size())) {
					wxString full_root = branch_name;
					full_root += branch_adder;
					full_root += wxString(wxFILE_SEP_PATH);

					wxTreeItemIdValue cookie;
					wxTreeItemId child = ptree->GetFirstChild(node, cookie);
					while (child.IsOk()) {
						wxString target_string = full_root;
						target_string += ptree->GetItemText(child);

						const std::vector<wxString>& missing = target_element->missing_names;
						for (std::vector<wxString>::const_iterator k = missing.begin();
							 k != missing.end(); ++k) {
							if (*k == target_string) {
								ptree->SetItemTextColour(child, wxColour(128, 128, 128));
								break;
							}
						}
						child = ptree->GetNextChild(node, cookie);
					}
				}
			}
		}
	}
}

void ChartGroupsUI::BuildNotebookPages(chart::ChartGroupArray* pGroupArray)
{
	for (unsigned int i = 0; i < pGroupArray->size(); i++) {
		chart::ChartGroup* pGroup = pGroupArray->at(i);
		wxTreeCtrl* ptc = AddEmptyGroupPage(pGroup->m_group_name);

		wxString itemname;
		int nItems = pGroup->m_element_array.size();
		for (int i = 0; i < nItems; i++) {
			wxString itemname = pGroup->m_element_array.at(i)->m_element_name;
			if (!itemname.IsEmpty()) {
				wxDirItemData* dir_item = new wxDirItemData(itemname, itemname, true);
				wxTreeItemId id = ptc->AppendItem(ptc->GetRootItem(), itemname, 0, -1, dir_item);

				if (wxDir::Exists(itemname))
					ptc->SetItemHasChildren(id);
			}
		}
	}
}

wxTreeCtrl* ChartGroupsUI::AddEmptyGroupPage(const wxString& label)
{
	wxGenericDirCtrl* GroupDirCtl = new wxGenericDirCtrl(m_GroupNB, wxID_ANY, _T("TESTDIR"));
	m_GroupNB->AddPage(GroupDirCtl, label);

	wxTreeCtrl* ptree = GroupDirCtl->GetTreeCtrl();
	ptree->DeleteAllItems();

	wxDirItemData* rootData = new wxDirItemData(wxEmptyString, wxEmptyString, true);
	wxString rootName = _T("Dummy");
	wxTreeItemId rootId = ptree->AddRoot(rootName, 3, -1, rootData);
	ptree->SetItemHasChildren(rootId);

	m_DirCtrlArray.push_back(GroupDirCtl);

	return ptree;
}

void ChartGroupsUI::CreatePanel(
		size_t WXUNUSED(parent),
		int border_size,
		int group_item_spacing,
		wxSize WXUNUSED(small_button_size))
{
	modified = false;
	m_border_size = border_size;
	m_group_item_spacing = group_item_spacing;

	groupsSizer = new wxFlexGridSizer(4, 2, border_size, border_size);
	groupsSizer->AddGrowableCol(0);
	groupsSizer->AddGrowableRow(1, 1);
	groupsSizer->AddGrowableRow(3, 1);

	SetSizer(groupsSizer);

	m_UIcomplete = false;
}

void ChartGroupsUI::CompletePanel(void)
{
	// The chart file/dir tree
	wxStaticText* allChartsLabel = new wxStaticText(this, wxID_ANY, _("All Available Charts"));
	groupsSizer->Add(allChartsLabel, 0, wxTOP | wxRIGHT | wxLEFT, m_border_size);

	wxStaticText* dummy1 = new wxStaticText(this, -1, _T(""));
	groupsSizer->Add(dummy1);

	wxBoxSizer* activeListSizer = new wxBoxSizer(wxVERTICAL);
	groupsSizer->Add(activeListSizer, 1, wxALL | wxEXPAND, 5);

	allAvailableCtl = new wxGenericDirCtrl(this, ID_GROUPAVAILABLE, _T(""), wxDefaultPosition,
										   wxDefaultSize, wxVSCROLL);
	activeListSizer->Add(allAvailableCtl, 1, wxEXPAND);

	m_pAddButton = new wxButton(this, ID_GROUPINSERTDIR, _("Add"));
	m_pAddButton->Disable();
	m_pRemoveButton = new wxButton(this, ID_GROUPREMOVEDIR, _("Remove Chart"));
	m_pRemoveButton->Disable();

	wxBoxSizer* addRemove = new wxBoxSizer(wxVERTICAL);
	addRemove->Add(m_pAddButton, 0, wxALL | wxEXPAND, m_group_item_spacing);
	groupsSizer->Add(addRemove, 0, wxALL | wxEXPAND, m_border_size);

	//    Add the Groups notebook control
	wxStaticText* groupsLabel = new wxStaticText(this, wxID_ANY, _("Chart Groups"));
	groupsSizer->Add(groupsLabel, 0, wxTOP | wxRIGHT | wxLEFT, m_border_size);

	wxStaticText* dummy2 = new wxStaticText(this, -1, _T(""));
	groupsSizer->Add(dummy2);

	wxBoxSizer* nbSizer = new wxBoxSizer(wxVERTICAL);
	m_GroupNB = new wxNotebook(this, ID_GROUPNOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_TOP);
	nbSizer->Add(m_GroupNB, 1, wxEXPAND);
	groupsSizer->Add(nbSizer, 1, wxALL | wxEXPAND, m_border_size);

	//    Add default (always present) Default Chart Group
	wxPanel* allActiveGroup = new wxPanel(m_GroupNB, -1, wxDefaultPosition, wxDefaultSize);
	m_GroupNB->AddPage(allActiveGroup, _("All Charts"));

	wxBoxSizer* page0BoxSizer = new wxBoxSizer(wxHORIZONTAL);
	allActiveGroup->SetSizer(page0BoxSizer);

	defaultAllCtl = new wxGenericDirCtrl(allActiveGroup, -1, _T(""), wxDefaultPosition,
										 wxDefaultSize, wxVSCROLL);

	//    Set the Font for the All Active Chart Group tree to be italic, dimmed
	iFont = wxTheFontList->FindOrCreateFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC,
											wxFONTWEIGHT_LIGHT);

	page0BoxSizer->Add(defaultAllCtl, 1, wxALIGN_TOP | wxALL | wxEXPAND);

	m_DirCtrlArray.push_back(defaultAllCtl);

	//    Add the Chart Group (page) "New" and "Delete" buttons
	m_pNewGroupButton = new wxButton(this, ID_GROUPNEWGROUP, _("New Group..."));
	m_pDeleteGroupButton = new wxButton(this, ID_GROUPDELETEGROUP, _("Delete Group"));

	wxBoxSizer* newDeleteGrp = new wxBoxSizer(wxVERTICAL);
	groupsSizer->Add(newDeleteGrp, 0, wxALL, m_border_size);

	newDeleteGrp->AddSpacer(25);
	newDeleteGrp->Add(m_pNewGroupButton, 0, wxALL | wxEXPAND, m_group_item_spacing);
	newDeleteGrp->Add(m_pDeleteGroupButton, 0, wxALL | wxEXPAND, m_group_item_spacing);
	newDeleteGrp->AddSpacer(25);
	newDeleteGrp->Add(m_pRemoveButton, 0, wxALL | wxEXPAND, m_group_item_spacing);

	// Connect this last, otherwise handler is called before all objects are initialized.
	this->Connect(wxEVT_COMMAND_TREE_SEL_CHANGED,
				  wxTreeEventHandler(ChartGroupsUI::OnAvailableSelection), NULL, this);

	m_UIcomplete = true;
}

chart::ChartGroupArray* ChartGroupsUI::CloneChartGroupArray(chart::ChartGroupArray* s)
{
	chart::ChartGroupArray* clone = new chart::ChartGroupArray;

	for (chart::ChartGroupArray::const_iterator i = s->begin(); i != s->end(); ++i) {
		const chart::ChartGroup* group = *i;

		chart::ChartGroup* pdg = new chart::ChartGroup;
		pdg->m_group_name = group->m_group_name;

		for (unsigned int j = 0; j < group->m_element_array.size(); j++) {
			chart::ChartGroupElement* pde = new chart::ChartGroupElement;
			pde->m_element_name = group->m_element_array.at(j)->m_element_name;
			// FIXME: use iterators
			for (unsigned int k = 0;
				 k < group->m_element_array.at(j)->missing_names.size(); k++) {
				wxString missing_name = group->m_element_array.at(j)->missing_names.at(k);
				pde->missing_names.push_back(missing_name);
			}
			pdg->m_element_array.push_back(pde);
		}
		clone->push_back(pdg);
	}

	return clone;
}

void ChartGroupsUI::EmptyChartGroupArray(chart::ChartGroupArray* s)
{
	if (!s)
		return;

	for (chart::ChartGroupArray::iterator i = s->begin(); i != s->end(); ++i) {
		chart::ChartGroup* group = *i;

		// FIXME: move this to the chart group where it belongs
		for (chart::ChartGroupElementArray::iterator j = group->m_element_array.begin();
			 j != group->m_element_array.end(); ++j) {
			chart::ChartGroupElement* pe = *j;
			pe->missing_names.clear();
			delete pe;
		}
		delete group;
	}

	s->clear();
}

