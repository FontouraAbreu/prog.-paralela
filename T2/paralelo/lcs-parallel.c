#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

typedef unsigned short mtype;


char *read_seq(const char *fname)
{
    FILE *fp = fopen(fname, "rt");
    if (!fp) { fprintf(stderr, "Erro abrindo %s\n", fname); MPI_Abort(MPI_COMM_WORLD, 1); }

    fseek(fp, 0L, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);

    char *seq = (char *)calloc(fsize + 1, sizeof(char));
    if (!seq) { fprintf(stderr, "Falha na malloc do arquivo\n"); MPI_Abort(MPI_COMM_WORLD, 1); }

    long len = 0;
    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (c != '\n' && c != '\r') seq[len++] = (char)c;
    }
    seq[len] = '\0';
    fclose(fp);
    return seq;
}


mtype *allocateScoreMatrix(int sizeA, int sizeB)
{
	mtype * matrix = (mtype *)calloc((sizeA + 1) * (sizeB + 1), sizeof(mtype));

    return matrix;
}

void initScoreMatrix(mtype *M, int sizeA, int sizeB)
{
	// primeira linha
    for (int j = 0; j <= sizeA; ++j)
        M[j] = 0;

    // primeira coluna
    for (int i = 1; i <= sizeB; ++i)
        M[i * (sizeA + 1)] = 0;
}

int LCS_MPI_antidiagonal(mtype *score_matrix,
                     int sizeA, int sizeB,
                     const char *seqA, const char *seqB,
                     int rank, int nproc)
{
    /* buffers auxiliares para Allgatherv (tamanho ≤ max(sizeA, sizeB)) */
    int max_k = sizeA + sizeB + 1;
    mtype *diag_recv = (mtype *)malloc(max_k * sizeof(mtype));
    
   

    free(diag_recv);

    /* elemento final (sizeB, sizeA) */
    return score_matrix[sizeB * (sizeA + 1) + sizeA];
}


int main(int argc, char **argv)
{
    int rank, nproc;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if (argc != 3) {
        if (rank == 0) 
			fprintf(stderr, "Uso: %s <seqA.txt> <seqB.txt>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    char *seqA = NULL, *seqB = NULL;
    int sizeA = 0, sizeB = 0;
	
	// Rank 0 faz as leituras
    if (rank == 0) {
        seqA = read_seq(argv[1]);
        seqB = read_seq(argv[2]);
        sizeA = strlen(seqA);
        sizeB = strlen(seqB);
    }

    // broadcast do tamanho das sequências
    MPI_Bcast(&sizeA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&sizeB, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // alocação das sequências em cada rank
    if (rank != 0) {
        seqA = (char *)malloc(sizeA + 1);
        seqB = (char *)malloc(sizeB + 1);
    }
	if (!seqA || !seqB) {
		fprintf(stderr, "Falha na alocacao das sequencias\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	
	// broadcast das sequências
    MPI_Bcast(seqA, sizeA + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(seqB, sizeB + 1, MPI_CHAR, 0, MPI_COMM_WORLD);


	/*---------------------------------------------------------------------*/
	// espera todos os ranks receberem os dados anteriores				   //
	/*---------------------------------------------------------------------*/
    MPI_Barrier(MPI_COMM_WORLD);
    double time_start = MPI_Wtime();

    mtype *score_matrix = allocateScoreMatrix(sizeA, sizeB);
    initScoreMatrix(score_matrix, sizeA, sizeB);

    // cada rank calcula sua parte da matriz
    int score = LCS_MPI_antidiagonal(score_matrix, sizeA, sizeB, seqA, seqB, rank, nproc);

    double time_end = MPI_Wtime();

    // rank 0 mostra o resultado
    if (rank == 0) {
        printf("Score: %d\n", score);
        printf("Time (%.0f×%.0f, %d proc): %.6f s\n",
               (double)sizeA, (double)sizeB, nproc, time_end - time_start);
    }

    free(score_matrix);
    free(seqA);
    free(seqB);

    MPI_Finalize();
    return 0;
}
