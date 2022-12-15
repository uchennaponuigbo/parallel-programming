#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <math.h>

const int MAX_THREADS = 1024;

void Usage(char prog_name[]);

void Get_input(int* bin_count_p, float* min_meas_p, float* max_meas_p, int* data_count_p);
void Get_args(int argc, char* argv[]);
void Gen_data(float min_meas, float max_meas, float data[], int data_count);
void Gen_bins(float min_meas, float max_meas, float bin_maxes[], int bin_counts[], int bin_count);
int Which_bin(float data, float bin_maxes[], int bin_count, float min_meas);
void Print_hist(float bin_maxes[], int bin_counts[], int bin_count, float min_meas);

void* thread_do_work(void*);

long long n;
int* local_bins;           
float* data;               
int* bin_counts;           
float min_meas, max_meas;  
float* bin_maxes;          
int bin_count;             
int thread_count;         
int data_count;            
pthread_t* threads;        

int counter = 0;
pthread_mutex_t mutex;
pthread_cond_t cond_var;

int main(int argc, char* argv[]) 
{  
   Get_args(argc, argv);
   Get_input(&bin_count, &min_meas, &max_meas, &data_count);

   bin_maxes = (float *)malloc(bin_count * sizeof(float));
   bin_counts = (int *)malloc(bin_count * sizeof(int));
   data = (float *)malloc(data_count * sizeof(float));
   threads = (pthread_t *)malloc(thread_count * sizeof(pthread_t));
   local_bins = (int *)malloc(bin_count * thread_count * sizeof(int));

   pthread_mutex_init(&mutex, NULL);
   pthread_cond_init(&cond_var, NULL);
   pthread_attr_t attr;
   pthread_attr_init(&attr); //look up what this is
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

   Gen_data(min_meas, max_meas, data, data_count);

   Gen_bins(min_meas, max_meas, bin_maxes, bin_counts, bin_count);

   for (int i = 0; i < thread_count; i++)
   {
      pthread_create(&threads[i], &attr, thread_do_work, (void *) (intptr_t) i);
   }

   for (int i = 0; i < thread_count; i++)
   {
      pthread_join(threads[i], NULL);
   }

   // printf("bin_counts in inputted ranges: ");
   // for (int i = 0; i < bin_count; i++)
   //    printf("%d ", bin_counts[i]);
   // printf("\n");

   Print_hist(bin_maxes, bin_counts, bin_count, min_meas);

   free(data);
   free(bin_maxes);
   free(bin_counts);
   free(threads);
   free(local_bins);
   return 0;

}

void Get_input(int* bin_count_p, float* min_meas_p, float* max_meas_p, int* data_count_p)
{
   printf("Input data\n");
   printf("Bin count: ");
   fflush(stdout);
   scanf("%d", bin_count_p);

   printf("Minimum measurement: ");
   fflush(stdout);
   scanf("%f", min_meas_p);

   printf("Maximum measurement: ");
   fflush(stdout);
   scanf("%f", max_meas_p);

   printf("Data Count: ");
   fflush(stdout);
   scanf("%d", data_count_p);

}

void Get_args(int argc, char* argv[]) 
{
   if (argc != 3) 
      Usage(argv[0]);
   thread_count = strtol(argv[1], NULL, 10);  
   if (thread_count <= 0 || thread_count > MAX_THREADS) 
      Usage(argv[0]);
   n = strtoll(argv[2], NULL, 10);
   if (n <= 0) 
      Usage(argv[0]);
}  /* Get_args */

void Usage(char* prog_name) 
{
   fprintf(stderr, "usage: %s <number of threads> <n>\n", prog_name);
   fprintf(stderr, "   n is the number of terms and should be >= 1\n");
   fprintf(stderr, "   n should be evenly divisible by the number of threads\n");
   exit(0);
}  /* Usage */

