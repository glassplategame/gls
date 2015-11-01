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
#include "player.h"

void player_free(struct player* player) {
	// Close socket.
	if (close(player->sockfd)) {
		log_warn(&g_log, g_serror("Closing player socket"));
	}

	// Clear data.
	memset((void*)player, 0, sizeof(struct player));
}
void player_init(struct player* player, int fd) {
	// Clear any previous data.
	memset((void*)player, 0, sizeof(struct player));

	// Set socket.
	player->sockfd = fd;

	// Set connected status.
	player->connected = 1;
}
