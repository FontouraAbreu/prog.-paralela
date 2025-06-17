#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>

#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

typedef unsigned short mtype;

char* read_seq(char *fname) {
    FILE *fseq = fopen(fname, "rt");
    if (fseq == NULL ) {
        printf("Error reading file %s\n", fname);
        exit(1);
    }

    fseek(fseq, 0L, SEEK_END);
    long size = ftell(fseq);
    rewind(fseq);

    char *seq = (char *) calloc(size + 1, sizeof(char));
    if (seq == NULL ) {
        printf("Error allocating memory for sequence %s.\n", fname);
        exit(1);
    }

    long i = 0;
    while (!feof(fseq)) {
        seq[i] = fgetc(fseq);
        if (seq[i] != '\n' && seq[i] != EOF)
            i++;
    }
    seq[i] = '\0';
    fclose(fseq);
    return seq;
}

mtype* allocateScoreMatrix(long sizeA, long sizeB) {
    mtype *scoreMatrix = (mtype*) calloc((sizeA + 1) * (sizeB + 1), sizeof(mtype));
    if (scoreMatrix == NULL) {
        printf("Error allocating score matrix\n");
        printf("Memory needed: %ld bytes\n", (long)((sizeA + 1) * (sizeB + 1) * sizeof(mtype)));
        exit(1);
    }
    return scoreMatrix;
}

void initScoreMatrix(mtype *scoreMatrix, long sizeA, long sizeB) {
    #pragma omp parallel for
    for (long j = 0; j <= sizeA; j++)
        scoreMatrix[j] = 0;

    #pragma omp parallel for
    for (long i = 0; i <= sizeB; i++)
        scoreMatrix[i * (sizeA + 1)] = 0;
}

mtype pLCS_antidiagonal(mtype *scoreMatrix, long sizeA, long sizeB, char *seqA, char *seqB) {
    long i, j, d;

    for (d = 2; d <= sizeA + sizeB; d++) {
        long row_start = d - sizeA;
        if (row_start < 1) row_start = 1;

        long row_end = d - 1;
        if (row_end > sizeB) row_end = sizeB;

        #pragma omp parallel for private(i, j)
        for (i = row_start; i <= row_end; i++) {
            j = d - i;
            if (j > sizeA) continue;

            long idx = i * (sizeA + 1) + j;
            long idx_left = i * (sizeA + 1) + (j - 1);
            long idx_up = (i - 1) * (sizeA + 1) + j;
            long idx_diag = (i - 1) * (sizeA + 1) + (j - 1);

            if (seqA[j - 1] == seqB[i - 1]) {
                scoreMatrix[idx] = scoreMatrix[idx_diag] + 1;
            } else {
                scoreMatrix[idx] = max(scoreMatrix[idx_up], scoreMatrix[idx_left]);
            }
        }
    }

    return scoreMatrix[sizeB * (sizeA + 1) + sizeA];
}

void freeScoreMatrix(mtype *scoreMatrix) {
    free(scoreMatrix);
}

void printMatrix(char *seqA, char *seqB, mtype *scoreMatrix, long sizeA, long sizeB) {
    printf("Score Matrix:\n");
    printf("    ");
    for (long j = 0; j < sizeA; j++)
        printf("%5c ", seqA[j]);
    printf("\n");

    for (long i = 0; i <= sizeB; i++) {
        if (i == 0)
            printf("  ");
        else
            printf("%c ", seqB[i - 1]);

        for (long j = 0; j <= sizeA; j++) {
            printf("%5d ", scoreMatrix[i * (sizeA + 1) + j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <fileA> <fileB>\n", argv[0]);
        exit(1);
    }

    double start = omp_get_wtime();

    char *seqA, *seqB;
    long sizeA, sizeB;

    #pragma omp parallel sections num_threads(2)
    {
        #pragma omp section
        { seqA = read_seq(argv[1]); }

        #pragma omp section
        { seqB = read_seq(argv[2]); }
    }

    sizeA = strlen(seqA);
    sizeB = strlen(seqB);

    mtype *scoreMatrix = allocateScoreMatrix(sizeA, sizeB);
    initScoreMatrix(scoreMatrix, sizeA, sizeB);

    mtype score = pLCS_antidiagonal(scoreMatrix, sizeA, sizeB, seqA, seqB);

#ifdef DEBUGMATRIX
    printMatrix(seqA, seqB, scoreMatrix, sizeA, sizeB);
#endif

    printf("\nScore: %d\n", score);

    freeScoreMatrix(scoreMatrix);
    free(seqA);
    free(seqB);

    double end = omp_get_wtime();
    printf("Time: %.6f seconds\n", end - start);

    return EXIT_SUCCESS;
}
