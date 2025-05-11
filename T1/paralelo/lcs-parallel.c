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
	//file pointer
	FILE *fseq = NULL;
	//sequence size
	long size = 0;
	//sequence pointer
	char *seq = NULL;
	//sequence index
	int i = 0;

	//open file
	fseq = fopen(fname, "rt");
	if (fseq == NULL ) {
		printf("Error reading file %s\n", fname);
		exit(1);
	}

	//find out sequence size to allocate memory afterwards
	fseek(fseq, 0L, SEEK_END);
	size = ftell(fseq);
	rewind(fseq);

	//allocate memory (sequence)
	seq = (char *) calloc(size + 1, sizeof(char));
	if (seq == NULL ) {
		printf("Erro allocating memory for sequence %s.\n", fname);
		exit(1);
	}

	//read sequence from file
	while (!feof(fseq)) {
		seq[i] = fgetc(fseq);
		if ((seq[i] != '\n') && (seq[i] != EOF))
			i++;
	}
	//insert string terminator
	seq[i] = '\0';

	//close file
	fclose(fseq);

	//return sequence pointer
	return seq;
}


mtype ** allocateScoreMatrix(int sizeA, int sizeB) {
	int i;
	//Allocate memory for LCS score matrix
	mtype ** scoreMatrix = (mtype **) malloc((sizeB + 1) * sizeof(mtype *));

	# pragma omp parallel for
	for (i = 0; i < (sizeB + 1); i++)
		scoreMatrix[i] = (mtype *) malloc((sizeA + 1) * sizeof(mtype));

	return scoreMatrix;
}

void initScoreMatrix(mtype ** scoreMatrix, int sizeA, int sizeB) {
	int i, j;
	//Fill first line of LCS score matrix with zeroes
	# pragma omp parallel for
	for (j = 0; j < (sizeA + 1); j++)
		scoreMatrix[0][j] = 0;

	//Do the same for the first collumn
	// Fill first column of LCS score matrix with zeroes
	# pragma omp parallel for
	for (i = 1; i < (sizeB + 1); i++)
		scoreMatrix[i][0] = 0;
}

int LCS_antidiagonal(mtype **scoreMatrix, int sizeA, int sizeB, char *seqA, char *seqB) {
    int i, j, d;

    // starting at 2 to avoid the first row and column
	// d is the diagonal index
    for (d = 2; d <= sizeA + sizeB; d++) {
        // i ranges from max(1, d - sizeA) to min(sizeB, d - 1)
        int row_start = d - sizeA;
		if (d - sizeA < 1) 
			row_start = 1;

		// j ranges from max(1, d - sizeB) to min(sizeA, d - 1)
        int row_end = d - 1;
		if (d - 1 > sizeB)
			row_end = sizeB;

		// Fill the diagonal
        for (i = row_start; i <= row_end; i++) {
			// if the column is out of bounds, skip
            j = d - i;
            if (j > sizeA) continue;

			// match case
            if (seqA[j - 1] == seqB[i - 1]) {
                scoreMatrix[i][j] = scoreMatrix[i - 1][j - 1] + 1;
			// mismatch case
            } else {
                scoreMatrix[i][j] = max(scoreMatrix[i - 1][j], scoreMatrix[i][j - 1]);
            }
        }
    }

    return scoreMatrix[sizeB][sizeA];
}

int pLCS_antidiagonal(mtype **scoreMatrix, int sizeA, int sizeB, char *seqA, char *seqB) {
    int i, j, d;

    // starting at 2 to avoid the first row and column
	// d is the diagonal index
    for (d = 2; d <= sizeA + sizeB; d++) {
        // i ranges from max(1, d - sizeA) to min(sizeB, d - 1)
        int row_start = d - sizeA;
		if (d - sizeA < 1) 
			row_start = 1;

		// j ranges from max(1, d - sizeB) to min(sizeA, d - 1)
        int row_end = d - 1;
		if (d - 1 > sizeB)
			row_end = sizeB;

		// Fill the diagonal
		#pragma omp parallel for private(i, j) shared(scoreMatrix, seqA, seqB)
        for (i = row_start; i <= row_end; i++) {
			// if the column is out of bounds, skip
            j = d - i;
            if (j > sizeA) continue;

			// match case
            if (seqA[j - 1] == seqB[i - 1]) {
                scoreMatrix[i][j] = scoreMatrix[i - 1][j - 1] + 1;
			// mismatch case
            } else {
                scoreMatrix[i][j] = max(scoreMatrix[i - 1][j], scoreMatrix[i][j - 1]);
            }
        }
    }

    return scoreMatrix[sizeB][sizeA];
}


