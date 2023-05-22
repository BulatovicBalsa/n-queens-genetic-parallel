#include "utils.hpp"

extern random_device device;
extern mt19937 rng;

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

void print_vector(vector<int> vec)
{
    for (int i = 0; i < vec.size(); ++i)
    {
        cout << vec[i] << " ";
    }

    cout << endl;
}