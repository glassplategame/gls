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

#include <errno.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "board.h"
#include "global.h"

int main(int argc, char* argv[]) {
	struct board board;
	const int command_size = 1024;
	int done;
	struct sockaddr_in sockaddr_in;
	int sockfd;
	socklen_t socklen;

	// Set up the logger.
	// FIXME: Don't bypass the proper init method.
	g_log.fd = STDOUT_FILENO;
	g_log.level = LOG_DEBUG;

	// Set up socket.
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == -1) {
		perror("Unable to create socket");
		exit(EXIT_FAILURE);
	}

	// Connect to the server.
	memset(&sockaddr_in, 0, sizeof(sockaddr_in));
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(13500);
	sockaddr_in.sin_addr.s_addr = INADDR_ANY;
	if (connect(sockfd, (struct sockaddr*)&sockaddr_in,
		sizeof(sockaddr_in)) == -1) {
		perror("Connecting to server");
		exit(EXIT_FAILURE);
	}

	// Read in the game board.
	if (board_read(&board, sockfd) == -1) {
		log_error(&g_log, "Getting board from server.");
	}

	// Print the game board.
	if (board_print(&board, STDOUT_FILENO) == -1) {
		log_error(&g_log, "Printing board.");
	}

	// Play the game (main loop).
	done = 0;
	while (!done) {
		int read_count;
		char command[command_size];
		const char prompt[] = "> ";

		// Issue command prompt.
		if (write(STDOUT_FILENO, prompt, sizeof(prompt)) <
			sizeof(prompt)) {
			log_error(&g_log, g_serror("Writing prompt"));
		}

		// Get command from user.
		memset(command, 0, sizeof(command));
		read_count = read(STDIN_FILENO, command, sizeof(command));
		if (read_count == -1) {
			// Read error.
			log_error(&g_log, g_serror("Getting command"));
			done = 1;
			break;
		} else if (!read_count) {
			// EOF.
			done = 1;
			if (write(STDOUT_FILENO, "\n", 1) < 1) {
				log_warn(&g_log, g_serror("Clean-quit on EOF"));
			}
			continue;
		} else if (read_count == sizeof(command)) {
			// Command too long.
			log_warn(&g_log, "Command too long; skipping");
			continue;
		}
		command[read_count - 1] = '\0';

		// Process user's command.
		if (!strcmp(command, "quit")) {
			done = 1;
		} else {
			log_warn(&g_log, "Command not recognized");
		}
	}

	// Close connection to the server.
	if (close(sockfd) == -1) {
		log_error(&g_log, "Closing connection to server.");
	}

	// Close the logger.
	log_free(&g_log);

	// Return success.
	exit(EXIT_SUCCESS);
}
