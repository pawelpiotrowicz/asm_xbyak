#include <iostream>
#include "xbyak/xbyak.h"
#include "xbyak/xbyak_util.h"
#include <vector>
#include <chrono>
#include <limits>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <random>
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

namespace my {
  template <class T>
  struct Code; /* Dot product implementation */

  template <>
  struct Code<float> : Xbyak::CodeGenerator
  {

    const char *name() { return "pawel_ver"; }
    // rdi , rsi , rdx, rcx
    Code() /* my version */
    {
      vxorps(xmm0, xmm0, xmm0);
      test(rdx, rdx);
      je("end", T_NEAR);
      xor_(rax, rax);
      cmp(rdx, 16);
      jl("remainder");
      vxorps(zmm3, zmm3, zmm3);
      vxorps(zmm2, zmm2, zmm2);
      mov(r8, rdx);
      and_(r8, 16 - 1);
      sub(rdx, r8);
      L("full_chunk");
      vmovups(zmm0, ptr[rdi + rax * 4]);
      vmovups(zmm1, ptr[rsi + rax * 4]);
      vmulps(zmm2, zmm1, zmm0);
      vaddps(zmm3, zmm2, zmm3);
      add(rax, 16);
      cmp(rdx, rax);
      jne("full_chunk");
      vextractf64x4(ymm0, zmm3, 0x0);
      vextractf64x4(ymm1, zmm3, 0x1);
      vaddps(ymm3, ymm1, ymm0);
      vextractf128(xmm1, ymm3, 0x0);
      vextractf128(xmm2, ymm3, 0x1);
      vaddps(xmm0, xmm1, xmm2);
      vshufps(xmm1, xmm0, xmm0, 0xb1);
      vaddps(xmm0, xmm1, xmm0);
      vshufps(xmm1, xmm0, xmm0, 0x02);
      vaddps(xmm0, xmm1, xmm0);
      cmp(r8, 0);
      je("end");
      L("set_remainder");
      add(rdx, r8);
      L("remainder");
      vmovss(xmm1, ptr[rdi + rax * 4]);
      vfmadd231ss(xmm0, xmm1, ptr[rsi + rax * 4]);
      inc(rax);
      cmp(rdx, rax);
      jne("remainder");
      L("end");
      ret();
    }

    template <class... P>
    float run(P... args)
    {
      return ((float (*)(P...))(this)->getCode())(args...);
    }
};

}

namespace gcc {

  template <class T>
  struct Code;

  template <>
  struct Code<float> : Xbyak::CodeGenerator
  {
    const char* name() { return "g++AVX512"; }
    // rdi , rsi , rdx, rcx
    Code() /* gcc version */
    {
      test(rdx, rdx);
      je("is_zero");
      xor_(rax, rax);
      vxorps(xmm0, xmm0, xmm0);
      L("again");
      vmovss(xmm1, ptr[rdi + rax * 4]);
      vfmadd231ss(xmm0, xmm1, ptr[rsi + rax * 4]);
      inc(rax);
      cmp(rdx, rax);
      jne("again");
      ret();
      L("is_zero");
      vxorps(xmm0, xmm0, xmm0);
      ret();
    }

    template <class... P>
    float run(P... args)
    {
      return ((float (*)(P...))(this)->getCode())(args...);
    }
};

}

#define log_err(x) { std::cout <<"[ERROR] " << x << std::endl; exit(1); }
#define log_info(x) std::cout << "[INFO] " << x << std::endl;

template<class T>
T test_dot_prod(const std::vector<T>& v1, const std::vector<T>& v2) {

         T ret =0;
          for(size_t i=0;i<v1.size();++i)
	           	ret+=v1[i]*v2[i];

	 return ret;

}


template<class T>
struct getTypeName {
   static const char* name() { return "<unknown>"; }
};

template<>
struct getTypeName<float> {
  static const char *name() { return "<float>"; }
};

template <>
struct getTypeName<double>
{
  static const char *name() { return "<double>"; }
};

template<class T>
bool AreSame(T a, T b)
{
  return std::fabs(a - b) < std::numeric_limits<T>::epsilon();
}
template<class V>
std::string winners(V& vec)
{
    std::stringstream ss;
    ss << " {";
    for(auto &v : vec)
    {
        ss << std::setw(11) << std::setfill(' ') << v.second <<",";
    }
    ss << "}";
    return ss.str();
}

