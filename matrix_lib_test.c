#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "timer.h"
#include "matrix_lib.h"

int store_matrix(struct matrix *matrix, char *filename) {
  unsigned long int i = 0;
  unsigned long int N = 0;
  FILE *fd = NULL;

  /* Check the numbers of the elements of the matrix */
  N = matrix->height * matrix->width;

  /* Check the integrity of the matrix */
  if (N == 0 || matrix->rows == NULL) return 0;

  /* Try to open file of floats */
  if ((fd = fopen(filename, "wb")) == NULL) {
    printf("Unable to open file %s\n", filename);
    return 0;
  }

  float *nxt_a = matrix->rows; 

  for ( i = 0;
	i < N; 
	i += 8, nxt_a += 8) {

	if (fwrite(nxt_a, sizeof(float), 8, fd) != 8) {
            printf("Error writing to file %s: short write (less than 8 floats)\n", filename);
            return 0;
	}
  }

  if (fd != NULL) fclose(fd);

  return 1;
}

int load_matrix(struct matrix *matrix, char *filename) {
  unsigned long int i = 0;
  unsigned long int N = 0;
  FILE *fd = NULL;

  /* Check the numbers of the elements of the matrix */
  N = matrix->height * matrix->width;

  /* Check the integrity of the matrix */
  if (N == 0 || matrix->rows == NULL) return 0;

  /* Try to open file of floats */
  if ((fd = fopen(filename, "rb")) == NULL) {
    printf("Unable to open file %s\n", filename);
    return 0;
  }

  float *nxt_a = matrix->rows; 

  for ( i = 0;
	i < N; 
	i += 8, nxt_a += 8) {

	if (fread(nxt_a, sizeof(float), 8, fd) != 8) {
            printf("Error reading from file %s: short read (less than 8 floats)\n", filename);
            return 0;
	}
  }

  if (fd != NULL) fclose(fd);

  return 1;
}

int initialize_matrix(struct matrix *matrix, float value, float inc) {
  unsigned long int i;
  unsigned long int N;
  unsigned long int nxt_inc;

  /* Check the numbers of the elements of the matrix */
  N = matrix->height * matrix->width;

  /* Check the integrity of the matrix */
  if (N == 0 || matrix->rows == NULL) return 0;

  float *nxt_a = matrix->rows; 
  for ( i = 0, nxt_inc = matrix->width; 
	i < N; 
	i += 8, nxt_a += 8) {

	  /* Check if it is time to increse value */
	  if (i == nxt_inc) {
		nxt_inc += matrix->width;
		value += inc;
	  }

  	  /* Initialize main vector */
  	  __m256 vec_a = _mm256_set1_ps(value);

	  /* Store the elements of the vectors in the arrays */
	  _mm256_store_ps(nxt_a, vec_a);
  }

  return 1;
}

int print_matrix(struct matrix *matrix) {
  unsigned long int i;
  unsigned long int N;
  unsigned long int nxt_newLine;

  /* Check the numbers of the elements of the matrix */
  N = matrix->height * matrix->width;

  /* Check the integrity of the matrix */
  if (N == 0 || matrix->rows == NULL) return 0;

  /* Initialize new line controol */
  nxt_newLine = matrix->width - 1;

  /* Print matrix elements */
  for (i = 0; i < N; i++) {
     printf("%5.1f ", matrix->rows[i]);
     if (i == nxt_newLine) {
	printf("\n");
	nxt_newLine += matrix->width;
     }
  }

  return 1;
}

int check_errors(struct matrix *matrix, float scalar_value) {
  unsigned long int i;
  unsigned long int N;

  /* Check the numbers of the elements of the matrix */
  N = matrix->height * matrix->width;

  /* Check the integrity of the matrix */
  if (N == 0 || matrix->rows == NULL) return 0;

  /* Check for errors (all values should be equal to scalar_value) */
  float maxError = 0.0f;
  float diffError = 0.0f;
  for (i = 0; i < N; i++)
    maxError = (maxError > (diffError=fabs(matrix->rows[i]-scalar_value)))? maxError : diffError;
  printf("Max error: %f\n", maxError);

  return 1;
}