int LCS(mtype ** scoreMatrix, int sizeA, int sizeB, char * seqA, char *seqB) {
	int i, j;
	for (i = 1; i < sizeB + 1; i++) {
		for (j = 1; j < sizeA + 1; j++) {
			if (seqA[j - 1] == seqB[i - 1]) {
				/* if elements in both sequences match,
				 the corresponding score will be the score from
				 previous elements + 1*/
				scoreMatrix[i][j] = scoreMatrix[i - 1][j - 1] + 1;
			} else {
				/* else, pick the maximum value (score) from left and upper elements*/
				scoreMatrix[i][j] =
						max(scoreMatrix[i-1][j], scoreMatrix[i][j-1]);
			}
		}
	}
	return scoreMatrix[sizeB][sizeA];
}

void printMatrix(char * seqA, char * seqB, mtype ** scoreMatrix, int sizeA,
		int sizeB) {
	int i, j;

	//print header
	printf("Score Matrix:\n");
	printf("========================================\n");

	//print LCS score matrix allong with sequences

	printf("    ");
	printf("%5c   ", ' ');

	for (j = 0; j < sizeA; j++)
		printf("%5c   ", seqA[j]);
	printf("\n");
	for (i = 0; i < sizeB + 1; i++) {
		if (i == 0)
			printf("    ");
		else
			printf("%c   ", seqB[i - 1]);
		for (j = 0; j < sizeA + 1; j++) {
			printf("%5d   ", scoreMatrix[i][j]);
		}
		printf("\n");
	}
	printf("========================================\n");
}

void freeScoreMatrix(mtype **scoreMatrix, int sizeB) {
	int i;
	#pragma omp parallel for
	for (i = 0; i < (sizeB + 1); i++)
		free(scoreMatrix[i]);
	free(scoreMatrix);
}

double start, end;

int main(int argc, char ** argv) {
	if (argc != 3) {
		printf("Usage: %s <fileA> <fileB>\n", argv[0]);
		exit(1);
	}

	start = omp_get_wtime(); // Start the timer

	// sequence pointers for both sequences

	char *seqA, *seqB;

	// sizes of both sequences
	int sizeA, sizeB;

	//read both sequences
    #pragma omp parallel sections num_threads(2)
	{
		#pragma omp section
		{
			seqA = read_seq(argv[1]);
		}
		#pragma omp section
		{
			seqB = read_seq(argv[2]);
		}
	}

	//find out sizes
    #pragma omp parallel sections num_threads(2)
    {
		#pragma omp section
		{
	    	sizeA = strlen(seqA);
		}
		#pragma omp section
		{
	    	sizeB = strlen(seqB);
		}
    }

	//print sizes
	printf("Size A: %d\n", sizeA);
	printf("Size B: %d\n", sizeB);
    
	// allocate LCS score matrix
	mtype ** scoreMatrix = allocateScoreMatrix(sizeA, sizeB);

	//initialize LCS score matrix
	initScoreMatrix(scoreMatrix, sizeA, sizeB);

	//fill up the rest of the matrix and return final score (element locate at the last line and collumn)
	mtype score = pLCS_antidiagonal(scoreMatrix, sizeA, sizeB, seqA, seqB);

	/* if you wish to see the entire score matrix,
	 for debug purposes, define DEBUGMATRIX. */
#ifdef DEBUGMATRIX
	printMatrix(seqA, seqB, scoreMatrix, sizeA, sizeB);
#endif

	//print score
	printf("\nScore: %d\n", score);

	//free score matrix
	freeScoreMatrix(scoreMatrix, sizeB);

	end = omp_get_wtime(); // End the timer

	printf("Time taken: %.6f seconds\n", end - start);

	return EXIT_SUCCESS;
}
