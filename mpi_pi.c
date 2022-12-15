#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void Read_input(unsigned long long int* number_of_tosses, int my_rank, MPI_Comm comm);
unsigned long long int Monte_Carlo(unsigned long long int number_of_tosses, int my_rank);

int main(void) 
{
  unsigned long long int number_of_tosses;
  unsigned long long int local_number_of_tosses;
  unsigned long long int number_in_circle;
  unsigned long long int local_number_in_circle;

  int my_rank;
  int comm_sz;

  MPI_Comm comm;
  MPI_Init(NULL, NULL);
  comm = MPI_COMM_WORLD;
  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &my_rank);

  Read_input(&number_of_tosses, my_rank, comm);

  local_number_of_tosses = number_of_tosses / comm_sz;
  number_of_tosses = local_number_of_tosses * comm_sz;

  local_number_in_circle = Monte_Carlo(local_number_of_tosses, my_rank);

  MPI_Reduce(&local_number_in_circle, &number_in_circle, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, comm);
  if(my_rank == 0) 
  {
    double pi = 4 * ((double) number_in_circle) / ((double) number_of_tosses);
    //printf("Number of Tosses: %llu\n", number_of_tosses);
    printf("Number in Circle: %llu\n", number_in_circle);
    printf("Approximate value of pi: %f\n", pi);
  }

  MPI_Finalize();
  return 0;
}  /* main */

void Read_input(unsigned long long int* number_of_tosses, int my_rank, MPI_Comm comm) 
{
  if(my_rank == 0) 
  {
    printf("Input number of tosses (int): ");
    fflush(stdout);
    scanf("%llu", number_of_tosses);
  }

  MPI_Bcast(number_of_tosses, 1, MPI_UNSIGNED_LONG_LONG, 0, comm);
}  

unsigned long long int Monte_Carlo(unsigned long long int local_number_of_tosses, int my_rank) 
{
  unsigned long long int number_in_circle = 0;
  double radius = 1;
  for(unsigned long long int i = 0; i < local_number_of_tosses; i++) 
  {
    double x = ((double) rand()) / ((double) RAND_MAX) * radius * 2;
    double y = ((double) rand()) / ((double) RAND_MAX) * radius * 2;
    int distance = sqrt((x * x) + (y * y));
    if(distance <= 1) 
      number_in_circle++;
  }
  return number_in_circle;
}  