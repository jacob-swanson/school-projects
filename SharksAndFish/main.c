#include <stdio.h>
#include <stdlib.h>

// Size of the ocean
#define WIDTH 512
#define HEIGHT 512

// Default chances
#define FISH_CHANCE 0.2
#define SHARK_CHANCE 0.1

// Constants
#define WATER 0
#define FISH 1
#define SHARK 2

/**
 * @brief fill_ocean Fill ocean with water, fish, and sharks
 * @param ocean Ocean(array) to fill
 * @param fish_chance Chance for a cell to be a fish
 * @param shark_chance Chance for a cell to be a shark
 */
void fill_ocean(int ocean[WIDTH][HEIGHT], double fish_chance, double shark_chance)
{
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; i++)
        {
            int type = WATER;
            // Roll chance for fish and sharks
            if (drand48() < fish_chance)
                type = FISH;
            if (drand48() < shark_chance)
                type = SHARK;

            ocean[i][j] = type;
        }
    }
}

int main(int argc, char* argv[])
{
    // Default values for fish and shark chance
    double fish_chance = FISH_CHANCE;
    double shark_chance = SHARK_CHANCE;
    unsigned int num_iterations = 1000;

    // Parse arguments
    if (argc == 1) {} // No arguments
    else if (argc == 2) // Number of iterations
    {
        num_iterations = atoi(argv[1]);
    }
    else if (argc == 4) // Number of iterations, fish chance, shark chance
    {
        num_iterations = atoi(argv[1]);
        fish_chance = atof(argv[2]);
        shark_chance = atof(argv[3]);
    }
    else
    {
        printf("Usage: SharksAndFish [number of iterations] [fish chance] [shark chance]\n");
        return 1;
    }

    // Create and fill the ocean
    int ocean[WIDTH][HEIGHT];
    fill_ocean(ocean, fish_chance, shark_chance);

    return 0;
}

