#include "flub.h"

struct flub* flub_append(struct flub* flub, char* format, ...) {
	va_list ap;
	static char buffer[256];

	// Format error message.
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);

	// Append error message.
	strlcat(flub->message, ", ", sizeof(flub->message));
	strlcat(flub->message, buffer, sizeof(flub->message));

	// Return specified flub.
	return flub;
}

void flub_grab(struct flub* flub) {
	// Free backtrace.
	if (flub->backtrace) {
		free(flub->backtrace);
	}
	memset(flub, 0, sizeof(struct flub));
}

void flub_printfsl(struct flub* flub, char* dest, size_t size) {
	static char bt[1024]; // hacky and tacky

	// Print error message.
	memset(dest, 0, size);
	strlcpy(dest, flub->message, size);

	// Print backtrace.
	strlcat(dest, "\n", size);
	flub_snbacktrace(flub, bt, sizeof(bt));
	strlcat(dest, bt, size);
}

void flub_snbacktrace(struct flub* flub, char* dest, size_t size) {
	int i;

	// Place the backtrace in dest.
	memset(dest, 0, size);
	for (i = 0; i < flub->backtrace_count; i++) {
		strlcat(dest, flub->backtrace[i], size);
		strlcat(dest, "\n", size);
	}
}

void flub_toss(struct flub* flub, char* format, ...) {
	va_list ap;
	va_start(ap, format);
	flub_tossv(flub, format, ap);
	va_end(ap);
	return;
}
void flub_tossv(struct flub* flub, char* format, va_list ap) {
	void* buffer[32];

	// Get backtrace.
	memset(flub, 0, sizeof(struct flub));
	flub->backtrace_count = backtrace((void**)&buffer, 32);

	// Convert backtrace to strings.
	flub->backtrace = backtrace_symbols((void**)&buffer,
		flub->backtrace_count);
	if (!flub->backtrace) {
		flub->backtrace_count = 0;
	}

	// Create error message.
	vsnprintf(flub->message, sizeof(flub->message), format, ap);
	return;
}
