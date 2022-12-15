#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h> 


void Check_for_error(int local_ok, char fname[], char message[], 
      MPI_Comm comm);


void Get_input(int* bin_count_ptr, double* min_meas_ptr, double* max_meas_ptr, int* data_count_ptr,
                  int* local_data_count_ptr, int my_rank, int comm_sz, MPI_Comm comm);

void Gen_random_data(double local_data[], int local_data_count, int data_count, double min_meas,
                      double max_meas, int my_rank, MPI_Comm comm);

void Set_bins(double bin_maxes[], int loc_bin_counts[], double min_meas, 
                double max_meas, int bin_count, int my_rank, MPI_Comm comm); 

void Find_bins(int bin_counts[], double local_data[], int loc_bin_cts[], int local_data_count, double bin_maxes[],
                  int bin_count, double min_meas, MPI_Comm comm);

int Which_bin(double local_data_value_in_arr, double bin_maxes[], int bin_count, double min_meas);

void Print_hist(double data[], int data_count, double bin_maxes[], int bin_counts[], int bin_count, double min_meas);


/*
Use MPI to implement the histogram program discussed in Chapter 2. Have 
process 0 read in the input data and distribute it among the processes. Also 
have process 0 printout the program. Note that the measurements are 
randomly generated based on data_count, a, and b input by the user
*/

