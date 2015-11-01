/**
 *  The server's conception of a player.
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
#ifndef player_H
#define player_H

#include "global.h"

#define PLAYER_NAME_LENGTH 32

struct player {
	// Player's nickname.
	char name[PLAYER_NAME_LENGTH];
	// Player connection.
	int sockfd;
	// Authentication status.
	unsigned authenticated:1;
	// Connection open.
	unsigned connected:1;
};

/**
 * Free possibly disconnected player.
 */
void player_free(struct player* player);

/**
 * Initialize newly-connected player.
 */
void player_init(struct player* player, int fd);

#endif // player_H
