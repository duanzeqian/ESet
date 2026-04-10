#include "src.hpp"

#include <vector>
#include <iostream>
#include <cmath>
#include <numeric>
#include <chrono>
#include <set>

using namespace std;
long long rd()
{
    long long rd1 = rand() % 30000, rd2 = rand() % 30000, rd3 = rand() % 30000;
    return rd1 * rd2 * rd3;
}

ESet <long long> s1[10005];
set <long long> s2[10005];
vector <double> data1, data2;
vector <long long> v;

int main()
{
    srand(time(nullptr));
    for (int i = 1; i <= 1000; ++i) v.push_back(rd() % 10000 + 1);
    for (int j = 1; j <= 10000; ++j)
    {
        auto start1 = chrono::high_resolution_clock::now();
        for (auto& val : v) s1[j].emplace(val);
        auto end1 = chrono::high_resolution_clock::now();
        auto start2 = chrono::high_resolution_clock::now();
        for (auto& val : v) s2[j].emplace(val);
        auto end2 = chrono::high_resolution_clock::now();
        double elapsed1 = chrono::duration<double, std::micro>(end1 - start1).count();
        double elapsed2 = chrono::duration<double, std::micro>(end2 - start2).count();
        data1.push_back(elapsed1), data2.push_back(elapsed2);
    }
    
    // cout << tot / 1000 / CLOCKS_PER_SEC << endl;

    double sum1 = accumulate(data1.begin(), data1.end(), 0.0);
    double mean1 = sum1 / data1.size();
    double sum2 = accumulate(data2.begin(), data2.end(), 0.0);
    double mean2 = sum2 / data2.size();

    // 计算方差
    double variance1 = 0.0, variance2 = 0.0;
    for (const auto& value : data1) {
    variance1 += pow(value - mean1, 2);
    }
    variance1 /= data1.size();
    for (const auto& value : data2) {
    variance2 += pow(value - mean2, 2);
    }
    variance2 /= data2.size();

    // 计算标准差
    double standard_deviation1 = sqrt(variance1);
    double standard_deviation2 = sqrt(variance2);
    
    cout << "ESet:" << endl << "mean: " << mean1 << endl << "standard_deviation: " << standard_deviation1 << endl;
    cout << endl;
    cout << "std:" << endl << "mean: " << mean2 << endl << "standard_deviation: " << standard_deviation2 << endl;
}