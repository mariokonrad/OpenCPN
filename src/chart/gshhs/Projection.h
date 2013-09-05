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

#ifndef __CHART__GSSHS__PROJECTION__H__
#define __CHART__GSSHS__PROJECTION__H__

// Subset of original Projection, only whats needed for GSHHS.
class Projection
{
	public:
		Projection();
		Projection(int w, int h, double cx, double cy);
		void SetScreenSize(int w, int h);
		void SetCenterInMap(double x, double y);
		void SetScale(double sc);
		bool isInBounderies( int x, int y ) const;
		double getXmin() const { return xW; }
		double getXmax() const { return xE; }
		double getYmin() const { return yS; }
		double getYmax() const { return yN; }
		void map2screen( double wx, double wy, int* x, int* y ) const;
		void map2screenDouble( double wx, double wy, double* x, double* y ) const;
		bool intersect( double w, double e, double s, double n ) const;
		int getW() { return W; }
		int getH() { return H; }
		double getCoefremp() { return coefremp; }
		double degToRad( double d ) const;
		double radToDeg( double r ) const;

	private:
		void updateBoundaries();

		int W, H;
		double CX, CY;
		double xW, xE, yN, yS;  // fenetre visible (repere longitude/latitude)
		double PX,PY;       // center in mercator projection
		double scale;       // Echelle courante
		double scalemax;    // Echelle maxi
		double scaleall;    // Echelle pour afficher le monde entier
		double coefremp;       // Coefficient de remplissage (surface_visible/pixels)
		bool frozen;
		bool useTempo;
};

#endif
