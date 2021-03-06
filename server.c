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

void server_handler(int sig) {
	// Set appropriate static signal flag.
	if (sig == SIGINT) {
		server_sigint = 1;
	} else if (sig == SIGTERM) {
		server_sigterm = 1;
	}
}

struct flub* server_init(struct server* server) {
	int sockfd;

	// Create a new game.
	board_init(&server->board);
	memset(server->players, 0, sizeof(struct player) * SERVER_PLAYER_MAX);

	// Set up socket.
	sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (sockfd == -1) {
		return g_flub_toss("Unable to create socket: '%s'",
			g_serr(errno));
	}
	server->sockfd = sockfd;
	memset(&server->sockaddr_in, 0, sizeof(struct sockaddr_in));
	server->sockaddr_in.sin_family = AF_INET;
	server->sockaddr_in.sin_port = htons(13500);
	server->sockaddr_in.sin_addr.s_addr = INADDR_ANY;
	if (bind(server->sockfd, (struct sockaddr*)&server->sockaddr_in,
		sizeof(struct sockaddr_in)) == -1) {
		return g_flub_toss("Socket binding failed: '%s'",
			g_serr(errno));
	}
	if (listen(server->sockfd, 16) == -1) {
		return g_flub_toss("Socket listening failed: '%s'",
			g_serr(errno));
	}

	// Not running.
	server->running = 0;
	return NULL;
}

struct flub* server_player_data(struct server* server, struct player* player) {
	struct gls_packet packet_in;
	struct gls_packet packet_out;
	struct flub* flub;

	// Get player's packet.
	memset(&packet_in, 0, sizeof(struct gls_packet));
	flub = gls_packet_read(&packet_in, player->pipe_server_to[0], 0);
	if (flub) {
		return flub;
	}

