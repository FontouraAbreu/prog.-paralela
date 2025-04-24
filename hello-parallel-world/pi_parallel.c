#include <omp.h>
#include <stdio.h>

# define NUM_THREADS 8


static long num_steps= 1000000000;
double step;


int main () {
    int i;
    double pi;
    
    double x = 0.0;
    double sum = 0.0;
    step = 1.0/(double) num_steps;
    
    #pragma omp parallel
    {
        #pragma omp for reduction private(x) (+:sum) schedule(auto)
        {
            for (i=0;i< num_steps; i++){
                x = (i + 0.5) * step; // Largura do retângulo
                sum = sum + 4.0 / (1.0 + x*x); // Sum += Área do retângulo
            }
        } 
    }
    pi = step * sum;

    printf("pi is: %.20f\n", pi);

}