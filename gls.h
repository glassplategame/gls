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

#include "include.h"

#include <arpa/inet.h>
#include <bsd/string.h>
#include <ctype.h>
#include <endian.h>
#include <stdint.h>
#include <sys/uio.h>
#include <unistd.h>

#include "global.h"

// Length of each thread's buffer.
#define GLS_BUFFER_SIZE 65536
// Length of a player's name.
#define GLS_NICK_LENGTH 32
// Lower-limit on pipe atomicity.
#define GLS_PIPE_BUF 4096

// Board dimensions.
#define GLS_BOARD_ROW_COUNT 8
#define GLS_BOARD_COLUMN_COUNT 8
// Board locations.
#define GLS_LOCATION_LENGTH 3

// Transparency colors.
#define GLS_COLOR_NULL		0  // a.k.a. "no color"
#define GLS_COLOR_RED 		1
#define GLS_COLOR_ORANGE 	2
#define GLS_COLOR_YELLOW	3
#define GLS_COLOR_GREEN		4
#define GLS_COLOR_BLUE		5
#define GLS_COLOR_PURPLE	6
#define GLS_COLOR_MAX		GLS_COLOR_PURPLE
#define GLS_COLOR_MIN		GLS_COLOR_RED
extern const char* gls_color_names[];

// Die definitions.
#define GLS_DIE_MAX		25


/**
 * Structure that represents an event in the Glass Plate Game.
 */
struct gls_header {
	// Event type.
	uint32_t event;
};

/**
 * Player nickname change.
 */
struct gls_nick_change {
	char old[GLS_NICK_LENGTH];
	char new[GLS_NICK_LENGTH];
};

/**
 * Request server to set nick to specified nickname.
 */
struct gls_nick_req {
	// Nickname requested.
	char nick[GLS_NICK_LENGTH];
};

/**
 * Server setting specific nickname.
 */
#define GLS_NICK_SET_REASON 64
struct gls_nick_set {
	// Nickname requested.
	char nick[GLS_NICK_LENGTH];
	// Reason for rejection.
	char reason[GLS_NICK_SET_REASON];
};

/**
 * Die has been placed on the board.
 */
struct gls_die_place {
	// Location die was placed at.
	char location[GLS_LOCATION_LENGTH];
	// Color for the die's transparency.
	uint32_t color;
	// Nick of player who placed the die.
	char nick[GLS_NICK_LENGTH];
	// The number of the die that was placed.
	uint32_t die;
};

/**
 * Server rejects die placement.
 */
#define GLS_DIE_PLACE_REJECT_REASON_LENGTH 64
struct gls_die_place_reject {
	// Location place attempted.
	char location[GLS_LOCATION_LENGTH];
	// Attempted color placement.
	uint32_t color;
	// Reason for rejection.
	char reason[GLS_DIE_PLACE_REJECT_REASON_LENGTH];
};

/**
 * Client tries to place a die.
 */
struct gls_die_place_try {
	// Location to place die.
	char location[GLS_LOCATION_LENGTH];
	// Color to place.
	uint32_t color;
};

/**
 * Plate placement.
 */
#define GLS_PLATE_ABBREV_LENGTH 4
#define GLS_PLATE_DESCRIPTION_LENGTH 256
#define GLS_PLATE_NAME_LENGTH 64
#define GLS_PLATE_FLAG_EMPTY 					0x00000001
struct gls_plate_place {
	char abbrev[GLS_PLATE_ABBREV_LENGTH];
	char description[GLS_PLATE_DESCRIPTION_LENGTH];
	char name[GLS_PLATE_NAME_LENGTH];
	char loc[GLS_LOCATION_LENGTH];
	uint32_t flags;
};

/**
 * Player joins game.
 */
struct gls_player_join {
	// Player joining.
	char nick[GLS_NICK_LENGTH];
};

/**
 * Player leaves game.
 */
struct gls_player_part {
	// Player parting.
	char nick[GLS_NICK_LENGTH];
};

/**
 * Protocol Version data.
 */
#define GLS_PROTOVER_MAGIC_LENGTH 4
#define GLS_PROTOVER_VERSION_LENGTH 16
#define GLS_PROTOVER_SOFTWARE_LENGTH 32
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

/**
 * Player message packets.
 * "say1" is from the player to the server.
 * "say2" is from the server to each of the players.
 */
#define GLS_SAY_MESSAGE_LENGTH 256
struct gls_say1 {
	char message[GLS_SAY_MESSAGE_LENGTH];
};
struct gls_say2 {
	char nick[GLS_NICK_LENGTH];
	uint64_t tval;
	char message[GLS_SAY_MESSAGE_LENGTH];
};

