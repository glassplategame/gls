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
	struct gls_nick_set set;
	struct gls_nick_req req;
	int ret;

	// Attempt to set nickname.
	memset(&req, 0, sizeof(struct gls_nick_req));
	strlcpy(req.nick, nickname, GLS_NICK_LENGTH);
	while (1) {
		// Write nickname to server.
		flub = gls_nick_req_write(&req, client->sockfd);
		if (flub) {
			return flub_append(flub, "unable to write nickname");
		} else {
			g_log_info("Nickname '%s' requested.", req.nick);
		}

		// Read nickname from server.
		flub = gls_header_read(&header, client->sockfd);
		if (flub) {
			// Unable to read header.
			return flub_append(flub, "unable to read gls header");
		}
		if (header.event != GLS_EVENT_NICK_SET) {
			// Unexpected event.
			return g_flub_toss("Unexpected event: '%x'", header.event);
		}
		flub = gls_nick_set_read(&set, client->sockfd, 1);
		if (flub) {
			return flub_append(flub, "unable to get nick set");
		}
		if (set.nick[0] != '\0') {
			// Nickname accepted.
			break;
		}

		// Nickname rejected.
		g_log_info("Nickname rejected: '%s'", set.reason);
		while (1) {
			const char* message = "Please request new nickname "
				"(must be 31 alphanumeric characters or less) "
				"by typing it and then pressing the 'Enter' "
				"key: ";
			if (write(STDOUT_FILENO, message, strlen(message)) ==
				-1) {
				return g_flub_toss("Unable to write new nick "
					"req prompt: '%s'", g_serr(errno));
			}
			memset(&req, 0, sizeof(struct gls_nick_req));
			if ((ret = read(STDIN_FILENO, &req.nick,
				GLS_NICK_LENGTH)) == -1) {
				return g_flub_toss("Unable to read new nick "
					"req: '%s'", g_serr(errno));
			}
			req.nick[ret - 1] = '\0';
			if ((flub = gls_nick_validate(req.nick, 0))) {
				g_log_info("Invalid nick: '%s'", flub->message);
				continue;
			}
			break;
		}
	}
	return NULL;
}

