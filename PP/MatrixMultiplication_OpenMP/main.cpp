#include <iostream>
#include <stdlib.h>
#include <iomanip>
#include <sys/time.h>
#include <omp.h>

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

// Fill matrix
void fillMatrix(float* array, int dim_m, int dim_n, bool transpose);

// Output matrix
void printMatrix(float* array, int dim_m, int dim_n, bool transpose);

using namespace std;

int main(int argc, char* argv[])
{
    int numThreads;
    int dim_l = 1;
    int dim_m = 0;
    int dim_n = 0;
    float* a;
    float* b;
    float* c;

    // Parse arguments
    if (argc == 4) {
        numThreads = atoi(argv[1]);
        dim_m = atoi(argv[2]);
        dim_n = atoi(argv[3]);
    } else {
        cout << "Usage: " << argv[0] << " num_threads m_dimension n_dimension" << endl;
        return 1;
    }    

    // Validate Input
    if (dim_l <= 0 || dim_m <= 0 || dim_n <= 0) {
        cout << "Error: Number of rows and/or columns must be greater than 0" << endl;
        return 1;
    } else if (numThreads <= 0 || numThreads > 8) {
        cout << "Error: Number of threads must be between 1 and 8" << endl;
        return 1;
    }

    int linewidth = dim_n * (cout.precision() + 3);

    // Allocate matrices
    a = new float[dim_l * dim_m];
    b = new float[dim_m * dim_n];
    c = new float[dim_l * dim_n];

    // Fill matrices
    srand48(SEED);
    fillMatrix(a, dim_l, dim_m, false);
    fillMatrix(b, dim_m, dim_n, true);

    // Print matrices
    if (linewidth <= 80) {
        cout << "A:" << endl;
        printMatrix(a, dim_l, dim_n, false);

        cout << "B:" << endl;
        printMatrix(b, dim_m, dim_n, true);
    }

    // Start timer
    TIMER_CLEAR;
    TIMER_START;

#pragma omp parallel shared(c) num_threads(numThreads)
    {
        for (int i = 0; i < dim_l; i++) {
#pragma omp for
            for (int j = 0; j < dim_n; j++) {
                float dotProd = 0.0;
                for (int k = 0; k < dim_m; k++) {
                    dotProd += A(i,k) * B(k,j);
                }
                C(i,j) = dotProd;
            }
        }
    }

    // Stop timer, output
    TIMER_STOP;
    if (linewidth <= 80) {
        cout << "C:" << endl;
        printMatrix(c, dim_l, dim_n, false);
    }
    cout << "Time: " << setprecision(8) << " " << TIMER_ELAPSED/1000.0 << " ms" << endl;


    return 0;
}

void fillMatrix(float* array, int dim_m, int dim_n, bool transpose) {
    for (int i = 0; i < dim_m; i++) {
        for (int j = 0; j < dim_n; j++) {
            if (!transpose) {
                array[i * dim_n + j] = drand48() * MAX_VALUE;
            } else {
                array[j * dim_m + i] = drand48() * MAX_VALUE;
            }
        }
    }
}

void printMatrix(float* array, int dim_m, int dim_n, bool transpose) {
    for (int i = 0; i < dim_m; i++) {
        for (int j = 0; j < dim_n; j++) {
            if (!transpose) {
                cout << array[i * dim_n + j] << " ";
            } else {
                cout << array[i * dim_m + j] << " ";
            }
        }
        cout << endl;
    }
}


