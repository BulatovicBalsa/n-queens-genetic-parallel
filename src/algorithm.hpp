#pragma once

#include <vector>
#include <tuple>
#include <algorithm>

#include "tbb/task_group.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/concurrent_vector.h"
#include "tbb/parallel_sort.h"

#include "utils.hpp"

#define ECHO false

using namespace std;
using namespace tbb;

vector<vector<int>> init(int n, int population_size);

int fitness_score(int n, const vector<int> &individual);

int get_group_size(int n);

pair<vector<int>, vector<int>> crossover(int n, vector<int> parent1, vector<int> parent2);

vector<pair<vector<int>, int>> next_generation(vector<pair<vector<int>, int>> &population_scores, int start, int end, int n);

pair<vector<int>, int> genetic_parallel(int n, int population_size);

pair<vector<int>, int> genetic_serial(int n, int population_size);