#include <vector>
#include <tuple>
#include <iostream>
#include <chrono>

#include "globals.hpp"
#include "algorithm.hpp"

// population size has to be even
#define POPULATION_SIZE 500
#define BOARD_SIZE 60

using namespace std;

int main()
{
    int generations;
    vector<int> optimal_solution;

    auto start = chrono::high_resolution_clock::now();
    tie(optimal_solution, generations) = genetic_parallel(BOARD_SIZE, POPULATION_SIZE);
    auto stop = chrono::high_resolution_clock::now();

    cout << "Broj generacija: " << generations << endl;
    cout << "Resenje: ";
    print_vector(optimal_solution);

    auto duration = chrono::duration_cast<chrono::seconds>(stop - start);
    cout << "Vreme izvrsavanja: " << duration.count() << "s" << endl;

    return 0;
}
