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

#include "plate.h"

struct flub* plate_print(struct plate* plate, int fd) {
	// Print plate info.
	if (plate->empty) {
		g_log_info("Location is empty");
		return NULL;
	}
	g_log_info("Name: %s", plate->name);
	g_log_info("Abbreviation: %s", plate->abbrev);
	g_log_info("Description: %s", plate->description);

	// Return success.
	return NULL;
}
