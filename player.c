/**
 *  The server's conception of a player.
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
#include "player.h"

void player_free(struct player* player, struct flub* status) {
	struct flub* flub;
	int ret;

	// Join player thread.
	ret = pthread_join(player->thread, (void**)&flub);
	if (ret) {
		g_log_warn("Unable to join player thread: '%s'",
			g_serr(ret));
	}
	if (flub) {
		g_log_warn("Player error: '%s'", flub->message);
	}

	// Close to-server pipe.
	if (close(player->pipe_server_to[0]) == -1) {
		g_log_warn("Unable to close to-server read end: '%s'",
			g_serr(errno));
	}

	// Close from-server pipe.
	if (close(player->pipe_server_from[0]) == -1) {
		g_log_warn("Unable to close from-server read end: '%s'",
			g_serr(errno));
	}
	if (!player->killed) { // Server "kills" player by closing write end.
		if (close(player->pipe_server_from[1]) == -1) {
			g_log_warn("Unable to close from-server write end: "
				"'%s'", g_serr(errno));
		}
	}

	// Close socket.
	if (close(player->sockfd)) {
		g_log_warn("Closing player socket: '%s'", g_serr(errno));
	}

	// Clear data.
	memset((void*)player, 0, sizeof(struct player));
}

struct flub* player_init(struct player* player, int fd) {
	int flags;
	struct flub* flub;
	int ret;
	long size;
	const struct timeval timeval = {60, 0};

	// Clear any previous data.
	memset((void*)player, 0, sizeof(struct player));

	// Set socket.
	player->sockfd = fd;
	player->connected = 1;
	if (setsockopt(player->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeval,
		sizeof(timeval)) == -1) {
		flub = g_flub_toss("Unable to set socket recieve timeout: "
			"'%s'", g_serr(errno));
		goto out;
	}

	// Create from-server pipe.
	if (pipe(player->pipe_server_from) == -1) {
		flub = g_flub_toss("Unable to create from-server pipe: "
			"'%s'", g_serr(errno));
		goto out;
	}
	ret = fcntl(player->pipe_server_from[1], F_GETPIPE_SZ);
	if (ret != G_PLAYER_PIPE_WRITE_SIZE) {
		// Resize pipe.
		if (fcntl(player->pipe_server_from[1], F_SETPIPE_SZ,
			G_PLAYER_PIPE_WRITE_SIZE) == -1) {
			flub = g_flub_toss("Unable to set from-server pipe "
				"size to '%u': '%s'", G_PLAYER_PIPE_WRITE_SIZE,
				g_serr(errno));
			goto out2;
		}
	}
	errno = 0;
	size = fpathconf(player->pipe_server_from[1], _PC_PIPE_BUF);
	if (size == -1 && errno) {
		flub = g_flub_toss("Unable to check from-server atomicity: "
			"'%s'", g_serr(errno));
		goto out2;
	} else if (size != -1 && size < GLS_PIPE_BUF) {
		flub = g_flub_toss("From-server pipe atomicity too small, "
			"was '%li', >='%u' expected", size, GLS_PIPE_BUF);
		goto out2;
	}
	flags = fcntl(player->pipe_server_from[1], F_GETFL);
	if (flags == -1) {
		flub = g_flub_toss("Unable to get from-server flags: '%s'",
			g_serr(errno));
		goto out2;
	}
	flags |= O_NONBLOCK;
	if(fcntl(player->pipe_server_from[1], F_SETFL, flags) == -1) {
		flub = g_flub_toss("Unable to set non-blocking on from-server "
			"flags: '%s'", g_serr(errno));
		goto out2;
	}

	// Create to-server pipe.
	if (pipe(player->pipe_server_to) == -1) {
		flub = g_flub_toss("Unable to create to-server pipe: "
			"'%s'", g_serr(errno));
		goto out2;
	}
	errno = 0;
	size = fpathconf(player->pipe_server_to[1], _PC_PIPE_BUF);
	if (size == -1 && errno) {
		flub = g_flub_toss("Unable to check to-server atomicity: "
			"'%s'", g_serr(errno));
		goto out3;
	} else if (size != -1 && size < GLS_PIPE_BUF) {
		flub = g_flub_toss("To-server pipe atomicity too small, "
			"was '%li', >='%u' expected", size, GLS_PIPE_BUF);
		goto out3;
	}
	flags = fcntl(player->pipe_server_to[0], F_GETFL);
	if (flags == -1) {
		flub = g_flub_toss("Unable to get to-server flags: '%s'",
			g_serr(errno));
		goto out3;
	}
	flags |= O_NONBLOCK;
	if (fcntl(player->pipe_server_to[0], F_SETFL, flags) == -1) {
		flub = g_flub_toss("Unable to set non-blocking on to-server "
			"flags: '%s'", g_serr(errno));
		goto out3;
	}

	// Create player thread.
	ret = pthread_create(&player->thread, NULL, player_thread,
		(void*)player);
	if (ret) {
		flub = g_flub_toss("Unable to create pthread: '%s'",
			g_serr(ret));
		goto out3;
	}
	return NULL;

out3:
	// Close to-server pipe.
	if (close(player->pipe_server_to[0]) == -1) {
		flub_append(flub, "unable to close to-server read end (%s)",
			g_serr(errno));
	}
	if (close(player->pipe_server_to[1]) == -1) {
		flub_append(flub, "unable to close to-server write end (%s)",
			g_serr(errno));
	}

out2:
	// Close from-server pipe.
	if (close(player->pipe_server_from[0]) == -1) {
		flub_append(flub, "unable to close from-server read end (%s)",
			g_serr(errno));
	}
	if (close(player->pipe_server_from[1]) == -1) {
		flub_append(flub, "unable to close from-server write end(%s)",
			g_serr(errno));
	}

out:
	// Close socket.
	if (close(player->sockfd) == -1) {
		flub_append(flub, "unable to close player socket (%s)",
			g_serr(errno));
	}
	player->connected = 0;
	return flub;
}

struct flub* player_kill(struct player* player) {
	int flags;

	// Close from-server pipe write end.
	if (close(player->pipe_server_from[1]) == -1) {
		g_log_warn("Unable to close from-server write end: '%s'",
			g_serr(errno));
	}

	// Set socket to non-blocking.
	flags = fcntl(player->sockfd, F_GETFD);
	if (flags == -1) {
		g_log_warn("Unable to get socket flags: '%s'", g_serr(errno));
	} else {
		flags |= O_NONBLOCK;
		if (fcntl(player->sockfd, F_SETFD, flags) == -1) {
			g_log_warn("Unable to set socket flags: '%s'",
				g_serr(errno));
		}
	}

	// Set killed flag.
	player->killed = 1;
	return NULL;
}

char* player_name(struct player* player) {
	if (!player->connected) {
		// Not connected.
		strlcpy(player->name, "(nobody)", GLS_NICK_LENGTH);
	} else if (!player->protoverokay || !player->authenticated) {
		// Socket metadata.
		socklen_t addrlen;
		struct sockaddr_in addr;

		memset(&addr, 0, sizeof(struct sockaddr_in));
		addrlen = sizeof(struct sockaddr_in);
		if (getpeername(player->sockfd, &addr, &addrlen) == -1) {
			strlcpy(player->name, "(error)", GLS_NICK_LENGTH);
		}
		snprintf(player->name, GLS_NICK_LENGTH, "%s port %u",
			inet_ntoa(addr.sin_addr),
			(unsigned)ntohs(addr.sin_port));
		player->name[GLS_NICK_LENGTH - 1] = '\0';
	} else {
		// Player nickname.
		strlcpy(player->name, player->nick, GLS_NICK_LENGTH);
	}
	return player->name;
}

void* player_thread(void* v_player) {
	const int FD_PIPE = 0;
	const int FD_SOCKET = 1;
	struct flub* flub;
	struct gls_packet packet;
	struct player* player = (struct player*)v_player;
	struct pollfd pollfds[2];
	int ret;

	// Initialize thread-specific data.
	ret = g_serr_init();
	if (ret) {
		g_log_error("Unable to setup system error buffer");
		goto out;
	}
	ret = g_flub_init();
	if (ret) {
		g_log_error("Unable to initialize flub: '%s'", g_serr(ret));
		goto out;
	}
	flub = gls_init();
	if (flub) {
		flub = g_flub_toss("Unable to initialize gls buffer: '%s'",
			flub->message);
		goto out;
	}

	// Run the event loop.
	flub = NULL;
	while (1) {
		// Check for io events.
		memset(pollfds, 0, sizeof(pollfds));
		pollfds[FD_PIPE].fd = player->pipe_server_from[0];
		pollfds[FD_PIPE].events = POLLIN | POLLRDHUP;
		pollfds[FD_SOCKET].fd = player->sockfd;
		pollfds[FD_SOCKET].events = POLLIN | POLLRDHUP;
		if (poll(pollfds, 2, -1) == -1) {
			flub = g_flub_toss("Unable to poll fds: '%s'",
				g_serr(errno));
			break;
		}
		// Data from socket.
		if (pollfds[FD_SOCKET].revents & POLLIN) {
			// Check for disconnect.
			if (pollfds[FD_SOCKET].revents &
				(POLLHUP | POLLRDHUP)) {
				// Finish reading from socket.
				if (ioctl(player->sockfd, FIONREAD, &ret)
					== -1) {
					flub = g_flub_toss("Unable to check "
						"for bytes after hangup: '%s'",
						g_serr(errno));
					break;
				} if (!ret) {
					// End of data.
					break;
				}
			}

			// Move data from socket to server.
			flub = gls_packet_read(&packet, player->sockfd, 1);
			if (flub) {
				break;
			}
			flub = gls_packet_write(&packet,
				player->pipe_server_to[1]);
			if (flub) {
				break;
			}
		} else if (pollfds[FD_SOCKET].revents & (POLLERR | POLLNVAL)) {
			// Socket polling error.
			flub = g_flub_toss("Socket polling error: '%s'",
				g_serr(errno));
			break;
		}
		// Data from server.
		if (pollfds[FD_PIPE].revents & (POLLIN | POLLHUP)) {
			// Check for server kill.
			if (pollfds[FD_SOCKET].revents & POLLHUP) {
				// Finish reading from server pipe.
				if (ioctl(player->pipe_server_from[0], FIONREAD,
					&ret)
					== -1) {
					flub = g_flub_toss("Unable to check "
						"for bytes after kill: '%s'",
						g_serr(errno));
					break;
				} if (!ret) {
					// End of data.
					break;
				}
			}

			// Move data from server to socket.
			flub = gls_packet_read(&packet,
				player->pipe_server_from[0], 0);
			if (flub) {
				break;
			}
			flub = gls_packet_write(&packet, player->sockfd);
			if (flub) {
				break;
			}
		} else if (pollfds[FD_PIPE].revents & (POLLERR | POLLNVAL)) {
			// Pipe polling error.
			flub = g_flub_toss("Pipe polling error: '%s'",
				g_serr(errno));
			break;
		}
	}

	// Close to-server pipe write end.
out:
	if (close(player->pipe_server_to[1]) == -1) {
		g_log_warn("Unable to close write-end of to-server pipe");
	}

	// Return success/failure.
	if (flub) {
		// The thread-specific data for the flub may be cleaned when this
		// function exits, so move it somewhere more permanent.
		memcpy(&player->flub, flub, sizeof(struct flub));
		return &player->flub;
	}
	return NULL;
}
