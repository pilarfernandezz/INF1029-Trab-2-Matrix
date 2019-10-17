#include <stdio.h>
#include "timer.h"

float timedifference_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

int main(int argc, char *argv[]) {
  // Define exit code and time markers variables 
  int exit_code = 0;
  struct timeval overall_t1, overall_t2;
  // Mark overall start time
  gettimeofday(&overall_t1, NULL);
  // Call target main function
  exit_code = main_func(argc, argv);
  //Mark overall stop time
  gettimeofday(&overall_t2, NULL);
  // Show elased time
  printf("Overall time: %f ms\n", timedifference_msec(overall_t1, overall_t2));
  // Return exit code
  return exit_code;
}

