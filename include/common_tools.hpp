#pragma once
#include <vector>
#include <chrono>
#include <limits>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <random>
#include <iostream>
#include <algorithm>

namespace cpugraph {

template <class T>
void GenerateRandomData(T *data, int dim)
{
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, 10000);
    for (int i = 0; i < dim; i++)
    {
        data[i] = (dist(rng) / 100);
    }
}

template <class T>
bool AreSame(T a, T b)
{
    // std::cout << "FABS=" << std::fabs(a - b) << std::endl;
    return (std::is_same<T, float>::value) ? (std::fabs(a - b) < std::numeric_limits<T>::epsilon()) : std::fabs(a - b) < 0.0000000000001;
    //  return  std::fabs(a - b) < std::numeric_limits<T>::epsilon();
}

template <class V, class F>
std::string join(V &vec, F f, size_t sw = 11, std::string pre = " {", std::string post = "}")
{
    std::stringstream ss;
    ss << pre;
    for (auto &v : vec)
    {
        ss << std::setw(sw) << std::setfill(' ') << f(v) << ",";
    }
    ss << post;
    return ss.str();
}

template <class T>
struct getTypeName
{
    static const char *name() { return "<unknown>"; }
};

template <>
struct getTypeName<float>
{
    static const char *name() { return "<float>"; }
};

template <>
struct getTypeName<double>
{
    static const char *name() { return "<double>"; }
};


}

#define log_err(x) { std::cout <<"[ERROR] " << x << std::endl; exit(1); }
#define log_info(x) std::cout << "[INFO] " << x << std::endl;