void Gen_data(float min_meas, float max_meas, float data[], int data_count) 
{
   srandom(0);
   for (int i = 0; i < data_count; i++)
      data[i] = min_meas + (max_meas - min_meas) * random()/ ((double) RAND_MAX);

   printf("data = ");
   for (int i = 0; i < data_count; i++)
      printf("%4.3f ", data[i]);
   printf("\n");
} 

void Gen_bins(float min_meas, float max_meas, float bin_maxes[], int bin_counts[], int bin_count) 
{
   float bin_width = (max_meas - min_meas) / bin_count;

   for (int i = 0; i < bin_count; i++) 
   {
      bin_maxes[i] = min_meas + (i + 1) * bin_width;
      bin_counts[i] = 0;
   }

   printf("bin_maxes = ");
   for (int i = 0; i < bin_count; i++)
      printf("%4.3f ", bin_maxes[i]);
   printf("\n");
} 

int Which_bin(float data, float bin_maxes[], int bin_count, float min_meas) 
{
   int bottom = 0;
   int top = bin_count - 1;
   int mid;
   float bin_max, bin_min;

   while (bottom <= top) 
   {
      mid = (bottom + top) / 2;
      bin_max = bin_maxes[mid];
      bin_min = (mid == 0) ? min_meas : bin_maxes[mid - 1];
      if (data >= bin_max) 
         bottom = mid + 1;
      else if (data < bin_min)
         top = mid - 1;
      else
         return mid;
   }

   fprintf(stderr, "Data = %f doesn't belong to a bin!\n", data);
   fprintf(stderr, "Quitting\n");
   exit(-1);
}

void Print_hist(float bin_maxes[], int bin_counts[], int bin_count, float min_meas) 
{
   float bin_max, bin_min;

   for (int i = 0; i < bin_count; i++) 
   {
      bin_max = bin_maxes[i];
      bin_min = (i == 0) ? min_meas : bin_maxes[i - 1];
      printf("%.3f-%.3f:\t", bin_min, bin_max);
      int count;
      for (count = 0; count < bin_counts[i]; count++) { } //counting number of data in bins
      printf("%d", count);
      printf("\n");
   }
}  /* Print_histo */

void* thread_do_work(void* thread_data)
{
   //set up data
   int id = (int)(intptr_t) thread_data;
   int num_elements = floor(data_count / thread_count);
   if(id == thread_count - 1) //last thread
   {
      num_elements = floor(data_count / thread_count) + (data_count % thread_count);
   }

   int start_n, stop_n;
   start_n = floor(data_count / thread_count) * id;
   stop_n = start_n + num_elements;

   //find out what bins they go in and put them there
   int k, bin;
   for(k = start_n; k < stop_n; k++)
   {
       bin = Which_bin(data[k], bin_maxes, bin_count, min_meas);
       local_bins[(id * bin_count)+bin]++;
   }

   pthread_mutex_lock(&mutex);
   counter++;
   if(counter == thread_count) //if this is the last thread to join the wait
   {
      counter = 0;
      pthread_cond_broadcast(&cond_var); //wake everyone up
   }
   else 
   {
      while(pthread_cond_wait(&cond_var, &mutex) != 0); //wait until condition
   }
   pthread_mutex_unlock(&mutex);

   int sum;
   if(bin_count > thread_count) //more bins than threads
   {
      int bins_per_thread = floor(bin_count/thread_count);
      if (id < (bin_count % thread_count)) 
      {
         bins_per_thread++;
      }

      for(int m = id; m < bin_count; m += thread_count)
      {
         sum = 0;
         for(k = 0; k < thread_count; k++)
         {
            sum += local_bins[m + (k * bin_count)];
         }
         bin_counts[m] = sum;
      }
   }
   else
   {
      if(id < bin_count)
      {
         sum = 0;
         for(k = 0; k < thread_count; k++)
         {
            sum += local_bins[id + (k * bin_count)];
         }
         bin_counts[id] = sum;
      }
   }
   return NULL;
}
