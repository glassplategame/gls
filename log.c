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

void log_debug(struct log* log, char* message) {
	// Check log level.
	if (log->level > LOG_DEBUG) {
		return;
	}

	// Write the message.
	snprintf(log->buffer, sizeof(log->buffer), "D: %s\n", message);
	log_write(log);
}

void log_error(struct log* log, char* message) {
	// Log message.
	snprintf(log->buffer, sizeof(log->buffer), "E: %s\n", message);
	log_write(log);
}

int log_free(struct log* log) {
	// Close log file.
	return close(log->fd);
}

void log_info(struct log* log, char* message) {
	// Check log level.
	if (log->level > LOG_INFO) {
		return;
	}

	// Write the message.
	snprintf(log->buffer, sizeof(log->buffer), "I: %s\n", message);
	log_write(log);
}

int log_init(struct log* log, char* path, enum log_level level) {
	int fd;

	// Open log file.
	if ((fd = open(path, O_CREAT | O_WRONLY, 0600)) == -1) {
		snprintf(log->buffer, sizeof(log->buffer),
			"Unable to open log file at '%s'", path);
		perror(log->buffer);
		return -1;
	}
	log->fd = fd;

	// Set log level.
	log->level = level;

	// Return success.
	return 0;
}

void log_warn(struct log* log, char* message) {
	// Check log level.
	if (log->level > LOG_WARN) {
		return;
	}

	// Write the message.
	snprintf(log->buffer, sizeof(log->buffer), "W: %s\n", message);
	log_write(log);
}

inline void log_write(struct log* log) {
	// Write the message.
	if (write(log->fd, log->buffer, strlen(log->buffer)) == -1) {
		perror("Unable to write the following message");
		fprintf(stderr, "%s\n", log->buffer);
	}
}
