#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

const int MAX_THREADS = 1024;
long thread_count;

void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
void Read_input(double * a_ptr, double * b_ptr, int* n_ptr);
double f(double x);

void* busy_wait_trapezoid(void* rank);
void* mutex_trapezoid(void* rank);
void* sem_trapezoid(void* rank);

double a = 0;         // left endpoint
double b = 0;         // right endpoint
int n = 0;       // number of trapezoids
double h = 0;         // height of trapezoids

double mutex_sum = 0;
double sem_sum = 0;
double busy_sum = 0;
pthread_mutex_t mutex;
sem_t semaphore;
int wait_flag = 0;

void mutex_main();
void sem_main();
void busy_wait_main();

int Get_left(long rank, long thread_n, double endpoint);
int Get_right(long rank, long thread_n, double endpoint);  

int main(int argc, char *argv[])
{
  Get_args(argc, argv);
  Read_input(&a, &b, &n);

  mutex_main();
  sem_main();
  busy_wait_main();

  return 0;
}

void* busy_wait_trapezoid(void* rank)
{
  long my_rank = (long)rank;
  double local_sum = 0.0;
  int special_case = (int)my_rank;

  int local_left_pt = Get_left(my_rank, thread_count, n);//my_rank * n / thread_count;//a + local_n * h;//
  int local_right_pt = Get_right(my_rank, thread_count, n);//local_a + local_n * h;

  if(special_case == 1)
  {
    local_sum += (f(a) + f(b)) / 2.0;
  }
//
  for(int i = local_left_pt; i <= local_right_pt; i++)
  {
    local_sum += f(a + (i * h));
  }
  local_sum = local_sum * h;

  while(wait_flag != my_rank);
  busy_sum += local_sum;
  //printf("Busy Sum: %.15f\n", busy_sum);
  wait_flag = (wait_flag + 1) % thread_count;

  return NULL;
}


void* mutex_trapezoid(void* rank)
{ 
    long my_rank = (long)rank;
    double local_sum = 0.0;
    int special_case = (int)my_rank;
    //int local_n = n / thread_count;
    int local_left_pt = Get_left(my_rank, thread_count, a);//my_rank * n / thread_count;//a + local_n * h;//
    int local_right_pt = Get_right(my_rank, thread_count, b);//local_a + local_n * h;

    // printf("local Left %d\nlocal right %d\n", local_left_pt, local_right_pt);
    // printf("local n: %d\n", local_n);
    if(special_case == 1)
    {
      local_sum += (f(a) + f(b)) / 2.0;
    }
    //local_sum += (f(a) + f(b)) / 2.0;
  //
    //local_sum += (f(local_left_pt) + f(local_right_pt)) / 2.0;
    //for(int i = 1; i <= local_n - 1; i++)
    for(int i = local_left_pt; i <= local_right_pt; i++)
    {//local_left_pt
      local_sum += f(a + (i * h));
    }
    local_sum = local_sum * h;

    pthread_mutex_lock(&mutex);
    mutex_sum += local_sum;
    //printf("Mutex Sum: %.15f\n", mutex_sum);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void* sem_trapezoid(void *rank)
{
    long my_rank = (long)rank;
    double local_sum = 0.0;
    int special_case = (int)my_rank;
    int local_left_pt = Get_left(my_rank, thread_count, n);//my_rank * n / thread_count;//a + local_n * h;//
    int local_right_pt = Get_right(my_rank, thread_count, n);//local_a + local_n * h;

    if(special_case == 1)
    {
      local_sum += (f(a) + f(b)) / 2.0;
    }

    for(int i = local_left_pt; i <= local_right_pt; i++)
    {
      local_sum += f(a + (i * h));
    }
    local_sum = local_sum * h;

    sem_wait(&semaphore);
    sem_sum += local_sum;
    //printf("Sem Sum: %.15f\n", sem_sum);
    sem_post(&semaphore);

    return NULL;
}

void mutex_main()
{
    long       thread;
    pthread_t* thread_handles;

    pthread_mutex_init(&mutex, NULL);
    thread_handles = malloc(thread_count * sizeof(pthread_t));

    for(thread = 0; thread < thread_count; thread++)
      pthread_create(&thread_handles[thread], NULL, mutex_trapezoid, (void*) thread );

    for( thread = 0; thread < thread_count; thread++)
      pthread_join( thread_handles[thread], NULL);

    printf("Mutex\n");
    printf("n = %d trapezoids, area ~= %.15f\n", n, mutex_sum);
      
    free(thread_handles);
    pthread_mutex_destroy(&mutex);
}


void sem_main()
{
    long thread;
    pthread_t* thread_handles;

    sem_init(&semaphore, 0, 1);
    thread_handles = malloc(thread_count * sizeof(pthread_t));

    for(thread = 0; thread < thread_count; thread++)
      pthread_create( &thread_handles[thread], NULL, sem_trapezoid, (void*) thread);

    for(thread = 0; thread < thread_count; thread++)
      pthread_join( thread_handles[thread], NULL);

    printf("Semaphore\n");
    printf("n = %d trapezoids, area ~= %.15f\n", n, sem_sum);

    free(thread_handles);
    sem_destroy(&semaphore);
}

void busy_wait_main()
{
    long thread;
    pthread_t* thread_handles;

    thread_handles = malloc(thread_count * sizeof(pthread_t));

    for(thread = 0; thread < thread_count; thread++)
      pthread_create( &thread_handles[thread], NULL, busy_wait_trapezoid, (void*) thread);

    for(thread = 0; thread < thread_count; thread++)
      pthread_join( thread_handles[thread], NULL);

    free(thread_handles);

    printf("Busy Wait\n");
    printf("n = %d trapezoids, area ~= %.15f\n", n, busy_sum); 
}

double f(double x)
{
    return x;
}

void Read_input(double * a_ptr, double * b_ptr, int* n_ptr) 
{
  printf("Enter left endpoint value (double)\n");
  fflush(stdout);
  scanf("%lf", a_ptr);

  printf("Enter right endpoint value (double)\n");
  fflush(stdout);
  scanf("%lf", b_ptr);

  // printf("Enter number of trapezoids (int)\n");
  // fflush(stdout);
  // scanf("%d", n_ptr);

  h = (b - a) / n;
}

int Get_left(long id, long p, double n)
{
  return (id) * (n) / (p);
}

int Get_right(long id, long p, double n)
{
  return Get_left((id) + 1, p, n) - 1;
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
}