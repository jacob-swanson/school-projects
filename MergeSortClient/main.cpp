#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8967   // Port to bind to
#define HOST "127.0.0.1"    // Host to connect to
#define MAXDATASIZE 524280
#define THREADS 8

void* thread(void *argsPtr);
void TopDownSplitMerge(int *A, int iBegin, int iEnd, int *B);
int TopDownMerge(int*, int, int, int, int*);
void CopyArray(int *B, int iBegin, int iEnd, int *A);

pthread_cond_t cond;
pthread_mutex_t mutex;
int condition = 0;
int *data;
int *sortedData;
int num;

using namespace std;

class Arguments
{
public:
    int id;
    int left;
    int right;
};

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

    int buf[MAXDATASIZE];
    if ((numBytes = recv(clientSocket, buf, MAXDATASIZE - 1, 0)) < 0)
    {
        cerr << "Error receiving data" << endl;
        close(clientSocket);
        return 1;
    }

    num = numBytes/4;
    data = new int[num];
    sortedData = new int[num];
    //cout << "Received: ";
    int received = 0;
    for (int i = 0; i < num; i++)
    {
        //cout << buf[i] << " ";
        data[i] = buf[i];
        received++;
    }
    cout << endl;
    cout << "Received: " << received << endl;

    // Create worker threads
    pthread_t workers[THREADS];
    int chunkSize = (num) / THREADS;
    for (int i = 0; i < THREADS; i++) {
        Arguments *args = new Arguments();
        args->id = i;

        // Calc left and right
        args->left = chunkSize * i;
        if (i < THREADS - 1)
        {
            args->right = args->left + chunkSize;
        }
        else
        {
            args->right = num;
        }

        if (pthread_create(&workers[i], NULL, thread, args)) {
            cout << "Error: Could not create worker threads" << endl;
            close(clientSocket);
            return 1;
        }
    }

    // Start threads
    pthread_mutex_lock(&mutex);
    condition = 1;
    if (pthread_cond_broadcast(&cond)) {
        cout << "Error: Failed to start threads" << endl;
        pthread_mutex_unlock(&mutex);
        close(clientSocket);
        return 1;
    }
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i < THREADS; i++) {
        pthread_join(workers[i], NULL);
    }

//    cout << "Sorted Data: ";
//    for (int i = 0; i < num; i++)
//    {
//        cout << sortedData[i] << " ";
//    }
//    cout << endl;

    // Return result
    if (send(clientSocket, sortedData, numBytes, 0) < 0)
    {
        cerr << "Error sending data" << endl;
        close(clientSocket);
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
    TopDownSplitMerge(data, args.left, args.right, sortedData);

    pthread_exit((void*) 0);
}

void TopDownSplitMerge(int *A, int iBegin, int iEnd, int *B)
{
    if(iEnd - iBegin < 2)                       // if run size == 1
        return;                                 //   consider it sorted
    // recursively split runs into two halves until run size == 1,
    // then merge them and return back up the call chain
    int iMiddle = (iEnd + iBegin) / 2;              // iMiddle = mid point
    TopDownSplitMerge(A, iBegin,  iMiddle, B);  // split / merge left  half
    TopDownSplitMerge(A, iMiddle, iEnd,    B);  // split / merge right half
    TopDownMerge(A, iBegin, iMiddle, iEnd, B);  // merge the two half runs
    CopyArray(B, iBegin, iEnd, A);              // copy the merged runs back to A
}

int TopDownMerge(int *A, int iBegin, int iMiddle, int iEnd, int *B)
{
    int i0 = iBegin, i1 = iMiddle;

    // While there are elements in the left or right runs
    for (int j = iBegin; j < iEnd; j++) {
        // If left run head exists and is <= existing right run head.
        if (i0 < iMiddle && (i1 >= iEnd || A[i0] <= A[i1]))
            B[j] = A[i0++];  // Increment i0 after using it as an index.
        else
            B[j] = A[i1++];  // Increment i1 after using it as an index.
    }

}

void CopyArray(int *B, int iBegin, int iEnd, int *A)
{
    for(int i = iBegin; i < iEnd; i++)
    {
        A[i] = B[i];
    }
}
