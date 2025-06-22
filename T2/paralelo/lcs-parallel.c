#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

typedef unsigned short mtype;

char* read_seq(const char *fname, long *size) {
    FILE *fp = fopen(fname, "rt");
    if (!fp) { fprintf(stderr, "Error reading file %s\n", fname); MPI_Abort(MPI_COMM_WORLD, 1); }
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);
    char *seq = (char*) malloc((fsize + 1) * sizeof(char));
    long len = 0;
    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (c != '\n' && c != '\r') seq[len++] = c;
    }
    seq[len] = '\0';
    *size = len;
    fclose(fp);
    return seq;
}

int main(int argc, char **argv) {
    int rank, nprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (argc != 3) {
        if (rank == 0) printf("Usage: %s <seqA.txt> <seqB.txt>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    char *seqA = NULL, *seqB = NULL;
    long sizeA = 0, sizeB = 0;

    if (rank == 0) {
        seqA = read_seq(argv[1], &sizeA);
        seqB = read_seq(argv[2], &sizeB);
    }

    MPI_Bcast(&sizeA, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&sizeB, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        seqA = (char*) malloc(sizeA + 1);
        seqB = (char*) malloc(sizeB + 1);
    }
    MPI_Bcast(seqA, sizeA + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(seqB, sizeB + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    mtype *prev_row = (mtype*) calloc(sizeB + 1, sizeof(mtype));
    mtype *curr_row = (mtype*) calloc(sizeB + 1, sizeof(mtype));
    mtype *prefix_max = (mtype*) calloc(sizeB + 1, sizeof(mtype));

    double total_start = MPI_Wtime();
    double comm_time = 0.0, calc_time = 0.0;
    MPI_Status status;

    for (long i = 1; i <= sizeA; i++) {
        if (i % nprocs != rank) continue;

        double t1_calc = MPI_Wtime();

        prefix_max[0] = prev_row[0];
        for (long j = 1; j <= sizeB; j++) {
            if (seqA[i - 1] == seqB[j - 1])
                curr_row[j] = prev_row[j - 1] + 1;
            else
                curr_row[j] = max(prev_row[j], prefix_max[j - 1]);

            prefix_max[j] = max(prefix_max[j - 1], curr_row[j]);
        }

        double t2_calc = MPI_Wtime();
        calc_time += (t2_calc - t1_calc);

        double t1_comm = MPI_Wtime();
        if (rank != nprocs - 1)
            MPI_Send(curr_row, sizeB + 1, MPI_UNSIGNED_SHORT, (rank + 1) % nprocs, 0, MPI_COMM_WORLD);

        if (rank != 0)
            MPI_Recv(prev_row, sizeB + 1, MPI_UNSIGNED_SHORT, (rank - 1 + nprocs) % nprocs, 0, MPI_COMM_WORLD, &status);
        double t2_comm = MPI_Wtime();
        comm_time += (t2_comm - t1_comm);

        // Troca as linhas
        mtype *tmp = prev_row;
        prev_row = curr_row;
        curr_row = tmp;
    }

    double total_end = MPI_Wtime();

    if (rank == (sizeA % nprocs)) {
        printf("Score: %d\n", prev_row[sizeB]);
        printf("TotalTime: %.6f\n", total_end - total_start);
        printf("CalcTime: %.6f\n", calc_time);
        printf("CommTime: %.6f\n", comm_time);
    }

    free(prev_row);
    free(curr_row);
    free(prefix_max);
    free(seqA);
    free(seqB);

    MPI_Finalize();
    return 0;
}
