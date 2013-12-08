#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Size of the ocean
#define WIDTH 512
#define HEIGHT 512

// Default values
#define FISH_CHANCE 0.6
#define SHARK_CHANCE 0.2
#define ITERATIONS 1000
#define SEED 1234
#define BIRTHING_AGE 3
#define DEATH_AGE 150

#define WATER 0
#define FISH 1
#define SHARK -1

using namespace std;

/**
 * @brief fill_ocean Fill ocean with water, fish, and sharks
 * @param ocean Ocean(array) to fill
 * @param fish_chance Chance for a cell to be a fish
 * @param shark_chance Chance for a cell to be a shark
 */
void fill_ocean(int ocean[WIDTH][HEIGHT], double fish_chance, double shark_chance)
{
    for (unsigned int i = 0; i < WIDTH; i++)
    {
        for (unsigned int j = 0; j < HEIGHT; j++)
        {
            // Roll chance for fish and sharks
            // Priority: Fish -> Shark -> Water
            ocean[i][j] = WATER;

            if (drand48() < shark_chance)
            {
                ocean[i][j] = SHARK;
            }

            if (drand48() < fish_chance)
            {
                ocean[i][j] = FISH;
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
void count_fish_and_sharks(int ocean[WIDTH][HEIGHT], unsigned int *fish, unsigned int *shark)
{
    for (unsigned int i = 0; i < WIDTH; i++)
    {
        for (unsigned int j = 0; j < HEIGHT; j++)
        {
            if (ocean[i][j] > WATER)
            {
                *fish = *fish + 1;
            }
            else if (ocean[i][j] < WATER)
            {
                *shark = *shark + 1;
            }
        }
    }
}

vector<pair<int,int> > get_adjacent_water(int ocean[WIDTH][HEIGHT], int updated[WIDTH][HEIGHT], int i, int j)
{
    vector<pair<int,int> > cells;

    for (int x = i - 1; x < i + 1; x++)
    {
        for (int y = j - 1; y < j + 1; y++)
        {
            if (ocean[x][y] == WATER && updated[x][y] == WATER)
            {
                pair<int,int> cell;
                cell.first = x;
                cell.second = y;
                cells.push_back(cell);
            }
        }
    }

    return cells;
}

vector<pair<int,int> > get_adjacent_fish(int ocean[WIDTH][HEIGHT], int updated[WIDTH][HEIGHT], int i, int j)
{
    vector<pair<int,int> > cells;

    for (int x = i - 1; x < i + 1; x++)
    {
        for (int y = j - 1; y < j + 1; y++)
        {
            if (ocean[x][y] > WATER && updated[x][y] > WATER)
            {
                pair<int,int> cell;
                cell.first = x;
                cell.second = y;
                cells.push_back(cell);
            }
        }
    }

    return cells;
}

void tick(int ocean[WIDTH][HEIGHT])
{
    static int updated[WIDTH][HEIGHT];

    // Ignoring edge cases for now
    for (unsigned int i = 1; i < WIDTH - 1; i++)
    {
        for (unsigned int j = 1; j < HEIGHT - 1; j++)
        {
            if (ocean[i][j] > WATER) // Is fish
            {
                // Death
                if (ocean[i][j] >= DEATH_AGE)
                {
                    ocean[i][j] = WATER;
                }
                else
                {
                    // Move
                    vector<pair<int,int> > adjacent_cells = get_adjacent_water(ocean, updated, i, j);
                    pair<int,int> new_pos;
                    if (adjacent_cells.empty()) // Is empty
                    {
                        // Move fish over in same place
                        updated[i][j] = ocean[i][j];
                        ocean[i][j] = WATER;
                        new_pos.first = i;
                        new_pos.second = j;
                    }
                    else
                    {
                        // One or more free cells, pick a random one
                        unsigned int index = rand() % adjacent_cells.size();
                        pair<int,int> cell = adjacent_cells[index];
                        updated[cell.first][cell.second] = ocean[i][j];

                        // Birthing
                        if (ocean[i][j] == BIRTHING_AGE)
                        {
                            updated[i][j] = FISH;
                        }

                        ocean[i][j] = WATER;
                        new_pos.first = cell.first;
                        new_pos.second = cell.second;
                    }

                    // Update age
                    updated[new_pos.first][new_pos.second]++;
                }
            }
            else if (ocean[i][j] < WATER) // Is shark
            {
                // Death
                if (ocean[i][j]*-1 >= DEATH_AGE)
                {
                    ocean[i][j] = WATER;
                }
                else
                {
                    // Eating and moving
                    vector<pair<int,int> > adjacent_fish = get_adjacent_fish(ocean, updated, i, j);
                    pair<int,int> new_pos;
                    if (adjacent_fish.empty()) // Is empty
                    {
                        // Move to a random cell
                        vector<pair<int,int> > adjacent_cells = get_adjacent_water(ocean, updated, i, j);
                        if (adjacent_cells.empty()) // Is empty
                        {
                            // Move shark over in same place
                            updated[i][j] = ocean[i][j];
                            ocean[i][j] = WATER;
                            new_pos.first = i;
                            new_pos.second = j;
                        }
                        else
                        {
                            // One or more free cells, pick a random one
                            unsigned int index = rand() % adjacent_cells.size();
                            pair<int,int> cell = adjacent_cells[index];
                            updated[cell.first][cell.second] = ocean[i][j];

                            // Birthing
                            if (ocean[i][j]*-1 == BIRTHING_AGE)
                            {
                                updated[i][j] = SHARK;
                            }

                            ocean[i][j] = WATER;
                            new_pos.first = cell.first;
                            new_pos.second = cell.second;
                        }
                    }
                    else // Eat a fish
                    {
                        // One or more free cells, pick a random one
                        unsigned int index = rand() % adjacent_fish.size();
                        pair<int,int> cell = adjacent_fish[index];
                        updated[cell.first][cell.second] = ocean[i][j];

                        // Birthing
                        if (ocean[i][j]*-1 == BIRTHING_AGE)
                        {
                            updated[i][j] = SHARK;
                        }

                        ocean[i][j] = WATER;
                        new_pos.first = cell.first;
                        new_pos.second = cell.second;
                    }

                    // Update age
                    updated[new_pos.first][new_pos.second]--;
                }
            }
        }
    }

    for (unsigned int i = 0; i < WIDTH; i++)
    {
        for (unsigned int j = 0; j < HEIGHT; j++)
        {
            ocean[i][j] = updated[i][j];
            updated[i][j] = WATER;
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
    static int ocean[WIDTH][HEIGHT]; // Declared as static because of large size
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

