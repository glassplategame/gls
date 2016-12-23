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

#include <arpa/inet.h>
#include <bsd/string.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/uio.h>
#include <unistd.h>

#include "player.h"

/**
 * Structure that represents an event in the Glass Plate Game.
 */
struct gls_header {
	// Event type.
	uint32_t event;
};

/**
 * Request server to set nick to specified nickname.
 */
struct gls_nick_req {
	// Nickname requested.
	char nick[PLAYER_NAME_LENGTH];
};

/**
 * Server response to nickname request.
 */
struct gls_nick_reply {
	// Nickname requested.
	char nick[PLAYER_NAME_LENGTH];
	// Non-zero if nickname accepted.
	uint16_t accepted;
};

/**
 * Protocol Version data.
 */
#define GLS_PROTOVER_MAGIC_LENGTH 4
#define GLS_PROTOVER_VERSION_LENGTH 16
#define GLS_PROTOVER_SOFTWARE_LENGTH 128
struct gls_protover {
	char magic[GLS_PROTOVER_MAGIC_LENGTH];
	char version[GLS_PROTOVER_VERSION_LENGTH];
	char software[GLS_PROTOVER_SOFTWARE_LENGTH];
};

#define GLS_PROTOVER_REASON_LENGTH 64
struct gls_protoverack {
	uint16_t ack;
	char reason[GLS_PROTOVER_REASON_LENGTH];
	struct gls_protover pver;
};

// Events.
// Nickname request.
#define GLS_EVENT_NICK_REQ 0x00000001
#define GLS_EVENT_NICK_REPLY 0x00000002

/**
 * Marshalls the speccified gls header data into the specified buffer.
 */
void gls_header_marshal(char* buffer, uint32_t event);

/**
 * Reads gls protocol data from the specified file descriptor.
 *
 * Returns 0 on success, -1 on error.
 */
struct flub* gls_header_read(struct gls_header* header, int fd);

/**
 * Read the nick reply from the specified file descriptor.
 *
 * Returns 0 on success, -1 on error.
 */
struct flub* gls_nick_reply_read(struct gls_nick_reply* reply, int fd);

/**
 * Write the nick reply to the specified file descriptor.
 *
 * Returns 0 on success, -1 on error.
 */
struct flub* gls_nick_reply_write(struct gls_nick_reply* reply, int fd);

/**
 * Read the nick request from the specified file descriptor.
 *
 * Returns 0 on success, -1 on error.
 */
struct flub* gls_nick_req_read(struct gls_nick_req* req, int fd);

/**
 * Write the nick request to the specified file descriptor.
 *
 * Returns 0 on success, -1 on error.
 */
struct flub* gls_nick_req_write(struct gls_nick_req* req, int fd);

/**
 * Read the protocol version information from the specified file descriptor.
 *
 * Returns 0 on success, -1 on error.
 */
struct flub* gls_protover_read(struct gls_protover* pver, int fd);

/**
 * Write the protocol version information to the specified file descriptor.
 *
 * Returns 0 on success, -1 on error.
 */
struct flub* gls_protover_write(struct gls_protover* pver, int fd);

/**
 * Read the protocol version information ackowledgement from the specified
 * file descriptor.
 *
 * Returns 0 on success, -1 on error.
 */
struct flub* gls_protoverack_read(struct gls_protoverack* pack, int fd);

/**
 * Write the protocol version information acknowledgement to the specified
 * file descriptor.
 *
 * Returns 0 on success, -1 on error.
 */
struct flub* gls_protoverack_write(struct gls_protoverack* pack, int fd);

#endif // gls_H