int main_func(int argc, char *argv[]) {
  unsigned long int DimA_M, DimA_N, DimB_M, DimB_N;
  char *matrixA_filename, *matrixB_filename, *result1_filename, *result2_filename;
  char *eptr = NULL;
  struct timeval start, stop;

  // Disable buffering entirely
  setbuf(stdout, NULL);

  // Check arguments
  if (argc != 11) {
        printf("Usage: %s <scalar_value> <DimA_M> <DimA_N> <DimB_M> <DimB_N> <NUM_THREADS> <matrixA_filename> <matrixB_filename> <result1_filename> <result2_filename>\n", argv[0]);
        return 0;
  } else {
    //printf("Number of args: %d\n", argc);
    //for (int i=0; i<argc; ++i)
      //       printf("argv[%d] = %s\n", i, argv[i]);
  }

  // Convert arguments
  scalar_value = strtof(argv[1], NULL);
  DimA_M = strtol(argv[2], &eptr, 10);
  DimA_N = strtol(argv[3], &eptr, 10);
  DimB_M = strtol(argv[4], &eptr, 10);
  DimB_N = strtol(argv[5], &eptr, 10);
  int num = strtol(argv[6], &eptr, 10);
  matrixA_filename = argv[7];
  matrixB_filename = argv[8];
  result1_filename = argv[9];
  result2_filename = argv[10];
  
  if(num == 0)  num = 1;

  set_num_threads(num);

  if ((scalar_value == 0.0f) || (DimA_M == 0) || (DimA_N == 0) || (DimB_M == 0) || (DimB_N == 0) || NUM_THREADS == 0) {
    printf("%s: erro na conversao do argumento: errno = %d\n", argv[0], errno);

    /* If a conversion error occurred, display a message and exit */
    if (errno == EINVAL)
    {
        printf("Conversion error occurred: %d\n", errno);
        return 1;
    }

    /* If the value provided was out of range, display a warning message */
    if (errno == ERANGE) {
        printf("The value provided was out of rangei: %d\n", errno);
        return 1;
    }
  }

  /* Allocate three arrays */
  printf("Allocating memory...");
  gettimeofday(&start, NULL);

  /* Allocate the arrays of the four matrixes */
  float *a = (float*)aligned_alloc(32, DimA_M*DimA_N*sizeof(float));
  float *b = (float*)aligned_alloc(32, DimB_M*DimB_N*sizeof(float));
  float *c = (float*)aligned_alloc(32, DimA_M*DimB_N*sizeof(float));

  gettimeofday(&stop, NULL);
  printf("%f ms\n", timedifference_msec(start, stop));

  if ((a == NULL) || (b == NULL) || (c == NULL)) {
	  printf("%s: array allocation problem.", argv[0]);
	  return 1;
  }

  /* Initialize the three matrixes */
  matrixA.height = DimA_M;
  matrixA.width = DimA_N;
  matrixA.rows = a;
  //if (!initialize_matrix(&matrixA, 5.0f, 0.0f)) {
  if (!load_matrix(&matrixA, matrixA_filename)) {
	  printf("%s: matrixA initialization problem.", argv[0]);
	  return 1;
  }

  /* Print matrix */
  printf("---------- Matrix A ----------\n");
  print_matrix(&matrixA);

  matrixB.height = DimB_M;
  matrixB.width = DimB_N;
  matrixB.rows = b;
  //if (!initialize_matrix(&matrixB, 1.0f, 0.0f)) {
  if (!load_matrix(&matrixB, matrixB_filename)) {
	  printf("%s: matrixB initialization problem.", argv[0]);
	  return 1;
  }

  /* Print matrix */
  printf("---------- Matrix B ----------\n");
  print_matrix(&matrixB);

  matrixC.height = DimA_M;
  matrixC.width = DimB_N;
  matrixC.rows = c;

  /* Print matrix */
  printf("---------- Matrix C ----------\n");
  print_matrix(&matrixC);

  /* Define auxiliary variables to work with threads */
  struct thread_data thread_data_array[NUM_THREADS];
  pthread_t thread[NUM_THREADS];
  int rc;
  long t;
  void *status;
  long unsigned int buffer_chunk = (matrixC.height * matrixC.width) / NUM_THREADS;

  /* Scalar product of matrix A */
  printf("Executing scalar_matrix_mult(%5.1f, matrixA)...\n",scalar_value);
  if (!scalar_matrix_mult(scalar_value, &matrixA)) {
    printf("%s: scalar_matrix_mult problem.", argv[0]);
    return 1;
  }

  /* Print matrix */
  printf("---------- Matrix A ----------\n");
  print_matrix(&matrixA);

  /* Write first result */
  if (!store_matrix(&matrixA, result1_filename)) {
	printf("%s: failed to write first result to file.", argv[0]);
	return 1;
  }


    /* Calculate the product between matrix A and matrix B */
  printf("Executing matrix_matrix_mult(matrixA, mattrixB, matrixC)...\n");
  if (!matrix_matrix_mult(&matrixA, &matrixB, &matrixC)) {
	  printf("%s: matrix_matrix_mult problem.", argv[0]);
	  return 1;
  }

  /* Print matrix */
  printf("---------- Matrix C ----------\n");
  print_matrix(&matrixC);
 
   /* Write second result */
  if (!store_matrix(&matrixC, result2_filename)) {
	printf("%s: failed to write second result to file.", argv[0]);
	return 1;
  }

  return 0;
}