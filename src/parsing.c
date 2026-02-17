#include "ft_traceroute.h"
#include <getopt.h>

int parse_args(int argc, char **argv, t_traceroute *tr)
{

    int opt = 0;
    int option_index = 0;

    static struct option long_options[] = {{"help", no_argument, 0, 'h'},
					   {"max-hops", required_argument, 0, 'm'},
					   {"no-dns", no_argument, 0, 'n'},
					   {"queries", required_argument, 0, 'q'},
					   {0, 0, 0, 0}};
    while ((opt = getopt_long(argc, argv, "hnm:q:", long_options, &option_index)) != 1) {
	switch (opt) {
	case 'h':
	    /* code */
	    break;

	default:
	    break;
	}
    }
}
