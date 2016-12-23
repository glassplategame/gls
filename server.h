/**
 *  Implementation of Dunbar's "Glass Plate Game" game, which is based off of
 *  Herman Hesse's novel, "The Glass Bead Game".
 *
 *  This is the server's portion of the code.
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
#ifndef server_H
#define server_H

#include "include.h"

#include <bsd/string.h>
#include <errno.h>
#include <poll.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "board.h"
#include "gls.h"
#include "log.h"
#include "player.h"

#define SERVER_PLAYER_MAX 64

/**
 * Game server abstraction.
 */
struct server {
	// Game board.
	struct board board;
	// Maximum number of players.
	struct player players[SERVER_PLAYER_MAX];
	// Server currently running.
	unsigned running:1;
	// Incoming connections socket.
	int sockfd;
	// Still not sure what exactly this thing is.
	struct sockaddr_in sockaddr_in;
};

/**
 * Prepare a server for running.
 */
struct flub* server_init(struct server* server);

/**
 * Process incoming data from the specified player.
 */
struct flub* server_player_data(struct server* server, struct player* player);

/**
 * Server run loop.
 */
struct flub* server_run(struct server* server);

#endif // server_H
