#include <stdio.h>

#define ITERATIONS 1000
#define WIDTH 512
#define HEIGHT 512
#define INIT_TEMP 200.0

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

double get_average_temp(double h[WIDTH][HEIGHT])
{
    double avg = 0.0;
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            avg += h[i][j];
        }
    }

    avg = avg / (WIDTH * HEIGHT);
    return avg;
}

int main(void)
{
    static double h[WIDTH][HEIGHT];
    static double g[WIDTH][HEIGHT];

    fill_temps(h);

    for (int iteration = 0; iteration < ITERATIONS; iteration++)
    {
        for (int i = 1; i < WIDTH; i++)
        {
            for (int j = 1; j < HEIGHT; j++)
            {
                g[i][j] = 0.25 * (h[i-1][j] + h[i+1][j] + h[i][j-1] + h[i][j+1]);
            }
        }

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
    return 0;
}

