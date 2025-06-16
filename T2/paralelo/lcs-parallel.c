#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

typedef unsigned short mtype;

/* Leitura de sequência */
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

/* Gera alfabeto dinâmico a partir de seqB */
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

/* Mapeia caractere para índice no alfabeto */
int char_to_index(char c, char *alphabet, int alphabet_size) {
    for (int i = 0; i < alphabet_size; i++) {
        if (alphabet[i] == c) return i;
    }
    return -1;
}

/* Constrói a tabela P (linearizada) */
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

    char *seqA = NULL, *seqB = NULL;
    int sizeA = 0, sizeB = 0;
    char *alphabet = NULL;
    int alphabet_size = 0;

    if (rank == 0) {
        seqA = read_seq(argv[1]);
        seqB = read_seq(argv[2]);
        sizeA = strlen(seqA);
        sizeB = strlen(seqB);
        alphabet_size = build_alphabet(seqB, sizeB, &alphabet);
    }

    /* Broadcast tamanhos */
    MPI_Bcast(&sizeA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&sizeB, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&alphabet_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        seqA = (char*) malloc(sizeA + 1);
        seqB = (char*) malloc(sizeB + 1);
        alphabet = (char*) malloc(alphabet_size);
    }

    /* Broadcast sequências e alfabeto */
    MPI_Bcast(seqA, sizeA + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(seqB, sizeB + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(alphabet, alphabet_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    /* Alocação da tabela P linearizada */
    int *P = (int*) calloc(alphabet_size * (sizeB + 1), sizeof(int));

    if (rank == 0)
        build_P_table(seqB, sizeB, P, alphabet_size, alphabet);

    /* Broadcast da tabela P */
    for (int c = 0; c < alphabet_size; c++)
        MPI_Bcast(&P[c * (sizeB + 1)], sizeB + 1, MPI_INT, 0, MPI_COMM_WORLD);

    /* Divisão de linhas entre os processos */
    int rows_per_proc = sizeA / nproc;
    int extra = sizeA % nproc;
    int my_start = rank * rows_per_proc + (rank < extra ? rank : extra) + 1;
    int my_rows = rows_per_proc + (rank < extra ? 1 : 0);

    mtype *prev_row = (mtype*) calloc(sizeB + 1, sizeof(mtype));
    mtype *curr_row = (mtype*) calloc(sizeB + 1, sizeof(mtype));

    if (my_start != 1)
        MPI_Recv(prev_row, sizeB + 1, MPI_UNSIGNED_SHORT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    /* Calcula minhas linhas */
    for (int i = my_start; i < my_start + my_rows; i++) {
        for (int j = 1; j <= sizeB; j++) {
            int c = char_to_index(seqA[i - 1], alphabet, alphabet_size);
            int pj = 0;
            if (c >= 0)
                pj = P[c * (sizeB + 1) + j];

            if (seqA[i - 1] == seqB[j - 1]) {
                curr_row[j] = prev_row[j - 1] + 1;
            } else if (pj == 0) {
                curr_row[j] = max(prev_row[j], 0);
            } else {
                curr_row[j] = max(prev_row[j], prev_row[pj - 1] + 1);
            }
        }
        memcpy(prev_row, curr_row, (sizeB + 1) * sizeof(mtype));
    }

    if (rank != nproc - 1)
        MPI_Send(prev_row, sizeB + 1, MPI_UNSIGNED_SHORT, rank + 1, 0, MPI_COMM_WORLD);

    if (rank == nproc - 1) {
        printf("length: %d\n", prev_row[sizeB]);
    }

    free(prev_row);
    free(curr_row);
    free(P);
    free(seqA);
    free(seqB);
    free(alphabet);

    MPI_Finalize();
    return 0;
}
