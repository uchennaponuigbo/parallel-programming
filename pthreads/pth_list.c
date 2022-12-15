#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include<stdbool.h>

const int MAX_THREADS = 1024;
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
long thread_count;

struct node
{
    int data;
    struct node* next;
};

void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
void Generate_list();

void *List_Work(void * rank);
bool Member(int data);
bool Insert(int data);
bool Delete(int data);
void PrintList(char message[]);
void FreeList();

int num_tasks = 0;
const int MAX_MEMBER_VALUE = 50;
struct node *head = NULL;
//struct node *current = NULL;

int main(int argc, char *argv[])
{
   long       thread;
   pthread_t* thread_handles;

   Get_args(argc, argv);
   Generate_list();

   thread_handles = malloc (thread_count * sizeof(pthread_t));

   for(thread = 0; thread < thread_count; thread++) 
   {
      pthread_create(&thread_handles[thread], NULL, List_Work, (void*) thread); 
   }

   for(thread = 0; thread < thread_count; thread++) 
   {
      pthread_join(thread_handles[thread], NULL);
   }

   PrintList("\nLinked List after thread executions\n");

   free(thread_handles);
   FreeList();

   pthread_exit(NULL);
   return 0;
}

void *List_Work(void * rank)
{
    long my_rank = (long) rank;
    srand(time(NULL) + my_rank * 100);
    //int local_tasks = num_tasks / thread_count; //you could divide the task work, or have every thread do the same number of tasks
    int seed; //rand() % 90;
    int value = 0; //rand() % MAX_MEMBER_VALUE;
    //printf("Thread %ld seed = %d\n", my_rank, seed);
    for(int i = 0; i < num_tasks; i++)
    {
        seed = rand() % 90;
        value = rand() % MAX_MEMBER_VALUE;
        if(seed >= 0 && seed < 30) //[0 - 30)
        {
            printf("Thread %ld Member Task, Value = %d\n", my_rank, value);
            pthread_rwlock_rdlock(&rwlock);
            Member(value);
            pthread_rwlock_unlock(&rwlock);   
        }
        else if(seed >= 30 && seed < 60) //[30 - 60)
        {
            printf("Thread %ld Insert Task, Value = %d\n", my_rank, value);
            pthread_rwlock_wrlock(&rwlock);
            Insert(value);
            pthread_rwlock_unlock(&rwlock);
        }
        else //[60 - 90]
        {
            printf("Thread %ld Delete Task, Value = %d\n", my_rank, value);
            pthread_rwlock_wrlock(&rwlock);
            Delete(value);
            pthread_rwlock_unlock(&rwlock);
        }
    }
    
    return NULL;
}

bool Member(int value)
{
    struct node* curr = head;
    while(curr != NULL && curr->data < value)
        curr = curr->next;

    if(curr == NULL || curr->data > value)
        return false;
    else
        return true;
}

bool Insert(int value)
{
    struct node* curr = head;
    struct node* pred = NULL;
    struct node* temp;

    while(curr != NULL && curr->data <= value)
    {
        pred = curr;
        curr = curr->next;
    }

    if(curr == NULL || curr->data > value)
    {
        temp = malloc(sizeof(struct node));
        temp->data = value;
        temp->next = curr;
        if(pred == NULL)
            head = temp;
        else
            pred->next = temp;
        return true;
    }

    return false;
}

bool Delete(int value)
{
    struct node* curr = head;
    struct node* pred = NULL;

    while(curr != NULL && curr->data < value)
    {
        pred = curr;
        curr = curr->next;
    }

    if(curr != NULL && curr->data == value)
    {
        if(pred == NULL)
        {
            head = curr->next;
            free(curr);
        }
        else
        {
            pred->next = curr->next;
            free(curr);
        }
        return true;
    }
    return false;
}

void PrintList(char message[])
{
    printf("%s", message);
    struct node* curr = head;
    while(curr != NULL)
    {
        printf("%d ", curr->data);
        curr = curr->next;
    }
    printf("\n\n");
}

void FreeList()
{
    struct node* curr;
    while (head != NULL)
    {
       curr = head;
       head = head->next;
       free(curr);
    }
}

void Generate_list()
{
    printf("Generating 10 Random values...\n");
    int value = 0;
    for(int i = 0; i < 10; i++)
    {
        value = rand() % MAX_MEMBER_VALUE;
        Insert(value);
    }
    //Delete(value); //test delete
    // bool doesHave = Member(value); 
    // if(doesHave)
    //     printf("yes");
    // else
    //     printf("no");
    PrintList("Linked List\n");
}

void Get_args(int argc, char* argv[]) 
{
   if (argc != 3) 
      Usage(argv[0]);
   thread_count = strtol(argv[1], NULL, 10);  
   if (thread_count <= 0 || thread_count > MAX_THREADS) 
      Usage(argv[0]);
   num_tasks = strtoll(argv[2], NULL, 10);
   if (num_tasks <= 0) 
      Usage(argv[0]);
}  /* Get_args */

void Usage(char* prog_name) 
{
   fprintf(stderr, "usage: %s <number of threads> <n>\n", prog_name);
   fprintf(stderr, "   n is the number of tasks to be executed and should be >= 1\n");
   fprintf(stderr, "   n should be evenly divisible by the number of threads\n");
   exit(0);
}  /* Usage */
