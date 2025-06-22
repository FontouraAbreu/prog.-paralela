#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

typedef unsigned short mtype;

// Direções de backtracking
#define DIR_NONE 0
#define DIR_DIAG 1
#define DIR_UP   2
#define DIR_LEFT 3

// #define DEBUG

char* read_seq(const char *fname) {
    FILE *fp = fopen(fname, "rt");
    if (!fp) { fprintf(stderr, "Erro ao abrir %s\n", fname); MPI_Abort(MPI_COMM_WORLD, 1); }
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);
    char *seq = (char*) calloc(fsize + 1, sizeof(char));
    long len = 0;
    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (c != '\n' && c != '\r') seq[len++] = c;
    }
    seq[len] = '\0';
    fclose(fp);
    return seq;
}

int main(int argc, char **argv) {
    int rank, nproc;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if (argc != 3) {
        if (rank == 0) fprintf(stderr, "Usage: %s <seqA.txt> <seqB.txt>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    double total_start = MPI_Wtime();
    double comm_time = 0.0;
    double calc_time = 0.0;

    char *seqA = NULL, *seqB = NULL;
    int sizeA = 0, sizeB = 0;

    if (rank == 0) {
        seqA = read_seq(argv[1]);
        seqB = read_seq(argv[2]);
        sizeA = strlen(seqA);
        sizeB = strlen(seqB);
    }

    MPI_Bcast(&sizeA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&sizeB, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        seqA = (char*) malloc(sizeA + 1);
        seqB = (char*) malloc(sizeB + 1);
    }
    MPI_Bcast(seqA, sizeA + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(seqB, sizeB + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    int rows_per_proc = sizeA / nproc;
    int extra = sizeA % nproc;
    int my_start = rank * rows_per_proc + (rank < extra ? rank : extra) + 1;
    int my_rows = rows_per_proc + (rank < extra ? 1 : 0);

    mtype *prev_row = (mtype*) calloc(sizeB + 1, sizeof(mtype));
    mtype *curr_row = (mtype*) calloc(sizeB + 1, sizeof(mtype));

    unsigned char **directions = (unsigned char**) malloc(my_rows * sizeof(unsigned char*));
    for (int i = 0; i < my_rows; i++)
        directions[i] = (unsigned char*) malloc((sizeB + 1) * sizeof(unsigned char));

    MPI_Request req_send, req_recv;
    MPI_Status status;

    if (my_start != 1) {
        double t1_comm = MPI_Wtime();
        MPI_Recv(prev_row, sizeB + 1, MPI_UNSIGNED_SHORT, rank - 1, 0, MPI_COMM_WORLD, &status);
        double t2_comm = MPI_Wtime();
        comm_time += (t2_comm - t1_comm);
    }

    double t1_calc = MPI_Wtime();
    for (int i = 0; i < my_rows; i++) {
        int global_i = my_start + i;
        for (int j = 1; j <= sizeB; j++) {
            if (seqA[global_i - 1] == seqB[j - 1]) {
                curr_row[j] = prev_row[j - 1] + 1;
                directions[i][j] = DIR_DIAG;
            } else if (prev_row[j] >= curr_row[j - 1]) {
                curr_row[j] = prev_row[j];
                directions[i][j] = DIR_UP;
            } else {
                curr_row[j] = curr_row[j - 1];
                directions[i][j] = DIR_LEFT;
            }
        }
        memcpy(prev_row, curr_row, (sizeB + 1) * sizeof(mtype));
    }
    double t2_calc = MPI_Wtime();
    calc_time += (t2_calc - t1_calc);

    if (rank != nproc - 1) {
        double t1_comm = MPI_Wtime();
        MPI_Send(prev_row, sizeB + 1, MPI_UNSIGNED_SHORT, rank + 1, 0, MPI_COMM_WORLD);
        double t2_comm = MPI_Wtime();
        comm_time += (t2_comm - t1_comm);
    }

    // Reunir a matriz de direções no último processo
    unsigned char *full_directions = NULL;
    int *recvcounts = NULL, *displs = NULL;
    if (rank == nproc - 1) {
        full_directions = (unsigned char*) malloc(sizeA * (sizeB + 1) * sizeof(unsigned char));
        recvcounts = (int*) malloc(nproc * sizeof(int));
        displs = (int*) malloc(nproc * sizeof(int));

        for (int p = 0; p < nproc; p++) {
            int rows = (sizeA / nproc) + (p < extra ? 1 : 0);
            recvcounts[p] = rows * (sizeB + 1);
            displs[p] = (p == 0) ? 0 : displs[p - 1] + recvcounts[p - 1];
        }
    }

    // Preparar buffer local para envio
    unsigned char *local_dirs_flat = (unsigned char*) malloc(my_rows * (sizeB + 1) * sizeof(unsigned char));
    for (int i = 0; i < my_rows; i++)
        memcpy(&local_dirs_flat[i * (sizeB + 1)], directions[i], (sizeB + 1) * sizeof(unsigned char));

    MPI_Gatherv(
        local_dirs_flat, my_rows * (sizeB + 1), MPI_UNSIGNED_CHAR,
        full_directions, recvcounts, displs, MPI_UNSIGNED_CHAR,
        nproc - 1, MPI_COMM_WORLD
    );

    double total_end = MPI_Wtime();
    double total_time = total_end - total_start;

    if (rank == nproc - 1) {
        printf("score: %d\n", prev_row[sizeB]);
        printf("TotalTime: %.6f\n", total_time);
        printf("CommTime: %.6f\n", comm_time);
        printf("CalcTime: %.6f\n", calc_time);


        #ifdef DEBUG
            // Reconstruir a LCS
            int i = sizeA;
            int j = sizeB;
            char *lcs = (char*) malloc((prev_row[sizeB] + 1) * sizeof(char));
            int lcs_index = prev_row[sizeB];

            lcs[lcs_index] = '\0';
            while (i > 0 && j > 0) {
                unsigned char dir = full_directions[(i - 1) * (sizeB + 1) + j];
                if (dir == DIR_DIAG) {
                    lcs[--lcs_index] = seqA[i - 1];
                    i--;
                    j--;
                } else if (dir == DIR_UP) {
                    i--;
                } else if (dir == DIR_LEFT) {
                    j--;
                } else {
                    break;
                }
            }
            printf("LCS: %s\n", lcs);
            free(lcs);
        #endif
        
        free(full_directions);
        free(recvcounts);
        free(displs);
    }

    free(local_dirs_flat);
    for (int i = 0; i < my_rows; i++)
        free(directions[i]);
    free(directions);
    free(prev_row);
    free(curr_row);
    free(seqA);
    free(seqB);

    MPI_Finalize();
    return 0;
}