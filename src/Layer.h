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

#ifndef __LAYER_H__
#define __LAYER_H__

#include <wx/string.h>
#include <wx/list.h>
#include <wx/datetime.h>

class Layer
{
	public:
		Layer(void);
		~Layer(void);
		wxString CreatePropString(void);
		bool IsVisibleOnChart() const;
		void SetVisibleOnChart(bool viz = true);
		bool IsVisibleOnListing() const;
		void SetVisibleOnListing(bool viz = true);
		bool HasVisibleNames() const;
		void SetVisibleNames(bool viz = true);

		bool m_bIsVisibleOnChart;
		bool m_bIsVisibleOnListing;
		bool m_bHasVisibleNames;
		long m_NoOfItems;
		int m_LayerID;

		wxString m_LayerName;
		wxString m_LayerFileName;
		wxString m_LayerDescription;
		wxDateTime m_CreateTime;
};

WX_DECLARE_LIST(Layer, LayerList);

wxString GetLayerName(int id);
Layer * getLayerAtIndex(int index);

#endif
