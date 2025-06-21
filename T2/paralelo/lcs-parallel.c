#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

typedef unsigned short mtype;

// Controle de debug
#define DEBUG_MODE 0


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
    long sizeA = 0, sizeB = 0;

    if (rank == 0) {
        seqA = read_seq(argv[1]);
        seqB = read_seq(argv[2]);
        sizeA = strlen(seqA);
        sizeB = strlen(seqB);
    }

    MPI_Bcast(&sizeA, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&sizeB, 1, MPI_LONG, 0, MPI_COMM_WORLD);

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

#if DEBUG_MODE
    mtype **local_matrix = (mtype**) malloc(my_rows * sizeof(mtype*));
    for (int i = 0; i < my_rows; i++) {
        local_matrix[i] = (mtype*) calloc(sizeB + 1, sizeof(mtype));
    }
#endif

    mtype *prev_row = (mtype*) calloc(sizeB + 1, sizeof(mtype));

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

#if DEBUG_MODE
        mtype *curr_row = local_matrix[i];
#else
        mtype *curr_row = (mtype*) calloc(sizeB + 1, sizeof(mtype));
#endif

        for (int j = 1; j <= sizeB; j++) {
            if (seqA[global_i - 1] == seqB[j - 1]) {
                curr_row[j] = prev_row[j - 1] + 1;
            } else {
                curr_row[j] = max(prev_row[j], curr_row[j - 1]);
            }
        }
        memcpy(prev_row, curr_row, (sizeB + 1) * sizeof(mtype));

#if !DEBUG_MODE
        free(curr_row);
#endif
    }
    double t2_calc = MPI_Wtime();
    calc_time += (t2_calc - t1_calc);

    if (rank != nproc - 1) {
        double t1_comm = MPI_Wtime();
        MPI_Send(prev_row, sizeB + 1, MPI_UNSIGNED_SHORT, rank + 1, 0, MPI_COMM_WORLD);
        double t2_comm = MPI_Wtime();
        comm_time += (t2_comm - t1_comm);
    }

#if DEBUG_MODE
    if (rank != 0) {
        for (int i = 0; i < my_rows; i++) {
            MPI_Send(local_matrix[i], sizeB + 1, MPI_UNSIGNED_SHORT, 0, 100 + i, MPI_COMM_WORLD);
        }
    }
#endif

    double total_end = MPI_Wtime();
    double total_time = total_end - total_start;

    if (rank == nproc - 1) {
        printf("score: %d\n", prev_row[sizeB]);
        printf("TotalTime: %.6f\n", total_time);
        printf("CommTime: %.6f\n", comm_time);
        printf("CalcTime: %.6f\n", calc_time);
    }

#if DEBUG_MODE
    if (rank == 0) {
        mtype **full_matrix = (mtype**) malloc((sizeA + 1) * sizeof(mtype*));
        for (int i = 0; i <= sizeA; i++) {
            full_matrix[i] = (mtype*) calloc(sizeB + 1, sizeof(mtype));
        }

        // Copiar linhas locais do rank 0
        for (int i = 0; i < my_rows; i++) {
            memcpy(full_matrix[my_start + i], local_matrix[i], (sizeB + 1) * sizeof(mtype));
        }

        // Receber linhas dos outros processos
        for (int source = 1; source < nproc; source++) {
            int source_rows = rows_per_proc + (source < extra ? 1 : 0);
            int source_start = source * rows_per_proc + (source < extra ? source : extra) + 1;

            for (int i = 0; i < source_rows; i++) {
                MPI_Recv(full_matrix[source_start + i], sizeB + 1, MPI_UNSIGNED_SHORT, source, 100 + i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        // Imprimir a matriz LCS completa
        printf("\nMatriz LCS completa:\n");
        for (int i = 1; i <= sizeA; i++) {
            for (int j = 1; j <= sizeB; j++) {
                printf("%3d ", full_matrix[i][j]);
            }
            printf("\n");
        }

        // Liberar matriz completa
        for (int i = 0; i <= sizeA; i++) free(full_matrix[i]);
        free(full_matrix);
    }

    // Liberar local_matrix de cada processo
    for (int i = 0; i < my_rows; i++) free(local_matrix[i]);
    free(local_matrix);
#endif

    free(prev_row);
    free(seqA);
    free(seqB);

    MPI_Finalize();
    return 0;
}
