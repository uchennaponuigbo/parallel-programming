#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include "timer.h"

// const int MAX_THREADS = 1024;

long thread_count;
long long n;
double sum;
const int BufferSize = 5

sem_t sem;
pthread_mutex_t mutex;

void* Thread_sum(void* rank);

/* Only executed by main thread */
void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
double Serial_pi(long long n);

//https://shivammitra.com/c/producer-consumer-problem-in-c/#
//https://www.chegg.com/homework-help/questions-and-answers/although-producer-consumer-synchronization-easy-implement-semaphores-s-also-possible-imple-q69943478


int main(int argc, char* argv[]) 
{
    
    pthread_t producer[5], consumer[5];
    pthread_mutex_init(&mutex, NULL);
   //  sem_init(&empty, 0, BufferSize);
   //  sem_init(&full, 0, 1);

    int a[5] = {1, 2, 3, 4, 5};

    for(int i = 0; i < 5; i++) 
    {
        pthread_create(&producer[i], NULL, (void *)producer, (void *)&a[i]);
    }
    for(int i = 0; i < 5; i++) 
    {
        pthread_create(&consumer[i], NULL, (void *)consumer, (void *)&a[i]);
    }

    for(int i = 0; i < 5; i++) 
    {
        pthread_join(producer[i], NULL);
    }
    for(int i = 0; i < 5; i++) 
    {
        pthread_join(consumer[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
   //  sem_destroy(&empty);
   //  sem_destroy(&full);



   return 0;
}  /* main */

/*------------------------------------------------------------------*/
void *producer(void *p_rank)
{   
    int item;
    for(int i = 0; i < MaxItems; i++) 
    {
        item = rand();
        //sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        buffer[in] = item;
        printf("Producer %d: Insert Item %d at %d\n", *((int *)p_rank), buffer[in], in);
        in = (in + 1) % BufferSize;
        pthread_mutex_unlock(&mutex);
        //sem_post(&full);
    }
}
void *consumer(void *c_rank)
{   
    for(int i = 0; i < MaxItems; i++) 
    {
        //sem_wait(&full);
        pthread_mutex_lock(&mutex);
        int item = buffer[out];
        printf("Consumer %d: Remove Item %d from %d\n", *((int *)c_rank), item, out);
        out = (out + 1) % BufferSize;
        pthread_mutex_unlock(&mutex);
        //sem_post(&empty);
    }
}

/*------------------------------------------------------------------
 * Function:    Get_args
 * Purpose:     Get the command line args
 * In args:     argc, argv
 * Globals out: thread_count, n
 */
void Get_args(int argc, char* argv[]) {
   if (argc != 3) Usage(argv[0]);
   thread_count = strtol(argv[1], NULL, 10);  
   if (thread_count <= 0 || thread_count > MAX_THREADS) Usage(argv[0]);
   n = strtoll(argv[2], NULL, 10);
   if (n <= 0) Usage(argv[0]);
}  /* Get_args */


/*------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Print a message explaining how to run the program
 * In arg:    prog_name
 */
void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <number of threads> <n>\n", prog_name);
   fprintf(stderr, "   n is the number of terms and should be >= 1\n");
   fprintf(stderr, "   n should be evenly divisible by the number of threads\n");
   exit(0);
}  /* Usage */
