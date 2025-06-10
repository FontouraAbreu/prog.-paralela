#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

typedef unsigned short mtype;

char* read_seq(char *fname) {
	FILE *fseq = fopen(fname, "rt");
	if (!fseq) {
		printf("Error reading file %s\n", fname);
		exit(1);
	}

	fseek(fseq, 0L, SEEK_END);
	long size = ftell(fseq);
	rewind(fseq);

	char *seq = (char *) calloc(size + 1, sizeof(char));
	if (!seq) {
		printf("Error allocating memory for sequence %s.\n", fname);
		exit(1);
	}

	int i = 0;
	while (!feof(fseq)) {
		seq[i] = fgetc(fseq);
		if ((seq[i] != '\n') && (seq[i] != EOF))
			i++;
	}
	seq[i] = '\0';
	fclose(fseq);
	return seq;
}

mtype* allocateScoreMatrix(int sizeA, int sizeB) {
	return (mtype *) calloc((sizeA + 1) * (sizeB + 1), sizeof(mtype));
}

void initScoreMatrix(mtype *scoreMatrix, int sizeA, int sizeB) {
	int i;
	#pragma omp parallel for
	for (i = 0; i <= sizeA; i++)
		scoreMatrix[i] = 0;

	#pragma omp parallel for
	for (i = 1; i <= sizeB; i++)
		scoreMatrix[i * (sizeA + 1)] = 0;
}

int pLCS_antidiagonal(mtype *scoreMatrix, int sizeA, int sizeB, char *seqA, char *seqB) {
	int i, j, d;

	for (d = 2; d <= sizeA + sizeB; d++) {
		int row_start = (d - sizeA < 1) ? 1 : d - sizeA;
		int row_end = (d - 1 > sizeB) ? sizeB : d - 1;

		#pragma omp parallel for private(i, j) shared(scoreMatrix, seqA, seqB)
		for (i = row_start; i <= row_end; i++) {
			j = d - i;
			if (j > sizeA) continue;

			int idx = i * (sizeA + 1) + j;
			int up = (i - 1) * (sizeA + 1) + j;
			int left = i * (sizeA + 1) + (j - 1);
			int diag = (i - 1) * (sizeA + 1) + (j - 1);

			if (seqA[j - 1] == seqB[i - 1]) {
				scoreMatrix[idx] = scoreMatrix[diag] + 1;
			} else {
				scoreMatrix[idx] = max(scoreMatrix[up], scoreMatrix[left]);
			}
		}
	}

	return scoreMatrix[sizeB * (sizeA + 1) + sizeA];
}

void printMatrix(char *seqA, char *seqB, mtype *scoreMatrix, int sizeA, int sizeB) {
	int i, j;

	printf("Score Matrix:\n");
	printf("========================================\n");
	printf("    %5c   ", ' ');

	for (j = 0; j < sizeA; j++)
		printf("%5c   ", seqA[j]);
	printf("\n");

	for (i = 0; i <= sizeB; i++) {
		if (i == 0)
			printf("    ");
		else
			printf("%c   ", seqB[i - 1]);

		for (j = 0; j <= sizeA; j++) {
			printf("%5d   ", scoreMatrix[i * (sizeA + 1) + j]);
		}
		printf("\n");
	}
	printf("========================================\n");
}

void freeScoreMatrix(mtype *scoreMatrix) {
	free(scoreMatrix);
}

double start, end;

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("Usage: %s <fileA> <fileB>\n", argv[0]);
		exit(1);
	}

	start = omp_get_wtime();

	char *seqA, *seqB;
	int sizeA, sizeB;

	#pragma omp parallel sections num_threads(2)
	{
		#pragma omp section
		seqA = read_seq(argv[1]);

		#pragma omp section
		seqB = read_seq(argv[2]);
	}

	#pragma omp parallel sections num_threads(2)
	{
		#pragma omp section
		sizeA = strlen(seqA);

		#pragma omp section
		sizeB = strlen(seqB);
	}

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

	end = omp_get_wtime();
	printf("Time: %.6f seconds\n", end - start);

	return EXIT_SUCCESS;
}
