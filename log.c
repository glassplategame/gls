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

#include "log.h"

int log_free(struct log* log) {
	// Close log file.
	return close(log->fd);
}

int log_init(struct log* log, char* path, enum log_level level) {
	int fd;

	// Open log file.
	if ((fd = open(path, O_APPEND | O_CREAT | O_WRONLY, 0600)) == -1) {
		snprintf(log->buffer, sizeof(log->buffer),
			"Unable to open log file at '%s'", path);
		perror(log->buffer);
		return -1;
	}
	log->fd = fd;

	// Set log level.
	log->level = level;
	log->header = 1;

	// Return success.
	return 0;
}

inline void log_write(struct log* log) {
	// Write the message.
	if (write(log->fd, log->buffer, strlen(log->buffer)) == -1) {
		perror("Unable to write the following message");
		fprintf(stderr, "%s\n", log->buffer);
	}
}

const char* LOG_STR_DEBUG = "DEBUG";
const char* LOG_STR_INFO = "INFO";
const char* LOG_STR_WARN = "WARN";
const char* LOG_STR_ERROR = "ERROR";

LOG_LEVEL_DECLARATION(debug, DEBUG)
LOG_LEVEL_DECLARATION(info, INFO)
LOG_LEVEL_DECLARATION(warn, WARN)
LOG_LEVEL_DECLARATION(error, ERROR)
