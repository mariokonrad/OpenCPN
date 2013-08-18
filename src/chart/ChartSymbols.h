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

#ifndef __CHARTSYMBOLS__H__
#define __CHARTSYMBOLS__H__

#include "s52plib.h"
#include "chart/Lookup.h"
#include "chart/SymbolSizeInfo.h"
#include "chart/ChartSymbol.h"
#include "chart/OCPNPattern.h"
#include "chart/LineStyle.h"
#include <tinyxml/tinyxml.h>

// FIXME: this is essentially a singleton, but not implemented as one...
class ChartSymbols
{
	public:
		ChartSymbols(void);
		~ChartSymbols(void);
		bool LoadConfigFile(s52plib * plibArg, const wxString & path);

		static void InitializeGlobals(void);
		static void DeleteGlobals(void);
		static int LoadRasterFileForColorTable(int tableNo);
		static wxArrayPtrVoid * GetColorTables();
		static int FindColorTable(const wxString & tableName);
		static S52color* GetColor(const char * colorName, int fromTable);
		static wxColor GetwxColor(const wxString & colorName, int fromTable);
		static wxColor GetwxColor(const char * colorName, int fromTable);
		static wxString HashKey(const char * symbolName);
		static wxImage GetImage(const char * symbolName);

	private:
		void ProcessVectorTag(TiXmlElement * subNodes, SymbolSizeInfo & vectorSize);
		void ProcessColorTables(TiXmlElement * colortableodes);
		void ProcessLookups(TiXmlElement * lookupNodes);
		void ProcessLinestyles(TiXmlElement * linestyleNodes);
		void ProcessPatterns(TiXmlElement * patternNodes);
		void ProcessSymbols(TiXmlElement * symbolNodes);
		void BuildLineStyle(LineStyle & lineStyle);
		void BuildLookup(Lookup & lookup);
		void BuildPattern(OCPNPattern & pattern);
		void BuildSymbol(ChartSymbol & symol);

	private:
		static wxBitmap rasterSymbols;
		static int rasterSymbolsLoadedColorMapNumber;
		static wxString configFileDirectory;
		static wxArrayPtrVoid * colorTables;

		s52plib * plib;
};

#endif
