#include <vector>
#include <random>
#include <tuple>
#include <algorithm>
#include <iostream>
#include <chrono>
#include "tbb/task_group.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/concurrent_vector.h"

// population size has to be even
#define POPULATION_SIZE 500
#define BOARD_SIZE 60

using namespace std;
using namespace tbb;

random_device device;
mt19937 rng(device());

int get_random_int(int min, int max)
{
    uniform_int_distribution<mt19937::result_type> dist(min, max);

    return dist(rng);
}

double get_random_real(double min, double max)
{
    uniform_real_distribution<double> dist(min, max);

    return dist(rng);
}

vector<vector<int>> init(int n, int population_size)
{
    vector<vector<int>> population;
    for (int i = 0; i < population_size; i++)
    {
        vector<int> individual;
        for (int j = 0; j < n; j++)
        {
            int genome = get_random_int(0, n - 1);
            individual.push_back(genome);
        }

        population.push_back(individual);
    }
    return population;
}

int fitness_score(int n, const vector<int> &individual)
{
    int res = 0;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (i == j)
            {
                continue;
            }

            // two queens are attacking each other if they are:
            // 1 - in the same column
            // 2 - on the same diagonal

            int x1 = i;
            int x2 = j;
            int y1 = individual[i];
            int y2 = individual[j];

            if (y1 == y2)
                res++;
            else if (abs(x1 - x2) == abs(y1 - y2))
                res++;
        }
    }

    // we counted each occasion twice
    return res / 2;
}

pair<vector<int>, vector<int>> crossover(int n, vector<int> parent1, vector<int> parent2)
{
    // random pivoting point
    int crossover_point = get_random_int(0, n - 1);

    vector<int> child1(parent1.begin(), parent1.begin() + crossover_point);
    child1.insert(child1.end(), parent2.begin() + crossover_point, parent2.end());

    vector<int> child2(parent2.begin(), parent2.begin() + crossover_point);
    child2.insert(child2.end(), parent1.begin() + crossover_point, parent1.end());

    // mutation - 70% chance of a random change in the child
    if (get_random_int(1, 10) <= 7)
    {
        int mutation_point = get_random_int(0, n - 1);
        child1[mutation_point] = get_random_int(0, n - 1);
    }

    if (get_random_int(1, 10) <= 7)
    {
        int mutation_point = get_random_int(0, n - 1);
        child2[mutation_point] = get_random_int(0, n - 1);
    }

    return make_pair(child1, child2);
}

vector<pair<vector<int>, int>> next_generation(vector<pair<vector<int>, int>> &population_scores, int start, int end, int n)
{
    vector<vector<int>> children;
    end = (population_scores.size() < end ? population_scores.size() : end);
    for (int i = start; i < end; i++)
    {
        // sortiranje po prilagodjenosti - selekcija
        vector<pair<vector<int>, double>> population_scores_roulette;
        transform(population_scores.begin(), population_scores.end(), back_inserter(population_scores_roulette), [&](pair<vector<int>, int> &ind)
                  { return make_pair(ind.first, ind.second * get_random_real(0.0, 1.0)); });
        sort(population_scores_roulette.begin(), population_scores_roulette.end(), [](const pair<vector<int>, double> &ind1, const pair<vector<int>, double> &ind2)
             { return ind1.second < ind2.second; });

        vector<int> child1, child2;
        tie(child1, child2) = crossover(n, population_scores_roulette[0].first, population_scores_roulette[1].first);
        children.push_back(child1);
        children.push_back(child2);
    }

    vector<pair<vector<int>, int>> children_scores;
    transform(children.begin(), children.end(), back_inserter(children_scores), [&](vector<int> &ind)
              { return make_pair(ind, fitness_score(n, ind)); });
    sort(children_scores.begin(), children_scores.end(), [](const pair<vector<int>, int> &ind1, const pair<vector<int>, int> &ind2)
         { return ind1.second < ind2.second; });

    // clear children to avoid reallocating memory
    children.clear();

    return children_scores;
}

pair<vector<int>, int> genetic(int n, int population_size)
{
    int group_size;

    if (n < 16)
    {
        group_size = 20;
    }
    else if (n < 30)
    {
        group_size = 10;
    }
    else if (n < 60)
    {
        group_size = 5;
    }
    else
    {
        group_size = 1;
    }

    int generation = 0;

    // inicijalizacija - kodiranje jedinki
    vector<vector<int>> population = init(n, population_size);
    // skladistimo fitness_score negde pa zip
    vector<pair<vector<int>, int>> population_scores(population.size());
    parallel_for(size_t(0), population.size(), [&](size_t i)
                 { population_scores[i] = make_pair(population[i], fitness_score(n, population[i])); });
    // transform(population.begin(), population.end(), back_inserter(population_scores), [&](vector<int> &ind)
    //           { return make_pair(ind, fitness_score(n, ind)); });
    task_group g;

    while (true)
    {
        generation++;
        concurrent_vector<vector<pair<vector<int>, int>>> parallel_res;

        parallel_for(blocked_range<int>(0, population_size), [&](blocked_range<int> range)
                     {
                    for(int i = range.begin(); i < range.end(); i+=group_size) {
                        g.run([&] { parallel_res.push_back(next_generation(population_scores, i, i + group_size, n));});
                    } });
        g.wait();

        vector<pair<vector<int>, int>> children_scores;

        for (int i = 0; i < parallel_res.size(); ++i)
        {
            children_scores.insert(children_scores.end(), make_move_iterator(parallel_res[i].begin()), make_move_iterator(parallel_res[i].end()));
        }

        sort(population_scores.begin(), population_scores.end(), [](const pair<vector<int>, int> &ind1, const pair<vector<int>, int> &ind2)
             { return ind1.second < ind2.second; });
        // pustamo 5% najboljih roditelja da prezive - elitizam
        int elitism_deg = population_size * 0.05;
        for (int i = 0; i < elitism_deg; i++)
        {
            if (children_scores[children_scores.size() - i - 1].second > population_scores[i].second)
            {
                swap(children_scores[children_scores.size() - i - 1], population_scores[i]);
            }
        }

        sort(children_scores.begin(), children_scores.end(), [](const pair<vector<int>, int> &ind1, const pair<vector<int>, int> &ind2)
             { return ind1.second < ind2.second; });
        int best_child = children_scores[0].second;
        cout << "\r" << generation << ". " << best_child << flush;
        if (best_child == 0)
        {
            cout << endl;
            return make_pair(children_scores[0].first, generation);
        }

        transform(children_scores.begin(), children_scores.end(), population_scores.begin(), [](const pair<vector<int>, int> &ind)
                  { return ind; });
    }
}

void print_vector(vector<int> vec)
{
    for (int i = 0; i < vec.size(); ++i)
    {
        cout << vec[i] << " ";
    }

    cout << endl;
}

int main()
{
    int generations;
    vector<int> optimal_solution;

    auto start = chrono::high_resolution_clock::now();
    tie(optimal_solution, generations) = genetic(BOARD_SIZE, POPULATION_SIZE);
    auto stop = chrono::high_resolution_clock::now();

    cout << "Broj generacija: " << generations << endl;
    cout << "Resenje: ";
    print_vector(optimal_solution);

    auto duration = chrono::duration_cast<chrono::seconds>(stop - start);
    cout << "Vreme izvrsavanja: " << duration.count() << "s" << endl;

    return 0;
}
