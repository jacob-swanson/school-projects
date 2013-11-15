#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8967   // Port to bind to
#define HOST "127.0.0.1"    // Host to connect to
#define MAXDATASIZE 1024
#define THREADS 8

void* thread(void *argsPtr);

pthread_cond_t cond;
pthread_mutex_t mutex;
int condition = 0;

using namespace std;

class Arguments
{
public:
    int id;
    int left;
    int right;
};

int *data;

int main()
{
    // Create the socket
    int clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        cerr << "Error opening socket" << endl;
        return 1;
    }

    // Connect the socket
    struct sockaddr_in clientAddress;
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(PORT);
    clientAddress.sin_addr.s_addr = inet_addr(HOST);
    int connectionStatus = connect(clientSocket,
                               (struct sockaddr *) &clientAddress,
                               sizeof(struct sockaddr_in));
    if (connectionStatus < 0)
    {
        cerr << "Error opening first socket" << endl;
        return 1;
    }

    // Receive data
    int numBytes;
    char buf[MAXDATASIZE];
    if ((numBytes = recv(clientSocket, buf, MAXDATASIZE - 1, 0)) < 0)
    {
        cerr << "Error receiving data" << endl;
        return 1;
    }

    buf[numBytes] = '\0';
    cout << "Received: " << buf << endl;

    // Create worker threads
    pthread_t workers[THREADS];
    for (int i = 0; i < THREADS; i++) {
        Arguments *args = new Arguments();
        args->id = i;

        if (pthread_create(&workers[i], NULL, thread, args)) {
            cout << "Error: Could not create worker threads" << endl;
            return 1;
        }
    }

    // Start threads
    pthread_mutex_lock(&mutex);
    condition = 1;
    if (pthread_cond_broadcast(&cond)) {
        cout << "Error: Failed to start threads" << endl;
        pthread_mutex_unlock(&mutex);
        return 1;
    }
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i < THREADS; i++) {
        pthread_join(workers[i], NULL);
    }

    // Return result
    if (send(clientSocket, "My result!", 10, 0) < 0)
    {
        cerr << "Error sending data" << endl;
        return 1;
    }

    close(clientSocket);

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

    // Do work

    pthread_exit((void*) 0);
}