/**
 * Server shutdown packet.
 */
#define GLS_SHUTDOWN_REASON_LENGTH 64
struct gls_shutdown {
	char reason[GLS_SHUTDOWN_REASON_LENGTH];
};

/**
 * Player synchronization finished packet.
 */
#define GLS_MOTD_LENGTH 256
struct gls_sync_end {
	char motd[GLS_MOTD_LENGTH];
};

// Packet headers.
#define GLS_EVENT_PROTOVER		0x00000001
#define GLS_EVENT_PROTOVERACK		0x00000002
#define GLS_EVENT_NICK_REQ		0x00000003
#define GLS_EVENT_NICK_SET		0x00000004
#define GLS_EVENT_NICK_CHANGE		0x00000005
#define GLS_EVENT_PLAYER_JOIN		0x00000006
#define GLS_EVENT_PLAYER_PART		0x00000007
#define GLS_EVENT_SHUTDOWN		0x00000008
#define GLS_EVENT_SAY1			0x00000009
#define GLS_EVENT_SAY2			0x0000000A
#define GLS_EVENT_SYNC_END		0x0000000B
#define GLS_EVENT_PLATE_PLACE		0x0000000C
#define GLS_EVENT_DIE_PLACE_TRY		0x0000000D
#define GLS_EVENT_DIE_PLACE_REJECT	0x0000000E
#define GLS_EVENT_DIE_PLACE		0x0000000F

// Union of all packets.
struct gls_packet {
	struct gls_header header;
	union {
		struct gls_nick_req nick_req;
		struct gls_nick_set nick_set;
		struct gls_protover protover;
		struct gls_protoverack protoverack;
		struct gls_nick_change nick_change;
		struct gls_player_join player_join;
		struct gls_player_part player_part;
		struct gls_shutdown shutdown;
		struct gls_say1 say1;
		struct gls_say2 say2;
		struct gls_sync_end sync_end;
		struct gls_plate_place plate_place;
		struct gls_die_place_try die_place_try;
		struct gls_die_place_reject die_place_reject;
		struct gls_die_place die_place;
	} data;
};

/**
 * Reads the specified Die Place packet from the specified file descriptor.
 */
struct flub* gls_die_place_read(struct gls_die_place* die, int fd,
	int validate);

/**
 * Writes the specified Die Place packet to the specified file descriptor.
 */
struct flub* gls_die_place_write(struct gls_die_place* die, int fd);

/**
 * Reads the specified Die Place Reject packet from the specified file
 * descriptor.
 */
struct flub* gls_die_place_reject_read(struct gls_die_place_reject* die, int fd,
	int validate);

/**
 * Writes the specified Die Place Reject packet to the specified file
 * descriptor.
 */
struct flub* gls_die_place_reject_write(struct gls_die_place_reject* die,
	int fd);

/**
 * Reads the specified Die Place Try packet from the specified file descriptor.
 */
struct flub* gls_die_place_try_read(struct gls_die_place_try* die, int fd,
	int validate);

/**
 * Writes the specified Die Place Try packet to the specified file descriptor.
 */
struct flub* gls_die_place_try_write(struct gls_die_place_try* die, int fd);

/**
 * Marshalls the speccified gls header data into the specified buffer.
 */
void gls_header_marshal(char* buffer, uint32_t event);

/**
 * Reads gls protocol data from the specified file descriptor.
 */
struct flub* gls_header_read(struct gls_header* header, int fd);

/**
 * Initialize the buffer for the gls protocol.
 */
struct flub* gls_init();

/**
 * Destructor function for the pthread-specific gls buffer.
 */
void gls_init_destructor(void* buffer);

/**
 * Return a flub if the specified location isn't valid.
 */
struct flub* gls_location_validate(char* location);

/**
 * Validate a MotD.
 */
struct flub* gls_motd_validate(char* message);

/**
 * Read the nick change notification from the specified file descriptor.
 */
struct flub* gls_nick_change_read(struct gls_nick_change* change, int fd,
	int validate);

/**
 * Write the nick change notification to the specified file descriptor.
 */
struct flub* gls_nick_change_write(struct gls_nick_change* change, int fd);

/**
 * Read the nick request from the specified file descriptor.
 */
struct flub* gls_nick_req_read(struct gls_nick_req* req, int fd, int validate);

/**
 * Write the nick request to the specified file descriptor.
 */
struct flub* gls_nick_req_write(struct gls_nick_req* req, int fd);