	// Handle client data.
	if (!player->protoverokay) { // Protocol version exchange.
		char* protocol = "0.0";
		struct gls_protover* pver;
		struct gls_protoverack* pack;
		int accepted;

		// Validate client protover.
		if (packet_in.header.event != GLS_EVENT_PROTOVER) {
			return g_flub_toss("Expected protover event, got '%u'",
				packet_in.header.event);
		}
		pver = &packet_in.data.protover;
		accepted = 1;
		if (strncmp(pver->version, protocol,
			GLS_PROTOVER_VERSION_LENGTH)) {
			accepted = 0;
		}

		// Return protover ack.
		memset(&packet_out, 0, sizeof(struct gls_packet));
		packet_out.header.event = GLS_EVENT_PROTOVERACK;
		pack = &packet_out.data.protoverack;
		pack->ack = accepted;
		if (!accepted) {
			snprintf(pack->reason, GLS_PROTOVER_REASON_LENGTH,
				"Invalid protocol version '%s' (expected '%s')",
				pver->version, protocol);
		}
		strlcpy(pack->pver.magic, "GLS", GLS_PROTOVER_MAGIC_LENGTH);
		strlcpy(pack->pver.version, "0.0", GLS_PROTOVER_VERSION_LENGTH);
		strlcpy(pack->pver.software, "glsd",
			GLS_PROTOVER_SOFTWARE_LENGTH);
		flub = gls_packet_write(&packet_out, player->sockfd);
		if (!accepted) {
			return g_flub_toss("Protcol version not accepted: %s",
				pack->reason);
		}
		player->protoverokay = 1;
	} else if (!player->authenticated) { // Expect nick request.
		int i;
		int j;
		struct gls_sync_end sync;

		// Read nick request.
		if (packet_in.header.event != GLS_EVENT_NICK_REQ) {
			return g_flub_toss("Expected nick request during "
				"protoverokay phase");
		}

		// Process nick request (updates player to auth).
		flub = server_player_nick(server, player,
			&packet_in.data.nick_req);
		if (flub) {
			return flub_append(flub, "processing player data");
		}

		// Synchronize game state.
		// Send plates.
		for (i = 0; i < GLS_BOARD_ROW_COUNT; i++) {
			for (j = 0; j < GLS_BOARD_COLUMN_COUNT; j++) {
				struct gls_plate_place place;
				struct plate* plate;
				char loc[3];

				// Prepare packet.
				memset(&place, 0, sizeof(place));
				plate = &server->board.plates[i][j];
				strlcpy(place.abbrev, plate->abbrev,
					GLS_PLATE_ABBREV_LENGTH);
				strlcpy(place.description, plate->description,
					GLS_PLATE_DESCRIPTION_LENGTH);
				strlcpy(place.name, plate->name,
					GLS_PLATE_NAME_LENGTH);
				loc[0] = 'A' + i;
				loc[1] = '1' + j;
				loc[2] = '\0';
				strlcpy(place.loc, loc, GLS_LOCATION_LENGTH);
				place.flags = plate->empty ?
					GLS_PLATE_FLAG_EMPTY : 0;

				// Send packet.
				if ((flub = gls_plate_place_write(&place,
					player->sockfd))) {
					return flub_append(flub, "sending plate");

				}
			}
		}
		// Send die placements.
		for (i = 0; i < GLS_DIE_MAX; i++) {
			if (!strlen(server->board.dice[i].location)) {
				// Die not placed.
				continue;
			}
			struct gls_die_place place;
			struct die* die = &server->board.dice[i];
			memset(&place, 0, sizeof(place));
			strlcpy(place.location, die->location,
				GLS_LOCATION_LENGTH);
			strlcpy(place.nick, die->nick, GLS_NICK_LENGTH);
			place.color = die->color;
			place.die = i;
			if ((flub = gls_die_place_write(&place,
				player->sockfd))) {
				return flub_append(flub, "placing die");
			}
		}
		// Sync end.
		memset(&sync, 0, sizeof(struct gls_sync_end));
		strlcpy(sync.motd, "Welcome to the Glass Plate Game test "
			"server!", GLS_MOTD_LENGTH);
		if ((flub = gls_sync_end_write(&sync, player->sockfd))) {
			return flub_append(flub, "synchronizing player");
		}
		player->synchronized = 1;
	} else { // Client generated packet.
		uint32_t die;
		int i;
		struct gls_die_place* place;
		struct gls_say1* say1;
		struct gls_say2* say2;

		switch(packet_in.header.event) {
		case GLS_EVENT_NICK_REQ:
			// Process nick request.
			flub = server_player_nick(server, player,
				&packet_in.data.nick_req);
			if (flub) {
				return flub_append(flub, "processing player "
					"data");
			}
			break;
		case GLS_EVENT_SAY1:
			// Prepare say2 packet.
			say1 = &packet_in.data.say1;
			say2 = &packet_out.data.say2;
			memset(say2, 0, sizeof(struct gls_say2));
			strlcpy(say2->nick, player->nick, GLS_NICK_LENGTH);
			say2->tval = (uint64_t)time(NULL);
			if (say2->tval == (uint64_t)-1) {
				return g_flub_toss("Unable to get time for "
					"Say2 packet: '%s'", g_serr(errno));
			}
			strlcpy(say2->message, say1->message,
				GLS_SAY_MESSAGE_LENGTH);

			// Send say2 packets.
			for (i = 0; i < SERVER_PLAYER_MAX; i++) {
				// Send packet to each player.
				if (!server->players[i].authenticated) {
					continue;
				}
				if ((flub = gls_say2_write(say2,
					server->players[i].sockfd))) {
					g_log_warn("Unable to message player "
						"'%s': '%s'",
						player_name(&server->players[i]),
						flub->message);
					player_kill(&server->players[i]);
				}
			}
			break;
		case GLS_EVENT_DIE_PLACE_TRY:
			// Place die on board.
			if ((flub = board_die_place(&server->board,
				player->nick,
				packet_in.data.die_place_try.location,
				&packet_in.data.die_place_try.color, &die))) {
				// Die not valid; send reject packet.
				struct gls_die_place_reject* reject;
				reject = &packet_out.data.die_place_reject;
				memset(&packet_out, 0, sizeof(packet_out));
				packet_out.header.event =
					GLS_EVENT_DIE_PLACE_REJECT;
				strlcpy(reject->location,
					packet_in.data.die_place_try.location,
					GLS_LOCATION_LENGTH);
				reject->color =
					packet_in.data.die_place_try.color;
				strlcpy(reject->reason, flub->message,
					GLS_DIE_PLACE_REJECT_REASON_LENGTH);
				if ((flub = gls_die_place_reject_write(reject,
					player->sockfd))) {
					// Unable to send reject packet.
					g_log_warn("Error sending die place "
						"reject to player '%s': %s",
						player_name(player),
						flub->message);
					player_kill(player);
					break;
				}
				break;
			}

			// Notify players of placed die.
			memset(&packet_out, 0, sizeof(packet_out));
			packet_out.header.event = GLS_EVENT_DIE_PLACE;
			place = &packet_out.data.die_place;
			strlcpy(place->location,
				packet_in.data.die_place_try.location,
				GLS_LOCATION_LENGTH);
			place->color = packet_in.data.die_place_try.color;
			strlcpy(place->nick, player->nick, GLS_NICK_LENGTH);
			place->die = die;
			for (i = 0; i < SERVER_PLAYER_MAX; i++) {
				// Send place packet to each player.
				if (!server->players[i].authenticated) {
					continue;
				}
				if ((flub = gls_die_place_write(place,
					server->players[i].sockfd))) {
					g_log_warn("Unable to place die for "
						"player '%s': '%s'",
						player_name(
							&server->players[i]),
						flub->message);
					player_kill(&server->players[i]);
				}
			}
			g_log_info("Player '%s' placed die '%u' (%s) at '%s'",
				player->nick, die,
				gls_color_names[
					packet_out.data.die_place.color],
				packet_out.data.die_place.location);
			break;
		default:
			return g_flub_toss("Unsupported event type '%i' from "
				"player", packet_in.header.event);
		}
	}
	return NULL;
}

