#include "algorithm.hpp"

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

// mora biti parno
int get_group_size(int n, int population_size)
{
    if (population_size <= 500)
    {
        if (n <= 60)
        {
            // 8-30, 6 za 1000 populaciju
            return 20;
        }
        else
        {
            return 10;
        }
    }
    else
    {
        if (n <= 60)
        {
            return 10;
        }
        else
        {
            return 6;
        }
    }
}

void mutate(vector<int> &child, int n)
{
    int mutation_point = get_random_int(0, n - 1);
    child[mutation_point] = get_random_int(0, n - 1);
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
        mutate(child1, n);
    }

    if (get_random_int(1, 10) <= 7)
    {
        mutate(child2, n);
    }

    return make_pair(child1, child2);
}

vector<pair<vector<int>, int>> next_generation(vector<pair<vector<int>, int>> population_scores, int start, int end, int n)
{
    vector<vector<int>> children;
    end = (population_scores.size() < end ? population_scores.size() : end);
    for (int i = 0; i < (end - start) / 2; i++)
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

pair<vector<int>, int> genetic_parallel(int n, int population_size)
{
    int group_size = get_group_size(n, population_size);
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

        for (int i = 0; i < population_size; i += group_size)
        {
            g.run([=, &parallel_res]
                  { parallel_res.push_back(next_generation(population_scores, i, i + group_size, n)); });
        }
        g.wait();

        vector<pair<vector<int>, int>> children_scores;

        for (int i = 0; i < parallel_res.size(); ++i)
        {
            children_scores.insert(children_scores.end(), parallel_res[i].begin(), parallel_res[i].end());
        }

        parallel_sort(population_scores.begin(), population_scores.end(), [](const pair<vector<int>, int> &ind1, const pair<vector<int>, int> &ind2)
                      { return ind1.second < ind2.second; });
        // we let 5% of the best parents live on - elitism
        int elitism_deg = population_size * 0.05;
        for (int i = 0; i < elitism_deg; i++)
        {
            if (children_scores[children_scores.size() - i - 1].second > population_scores[i].second)
            {
                swap(children_scores[children_scores.size() - i - 1], population_scores[i]);
            }
        }

        parallel_sort(children_scores.begin(), children_scores.end(), [](const pair<vector<int>, int> &ind1, const pair<vector<int>, int> &ind2)
                      { return ind1.second < ind2.second; });
        int best_child = children_scores[0].second;
        if (ECHO)
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

pair<vector<int>, int> genetic_serial(int n, int population_size)
{
    int generation = 0;
    int counter = 0;

    // inicijalizacija - kodiranje jedinki
    vector<vector<int>> population = init(n, population_size);
    // skladistimo fitness_score negde pa zip
    vector<pair<vector<int>, int>> population_scores;
    transform(population.begin(), population.end(), back_inserter(population_scores), [&](vector<int> &ind)
              { return make_pair(ind, fitness_score(n, ind)); });

    while (true)
    {
        counter += 1;
        // pravljenje dece - ukrstanje
        vector<vector<int>> children;
        for (int i = 0; i < population_size / 2; i++)
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

        generation += 1;

        // we let 5% of the best parents live on - elitism
        int elitism_deg = population_size * 0.05;

        vector<pair<vector<int>, int>> children_scores;
        transform(children.begin(), children.end(), back_inserter(children_scores), [&](vector<int> &ind)
                  { return make_pair(ind, fitness_score(n, ind)); });
        sort(children_scores.begin(), children_scores.end(), [](const pair<vector<int>, int> &ind1, const pair<vector<int>, int> &ind2)
             { return ind1.second < ind2.second; });
        sort(population_scores.begin(), population_scores.end(), [](const pair<vector<int>, int> &ind1, const pair<vector<int>, int> &ind2)
             { return ind1.second < ind2.second; });

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
        if (ECHO)
            cout << "\r" << counter << ". " << best_child << flush;
        if (best_child == 0)
        {
            cout << endl;
            return make_pair(children_scores[0].first, generation);
        }

        transform(children_scores.begin(), children_scores.end(), population_scores.begin(), [](const pair<vector<int>, int> &ind)
                  { return ind; });

        // clear children to avoid reallocating memory
        children.clear();
    }
}