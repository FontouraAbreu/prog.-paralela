#include <omp.h>
#include <stdio.h>

# define NUM_THREADS 8


static long num_steps= 1000000000;

void main() {

    int i, nthreads;
    double pi, sum[NUM_THREADS][8] = {{0}};
    double step = 1.0/(double) num_steps;

    #pragma omp parallel num_threads(NUM_THREADS)
    {
        int i, id, nthrds;
        double x;
        id = omp_get_thread_num();
        nthrds = omp_get_num_threads();

        if (id==0)
            nthreads = nthrds;
        
        // sum[id][0]=0.0;
        #pragma omp for
        for (i=id; i<num_steps; i+=nthrds) {
            x = (i + 0.5)*step;
            sum[id][0] += 4.0 / (1.0+x*x);
        }
    }

    for (i=0, pi=0.0; i< nthreads; i++) {
        pi += sum[i][0]*step;
    }

    printf("pi is: %.12f\n", pi);

}