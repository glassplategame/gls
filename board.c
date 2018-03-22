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

static size_t board_print_border(struct board* board, char* buffer);

struct flub* board_die_place(struct board* board, char* nick, char* location,
	uint32_t* color, uint32_t* die) {
	struct flub* flub;

	// Check if the die can be placed.
	if ((flub = board_die_place_check(board, location, color, die))) {
		// Die cannot be placed.
		return flub;
	}

	// Place die.
	strlcpy(board->dice[(*die)].nick, nick, GLS_NICK_LENGTH);
	strlcpy(board->dice[(*die)].location, location, GLS_LOCATION_LENGTH);
	board->dice[(*die)].color = (*color);
	return NULL;
}

struct flub* board_die_place_check(struct board* board, char* location,
	uint32_t* color, uint32_t* die) {
	int i;

	// Check for color.
	if ((*color) == GLS_COLOR_NULL) {
		int newc;

		// Check for available color.
		for (newc = GLS_COLOR_MIN; newc <= GLS_COLOR_MAX; newc++) {
			for (i = 0; i < GLS_DIE_MAX; i++) {
				if (board->dice[i].color == newc) {
					break; // Color in use.
				}
			}
			if (i == GLS_DIE_MAX) {
				break; // Color found.
			}
		}
		if (newc > GLS_COLOR_MAX) {
			return g_flub_toss("No colors left");
		}
		(*color) = newc;
	} else {
		// Check if color in use.
		for (i = 0; i < GLS_DIE_MAX; i++) {
			if (board->dice[i].color == (*color)) {
				break; // Color in use.
			}
		}
		if (i != GLS_DIE_MAX) {
			return g_flub_toss("Color in use");
		}
	}

	// Check for empty plate.
	if (board->plates[location[0] - 'A'][location[1] - '1'].empty) {
		return g_flub_toss("No plate at location '%s'", location);
	}

	// Check for a die.
	for (i = 0; i < GLS_DIE_MAX; i++) {
		if (!strlen(board->dice[i].location)) {
			break;
		}
	}
	if (i == GLS_DIE_MAX) {
		return g_flub_toss("No dice left");
	}
	(*die) = i;
	return NULL;
}

