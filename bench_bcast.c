#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define MIN_MSG 4
#define MAX_MSG 16777216
#define MIN_SEG 4096
#define MAX_SEG 1048576
#define ITERS 10
#define SKIP 3

static void show_usage() {
    printf("--min_msg: set the minimal message size\n");
    printf("--max_msg: set the maximum message size\n");
    printf("--iter: set the iterations\n");
}

int main(int argc, char *argv[])
{
    int i = 0;
    int min_msg = MIN_MSG;
    int max_msg = MAX_MSG;
    int iters = ITERS;
    int skip = SKIP;
    if (argc %2 != 1) {
        printf("wrong arguments\n");
        return -1;
    }

    for (i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "--min_msg") == 0) {
            min_msg = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "--max_msg") == 0) {
            max_msg = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "--iters") == 0) {
            iters = atoi(argv[i+1]);
        }
        else {
            printf("wrong arguments\n");
            return -1;
        }
    }

    int rank, size;
    
    /* set up buf */
    void *buf;
    size_t alignment = sysconf(_SC_PAGESIZE);
    posix_memalign(&buf, alignment, max_msg);
    memset(buf, 1, max_msg);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Status status;
    MPI_Request request = MPI_REQUEST_NULL;

    /* run benchmark */
    int m;
    double s, e, bcast;

    for (m = min_msg; m <= max_msg; m *= 2) {
        bcast = 0;
        for (i = 0; i < iters + SKIP; i++) {
            MPI_Barrier(MPI_COMM_WORLD);

            /* bcast */
            s = MPI_Wtime();
            MPI_Bcast(buf, m, MPI_CHAR, 0, MPI_COMM_WORLD);
            e = MPI_Wtime();
            if (i >= SKIP) {
                bcast += e - s;
            }
        }

        bcast = (bcast * 1e6) / iters;

        if (rank != 0) {
            MPI_Reduce(&bcast, &bcast, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        } else {
            MPI_Reduce(MPI_IN_PLACE, &bcast, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        }

        if (rank == 0) {
            printf("%d %f\n", m, bcast);
        }

    }


    free(buf);
    MPI_Finalize();

    return 0;
}