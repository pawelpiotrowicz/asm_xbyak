
#include "dot_reduce.hpp"


namespace gcc {


}



template<class T>
void test_reduce_gcc_without_avx(T *out, T* l, T *r, size_t size_out, size_t size_reduce = 1 ) {

      for(size_t j=0;j<size_out;j++)
      {
        T ret =0;
        for(size_t i=0;i<size_reduce;++i)
          ret += l[j * size_reduce + i] * r[j * size_reduce + i];
        out[j]=ret;
      }

}


template<class T>
struct Content {
  std::vector<T> Out;
  std::vector<T> Left;
  std::vector<T> Right;
  size_t reduce_size;
  size_t getSize() { return Out.size(); }

  Content(const std::vector<T>& _L, const std::vector<T>& _R, size_t _reduce_size=3)  {

           if( (_L.size() != _R.size()) || !_L.size() || !_R.size() )
           {
             log_err("Incorrect size L&R " << _L.size());
           }

           if ( !_reduce_size ||  (_L.size() % _reduce_size) )
           {
             log_err("Incorrect reduce size "<< _reduce_size << " for " << _L.size());
           }

           reduce_size = _reduce_size;

           Out.resize(_L.size() / _reduce_size);
           std::fill(Out.begin(),Out.end(),0);

           Left = _L;
           Right = _R;
  }

};

template<class T>
std::ostream& operator<<(std::ostream& o, const Content<T>& c) {

    o << "ReduceSize="<< c.reduce_size << " ";
    o << cpugraph::join(c.Out,[](T v) { return v;},4);

  return o;
}


template<class T>
void run_case()
{
  cpugraph::dot_reduce<T> my;

  std::vector<Content<T>> v;
  v.push_back(Content<T>({1.0, 2.0, 3.0}, {1.0, 2.0, 3.0}, 1));
  v.push_back(Content<T>({1.0, 2.0, 3.0}, {1.0, 2.0, 3.0}, 3));

  for (auto &item : v)
  {

    auto beg = std::chrono::high_resolution_clock::now();
    test_reduce_gcc_without_avx((T *)item.Out.data(), (T *)item.Left.data(), (T *)item.Right.data(), item.Out.size(), item.reduce_size);
    auto end = std::chrono::high_resolution_clock::now();
    auto diff_duration_reduce_gcc = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);

    auto CopyGccOutput = item.Out;
    std::fill(item.Out.begin(),item.Out.end(),0);

    beg = std::chrono::high_resolution_clock::now();
    my.run((T *)item.Out.data(), (T *)item.Left.data(), (T *)item.Right.data(), item.Out.size(), item.reduce_size);
    end = std::chrono::high_resolution_clock::now();
    auto diff_duration_my = std::chrono::duration_cast<std::chrono::nanoseconds>(end - beg);

    for(size_t i=0;i<CopyGccOutput.size();i++)
    {
      if (!cpugraph::AreSame<T>(CopyGccOutput[i], item.Out[i]))
      {
        log_err("type=" << cpugraph::getTypeName<T>::name()  << " Not equal vec_size=" << CopyGccOutput.size() << " reduction=" << item.reduce_size << "  CopyGccOutput[" << i << "] != My[" << i << "] => {" << CopyGccOutput[i] << " != " << item.Out[i] << "}");
      }
    }

    log_info("OK :) "<< "type=" << cpugraph::getTypeName<T>::name() << " vec_size{L,R}=" << item.Right.size() << " and reduction=" << item.reduce_size );
   }

}



int main(int argc, char **argv) {


   run_case<float>();
 // run_case<double>();



  return 0;
}