void board_init(struct board* board) {
	// Use a default set of cards. Hacky.
	memset(board, 0, sizeof(struct board));
	strncpy(board->plates[0][0].name, "Ambivalence", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[0][0].abbrev, "Amb", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][1].name, "Art Versus Nature", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[0][1].abbrev, "AVN", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][2].name, "The Need Not to Judge", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[0][2].abbrev, "TNJ", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][3].name, "Calculus", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[0][3].abbrev, "Clc", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][4].name, "City as Artifact", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[0][4].abbrev, "CaA", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][5].name, "Coding", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[0][5].abbrev, "Cde", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][6].name, "Contemplation", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[0][6].abbrev, "Ctp", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[0][7].name, "Continuity/Eternity", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[0][7].abbrev, "C/E", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][0].name, "Creation", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[1][0].abbrev, "Crt", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][1].name, "Ontogeny Recapitulates Phylogeny", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[1][1].abbrev, "ORP", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][2].name, "Education", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[1][2].abbrev, "Edu", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][3].name, "Helplessness", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[1][3].abbrev, "Hlp", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][4].name, "Intuition", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[1][4].abbrev, "Itu", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][5].name, "Monetary Value", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[1][5].abbrev, "MnV", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][6].name, "Wavicle", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[1][6].abbrev, "Wav", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[1][7].name, "Freedom", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[1][7].abbrev, "Fre", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][0].name, "Emotional Manipulation", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[2][0].abbrev, "EmM", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][1].name, "Gestalt", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[2][1].abbrev, "Gst", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][2].name, "Harmony", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[2][2].abbrev, "Hrm", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][3].name, "Hidden Potential", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[2][3].abbrev, "HdP", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][4].name, "Joy", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[2][4].abbrev, "Joy", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][5].name, "Magic", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[2][5].abbrev, "Mgc", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][6].name, "Mechanical Advantage", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[2][6].abbrev, "McA", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[2][7].name, "Metamorphosis", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[2][7].abbrev, "Mtm", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][0].name, "Nature Tending Towards Perfection", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[3][0].abbrev, "NTP", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][1].name, "Myth", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[3][1].abbrev, "Mth", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][2].name, "Coexisting Species", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[3][2].abbrev, "CxS", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][3].name, "Perspective", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[3][3].abbrev, "Prp", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][4].name, "Reaching Out", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[3][4].abbrev, "RcO", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][5].name, "Return", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[3][5].abbrev, "Rtn", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][6].name, "Society as Active/Passive Hierarchy", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[3][6].abbrev, "SoH", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[3][7].name, "Structural Strength", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[3][7].abbrev, "StS", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[4][0].name, "Struggle", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[4][0].abbrev, "Stg", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[4][1].name, "Synergy", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[4][1].abbrev, "Syg", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[4][2].name, "Syntax", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[4][2].abbrev, "Stx", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[4][3].name, "Unwanted Relationships", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[4][3].abbrev, "UwR", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[4][4].name, "Structural Improvisation", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[4][4].abbrev, "StI", GLS_PLATE_ABBREV_LENGTH);
	strncpy(board->plates[4][5].name, "Anthropomorphism", GLS_PLATE_NAME_LENGTH);
	strncpy(board->plates[4][5].abbrev, "Anp", GLS_PLATE_ABBREV_LENGTH);
	board->plates[4][6].empty = 1;
	board->plates[4][7].empty = 1;
	board->plates[5][0].empty = 1;
	board->plates[5][1].empty = 1;
	board->plates[5][2].empty = 1;
	board->plates[5][3].empty = 1;
	board->plates[5][4].empty = 1;
	board->plates[5][5].empty = 1;
	board->plates[5][6].empty = 1;
	board->plates[5][7].empty = 1;
	board->plates[6][0].empty = 1;
	board->plates[6][1].empty = 1;
	board->plates[6][2].empty = 1;
	board->plates[6][3].empty = 1;
	board->plates[6][4].empty = 1;
	board->plates[6][5].empty = 1;
	board->plates[6][6].empty = 1;
	board->plates[6][7].empty = 1;
	board->plates[7][0].empty = 1;
	board->plates[7][1].empty = 1;
	board->plates[7][2].empty = 1;
	board->plates[7][3].empty = 1;
	board->plates[7][4].empty = 1;
	board->plates[7][5].empty = 1;
	board->plates[7][6].empty = 1;
	board->plates[7][7].empty = 1;
}

struct flub* board_print(struct board* board, int fd) {
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
		return g_flub_toss("Unable to allocate board print buffer");
	}

	// Write column headers.
	offset = 0;
	for (i = 0; i < 16; i++) {
		buffer[offset++] = ' ';
	}
	for (i = 0; i < GLS_BOARD_ROW_COUNT; i++) {
		strcpy(&buffer[offset], "   1  ");
		buffer[offset + 3] += i; // So hack.
		offset += 6;
	}
	buffer[offset++] = '\n';

	// Write each plate to the file descriptor.
	for (i = 0; i < GLS_BOARD_ROW_COUNT; i++) {
		// Write row border.
		offset += board_print_border(board, &buffer[offset]);

		// Write left margin.
		for (k = 0; k < 14; k++) {
			buffer[offset++] = ' ';
		}
		buffer[offset++] = 'A' + i; // So hack.
		buffer[offset++] = ' ';

		// Write plate abbreviations.
		for (j = 0; j < GLS_BOARD_COLUMN_COUNT; j++) {
			// Write plate abbreviations.
			strcpy(&buffer[offset], "| ");
			offset += 2;
			if (board->plates[i][j].empty) {
				strcpy(&buffer[offset], " - ");
			} else {
				strncpy(&buffer[offset],
					board->plates[i][j].abbrev,
					3);
			}
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
		g_flub_toss("Unable to write board: '%s'", g_serr(errno));
	}

	// Free buffer.
	free(buffer);

	// Return success.
	return NULL;
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
