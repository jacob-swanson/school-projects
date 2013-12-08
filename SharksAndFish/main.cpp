#include <stdio.h>
#include <stdlib.h>

// Size of the ocean
#define WIDTH 512
#define HEIGHT 512

// Default values
#define FISH_CHANCE 0.5
#define SHARK_CHANCE 0.1
#define ITERATIONS 1000
#define SEED 1234

enum cell_type
{
    WATER,
    FISH,
    SHARK
};

using namespace std;

class Cell
{
public:
    cell_type type;
    int generation;
    int fed;

};

/**
 * @brief fill_ocean Fill ocean with water, fish, and sharks
 * @param ocean Ocean(array) to fill
 * @param fish_chance Chance for a cell to be a fish
 * @param shark_chance Chance for a cell to be a shark
 */
void fill_ocean(Cell* ocean[WIDTH][HEIGHT], double fish_chance, double shark_chance)
{
    for (unsigned int i = 0; i < WIDTH; i++)
    {
        for (unsigned int j = 0; j < HEIGHT; j++)
        {
            // Roll chance for fish and sharks
            // Priority: Fish -> Shark -> Water
            ocean[i][j]->type = WATER;

            if (drand48() < shark_chance)
            {
                ocean[i][j]->type = SHARK;
            }

            if (drand48() < fish_chance)
            {
                ocean[i][j]->type = FISH;
            }
        }
    }
}

/**
 * @brief count_fish_and_sharks Count the number of fish and sharks
 * @param ocean Ocean to count in
 * @param fish Pointer to fish counter variable
 * @param shark Pointer to shark counter variable
 */
void count_fish_and_sharks(Cell* ocean[WIDTH][HEIGHT], unsigned int *fish, unsigned int *shark)
{
    for (unsigned int i = 0; i < WIDTH; i++)
    {
        for (unsigned int j = 0; j < HEIGHT; j++)
        {
            if (ocean[i][j]->type == FISH)
            {
                *fish = *fish + 1;
            }
            else if (ocean[i][j]->type == SHARK)
            {
                *shark = *shark + 1;
            }
        }
    }
}

/**
 * @brief move_fish Move fish or shark from (i,j) to (x,y)
 * @param i
 * @param j
 * @param x
 * @param y
 */
void move_fish(int i, int j, int x, int y)
{

}

/**
 * @brief kill_fish Kill fish or shark at (i,j)
 * @param i
 * @param j
 */
void kill_fish(int i, int j)
{

}

void tick(Cell* ocean[WIDTH][HEIGHT])
{
    for (unsigned int i = 0; i < WIDTH; i++)
    {
        for (unsigned int j = 0; j < HEIGHT; j++)
        {
            if (ocean[i][j]->type == FISH)
            {
                // Check adjacent cells

                // If one empty, move to it

                // If multiple empty, pick a random one

                // If breeding age, create baby in left cell

                // If old, die

            }
            else if (ocean[i][j]->type == SHARK)
            {
                // If adjacent with fish, move to it, and eat fish

                // If more adjancent with fish, choose fish at random

                // If no fish, choose random cell

                // If breeding age, give birth

                // If not eaten for y generations, die
            }
        }
    }
}

int main(int argc, char* argv[])
{
    // Default values for fish and shark chance
    double fish_chance = FISH_CHANCE;
    double shark_chance = SHARK_CHANCE;
    unsigned int num_iterations = ITERATIONS;

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
        printf("Usage: SharksAndFish <number of iterations> [fish chance] [shark chance]\n");
        return 1;
    }

    // Seed RNG
    srand(SEED);

    // Create and fill the ocean
    static Cell* ocean[WIDTH][HEIGHT]; // Declared as static because of large size
    fill_ocean(ocean, fish_chance, shark_chance);


    for (unsigned int iteration = 0; iteration < num_iterations; iteration++)
    {
        tick(ocean);

        // Get stats every 100 ticks, and the last tick
        if (iteration % 100 == 0 || iteration == num_iterations - 1)
        {
            unsigned int fish = 0;
            unsigned int shark = 0;
            count_fish_and_sharks(ocean, &fish, &shark);
            printf("Tick: %i\nFish: %i (%.2f%%)\nSharks: %i (%.2f%%)\n\n", iteration, fish, ((double)fish/(WIDTH*HEIGHT))*100, shark, ((double)shark/(WIDTH*HEIGHT))*100);

        }
    }

    return 0;
}

