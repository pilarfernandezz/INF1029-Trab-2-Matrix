#include <pthread.h>

#define VECTOR_SIZE 8

struct matrix {
	unsigned long int height;
	unsigned long int width;
	float *rows;
};

struct thread_data {
	int thread_id;
	long unsigned int buffer_begin;
	long unsigned int buffer_end;
	long unsigned int buffer_size;
	long unsigned int stride;
};

float scalar_value;
float *a;
float *b;
float *c;
float *scalar;
float *result;
int NUM_THREADS; 
pthread_attr_t attr;

void * mult_scalar(void *threadarg);

void * mult_matrix(void *threadarg);

int scalar_matrix_mult(float scalar_value, struct matrix *matrix);

int matrix_matrix_mult(struct matrix *a, struct matrix *b, struct matrix *c);
