/**
 *  Contains a mechanism for error-handling.
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

#ifndef flub_H
#define flub_H

#include <bsd/string.h>
#include <execinfo.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct flub {
	// Error message.
	char message[256];
	// Stack trace.
	char** backtrace;
	int backtrace_count;
};

/**
 * Appends the specified error message to the specified flub.
 */
struct flub* flub_append(struct flub* flub, char* format, ...)
	__attribute__((format(printf, 2, 3)));

/**
 * Frees the resources associated with the specified flub.
 */
void flub_grab(struct flub* flub);

/**
 * Pretty-print the specified flub to the specified string 'dest' with the
 * specified size 'size'.  Truncates like stlcpy.
 */
void flub_printfsl(struct flub*, char* dest, size_t size);

/**
 * Prints the specified flub's backtrace to the specified string 'dest'.
 */
void flub_snbacktrace(struct flub* flub, char* dest, size_t size);

/**
 * Initializes the specified flub with a backtrace containing the current
 * program stack.  The flub must later be reset via 'flub_grab'.
 */
void flub_toss(struct flub* flub, char* format, ...)
	__attribute__((format(printf, 2, 3)));
void flub_tossv(struct flub* flub, char* format, va_list ap);

#endif // flub_H
