/**
 *  Definition of events that can be sent in-between the server and client.
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
#ifndef gls_H
#define gls_H

#include "player.h"

// Maximum size for a single GLS command.
#define GLS_SIZE_MAX 4096

/**
 * Request server to set nick to specified nickname.
 */
struct gls_nick_req {
	// Client sequence number.
	uint32_t seqnumc;
	// Nickname requested.
	char nick[PLAYER_NAME_LENGTH];
};

/**
 * Server response to nickname request.
 */
struct gls_nick_reply {
	// Client sequence number of request.
	uint32_t seqnumc;
	// Nickname requested.
	char nick[PLAYER_NAME_LENGTH];
	// Non-zero if nickname accepted.
	int accepted;
};

/**
 * Structure that represents an event in the Glass Plate Game.
 */
struct gls {
	// Event type.
	uint32_t event;
	// Event data.
	union {
		struct gls_nick_req nick_req;
		struct gls_nick_reply nick_reply;
		char data[GLS_SIZE_MAX - 4];
	} data;
};

// Request nickname.
#define GLS_EVENT_NICK_REQ 0x00000001
#define GLS_EVENT_NICK_REPLY 0x00000002

#endif // gls_H
