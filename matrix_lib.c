#include <immintrin.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "timer.h"
#include "matrix_lib.h"

void set_num_threads(int num_threads){
	NUM_THREADS = num_threads;
}

int scalar_matrix_mult(float scalar_value, struct matrix *matrix) { 
  /* Define auxiliary variables to work with threads */
  struct thread_data thread_data_array[NUM_THREADS];
  pthread_t thread[NUM_THREADS];
  int rc;
  long t;
  void *status;
  long unsigned int buffer_chunk = (matrix->height * matrix->width) / NUM_THREADS;

	/* Initialize and set thread detached attribute */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  /* Create threads to calculate product of matrix and scalar*/
  for(t=0; t<NUM_THREADS; t++){
    thread_data_array[t].thread_id = t;
    thread_data_array[t].buffer_begin = t * buffer_chunk;
    thread_data_array[t].buffer_end = t * buffer_chunk + buffer_chunk;
    thread_data_array[t].buffer_size = matrix->height * matrix->width;
    thread_data_array[t].stride = 8;

    if (rc = pthread_create(&thread[t], &attr, mult_scalar, (void *) &thread_data_array[t])) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
  }

  /* Free attribute and wait for the other threads */
  pthread_attr_destroy(&attr);
  for(t=0; t<NUM_THREADS; t++) {
    if (rc = pthread_join(thread[t], &status)) {
      printf("ERROR; return code from pthread_join() is %d\n", rc);
      exit(-1);
    }
  }

  return 1;
}

int matrix_matrix_mult(struct matrix *a, struct matrix *b, struct matrix *c) {

  /* Define auxiliary variables to work with threads */
  struct thread_data thread_data_array[NUM_THREADS];
  pthread_t thread[NUM_THREADS];
  int rc;
  long t;
  void *status;
  long unsigned int buffer_chunk = (c->height * c->width) / NUM_THREADS;

	/* Initialize and set thread detached attribute */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  /* Create threads to calculate product of matrix and scalar*/
  for(t=0; t<NUM_THREADS; t++){
    thread_data_array[t].thread_id = t;
    thread_data_array[t].buffer_begin = t * buffer_chunk;
    thread_data_array[t].buffer_end = t * buffer_chunk + buffer_chunk;
    thread_data_array[t].buffer_size = c->height * c->width;
    thread_data_array[t].stride = 8;

    if (rc = pthread_create(&thread[t], &attr, mult_matrix, (void *) &thread_data_array[t])) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
  }

  /* Free attribute and wait for the other threads */
  pthread_attr_destroy(&attr);
  for(t=0; t<NUM_THREADS; t++) {
    if (rc = pthread_join(thread[t], &status)) {
      printf("ERROR; return code from pthread_join() is %d\n", rc);
      exit(-1);
    }
  }

  return 1;
}