struct flub* server_player_nick(struct server* server, struct player* player,
	struct gls_nick_req* req) {
	struct gls_nick_change change;
	struct flub* flub;
	int i;
	struct gls_nick_set set;

	// Process nick request.
	memset(&set, 0, sizeof(struct gls_nick_set));
	memset(&change, 0, sizeof(struct gls_nick_change));
	for (i = 0; i < SERVER_PLAYER_MAX; i++) {
		if (!strncmp(server->players[i].nick, req->nick,
			GLS_NICK_LENGTH)) {
			// Nick already in use.
			strlcpy(set.reason, "Already in use",
				GLS_NICK_SET_REASON);
			break;
		}
	}
	if (i == SERVER_PLAYER_MAX) {
		// Nick not in use.
		strlcpy(change.old, player->nick, GLS_NICK_LENGTH);
		strlcpy(change.new, req->nick, GLS_NICK_LENGTH);
		strlcpy(player->nick, req->nick, GLS_NICK_LENGTH);
		strlcpy(set.nick, req->nick, GLS_NICK_LENGTH);
	}
	g_log_info("Player '%s' requested nick '%s' (%s)",
		player->authenticated ? set.nick[0] == '\0' ? player->nick :
		change.old : "(unauthenticated)",
		req->nick, set.nick[0] == '\0' ? set.reason : "accepted");
	flub = gls_nick_set_write(&set, player->sockfd);
	if (flub) {
		return flub_append(flub, "unable to write nick set");
	} else if (set.nick[0] == '\0') {
		// Nick was rejected, done now.
		return NULL;
	}

	// Inform other players.
	if (!player->authenticated) {
		struct gls_player_join join;

		// Minor hack for authentication.
		player->authenticated = 1;

		// Inform other players of join.
		memset(&join, 0, sizeof(join));
		strlcpy(join.nick, player->nick, GLS_NICK_LENGTH);
		for (i = 0; i < SERVER_PLAYER_MAX; i++) {
			if (!server->players[i].authenticated ||
				player == &server->players[i]) {
				// Not playing or is current player.
				continue;
			}
			if ((flub = gls_player_join_write(&join,
				server->players[i].sockfd))) {
				g_log_warn("Unable to inform player '%s' of "
					"player join: %s",
					player_name(&server->players[i]),
					flub->message);
				player_kill(&server->players[i]);
			}
		}
	} else {
		// Send nick change to other players.
		for (i = 0; i < SERVER_PLAYER_MAX; i++) {
			if (!server->players[i].authenticated ||
				player == &server->players[i]) {
				// Not playing or is current player.
				continue;
			}
			if ((flub = gls_nick_change_write(&change,
				server->players[i].sockfd))) {
				g_log_warn("Unable to inform player '%s' of "
					"nick change: %s",
					player_name(&server->players[i]),
					flub->message);
				player_kill(&server->players[i]);
			}
		}
	}
	return NULL;
}

