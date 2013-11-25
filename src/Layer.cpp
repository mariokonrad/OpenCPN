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

LayerList* pLayerList = NULL;

wxString GetLayerName(int id)
{
	wxString name(_T("unknown layer"));
	if (id <= 0)
		return name;

	for (LayerList::const_iterator it = pLayerList->begin(); it != pLayerList->end(); ++it) {
		if ((*it)->getID() == id)
			return (*it)->getName();
	}
	return name;
}

Layer* getLayerAtIndex(int index)
{
	if (index < 0)
		return NULL;
	if (index >= static_cast<int>(pLayerList->size()))
		return NULL;

	LayerList::iterator i = pLayerList->begin();
	std::advance(i, index);
	return *i;
}

Layer::Layer(int id, const wxString& filename, bool visible)
	: m_LayerID(id)
	, m_NoOfItems(0)
	, m_bHasVisibleNames(true)
	, m_bIsVisibleOnChart(visible)
	, m_bIsVisibleOnListing(false)
	, m_LayerName(_T(""))
	, m_LayerFileName(filename)
{
}

Layer::~Layer(void)
{
	// Remove this layer from the global layer list
	if (NULL != pLayerList)
		pLayerList->remove(this);
}

int Layer::getID() const
{
	return m_LayerID;
}

void Layer::setID(int id)
{
	m_LayerID = id;
}

const wxString& Layer::getName() const
{
	return m_LayerName;
}

void Layer::setName(const wxString& name)
{
	m_LayerName = name;
}

long Layer::getNoOfItems() const
{
	return m_NoOfItems;
}

void Layer::setNoOfItems(long number)
{
	m_NoOfItems = number;
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

