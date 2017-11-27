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

#include "include.h"

#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "global.h"
#include "gls.h"

struct player {
	// Thread-specific flub.
	struct flub flub;
	// Private buffer for player identification.
	char name[GLS_NICK_LENGTH];
	// Player's nickname.
	char nick[GLS_NICK_LENGTH];
	// Pipes going to and from the server.
	int pipe_server_from[2];
	int pipe_server_to[2];
	// Player connection.
	int sockfd;
	// Thread identifier.
	pthread_t thread;
	// Authentication status.
	unsigned authenticated:1;
	// Connection open.
	unsigned connected:1;
	// Protover verified.
	unsigned protoverokay:1;
	// Killed by server.
	unsigned killed:1;
};

/**
 * Free possibly disconnected player.
 */
void player_free(struct player* player, struct flub* status);

/**
 * Initialize newly-connected player.
 */
struct flub* player_init(struct player* player, int fd);

/**
 * Force player to prepare for player_free.
 */
struct flub* player_kill(struct player* player);

/**
 * Returns a statically-allocated human-readable name for the player regardless
 * of player state (unlike nickname, where the player must be authenticated).
 * Not reentrant.
 */
char* player_name(struct player* player);

/**
 * Thread to buffer data in-between the raw socket and the server.
 */
void* player_thread(void* player);

#endif // player_H
