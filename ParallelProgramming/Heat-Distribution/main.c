/**
  * Heat Distribution Problem
  * Author: Jacob Swanson
  * Date: 8 December 2013
  * Build Instructions:
  *     gcc main.c -o heat-distribution -fopenmp
  * Usage:
  *     heat-distribution [number of threads (optional)]
  */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define ITERATIONS 1000
#define WIDTH 512
#define HEIGHT 512
#define INIT_TEMP 200.0

unsigned int num_threads = 1;

// copied from mpbench
#define TIMER_CLEAR     (tv1.tv_sec = tv1.tv_usec = tv2.tv_sec = tv2.tv_usec = 0)
#define TIMER_START     gettimeofday(&tv1, (struct timezone*)0)
#define TIMER_ELAPSED   ((tv2.tv_usec-tv1.tv_usec)+((tv2.tv_sec-tv1.tv_sec)*1000000))
#define TIMER_STOP      gettimeofday(&tv2, (struct timezone*)0)
struct timeval tv1,tv2;

/**
 * @brief fill_temps Fill the temperature matrix. The border will be INIT_TEMP, and the inside will be 0.0.
 * @param h Matrix
 */
void fill_temps(double h[WIDTH][HEIGHT])
{
    // Fill center
    for (int i = 1; i < WIDTH - 1; i++)
    {
        for (int j = 1; j < HEIGHT - 1; j++)
        {
            h[i][j] = 0.0;
        }
    }

    // Fill top row
    for (int i = 0; i < WIDTH; i++)
    {
        h[i][0] = INIT_TEMP;
    }

    // Fill bottom row
    for (int i = 0; i < WIDTH; i++)
    {
        h[i][HEIGHT - 1] = INIT_TEMP;
    }

    // Fill left column
    for (int j = 1; j < HEIGHT; j++)
    {
        h[0][j] = INIT_TEMP;
    }

    // Fill right column
    for (int j = 1; j < HEIGHT; j++)
    {
        h[WIDTH-1][j] = INIT_TEMP;
    }
}

/**
 * @brief get_average_temp Calculate the average of the matrix
 * @param h Matrix
 * @return
 */
double get_average_temp(double h[WIDTH][HEIGHT])
{
    double avg = 0.0;
#pragma omp parallel for shared(avg) collapse(2) num_threads(num_threads)
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
#pragma omp atomic
            avg += h[i][j];
        }
    }

    avg = avg / (WIDTH * HEIGHT);
    return avg;
}

int main(int argc, char* argv[])
{
    static double h[WIDTH][HEIGHT];
    static double g[WIDTH][HEIGHT];

    // Parse argument(s)
    if (argc == 2)
    {
        num_threads = atoi(argv[1]);
    }

    // Fill the temp matrix
    fill_temps(h);

    TIMER_CLEAR;
    TIMER_START;
    for (int iteration = 0; iteration < ITERATIONS; iteration++)
    {
        // Parallelize the nested loop using "collapse"
#pragma omp parallel for shared(g) collapse(2) num_threads(num_threads)
        for (int i = 1; i < WIDTH; i++)
        {
            for (int j = 1; j < HEIGHT; j++)
            {
                g[i][j] = 0.25 * (h[i-1][j] + h[i+1][j] + h[i][j-1] + h[i][j+1]);
            }
        }

#pragma omp parallel for shared(h) collapse(2) num_threads(num_threads)
        for (int i = 1; i < WIDTH; i++)
        {
            for (int j = 1; j < HEIGHT; j++)
            {
                h[i][j] = g[i][j];
            }
        }

        if (iteration % 100 == 0 || iteration == ITERATIONS - 1)
        {
            printf("Iteration: %i\n", iteration);
            printf("Average Temp: %f\n\n", get_average_temp(h));
        }
    }

    TIMER_STOP;
    printf("Time Taken: %.3f sec\n", TIMER_ELAPSED/1000000.0);
    return 0;
}

