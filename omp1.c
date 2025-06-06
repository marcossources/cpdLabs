
#include<stdio.h>
#include<stdlib.h>
#include<omp.h> // this is a new library

#define NUMITER 30

/* main: process parameters */
int main(int argc, char *argv[]) {

  /* variables */
  int i, tid;

/* create parallel region, make both variable thread private */
#pragma omp parallel private(i,tid)
  {
      /* get id of thread */
      tid = omp_get_thread_num();

/* divide loop iterations evenly by threads */
#pragma omp for nowait 
    for(i = 0; i < NUMITER; i++)
	  	printf("Thread: %d\titer=%d\n", tid, i);  fflush(stdout);

    /* one of these messages per thread */
    #pragma omp barrier 
    printf("Thread %d, almost..\n", tid);fflush(stdout);
    #pragma omp barrier 
    printf("Thread %d, done!\n", tid);  fflush(stdout);
  }

  /* only master thread prints this one */
  printf("All threads have finished!\n");
}