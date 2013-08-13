/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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

#ifndef __UNDO__H__
#define __UNDO__H__

#include <deque>
#include "UndoAction.h"

class Undo
{
	public:
		Undo();
		~Undo();
		bool AnythingToUndo() const;
		bool AnythingToRedo() const;
		void InvalidateRedo();
		void InvalidateUndo();
		void Invalidate();
		bool InUndoableAction() const;
		UndoAction * GetNextUndoableAction();
		UndoAction * GetNextRedoableAction();
		bool UndoLastAction();
		bool RedoNextAction();
		bool BeforeUndoableAction(
				UndoAction::Type type,
				UndoAction::ItemPointer before,
				UndoAction::BeforePointerType beforeType,
				UndoAction::ItemPointer selectable);
		bool AfterUndoableAction(UndoAction::ItemPointer after);
		bool CancelUndoableAction(bool noDataDelete = false);

	private:
		void doUndoMoveWaypoint(UndoAction * action);
		void doUndoDeleteWaypoint(UndoAction * action);
		void doRedoDeleteWaypoint(UndoAction * action);
		void doUndoAppendWaypoint(UndoAction * action);
		void doRedoAppendWaypoint(UndoAction * action);

		bool isInsideUndoableAction;
		UndoAction * candidate;
		unsigned int stackpointer;
		unsigned int depthSetting;
		std::deque<UndoAction *> undoStack;
};

#endif
