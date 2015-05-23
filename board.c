/**
 *  See 'board.h'.
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
#include "board.h"

void board_init(struct board* board) {
	int i;
	int j;

	// Use a default set of cards. Hacky.
	memset(board, 0, sizeof(struct board));
	strncpy(board->plates[0][0].name, "Ambivalnce", PLATE_NAME_LENGTH);
	strncpy(board->plates[0][0].abbrev, "Amb", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][1].name, "Art Versus Nature", PLATE_NAME_LENGTH);
	strncpy(board->plates[0][1].abbrev, "AVN", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][2].name, "The Need Not to Judge", PLATE_NAME_LENGTH);
	strncpy(board->plates[0][2].abbrev, "TNJ", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][3].name, "Calculus", PLATE_NAME_LENGTH);
	strncpy(board->plates[0][3].abbrev, "Clc", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][4].name, "City as Artifact", PLATE_NAME_LENGTH);
	strncpy(board->plates[0][4].abbrev, "CaA", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][5].name, "Coding", PLATE_NAME_LENGTH);
	strncpy(board->plates[0][5].abbrev, "Cde", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][6].name, "Contemplation", PLATE_NAME_LENGTH);
	strncpy(board->plates[0][6].abbrev, "Ctp", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][7].name, "Continuity/Eternity", PLATE_NAME_LENGTH);
	strncpy(board->plates[0][7].abbrev, "C/E", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][0].name, "Creation", PLATE_NAME_LENGTH);
	strncpy(board->plates[1][0].abbrev, "Crt", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][1].name, "Ontogeny Recapitulates Phylogeny", PLATE_NAME_LENGTH);
	strncpy(board->plates[1][1].abbrev, "ORP", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][2].name, "Education", PLATE_NAME_LENGTH);
	strncpy(board->plates[1][2].abbrev, "Edu", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][3].name, "Helplessness", PLATE_NAME_LENGTH);
	strncpy(board->plates[1][3].abbrev, "Hlp", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][4].name, "Intuition", PLATE_NAME_LENGTH);
	strncpy(board->plates[1][4].abbrev, "Itu", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][5].name, "Monetary Value", PLATE_NAME_LENGTH);
	strncpy(board->plates[1][5].abbrev, "MnV", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][6].name, "Wavicle", PLATE_NAME_LENGTH);
	strncpy(board->plates[1][6].abbrev, "Wav", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][7].name, "Freedom", PLATE_NAME_LENGTH);
	strncpy(board->plates[1][7].abbrev, "Fre", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][0].name, "Emotional Manipulation", PLATE_NAME_LENGTH);
	strncpy(board->plates[2][0].abbrev, "EmM", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][1].name, "Gestalt", PLATE_NAME_LENGTH);
	strncpy(board->plates[2][1].abbrev, "Gst", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][2].name, "Harmony", PLATE_NAME_LENGTH);
	strncpy(board->plates[2][2].abbrev, "Hrm", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][3].name, "Hidden Potential", PLATE_NAME_LENGTH);
	strncpy(board->plates[2][3].abbrev, "HdP", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][4].name, "Joy", PLATE_NAME_LENGTH);
	strncpy(board->plates[2][4].abbrev, "Joy", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][5].name, "Magic", PLATE_NAME_LENGTH);
	strncpy(board->plates[2][5].abbrev, "Mgc", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][6].name, "Mechanical Advantage", PLATE_NAME_LENGTH);
	strncpy(board->plates[2][6].abbrev, "McA", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][7].name, "Metamorphosis", PLATE_NAME_LENGTH);
	strncpy(board->plates[2][7].abbrev, "Mtm", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][0].name, "Nature Tending Towards Perfection", PLATE_NAME_LENGTH);
	strncpy(board->plates[3][0].abbrev, "NTP", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][1].name, "Myth", PLATE_NAME_LENGTH);
	strncpy(board->plates[3][1].abbrev, "Mth", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][2].name, "Coexisting Species", PLATE_NAME_LENGTH);
	strncpy(board->plates[3][2].abbrev, "CxS", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][3].name, "Perspective", PLATE_NAME_LENGTH);
	strncpy(board->plates[3][3].abbrev, "Prp", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][4].name, "Reaching Out", PLATE_NAME_LENGTH);
	strncpy(board->plates[3][4].abbrev, "RcO", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][5].name, "Return", PLATE_NAME_LENGTH);
	strncpy(board->plates[3][5].abbrev, "Rtn", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][6].name, "Society as Active/Passive Hierarchy", PLATE_NAME_LENGTH);
	strncpy(board->plates[3][6].abbrev, "SoH", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][7].name, "Structural Strength", PLATE_NAME_LENGTH);
	strncpy(board->plates[3][7].abbrev, "StS", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[4][0].name, "Struggle", PLATE_NAME_LENGTH);
	strncpy(board->plates[4][0].abbrev, "Stg", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[4][1].name, "Synergy", PLATE_NAME_LENGTH);
	strncpy(board->plates[4][1].abbrev, "Syg", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[4][2].name, "Syntax", PLATE_NAME_LENGTH);
	strncpy(board->plates[4][2].abbrev, "Stx", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[4][3].name, "Unwanted Relationships", PLATE_NAME_LENGTH);
	strncpy(board->plates[4][3].abbrev, "UwR", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[4][4].name, "Structural Improvisation", PLATE_NAME_LENGTH);
	strncpy(board->plates[4][4].abbrev, "StI", PLATE_ABBREV_LENGTH);
	strncpy(board->plates[4][5].name, "Anthropomorphism", PLATE_NAME_LENGTH);
	strncpy(board->plates[4][5].abbrev, "Anp", PLATE_ABBREV_LENGTH);
}

int board_print(struct board* board, int fd) {
	int offset;
	char* buffer;
	const int height = 25;
	int i;
	int j;
	int k;
	const int width = 80;

	// Allocate buffer for writing board.
	buffer = malloc(width * height);
	if (!buffer) {
		log_error(&g_log, "Allocating board print buffer");
		return -1;
	}

	// Write column headers.
	offset = 0;
	for (i = 0; i < 16; i++) {
		buffer[offset++] = ' ';
	}
	for (i = 0; i < BOARD_PLATE_COLUMN_COUNT; i++) {
		strcpy(&buffer[offset], "   1  ");
		buffer[offset + 3] += i; // So hack.
		offset += 6;
	}
	buffer[offset++] = '\n';

	// Write each plate to the file descriptor.
	for (i = 0; i < BOARD_PLATE_ROW_COUNT; i++) {
		// Write row border.
		offset += board_print_border(board, &buffer[offset]);

		// Write left margin.
		for (k = 0; k < 14; k++) {
			buffer[offset++] = ' ';
		}
		buffer[offset++] = 'A' + i; // So hack.
		buffer[offset++] = ' ';

		// Write plate abbreviations.
		for (j = 0; j < BOARD_PLATE_COLUMN_COUNT; j++) {
			// Write plate abbreviations.
			strcpy(&buffer[offset], "| ");
			offset += 2;
			strncpy(&buffer[offset], board->plates[i][j].abbrev,
				3);
			for (k = 0; k < 3; k++) {
				// Replace empty characters with space.
				if (buffer[offset + k] == '\0') {
					buffer[offset + k] = ' ';
				}
			}
			offset += k;
			buffer[offset++] = ' ';
		}
		buffer[offset++] = '|';
		buffer[offset++] = '\n';
	}
	offset += board_print_border(board, &buffer[offset]);

	// Write board to file.
	if (write(fd, buffer, strlen(buffer)) == -1) {
		log_error(&g_log, g_serror("Printing board"));
	}

	// Free buffer.
	free(buffer);

	// Return success.
	return 0;
}

static size_t board_print_border(struct board* board, char* buffer) {
	int i;

	// Write row border.
	for (i = 0; i < 16; i++) {
		buffer[i] = ' ';
	}
	for ( ; i < 65; i++) {
		if ((i - 16) % 6) {
			buffer[i] = '-';
		} else {
			buffer[i] = '+';
		}
	}
	buffer[i++] = '\n';

	// Return bytes buffered.
	return i;
}

int board_read(struct board* board, int fd) {
	int i;
	int j;

	// Read each plate.
	for (i = 0; i < BOARD_PLATE_ROW_COUNT; i++) {
		for (j = 0; j < BOARD_PLATE_COLUMN_COUNT; j++) {
			if (plate_read(&board->plates[i][j], fd) == -1) {
				return -1;
			}
		}
	}

	// Return success.
	return 0;
}

int board_write(struct board* board, int fd) {
	int i;
	int j;

	// Write each plate.
	for (i = 0; i < BOARD_PLATE_ROW_COUNT; i++) {
		for (j = 0; j < BOARD_PLATE_ROW_COUNT; j++) {
			if (plate_write(&board->plates[i][j], fd) == -1) {
				return -1;
			}
		}
	}

	// Return success;
	return 0;
}
