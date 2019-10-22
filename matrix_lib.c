#include <immintrin.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "timer.h"
#include "matrix_lib.h"

void *mult_scalar(void *threadarg) {
  struct thread_data *my_data;
  my_data = (struct thread_data *) threadarg;

  float *nxt_a = matrixA.rows + my_data->buffer_begin;
  float *nxt_result = matrixA.rows + my_data->buffer_begin;

   __m256 vec_scalar_value = _mm256_set1_ps(scalar_value);

  for (long unsigned int i = my_data->buffer_begin;
	i < my_data->buffer_end; 
	i += my_data->stride, nxt_a += my_data->stride, 
	nxt_result += my_data->stride) {

        printf("thread %d executando linha %d\n", my_data->thread_id, i/8);

          __m256 vec_a= _mm256_load_ps(nxt_a);

       /* Compute the difference between the two arrays */
          __m256 vec_result = _mm256_mul_ps(vec_a, vec_scalar_value);

       /* Store the elements of the result array */
          _mm256_store_ps(nxt_result, vec_result);
  }

  pthread_exit(NULL);
}

void *mult_matrix(void *threadarg) {

  int i, j, k;
  struct thread_data *my_data;

  my_data = (struct thread_data *) threadarg;

  float *nxt_a;
  float *nxt_b;
  float *nxt_c;

  for ( i = my_data->thread_id, nxt_a = matrixA.rows; 
    i < matrixA.height; 
    i += NUM_THREADS) {
    
      printf("thread %d executando linha %d\n", my_data->thread_id, i);

      /* Set nxt_b to the begining of matrixB */
      nxt_b = matrixB.rows;

      for ( j = 0; 
      j < matrixA.width; 
      j += 1, nxt_a += 1) {
      /* Initialize the scalar vector with the next scalar value */
        __m256 vec_a = _mm256_set1_ps(*nxt_a);

        /* 
        * Compute the product between the scalar vector and the elements of 
        * a row of matrixB, 8 elements at a time, and add the result to the 
        * respective elements of a row of matrixC, 8 elements at a time.
        */
          for (k = 0, nxt_c = matrixC.rows + (matrixC.width * i);
            k < matrixB.width;
          k += VECTOR_SIZE, nxt_b += VECTOR_SIZE, nxt_c += VECTOR_SIZE) {

            /* Load part of b row (size of vector) */
            __m256 vec_b = _mm256_load_ps(nxt_b);

                /* Initialize vector c with zero or load part of c row (size of vector) */
            __m256 vec_c;

            if (j == 0) { /* if vec_a is the first scalar vector, vec_c is set to zero */
            vec_c = _mm256_setzero_ps();
            } else { /* otherwise, load part of c row (size of vector) to vec_c */
            vec_c = _mm256_load_ps(nxt_c);
            }

            /* Compute the expression res = a * b + c between the three vectors */
            vec_c = _mm256_fmadd_ps(vec_a, vec_b, vec_c);

            /* Store the elements of the result vector */
            _mm256_store_ps(nxt_c, vec_c);
		
        }
      }
    }

  pthread_exit(NULL);
}


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

