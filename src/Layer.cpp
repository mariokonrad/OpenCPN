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

#include "Layer.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(LayerList);

extern bool g_bShowLayers;

LayerList * pLayerList = NULL;

wxString GetLayerName(int id)
{
	wxString name(_T("unknown layer"));
	if (id <= 0)
		return name;

	for (LayerList::const_iterator it = pLayerList->begin(); it != pLayerList->end(); ++it) {
		if ((*it)->m_LayerID == id)
			return (*it)->m_LayerName;
	}
	return name;
}

Layer * getLayerAtIndex(int index)
{
	return pLayerList->Item(index)->GetData();
}

Layer::Layer(void)
	: m_NoOfItems(0)
	, m_LayerName(_T(""))
	, m_LayerFileName(_T(""))
	, m_LayerDescription(_T(""))
	, m_bHasVisibleNames(true)
	, m_bIsVisibleOnListing(false)
{
	m_bIsVisibleOnChart = g_bShowLayers;
	m_CreateTime = wxDateTime::Now();
}

Layer::~Layer(void)
{
	// Remove this layer from the global layer list
	if (NULL != pLayerList)
		pLayerList->remove(this);
}

wxString Layer::CreatePropString(void)
{
	return m_LayerFileName;
}

bool Layer::IsVisibleOnChart() const
{
	return m_bIsVisibleOnChart;
}

void Layer::SetVisibleOnChart(bool viz)
{
	m_bIsVisibleOnChart = viz;
}

bool Layer::IsVisibleOnListing() const
{
	return m_bIsVisibleOnListing;
}

void Layer::SetVisibleOnListing(bool viz)
{
	m_bIsVisibleOnListing = viz;
}

bool Layer::HasVisibleNames() const
{
	return m_bHasVisibleNames;
}

void Layer::SetVisibleNames(bool viz)
{
	m_bHasVisibleNames = viz;
}

