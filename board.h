/**
 *  Holds state for what would traditionally be the visible portion of the game.
 *
 *  Copyright (C) 2015  "Frostsnow" (Wade T. Cline).
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef board_H
#define board_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "global.h"
#include "gls.h"
#include "plate.h"

/**
 * Holds information about the actual game.
 */
struct board {
	// Plates are arrange in a static 8x8 square for now.
	struct plate plates[GLS_BOARD_ROW_COUNT][GLS_BOARD_COLUMN_COUNT];
};

/**
 * Initialize the game board.
 */
void board_init(struct board* board);

/**
 * Pretty-print the board to the specified file descriptor.
 */
struct flub* board_print(struct board* board, int fd);

#endif // board_H
