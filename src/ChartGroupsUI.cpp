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

void ChartGroupsUI::SetGroupArray(ChartGroupArray* pGroupArray)
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
	int nDir = dir_array.GetCount();
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
			ChartGroup* pGroup = m_pGroupArray->Item(m_GroupSelectedPage - 1);
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

				ChartGroupElement* pnew_element = new ChartGroupElement;
				pnew_element->m_element_name = insert_candidate;
				pGroup->m_element_array.Add(pnew_element);
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
		ChartGroup* pGroup = m_pGroupArray->Item(m_GroupSelectedPage - 1);

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
						ChartGroupElement* pelement
							= pGroup->m_element_array.Item(group_item_index);
						bool b_duplicate = false;
						for (unsigned int k = 0; k < pelement->m_missing_name_array.GetCount();
							 k++) {
							if (pelement->m_missing_name_array.Item(k) == sel_item) {
								b_duplicate = true;
								break;
							}
						}
						if (!b_duplicate) {
							pelement->m_missing_name_array.Add(sel_item);
						}

						//    Special case...
						//    If the selection is a branch itself,
						//    Then delete from the tree, and delete from the group
						if (branch_adder == _T("")) {
							ptree->Delete(id);
							pGroup->m_element_array.RemoveAt(group_item_index);
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
		ChartGroup* pGroup = new ChartGroup;
		pGroup->m_group_name = pd->GetValue();
		if (m_pGroupArray)
			m_pGroupArray->Add(pGroup);

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
			m_pGroupArray->RemoveAt(m_GroupSelectedPage - 1);
		m_GroupNB->DeletePage(m_GroupSelectedPage);
		modified = true;
	}
}

int ChartGroupsUI::FindGroupBranch(ChartGroup* pGroup, wxTreeCtrl* ptree, wxTreeItemId item,
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
	for (unsigned int i = 0; i < pGroup->m_element_array.GetCount(); i++) {
		wxString target = pGroup->m_element_array.Item(i)->m_element_name;
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
		ChartGroup* pGroup = m_pGroupArray->Item(m_GroupSelectedPage - 1);
		if (pDirCtrl) {
			wxTreeCtrl* ptree = pDirCtrl->GetTreeCtrl();

			wxString branch_adder;
			int target_item_index = FindGroupBranch(pGroup, ptree, node, &branch_adder);
			if (target_item_index >= 0) {
				ChartGroupElement* target_element = pGroup->m_element_array.Item(target_item_index);
				wxString branch_name = target_element->m_element_name;

				//    Walk the children of the expanded node, marking any items which appear in
				//    the "missing" list
				if ((target_element->m_missing_name_array.GetCount())) {
					wxString full_root = branch_name;
					full_root += branch_adder;
					full_root += wxString(wxFILE_SEP_PATH);

					wxTreeItemIdValue cookie;
					wxTreeItemId child = ptree->GetFirstChild(node, cookie);
					while (child.IsOk()) {
						wxString target_string = full_root;
						target_string += ptree->GetItemText(child);

						for (unsigned int k = 0;
							 k < target_element->m_missing_name_array.GetCount(); k++) {
							if (target_element->m_missing_name_array.Item(k) == target_string) {
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

void ChartGroupsUI::BuildNotebookPages(ChartGroupArray* pGroupArray)
{
	for (unsigned int i = 0; i < pGroupArray->GetCount(); i++) {
		ChartGroup* pGroup = pGroupArray->Item(i);
		wxTreeCtrl* ptc = AddEmptyGroupPage(pGroup->m_group_name);

		wxString itemname;
		int nItems = pGroup->m_element_array.GetCount();
		for (int i = 0; i < nItems; i++) {
			wxString itemname = pGroup->m_element_array.Item(i)->m_element_name;
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

ChartGroupArray* ChartGroupsUI::CloneChartGroupArray(ChartGroupArray* s)
{
	ChartGroupArray* d = new ChartGroupArray;
	for (unsigned int i = 0; i < s->GetCount(); i++) {
		ChartGroup* psg = s->Item(i);
		ChartGroup* pdg = new ChartGroup;
		pdg->m_group_name = psg->m_group_name;

		for (unsigned int j = 0; j < psg->m_element_array.GetCount(); j++) {
			ChartGroupElement* pde = new ChartGroupElement;
			pde->m_element_name = psg->m_element_array.Item(j)->m_element_name;
			for (unsigned int k = 0;
				 k < psg->m_element_array.Item(j)->m_missing_name_array.GetCount(); k++) {
				wxString missing_name = psg->m_element_array.Item(j)->m_missing_name_array.Item(k);
				pde->m_missing_name_array.Add(missing_name);
			}
			pdg->m_element_array.Add(pde);
		}
		d->Add(pdg);
	}
	return d;
}

void ChartGroupsUI::EmptyChartGroupArray(ChartGroupArray* s)
{
	if (!s)
		return;
	for (unsigned int i = 0; i < s->GetCount(); i++) {
		ChartGroup* psg = s->Item(i);

		for (unsigned int j = 0; j < psg->m_element_array.GetCount(); j++) {
			ChartGroupElement* pe = psg->m_element_array.Item(j);
			pe->m_missing_name_array.Clear();
			psg->m_element_array.RemoveAt(j);
			delete pe;
		}
		s->RemoveAt(i);
		delete psg;
	}

	s->Clear();
}

