#include <iostream>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

#define MAX_CHILDREN 100

// Structures
class Arguments {
public:
    int id;
    int numItems;
};

class Item {
public:
    int producerId;
    int value;
};

// Global variables
int numProducers = 0;
int numConsumers = 0;
int numItems = 0;
int bufferSize = 0;
Item* buffer;

int producerIndex = 0;
int consumerIndex = 0;
int consumedItems = 0;

sem_t mutex;
sem_t full;
sem_t empty;

ofstream diaryFile;

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
    buffer = new Item[bufferSize];

    // Open diary file
    diaryFile.open("diary");

    // Create producers
    pthread_t producers[numProducers];
    int baseItems = numItems / numProducers;
    int extraItems = numItems % numProducers;
    for (int i = 0; i < numProducers; i++) {
        // args must be allocated dynamically
        Arguments* args = new Arguments();
        args->numItems = baseItems + (extraItems > i);
        args->id = i;
        int status = pthread_create(&producers[i], NULL, producer, args);
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
        // args must be allocated dynamically
        Arguments* args = new Arguments();
        args->numItems = baseItems + (extraItems > i);
        args->id = i;
        int status = pthread_create(&consumers[i], NULL, consumer, args);
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

    // Close diary file
    diaryFile.close();

    return 0;
}

void* producer(void* arg)
{
    Arguments args = *((Arguments*)arg);

    // Open output file
    ofstream output;
    stringstream filename;
    filename << "prod_" << args.id;
    output.open(filename.str().c_str());

    sem_wait(&mutex);
    srand(1234);
    sem_post(&mutex);


    for (int i = 0; i < args.numItems; i++) {
        // Create data
        Item data;
        data.value = rand() % 100 + 1;
        data.producerId = args.id;

        sem_wait(&empty);
        sem_wait(&mutex);

        // Place data into buffer
        buffer[producerIndex] = data;

        output << "Producer" << args.id
             << " placed data [" << data.value
             << "] in buffer slot(" << producerIndex << ")" << endl;
        diaryFile << "Producer" << args.id
             << " placed data [" << data.value
             << "] in buffer slot(" << producerIndex << ")" << endl;


        producerIndex = (producerIndex + 1) % bufferSize;

        sem_post(&mutex);
        sem_post(&full);
    }

    // Close output file
    output.close();

    return 0;
}

void* consumer(void* arg)
{
    Arguments args = *((Arguments*)arg);

    // Open output file
    ofstream output;
    stringstream filename;
    filename << "con_" << args.id;
    output.open(filename.str().c_str());

    for (int i = 0; i < args.numItems; i++) {
        sem_wait(&full);
        sem_wait(&mutex);

        Item data = buffer[consumerIndex];

        output << "Consumer" << args.id
             << " received Producer" << data.producerId
             << "'s data item [" << data.value
             << "] via buffer slot(" << consumerIndex << ")" << endl;
        diaryFile << "Consumer" << args.id
             << " received Producer" << data.producerId
             << "'s data item [" << data.value
             << "] via buffer slot(" << consumerIndex << ")" << endl;


        consumerIndex = (consumerIndex + 1) % bufferSize;

        sem_post(&mutex);
        sem_post(&empty);
    }

    // Close output file
    output.close();

    return 0;
}

