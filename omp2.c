#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define TOTALSIZE 1000
#define NUMITER 200
#define f(x,y) ((x + y) / 2.0)

int main(int argc, char *argv[]) {
  int i, iter;
  double *V1 = (double *) malloc(TOTALSIZE * sizeof(double)); //vetor 1
  double *V2 = (double *) malloc(TOTALSIZE * sizeof(double)); //vetor 2

  // for que inicializa o vetor principal, nesse caso o 1 
  for(i = 0; i < TOTALSIZE; i++) {
    V1[i] = 0.0 + i;
  }

  // iterações
  for(iter = 0; iter < NUMITER; iter++) {

    #pragma omp parallel for
    for(i = 0; i < TOTALSIZE-1; i++) {
      V2[i] = f(V1[i], V1[i+1]);
    }
    //copiando o ultimo elemento 
    V2[TOTALSIZE - 1] = V1[TOTALSIZE - 1]; 

    //trocando os ponteiros
    double *temp = V1; 
    V1 = V2 ; 
    V2 = temp ;
  }

  // Saída final
  printf("Output:\n");
  for(i = 0; i < TOTALSIZE; i++) {
    printf("%4d %f\n", i, V1[i]);
  }

  free(V1);
  free(V2);

  return 0;
}
