#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h> 


void Read_size_n(int * n_ptr, int * local_n_ptr, int my_rank, int comm_sz, MPI_Comm comm);

void para_prefixSums(int local_arr[], int local_prefixSum[], int local_size_n);

//void generate_data(int arr[], int n, int lower, int upper);
void random_data(int arr[], int local_n, int upper, int lower, int my_rank, int comm_sz, MPI_Comm comm);

void print_array(int prefixSum[], int local_n, int n, char title[], int my_rank, MPI_Comm comm);
/*

*/
int main(void) 
{
   //
   int* local_arr;
   //int* local_prefixSum;
   int n, local_n = 0;

   int comm_sz; //no. of processes in communicator
   int my_rank;  //the process making the call
   MPI_Comm comm; //collection of processes that can send messages to each other

   MPI_Init(NULL, NULL);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &comm_sz);
   MPI_Comm_rank(comm, &my_rank);

   Read_size_n(&n, &local_n, my_rank, comm_sz, comm);
   local_arr = malloc(n * sizeof(int));
   //local_prefixSum = malloc(n * sizeof(int));
   random_data(local_arr, local_n, 20, 1, my_rank, comm_sz, comm);
   for(int i = 1; i < local_n; i++)
   {
      local_arr[i] += local_arr[i - 1];
   }

   int temp, offset = 0;
   MPI_Scan(&local_arr[local_n - 1], &temp, 1, MPI_INT, MPI_SUM, comm);
   offset = temp - local_arr[local_n - 1];
   local_arr[local_n - 1] = temp;
   for(int i = 0; i < local_n - 1; i++)
   {
      local_arr[i] += offset;
   }


   //print_array(local_arr, local_n, n, "Input Data (int): ", my_rank, comm);
   print_array(local_arr, local_n, n, "Prefix Sum (int): ", my_rank, comm);
   free(local_arr);
   //free(local_prefixSum);
   MPI_Finalize();
   return 0;
}

void Read_size_n(int * n_ptr, int * local_n_ptr, int my_rank, int comm_sz, MPI_Comm comm)
{
   if(my_rank == 0)
   {
      printf("Input size of array (int): ");
      fflush(stdout);
      scanf("%d", n_ptr);

      *local_n_ptr = *n_ptr / comm_sz;
   }

   MPI_Bcast(n_ptr, 1, MPI_INT, 0, comm);
   MPI_Bcast(local_n_ptr, 1, MPI_INT, 0, comm);
}

void para_prefixSums(int local_arr[], int local_prefixSum[], int local_size_n)
{
      local_prefixSum[0] = local_arr[0];
      for(int i = 1; i < local_size_n; i++)
      {
            local_prefixSum[i] = local_arr[i] + local_prefixSum[i - 1];
      }
}

void generate_data(int arr[], int n, int lower, int upper)
{
   for(int i = 0; i < n; i++)
   {
      arr[i] = (upper - lower + 1) + lower;
   }
}

void random_data(int arr[], int local_n, int upper, int lower, int my_rank, int comm_sz, MPI_Comm comm)
{
   int *temp = NULL;
   if(my_rank == 0)
   {
      temp = malloc(local_n * comm_sz * sizeof(int));
      printf("Enter the values for array (int)\n");
      for(int i = 0; i < local_n; i++)
      {
         //temp[i] = (upper - lower + 1) + lower;
         scanf("%d", &temp[i]);
      }
      MPI_Scatter(temp, local_n, MPI_INT, arr, local_n, MPI_INT, 0, comm);
      free(temp);
   }
   else
   {
      MPI_Scatter(temp, local_n, MPI_INT, arr, local_n, MPI_INT, 0, comm);
   }
}

void print_array(int prefixSum[], int local_n, int n, char title[], int my_rank, MPI_Comm comm)
{
   int *arr = NULL;
   if(my_rank == 0)
   {
      arr = malloc(n * sizeof(int));
      MPI_Gather(prefixSum, local_n, MPI_INT, arr, local_n, MPI_INT, 0, comm);
      printf("%s", title);
      for(int i = 0; i < n; i++)
         printf("%d ", arr[i]);
      printf("\n");

      free(arr);
   }
   else
   {
      //MPI_Gather(local_vec, local_n, MPI_DOUBLE, a, local_n, MPI_DOUBLE, 0, comm);
      MPI_Gather(prefixSum, local_n, MPI_INT, arr, local_n, MPI_INT, 0, comm);

   }
   
}
