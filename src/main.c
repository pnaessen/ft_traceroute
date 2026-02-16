#include "ft_traceroute.h"

int	main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <args>\n", argv[0]);
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}
