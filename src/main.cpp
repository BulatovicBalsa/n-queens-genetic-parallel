#include <vector>
#include <tuple>
#include <iostream>
#include <chrono>

#include "globals.hpp"
#include "algorithm.hpp"

// population size has to be even
#define POPULATION_SIZE 300
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
    cout << "Vreme izvrsavanja paralelnog algoritma: " << duration.count() << "s" << endl;
    cout << "Vreme po generaciji paralelnog algoritma: " << duration.count() / (generations * 1.0) << "s" << endl;
    if (fitness_score(BOARD_SIZE, optimal_solution) == 0)
    {
        cout << "Resenje tacno!" << endl;
    }
    else
    {
        cout << "Resenje nije tacno!" << endl;
    }
    cout << endl;

    start = chrono::high_resolution_clock::now();
    tie(optimal_solution, generations) = genetic_serial(BOARD_SIZE, POPULATION_SIZE);
    stop = chrono::high_resolution_clock::now();

    cout << "Broj generacija: " << generations << endl;
    cout << "Resenje: ";
    print_vector(optimal_solution);

    duration = chrono::duration_cast<chrono::seconds>(stop - start);
    cout << "Vreme izvrsavanja serijskog algoritma: " << duration.count() << "s" << endl;
    cout << "Vreme po generaciji serijskog algoritma: " << duration.count() / (generations * 1.0) << "s" << endl;
    if (fitness_score(BOARD_SIZE, optimal_solution) == 0)
    {
        cout << "Resenje tacno!" << endl;
    }
    else
    {
        cout << "Resenje nije tacno!" << endl;
    }

    return 0;
}
