/**
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

#include "global.h"

G_LOG_DEFINITION(debug)
G_LOG_DEFINITION(info)
G_LOG_DEFINITION(warn)
G_LOG_DEFINITION(error)

void g_flub_destructor(void* flub) {
	free(flub);
}

int g_flub_init() {
	struct flub* flub;
	int ret;

	// Create thread-specific flub.
	flub = (struct flub*)pthread_getspecific(g_flub_key);
	if (flub) {
		// Already exists.
		return 0;
	}
	flub = (struct flub*)malloc(sizeof(struct flub));
	if (!flub) {
		return errno;
	}
	ret = pthread_setspecific(g_flub_key, (const void*)flub);
	if (ret) {
		return ret;
	}
	return 0;
}

struct flub* g_flub_toss(char* format, ...) {
	va_list ap;
	struct flub* flub;

	// Get the flub.
	flub = pthread_getspecific(g_flub_key);
	if (!flub) {
		// Thread-specific flub not found.  This should never happen
		// so long as 'g_flub_init' is called first.
		g_log_error("Getting thread-specific flub *FAILED*, panicing!");
		exit(EXIT_FAILURE);
	}

	// Initialize the flub.
	va_start(ap, format);
	flub_tossv(flub, format, ap);
	va_end(ap);

	// Return the flub.
	return (flub);
}

char* g_serr(int err) {
	char* buf;

	// Get buffer.
	buf = pthread_getspecific(g_serr_key);
	if (!buf) {
		return "THREAD-SPECIFIC BUFFER FAILURE";
	}

	// Return string.
	return strerror_r(errno, buf, G_SERR_SIZE);
}

void g_serr_destructor(void* buffer) {
	free(buffer);
}

int g_serr_init() {
	char* buffer;
	static int key_created = 0;
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	int ret;
	int ugh;

	// Initialize key.
	ugh = 0;
	ret = pthread_mutex_lock(&mutex);
	if (ret) {
		g_log_error("Unable to lock mutex: '%s'",
			strerror(ret)); // Unsafe.
		return -1;
	}
	if (!key_created) {
		ugh = pthread_key_create(&g_serr_key, g_serr_destructor);
		if (ugh) {
			g_log_error("Unable to create key: '%s'",
				strerror(ret)); // Unsafe.
			goto unlock;
		}
		key_created = 1;
	}
unlock:
	ret = pthread_mutex_unlock(&mutex);
	if (ret) {
		g_log_error("Mutex unlock failed: '%s'",
			strerror(ret)); // Unsafe.
	}
	if (ugh || ret) {
		return -1;
	}

	// Initialize buffer.
	buffer = (char*)malloc(G_SERR_SIZE);
	if (!buffer) {
		g_log_error("Unable to allocate buffer");
		return -1;
	}

	// Set buffer.
	ret = pthread_setspecific(g_serr_key, buffer);
	if (ret) {
		g_log_error("Unable to set pthread buffer: '%s'",
			strerror(ret)); // Unsafe.
		return -1;
	}
	return 0;
}
