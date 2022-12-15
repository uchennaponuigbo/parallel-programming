#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h> 

void Get_Input(double * a_ptr, double * b_ptr, int* n_ptr, int my_rank, int comm_sz, MPI_Comm comm); //int* local_n_ptr
void Check_for_error(int local_ok, char fname[], char message[], 
      MPI_Comm comm);

double TrapedozialRuleParallel(double left_endpoint, double right_endpoint, int trap_count, double base_length);
double f(double x);
/*
Implement a revised version of Program 3.2: Trapezoidal rule of 
calculating the definite integral ∫ f(x)dx '
( such that the users can have 
their input, note that f(x) is a function hardcoded in your program.
*/
int main(void) 
{
   int local_n = 0;
   double b, a, h; 
   double local_a, local_b;
   int n = 0; //b: end interval, a: start interval
   double local_integral, total_integral = 0.0;

   int comm_sz; //no. of processes in communicator
   int my_rank;  //the process making the call
   //int source;
   MPI_Comm comm; //collection of processes that can send messages to each other

   //Initilazing MPI
   comm = MPI_COMM_WORLD;
   MPI_Init(NULL, NULL);
   MPI_Comm_size(comm, &comm_sz);
   MPI_Comm_rank(comm, &my_rank);
   
   //input
   Get_Input(&a, &b, &n, my_rank, comm_sz, comm);
   // printf("a: %f: \n", a);
   // printf("b: %f: \n", b);
   // printf("n: %d: \n", n);

   //major problem was type mismatching. ints would round to nearest value, so h would round to 0
   //found other type mismatching in MPI functions, called MPI_INT instead of MPI_DOUBLE
   //type mismatching in method and parameter types too.

   h = (b - a) / n;
   local_n = n / comm_sz;
   // printf("a: %d: \n", a);
   // printf("b: %d: \n", b);
   //printf("Value of h: %d\n", h);
   // printf("Number of Local Trapezoids: %d\n", local_n);

   local_a = a + local_n * h;
   //printf("local a: %f \n", local_a);
   local_b = local_a + local_n * h;
   //printf("local b: %f \n", b);
   local_integral = TrapedozialRuleParallel(local_a, local_b, local_n, h);

   MPI_Reduce(&local_integral, &total_integral, 1, MPI_DOUBLE, MPI_SUM, 0, comm);

   if(my_rank == 0)
   {
      printf("Approximate Area from %f to %f: %f\n", a, b, total_integral);
   }

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

//note that f(x) is a function hardcoded in your program
double f(double x)
{ //f(x) = x^2 - x + 13
   //return (x * x) - x + 13.0;
   return x;
}

void Get_Input(double* a_ptr, double* b_ptr, int* n_ptr,
               int my_rank, int comm_sz, MPI_Comm comm)
{
   if(my_rank == 0)
   {
      printf("Enter a (double)\n");
      fflush(stdout);
      scanf("%lf", a_ptr);

      printf("Enter b (double)\n");
      fflush(stdout);
      scanf("%lf", b_ptr);

      printf("Enter n (int)\n");
      fflush(stdout);
      scanf("%d", n_ptr);

      for(int destination = 1; destination < comm_sz; destination++)
      { 
         MPI_Send(a_ptr, 1, MPI_DOUBLE, destination, 0, comm);
         MPI_Send(b_ptr, 1, MPI_DOUBLE, destination, 0, comm);
         MPI_Send(n_ptr, 1, MPI_INT, destination, 0, comm);
      }
   }
   else
   {
      MPI_Recv(a_ptr, 1, MPI_DOUBLE, 0, 0, comm, MPI_STATUS_IGNORE);
      MPI_Recv(b_ptr, 1, MPI_DOUBLE, 0, 0, comm, MPI_STATUS_IGNORE);
      MPI_Recv(n_ptr, 1, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
   }
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

double TrapedozialRuleParallel(double left_endpoint, double right_endpoint, 
                              int trap_count, double base_length)
{
   double estimated_sum = (f(left_endpoint) + f(right_endpoint)) / 2.0;
   //printf("Sum: %f", estimated_sum);
   double x;
   for(int i = 1; i <= trap_count - 1; i++)
   {
      x = left_endpoint + i * base_length;
      estimated_sum += f(x);
   }
   //printf("before Sum: %f", estimated_sum);
   estimated_sum = estimated_sum * base_length;
   //printf("base_length: %d",  base_length);
   return estimated_sum;
}