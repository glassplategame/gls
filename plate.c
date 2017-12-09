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
	char* buffer;
	const size_t buffer_size = 2048;
	int offset;
	char* string;

	// Allocate buffer.
	buffer = (char*)malloc(buffer_size);
	if (!buffer) {
		return g_flub_toss("Unable to allocate plate buffer");
	}
	memset(buffer, 0, buffer_size);

	// Place contents in buffer.
	offset = 0;
	string = "Name: ";
	strcpy(buffer, string);
	offset += strlen(string);
	strcpy(&buffer[offset], plate->name);
	offset += strlen(plate->name);
	buffer[offset++] = '\n';
	string = "Abbreviation: ";
	strcpy(&buffer[offset], string);
	offset += strlen(string);
	strcpy(&buffer[offset], plate->abbrev);
	offset += strlen(plate->abbrev);
	buffer[offset++] = '\n';
	if (strlen(plate->description)) {
		string = "Description: ";
		strcpy(&buffer[offset], string);
		offset += strlen(string);
		strcpy(&buffer[offset], plate->description);
		offset += strlen(plate->description);
		buffer[offset++] = '\n';
	}

	// Write buffer to file.
	if (write(fd, buffer, strlen(buffer)) == -1) {
		g_log_error("Printing plate buffer: '%s'", g_serr(errno));
	}

	// Free buffer.
	free(buffer);

	// Return success.
	return NULL;
}
