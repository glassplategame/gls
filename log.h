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
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

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

/**
 * Writes data to a file.
 */
struct log {
	// Internal buffer for storing messages before writing.
	char buffer[256];
	// Write messages at or above this log level.
	enum log_level level;
	// File to write to.
	int fd;
};

/**
 * Log a debug message.
 */
void log_debug(struct log* log, char* message);

/**
 * Log an error message.
 */
void log_error(struct log* log, char* message);

/**
 * Frees any resources in use by the specified logger.
 */
int log_free(struct log* log);

/**
 * Initialize the logger at the specified file path.
 */
int log_init(struct log* log, char* path, enum log_level level);

/**
 * Log a warning message.
 */
void log_warn(struct log* log, char* message);

/**
 * Internal function that actually writes and catches log errors using the
 * internal buffer.
 */
void log_write(struct log* log);

#endif // log_H
