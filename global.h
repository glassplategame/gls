/**
 *  Contains global variables.
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

#ifndef global_H
#define global_H

#include <errno.h>

#include "log.h"

// Global logger.
struct log g_log;

#define G_LOG_DECLARATION(level)					\
void g_log_##level(char* format, ...);

#define G_LOG_DEFINITION(level)						\
void g_log_##level(char* format, ...) {					\
	va_list ap;							\
									\
	va_start(ap, format);						\
	log_v##level(&g_log, format, ap);				\
	va_end(ap);							\
}

G_LOG_DECLARATION(debug)
G_LOG_DECLARATION(info)
G_LOG_DECLARATION(warn)
G_LOG_DECLARATION(error)

/**
 * Concates the user-defined message and the current system error message (as
 * specified via 'errno') and returns a pointer to the resulting concatenation.
 */
char* g_serror(char* message);

#endif // global_H
