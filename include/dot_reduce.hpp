#pragma once
#include "xbyak/xbyak.h"
#include "xbyak/xbyak_util.h"

namespace cpugraph {

    template <class T>
    struct dot_reduce; /* Dot product implementation */

    template <>
    struct dot_reduce<float> : Xbyak::CodeGenerator
    {
        typedef float self_type;
        const char *name() { return "paw_reduce_float"; }
        dot_reduce()
        {
        }
        void run(self_type *out, const self_type *l, const self_type *r, size_t size_out, size_t reduce_size)
        {
            return ((void (*)(const self_type *, const self_type *, const self_type *, size_t, size_t))(this)->getCode())(out, l, r, size_out, reduce_size);
        }
    };

    template <>
    struct dot_reduce<double> : Xbyak::CodeGenerator
    {
        typedef double self_type;
        const char *name() { return "paw_reduce_double"; }
        dot_reduce()
        {

        }
        void run(self_type *out, const self_type *l, const self_type *r, size_t size_out, size_t reduce_size)
        {
            return ((void (*)(const self_type *, const self_type *, const self_type *, size_t, size_t))(this)->getCode())(out, l, r, size_out, reduce_size);
        }
    };




}