int main(void) 
{
   int comm_sz; //no. of processes in communicator
   int my_rank;  //the process making the call
   MPI_Comm comm; //collection of processes that can send messages to each other

   //Initilazing MPI
   comm = MPI_COMM_WORLD;
   MPI_Init(NULL, NULL);
   MPI_Comm_size(comm, &comm_sz);
   MPI_Comm_rank(comm, &my_rank);

   int bin_count = 0;
   double min_meas, max_meas = 0.0;
   double* bin_maxes;
   int* bin_counts;
   int* local_bin_counts;
   int data_count = 0;
   int local_data_count = 0;
   double* data;
   double* local_data;

   //input
   Get_input(&bin_count, &min_meas, &max_meas, &data_count,
            &local_data_count, my_rank, comm_sz, comm);

   bin_maxes = malloc(bin_count * sizeof(double));
   bin_counts = malloc(bin_count * sizeof(int));
   local_bin_counts = malloc(bin_count * sizeof(int));
   data = malloc(data_count * sizeof(double));
   local_data = malloc(local_data_count * sizeof(double));

  //  printf("bin count %d", bin_count);
  //  printf("min and max measurements %f  %f", min_meas, max_meas);
  //  printf("local bin counts %n", local_bin_counts);
  //  printf("data count %d", data_count);
  //  printf("local data count %d", local_data_count);

   //
   Set_bins(bin_maxes, local_bin_counts, min_meas, max_meas, bin_count, my_rank, comm);
   Gen_random_data(local_data, local_data_count, data_count, min_meas, max_meas, my_rank, comm);
   Find_bins(bin_counts, local_data, local_bin_counts, local_data_count, bin_maxes, bin_count, min_meas, comm);

   MPI_Gather(local_data, local_data_count, MPI_DOUBLE, data, local_data_count, MPI_DOUBLE, 0, comm);
  //forgot to create and pass local bin count around
   //MPI_Gather(local_bin_counts, local_bin_count, MPI_INT, bin_counts, local_bin_count, MPI_INT, 0, comm);

  if(my_rank == 0)
  {
    Print_hist(data, data_count, bin_maxes, bin_counts, bin_count, min_meas);
  }
  // else
  // {
  //   //MPI_Gather(local_vec, local_n, MPI_DOUBLE, a, local_n, MPI_DOUBLE, 0, comm);
  //   MPI_Gather(local_data, local_data_count, MPI_DOUBLE, data, local_data_count, MPI_DOUBLE, 0, comm);
  //   MPI_Gather(local_bin_counts, local_data_count, MPI_INT, bin_count, local_data_count, MPI_INT, 0, comm);
  //   //MPI_Scatter(data, local_data_count, MPI_DOUBLE, local_data, local_data_count, MPI_DOUBLE, 0, comm);
  // }
    

   free(bin_maxes);
   free(bin_counts);
   free(local_bin_counts);
   free(data);
   free(local_data);

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


void Get_input(int* bin_count_ptr, double* min_meas_ptr, double* max_meas_ptr, int* data_count_ptr,
                  int* local_data_count_ptr, int my_rank, int comm_sz, MPI_Comm comm) 
{
  if(my_rank == 0) 
  {
    printf("Input Number of bins (int): ");
    fflush(stdout);
    scanf("%d", bin_count_ptr);

    printf("Input Minimum value (double): ");
    fflush(stdout);
    scanf("%lf", min_meas_ptr);

    printf("Input Maximum value (double): ");
    fflush(stdout);
    scanf("%lf", max_meas_ptr);
    
    printf("Input Number of values (int): ");
    fflush(stdout);
    scanf("%d", data_count_ptr);

    // Make sure data_count is a multiple of comm_sz
    *local_data_count_ptr = *data_count_ptr / comm_sz;
    *data_count_ptr = *local_data_count_ptr * comm_sz;
    printf("\n");
  }

  MPI_Bcast(bin_count_ptr, 1, MPI_INT, 0, comm);
  MPI_Bcast(min_meas_ptr, 1, MPI_DOUBLE, 0, comm);
  MPI_Bcast(max_meas_ptr, 1, MPI_DOUBLE, 0, comm);
  MPI_Bcast(data_count_ptr, 1, MPI_INT, 0, comm);
  MPI_Bcast(local_data_count_ptr, 1, MPI_INT, 0, comm);
} 

void Set_bins(double bin_maxes[], int loc_bin_counts[], double min_meas, 
                double max_meas, int bin_count, int my_rank, MPI_Comm comm) 
{
  double range = max_meas - min_meas;
  double interval = range / bin_count;

  //printf("range: %f\n", range);
  //printf("interval: %f\n", interval);
  //printf("min_meas from Set_bins: %f\n", min_meas);

  for(int i = 0; i < bin_count; i++) 
  {
    bin_maxes[i] = interval * (double)(i + 1) + min_meas;
    //printf("bin_maxes %f at %d\n", bin_maxes[i], i);
    loc_bin_counts[i] = 0;
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


void Find_bins(int bin_counts[], double local_data[], int loc_bin_counts[], int local_data_count, double bin_maxes[],
                  int bin_count, double min_meas, MPI_Comm comm)
{
/* Find out the appropriate bin for each data in local_data and increase the number of data in this bin  */
//printf("min_meas from Find_bins: %f\n", min_meas);
  for(int i = 0; i < local_data_count; i++) 
  {
    int bin = Which_bin(local_data[i], bin_maxes, bin_count, min_meas);  
    //printf("bin maxes %f\n", bin_maxes[i]);
    loc_bin_counts[bin]++;
    //printf("local bin counts %d\n", loc_bin_counts[i]);
    
  }
} 

/* Find out the appropriate bin for each data */
int Which_bin(double local_data_value_in_arr, double bin_maxes[], int bin_count, double min_meas) 
{
  int i;
  for(i = 0; i < bin_count - 1; i++) 
  {
    if(local_data_value_in_arr <= bin_maxes[i]) 
        break;
  }
  return i;
} 

void Gen_random_data(double local_data[], int local_data_count, int data_count, double min_meas,
                      double max_meas, int my_rank, MPI_Comm comm)
{
  double* data;
  if(my_rank == 0) 
  {
    double range = max_meas - min_meas;
    data = malloc(data_count * sizeof(double));

    for(int i = 0; i < data_count; i++) 
    {
      data[i] = (double) rand() / (double) RAND_MAX * range + min_meas; //some rng I found online
      //printf("%f ", data[i]); //random data is being generated
    }
    printf("\n");
  }
  MPI_Scatter(data, local_data_count, MPI_DOUBLE, local_data, local_data_count, MPI_DOUBLE, 0, comm);

  if(my_rank == 0) 
    free(data);
} 

void Print_hist(double data[], int data_count, double bin_maxes[], int bin_counts[], int bin_count, double min_meas) 
{
  int max = 0;
  int i;

  //printf("bin count: %d\n", bin_count);
  // get max count
  printf("Data\n");
  for(i = 0; i < data_count; i++)
  {
      printf("%f ", data[i]);
  }

  printf("\n\n");
  for(i = 0; i < bin_count; i++) 
  {
    //printf("bin counts %d\n", bin_counts[i]);
    if(bin_counts[i] > max)
      max = bin_counts[i];
  }

  for(i = 0; i < bin_count - 1; i++) 
  {
    //printf("Bin Range %f to %f", bin_maxes[i], bin_maxes[i + 1]);
    printf("Bin Range: %10.3f |", bin_maxes[i]);

    printf("\nCount: %d\n", bin_counts[i]);
    //printf("Measurements: %f\n", min_meas); //the number of values in each bin, the measurements
  }
}