struct flub* server_run(struct server* server) {
	struct flub* flub;
	struct gls_shutdown shutdown;
	int i;
	int j;

	// Run the server.
	server->running = 1;
	while (server->running) {
		int connection;
		nfds_t pollfd_count;
		struct pollfd pollfds[SERVER_PLAYER_MAX];
		int ret;
		socklen_t socklen;

		// Accept new connection.
		connection = accept4(server->sockfd,
			(struct sockaddr*)&server->sockaddr_in,
			&socklen, 0);
		if (connection != -1) {
			struct player* player;

			// Find player slot.
			player = NULL;
			g_log_debug("New connection"); // TODO: Conn info.
			for (i = 0; i < SERVER_PLAYER_MAX; i++) {
				if (server->players[i].connected) {
					// Player slot in use.
					continue;
				}
				player = &server->players[i];
			}
			if (!player) {
				// No player slots available.
				// TODO: Send protover ack with reason.
				g_log_warn("No player slots available");
				if (close(connection) == -1) {
					g_log_warn("Closing connection: '%s'",
						g_serr(errno));
				}
			} else {
				// Initialize new player.
				flub = player_init(player, connection);
				if (flub) {
					g_log_warn("Unable to initialize "
						"player: '%s'", flub->message);
				}
			}
		} else if (connection == -1 && errno != EWOULDBLOCK) {
			// Connection error.
			g_log_warn("Accepting connection failed: '%s'",
				g_serr(errno));
		}

		// Read player data.
		do {
			// Build pollfd list.
			pollfd_count = 0;
			memset(pollfds, 0, sizeof(pollfds));
			for (i = 0; i < SERVER_PLAYER_MAX; i++) {
				if (!server->players[i].connected) {
					// Invalid player slot.
					continue;
				}
				pollfds[pollfd_count].fd =
					server->players[i].pipe_server_to[0];
				pollfds[pollfd_count].events |= POLLIN;
				pollfds[pollfd_count].events |= POLLRDHUP;
				pollfd_count++;
			}

			// Call poll.
			ret = poll(pollfds, pollfd_count, 10);
			if (ret == -1) {
				if (errno != EINTR) {
					g_log_error("Unable to poll: '%s'",
						g_serr(errno));
				}
				break;
			}

			// Read player data.
			for (i = 0; i < pollfd_count; i++) {
				struct player* player;
				struct pollfd* pollfd;

				// Get player.
				pollfd = &pollfds[i];
				for (j = 0; j < SERVER_PLAYER_MAX; j++) {
					if (server->players[j].pipe_server_to[0]
						== pollfd->fd) {
						player = &server->players[j];
					}
				}

				// Check for player data.
				if (pollfd->revents & POLLERR) {
					// Error: disconnect player.
					g_log_warn("Polling error");
					player_kill(player);
					continue;
				} else if (pollfd->revents & POLLHUP ||
					pollfd->revents & POLLRDHUP) {
					struct gls_player_part part;

					// Save player nick.
					memset(&part, 0, sizeof(part));
					strlcpy(part.nick, player->nick,
						GLS_NICK_LENGTH);

					// Player thread done.
					g_log_info("Freeing player '%s'",
						player->nick);
					player_free(player, flub);
					if (flub) {
						g_log_warn("Player error: '%s'",
							flub->message);
					}

					// Inform other players.
					// TODO: Too deep, refactor.
					for (i = 0; i < SERVER_PLAYER_MAX;
						i++) {
						if (!server->players[i].authenticated) {
							continue;
						}
						if ((flub = gls_player_part_write(
							&part, server->players[i].sockfd))) {
							g_log_warn("Unable to inform player '%s' of part: %s",
								player_name(&server->players[i]),
								flub->message);
							player_kill(&server->players[i]);
						}
					}
					continue;
				} else if (pollfd->revents & POLLNVAL) {
					// Invalid request.
					g_log_warn("Invalid poll request");
					player_kill(player);
					continue;
				} else if (!(pollfd->revents & POLLIN)) {
					// No data from player.
					continue;
				}

				// Handle player data.
				flub = server_player_data(server, player);
				if (flub) {
					log_warn(&g_log, "Error handling "
						"player data: '%s'",
						flub->message);
					player_kill(player);
				}
			}
		} while (ret); // Handle player data.

		// Check for signal.
		if (server_sigint) {
			g_log_info("Server received SIGINT");
			server->running = 0;
		} else if (server_sigterm) {
			g_log_info("Server received SIGTERM");
			server->running = 0;
		}
	}

