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

#include "server.h"

int server_init(struct server* server) {
	int sockfd;

	// Reset sequence number.
	server->seqnum = 0;

	// Create a new game.
	board_init(&server->board);
	memset(server->players, 0, sizeof(struct player) * SERVER_PLAYER_MAX);

	// Set up socket.
	sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (sockfd == -1) {
		log_error(&g_log, g_serror("Unable to create socket"));
		return -1;
	}
	server->sockfd = sockfd;
	memset(&server->sockaddr_in, 0, sizeof(struct sockaddr_in));
	server->sockaddr_in.sin_family = AF_INET;
	server->sockaddr_in.sin_port = htons(13500);
	server->sockaddr_in.sin_addr.s_addr = INADDR_ANY;
	if (bind(server->sockfd, (struct sockaddr*)&server->sockaddr_in,
		sizeof(struct sockaddr_in)) == -1) {
		log_error(&g_log, g_serror("Socket binding failed"));
		return -1;
	}
	if (listen(server->sockfd, 16) == -1) {
		log_error(&g_log, g_serror("Socket listening failed"));
		return -1;
	}

	// Not running.
	server->running = 0;
	return 0;
}

server_run(struct server* server) {
	int i;
	int j;

	// Run the server.
	server->running = 1;
	while (server->running) {
		int connection;
		int pollfd_count;
		struct pollfd pollfds[SERVER_PLAYER_MAX];
		int ret;
		socklen_t socklen;

		// Accept new connection.
		connection = accept4(server->sockfd,
			(struct sockaddr*)&server->sockaddr_in,
			&socklen, SOCK_NONBLOCK);
		if (connection != -1) {
			struct player* player;

			// Find player slot.
			player = NULL;
			log_debug(&g_log, "New connection");
			for (i = 0; i < SERVER_PLAYER_MAX; i++) {
				if (server->players[i].connected) {
					// Player slot in use.
					continue;
				}
				player = &server->players[i];
			}
			if (!player) {
				// No player slots available.
				log_error(&g_log, "No player slots available");
				if (close(connection) == -1) {
					g_serror("Closing connection");
				}
			} else {
				// Initialize new player.
				player_init(player, connection);

				// Send player sequence number.
				if (write(player->sockfd, &server->seqnum,
					sizeof(uint32_t)) != sizeof(uint32_t)) {
					// Send failed.
					log_info(&g_log,
						"Sequence server send failed; "
						"player disconnected");
					close(connection);
					memset(player, 0,
						sizeof(struct player)); // FIXME
				}
			}
		} else if (connection == -1 && errno != EWOULDBLOCK) {
			// Connection error.
			log_warn(&g_log,
				g_serror("Accepting connection failed"));
		}

		// Read player data.
		do {
			// Build pollfd list.
			pollfd_count = 0;
			memset(&pollfds, 0, sizeof(pollfds));
			for (i = 0; i < SERVER_PLAYER_MAX; i++) {
				if (!server->players[i].connected) {
					// Invalid player slot.
					continue;
				}
				pollfds[pollfd_count].fd =
					server->players[i].sockfd;
				pollfds[pollfd_count].events |= POLLIN;
				pollfds[pollfd_count].events |= POLLRDHUP;
				pollfd_count++;
			}

			// Call poll.
			char buffer[256];
			ret = poll(pollfds, pollfd_count, 10);
			if (ret == -1) {
				log_error(&g_log, g_serror(
					"Polling players fds"));
				break;
			}
			snprintf(buffer, sizeof(buffer), "Poll ret val: %i", ret);
			log_info(&g_log, buffer);

			// Read player data.
			for (i = 0; i < pollfd_count; i++) {
				struct gls gls;
				struct player* player;
				struct pollfd* pollfd;
				int ret;

				// Get player.
				pollfd = &pollfds[i];
				for (j = 0; j < SERVER_PLAYER_MAX; j++) {
					if (server->players[j].sockfd ==
						pollfd->fd) {
						player = &server->players[j];
					}
				}

				// Check for player data.
				if (pollfd->revents & POLLERR) {
					// Error: disconnect player.
					log_warn(&g_log, "Polling error");
					player_free(player);
					continue;
				} else if (pollfd->revents & POLLHUP ||
					pollfd->revents & POLLRDHUP) {
					// Player hangup.
					log_info(&g_log, "Player hangup");
					player_free(player);
					continue;
				} else if (pollfd->revents & POLLNVAL) {
					// Invalid request.
					log_warn(&g_log, "Invalid poll request");
					player_free(player);
					continue;
				} else if (!(pollfd->revents & POLLIN)) {
					// No data from player.
					continue;
				}
				snprintf(buffer, sizeof(buffer), "revents: %i", pollfd->revents);
				log_info(&g_log, buffer);

				// Read player data.
				memset(&gls, 0, sizeof(gls));
				if ((ret = read(pollfd->fd, &gls,
					sizeof(gls))) == -1) {
					// Read error.
					log_warn(&g_log, g_serror(
						"Reading player data"));
					continue;
				} else if (ret != sizeof(gls)) {
					snprintf(buffer, sizeof(buffer), "ret: %i", ret);
					log_info(&g_log, buffer);
					log_warn(&g_log, g_serror("Unexpected "
						"buffer length")); // TODO
					continue;
				}
				if (gls.event != GLS_EVENT_NICK_REQ) {
					// Unsupported event.
					log_warn(&g_log,
						"Unsupported event type");
					continue;
				}

				// Handle client event.
				uint32_t seqnumc;
				seqnumc = gls.data.nick_req.seqnumc;
				strncpy(player->name, gls.data.nick_req.nick,
					PLAYER_NAME_LENGTH);
				player->name[PLAYER_NAME_LENGTH - 1] = '\0';
				player->authenticated = 1;

				// Reply to client event.
				memset(&gls, 0, sizeof(gls));
				gls.event = GLS_EVENT_NICK_REPLY;
				gls.data.nick_reply.seqnumc = seqnumc;
				strncpy(gls.data.nick_reply.nick, player->name,
					PLAYER_NAME_LENGTH);
				gls.data.nick_reply.accepted = 1;
				if (write(player->sockfd, &gls, sizeof(gls))
					== -1) {
					log_warn(&g_log, g_serror("Unable to "
						"reply to nick event"));
					player_free(player);
				}
			}
		} while (ret);
	}
}

/**
 * Runs the Glass Plate Game server.
 */
int main(int argc, char* argv[]) {
	struct server server;
	struct sockaddr_in sockaddr_in;

	// Open log file.
	if (log_init(&g_log, "./glsd.log", LOG_DEBUG) == -1) {
		// Have the logger write to 'stdout'.
		// Hacky, but reasonable, me thinks.
		g_log.fd = STDOUT_FILENO;
	}

	// Ignore 'SIGPIPE' signals.
	// Note that, according to the SIGNAL(2) man page dated 2014-08-19,
	// this is one of the few portable uses of 'singal'; if this gets
	// refactored in order to use a signal handler function, change to
	// the 'sigaction' system call.
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		log_error(&g_log, g_serror("Unable to ignore SIGPIPE"));
		exit(EXIT_FAILURE);
	}

	// Setup server.
	if (server_init(&server) == -1) {
		log_error(&g_log, "Initializing server");
		exit(EXIT_FAILURE);
	}

	// Run the server.
	log_info(&g_log, "Running server");
	if (server_run(&server) == -1) {
		log_error(&g_log, "Running server");
		exit(EXIT_FAILURE);
	}

	// Stop logging.
	log_free(&g_log);

	// Exit the program.
	exit(EXIT_SUCCESS);
}
