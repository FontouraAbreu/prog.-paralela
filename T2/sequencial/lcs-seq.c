#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

typedef unsigned short mtype;

char* read_seq(char *fname) {
    FILE *fseq = NULL;
    long size = 0;
    char *seq = NULL;
    int i = 0;

    fseq = fopen(fname, "rt");
    if (fseq == NULL) {
        printf("Error reading file %s\n", fname);
        exit(1);
    }

    fseek(fseq, 0L, SEEK_END);
    size = ftell(fseq);
    rewind(fseq);

    seq = (char *) calloc(size + 1, sizeof(char));
    if (seq == NULL) {
        printf("Error allocating memory for sequence %s.\n", fname);
        exit(1);
    }

    while (!feof(fseq)) {
        seq[i] = fgetc(fseq);
        if ((seq[i] != '\n') && (seq[i] != EOF))
            i++;
    }
    seq[i] = '\0';
    fclose(fseq);
    return seq;
}

mtype** allocateScoreMatrix(int sizeA, int sizeB) {
    int i;
    mtype **scoreMatrix = (mtype **) malloc((sizeB + 1) * sizeof(mtype *));
    for (i = 0; i < (sizeB + 1); i++)
        scoreMatrix[i] = (mtype *) malloc((sizeA + 1) * sizeof(mtype));
    return scoreMatrix;
}

void initScoreMatrix(mtype **scoreMatrix, int sizeA, int sizeB) {
    int i, j;
    for (j = 0; j < (sizeA + 1); j++)
        scoreMatrix[0][j] = 0;
    for (i = 1; i < (sizeB + 1); i++)
        scoreMatrix[i][0] = 0;
}

int LCS(mtype **scoreMatrix, int sizeA, int sizeB, char *seqA, char *seqB) {
    int i, j;
    for (i = 1; i < sizeB + 1; i++) {
        for (j = 1; j < sizeA + 1; j++) {
            if (seqA[j - 1] == seqB[i - 1]) {
                scoreMatrix[i][j] = scoreMatrix[i - 1][j - 1] + 1;
            } else {
                scoreMatrix[i][j] = max(scoreMatrix[i-1][j], scoreMatrix[i][j-1]);
            }
        }
    }
    return scoreMatrix[sizeB][sizeA];
}

void freeScoreMatrix(mtype **scoreMatrix, int sizeB) {
    for (int i = 0; i < (sizeB + 1); i++)
        free(scoreMatrix[i]);
    free(scoreMatrix);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <fileA> <fileB>\n", argv[0]);
        exit(1);
    }

    clock_t start, end;
    double cpu_time_used;

#ifndef MEASURE_LCS_ONLY
    start = clock();
#endif

    char *seqA = read_seq(argv[1]);
    char *seqB = read_seq(argv[2]);
    int sizeA = strlen(seqA);
    int sizeB = strlen(seqB);
    mtype **scoreMatrix = allocateScoreMatrix(sizeA, sizeB);
    initScoreMatrix(scoreMatrix, sizeA, sizeB);

#ifdef MEASURE_LCS_ONLY
    clock_t lcs_start = clock();
#endif

    mtype score = LCS(scoreMatrix, sizeA, sizeB, seqA, seqB);

#ifdef MEASURE_LCS_ONLY
    clock_t lcs_end = clock();
    double lcs_time = ((double)(lcs_end - lcs_start)) / CLOCKS_PER_SEC;
    printf("LCS_Time: %f seconds\n", lcs_time);
#endif

    printf("\nScore: %d\n", score);

    freeScoreMatrix(scoreMatrix, sizeB);
    free(seqA);
    free(seqB);

#ifndef MEASURE_LCS_ONLY
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Time: %f seconds\n", cpu_time_used);
#endif

    return EXIT_SUCCESS;
}
