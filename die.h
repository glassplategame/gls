/**
 *  Dies which are placed on the game board.
 *
 *  Copyright (C) 2017  Wade T. Cline.
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
#ifndef DIE_H
#define DIE_H

#include "gls.h"

struct die {
	// Player that placed the die.
	char nick[GLS_NICK_LENGTH];
	// Location of the die.
	char location[GLS_LOCATION_LENGTH];
	// Transparency color associated with the die.
	uint32_t color;
};

#endif // DIE_H