template <class T, class G1, class G2>
T dot_prod(const std::vector<T> v1, const std::vector<T> v2, G1 &g1, G2 &g2)
{

  if (v1.size() != v2.size())
  {
    log_err("Incorrect size v1 & v2 diff=" << v1.size() - v2.size());
  }

  T test_value = 9.0;

  auto beg = std::chrono::high_resolution_clock::now();
  test_value = test_dot_prod(v1, v2);
  auto end = std::chrono::high_resolution_clock::now();
  auto diff_duration_org = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);

  T ret = 0;
  beg = std::chrono::high_resolution_clock::now();
  ret = g1.run(v1.data(), v2.data(), v1.size());
  end = std::chrono::high_resolution_clock::now();
  auto diff_duration_g1 = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);

  if (!AreSame<T>(ret, test_value))
  {
    log_err("Incorrect values " << getTypeName<T>::name() << "  " << ret << " != " << test_value << "  vector_size=" << v1.size() <<  " name="<< g1.name());
  }

  beg = std::chrono::high_resolution_clock::now();
  ret = g2.run(v1.data(), v2.data(), v1.size());
  end = std::chrono::high_resolution_clock::now();
  auto diff_duration_g2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);

  std::vector<std::pair<int64_t, std::string>> result{
      {diff_duration_org.count(), "dgl-g++-O2"},
      {diff_duration_g1.count(), g1.name()},
      {diff_duration_g2.count(), g2.name()}};

  std::sort(result.begin(), result.end(), [](std::pair<int64_t, std::string> &p1, std::pair<int64_t, std::string>& p2) { return p1.first<p2.first; });

  if (!AreSame<T>(ret, test_value))
  {
    log_err("Incorrect values " << getTypeName<T>::name() << "  " << ret << " != " << test_value << "  vector_size=" << v1.size() << " name=" << g1.name());
  }

   auto pair_best = result[0];
   auto pair_middle = result[1];
   auto pair_worst = result[2];

   int proc_middle = (int)((((double)pair_middle.first / (double)pair_best.first) - 1) * 100);
   int proc_worst = (int)((((double)pair_worst.first / (double)pair_best.first) - 1 ) * 100);
   #define pad(x) std::setw(x) << std::setfill(' ')
   log_info("OK " << getTypeName<T>::name() << " size=" << pad(4) << v1.size() << winners(result) << " " << pad(6) << proc_middle << "%" << pad(6) << proc_worst << "%   "
                  << "times={" << pad(5) << pair_best.first << "," << pad(5) << pair_middle.first << "," << pad(5) << pair_worst.first<< " }" );

   return ret;
}

template <class T>
using my_pair = std::pair<std::vector<T>, std::vector<T>>;

template <class T>
using vec_pair = std::vector<my_pair<T>>;

int main(int argc, char **argv) {

  vec_pair<float> vec_pair_float;
  vec_pair_float.emplace_back(my_pair<float>());
  vec_pair_float.emplace_back(my_pair<float>({4.4}, {4.5}));
  vec_pair_float.emplace_back(my_pair<float>({3.0, 1.3, 3.0}, {4.0, 1.3, 3.0}));
  vec_pair_float.emplace_back(my_pair<float>({3.0, 1.3, 3.0,6.0}, {4.0, 1.3, 3.0,5.0}));
  vec_pair_float.emplace_back(my_pair<float>({3.0, 1.3, 3.0,2.1,2.3}, {4.0, 1.3, 3.0,3.4,3.2}));
  vec_pair_float.emplace_back(my_pair<float>({1.0,1.3,3.0,1.0,1.3,3.0,1.0,1.0,1.0,1.3,3.0,1.0,1.3,3.0,1.0,1.0,},{1.0,1.3,3.0,1.0,1.3,3.0,1.0,1.0,1.0,1.3,3.0,1.0,1.3,3.0,1.0,1.0,}));
  vec_pair_float.emplace_back(my_pair<float>({1.0,
                                              1.0,
                                              1.3,
                                              3.0,
                                              1.0,
                                              1.2,
                                              3.0,
                                              1.0,
                                              1.0,
                                              1.0,
                                              1.2,
                                              3.0,
                                              1.0,
                                              1.2,
                                              3.0,
                                              1.0,
                                              1.0,
                                              9, 14, 1.3,
                                              3.0,
                                              1.0,
                                              1.2,
                                              3.0,
                                              1.0,
                                              1.0,
                                              1.0,
                                              1.2,
                                              3.0,
                                              1.0,
                                              1.2,
                                              3.0,
                                              1.0,
                                              1.0,
                                              9, 14},
                                             {1.0,
                                              1.3,
                                              3.0,
                                              1.0,
                                              1.2,
                                              3.0,
                                              1.0,
                                              1.0,
                                              1.0,
                                              1.2,
                                              3.0,
                                              1.0,
                                              1.2,
                                              3.0,
                                              1.0,
                                              1.0,
                                              9, 14, 1.0,
                                              1.3,
                                              3.0,
                                              1.0,
                                              1.2,
                                              3.0,
                                              1.0,
                                              1.0,
                                              1.0,
                                              1.2,
                                              3.0,
                                              1.0,
                                              1.2,
                                              3.0,
                                              1.0,
                                              1.0,
                                              9, 14}));


  for(int i=64;i<1024;i+=64)
  {
    {
    my_pair<float> pair;
    pair.first.resize(i);
    pair.second.resize(i);
    GenerateRandomData((float*)pair.first.data(), i);
    GenerateRandomData((float *)pair.second.data(), i);
    vec_pair_float.emplace_back(pair);
    }

    {
      my_pair<float> pair;
      pair.first.resize(i+2);
      pair.second.resize(i+2);
      GenerateRandomData((float *)pair.first.data(), i+2);
      GenerateRandomData((float *)pair.second.data(), i+2);
      vec_pair_float.emplace_back(pair);
    }
  }



  my::Code<float> my_c; /* my version */

  gcc::Code<float> gcc_c;  /* gcc version */

  for (auto v : vec_pair_float)
    dot_prod<float>(v.first, v.second, my_c, gcc_c);


  return 0;
}
