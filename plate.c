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

int plate_read(struct plate* plate, int fd) {
	char* buffer;
	char* offset;

	// Create a buffer in memory.
	buffer = (char*)malloc(PLATE_SIZE_PACKED);
	if (!buffer) {
		log_error(&g_log, "Allocating plate buffer");
		return -1;
	}

	// Write the buffer to the file.
	if (read(fd, buffer, PLATE_SIZE_PACKED) < PLATE_SIZE_PACKED) {
		log_error(&g_log, "Reading plate buffer");
		free(buffer);
		return -1;
	}

	// Read data from the buffer.
	offset = buffer;
	memcpy((void*)plate->name, (void*)offset, sizeof(plate->name));
	offset += sizeof(plate->name);
	memcpy((void*)plate->abbrev, (void*)offset, sizeof(plate->abbrev));
	offset += sizeof(plate->abbrev);
	memcpy((void*)plate->description, (void*)offset,
		sizeof(plate->description));

	// Safely terminate strings.
	plate->name[PLATE_NAME_LENGTH - 1] = '\0';
	plate->abbrev[PLATE_ABBREV_LENGTH - 1] = '\0';
	plate->description[PLATE_DESCRIPTION_LENGTH - 1] = '\0';

	// Free the buffer from memory.
	free(buffer);

	// Return success.
	return 0;
}

int plate_write(struct plate* plate, int fd) {
	char* buffer;
	char* offset;

	// Create a buffer in memory.
	buffer = (char*)malloc(PLATE_SIZE_PACKED);
	if (!buffer) {
		log_error(&g_log, "Allocating plate buffer");
		return -1;
	}

	// Place data into the buffer.
	offset = buffer;
	memcpy((void*)offset, (void*)plate->name, sizeof(plate->name));
	offset += sizeof(plate->name);
	memcpy((void*)offset, (void*)plate->abbrev, sizeof(plate->abbrev));
	offset += sizeof(plate->abbrev);
	memcpy((void*)offset, (void*)plate->description,
		sizeof(plate->description));

	// Write the buffer to the file.
	if (write(fd, buffer, PLATE_SIZE_PACKED) < PLATE_SIZE_PACKED) {
		log_error(&g_log, "Writing plate buffer");
		free(buffer);
		return -1;
	}

	// Free the buffer from memory.
	free(buffer);

	// Return success.
	return 0;
}
