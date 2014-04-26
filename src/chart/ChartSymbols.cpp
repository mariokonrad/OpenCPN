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

// FIXME: clean up this fucking mess with gotos

#include "ChartSymbols.h"

#include <chart/ColorTable.h>

#include <tinyxml/tinyxml.h>

#include <wx/filename.h>
#include <wx/log.h>
#include <wx/image.h>

#include <cstdlib>

namespace chart {

//--------------------------------------------------------------------------------------
// The below data is global since there will ever only be one ChartSymbols instance,
// and some methods+data of class S52plib are needed inside ChartSymbol, and s2plib
// needs some methods from ChartSymbol. So s52plib only calls static methods in
// order to resolve circular include file dependencies.

wxBitmap ChartSymbols::rasterSymbols;
int ChartSymbols::rasterSymbolsLoadedColorMapNumber = -1;
wxString ChartSymbols::configFileDirectory;
ChartSymbols::ColorTables ChartSymbols::colorTables;
ChartSymbols::symbolGraphicsHashMap* ChartSymbols::symbolGraphicLocations = NULL;

using chart::SymbolSizeInfo;
using chart::OCPNPattern;
using chart::ChartSymbol;


ChartSymbols::ChartSymbols(void)
	: plib(NULL)
{
}

ChartSymbols::~ChartSymbols(void)
{
}

void ChartSymbols::clear_color_table()
{
	for (ColorTables::iterator i = colorTables.begin(); i != colorTables.end(); ++i) {
		ColorTable* table = *i;
		table->tableName.clear();
		table->colors.clear();
		table->wxColors.clear();
		delete table;
	}
	colorTables.clear();
}

void ChartSymbols::InitializeGlobals(void)
{
	clear_color_table();
	if (!symbolGraphicLocations)
		symbolGraphicLocations = new symbolGraphicsHashMap;
	rasterSymbolsLoadedColorMapNumber = -1;
}

void ChartSymbols::DeleteGlobals(void)
{
	delete symbolGraphicLocations;
	symbolGraphicLocations = NULL;

	clear_color_table();
}

template <typename T>
static void property_num_value(const TiXmlElement* node, const char* name, T& value)
{
	wxString property = wxString(node->Attribute(name), wxConvUTF8);
	long number = 0;
	property.ToLong(&number, 0);
	value = static_cast<T>(number);
}

void ChartSymbols::ProcessColorTables(TiXmlElement* colortableNodes)
{
	for (TiXmlNode* childNode = colortableNodes->FirstChild(); childNode;
		 childNode = childNode->NextSibling()) {
		TiXmlElement* child = childNode->ToElement();
		ColorTable* colortable = new ColorTable(wxString(child->Attribute("name"), wxConvUTF8));

		TiXmlElement* colorNode = child->FirstChild()->ToElement();

		while (colorNode) {
			S52color color;

			if (wxString(colorNode->Value(), wxConvUTF8) == _T("graphics-file")) {
				colortable->rasterFileName = wxString(colorNode->Attribute("name"), wxConvUTF8);
				goto next;
			} else {
				property_num_value(colorNode, "r", color.R);
				property_num_value(colorNode, "g", color.G);
				property_num_value(colorNode, "b", color.B);

				wxString key(colorNode->Attribute("name"), wxConvUTF8);
				strncpy(color.colName, key.char_str(), 5);
				color.colName[5] = 0;

				colortable->colors[key] = color;

				wxColour wxcolor(color.R, color.G, color.B);
				colortable->wxColors[key] = wxcolor;
			}
next:
			colorNode = colorNode->NextSiblingElement();
		}
		colorTables.push_back(colortable);
	}
}

void ChartSymbols::ProcessLookups(TiXmlElement* lookupNodes)
{
	chart::Lookup lookup;

	for (TiXmlNode* childNode = lookupNodes->FirstChild(); childNode;
		 childNode = childNode->NextSibling()) {
		TiXmlElement* child = childNode->ToElement();

		property_num_value(child, "id", lookup.id);
		property_num_value(child, "RCID", lookup.RCID);
		lookup.name = wxString(child->Attribute("name"), wxConvUTF8);
		lookup.attributeCodeArray = NULL;

		TiXmlElement* subNode = child->FirstChild()->ToElement();

		while (subNode) {
			wxString nodeType(subNode->Value(), wxConvUTF8);
			wxString nodeText(subNode->GetText(), wxConvUTF8);

			if (nodeType == _T("type")) {

				if (nodeText == _T("Area"))
					lookup.type = AREAS_T;
				else if (nodeText == _T("Line"))
					lookup.type = LINES_T;
				else
					lookup.type = POINT_T;

				goto nextNode;
			}

			if (nodeType == _T("disp-prio")) {
				lookup.displayPrio = PRIO_NODATA;
				if (nodeText == _T("Group 1"))
					lookup.displayPrio = PRIO_GROUP1;
				else if (nodeText == _T("Area 1"))
					lookup.displayPrio = PRIO_AREA_1;
				else if (nodeText == _T("Area 2"))
					lookup.displayPrio = PRIO_AREA_2;
				else if (nodeText == _T("Point Symbol"))
					lookup.displayPrio = PRIO_SYMB_POINT;
				else if (nodeText == _T("Line Symbol"))
					lookup.displayPrio = PRIO_SYMB_LINE;
				else if (nodeText == _T("Area Symbol"))
					lookup.displayPrio = PRIO_SYMB_AREA;
				else if (nodeText == _T("Routing"))
					lookup.displayPrio = PRIO_ROUTEING;
				else if (nodeText == _T("Hazards"))
					lookup.displayPrio = PRIO_HAZARDS;
				else if (nodeText == _T("Mariners"))
					lookup.displayPrio = PRIO_MARINERS;
				goto nextNode;
			}
			if (nodeType == _T("radar-prio")) {
				if (nodeText == _T("On Top"))
					lookup.radarPrio = RAD_OVER;
				else
					lookup.radarPrio = RAD_SUPP;
				goto nextNode;
			}
			if (nodeType == _T("table-name")) {
				if (nodeText == _T("Simplified"))
					lookup.tableName = SIMPLIFIED;
				else if (nodeText == _T("Lines"))
					lookup.tableName = LINES;
				else if (nodeText == _T("Plain"))
					lookup.tableName = PLAIN_BOUNDARIES;
				else if (nodeText == _T("Symbolized"))
					lookup.tableName = SYMBOLIZED_BOUNDARIES;
				else
					lookup.tableName = PAPER_CHART;
				goto nextNode;
			}
			if (nodeType == _T("display-cat")) {
				if (nodeText == _T("Displaybase"))
					lookup.displayCat = DISPLAYBASE;
				else if (nodeText == _T("Standard"))
					lookup.displayCat = STANDARD;
				else if (nodeText == _T("Other"))
					lookup.displayCat = OTHER;
				else if (nodeText == _T("Mariners"))
					lookup.displayCat = MARINERS_STANDARD;
				else
					lookup.displayCat = OTHER;
				goto nextNode;
			}
			if (nodeType == _T("comment")) {
				wxString comment(subNode->GetText(), wxConvUTF8);
				long value;
				comment.ToLong(&value, 0);
				lookup.comment = value;
				goto nextNode;
			}

			if (nodeType == _T("instruction")) {
				lookup.instruction = nodeText;
				lookup.instruction.Append('\037');
				goto nextNode;
			}
			if (nodeType == _T("attrib-code")) {
				if (!lookup.attributeCodeArray)
					lookup.attributeCodeArray = new wxArrayString();
				wxString value = wxString(subNode->GetText(), wxConvUTF8);
				if (value == _T("ORIENT"))
					value << _T(" ");
				lookup.attributeCodeArray->Add(value);
				goto nextNode;
			}

nextNode:
			subNode = subNode->NextSiblingElement();
		}

		BuildLookup(lookup);
	}
}

void ChartSymbols::BuildLookup(chart::Lookup& lookup)
{
	LUPrec* LUP = (LUPrec*)calloc(1, sizeof(LUPrec));
	plib->pAlloc->Add(LUP);

	LUP->RCID = lookup.RCID;
	LUP->nSequence = lookup.id;
	LUP->DISC = lookup.displayCat;
	LUP->FTYP = lookup.type;
	LUP->DPRI = lookup.displayPrio;
	LUP->RPRI = lookup.radarPrio;
	LUP->TNAM = lookup.tableName;
	LUP->OBCL[6] = 0;
	strncpy(LUP->OBCL, lookup.name.mb_str(), 7);

	LUP->ATTCArray = lookup.attributeCodeArray;

	LUP->INST = new wxString(lookup.instruction);
	LUP->LUCM = lookup.comment;

	// Add LUP to array
	// Search the LUPArray to see if there is already a LUP with this RCID
	// If found, replace it with the new LUP
	// This provides a facility for updating the LUP tables after loading a basic set

	unsigned int index = 0;
	wxArrayOfLUPrec* pLUPARRAYtyped = plib->SelectLUPARRAY(LUP->TNAM);

	while (index < pLUPARRAYtyped->size()) {
		LUPrec* pLUPCandidate = pLUPARRAYtyped->Item(index);
		if (LUP->RCID == pLUPCandidate->RCID) {
			plib->DestroyLUP(pLUPCandidate); // empties the LUP
			pLUPARRAYtyped->Remove(pLUPCandidate);
			break;
		}
		index++;
	}

	pLUPARRAYtyped->Add(LUP);
}

void ChartSymbols::ProcessVectorTag(TiXmlElement* vectorNode, SymbolSizeInfo& vectorSize)
{
	property_num_value(vectorNode, "width", vectorSize.size.x);
	property_num_value(vectorNode, "height", vectorSize.size.y);

	TiXmlElement* vectorNodes = vectorNode->FirstChild()->ToElement();

	while (vectorNodes) {
		wxString nodeType(vectorNodes->Value(), wxConvUTF8);

		if (nodeType == _T("distance")) {
			property_num_value(vectorNodes, "min", vectorSize.minDistance);
			property_num_value(vectorNodes, "max", vectorSize.maxDistance);
			goto nextVector;
		}
		if (nodeType == _T("origin")) {
			property_num_value(vectorNodes, "x", vectorSize.origin.x);
			property_num_value(vectorNodes, "y", vectorSize.origin.y);
			goto nextVector;
		}
		if (nodeType == _T("pivot")) {
			property_num_value(vectorNodes, "x", vectorSize.pivot.x);
			property_num_value(vectorNodes, "y", vectorSize.pivot.y);
			goto nextVector;
		}
nextVector:
		vectorNodes = vectorNodes->NextSiblingElement();
	}
}

void ChartSymbols::ProcessLinestyles(TiXmlElement* linestyleNodes)
{
	chart::LineStyle lineStyle;

	for (TiXmlNode* childNode = linestyleNodes->FirstChild(); childNode;
		 childNode = childNode->NextSibling()) {
		TiXmlElement* child = childNode->ToElement();

		property_num_value(child, "RCID", lineStyle.RCID);

		TiXmlElement* subNode = child->FirstChild()->ToElement();

		while (subNode) {
			wxString nodeType(subNode->Value(), wxConvUTF8);
			wxString nodeText(subNode->GetText(), wxConvUTF8);

			if (nodeType == _T("description")) {
				lineStyle.description = nodeText;
				goto nextNode;
			}
			if (nodeType == _T("name")) {
				lineStyle.name = nodeText;
				goto nextNode;
			}
			if (nodeType == _T("color-ref")) {
				lineStyle.colorRef = nodeText;
				goto nextNode;
			}
			if (nodeType == _T("HPGL")) {
				lineStyle.HPGL = nodeText;
				goto nextNode;
			}
			if (nodeType == _T("vector")) {
				ProcessVectorTag(subNode, lineStyle.vectorSize);
			}
nextNode:
			subNode = subNode->NextSiblingElement();
		}

		BuildLineStyle(lineStyle);
	}
}

void ChartSymbols::BuildLineStyle(chart::LineStyle& lineStyle)
{
	Rule* lnstmp = NULL;
	Rule* lnst = (Rule*)calloc(1, sizeof(Rule));
	plib->pAlloc->Add(lnst);

	lnst->RCID = lineStyle.RCID;
	strncpy(lnst->name.PANM, lineStyle.name.mb_str(), 8);
	lnst->bitmap.PBTM = NULL;

	lnst->vector.LVCT = (char*)malloc(lineStyle.HPGL.Len() + 1);
	strcpy(lnst->vector.LVCT, lineStyle.HPGL.mb_str());

	lnst->colRef.LCRF = (char*)malloc(lineStyle.colorRef.Len() + 1);
	strcpy(lnst->colRef.LCRF, lineStyle.colorRef.mb_str());

	lnst->pos.line.minDist.PAMI = lineStyle.vectorSize.minDistance;
	lnst->pos.line.maxDist.PAMA = lineStyle.vectorSize.maxDistance;

	lnst->pos.line.pivot_x.PACL = lineStyle.vectorSize.pivot.x;
	lnst->pos.line.pivot_y.PARW = lineStyle.vectorSize.pivot.y;

	lnst->pos.line.bnbox_w.PAHL = lineStyle.vectorSize.size.x;
	lnst->pos.line.bnbox_h.PAVL = lineStyle.vectorSize.size.y;

	lnst->pos.line.bnbox_x.SBXC = lineStyle.vectorSize.origin.x;
	lnst->pos.line.bnbox_y.SBXR = lineStyle.vectorSize.origin.y;

	lnstmp = (*plib->_line_sym)[lineStyle.name];

	if (NULL == lnstmp)
		(*plib->_line_sym)[lineStyle.name] = lnst;
	else if (lnst->name.LINM != lnstmp->name.LINM)
		(*plib->_line_sym)[lineStyle.name] = lnst;
}

void ChartSymbols::ProcessPatterns(TiXmlElement* patternNodes)
{
	OCPNPattern pattern;

	for (TiXmlNode* childNode = patternNodes->FirstChild(); childNode;
		 childNode = childNode->NextSibling()) {
		TiXmlElement* child = childNode->ToElement();

		property_num_value(child, "RCID", pattern.RCID);

		pattern.hasVector = false;
		pattern.hasBitmap = false;
		pattern.preferBitmap = true;

		TiXmlElement* subNodes = child->FirstChild()->ToElement();

		while (subNodes) {
			wxString nodeType(subNodes->Value(), wxConvUTF8);
			wxString nodeText(subNodes->GetText(), wxConvUTF8);

			if (nodeType == _T("description")) {
				pattern.description = nodeText;
				goto nextNode;
			}
			if (nodeType == _T("name")) {
				pattern.name = nodeText;
				goto nextNode;
			}
			if (nodeType == _T("filltype")) {
				pattern.fillType = (subNodes->GetText())[0];
				goto nextNode;
			}
			if (nodeType == _T("spacing")) {
				pattern.spacing = (subNodes->GetText())[0];
				goto nextNode;
			}
			if (nodeType == _T("color-ref")) {
				pattern.colorRef = nodeText;
				goto nextNode;
			}
			if (nodeType == _T("definition")) {
				if (!strcmp(subNodes->GetText(), "V"))
					pattern.hasVector = true;
				goto nextNode;
			}
			if (nodeType == _T("prefer-bitmap")) {
				if (nodeText.Lower() == _T("no"))
					pattern.preferBitmap = false;
				if (nodeText.Lower() == _T("false"))
					pattern.preferBitmap = false;
				goto nextNode;
			}
			if (nodeType == _T("bitmap")) {
				property_num_value(subNodes, "width", pattern.bitmapSize.size.x);
				property_num_value(subNodes, "height", pattern.bitmapSize.size.y);
				pattern.hasBitmap = true;

				TiXmlElement* bitmapNodes = subNodes->FirstChild()->ToElement();
				while (bitmapNodes) {
					wxString bitmapnodeType(bitmapNodes->Value(), wxConvUTF8);

					if (bitmapnodeType == _T("distance")) {
						property_num_value(bitmapNodes, "min", pattern.bitmapSize.minDistance);
						property_num_value(bitmapNodes, "max", pattern.bitmapSize.maxDistance);
						goto nextBitmap;
					}
					if (bitmapnodeType == _T("origin")) {
						property_num_value(bitmapNodes, "x", pattern.bitmapSize.origin.x);
						property_num_value(bitmapNodes, "y", pattern.bitmapSize.origin.y);
						goto nextBitmap;
					}
					if (bitmapnodeType == _T("pivot")) {
						property_num_value(bitmapNodes, "x", pattern.bitmapSize.pivot.x);
						property_num_value(bitmapNodes, "y", pattern.bitmapSize.pivot.y);
						goto nextBitmap;
					}
					if (bitmapnodeType == _T("graphics-location")) {
						property_num_value(bitmapNodes, "x", pattern.bitmapSize.graphics.x);
						property_num_value(bitmapNodes, "y", pattern.bitmapSize.graphics.y);
					}
				nextBitmap:
					bitmapNodes = bitmapNodes->NextSiblingElement();
				}
				goto nextNode;
			}
			if (nodeType == _T("HPGL")) {
				pattern.hasVector = true;
				pattern.HPGL = nodeText;
				goto nextNode;
			}
			if (nodeType == _T("vector")) {
				ProcessVectorTag(subNodes, pattern.vectorSize);
			}
nextNode:
			subNodes = subNodes->NextSiblingElement();
		}

		BuildPattern(pattern);
	}
}

void ChartSymbols::BuildPattern(OCPNPattern& pattern)
{
	Rule* pattmp = NULL;

	Rule* patt = (Rule*)calloc(1, sizeof(Rule));
	plib->pAlloc->Add(patt);

	patt->RCID = pattern.RCID;
	patt->exposition.PXPO = new wxString(pattern.description);
	strncpy(patt->name.PANM, pattern.name.mb_str(), 8);
	patt->bitmap.PBTM = NULL;
	patt->fillType.PATP = pattern.fillType;
	patt->spacing.PASP = pattern.spacing;

	patt->vector.PVCT = (char*)malloc(pattern.HPGL.Len() + 1);
	strcpy(patt->vector.PVCT, pattern.HPGL.mb_str());

	patt->colRef.PCRF = (char*)malloc(pattern.colorRef.Len() + 1);
	strcpy(patt->colRef.PCRF, pattern.colorRef.mb_str());

	SymbolSizeInfo patternSize;

	if (pattern.hasVector && !(pattern.preferBitmap && pattern.hasBitmap)) {
		patt->definition.PADF = 'V';
		patternSize = pattern.vectorSize;
	} else {
		patt->definition.PADF = 'R';
		patternSize = pattern.bitmapSize;
	}

	patt->pos.patt.minDist.PAMI = patternSize.minDistance;
	patt->pos.patt.maxDist.PAMA = patternSize.maxDistance;

	patt->pos.patt.pivot_x.PACL = patternSize.pivot.x;
	patt->pos.patt.pivot_y.PARW = patternSize.pivot.y;

	patt->pos.patt.bnbox_w.PAHL = patternSize.size.x;
	patt->pos.patt.bnbox_h.PAVL = patternSize.size.y;

	patt->pos.patt.bnbox_x.SBXC = patternSize.origin.x;
	patt->pos.patt.bnbox_y.SBXR = patternSize.origin.y;

	wxRect graphicsLocation(pattern.bitmapSize.graphics, pattern.bitmapSize.size);
	(*symbolGraphicLocations)[pattern.name] = graphicsLocation;

	// check if key already there

	pattmp = (*plib->_patt_sym)[pattern.name];

	if (NULL == pattmp) {
		(*plib->_patt_sym)[pattern.name] = patt; // insert in hash table
	} else { // already something here with same key...
		// if the pattern names are not identical
		if (patt->name.PANM != pattmp->name.PANM) {
			(*plib->_patt_sym)[pattern.name] = patt; // replace the pattern
			plib->DestroyPatternRuleNode(pattmp); // remember to free to replaced node
			// the node itself is destroyed as part of pAlloc
		}
	}
}

void ChartSymbols::ProcessSymbols(TiXmlElement* symbolNodes)
{
	ChartSymbol symbol;

	for (TiXmlNode* childNode = symbolNodes->FirstChild(); childNode;
		 childNode = childNode->NextSibling()) {
		TiXmlElement* child = childNode->ToElement();

		property_num_value(child, "RCID", symbol.RCID);

		symbol.hasVector = false;
		symbol.hasBitmap = false;
		symbol.preferBitmap = true;

		TiXmlElement* subNodes = child->FirstChild()->ToElement();

		while (subNodes) {
			wxString nodeType(subNodes->Value(), wxConvUTF8);
			wxString nodeText(subNodes->GetText(), wxConvUTF8);

			if (nodeType == _T("description")) {
				symbol.description = nodeText;
				goto nextNode;
			}
			if (nodeType == _T("name")) {
				symbol.name = nodeText;
				goto nextNode;
			}
			if (nodeType == _T("color-ref")) {
				symbol.colorRef = nodeText;
				goto nextNode;
			}
			if (nodeType == _T("definition")) {
				if (!strcmp(subNodes->GetText(), "V"))
					symbol.hasVector = true;
				goto nextNode;
			}
			if (nodeType == _T("HPGL")) {
				symbol.HPGL = nodeText;
				goto nextNode;
			}
			if (nodeType == _T("prefer-bitmap")) {
				if (nodeText.Lower() == _T("no"))
					symbol.preferBitmap = false;
				if (nodeText.Lower() == _T("false"))
					symbol.preferBitmap = false;
				goto nextNode;
			}
			if (nodeType == _T("bitmap")) {
				property_num_value(subNodes, "width", symbol.bitmapSize.size.x);
				property_num_value(subNodes, "height", symbol.bitmapSize.size.y);
				symbol.hasBitmap = true;

				TiXmlElement* bitmapNodes = subNodes->FirstChild()->ToElement();
				while (bitmapNodes) {
					wxString bitmapnodeType(bitmapNodes->Value(), wxConvUTF8);
					if (bitmapnodeType == _T("distance")) {
						property_num_value(bitmapNodes, "min", symbol.bitmapSize.minDistance);
						property_num_value(bitmapNodes, "max", symbol.bitmapSize.maxDistance);
						goto nextBitmap;
					}
					if (bitmapnodeType == _T("origin")) {
						property_num_value(bitmapNodes, "x", symbol.bitmapSize.origin.x);
						property_num_value(bitmapNodes, "y", symbol.bitmapSize.origin.y);
						goto nextBitmap;
					}
					if (bitmapnodeType == _T("pivot")) {
						property_num_value(bitmapNodes, "x", symbol.bitmapSize.pivot.x);
						property_num_value(bitmapNodes, "y", symbol.bitmapSize.pivot.y);
						goto nextBitmap;
					}
					if (bitmapnodeType == _T("graphics-location")) {
						property_num_value(bitmapNodes, "x", symbol.bitmapSize.graphics.x);
						property_num_value(bitmapNodes, "y", symbol.bitmapSize.graphics.y);
					}
				nextBitmap:
					bitmapNodes = bitmapNodes->NextSiblingElement();
				}
				goto nextNode;
			}
			if (nodeType == _T("vector")) {
				property_num_value(subNodes, "width", symbol.vectorSize.size.x);
				property_num_value(subNodes, "height", symbol.vectorSize.size.y);
				symbol.hasVector = true;

				TiXmlElement* vectorNodes = subNodes->FirstChild()->ToElement();
				while (vectorNodes) {
					wxString vectornodeType(vectorNodes->Value(), wxConvUTF8);
					if (vectornodeType == _T("distance")) {
						property_num_value(vectorNodes, "min", symbol.vectorSize.minDistance);
						property_num_value(vectorNodes, "max", symbol.vectorSize.maxDistance);
						goto nextVector;
					}
					if (vectornodeType == _T("origin")) {
						property_num_value(vectorNodes, "x", symbol.vectorSize.origin.x);
						property_num_value(vectorNodes, "y", symbol.vectorSize.origin.y);
						goto nextVector;
					}
					if (vectornodeType == _T("pivot")) {
						property_num_value(vectorNodes, "x", symbol.vectorSize.pivot.x);
						property_num_value(vectorNodes, "y", symbol.vectorSize.pivot.y);
						goto nextVector;
					}
					if (vectornodeType == _T("HPGL")) {
						symbol.HPGL = wxString(vectorNodes->GetText(), wxConvUTF8);
					}
				nextVector:
					vectorNodes = vectorNodes->NextSiblingElement();
				}
			}
		nextNode:
			subNodes = subNodes->NextSiblingElement();
		}
		BuildSymbol(symbol);
	}
}

void ChartSymbols::BuildSymbol(ChartSymbol& symbol)
{
	Rule* symb = (Rule*)calloc(1, sizeof(Rule));
	plib->pAlloc->Add(symb);

	wxString SVCT;
	wxString SCRF;

	symb->RCID = symbol.RCID;
	strncpy(symb->name.SYNM, symbol.name.char_str(), 8);

	symb->exposition.SXPO = new wxString(symbol.description);

	symb->vector.SVCT = (char*)malloc(symbol.HPGL.Len() + 1);
	strcpy(symb->vector.SVCT, symbol.HPGL.mb_str());

	symb->colRef.SCRF = (char*)malloc(symbol.colorRef.Len() + 1);
	strcpy(symb->colRef.SCRF, symbol.colorRef.mb_str());

	symb->bitmap.SBTM = NULL;

	SymbolSizeInfo symbolSize;

	if (symbol.hasVector && !(symbol.preferBitmap && symbol.hasBitmap)) {
		symb->definition.SYDF = 'V';
		symbolSize = symbol.vectorSize;
	} else {
		symb->definition.SYDF = 'R';
		symbolSize = symbol.bitmapSize;
	}

	symb->pos.symb.minDist.PAMI = symbolSize.minDistance;
	symb->pos.symb.maxDist.PAMA = symbolSize.maxDistance;

	symb->pos.symb.pivot_x.SYCL = symbolSize.pivot.x;
	symb->pos.symb.pivot_y.SYRW = symbolSize.pivot.y;

	symb->pos.symb.bnbox_w.SYHL = symbolSize.size.x;
	symb->pos.symb.bnbox_h.SYVL = symbolSize.size.y;

	symb->pos.symb.bnbox_x.SBXC = symbolSize.origin.x;
	symb->pos.symb.bnbox_y.SBXR = symbolSize.origin.y;

	wxRect graphicsLocation(symbol.bitmapSize.graphics, symbol.bitmapSize.size);
	(*symbolGraphicLocations)[symbol.name] = graphicsLocation;

	// Already something here with same key? Then free its strings, otherwise they leak.
	Rule* symbtmp = (*plib->_symb_sym)[symbol.name];
	if (symbtmp) {
		free(symbtmp->colRef.SCRF);
		free(symbtmp->vector.SVCT);
		delete symbtmp->exposition.SXPO;
	}

	(*plib->_symb_sym)[symbol.name] = symb;
}

bool ChartSymbols::LoadConfigFile(s52plib* plibArg, const wxString& s52ilePath)
{
	TiXmlDocument doc;

	plib = plibArg;

	// Expect to find library data XML file in same folder as other S52 data.
	// Files in CWD takes precedence.

	wxString name, extension;
	wxString xmlFileName = _T("chartsymbols.xml");

	wxFileName::SplitPath(s52ilePath, &configFileDirectory, &name, &extension);
	wxString fullFilePath = configFileDirectory + wxFileName::GetPathSeparator() + xmlFileName;

	if (wxFileName::FileExists(xmlFileName)) {
		fullFilePath = xmlFileName;
		configFileDirectory = _T(".");
	}

	if (!wxFileName::FileExists(fullFilePath)) {
		wxString msg(_T("ChartSymbols ConfigFile not found: "));
		msg += fullFilePath;
		wxLogMessage(msg);
		return false;
	}

	if (!doc.LoadFile((const char*)fullFilePath.mb_str())) {
		wxString msg(_T("    ChartSymbols ConfigFile Failed to load "));
		msg += fullFilePath;
		wxLogMessage(msg);
		return false;
	}

	wxString msg(_T("ChartSymbols loaded from "));
	msg += fullFilePath;
	wxLogMessage(msg);

	TiXmlHandle hRoot(doc.RootElement());

	wxString root = wxString(doc.RootElement()->Value(), wxConvUTF8);
	if (root != _T("chartsymbols" )) {
		wxLogMessage(
			_T("    ChartSymbols::LoadConfigFile(): Expected XML Root <chartsymbols> not found."));
		return false;
	}

	TiXmlElement* pElem = hRoot.FirstChild().Element();

	for (; pElem != 0; pElem = pElem->NextSiblingElement()) {
		wxString child = wxString(pElem->Value(), wxConvUTF8);

		if (child == _T("color-tables"))
			ProcessColorTables(pElem);
		if (child == _T("lookups"))
			ProcessLookups(pElem);
		if (child == _T("line-styles"))
			ProcessLinestyles(pElem);
		if (child == _T("patterns"))
			ProcessPatterns(pElem);
		if (child == _T("symbols"))
			ProcessSymbols(pElem);
	}

	return true;
}

int ChartSymbols::LoadRasterFileForColorTable(int tableNo)
{
	if (tableNo == rasterSymbolsLoadedColorMapNumber)
		return true;

	ColorTable* coltab = colorTables.at(tableNo);

	wxString filename = configFileDirectory + wxFileName::GetPathSeparator()
						+ coltab->rasterFileName;

	wxImage rasterFileImg;
	if (rasterFileImg.LoadFile(filename, wxBITMAP_TYPE_PNG)) {
		rasterSymbols = wxBitmap(rasterFileImg, -1);
		rasterSymbolsLoadedColorMapNumber = tableNo;
		return true;
	}

	wxString msg(_T("ChartSymbols...Failed to load raster symbols file "));
	msg += filename;
	wxLogMessage(msg);
	return false;
}

void ChartSymbols::add(ColorTable* table)
{
	if (!table)
		return;

	colorTables.push_back(table);
}

S52color* ChartSymbols::GetColor(const char* colorName, int fromTable)
{
	wxString key(colorName, wxConvUTF8, 5);
	ColorTable* colortable = colorTables.at(fromTable);
	return &(colortable->colors[key]);
}

wxColor ChartSymbols::GetwxColor(const wxString& colorName, int fromTable)
{
	ColorTable* colortable = colorTables.at(fromTable);
	return colortable->wxColors[colorName];
}

wxColor ChartSymbols::GetwxColor(const char* colorName, int fromTable)
{
	wxString key(colorName, wxConvUTF8, 5);
	return GetwxColor(key, fromTable);
}

int ChartSymbols::FindColorTable(const wxString& tableName)
{
	for (unsigned int i = 0; i < colorTables.size(); i++) {
		const ColorTable* ct = colorTables.at(i);
		if (tableName.IsSameAs(ct->tableName)) {
			return i;
		}
	}
	return 0;
}

wxString ChartSymbols::HashKey(const char* symbolName)
{
	char key[9];
	key[8] = 0;
	strncpy(key, symbolName, 8);
	return wxString(key, wxConvUTF8);
}

wxImage ChartSymbols::GetImage(const char* symbolName)
{
	wxRect bmArea = (*symbolGraphicLocations)[HashKey(symbolName)];
	wxBitmap bitmap = rasterSymbols.GetSubBitmap(bmArea);
	return bitmap.ConvertToImage();
}

}

