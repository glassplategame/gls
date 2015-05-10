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

#include <errno.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "board.h"
#include "log.h"

// Global buffer for holding messages.
char buffer[256];

/**
 * Runs the Glass Plate Game server.
 */
int main(int argc, char* argv[]) {
	struct board board;
	int running;
	struct sockaddr_in sockaddr_in;
	int sockfd;

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
		snprintf(buffer, sizeof(buffer), "Unable to ignore SIGPIPE: %s",
			strerror(errno));
		log_error(&g_log, buffer);
		exit(EXIT_FAILURE);
	}

	// Create a new game.
	board_init(&board);

	// Set up socket.
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == -1) {
		snprintf(buffer, sizeof(buffer),
			"Unable to create socket: %s", strerror(errno));
		log_error(&g_log, buffer);
		exit(EXIT_FAILURE);
	}
	memset(&sockaddr_in, 0, sizeof(sockaddr_in));
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(13500);
	sockaddr_in.sin_addr.s_addr = INADDR_ANY;
	if (bind(sockfd, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1) {
		snprintf(buffer, sizeof(buffer),
			"Socket binding failed: %s", strerror(errno));
		log_error(&g_log, buffer);
		exit(EXIT_FAILURE);
	}
	if (listen(sockfd, 16) == -1) {
		snprintf(buffer, sizeof(buffer),
			"Socket listening failed: %s", strerror(errno));
		log_error(&g_log, buffer);
		exit(EXIT_FAILURE);
	}

	// Run the server.
	running = 1;
	while (running) {
		socklen_t socklen;
		int connection;

		// Accept new connection.
		log_debug(&g_log, "Connecting");
		connection = accept(sockfd, (struct sockaddr*)&sockaddr_in,
			&socklen);
		if (connection == -1) {
			snprintf(buffer, sizeof(buffer),
				"Accepting connection failed: %s",
				strerror(errno));
			log_warn(&g_log, buffer);
			continue;
		}
		log_info(&g_log, "Connected");

		// Send connected player the current game.
		if (board_write(&board, connection) == -1) {
			log_error(&g_log, "Sending board");
		}

		// Disconnect the player.
		if (close(connection) == -1) {
			log_error(&g_log, "Closing connection");
		}
	}

	// Stop logging.
	log_free(&g_log);

	// Exit the program.
	exit(EXIT_SUCCESS);
}
