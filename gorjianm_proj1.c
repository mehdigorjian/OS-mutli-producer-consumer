/*
COMPILATION: gcc gorjianm_proj1.c -o gorjianm_proj1.exe -pthread
        RUN: ./gorjianm_proj1.exe (1)producer_number (2)consumer_number (3)QueueSize (4)Max_Loop_Size_inside_producer_consumer

*** RUNNING EXAMPLE: ./gorjianm_proj1.exe 5 5 5 5

* the code should be compiled with the -pthread flag
* the code should be run with 4 arguments:
                (1)number of producers
                (2)number of consumers
                (3)Queue size
                (4)Max loop size inside the producer and consumer function
*/

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
 
/////////////////////////////////////////////// QUEUE IMPLEMENTATION from ref. [1]

// A structure to represent a queue
struct Queue {
    int front, rear, size;
    unsigned capacity;
    int* array;
};
 
// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
 
    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(queue->capacity * sizeof(int));
    return queue;
}
 
// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}
 
// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}
 
// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}
 
// Function to remove an item from queue.
// It changes front and size
int dequeue(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}
 
// Function to get front of queue
int front(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}
 
// Function to get rear of queue
int rear(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->rear];
}
/////////////////////////////////////////////// END OF THE QUEUE IMPLEMENTATION
struct Queue* queue;

sem_t empty;
sem_t full;
pthread_mutex_t mutex;

struct arguments
{
    int Max_Loop;
    int COUNT;
};

void *producer(void *producer_number)
{   
    int item;
    for(int i = 0; i < ((struct arguments*)producer_number)->Max_Loop; i++) {
        // generate a random item
        item = rand();
        // producer attempts to produce
        sem_wait(&empty);
        // producer got the permission for producing and waiting for its turn
        pthread_mutex_lock(&mutex);
        // pushing the item to the queue
        enqueue(queue, item);
        // producer producing the data
        printf("Producer [%d]: PRODUCED item [%d]\n", ((struct arguments*)producer_number)->COUNT, item);
        // sleep(1);
        // producer locking the critical sector
        pthread_mutex_unlock(&mutex);
        // producer incrementing the value (full)
        sem_post(&full);
    }
}
void *consumer(void *consumer_number)
{       
    for(int i = 0; i < ((struct arguments*)consumer_number)->Max_Loop; i++) {
        // consumer attempts to consume
        sem_wait(&full);
        // consumer got the permission for consuming and waiting for its turn
        pthread_mutex_lock(&mutex);
        // poping the item from the queue
        int item = dequeue(queue);
        // consumer consuming the data
        printf("Consumer [%d]: CONSUMED item [%d]\n", ((struct arguments*)consumer_number)->COUNT, item);
        // sleep(1);
        // consumer locking the critical sector
        pthread_mutex_unlock(&mutex);
        // consumer decrementing the value (empty)
        sem_post(&empty);
    }
}

int main(int argc,char *argv[]) {
    // getting the number of puoducers and consumers and
    // queue size and the Max loop number for each producer/consumer
    int producer_num = atoi(argv[1]);
    int consumer_num = atoi(argv[2]);
    int QueueSize = atoi(argv[3]);
    int Max_Num = atoi(argv[4]);

    //initializing the queue
    queue = createQueue(QueueSize);

    // thread variables and the mutex initialization
    pthread_t produce[producer_num],consume[consumer_num];
    pthread_mutex_init(&mutex, NULL);

    // initialize semaphore variables
    // (1) var name, (2) 0 means no-shared memory, (3) initial var
    sem_init(&empty, 0, QueueSize);
    sem_init(&full, 0, 0);

    // creating the arguments struct to pass it to the pthread_create function
    struct arguments *prod = (struct arguments *)malloc(sizeof(struct arguments));
    prod->Max_Loop = Max_Num;

    struct arguments *cons = (struct arguments *)malloc(sizeof(struct arguments));
    cons->Max_Loop = Max_Num;

    // producer threads creation
    for(int i = 0; i < producer_num; i++) {
        // setting the COUNT to the loop counter to pass it to the 'producer' function as the producer counter
        prod->COUNT = i;
        pthread_create(&produce[i], NULL, (void *)producer, (void *)prod);
    }
    // consumer threads creation
    for(int j = 0; j < consumer_num; j++) {
        // setting the COUNT to the loop counter to pass it to the 'consumer' function as the consumer counter
        cons->COUNT = j;
        pthread_create(&consume[j], NULL, (void *)consumer, (void *)cons);
    }
    // producer threads join
    for(int k = 0; k < producer_num; k++) {
        pthread_join(produce[k], NULL);
    }
    // consumer threads join
    for(int l = 0; l < consumer_num; l++) {
        pthread_join(consume[l], NULL);
    }
    
    // destroying pthread mutex, and the semaphore
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    return 0; 
}


/*
The queue implementation is based on the explanatiion in reference [1] 
the producer consumer idea was derived from the reference [2] however 
I totaly redesign it with the queue implementation, and the multi-parameter 
passing structure to eliminate manual parameter setting for the Max loop 
number, and  using only the commandline args.


REFERENCE
[1] https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/
[2] https://shivammitra.com/c/producer-consumer-problem-in-c/#
*/