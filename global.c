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
	static int key_created = 0;
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	int ret;
	int ret2;

	// Initialize key.
	ret = pthread_mutex_lock(&mutex);
	if (ret) {
		g_log_error("Unable to lock mutex: '%s'", g_serr(ret));
		return -1;
	}
	if (!key_created) {
		ret = pthread_key_create(&g_flub_key, g_flub_destructor);
		if (ret) {
			g_log_error("Unable to create key: '%s'", g_serr(ret));
			goto unlock;
		}
		key_created = 1;
	}
unlock:
	ret2 = pthread_mutex_unlock(&mutex);
	if (ret2) {
		g_log_error("Mutex unlock failed: '%s'", g_serr(ret2));
	}
	if (ret || ret2) {
		return -1;
	}

	// Initialize buffer.
	flub = (struct flub*)malloc(sizeof(struct flub));
	if (!flub) {
		g_log_error("Unable to allocate flub buffer");
		return -1;
	}

	// Set buffer.
	ret = pthread_setspecific(g_flub_key, flub);
	if (ret) {
		g_log_error("Unable to set pthread buffer: '%s'", g_serr(ret));
		return -1;
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
	// TODO: Use strerror_r for errors.
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
