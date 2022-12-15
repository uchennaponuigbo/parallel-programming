#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h> 

void Read_n(int* n_p, int* local_n_p, int my_rank, int comm_sz, 
      MPI_Comm comm);
void Check_for_error(int local_ok, char fname[], char message[], 
      MPI_Comm comm);
void Read_data(double local_vec1[], double local_vec2[], double* scalar_p,
      int local_n, int my_rank, int comm_sz, MPI_Comm comm);
void Print_vector(double local_vec[], int local_n, int n, char title[], 
      int my_rank, MPI_Comm comm);
double Par_dot_product(double local_vec1[], double local_vec2[], 
      int local_n, MPI_Comm comm);
void Par_vector_scalar_mult(double local_vec[], double scalar, 
      double local_result[], int local_n);

int main(void) 
{
   int n, local_n;
   double *local_vec1, *local_vec2;
   double scalar = 0;
   double *local_scalar_mult1, *local_scalar_mult2;
   double dot_product;

   int comm_sz; //no. of processes in communicator
   int my_rank;  //the process making the call
   MPI_Comm comm; //collection of processes that can send messages to each other

   comm = MPI_COMM_WORLD;
   //Initilazing MPI
   MPI_Init(NULL, NULL);
   MPI_Comm_size(comm, &comm_sz);
   MPI_Comm_rank(comm, &my_rank);
   n = local_n = 0; //prevent error of unintitialized variable
   /* Print input data */
   Read_n(&n, &local_n, my_rank, comm_sz, comm);

   /* Print results */
   //printf("Process %d > n = %d, local_n = %d\n", my_rank, n, local_n);

   //allocate variables
   local_vec1 = malloc(local_n * sizeof(double));
   local_vec2 = malloc(local_n * sizeof(double));
   

   /* Compute and print dot product 
   Par_dot_product(double local_vec1[], double local_vec2[], 
      int local_n, MPI_Comm comm) 
   */
   Read_data(local_vec1, local_vec2, &scalar, local_n, my_rank, comm_sz, comm);
   //Read_Data(local_vec1, local_vec2, local_scalar_mult2, local_n, my_rank, comm_sz, comm);
   Print_vector(local_vec1, local_n, n, "Vector 1", my_rank, comm);
   Print_vector(local_vec2, local_n, n, "Vector 2", my_rank, comm);

   dot_product = Par_dot_product(local_vec1, local_vec2, local_n, comm); 
   if(my_rank == 0) //reached root/first process
   {
      printf("Dot Product: %f", dot_product);
      
      printf("\n");
   }

   /* Compute scalar multiplication and print out result */
   local_scalar_mult1 = malloc(n * sizeof(double));
   local_scalar_mult2 = malloc(n * sizeof(double));

    Par_vector_scalar_mult(local_vec1, scalar, local_scalar_mult1, local_n);
    Par_vector_scalar_mult(local_vec2, scalar, local_scalar_mult2, local_n);

//    printf("Scalar: %lf", scalar);
   /*Par_vector_scalar_mult(double local_vec[], double scalar, 
      double local_result[], int local_n)*/
   Print_vector(local_scalar_mult1, local_n, n, "Scalar Mult Vec 1:", my_rank, comm);
   Print_vector(local_scalar_mult2, local_n, n, "Scalar Mult Vec 2:", my_rank, comm);

   free(local_scalar_mult2);
   free(local_scalar_mult1);
   free(local_vec2);
   free(local_vec1);
   
   MPI_Finalize();
   return 0;

   /*
MPI_Scatter can be used in a function that 
reads in an entire vector on process 0 but 
only sends the needed components to 
each of the other processes.   
   */

   /*
   MPI_Gather: Collect all of the components of the vector 
onto process 0, and then process 0 can 
process all of the components.
   */

/*
MPI_AllGather
Concatenates the contents of each 
process’ send_buf_p and stores this in 
each process’ recv_buf_p. 

recv_count is the amount of data 
being received from each process.
*/
}

/*-------------------------------------------------------------------*/
void Check_for_error(
                int       local_ok   /* in */, 
                char      fname[]    /* in */,
                char      message[]  /* in */, 
                MPI_Comm  comm       /* in */) 
                {
   int ok;
   
   MPI_Allreduce(&local_ok, &ok, 1, MPI_INT, MPI_MIN, comm);
   if (ok == 0) 
   {
      int my_rank;
      MPI_Comm_rank(comm, &my_rank);
      if (my_rank == 0) 
      {
         fprintf(stderr, "Process %d > In %s, %s\n", my_rank, fname, 
               message);
         fflush(stderr);
      }
      MPI_Finalize();
      exit(-1);
   }
}  /* Check_for_error */


