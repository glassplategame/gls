/**
 *  Usually implemented in meat-space as a small, thick, rectangular piece of
 *  paper.
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
#ifndef plate_H
#define plate_H

#include <stdlib.h>

#include "global.h"
#include "gls.h"

struct plate {
	// The plate's concept.
	char name[GLS_PLATE_NAME_LENGTH];
	// A short, three-letter abbreviation for the plate's name. Names are
	// not meant to be unique.
	char abbrev[GLS_PLATE_ABBREV_LENGTH];
	// Description of the plate's concept.
	char description[GLS_PLATE_DESCRIPTION_LENGTH];
	// Whether or not the plate is empty.
	uint32_t empty:1;
};

/**
 * Pretty-prints the plate to the specified file.
 */
struct flub* plate_print(struct plate* plate, int fd);

#endif // plate_H