	// Shutdown message.
	if (server_sigint) {
		strlcpy(shutdown.reason, "Server received SIGINT",
			GLS_SHUTDOWN_REASON_LENGTH);
	} else if (server_sigterm) {
		strlcpy(shutdown.reason, "Server received SIGTERM",
			GLS_SHUTDOWN_REASON_LENGTH);
	} else {
		strlcpy(shutdown.reason, "Server shutdown",
			GLS_SHUTDOWN_REASON_LENGTH);
	}
	for (i = 0; i < SERVER_PLAYER_MAX; i++) {
		struct player* player;

		// Send message to players.
		player = &server->players[i];
		if (!player->connected) {
			continue;
		}
		if ((flub = gls_shutdown_write(&shutdown, player->sockfd))) {
			g_log_error("Unable to inform player '%s' of shutdown: "
				"'%s'", player_name(player), flub->message);
		}

		// Remove the player.
		// This is kind of sloppy, and might end up hanging things due
		// to pthreads weirdness.
		player_kill(player);
		player_free(player, flub);
	}

	return NULL;
}

/**
 * Runs the Glass Plate Game server.
 */
int main(int argc, char* argv[]) {
	struct flub* flub;
	struct server server;
	struct sigaction sa;
	int ret;

	// Open log file.
	if (log_init(&g_log, "./glsd.log", LOG_DEBUG, 1) == -1) {
		// Have the logger write to 'stdout'.
		// Hacky, but reasonable, me thinks.
		g_log.fd = STDOUT_FILENO;
	}

	// Setup flub.
	ret = g_serr_init();
	if (ret) {
		g_log_error("Unable to setup system error buffer");
		goto err;
	}
	ret = g_flub_init();
	if (ret) {
		g_log_error("Unable to initialize flub: '%s'", g_serr(ret));
		goto err;
	}
	flub = gls_init();
	if (flub) {
		g_log_error("Unable to initialize gls buffer: '%s'",
			flub->message);
		goto err;
	}

	// Ignore 'SIGPIPE' signals.
	// Note that, according to the SIGNAL(2) man page dated 2014-08-19,
	// this is one of the few portable uses of 'singal'; if this gets
	// refactored in order to use a signal handler function, change to
	// the 'sigaction' system call.
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		g_log_error("Unable to ignore SIGPIPE: '%s'", g_serr(errno));
		goto err;
	}

	// Handle 'SIGINT' and 'SIGTERM' signals.
	sa.sa_handler = server_handler;
	sa.sa_flags = SA_RESTART | SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		g_log_error("Unable to setup SIGINT handler: '%s'",
			g_serr(errno));
		goto err;
	}
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		g_log_error("Unable to setup SIGTERM handler: '%s'",
			g_serr(errno));
		goto err;
	}

	// Setup server.
	flub = server_init(&server);
	if (flub) {
		log_error(&g_log, "Unable to initialize server: '%s'",
			flub->message);
		goto err;
	}

	// Run the server.
	g_log_info("Running server");
	flub = server_run(&server);
	if (flub) {
		log_error(&g_log, "Error running server: '%s'", flub->message);
		goto err;
	}

	// Stop logging.
	if (log_free(&g_log) == -1) {
		fprintf(stderr, "Unable to close log file: '%s'",
			strerror(errno));
	}

	// Exit the program.
	exit(EXIT_SUCCESS);

err:
	// Stop logging.
	if (log_free(&g_log) == -1) {
		fprintf(stderr, "Unable to close log file: '%s'",
			strerror(errno));
	}
	exit(EXIT_FAILURE);
}
