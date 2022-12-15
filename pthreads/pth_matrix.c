
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

const int MAX_THREADS = 1024;

int thread_count;
int m, n = 0;
double* A;
double* x;
double* y;

void Usage(char* prog_name);
void Get_args(int argc, char* argv[]);
void Read_matrix(char* prompt, double A[], int m, int n);
void Read_vector(char* prompt, double x[], int n);
void Print_matrix(char* title, double A[], int m, int n);
void Print_vector(char* title, double y[], double m);

void *Pth_mat_vect(void* rank);


int main(int argc, char* argv[]) 
{
    long thread;
    pthread_t* thread_handles;

    Get_args(argc, argv);

    thread_handles = malloc(thread_count * sizeof(pthread_t));

    printf("Enter sizes\n");

    printf("m: ");
    fflush(stdout);
    scanf("%d", &m);

    printf("n: ");
    fflush(stdout);
    scanf("%d", &n);

    printf("%d x %d (m x n) matrix\n", m, n);
    A = malloc(m * n * sizeof(double));
    x = malloc(n * sizeof(double));
    y = malloc(m * sizeof(double));

    Read_matrix("Enter the matrix", A, m, n);
    Print_matrix("Matrix Values", A, m, n);

    Read_vector("Enter the vector", x, n);
    Print_vector("Vector values", x, n);

    for (thread = 0; thread < thread_count; thread++)
    {
        pthread_create(&thread_handles[thread], NULL, Pth_mat_vect, (void*) thread);
    }
    
    for (thread = 0; thread < thread_count; thread++)
    {
        pthread_join(thread_handles[thread], NULL);
    }
        

    Print_vector("Product y", y, m);

    free(A);
    free(x);
    free(y);

    return 0;
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


void Read_matrix(char* prompt, double A[], int m, int n) 
{
    printf("%s\n", prompt);
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; j++)
                scanf("%lf", &A[i*n+j]);
}

void Read_vector(char* prompt, double x[], int n) 
{
    printf("%s\n", prompt);
    for (int i = 0; i < n; i++)
        scanf("%lf", &x[i]);
}


void *Pth_mat_vect(void* rank) 
{
    long my_rank = (long) rank;
    int local_m = m / thread_count;
    int first_row = my_rank * local_m;
    int last_row = (my_rank + 1) * local_m - 1;

    for (int i = first_row; i <= last_row; i++) 
    {
        y[i] = 0.0; 
        for (int j = 0; j < n; j++)
            y[i] += A[i * n + j] * x[j]; //copied to shared matrix value
    }

    return NULL;
}


void Print_matrix(char* title, double A[], int m, int n) 
{
    printf("%s\n", title);
    for (int i = 0; i < m; i++) 
    {
        for (int j = 0; j < n; j++)
            printf("%4.1f ", A[i * n + j]);
        printf("\n");
    }
}

void Print_vector(char* title, double y[], double m) 
{
    printf("%s\n", title);
    for (int i = 0; i < m; i++)
        printf("%4.1f ", y[i]);
    printf("\n");
}