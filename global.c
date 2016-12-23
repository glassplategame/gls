/**
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

#include "global.h"

G_LOG_DEFINITION(debug)
G_LOG_DEFINITION(info)
G_LOG_DEFINITION(warn)
G_LOG_DEFINITION(error)

struct flub* g_flub_toss(char* format, ...) {
	va_list ap;

	// Initialize global flub.
	va_start(ap, format);
	flub_tossv(&g_flub, format, ap);
	va_end(ap);

	// Return global flub.
	return (&g_flub);
}

char* g_serror(char* message) {
	static char buffer[256];

	// Concatenate messages.
	snprintf(buffer, sizeof(buffer), "%s: %s", message, strerror(errno));

	// Return concatenation.
	return buffer;
}
