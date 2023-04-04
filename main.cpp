#include <vector>
#include <random>
#include <tuple>
#include <algorithm>
#include <iostream>

using namespace std;

vector<vector<int>> init(int n, int population_n) {
    vector<vector<int>> population;
    for (int i = 0; i < population_n; i++)
    {
        vector<int> individual;
        for (int i = 0; i < n; i++)
        {
            int gen = rand() % n;
            individual.push_back(gen); 
        }
        
        population.push_back(individual);
    }
    return population;
}

int fitness_score(int n, vector<int> individual) {
    int res = 0;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (i == j)
            {
                continue;   
            }
            int x1 = i;
            int x2 = j;
            int y1 = individual[i];
            int y2 = individual[j];

            if (y1 == y2){
                res++;
            }
            else
            {
                if (abs(x1 - x2) == abs(y1 - y2)) {
                    res++;
                }
            }
        }
    }
    return res / 2;
}

pair<vector<int>, vector<int>> crossover(int n, vector<int> parent1, vector<int> parent2) {
  // random pivoting point
  int crossover_point = rand() % n;

  vector<int> child1(parent1.begin(), parent1.begin() + crossover_point);
  child1.insert(child1.end(), parent2.begin() + crossover_point, parent2.end());

  vector<int> child2(parent2.begin(), parent2.begin() + crossover_point);
  child2.insert(child2.end(), parent1.begin() + crossover_point, parent1.end());

  // mutation - 70% chance of random change in child
  if (static_cast<double>(rand()) / RAND_MAX > 0.3) {
    int mutation_point = rand() % n;
    child1[mutation_point] = rand() % n;
  }

  if (static_cast<double>(rand()) / RAND_MAX > 0.3) {
    int mutation_point = rand() % n;
    child2[mutation_point] = rand() % n;
  }

  return make_pair(child1, child2);
}

bool compare(const pair<vector<int>, double>& p1, const pair<vector<int>, double>& p2) {
    return p1.second * rand() < p2.second * rand();
}


// pair<vector<int>, int> genetic2(int n, int population_n) {
//     int generation = 0;
//     int counter = 0;
// 
//     auto population = init(n, population_n);
//     
//     vector<int> population_scores;
//     for (const auto& ind : population) {
//         int score = fitness_score(n, ind);
//         population_scores.push_back(score);
//     }
// 
//     vector<pair<vector<int>, double>> population_zipped;
//     for (int i = 0; i < population.size(); ++i) {
//         population_zipped.push_back(std::make_pair(population[i], population_scores[i]));
//     }
// 
//     while (true)
//     {
//         counter++;
//         vector<vector<int>> children;
//         for (int i = 0; i < population_n/2; i++)
//         {
//             sort(population_zipped.begin(), population_zipped.end(), compare);
//             vector<int> child1, child2;
//             tie(child1, child2) = crossover(n, population_zipped[0][0], population_zipped[1][0]);
//             children.push_back(child1);
//             children.push_back(child2);
//         }
//         generation++;
// 
//         int elitism_deg = population_n*0.05;
// 
//         vector<int> children_scores;
//         for (const auto& ind : children) {
//             int score = fitness_score(n, ind);
//             population_scores.push_back(score);
//         }
// 
// 
//     }
//     
// }


pair<vector<int>, int> genetic(int n, int population_n) {
    int generation = 0;
    int counter = 0;

    // inicijalizacija - kodiranje jedinki
    vector<vector<int>> population = init(n, population_n);
    // skladistimo fitness_score negde pa zip
    vector<pair<vector<int>, int>> population_scores;
    transform(population.begin(), population.end(), back_inserter(population_scores), [&](vector<int>& ind) { return make_pair(ind, fitness_score(n, ind)); });

    while (true) {
        counter += 1;
        // pravljenje dece - ukrstanje
        vector<vector<int>> children;
        for (int i = 0; i < population_n / 2; i++) {
            // sortiranje po prilagodjenosti - selekcija
            sort(population_scores.begin(), population_scores.end(), [](const pair<vector<int>, int>& ind1, const pair<vector<int>, int>& ind2) { return ind1.second * rand() / RAND_MAX < ind2.second * rand() / RAND_MAX; });

            vector<int> child1, child2;
            tie(child1, child2) = crossover(n, population_scores[0].first, population_scores[1].first);
            children.push_back(child1);
            children.push_back(child2);
        }

        generation += 1;

        // pustamo 5% najboljih roditelja da prezive - elitizam
        int elitism_deg = population_n * 0.05;

        vector<pair<vector<int>, int>> children_scores;
        transform(children.begin(), children.end(), back_inserter(children_scores), [&](vector<int>& ind) { return make_pair(ind, fitness_score(n, ind)); });
        sort(children_scores.begin(), children_scores.end(), [](const pair<vector<int>, int>& ind1, const pair<vector<int>, int>& ind2) { return ind1.second < ind2.second; });
        sort(population_scores.begin(), population_scores.end(), [](const pair<vector<int>, int>& ind1, const pair<vector<int>, int>& ind2) { return ind1.second < ind2.second; });

        for (int i = 0; i < elitism_deg; i++) {
            if (children_scores[children_scores.size() - i - 1].second > population_scores[i].second) {
                swap(children_scores[children_scores.size() - i - 1], population_scores[i]);
            }
        }

        sort(children_scores.begin(), children_scores.end(), [](const pair<vector<int>, int>& ind1, const pair<vector<int>, int>& ind2) { return ind1.second < ind2.second; });
        int best_child = children_scores[0].second;
        cout << "\r" << counter << ". " << best_child << flush;
        if (best_child == 0) {
            cout << endl;
            return make_pair(children_scores[0].first, generation);
        }

        transform(children_scores.begin(), children_scores.end(), population_scores.begin(), [](const pair<vector<int>, int>& ind) { return ind; });

        // clear children to avoid reallocating memory
        children.clear();
    }
}

int main() {
    int n;
    cout << "Unesite n" << endl;
    cin >> n;

    int generations;
    vector<int> optimal_solution;
    tie(optimal_solution, generations) = genetic(n, 50);

    cout << "Broj generacija: " << generations << endl;
    cout << "Resenje: ";
    for (int i = 0; i < n; i++) {
        cout << optimal_solution[i] << " ";
    }
    cout << endl;

    //draw(optimal_solution, n);

    return 0;
}



