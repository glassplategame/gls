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

#include "die.h"
#include "global.h"
#include "gls.h"
#include "plate.h"

/**
 * Holds information about the actual game.
 */
struct board {
	// Plates are arrange in a static 8x8 square for now.
	struct plate plates[GLS_BOARD_ROW_COUNT][GLS_BOARD_COLUMN_COUNT];
	// Game dice.
	struct die dice[GLS_DIE_MAX];
};

/**
 * Attempts to place a die at the specified location on the board with the
 * specified color.  Sets 'color' and 'die' as per the corresponding, suffixed
 * '_check' function. */
struct flub* board_die_place(struct board* board, char* nick, char* location,
	uint32_t* color, uint32_t* die);

/**
 * Check if a die may be placed at the specified location and with the given
 * color.  Outputs the color that will be used (if specified as the special
 * "null" color) and die that will be used.
 */
struct flub* board_die_place_check(struct board* board, char* location,
	uint32_t* color, uint32_t* die);

/**
 * Initialize the game board.
 */
void board_init(struct board* board);

/**
 * Pretty-print the board to the specified file descriptor.
 */
struct flub* board_print(struct board* board, int fd);

#endif // board_H
