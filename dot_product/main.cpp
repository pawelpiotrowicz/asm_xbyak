#include "dot_product.hpp"

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

template <>
struct Code<double> : Xbyak::CodeGenerator
{
  const char *name() { return "g++AVX512"; }
  // rdi , rsi , rdx, rcx
  Code() /* gcc version */
  {
    test(rdx, rdx);
    je("is_zero");
    xor_(rax, rax);
    vxorpd(xmm0, xmm0, xmm0);
    L("again");
    vmovsd(xmm1, ptr[rdi + rax * 8]);
    vfmadd231sd(xmm0, xmm1, ptr[rsi + rax * 8]);
    inc(rax);
    cmp(rdx, rax);
    jne("again");
    ret();
    L("is_zero");
    vxorpd(xmm0, xmm0, xmm0);
    ret();
  }

  template <class... P>
  double run(P... args)
  {
    return ((double (*)(P...))(this)->getCode())(args...);
  }
};
}

template<class T>
T test_dot_prod_gcc_without_avx(const std::vector<T>& v1, const std::vector<T>& v2) {

         T ret =0;
          for(size_t i=0;i<v1.size();++i)
	           	ret+=v1[i]*v2[i];

	 return ret;

}





template <class T, class G1, class G2>
T dot_prod(const std::vector<T> v1, const std::vector<T> v2, G1 &our_AVX, G2 &gcc_AVX)
{

  if (v1.size() != v2.size())
  {
    log_err("Incorrect size v1 & v2 diff=" << v1.size() - v2.size());
  }

  T test_value = 9.0;

  auto beg = std::chrono::high_resolution_clock::now();
  test_value = test_dot_prod_gcc_without_avx(v1, v2);
  auto end = std::chrono::high_resolution_clock::now();
  auto diff_duration_org = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);

  T ret_gcc_AVX = 0;
  beg = std::chrono::high_resolution_clock::now();
  ret_gcc_AVX = gcc_AVX.run(v1.data(), v2.data(), v1.size());
  end = std::chrono::high_resolution_clock::now();
  auto diff_duration_gcc_AVX = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);

  if (!cpugraph::AreSame<T>(ret_gcc_AVX, test_value))
  {
    log_err("Incorrect values gcc_AVX vs GCC_without_avx -" << cpugraph::getTypeName<T>::name() << "  " << ret_gcc_AVX << " != " << test_value << "  vector_size=" << v1.size() << " name=" << gcc_AVX.name());
  }

  T ret_our_AVX = 0;
  beg = std::chrono::high_resolution_clock::now();
  ret_our_AVX = our_AVX.run(v1.data(), v2.data(), v1.size());
  end = std::chrono::high_resolution_clock::now();
  auto diff_duration_our_AVX = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);

  if (!cpugraph::AreSame<T>(ret_our_AVX, test_value))
  {
    log_err("Incorrect values Our vs GCC_without_avx- " << cpugraph::getTypeName<T>::name() << "  " << std::setprecision(16)<< ret_our_AVX << " != " << test_value << "  vector_size=" << v1.size() << " name=" << our_AVX.name() << " epsilon=" << std::numeric_limits<T>::epsilon() << " abs=" << std::fabs(ret_our_AVX - test_value));
  }

  std::vector<std::pair<int64_t, std::string>> result{
      {diff_duration_org.count(), "dgl-g++-O2"},
      {diff_duration_our_AVX.count(), our_AVX.name()},
      {diff_duration_gcc_AVX.count(), gcc_AVX.name()}};

  std::sort(result.begin(), result.end(), [](std::pair<int64_t, std::string> &p1, std::pair<int64_t, std::string>& p2) { return p1.first<p2.first; });


   auto pair_best = result[0];
   auto pair_middle = result[1];
   auto pair_worst = result[2];

   int proc_middle = (int)((((double)pair_middle.first / (double)pair_best.first) - 1) * 100);
   int proc_worst = (int)((((double)pair_worst.first / (double)pair_best.first) - 1 ) * 100);
   #define pad(x) std::setw(x) << std::setfill(' ')
   log_info("OK " << cpugraph::getTypeName<T>::name() << " size=" << pad(4) << v1.size() << cpugraph::join(result, [](std::pair<int64_t, std::string> &in) { return in.second; }) << " " << pad(6) << proc_middle << "%" << pad(6) << proc_worst << "%   "
                  << "times={" << pad(5) << pair_best.first << "," << pad(5) << pair_middle.first << "," << pad(5) << pair_worst.first << " }");

   return ret_our_AVX;
}

template <class T>
using my_pair = std::pair<std::vector<T>, std::vector<T>>;

template <class T>
using vec_pair = std::vector<my_pair<T>>;


template<class T>
void run_case() {

  vec_pair<T> vec_pair_tab;
  vec_pair_tab.emplace_back(my_pair<T>());
  vec_pair_tab.emplace_back(my_pair<T>({4.4}, {4.51}));
  vec_pair_tab.emplace_back(my_pair<T>({3.0, 1.3, 3.0}, {4.0, 1.3, 3.0}));
  vec_pair_tab.emplace_back(my_pair<T>({3.0, 1.3, 3.0, 6.0}, {4.0, 1.3, 3.0, 5.0}));
  vec_pair_tab.emplace_back(my_pair<T>({3.0, 1.3, 3.0, 2.1, 2.3}, {4.0, 1.3, 3.0, 3.4, 3.2}));
  vec_pair_tab.emplace_back(my_pair<T>({3.0, 1.3, 3.0, 2.1, 3.0, 1.3, 3.0, 2.1}, {3.0, 1.3, 3.0, 2.1, 1.3, 3.0, 3.4, 3.2}));
  vec_pair_tab.emplace_back(my_pair<T>({
                                             1.0,
                                             1.3,
                                             3.0,
                                             1.0,
                                             1.3,
                                             3.0,
                                             1.0,
                                             1.0,
                                             1.0,
                                             1.3,
                                             3.0,
                                             1.0,
                                             1.3,
                                             3.0,
                                             1.0,
                                             1.0,
                                         },
                                         {
                                             1.0,
                                             1.3,
                                             3.0,
                                             1.0,
                                             1.3,
                                             3.0,
                                             1.0,
                                             1.0,
                                             1.0,
                                             1.3,
                                             3.0,
                                             1.0,
                                             1.3,
                                             3.0,
                                             1.0,
                                             1.0,
                                         }));
  vec_pair_tab.emplace_back(my_pair<T>({1.0,
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

  for (int i = 64; i < 1024; i += 64)
  {


    {
      my_pair<T> pair;
      pair.first.resize(i);
      pair.second.resize(i);
      cpugraph::GenerateRandomData((T *)pair.first.data(), i);
      cpugraph::GenerateRandomData((T *)pair.second.data(), i);
      vec_pair_tab.emplace_back(pair);
    }

    {
      my_pair<T> pair;
      pair.first.resize(i + 2);
      pair.second.resize(i + 2);
      cpugraph::GenerateRandomData((T *)pair.first.data(), i + 2);
      cpugraph::GenerateRandomData((T *)pair.second.data(), i + 2);
      vec_pair_tab.emplace_back(pair);
    }
  }

  cpugraph::dot_product<T> my_c; /* my version */

  gcc::Code<T> gcc_c; /* gcc version */

  for (auto v : vec_pair_tab)
    dot_prod<T>(v.first, v.second, my_c, gcc_c);
}




int main(int argc, char **argv) {

 run_case<float>();
 run_case<double>();



  return 0;
}
