/**
 *  Implementation of Dunbar's "Glass Plate Game" game, which is based off of
 *  Herman Hesse's novel, "The Glass Bead Game".
 *
 *  This is the client's portion of the code.
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

#include "client.h"

struct flub* client_nickname_write(struct client* client, char* nickname) {
	struct flub* flub;
	struct gls_header header;
	struct gls_nick_reply reply;
	struct gls_nick_req req;

	// Write nickname to server.
	memset(&req, 0, sizeof(struct gls_nick_req));
	strlcpy(req.nick, nickname, GLS_NAME_LENGTH);
	flub = gls_nick_req_write(&req, client->sockfd);
	if (flub) {
		return flub_append(flub, "unable to write nickname");
	} else {
		g_log_info("Nickname '%s' requested.", nickname);
	}

	// Read nickname from server.
	flub = gls_header_read(&header, client->sockfd);
	if (flub) {
		// Unable to read header.
		return flub_append(flub, "unable to read gls header");
	}
	if (header.event != GLS_EVENT_NICK_REPLY) {
		// Unexpected event.
		return g_flub_toss("Unexpected event: '%x'", header.event);
	}
	flub = gls_nick_reply_read(&reply, client->sockfd, 1);
	if (flub) {
		return flub_append(flub, "unable to get nick reply");
	}
	if (!reply.accepted) {
		// Nickname rejected.
		return g_flub_toss("Nickname rejected");
	} else {
		// Nickname accepted.
		return NULL;
	}
}

int main(int argc, char* argv[]) {
	struct client client;
	int done;
	struct flub* flub;
	struct gls_packet packet;
	struct sockaddr_in sockaddr_in;
	int ret;

	// Set up the globals.
	// FIXME: Don't bypass the proper init method.
	g_log.fd = STDOUT_FILENO;
	g_log.level = LOG_DEBUG;
	g_log.header = 0;
	if ((ret = g_serr_init())) {
		g_log_error("Unable to create system error buffer");
		exit(EXIT_FAILURE);
	}
	ret = pthread_key_create(&g_flub_key, g_flub_destructor);
	if (ret) {
		g_log_error("Error creating flub key: '%s'", g_serr(ret));
		exit(EXIT_FAILURE);
	}
	ret = g_flub_init();
	if (ret) {
		g_log_error("Unable to initialize flub: '%s'", g_serr(ret));
		exit(EXIT_FAILURE);
	}
	flub = gls_init();
	if (flub) {
		g_log_error("Unable to initialize gls buffer: '%s'",
			flub->message);
		exit(EXIT_FAILURE);
	}

	// Set up socket.
	memset(&client.board, 0, sizeof(struct board));
	client.sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client.sockfd == -1) {
		perror("Unable to create socket");
		exit(EXIT_FAILURE);
	}

	// Connect to the server.
	memset(&sockaddr_in, 0, sizeof(sockaddr_in));
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(13500);
	sockaddr_in.sin_addr.s_addr = INADDR_ANY;
	if (connect(client.sockfd, (struct sockaddr*)&sockaddr_in,
		sizeof(sockaddr_in)) == -1) {
		perror("Connecting to server");
		exit(EXIT_FAILURE);
	}

	// Exchange protocol versions.
	memset(&packet, 0, sizeof(struct gls_packet));
	packet.header.event = GLS_EVENT_PROTOVER;
	strlcpy(packet.data.protover.magic, "GLS", GLS_PROTOVER_MAGIC_LENGTH);
	strlcpy(packet.data.protover.version, "0.0",
		GLS_PROTOVER_VERSION_LENGTH);
	strlcpy(packet.data.protover.software, "gls",
		GLS_PROTOVER_SOFTWARE_LENGTH);
	flub = gls_packet_write(&packet, client.sockfd);
	if (flub) {
		fprintf(stderr, "Unable to write protover: '%s'\n",
			flub->message);
		exit(EXIT_FAILURE);
	}
	flub = gls_packet_read(&packet, client.sockfd, 1);
	if (flub) {
		fprintf(stderr, "Unable to read protover ack: '%s'\n",
			flub->message);
		exit(EXIT_FAILURE);
	} else if (packet.header.event != GLS_EVENT_PROTOVERACK) {
		fprintf(stderr, "Expected protover ack ('%u'), got '%u'\n",
			GLS_EVENT_PROTOVERACK, packet.header.event);
		exit(EXIT_FAILURE);
	}
	if (!packet.data.protoverack.ack) {
		fprintf(stderr, "Server refused connection: '%s'\n",
			packet.data.protoverack.reason);
		exit(EXIT_FAILURE);
	}

	// Set nickname.
	flub = client_nickname_write(&client, "knecht");
	if (flub) {
		fprintf(stderr, "Nickname set failed: '%s'\n",
			flub->message);
	} else {
		fprintf(stderr, "Nickname accepted.\n");
	}

	// Play the game (main loop).
	done = 0;
	while (!done) {
		int read_count;
		char command[CLIENT_COMMAND_SIZE];
		const char prompt[] = "> ";

		// Issue command prompt.
		if (write(STDOUT_FILENO, prompt, sizeof(prompt)) <
			sizeof(prompt)) {
			g_log_error("Unable to write prompt: '%s'",
				g_serr(errno));
		}

		// Get command from user.
		memset(command, 0, sizeof(command));
		read_count = read(STDIN_FILENO, command, sizeof(command));
		if (read_count == -1) {
			// Read error.
			g_log_error("Unable to get command: '%s'",
				g_serr(errno));
			done = 1;
			break;
		} else if (!read_count) {
			// EOF.
			done = 1;
			if (write(STDOUT_FILENO, "\n", 1) < 1) {
				g_log_warn("No clean-quit on EOF: '%s'",
					g_serr(errno));
			}
			continue;
		} else if (read_count == sizeof(command)) {
			// Command too long.
			log_warn(&g_log, "Command too long; skipping");
			continue;
		}
		command[read_count - 1] = '\0';

		// Process user's command.
		if (!strcmp(command, "board")) {
			// Print game board.
			board_print(&client.board, STDOUT_FILENO);
		} else if (!strcmp(command, "help") || !strcmp(command, "?")) {
			// Print help message.
			char* message =
				"board: Print the game board.\n"
				"plate <RowColumn>: Print specifed plate.\n"
				"help: Show this help menu.\n"
				"quit: Exit the program.\n"
				"?: Same as 'help'.\n";
			if (write(STDOUT_FILENO, message, strlen(message)) <
				strlen(message)) {
				log_error(&g_log, "Writing help message");
			}
		} else if (!strncmp(command, "plate", 5)) {
			int column;
			int offset;
			int row;
			int scanning;

			// Scan past whitespace.
			offset = 4;
			scanning = 1;
			while (scanning) {
				// Check for buffer overruns.
				if (++offset >= CLIENT_COMMAND_SIZE - 2) {
					log_info(&g_log, "Command too long");
					break;
				}

				// Scan past whitespace.
				if (!isspace(command[++offset])) {
					scanning = 0;
				}
			}
			if (scanning) {
				// Command too long; skip.
				continue;
			}

			// Parse board coordinates.
			if (!isalpha(command[offset])) {
				log_info(&g_log, "Invalid row");
				continue;
			}
			row = command[offset] - 'A';
			if (row < 0) {
				log_info(&g_log, "Row too low");
				continue;
			} else if (row >= BOARD_PLATE_ROW_COUNT) {
				log_info(&g_log, "Row too high");
				continue;
			}
			offset++;
			if (!isdigit(command[offset])) {
				log_info(&g_log, "Invalid column");
				continue;
			}
			column = command[offset] - '1';
			if (column < 0) {
				log_info(&g_log, "Column too low");
				continue;
			} else if (column >= BOARD_PLATE_COLUMN_COUNT) {
				log_info(&g_log, "Column too high");
				continue;
			}

			// Print plate.
			plate_print(&client.board.plates[row][column],
				STDOUT_FILENO);
		} else if (!strcmp(command, "quit")) {
			// Quit the game.
			done = 1;
		} else {
			// Unknown command.
			log_info(&g_log, "Command not recognized");
		}
	}

	// Close connection to the server.
	if (close(client.sockfd) == -1) {
		log_error(&g_log, "Closing connection to server.");
	}

	// Close the logger.
	log_free(&g_log);

	// Return success.
	exit(EXIT_SUCCESS);
}
