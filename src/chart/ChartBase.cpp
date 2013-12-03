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

#include "ChartBase.h"
#include <wx/bitmap.h>

namespace chart {

ChartBase::ChartBase()
{
	pcached_bitmap = NULL;

	m_depth_unit_id = DEPTH_UNIT_UNKNOWN;

	pThumbData = new ThumbData;

	m_global_color_scheme = GLOBAL_COLOR_SCHEME_RGB;

	bReadyToRender = false;

	Chart_Error_Factor = 0;

	m_Chart_Scale = 10000; // a benign value
	m_Chart_Skew = 0.0;

	m_nCOVREntries = 0;
	m_pCOVRTable = NULL;
	m_pCOVRTablePoints = NULL;

	m_nNoCOVREntries = 0;
	m_pNoCOVRTable = NULL;
	m_pNoCOVRTablePoints = NULL;

	m_EdDate.Set(1, wxDateTime::Jan, 2000);

	m_lon_datum_adjust = 0.;
	m_lat_datum_adjust = 0.;

	m_projection = PROJECTION_MERCATOR; // default
}

ChartBase::~ChartBase()
{
	if (pcached_bitmap)
		delete pcached_bitmap;

	delete pThumbData;

	//    Free the COVR tables

	for (unsigned int j = 0; j < (unsigned int)m_nCOVREntries; ++j)
		free(m_pCOVRTable[j]);

	free(m_pCOVRTable);
	free(m_pCOVRTablePoints);
}

void ChartBase::Activate(void)
{}

void ChartBase::Deactivate(void)
{}

OcpnProjType ChartBase::GetChartProjectionType() const
{
	return m_projection;
}

wxDateTime ChartBase::GetEditionDate(void)
{
	return m_EdDate;
}

wxString ChartBase::GetPubDate() const
{
	return m_PubYear;
}

int ChartBase::GetNativeScale() const
{
	return m_Chart_Scale;
}

wxString ChartBase::GetFullPath() const
{
	return m_FullPath;
}

wxString ChartBase::GetName() const
{
	return m_Name;
}

wxString ChartBase::GetDescription() const
{
	return m_Description;
}

wxString ChartBase::GetID() const
{
	return m_ID;
}

wxString ChartBase::GetSE() const
{
	return m_SE;
}

wxString ChartBase::GetDepthUnits() const
{
	return m_DepthUnits;
}

wxString ChartBase::GetSoundingsDatum() const
{
	return m_SoundingsDatum;
}

wxString ChartBase::GetDatumString() const
{
	return m_datum_str;
}

wxString ChartBase::GetExtraInfo() const
{
	return m_ExtraInfo;
}

double ChartBase::GetChart_Error_Factor() const
{
	return Chart_Error_Factor;
}

ChartTypeEnum ChartBase::GetChartType() const
{
	return m_ChartType;
}

chart::ChartFamilyEnum ChartBase::GetChartFamily() const
{
	return m_ChartFamily;
}

double ChartBase::GetChartSkew() const
{
	return m_Chart_Skew;
}

ChartDepthUnitType ChartBase::GetDepthUnitType(void) const
{
	return m_depth_unit_id;
}

bool ChartBase::IsReadyToRender() const
{
	return bReadyToRender;
}

int ChartBase::GetCOVREntries() const
{
	return  m_nCOVREntries;
}

int ChartBase::GetCOVRTablePoints(int iTable) const
{
	return m_pCOVRTablePoints[iTable];
}

int ChartBase::GetCOVRTablenPoints(int iTable) const
{
	return m_pCOVRTablePoints[iTable];
}

float * ChartBase::GetCOVRTableHead(int iTable)
{
	return m_pCOVRTable[iTable];
}

int ChartBase::GetNoCOVREntries() const
{
	return  m_nNoCOVREntries;
}

int ChartBase::GetNoCOVRTablePoints(int iTable) const
{
	return m_pNoCOVRTablePoints[iTable];
}

int ChartBase::GetNoCOVRTablenPoints(int iTable) const
{
	return m_pNoCOVRTablePoints[iTable];
}

float * ChartBase::GetNoCOVRTableHead(int iTable)
{
	return m_pNoCOVRTable[iTable];
}

}

