#include <immintrin.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "timer.h"
#include "matrix_lib.h"



/* Thread to multiply arrays */
void *mult_scalar(void *threadarg) {
  struct thread_data *my_data;

  my_data = (struct thread_data *) threadarg;

	float *nxt_a = a + my_data->buffer_begin;
  float *nxt_scalar = scalar + my_data->buffer_begin;
  float *nxt_result = result + my_data->buffer_begin;

  for (long unsigned int i = my_data->buffer_begin;
	i < my_data->buffer_end; 
	i += my_data->stride, nxt_matrix_a += my_data->stride, 
	nxt_result += my_data->stride) {
	
	  /* Initialize the three argument vectors */
	  __m256 vec_matrix_a = _mm256_load_ps(nxt_matrix_a);

	  /* Compute the expression res = a * b + c between the three vectors */
	  __m256 vec_result = _mm256_mul_ps(vec_scalar_value, vec_matrix_a);

	  /* Store the elements of the result vector */
	  _mm256_store_ps(nxt_result, vec_result);
  }
  pthread_exit(NULL);
}


int scalar_matrix_mult(float scalar_value, struct matrix *matrix) {
  unsigned long int N;
	long t;
  
	/* Check the numbers of the elements of the matrix */
  N = matrix->height * matrix->width;

  /* Check the integrity of the matrix */
  if (N == 0 || matrix->rows == NULL) return 0;

	float *nxt_matrix_a = matrix->rows; 
  float *nxt_result = matrix->rows; 
  

	  /* Initialize and set thread detached attribute */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  /* Create threads to calculate product of matrix and scalar*/
  for(t=0; t<NUM_THREADS; t++){
    thread_data_array[t].thread_id = t;
    thread_data_array[t].buffer_begin = t * buffer_chunk;
    thread_data_array[t].buffer_end = t * buffer_chunk + buffer_chunk;
    thread_data_array[t].buffer_size = N;
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
  unsigned long int NA, NB, NC, i, j, k;
  float* nxt_a; 
  float* nxt_b; 
  float* nxt_c; 

  /* Check the numbers of the elements of the matrix */
  NA = a->height * a->width;
  NB = b->height * b->width;
  NC = c->height * c->width;

  /* Check the integrity of the matrix */
  if ( (NA == 0 || a->rows == NULL) ||
       (NB == 0 || b->rows == NULL) ||
       (NC == 0 || c->rows == NULL) ) return 0;

  /* Check if we can execute de product of matrix A and matrib B */
  if ( (a->width != b->height) ||
       (c->height != a->height) ||
       (c->width != b->width) ) return 0;

  for ( i = 0, nxt_a = a->rows; 
	i < a->height; 
	i += 1) {

	  /* Set nxt_b to the begining of matrixB */
	  nxt_b = b->rows;

      	  for ( j = 0; 
	    j < a->width; 
	    j += 1, nxt_a += 1) {
		/* Initialize the scalar vector with the next scalar value */
	  	__m256 vec_a = _mm256_set1_ps(*nxt_a);

		/* 
		 * Compute the product between the scalar vector and the elements of 
		 * a row of matrixB, 8 elements at a time, and add the result to the 
		 * respective elements of a row of matrixC, 8 elements at a time.
		 */
          	for (k = 0, nxt_c = c->rows + (c->width * i);
	    	     k < b->width;
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

  return 1;
}
