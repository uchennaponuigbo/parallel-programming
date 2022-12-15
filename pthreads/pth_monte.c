
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h> 

const int MAX_THREADS = 20000000;

void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
void Read_input(unsigned long long int* number_of_tosses);
void *Monte_Carlo(void * rank);

unsigned long long int number_of_tosses = 0;
unsigned long long int number_in_circle = 0;
pthread_mutex_t mutex;
long thread_count;
long long n;

int main(int argc, char *argv[])
{
   long       thread;
   pthread_t* thread_handles;

   Get_args(argc, argv);

   Read_input(&number_of_tosses);
   pthread_mutex_init(&mutex, NULL);

   thread_handles = malloc (thread_count * sizeof(pthread_t));

   for(thread = 0; thread < thread_count; thread++) 
   {
      pthread_create(&thread_handles[thread], NULL, Monte_Carlo, (void*) thread); 
   }

   for(thread = 0; thread < thread_count; thread++) 
   {
      pthread_join(thread_handles[thread], NULL);
   }

   double pi = 4 * ((double) number_in_circle) / ((double) number_of_tosses);
   printf("Number in Circle: %llu\n", number_in_circle);
   printf("pi ~= %f\n", pi);

   pthread_mutex_destroy(&mutex);
   free(thread_handles);
   pthread_exit(NULL);
   return 0;
}

void Read_input(unsigned long long int* number_of_tosses) 
{
    printf("Input number of tosses (int): ");
    fflush(stdout);
    scanf("%llu", number_of_tosses);
}

void *Monte_Carlo(void *rank)
{
   long my_rank = (long) rank;
   int local_number_of_tosses = number_of_tosses / thread_count;


   unsigned long long int local_number_in_circle = 0;
   double radius = 1;
   for(unsigned long long int i = 0; i < local_number_of_tosses; i++) 
   {
      double x = ((double) rand()) / ((double) RAND_MAX) * radius * 2;
      double y = ((double) rand()) / ((double) RAND_MAX) * radius * 2;
      int distance = sqrt((x * x) + (y * y));
      if(distance <= 1) 
         local_number_in_circle++;
   }

   pthread_mutex_lock(&mutex);
   number_in_circle += local_number_in_circle;
   //number_of_tosses = number_of_tosses + (local_number_of_tosses * thread_count);
   //printf("Number in circle: %llu\n", number_in_circle);
   pthread_mutex_unlock(&mutex);

   //if(my_rank == 0)
   //number_of_tosses = local_number_of_tosses * thread_count;
   printf("Number of tosses from thread %lu: %d\n", my_rank, local_number_of_tosses);
   
   
   return NULL;
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