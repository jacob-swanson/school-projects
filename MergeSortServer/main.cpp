#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <iomanip>
#include <cstdlib>
#include <errno.h>

#define TIMER_CLEAR     (tv1.tv_sec = tv1.tv_usec = tv2.tv_sec = tv2.tv_usec = 0)
#define TIMER_START     gettimeofday(&tv1, (struct timezone*)0)
#define TIMER_ELAPSED   ((tv2.tv_usec-tv1.tv_usec)+((tv2.tv_sec-tv1.tv_sec)*1000000))
#define TIMER_STOP      gettimeofday(&tv2, (struct timezone*)0)
struct timeval tv1,tv2;

#define PORT 8967   // Port to bind to
#define BACKLOG 100  // Size of pending connections queue
#define NUM_CLIENTS 2  // Number of clients before work begins
#define MAXDATASIZE 524280    // Buffer size
#define SEED 1234   // Seed for random
#define DATA_SIZE 10000    // Amount of data to produce
#define EXPECTED_DATA 40000

void* thread(void*);

pthread_cond_t cond;
pthread_mutex_t mutex;
int condition = 0;
int data[DATA_SIZE];
int chunkSize = DATA_SIZE / NUM_CLIENTS;

using namespace std;

class Arguments
{
public:
    int id;
    int mySocket;
};

int main()
{
    // Create data
    srand(SEED);
    for (int i = 0; i < DATA_SIZE; i++)
    {
        data[i] = rand() % 1000;
    }

    // Create the socket
    int serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        cerr << "Error opening socket" << endl;
        close(serverSocket);
        return 1;
    }

    int yes = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
    {
        cerr << "Error setting socket options" << endl;
        close(serverSocket);
        return 1;
    }

    // Bind socket to a port
    struct sockaddr_in sockAddress;
    sockAddress.sin_family = AF_INET;
    sockAddress.sin_port = htons(PORT);
    sockAddress.sin_addr.s_addr = INADDR_ANY;

    int bindStatus = bind(
                serverSocket,
                (struct sockaddr *) &sockAddress,
                sizeof(struct sockaddr_in));
    if (bindStatus < 0)
    {
        cerr << "Error binding socket" << endl;
        close(serverSocket);
        return 1;
    }

    // Start listening
    int listenStatus = listen(serverSocket, BACKLOG);
    if (listenStatus < 0)
    {
        cerr << "Error listening on socket" << endl;
        close(serverSocket);
        return 1;
    }

    cout << "Server listening..." << endl;

    // Main loop
    pthread_t workers[NUM_CLIENTS];
    int numWorkers = 0;
    while (true)
    {
        // Accept connections
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength;
        int clientSocket = accept4(
                    serverSocket,
                    (struct sockaddr *) &clientAddress,
                    &clientAddressLength, SOCK_CLOEXEC);
        if (clientSocket < 0)
        {
            cerr << "Error " << clientSocket << " accepting connection" << endl;
            close(serverSocket);
            return 1;
        }
        else
        {
            cout << "Accepted connection from: "
                    << inet_ntoa(clientAddress.sin_addr)
                    << ":" << ntohs(clientAddress.sin_port)
                    << endl;

            // Create client thread
            Arguments *args = new Arguments();
            args->id = numWorkers;
            args->mySocket = clientSocket;
            if (pthread_create(&workers[numWorkers], NULL, thread, args))
            {
                cerr << "Error creating worker thread" << endl;
                close(serverSocket);
                return 1;
            }
            numWorkers++;

            // Check if all clients have connected
            if (numWorkers == NUM_CLIENTS)
            {
                // Start timer
                TIMER_CLEAR;
                TIMER_START;

                cout << "Starting workers" << endl;
                // Broadcast the starting condition
                pthread_mutex_lock(&mutex);
                condition = 1;
                if (pthread_cond_broadcast(&cond))
                {
                    cout << "Error setting condition" << endl;
                    pthread_mutex_unlock(&mutex);
                    close(serverSocket);
                    return 1;
                }
                pthread_mutex_unlock(&mutex);

                // Wait for workers to finish
                for (int i = 0; i < NUM_CLIENTS; i++)
                {
                    pthread_join(workers[i], NULL);
                }

                // Combine work

                // Stop timer
                TIMER_STOP;

                cout << "Time taken: " << setprecision(8) << TIMER_ELAPSED/1000.0 << " ms" << endl;

                break;
            }
        }
    }

    // Exit
    sleep(2);
    close(serverSocket);
    return 0;
}

void* thread(void *argsPtr)
{
    Arguments args = *((Arguments*)argsPtr);

    // Wait for the condition
    pthread_mutex_lock(&mutex);
    while (!condition)
    {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    // Send data
    if (send(args.mySocket, &data, sizeof(data), 0) < 0)
    {
        cerr << "Error sending to client" << endl;
        pthread_exit((void*) 1);
    }

    // Receive result
    int numBytes;
    int buf[MAXDATASIZE];
    if ((numBytes = recv(args.mySocket, buf, EXPECTED_DATA, MSG_WAITALL)) < 0)
    {
        cerr << "Error receiving data" << endl;
        pthread_exit((void*) 1);
    }

    cout << "Received: ";
    for (int i = 0; i < numBytes/4; i++)
    {
        cout << buf[i] << " ";
    }
    cout << endl;

    // Exit
    close(args.mySocket);
    pthread_exit((void*) 0);
}
