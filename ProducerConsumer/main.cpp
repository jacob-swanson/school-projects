#include <iostream>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>

using namespace std;

#define MAX_CHILDREN 100

// Global variables
int numProducers = 0;
int numConsumers = 0;
int numItems = 0;
int bufferSize = 0;
int* buffer;

int producerIndex = 0;
int consumerIndex = 0;
int consumedItems = 0;

sem_t mutex;
sem_t full;
sem_t empty;

// Functions
void* producer(void* arg);
void* consumer(void* arg);

int main(int argc, char* argv[])
{
    // Get user input
    while (numProducers < 1) {
        cout << "Enter number of producers: ";
        cin >> numProducers;
    }

    while (numConsumers < 1) {
        cout << "Enter number of consumers: ";
        cin >> numConsumers;
    }

    while (numItems < 1) {
        cout << "Enter the total number of data items produced/consumed: ";
        cin >> numItems;
    }

    while (bufferSize < 1) {
        cout << "Enter the size of the circular buffer: ";
        cin >> bufferSize;
    }

    if (numProducers + numConsumers > MAX_CHILDREN) {
        cout << "Error: Too many producer/consumer threads requested." << endl;
        return 1;
    }

    // Setup semaphores
    int status = sem_init(&mutex, 0, 1);
    if (status != 0) {
        cerr << "Error creating mutex semaphore" << endl;
        return 1;
    }
    status = sem_init(&full, 0, 0);
    if (status != 0) {
        cerr << "Error creating full semaphore" << endl;
        return 1;
    }

    status = sem_init(&empty, 0, bufferSize);
    if (status != 0) {
        cerr << "Error creating empty semaphore" << endl;
        return 1;
    }

    // Create buffer
    buffer = new int[bufferSize];

    // Create producers
    pthread_t producers[numProducers];
    int baseItems = numItems / numProducers;
    int extraItems = numItems % numProducers;
    for (int i = 0; i < numProducers; i++) {
        int* producingItems = new int;
        *producingItems = baseItems + (extraItems > i);
        int status = pthread_create(&producers[i], NULL, producer, producingItems);
        if (status != 0) {
            cerr << "Error creating producer" << endl;
            return 1;
        }
    }

    // Create consumers
    pthread_t consumers[numConsumers];
    baseItems = numItems / numConsumers;
    extraItems = numItems % numConsumers;
    for (int i = 0; i < numConsumers; i++) {
        int* consumingItems = new int;
        *consumingItems = baseItems + (extraItems > i);
        int status = pthread_create(&consumers[i], NULL, consumer, consumingItems);
        if (status != 0) {
            cerr << "Error creating consumer" << endl;
            return 1;
        }

    }

    // Wait for consumers and producers to exit
    for (int i = 0; i < numConsumers; i++) {
        int status = pthread_join(consumers[i], NULL);
        if (status != 0) {
            cerr << "Error in consumer thread" << endl;
        }
    }
    for (int i = 0; i < numProducers; i++) {
        int status = pthread_join(producers[i], NULL);
        if (status != 0) {
            cerr << "Error in producer thread" << endl;
        }
    }


    return 0;
}

void* producer(void* arg)
{
    int numItems = *((int*)arg);
    pthread_t self = pthread_self();

    sem_wait(&mutex);
    srand(1234);
    cout << "Producer " << self << "items to produce: " << numItems << endl;
    sem_post(&mutex);


    for (int i = 0; i < numItems; i++) {
        int data = rand() % 100 + 1;

        sem_wait(&empty);
        sem_wait(&mutex);

        // Place data into buffer
        buffer[producerIndex] = data;

        cout << "Producer " << self
             << " placed data [" << data
             << "] in buffer slot [" << producerIndex << "]" << endl;

        producerIndex = (producerIndex + 1) % bufferSize;

        sem_post(&mutex);
        sem_post(&full);
    }

    return 0;
}

void* consumer(void* arg)
{
    int numItems = *((int*)arg);
    pthread_t self = pthread_self();

    sem_wait(&mutex);
    cout << "Consumer " << self << "items to consume: " << numItems << endl;
    sem_post(&mutex);


    for (int i = 0; i < numItems; i++) {
        sem_wait(&full);
        sem_wait(&mutex);

        int data = buffer[consumerIndex];

        cout << "Consumer " << self
             << " received data item [" << data
             << "] via buffer slot [" << consumerIndex << "]" << endl;

        consumerIndex = (consumerIndex + 1) % bufferSize;

        sem_post(&mutex);
        sem_post(&empty);
    }

    return 0;
}

