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

#ifndef __CHART__CHARTTYPE__H__
#define __CHART__CHARTTYPE__H__

namespace chart {

// ChartType constants
enum ChartTypeEnum
{
	CHART_TYPE_UNKNOWN = 0,
	CHART_TYPE_DUMMY,
	CHART_TYPE_DONTCARE,
	CHART_TYPE_KAP,
	CHART_TYPE_GEO,
	CHART_TYPE_S57,
	CHART_TYPE_CM93,
	CHART_TYPE_CM93COMP,
	CHART_TYPE_PLUGIN
};

}

#endif
