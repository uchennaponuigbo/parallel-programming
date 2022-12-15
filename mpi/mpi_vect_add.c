#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h> 

void Read_size_n(int * n_ptr, int * local_n_ptr, int my_rank, int comm_sz, MPI_Comm comm);

void Read_vector( double local_x_vec[], double local_y_vec[], 
                  int* size_n_ptr, int* local_size_n_ptr, int my_rank, int comm_sz, MPI_Comm comm, MPI_Datatype type);

void Add_vector(double local_x_vec[], double local_y_vec[], double local_sum_vec[], int local_size_n); //, MPI_Comm comm, MPI_Datatype type

void Print_vector(double local_vector[], int local_n, int size_n, 
                  char title[], int my_rank, MPI_Comm comm, MPI_Datatype type);

/*
Give a parallel program for adding two vectors x and y. Process 0 lets the user input the order, x 
and y, and then print x, y and x+y. Your program should have Read_vector and Print_vector 
functions
*/
int main(void) 
{
   double* local_x, *local_y, *local_sum;
   int n, local_n = 0;
   
   int comm_sz; //no. of processes in communicator
   int my_rank;  //the process making the call
   MPI_Comm comm; //collection of processes that can send messages to each other

   MPI_Datatype type; 
   MPI_Init(NULL, NULL);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &comm_sz);
   MPI_Comm_rank(comm, &my_rank);

   Read_size_n(&n, &local_n, my_rank, comm_sz, comm);
   local_x = malloc(n * sizeof(double));
   local_y = malloc(n * sizeof(double));
   local_sum = malloc(n * sizeof(double));
   /*For Scatter and gather the amount of data you are inputting is local_n and MPI_DOUBLE, but after using your new type, it should be 1 and yourType
So for MPI_Type_contiguous(local_n, MPI_DOUBLE, (insert name of your type))*/
   MPI_Type_contiguous(local_n, MPI_DOUBLE, &type);
   MPI_Type_commit(&type);

   /*So in the MPI Gather and Scatter functions, when you input the 
   amount/count and the type, replace those values with 1 and then your custom type*/
   Read_vector(local_x, local_y, &n, &local_n, my_rank, comm_sz, comm, type);
   Print_vector(local_x, local_n, n, "Vector x: ", my_rank, comm, type);
   Print_vector(local_y, local_n, n, "Vector y: ", my_rank, comm, type); 
   Add_vector(local_x, local_y, local_sum, local_n);
   Print_vector(local_sum, local_n, n, "Vector sum: ", my_rank, comm, type); 

   free(local_x);
   free(local_y);
   free(local_sum);
   free(type);

   MPI_Finalize();

   return 0;
}

void Read_size_n(int * n_ptr, int * local_n_ptr, int my_rank, int comm_sz, MPI_Comm comm)
{
      if(my_rank == 0)
      {
            printf("Enter the size of the vector (int)\n");
            fflush(stdout);
            scanf("%d", n_ptr);
            *local_n_ptr = *n_ptr / comm_sz;
      }
      MPI_Bcast(n_ptr, 1, MPI_INT, 0, comm);     
      MPI_Bcast(local_n_ptr, 1, MPI_INT, 0, comm);
}

void Read_vector( double local_x_vec[], double local_y_vec[], 
                  int* size_n_ptr, int* local_size_n_ptr, int my_rank, int comm_sz, MPI_Comm comm, MPI_Datatype type)
{
      double *x = NULL;
      double *y = NULL;
      
      if(my_rank == 0)
      {
            x = malloc(*local_size_n_ptr * comm_sz * sizeof(double));
            printf("Input %d elements into the vectors (double)\n", *size_n_ptr);
            printf("Vector x: ");
            fflush(stdout);
            for (int i = 0; i < *local_size_n_ptr * comm_sz; i++) 
                  scanf("%lf", &x[i]);

            y = malloc(*local_size_n_ptr * comm_sz * sizeof(double));
            printf("Vector y: ");
            fflush(stdout);
            for (int j = 0; j < *local_size_n_ptr * comm_sz; j++) 
                  scanf("%lf", &y[j]);

            MPI_Scatter(x, 1, type, local_x_vec, 1, type, 0, comm);
            MPI_Scatter(y, 1, type, local_y_vec, 1, type, 0, comm);
            free(x);
            free(y);
      }
      else
      {
            MPI_Scatter(x, 1, type, local_x_vec, 1, type, 0, comm);
            MPI_Scatter(y, 1, type, local_y_vec, 1, type, 0, comm);
      }

      
} 

void Add_vector(double local_x_vec[], double local_y_vec[], double local_sum_vec[], int local_size_n) //, MPI_Comm comm, MPI_Datatype type
{
      //MPI_Reduce(&local_integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, 0, comm);
      for(int i = 0; i < local_size_n; i++)
      {
            local_sum_vec[i] = local_x_vec[i] + local_y_vec[i];
      }
}

void Print_vector(double local_vector[], int local_n, int size_n, 
                  char title[], int my_rank, MPI_Comm comm, MPI_Datatype type)
{
      double *vec = NULL;
      if(my_rank == 0)
      {
            vec = malloc(size_n * sizeof(double));
            MPI_Gather(local_vector, 1, type, vec, 1, type, 0, comm);

            printf("%s", title);
            for(int i = 0; i < size_n; i++)
                  printf("%.2f ", vec[i]);
            printf("\n");

            free(vec);
      }
      else
      {
            MPI_Gather(local_vector, 1, type, vec, 1, type, 0, comm);
      }
}