int main(int argc, char* argv[]) {
	struct cargs cargs;
	struct client client;
	int done;
	char errbuf[128];
	struct flub* flub;
	struct gls_packet packet;
	struct sockaddr_in sockaddr_in;
	int ret;

	// Set up the globals.
	if (log_init(&g_log, NULL, LOG_DEBUG, 0) == -1) {
		fprintf(stderr, "Unable to open log, QUITTING!");
		exit(EXIT_FAILURE);
	}
	if ((ret = g_serr_init())) {
		g_log_error("Unable to create system error buffer");
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

	// Parse arguments.
	flub = cargs_parse(&cargs, argc, argv);
	if (flub) {
		g_log_error("Unable to parse arguments: '%s'", flub->message);
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
	flub = client_nickname_write(&client, cargs.nick);
	if (flub) {
		fprintf(stderr, "Nickname set failed: '%s'\n",
			flub->message);
		exit(EXIT_FAILURE);
	} else {
		fprintf(stderr, "Nickname accepted.\n");
	}

	// Synchronize.
	do {
		int i;
		struct plate* plate;

		// Check header.
		if ((flub = gls_header_read(&packet.header, client.sockfd))) {
			g_log_error("Unable to read header: '%s'",
			flub->message);
			exit(EXIT_FAILURE);
		}
		if (packet.header.event == GLS_EVENT_SYNC_END) {
			// End of sync.
			break;
		} else if (packet.header.event != GLS_EVENT_PLATE_PLACE) {
			g_log_error("Unexpected event: '%u'",
				packet.header.event);
			exit(EXIT_FAILURE);
		}

		// Read in plate.
		if ((flub = gls_plate_place_read(&packet.data.plate_place,
			client.sockfd, 1))) {
			g_log_error("Unable to read plate: '%s'",
				flub->message);
			exit(EXIT_FAILURE);
		}

		// Copy plate to game board.
		i = ((packet.data.plate_place.loc[0] - 'A') * 8) +
			((packet.data.plate_place.loc[1] - '1'));
		plate = ((struct plate*)client.board.plates) + i;
		strlcpy(plate->abbrev, packet.data.plate_place.abbrev,
			GLS_PLATE_ABBREV_LENGTH);
		strlcpy(plate->description, packet.data.plate_place.description,
			GLS_PLATE_DESCRIPTION_LENGTH);
		strlcpy(plate->name, packet.data.plate_place.name,
			GLS_PLATE_NAME_LENGTH);
		plate->empty = packet.data.plate_place.flags &
			GLS_PLATE_FLAG_EMPTY;
	} while (1);
	if ((flub = gls_sync_end_read(&packet.data.sync_end, client.sockfd,
		1))) {
		g_log_error("Unable to read sync_end packet: '%s'",
		flub->message);
		exit(EXIT_FAILURE);
	}
	if (strlen(packet.data.sync_end.motd)) {
		g_log_info("MotD: '%s'", packet.data.sync_end.motd);
	}

	// Play the game (main loop).
	done = 0;
	const int REGMATCH_COUNT = 3;
	regex_t regex_board;
	regex_t regex_command;
	regex_t regex_help;
	regex_t regex_nick;
	regex_t regex_plate;
	regex_t regex_quit;
	regmatch_t regmatch[REGMATCH_COUNT];
	if ((ret = regcomp(&regex_board, "^board\\s*$",
		REG_EXTENDED | REG_NOSUB))) {
		regerror(ret, &regex_board, errbuf, sizeof(errbuf));
		g_log_error("Unable to compile board regex: '%s'", errbuf);
		exit(EXIT_FAILURE);
	} else if ((ret = regcomp(&regex_command, "^/(.*)$",
		REG_EXTENDED))) {
		regerror(ret, &regex_command, errbuf, sizeof(errbuf));
		g_log_error("Unable to compile command regex: '%s'", errbuf);
		exit(EXIT_FAILURE);
	} else if ((ret = regcomp(&regex_help, "^(help|\\?)\\s*$",
		REG_EXTENDED | REG_NOSUB))) {
		regerror(ret, &regex_help, errbuf, sizeof(errbuf));
		g_log_error("Unable to compile help regex: '%s'", errbuf);
		exit(EXIT_FAILURE);
	} else if ((ret = regcomp(&regex_nick, "^nick(\\s+(\\w+))?\\s*$",
		REG_EXTENDED))) {
		regerror(ret, &regex_nick, errbuf, sizeof(errbuf));
		g_log_error("Unable to compile nick regex: '%s'", errbuf);
		exit(EXIT_FAILURE);
	} else if ((ret = regcomp(&regex_plate, "^plate(\\s+(\\w+))?\\s*$",
		REG_EXTENDED))) {
		regerror(ret, &regex_plate, errbuf, sizeof(errbuf));
		g_log_error("Unable to compile plate regex: '%s'", errbuf);
		exit(EXIT_FAILURE);
	} else if ((ret = regcomp(&regex_quit, "^quit\\s*$",
		REG_EXTENDED | REG_NOSUB))) {
		regerror(ret, &regex_quit, errbuf, sizeof(errbuf));
		g_log_error("Unable to compile quit regex: '%s'", errbuf);
		exit(EXIT_FAILURE);
	}
	while (!done) {
		int read_count;
		char* cmd;
		char command[CLIENT_COMMAND_SIZE];
		struct gls_packet packet;
		const char prompt[] = "> ";

		// Read data from server.
		do {
			time_t tval;
			struct tm tm;
			char tstr[10];

			// Check for data from server.
			if (ioctl(client.sockfd, FIONREAD, &ret) == -1) {
				g_log_error("Unable to peek socket read end: "
					"'%s'", g_serr(errno));
				done = 1;
				break;
			}
			if (!ret) {
				// No data from server.
				break;
			}

			// Read data from server.
			if ((flub = gls_packet_read(&packet, client.sockfd,
				1))) {
				g_log_error("Unable to read packet: '%s'",
					flub->message);
				done = 1;
				break;
			}
			switch (packet.header.event) {
			case (GLS_EVENT_NICK_SET):
				if (!strlen(packet.data.nick_set.nick)) {
					g_log_info("Server rejected nickname: "
						"'%s'",
						packet.data.nick_set.reason);
				} else {
					g_log_info("Server set nickname to "
						"'%s'",
						packet.data.nick_set.nick);
				}
				break;
			case (GLS_EVENT_NICK_CHANGE):
				g_log_info("Player '%s' is now known as '%s'",
					packet.data.nick_change.old,
					packet.data.nick_change.new);
				break;
			case (GLS_EVENT_PLAYER_JOIN):
				g_log_info("Player '%s' has joined",
					packet.data.player_join.nick);
				break;
			case (GLS_EVENT_PLAYER_PART):
				g_log_info("Player '%s' has parted",
					packet.data.player_part.nick);
				break;
			case (GLS_EVENT_SHUTDOWN):
				g_log_info("Server shutdown: '%s'",
					packet.data.shutdown.reason);
				done = 1;
				break;
			case (GLS_EVENT_SAY2):
				// Format time.
				tval = (time_t)packet.data.say2.tval;
				if (localtime_r(&tval, &tm) == NULL) {
					g_log_warn("Unable to get localtime: "
						"'%s'", g_serr(errno));
					strlcpy(tstr, "??:??", sizeof(tstr));
				} else {
					if (!strftime(tstr, sizeof(tstr),
						"%H:%M", &tm)) {
						g_log_warn("Unable to format "
							"time string");
						strlcpy(tstr, "??:??",
							sizeof(tstr));
					}
				}

				// Display message.
				g_log_info("%s %s: %s", tstr,
					packet.data.say2.nick,
					packet.data.say2.message);
				break;
			default:
				g_log_error("Unknown event: '%i'",
					packet.header.event);
				done = 1;
				break;
			}
		} while (1);
		if (done) {
			break;
		}

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
			g_log_warn("Command too long (must be < %i bytes); "
				"skipping", CLIENT_COMMAND_SIZE);
			continue;
		}
		command[read_count - 1] = '\0';

		// Process user's command.
		if (!strlen(command)) {
			continue;
		} else if (regexec(&regex_command, command, REGMATCH_COUNT,
			regmatch, 0)) {
			struct gls_say1 say;

			// Prepare Say1 packet.
			memset(&say, 0, sizeof(struct gls_say1));
			if (strlcpy(say.message, command,
				GLS_SAY_MESSAGE_LENGTH) >=
				GLS_SAY_MESSAGE_LENGTH) {
				// TODO: Break long messages into little
				// messages.
				g_log_warn("Message too long; must be less "
					"than '%i' characters",
					GLS_SAY_MESSAGE_LENGTH);
				continue;
			}

			// Write Say1 packet to server.
			if ((flub = gls_say1_write(&say, client.sockfd))) {
				g_log_warn("Unable to send message: '%s'",
					flub->message);
			}
			continue;
		}
		cmd = &command[regmatch[1].rm_so];
		if (!regexec(&regex_board, cmd, REGMATCH_COUNT, regmatch, 0)) {
			// Print game board.
			board_print(&client.board, STDOUT_FILENO);
		} else if (!regexec(&regex_help, cmd, 0, NULL, 0)) {
			// Print help message.
			char* message =
				"/board: Print the game board.\n"
				"/help: Show this help menu.\n"
				"/nick <nick>: Request specified nickname.\n"
				"/plate <RowColumn>: Print specifed plate.\n"
				"/quit: Exit the program.\n"
				"/?: Same as 'help'.\n";
			if (write(STDOUT_FILENO, message, strlen(message)) <
				strlen(message)) {
				g_log_error("Writing help message");
			}
		} else if (!regexec(&regex_nick, cmd, REGMATCH_COUNT, regmatch,
			0)) {
			// Send Nick Request.
			struct gls_nick_req req;
			regoff_t len;

			// Check for nick argument.
			if (regmatch[2].rm_so == -1) {
				g_log_warn("Missing nickname");
				continue;
			}

			// Read nick from client.
			memset(&req, 0, sizeof(struct gls_nick_req));
			len = regmatch[2].rm_eo - regmatch[2].rm_so + 1;
			if (strlcpy(req.nick, &cmd[regmatch[2].rm_so],
				len < GLS_NICK_LENGTH ? len : GLS_NICK_LENGTH)
				>= GLS_NICK_LENGTH) {
				g_log_warn("Nick must be less than '%i' "
					"characters", GLS_NICK_LENGTH);
				continue;
			} else if ((flub = gls_nick_validate(req.nick, 0))) {
				g_log_warn("Invalid nick: '%s'", flub->message);
				continue;
			}

			// Send nick request to server.
			if ((flub = gls_nick_req_write(&req, client.sockfd))) {
				g_log_warn("Unable to send nick request: '%s'",
					flub->message);
				break;
			}
			g_log_info("Requested nick '%s'", req.nick);
		} else if (!regexec(&regex_plate, cmd, REGMATCH_COUNT, regmatch,
			0)) {
			// Print specified plate.
			int column;
			int offset;
			int row;

			// Check for plate specification.
			if (regmatch[2].rm_so == -1) {
				g_log_warn("Missing plate location");
				continue;
			}

			// Parse board coordinates.
			// TODO: Deal with lowercase, multiple letters/numbers,
			// make less sucky in general.
			offset = regmatch[2].rm_so;
			if (!isalpha(cmd[offset])) {
				g_log_info("Invalid row '%c'", cmd[offset]);
				continue;
			}
			row = cmd[offset] - 'A';
			if (row < 0) {
				g_log_info("Row '%c' too low", cmd[offset]);
				continue;
			} else if (row >= GLS_BOARD_ROW_COUNT) {
				g_log_info("Row '%c' too high (must be less "
					"than '%c')", cmd[offset],
					GLS_BOARD_ROW_COUNT + 'A');
				continue;
			}
			offset++;
			if (!isdigit(cmd[offset])) {
				g_log_info("Invalid column '%c'; expected "
					"digit", cmd[offset]);
				continue;
			}
			column = cmd[offset] - '1';
			if (column < 0) {
				g_log_info("Column '%c' too low", cmd[offset]);
				continue;
			} else if (column >= GLS_BOARD_COLUMN_COUNT) {
				g_log_info("Column '%c' too high (must be "
				"less than '%c')", cmd[offset],
				GLS_BOARD_COLUMN_COUNT + '1');
				continue;
			}

			// Print plate.
			plate_print(&client.board.plates[row][column],
				STDOUT_FILENO);
		} else if (!regexec(&regex_quit, cmd, REGMATCH_COUNT, regmatch,
			0)) {
			// Quit the game.
			done = 1;
		} else {
			// Unknown command.
			g_log_info("Command not recognized");
		}
	} // Playing.

	// Close connection to the server.
	if (close(client.sockfd) == -1) {
		log_error(&g_log, "Closing connection to server.");
	}

	// Close the logger.
	log_free(&g_log);

	// Return success.
	exit(EXIT_SUCCESS);
}
