#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define MIN_MSG 4
#define MIN_SEG 4096
#define MAX_SEG 1048576
#define ITERS 10
#define SKIP 3

static void show_usage() {
    printf("--min_msg: set the minimal message size\n");
    printf("--min_seg: set the minimal segment size\n");
    printf("--max_seg: set the maximum segment size\n");
    printf("--iter: set the iterations\n");
}

int main(int argc, char *argv[])
{
    int i = 0;
    int min_msg = MIN_MSG;
    int min_seg = MIN_SEG;
    int max_seg = MAX_SEG;
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
        else if (strcmp(argv[i], "--min_seg") == 0) {
            min_seg = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "--max_seg") == 0) {
            max_seg = atoi(argv[i+1]);
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
    posix_memalign(&buf, alignment, max_seg);
    memset(buf, 1, max_seg);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* set up low_comm and up_comm */
    int low_rank, low_size, up_rank, up_size;
    MPI_Comm MPI_COMM_LOW;
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &MPI_COMM_LOW);
    MPI_Comm_rank(MPI_COMM_LOW, &low_rank);
    MPI_Comm_size(MPI_COMM_LOW, &low_size);
    MPI_Comm MPI_COMM_UP;
    MPI_Comm_split(MPI_COMM_WORLD, low_rank, rank, &MPI_COMM_UP);
    MPI_Comm_rank(MPI_COMM_UP, &up_rank);
    MPI_Comm_size(MPI_COMM_UP, &up_size);
    MPI_Status status;
    MPI_Request request = MPI_REQUEST_NULL;

    /* run benchmark */
    int m = min_msg < min_seg ? min_msg : min_seg;
    double s, e, ib;
    for (; m <= max_seg; m *= 2) {
        ib = 0;
        for (i = 0; i < iters + SKIP; i++) {
            MPI_Barrier(MPI_COMM_WORLD);

            /* ib */
            s = MPI_Wtime();
            if (low_rank == 0) {
                MPI_Ibcast(buf, m, MPI_CHAR, 0, MPI_COMM_UP, &request);
                MPI_Wait(&request, &status);
            }
            e = MPI_Wtime();
            if (i >= SKIP) {
                ib += e - s;
            }

        }

        ib = (ib * 1e6) / iters;

        if (low_rank == 0) {
            printf("%d %d %f\n", up_rank, m, ib);
        }
    }

    free(buf);
    MPI_Finalize();

    return 0;
}