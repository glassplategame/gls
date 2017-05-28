/**
 *  Contains global variables.
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

#ifndef global_H
#define global_H

#include "include.h"

#include <errno.h>
#include <pthread.h>

#include "flub.h"
#include "log.h"

// Size for server to player-thread pipe; acts as effective send queue
// maximum.
#define G_PLAYER_PIPE_WRITE_SIZE 65536
// Size for system error (serr) buffer.
#define G_SERR_SIZE 256

// Global key for thread-specific flubs.
pthread_key_t g_flub_key;
// Global logger.
struct log g_log;
// Key for system error (serr) thread-specific data.
pthread_key_t g_serr_key;

#define G_LOG_DECLARATION(level)					\
void g_log_##level(char* format, ...);

#define G_LOG_DEFINITION(level)						\
void g_log_##level(char* format, ...) {					\
	va_list ap;							\
									\
	va_start(ap, format);						\
	log_v##level(&g_log, format, ap);				\
	va_end(ap);							\
}

G_LOG_DECLARATION(debug)
G_LOG_DECLARATION(info)
G_LOG_DECLARATION(warn)
G_LOG_DECLARATION(error)

/**
 * Destructor function for thread-specific flub.
 */
void g_flub_destructor(void* flub);

/**
 * Initialize thread-specific flub.
 *
 * Returns 0 on success, or an error number on error.
 */
int g_flub_init();

/**
 * Returns a pointer to a thread-specific flub instantiated with the
 * current call stack and the specified error message.  Must call
 * 'g_flub_init' before calling this function.
 */
struct flub* g_flub_toss(char* format, ...)
	__attribute__((format(printf, 1, 2)));

/**
 * Thread-safe version of 'strerror_r' with less cruft.
 */
char* g_serr(int err);

/**
 * Destructor for thread-specific serr data.
 */
void g_serr_destructor(void* buffer);

/**
 * Call before using 'g_serr'.
 *
 * Return 0 on success, -1 on failure.
 */
int g_serr_init();

#endif // global_H