/* Get the input of n: size of the vectors, and then calculate local_n according to comm_sz and n */
/* where local_n is the number of elements each process obtains 


*/
/*-------------------------------------------------------------------*/
void Read_n(int* n_p, int* local_n_p, int my_rank, int comm_sz, MPI_Comm comm) 
{
      if(my_rank == 0)
      {
            printf("Input the size n of the vectors.\n");
            fflush(stdout); //sometimes hangs program, so  I added this
            scanf("%d", n_p);
      }
            
      Check_for_error(local_ok, "Read_n", "n should be >= 0 and evenly divisible by comm_sz", comm);
      //local_n is the number of elements each process has
      *local_n_p = *n_p / comm_sz;

      MPI_Bcast(n_p, 1, MPI_INT, 0, comm);
      MPI_Bcast(local_n_p , 1, MPI_INT, 0, comm);
}  /* Read_n */


/* local_vec1 and local_vec2 are the two local vectors of size local_n which the process pertains */
/* process 0 will take the input of the scalar, the two vectors a and b */
/* process 0 will scatter the two vectors a and b across all processes */
/*-------------------------------------------------------------------*/
void Read_data(double local_vec1[], double local_vec2[], double* scalar_p,
      int local_n, int my_rank, int comm_sz, MPI_Comm comm) 
{
   double* a = NULL;
   double *b = NULL;
   int i;
   if (my_rank == 0)
   {
      printf("Input the scalar\n");
      scanf("%lf", scalar_p);
   }
   
   MPI_Bcast(scalar_p, 1, MPI_DOUBLE, 0, comm);
   
   if (my_rank == 0)
   {
      a = malloc(local_n * comm_sz * sizeof(double));
      printf("Enter the first vector\n");
      for (i = 0; i < local_n * comm_sz; i++) 
         scanf("%lf", &a[i]);

         //would I need to call scatter on both vectors here?
      //MPI_Scatter(a, local_n, MPI_DOUBLE, a, local_n, MPI_DOUBLE, 0, comm); 
      MPI_Scatter(a, local_n, MPI_DOUBLE, local_vec1, local_n, MPI_DOUBLE, 0, comm); 

      // free(a);
      // a = NULL; //clearing a so that I could reuse it for insertion
      // a = malloc(local_n * comm_sz * sizeof(double));
      b = malloc(local_n * comm_sz * sizeof(double));
      printf("Enter the second vector\n");
      for (i = 0; i < local_n * comm_sz; i++) 
         scanf("%lf", &b[i]);
      // MPI_Scatter(a, local_n, MPI_DOUBLE, local_vec2, local_n, MPI_DOUBLE, 0, comm); 
      MPI_Scatter(a, local_n, MPI_DOUBLE, local_vec1, local_n, MPI_DOUBLE, 0, comm);
      MPI_Scatter(b, local_n, MPI_DOUBLE, local_vec2, local_n, MPI_DOUBLE, 0, comm);
      free(a);
      free(b);
   } 
   else 
   {
      //Check_for_error(local_ok, "Read_Data", "Error with Vector Allocation", comm);
      MPI_Scatter(a, local_n, MPI_DOUBLE, local_vec1, local_n, MPI_DOUBLE, 0, comm);
      MPI_Scatter(b, local_n, MPI_DOUBLE, local_vec2, local_n, MPI_DOUBLE, 0, comm);
   }
}  /* Read_data */

/* The print_vector gathers the local vectors from all processes and print the gathered vector */
/*-------------------------------------------------------------------*/
void Print_vector(double local_vec[], int local_n, int n, char title[], 
      int my_rank, MPI_Comm comm) 
{
   double* a = NULL;
   int i;
   
   if (my_rank == 0) 
   {
      a = malloc(n * sizeof(double));

      MPI_Gather(local_vec, local_n, MPI_DOUBLE, a, local_n, MPI_DOUBLE, 0, comm);
      printf("%s\n", title);
      for(i = 0; i < n; i++)
      {
            printf("%f ", a[i]);
      }
      printf("\n");
      free(a);
   } 
   else 
   {
      //Check_for_error(local_ok, "Print_vector", "Error with vector printing (can't allocate temporary vector)", comm);
      MPI_Gather(local_vec, local_n, MPI_DOUBLE, a, local_n, MPI_DOUBLE, 0, comm);
   }

}  /* Print_vector */


/* This function computes and returns the partial dot product of local_vec1 and local_vec2 */
/*-------------------------------------------------------------------*/
double Par_dot_product(double local_vec1[], double local_vec2[], 
      int local_n, MPI_Comm comm) 
{
      double local_product = 0.0;
      double dot_product = 0.0;
      for(int i = 0; i < local_n; i++)
            local_product += local_vec1[i] * local_vec2[i];

      MPI_Reduce(&local_product, &dot_product, 1 , MPI_DOUBLE, MPI_SUM, 0, comm);
      return local_product;
}  /* Par_dot_product */


/* This function gets the vector which is the scalar times local_vec, 
and put the vector into local_result */
/*-------------------------------------------------------------------*/
void Par_vector_scalar_mult(double local_vec[], double scalar, 
      double local_result[], int local_n) 
{
      for(int i = 0; i < local_n; i++)
            local_result[i] = scalar * local_vec[i];
}  /* Par_vector_scalar_mult */
