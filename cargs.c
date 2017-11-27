#include "cargs.h"

static char CARGS_NICK_DEFAULT[] = "knetch";

void cargs_help(struct cargs* args, struct flub* flub) {
	FILE* out;

	// Print error.
	if (flub) {
		out = stderr;
		fprintf(out, "ERROR: '%s'\n\n", flub->message);
	} else {
		out = stdout;
	}

	// Print usage.
	fprintf(out, "gls [ARGS]\n\nARGS:\n");
	fprintf(out, "\t-h --help  Print this usage message\n");
	fprintf(out, "\t-n --nick  Connect using specified nickname (default: "
		"'%s', cur: '%s')\n", CARGS_NICK_DEFAULT, args->nick);

	// Exit program.
	if (flub) {
		exit(EXIT_FAILURE);
	} else {
		exit(EXIT_SUCCESS);
	}
}

struct flub* cargs_parse(struct cargs* args, int argc, char* argv[]) {
	struct flub* flub;
	struct option longopts[] = {
		{"help", 0, NULL, 'h'},
		{"nick", 1, NULL, 'n'},
		{0, 0, 0, 0}
	};
	int ret;

	// Set defaults.
	memset(args, 0, sizeof(struct cargs));
	strcpy(args->nick, CARGS_NICK_DEFAULT);

	// Parse arguments.
	while((ret = getopt_long(argc, argv, ":n:", longopts, NULL)) != -1) {
		switch(ret) {
		case 'h':
			cargs_help(args, NULL);
		case 'n':
			strlcpy(args->nick, optarg, GLS_NICK_LENGTH);
			if ((flub = gls_nick_validate(optarg, 0))) {
				cargs_help(args, flub);
			}
			break;
		case ':':
			flub = g_flub_toss("Missing argument after '%c'",
				optopt);
			cargs_help(args, flub);
		case '?':
			// Unknown option.
			if (optopt) {
				flub = g_flub_toss("Unknown argument '%c'",
					optopt);
			} else {
				flub = g_flub_toss("Unknown longopt at index "
					"'%i'", optind);
			}
			cargs_help(args, flub);
		default:
			// Error!
			flub = g_flub_toss("Unable to parse arguments");
			cargs_help(args, flub);
		}
	}
	return NULL;
}
