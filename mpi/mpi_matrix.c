#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void Read_cols_rows(int* m_ptr, int* local_m_ptr, int* n_ptr, int* local_n_ptr, int my_rank, int comm_sz, MPI_Comm comm);
void Read_matrix(char message[], double local_A[], int m, int local_m, int n, int my_rank, MPI_Comm comm);
void Read_vector(char message[], double local_vec[], int n, int local_n, int my_rank, MPI_Comm comm);
void Print_matrix(char message[], double local_A[], int m, int local_m, int n, int my_rank, MPI_Comm comm);
void Print_vector(char message[], double local_vec[], int n,int local_n, int my_rank, MPI_Comm comm);
void Mat_vect_mult(double local_A[], double local_x[], double local_y[], int local_m, int n, int local_n, MPI_Comm comm);

int main(void) 
{
   double* local_A;
   double* local_x;
   double* local_y;
   int m, local_m, n, local_n = 0;
   int my_rank, comm_sz;
   MPI_Comm comm;

   MPI_Init(NULL, NULL);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &comm_sz);
   MPI_Comm_rank(comm, &my_rank);

   Read_cols_rows(&m, &local_m, &n, &local_n, my_rank, comm_sz, comm);

   local_A = malloc(local_m * n * sizeof(double));
   local_x = malloc(local_n * sizeof(double));
   local_y = malloc(local_m * sizeof(double));

   Read_matrix("A", local_A, m, local_m, n, my_rank, comm);

   Print_matrix("A", local_A, m, local_m, n, my_rank, comm);

   Read_vector("x", local_x, n, local_n, my_rank, comm);

   Print_vector("x", local_x, n, local_n, my_rank, comm);

   Mat_vect_mult(local_A, local_x, local_y, local_m, n, local_n, comm);

   Print_vector("y", local_y, m, local_m, my_rank, comm);

   free(local_A);
   free(local_x);
   free(local_y);
   MPI_Finalize();
   return 0;
} 

void Read_cols_rows(int* m_ptr, int* local_m_ptr, int* n_ptr, int* local_n_ptr, int my_rank, int comm_sz, MPI_Comm comm) 
{
   if (my_rank == 0) 
   {
      printf("Enter the number of rows (int)\n");
      fflush(stdout);
      scanf("%d", m_ptr);

      printf("Enter the number of columns (int)\n");
      fflush(stdout);
      scanf("%d", n_ptr);
   }
   MPI_Bcast(m_ptr, 1, MPI_INT, 0, comm);
   MPI_Bcast(n_ptr, 1, MPI_INT, 0, comm);

   *local_m_ptr = *m_ptr / comm_sz;
   *local_n_ptr = *n_ptr / comm_sz;
}

void Read_matrix(char message[], double local_A[], int m, int local_m, int n, int my_rank, MPI_Comm comm) 
{
   double* A = NULL;

   if (my_rank == 0) 
   {
      A = malloc(m * n * sizeof(double));
      printf("Enter %d values for the matrix %s\n", m * n, message);
      fflush(stdout);

      for (int i = 0; i < m; i++)
         for (int j = 0; j < n; j++)
            scanf("%lf", &A[i * n + j]);

      MPI_Scatter(A, local_m * n, MPI_DOUBLE, local_A, local_m * n, MPI_DOUBLE, 0, comm);
      free(A);
   } 
   else 
   {
      MPI_Scatter(A, local_m * n, MPI_DOUBLE, local_A, local_m * n, MPI_DOUBLE, 0, comm);
   }
}

void Read_vector(char message[], double local_vec[], int n, int local_n, int my_rank, MPI_Comm comm) 
{
   double* vec = NULL;

   if (my_rank == 0) 
   {
      vec = malloc(n * sizeof(double));
      printf("Enter %d values for the vector %s (double)\n", n, message);
      fflush(stdout);

      for (int i = 0; i < n; i++)
         scanf("%lf", &vec[i]);

      MPI_Scatter(vec, local_n, MPI_DOUBLE, local_vec, local_n, MPI_DOUBLE, 0, comm);
      free(vec);
   } 
   else 
   {
      MPI_Scatter(vec, local_n, MPI_DOUBLE, local_vec, local_n, MPI_DOUBLE, 0, comm);
   }
}

void Print_matrix(char message[], double local_A[], int m, int local_m, int n, int my_rank, MPI_Comm comm) 
{
   double* A = NULL;

   if (my_rank == 0) 
   {
      A = malloc(m * n * sizeof(double));
      MPI_Gather(local_A, local_m * n, MPI_DOUBLE, A, local_m * n, MPI_DOUBLE, 0, comm);

      printf("\n%d x %d Matrix %s\n", n, m, message);
      for (int i = 0; i < m; i++) 
      {
         for (int j = 0; j < n; j++)
            printf("%f ", A[i * n + j]);
         printf("\n");
      }
      printf("\n");
      free(A);
   } 
   else 
   {
      MPI_Gather(local_A, local_m * n, MPI_DOUBLE, A, local_m * n, MPI_DOUBLE, 0, comm);
   }
}

void Print_vector(char message[], double local_vec[], int n, int local_n, int my_rank, MPI_Comm comm) 
{
   double* vec = NULL;

   if (my_rank == 0) 
   {
      vec = malloc(n * sizeof(double));

      MPI_Gather(local_vec, local_n, MPI_DOUBLE, vec, local_n, MPI_DOUBLE, 0, comm);
      printf("\nVector %s = \n[", message);
      for (int i = 0; i < n; i++)
         printf("%f ", vec[i]);
      printf("]\n");
      free(vec);
   }  
   else 
   {
      MPI_Gather(local_vec, local_n, MPI_DOUBLE, vec, local_n, MPI_DOUBLE, 0, comm);
   }
}

void Mat_vect_mult(double local_A[], double local_x[], double local_y[], int local_m, int n, int local_n, MPI_Comm comm) 
{
   double* x;

   x = malloc(n * sizeof(double));
   MPI_Allgather(local_x, local_n, MPI_DOUBLE, x, local_n, MPI_DOUBLE, comm);

   for (int local_i = 0; local_i < local_m; local_i++) 
   {
      local_y[local_i] = 0.0;
      for (int j = 0; j < n; j++)
         local_y[local_i] += local_A[local_i * n + j] * x[j];
   }
   free(x);
}
