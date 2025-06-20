#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
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

	int i = 0, c;
	while ((c = fgetc(fseq)) != EOF) {
		if (c != '\n' && c != '\r')
			seq[i++] = c;
	}
	seq[i] = '\0';
	fclose(fseq);
	return seq;
}

mtype ** allocateScoreMatrix(int sizeA, int sizeB) {
	mtype **scoreMatrix = (mtype **) malloc((sizeB + 1) * sizeof(mtype *));
	for (int i = 0; i < sizeB + 1; i++)
		scoreMatrix[i] = (mtype *) malloc((sizeA + 1) * sizeof(mtype));
	return scoreMatrix;
}

void initScoreMatrix(mtype **scoreMatrix, int sizeA, int sizeB) {
	for (int j = 0; j < sizeA + 1; j++)
		scoreMatrix[0][j] = 0;
	for (int i = 1; i < sizeB + 1; i++)
		scoreMatrix[i][0] = 0;
}

int LCS(mtype **scoreMatrix, int sizeA, int sizeB, char *seqA, char *seqB) {
	for (int i = 1; i <= sizeB; i++) {
		for (int j = 1; j <= sizeA; j++) {
			if (seqA[j - 1] == seqB[i - 1])
				scoreMatrix[i][j] = scoreMatrix[i - 1][j - 1] + 1;
			else
				scoreMatrix[i][j] = max(scoreMatrix[i - 1][j], scoreMatrix[i][j - 1]);
		}
	}
	return scoreMatrix[sizeB][sizeA];
}

void freeScoreMatrix(mtype **scoreMatrix, int sizeB) {
	for (int i = 0; i < sizeB + 1; i++)
		free(scoreMatrix[i]);
	free(scoreMatrix);
}

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("Usage: %s <fileA> <fileB>\n", argv[0]);
		exit(1);
	}

	clock_t total_start = clock();
	clock_t seq_start = clock();

	// Sequencial (leitura e alocação)
	char *seqA = read_seq(argv[1]);
	char *seqB = read_seq(argv[2]);
	int sizeA = strlen(seqA);
	int sizeB = strlen(seqB);
	mtype **scoreMatrix = allocateScoreMatrix(sizeA, sizeB);
	initScoreMatrix(scoreMatrix, sizeA, sizeB);

	clock_t seq_end = clock();
	double seq_time = ((double)(seq_end - seq_start)) / CLOCKS_PER_SEC;

	// Parte paralelizável (preenchimento da matriz)
	clock_t par_start = clock();
	mtype score = LCS(scoreMatrix, sizeA, sizeB, seqA, seqB);
	clock_t par_end = clock();
	double par_time = ((double)(par_end - par_start)) / CLOCKS_PER_SEC;

	// Total
	clock_t total_end = clock();
	double total_time = ((double)(total_end - total_start)) / CLOCKS_PER_SEC;

	printf("\nScore: %d\n", score);
	printf("TotalTime: %f seconds\n", total_time);
	printf("SeqTime: %f seconds\n", seq_time);
	printf("ParTime: %f seconds\n", par_time);

	freeScoreMatrix(scoreMatrix, sizeB);
	free(seqA);
	free(seqB);

	return EXIT_SUCCESS;
}
