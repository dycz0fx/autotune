#include <mpi.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define MIN_MES 4
#define MIN_SEG 4096
#define MAX_MES 8388608
#define ITERS 10
#define SKIP 3

static void show_usage() {
    printf("--min: set the minimal message size\n");
    printf("--max: set the maximum message size\n");
    printf("--seg: set the minimal segment size\n");
    printf("--iter: set the iterations\n");
}

int main(int argc, char *argv[])
{
    int i = 0;
    int min_mes = MIN_MES;
    int min_seg = MIN_SEG;
    int max_mes = MAX_MES;
    int iters = ITERS;
    int skip = SKIP;
    if (argc %2 != 1) {
        printf("wrong arguments\n");
        return -1;
    }

    for (i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "--min") == 0) {
            min_mes = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "--max") == 0) {
            max_mes = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "--seg") == 0) {
            min_seg = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "--iters") == 0) {
            iters = atoi(argv[i+1]);
        }
        else {
            printf("wrong arguments\n");
            return -1;
        }
    }

    if (min_seg < min_mes || min_seg > max_mes) {
        printf("wrong segment size\n");
    }
    int rank, size;
    
    /* set up buf */
    void *buf;
    size_t alignment = sysconf(_SC_PAGESIZE);
    posix_memalign(&buf, alignment, max_mes);
    memset(buf, 1, max_mes);

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
    int m;
    double s, e, ib, ibsb, sb;
    /* the messages smaller than min_seg */
    for (m = min_mes; m <= min_seg; m *= 2) {
        ib = 0;
        sb = 0;
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

            /* sb */
            s = MPI_Wtime();
            MPI_Bcast(buf, m, MPI_CHAR, 0, MPI_COMM_LOW);
            e = MPI_Wtime();
            if (i >= SKIP) {
                sb += e - s;
            }
        }

        ib = (ib * 1e6) / iters;
        sb = (sb * 1e6) / iters;

        if (low_rank == 0) {
            printf("%s %s %d %d %f\n", "s", "ib", rank, m, ib);
            printf("%s %s %d %d %f\n", "s", "sb", rank, m, sb);
        }       

        if (up_rank != 0 && low_rank == 0) {
            MPI_Reduce(&ib, &ib, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_UP);
            MPI_Reduce(&sb, &sb, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_UP);
        } 
        if (up_rank == 0 && low_rank == 0) {
            MPI_Reduce(MPI_IN_PLACE, &ib, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_UP);
            MPI_Reduce(MPI_IN_PLACE, &sb, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_UP);
        }

        if (up_rank == 0 && low_rank == 0) {
            printf("%s %s %d %f\n", "s", "ib", m, ib);
            printf("%s %s %d %f\n", "s", "sb", m, sb);
        }
    }

    /* the messages bigger than min_seg */
    for (; m <= max_mes; m *= 2) {
        ib = 0;
        ibsb = 0;
        sb = 0;
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
            /* nibsb */
            int j = 0;
            int num_prev;
            if (m < 64 * 1024) {
                num_prev = 10;
            }
            else if (m < 1024 * 1024) {
                num_prev = 5;
            }
            else {
                num_prev = 2;
            }
            for(j=0; j<num_prev; j++){
                if (low_rank == 0) {
                    MPI_Ibcast(buf, m, MPI_CHAR, 0, MPI_COMM_UP, &request);
                }
                MPI_Bcast(buf, m, MPI_CHAR, 0, MPI_COMM_LOW);
                if (low_rank == 0) {
                    MPI_Wait(&request, &status);
                }
            }

            /* ibsb */
            s = MPI_Wtime();
            if (low_rank == 0) {
                MPI_Ibcast(buf, m, MPI_CHAR, 0, MPI_COMM_UP, &request);
            }
            MPI_Bcast(buf, m, MPI_CHAR, 0, MPI_COMM_LOW);
            if (low_rank == 0) {
                MPI_Wait(&request, &status);
            }
            e = MPI_Wtime();
            if (i >= SKIP) {
                ibsb += e - s;
            }

            /* sb */
            s = MPI_Wtime();
            MPI_Bcast(buf, m, MPI_CHAR, 0, MPI_COMM_LOW);
            e = MPI_Wtime();
            if (i >= SKIP) {
                sb += e - s;
            }
        }

        ib = (ib * 1e6) / iters;
        ibsb = (ibsb * 1e6) / iters;
        sb = (sb * 1e6) / iters;
        /*
        if (low_rank == 0) {
            printf("%s %s %d %d %f\n", "l", "ib", rank, m, ib);
            printf("%s %s %d %d %f\n", "l", "ibsb_nibsb_ib", rank, m, ibsb);
            printf("%s %s %d %d %f\n", "l", "sb", rank, m, sb);
        }       
        */
        if(up_rank != 0) {
            MPI_Reduce(&ib, &ib, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_UP);
            MPI_Reduce(&sb, &sb, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_UP);
        } else {
            MPI_Reduce(MPI_IN_PLACE, &ib, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_UP);
            MPI_Reduce(MPI_IN_PLACE, &sb, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_UP);
        }

        if (rank == 0) {
            printf("%s %s %d %f\n", "s", "ib", m, ib);
            printf("%s %s %d %f\n", "s", "sb", m, sb);
        }
    }

    free(buf);
    MPI_Finalize();

    return 0;
}