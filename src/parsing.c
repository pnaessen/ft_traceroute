#include "ft_traceroute.h"
#include <getopt.h>

static void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s [options] <destination>\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h, --help        Display this help and exit\n");
    fprintf(stderr, "  -m, --max-hops N  Set the max number of hops (default: %d, max: 255)\n",
	    DEF_MAX_HOPS);
    fprintf(stderr, "  -n, --no-dns      Do not resolve IP addresses to their domain names\n");
    fprintf(stderr, "  -q, --queries N   Set the number of probes per hop (default: %d, max: 10)\n",
	    DEF_PROBES_PER_HOP);
    fprintf(stderr, "  -I, --icmp        Use ICMP ECHO for probes\n");
}

static int get_positive_int(const char *str, int min, int max)
{
    char *endptr;
    long val;

    errno = 0;
    val = strtol(str, &endptr, 10);

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0) ||
	endptr == str || *endptr != '\0') {
	return -1;
    }

    if (val < min || val > max) {
	return -1;
    }

    return (int)val;
}

int parse_args(int argc, char **argv, t_traceroute *tr)
{
    int opt;
    int option_index = 0;
    int val;

    static struct option long_options[] = {
	{"help", no_argument, 0, 'h'},		 {"icmp", no_argument, 0, 'I'},
	{"max-hops", required_argument, 0, 'm'}, {"no-dns", no_argument, 0, 'n'},
	{"queries", required_argument, 0, 'q'},	 {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "Ihm:nq:", long_options, &option_index)) != -1) {
	switch (opt) {
	case 'h':
	    print_usage(argv[0]);
	    exit(EXIT_SUCCESS);

	case 'n':
	    tr->resolve_dns = false;
	    break;

	case 'm':
	    val = get_positive_int(optarg, 1, 255);
	    if (val == -1) {
		fprintf(stderr, "Error: invalid max-hops (must be between 1 and 255)\n");
		return EXIT_FAILURE;
	    }
	    tr->max_hops = val;
	    break;

	case 'I':
	    tr->use_icmp = true;
	    break;
	case 'q':
	    val = get_positive_int(optarg, 1, 10);
	    if (val == -1) {
		fprintf(stderr, "Error: invalid queries (must be between 1 and 10)\n");
		return EXIT_FAILURE;
	    }
	    tr->probes_per_hop = val;
	    break;

	case '?':
	    print_usage(argv[0]);
	    return EXIT_FAILURE;

	default:
	    break;
	}
    }

    if (optind < argc) {
	tr->target = argv[optind];
	if (optind + 1 < argc) {
	    fprintf(stderr, "Error: too many arguments.\n");
	    print_usage(argv[0]);
	    return EXIT_FAILURE;
	}
    } else {
	fprintf(stderr, "Error: missing destination.\n");
	print_usage(argv[0]);
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
