#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

#define SEED 2397
#define MAX_VALUE 100.0

// Access arrays as two dimensional arrays
#define A(i,j) *(a+i*dim_m+j)
#define B(i,j) *(b+i*dim_m+j)
#define C(i,j) *(c+i*dim_n+j)

// copied from mpbench
#define TIMER_CLEAR     (tv1.tv_sec = tv1.tv_usec = tv2.tv_sec = tv2.tv_usec = 0)
#define TIMER_START     gettimeofday(&tv1, (struct timezone*)0)
#define TIMER_ELAPSED   ((tv2.tv_usec-tv1.tv_usec)+((tv2.tv_sec-tv1.tv_sec)*1000000))
#define TIMER_STOP      gettimeofday(&tv2, (struct timezone*)0)
struct timeval tv1,tv2;

using namespace std;

// Fill array with random data
void fillMatrix(float* array, int dim_m, int dim_n);

// Fill array with tranposed random data
void fillMatrixTrans(float* array, int dim_m, int dim_n);

// Output matrix
void printMatrix(float* array, int dim_m, int dim_n);

// Thread function for multiplication
void* thread(void* args);

// Global variables
int dim_l, dim_n, dim_m;
float* a;
float* b;
float* c;
int numThreads;
int numRows;

int main(int argc, char* argv[])
{
    // Parse arguments
    if (argc != 3 && argc != 5) {
        // Incorrect number of arguments
        cout << "Usage: " << argv[0] << " [n_threads] [l_dimension] <m_dimension n_dimension>" << endl;
        return 1;
    } else {
        if (argc == 3) {
            // Only required args
            numThreads = atoi(argv[1]);
            dim_l = dim_n = dim_m = atoi(argv[2]);
        } else {
            // Optional args
            numThreads = atoi(argv[1]);
            dim_l = atoi(argv[2]);
            dim_m = atoi(argv[3]);
            dim_n = atoi(argv[4]);
        }
    }
    numRows = dim_l / numThreads;

    // Input validation
    if (dim_l <= 0 || dim_m <= 0 || dim_m <= 0) {
        cout << "Error: Number of rows and/or columns must be greater than 0" << endl;
        return 1;
    } else if (numThreads <= 0 || numThreads > 8) {
        cout << "Error: Number of threads must be between 1 and 8" << endl;
        return 1;
    }

    // Allocate matrices with given dimensions
    a = new (nothrow) float[dim_l * dim_m];
    b = new (nothrow) float[dim_m * dim_n];
    c = new (nothrow) float[dim_l * dim_n];
    if (a == 0 || b == 0 || c == 0) {
        // Error allocating matrices
        cout << "Error: Insufficient memory" << endl;
        return 1;
    }

    // Fill matrices
    srand48(SEED);
    fillMatrix(a, dim_l, dim_n);
    fillMatrixTrans(b, dim_m, dim_n);

    // Print input matrices
    cout << "A:" << endl;
    printMatrix(a, dim_l, dim_m);

    cout << "B:" << endl;
    printMatrix(b, dim_m, dim_n);

    // Create worker threads
    pthread_t workers[numThreads];
    for (int i = 0; i < numThreads; i++) {
        int* id = new int;
        *id = i;
        if (pthread_create(&workers[i], NULL, thread, id)) {
            cout << "Error: Could not create worker threads" << endl;
            return 1;
        }
    }


    // Start recording time
    TIMER_CLEAR;
    TIMER_START;

    for (int i = 0; i < numThreads; i++) {
        pthread_join(workers[i], NULL);
    }

    // Stop timer
    TIMER_STOP;

    // Output result matrix
    cout << "C:" << endl;
    printMatrix(c, dim_l, dim_n);

    // Output multiplication time
    cout << "Time: " << setprecision(8) << " " << TIMER_ELAPSED/1000.0 << " ms" << endl;

    return 0;
}

void fillMatrix(float* array, int dim_m, int dim_n) {
    for (int i = 0; i < dim_m; i++) {
        for (int j = 0; j < dim_n; j++) {
            array[i * dim_n + j] = drand48() * MAX_VALUE;
        }
    }
}

void fillMatrixTrans(float* array, int dim_m, int dim_n) {
    for (int i = 0; i < dim_m; i++) {
        for (int j = 0; j < dim_n; j++) {
            array[j * dim_n + i] = drand48() * MAX_VALUE;
        }
    }

}

void printMatrix(float* array, int dim_m, int dim_n) {
    for (int i = 0; i < dim_m; i++) {
        for (int j = 0; j < dim_n; j++) {
            cout << array[i * dim_n + j] << " ";
        }
        cout << endl;
    }
}

void* thread(void *args) {
    int id = *((int*)args);

    // Multiply matrices
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < dim_n; j++) {
            float dotProd = 0.0;
            for (int k = 0; k < dim_m; k++) {
                dotProd += A(i + id,k) * B(k,j);
            }
            C(i + id,j) = dotProd;
        }
    }


    return 0;
}


