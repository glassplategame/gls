/**
 *  Stores messages useful for analysis and debugging purposes.
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

#ifndef log_H
#define log_H

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * Describes various logging levels.
 *   debug = verbose information
 *   info  = analysis information (for sysadmins)
 *   warn  = something probably undesireable happened
 *   error = something definitely undesireable happened
 */
enum log_level {
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR
};
extern const char* LOG_STR_DEBUG;
extern const char* LOG_STR_INFO;
extern const char* LOG_STR_WARN;
extern const char* LOG_STR_ERROR;

/**
 * Writes data to a file.
 */
struct log {
	// Internal buffer for storing messages before writing.
	char buffer[256];
	// File to write to.
	int fd;
	// Whether to print level header for each message.
	int header;
	// Write messages at or above this log level.
	enum log_level level;
	// Another internal buffer.
	char tmp[256];
};

/**
 * Frees any resources in use by the specified logger.
 */
int log_free(struct log* log);

/**
 * Initialize the logger at the specified file path.
 */
int log_init(struct log* log, char* path, enum log_level level);

/**
 * Internal function that actually writes and catches log errors using the
 * internal buffer.
 */
void log_write(struct log* log);

#define LOG_LEVEL_DEFINITION(name, NAME)				\
void log_##name(struct log* log, char* format, ...)			\
	__attribute__((format(printf, 2, 3)));				\
void log_v##name(struct log* log, char* format, va_list ap);

#define LOG_LEVEL_DECLARATION(name, NAME)				\
void log_##name(struct log* log, char* format, ...) {			\
	va_list ap;							\
									\
	va_start(ap, format); 						\
	log_v##name(log, format, ap);					\
	va_end(ap);							\
}									\
void log_v##name(struct log* log, char* format, va_list ap) {		\
	if (log->level > LOG_##NAME) {					\
		return;							\
	}								\
									\
	if (log->header) {						\
		snprintf(log->tmp, sizeof(log->tmp), "[%s]: %s\n",	\
			LOG_STR_##NAME,	format);			\
	} else {							\
		snprintf(log->tmp, sizeof(log->tmp), "%s\n", format);	\
	}								\
	vsnprintf(log->buffer, sizeof(log->buffer), log->tmp, ap);	\
	log_write(log);							\
}

LOG_LEVEL_DEFINITION(debug, DEBUG)
LOG_LEVEL_DEFINITION(info, INFO)
LOG_LEVEL_DEFINITION(warn, WARN)
LOG_LEVEL_DEFINITION(error, ERROR)

#endif // log_H
