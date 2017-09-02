#include "cargs.h"

struct flub* cargs_parse(struct cargs* args, int argc, char* argv[]) {
	struct flub* flub;
	struct option longopts[] = {
		{"nick", 1, NULL, 'n'},
		{0, 0, 0, 0}
	};
	int ret;

	// Set defaults.
	memset(args, 0, sizeof(struct cargs));
	strcpy(args->nick, "knetch");

	// Parse arguments.
	while((ret = getopt_long(argc, argv, ":n:", longopts, NULL)) != -1) {
		switch(ret) {
		case 'n':
			strlcpy(args->nick, optarg, GLS_NAME_LENGTH);
			if ((flub = gls_nick_validate(optarg, 0))) {
				return flub_append(flub, "parsing nick "
					"argument");
			}
			break;
		case ':':
			return g_flub_toss("Missing argument after '%c'",
				optopt);
		case '?':
			// Unknown option.
			if (optopt) {
				return g_flub_toss("Unknown argument '%c'",
					optopt);
			} else {
				return g_flub_toss("Unknown longopt at index "
					"'%i'", optind);
			}
		default:
			// Error!
			return g_flub_toss("Unable to parse arguments");
		}
	}
	return NULL;
}
