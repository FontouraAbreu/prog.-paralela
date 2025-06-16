#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

typedef unsigned short mtype;


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

int build_alphabet(char *seqB, int sizeB, char **alphabet_out) {
    int ascii[256] = {0};
    int count = 0;
    for (int i = 0; i < sizeB; i++) {
        unsigned char c = seqB[i];
        if (!ascii[c]) {
            ascii[c] = 1;
            count++;
        }
    }
    *alphabet_out = (char*) malloc(count);
    int pos = 0;
    for (int c = 0; c < 256; c++) {
        if (ascii[c]) (*alphabet_out)[pos++] = (char)c;
    }
    return count;
}

int char_to_index(char c, char *alphabet, int alphabet_size) {
    for (int i = 0; i < alphabet_size; i++) {
        if (alphabet[i] == c) return i;
    }
    return -1;
}

void build_P_table(char *B, int sizeB, int *P, int alphabet_size, char *alphabet) {
    for (int c = 0; c < alphabet_size; c++) {
        P[c * (sizeB + 1) + 0] = 0;
        for (int j = 1; j <= sizeB; j++) {
            if (B[j - 1] == alphabet[c])
                P[c * (sizeB + 1) + j] = j;
            else
                P[c * (sizeB + 1) + j] = P[c * (sizeB + 1) + (j - 1)];
        }
    }
}


void calculate_row_autovec(
    mtype * restrict curr_row,
    const mtype * restrict prev_row,
    const char * restrict seqB,
    int sizeB,
    char char_A,
    const int * restrict P_row)
{
    // by using restrict, the compiler will try to optimize memory access
    for (int j = 1; j <= sizeB; j++) {
        //  match
        const int is_match = (seqB[j - 1] == char_A);
        const mtype val_match = prev_row[j - 1] + 1;

        // mismatch
        const int pj = P_row[j];
        const mtype val_gathered = (pj > 0) ? prev_row[pj - 1] + 1 : 1;
        const mtype val_mismatch = max(prev_row[j], val_gathered);

        // calculate the current cell value using algebraic expression in order to enable vectorization
        curr_row[j] = (is_match * val_match) + ((1 - is_match) * val_mismatch);
    }
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

    // time variables
    double total_start = MPI_Wtime();
    double comm_time = 0.0;
    double calc_time = 0.0;

    // sequence and P table variables
    char *seqA = NULL, *seqB = NULL;
    int sizeA = 0, sizeB = 0;
    char *alphabet = NULL;
    int alphabet_size = 0;

    double t1 = MPI_Wtime();

    // Rank 0 reads the sequences and builds the alphabet
    if (rank == 0) {
        seqA = read_seq(argv[1]);
        seqB = read_seq(argv[2]);
        sizeA = strlen(seqA);
        sizeB = strlen(seqB);
        alphabet_size = build_alphabet(seqB, sizeB, &alphabet);
    }    
    // Broadcast the sizes and alphabet to all ranks
    MPI_Bcast(&sizeA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&sizeB, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&alphabet_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Allocate memory for sequences and alphabet on all ranks
    if (rank != 0) {
        seqA = (char*) malloc(sizeA + 1);
        seqB = (char*) malloc(sizeB + 1);
        alphabet = (char*) malloc(alphabet_size);
    }
    // Broadcast the sequences and alphabet
    MPI_Bcast(seqA, sizeA + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(seqB, sizeB + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(alphabet, alphabet_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Build the P table on rank 0 and broadcast it to all ranks
    int *P = (int*) calloc(alphabet_size * (sizeB + 1), sizeof(int));
    if (rank == 0)
        build_P_table(seqB, sizeB, P, alphabet_size, alphabet);

    // Broadcast the P table to all ranks*
    for (int c = 0; c < alphabet_size; c++)
        MPI_Bcast(&P[c * (sizeB + 1)], sizeB + 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Timing the broadcast
    double t2 = MPI_Wtime();
    comm_time += (t2 - t1);

    // Calculate the number of rows each process will handle
    int rows_per_proc = sizeA / nproc;
    int extra = sizeA % nproc;
    int my_start = rank * rows_per_proc + (rank < extra ? rank : extra) + 1;
    int my_rows = rows_per_proc + (rank < extra ? 1 : 0);

    // Allocate memory for the current and previous rows
    mtype *buffer[2];
    buffer[0] = (mtype*) calloc(sizeB + 1, sizeof(mtype));
    buffer[1] = (mtype*) calloc(sizeB + 1, sizeof(mtype));
    
    
    mtype *curr_row, *prev_row;
    int buf_idx = 0;

    MPI_Request req_recv, req_send;
    MPI_Status status;

    // Initialize the first row (base case)
    if (my_start != 1) {
        double t1_comm = MPI_Wtime();

        // Receive the first row from the previous rank
        MPI_Irecv(buffer[buf_idx], sizeB + 1, MPI_UNSIGNED_SHORT, rank - 1, 0, MPI_COMM_WORLD, &req_recv);
        MPI_Wait(&req_recv, &status);

        double t2_comm = MPI_Wtime();
        comm_time += (t2_comm - t1_comm);
    }


    // each rank processes its assigned rows
    for (int i = my_start; i < my_start + my_rows; i++) {
        prev_row = buffer[buf_idx];
        curr_row = buffer[1 - buf_idx];

        double t1_calc = MPI_Wtime();

        char char_A = seqA[i-1];
        int c_idx = char_to_index(char_A, alphabet, alphabet_size);

        // If the character is in the alphabet, calculate the current row
        if (c_idx >= 0) {
            const int* P_row = &P[c_idx * (sizeB + 1)];
            calculate_row_autovec(curr_row, prev_row, seqB, sizeB, char_A, P_row);
        } else { // If the character is not in the alphabet, copy the previous row
             for (int j = 1; j <= sizeB; j++) {
                curr_row[j] = prev_row[j];
             }
        }

        double t2_calc = MPI_Wtime();
        calc_time += (t2_calc - t1_calc);

        if (i == my_start + my_rows - 1 && rank != nproc - 1) {
            double t1_send = MPI_Wtime();
            MPI_Isend(curr_row, sizeB + 1, MPI_UNSIGNED_SHORT, rank + 1, 0, MPI_COMM_WORLD, &req_send);
            MPI_Wait(&req_send, &status);
            double t2_send = MPI_Wtime();
            comm_time += (t2_send - t1_send);
        }

        buf_idx = 1 - buf_idx;
    }

    double total_end = MPI_Wtime();
    double total_time = total_end - total_start;

    // If this is the last rank, print the result
    if (rank == nproc - 1) {
        printf("score: %d\n", buffer[1 - buf_idx][sizeB]);
        printf("TotalTime: %.6f\n", total_time);
        printf("CommTime: %.6f\n", comm_time);
        printf("CalcTime: %.6f\n", calc_time);
    }

    free(buffer[0]);
    free(buffer[1]);
    free(P);
    free(seqA);
    free(seqB);
    free(alphabet);

    MPI_Finalize();
    return 0;
}