/**
 * Read the nick set from the specified file descriptor.
 */
struct flub* gls_nick_set_read(struct gls_nick_set* set, int fd,
	int validate);

/**
 * Write the nick set to the specified file descriptor.
 */
struct flub* gls_nick_set_write(struct gls_nick_set* set, int fd);

/**
 * Return a flub if the specified nickname is not valid, NULL otherwise.  A
 * nonzero value in the 'empty' parameter specifies that the nick may be empty.
 */
struct flub* gls_nick_validate(char* nick, int empty);

/**
 * Reads an arbitrary packet from the specified file descriptor.
 */
struct flub* gls_packet_read(struct gls_packet* packet, int fd, int validate);

/**
 * Writes the specified packet to the specified file descriptor.
 */
struct flub* gls_packet_write(struct gls_packet* packet, int fd);

/**
 * Read the specified plate placement from the specified file descriptor.
 */
struct flub* gls_plate_place_read(struct gls_plate_place* plate, int fd,
	int validate);

/**
 * Write the specified plate placement to the specified file descriptor.
 */
struct flub* gls_plate_place_write(struct gls_plate_place* plate, int fd);

/**
 * Read the specified Player Join packet from the specified file descriptor.
 */
struct flub* gls_player_join_read(struct gls_player_join* join, int fd,
	int validate);

/**
 * Write the specified Player Join packet to the specified file descriptor.
 */
struct flub* gls_player_join_write(struct gls_player_join* join, int fd);

/**
 * Read the specified Player Part packet from the specified file descriptor.
 */
struct flub* gls_player_part_read(struct gls_player_part* part, int fd,
	int validate);

/**
 * Write the specified Player Part packet to the specified file descriptor.
 */
struct flub* gls_player_part_write(struct gls_player_part* part, int fd);

/**
 * Read the protocol version information from the specified file descriptor.
 */
struct flub* gls_protover_read(struct gls_protover* pver, int fd,
	int validate);

/**
 * Write the protocol version information to the specified file descriptor.
 */
struct flub* gls_protover_write(struct gls_protover* pver, int fd);

/**
 * Read the protocol version information ackowledgement from the specified
 * file descriptor.
 */
struct flub* gls_protoverack_read(struct gls_protoverack* pack, int fd,
	int validate);

/**
 * Write the protocol version information acknowledgement to the specified
 * file descriptor.
 */
struct flub* gls_protoverack_write(struct gls_protoverack* pack, int fd);

/**
 * Re-attempts partial reads.
 *
 * Returns the number of bytes read, or -1 on error.
 */
ssize_t gls_readn(int fd, void* buffer, size_t count);

/**
 * Re-attempts partial reads.
 *
 * Returns the number of bytes read, or -1 on error.
 */
ssize_t gls_readvn(int fd, struct iovec* iov, int iovcnt);

/**
 * Validate say message.
 */
struct flub* gls_say_message_validate(char* message);

/**
 * Read the specified Say1 packet from the specified file descriptor.
 */
struct flub* gls_say1_read(struct gls_say1* say, int fd, int validate);

/**
 * Write the specified Say1 packet to the specified file descriptor.
 */
struct flub* gls_say1_write(struct gls_say1* say, int fd);

/**
 * Read the specified Say2 packet from the specified file descriptor.
 */
struct flub* gls_say2_read(struct gls_say2* say, int fd, int validate);

/**
 * Write the specified Say2 packet to the specified file descriptor.
 */
struct flub* gls_say2_write(struct gls_say2* say, int fd);

/**
 * Read the specified Shutdown packet from the specified file descriptor.
 */
struct flub* gls_shutdown_read(struct gls_shutdown* shutdown, int fd,
	int validate);

/**
 * Write the specified Shutdown packet to the specified file descriptor.
 */
struct flub* gls_shutdown_write(struct gls_shutdown* shutdown, int fd);

/**
 * Read the specified sync end event from the specified file descriptor.
 */
struct flub* gls_sync_end_read(struct gls_sync_end* sync_end, int fd,
	int validate);

/**
 * Write the specified sync end event to the specified file descriptor.
 */
struct flub* gls_sync_end_write(struct gls_sync_end* sync_end, int fd);

/**
 * Re-attempts partial writes.
 *
 * Returns the number of bytes written, or -1 on error.
 */
ssize_t gls_writen(int fd, void* buffer, size_t count);

/**
 * Re-attempts partial writes.
 *
 * Returns the number of bytes written, or -1 on error.
 */
ssize_t gls_writevn(int fd, struct iovec* iov, int iovcnt);

#endif // gls_H
