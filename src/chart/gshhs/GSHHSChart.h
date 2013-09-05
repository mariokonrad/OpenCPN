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

#ifndef __CHART__GSHHS__GSHHSCHART__H__
#define __CHART__GSHHS__GSHHSCHART__H__

#include <wx/colour.h>
#include <ColorScheme.h>

class ocpnDC;
class ViewPort;
class Projection;
class GshhsReader;

class GSHHSChart
{
	public:
		GSHHSChart();
		~GSHHSChart();
		void SetColorScheme(ColorScheme scheme);
		void RenderViewOnDC(ocpnDC & dc, ViewPort & VPoint);

	private:
		Projection* proj;
		GshhsReader* reader;
		wxColor land;
		wxColor water;
};

#endif
