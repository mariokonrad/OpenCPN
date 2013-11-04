/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __SELECTITEM_H__
#define __SELECTITEM_H__

#include <list>

class SelectItem
{
public:
	SelectItem();
	~SelectItem();

	int GetUserData(void);
	void SetUserData(int data);

	float m_slat;
	float m_slon;
	float m_slat2;
	float m_slon2;
	int m_seltype;
	bool m_bIsSelected;

	const void* m_pData1; // FIXME: void*
	void* m_pData2; // FIXME: void*
	void* m_pData3; // FIXME: void*
	int m_Data4; // FIXME: Data4??
};

typedef std::list<SelectItem*> SelectableItemList;

#endif
