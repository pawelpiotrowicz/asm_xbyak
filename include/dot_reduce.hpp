#pragma once
#include "xbyak/xbyak.h"
#include "xbyak/xbyak_util.h"
#include "common_tools.hpp"

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
           // out=RDI, L=RSI, R=RDX, size_out=RCX, size_reduce=R8
            // push(r10);
            // xor_(r10, r10);
            // L("for_i");
            // dot_product_float(rsi, rdx, r8); // L, R, reduce_size
            // vmovss(ptr[rdi + r10 * 4], xmm0);
            // inc(r10);
            // add(rsi,4);
            // add(rdx,4);
            // cmp(r10, rcx);
            // jne("for_i");
            // pop(r10);
            // ret();

           push(r10);
           push(r14);
           push(r15);
           xor_(r10, r10);
           xor_(r11, r11);
           mov(r14, rsi);
           mov(r15, rdx);
           L("for_i");
           dot_product_float(rsi, rdx, r8); // L, R, reduce_size
           vmovss(ptr[rdi + r10 * 4], xmm0);
           add(r11, r8);
           inc(r10);
           lea(rsi, ptr[r14 + r11 * 4]);
           lea(rdx, ptr[r15 + r11 * 4]);
           cmp(r10, rcx);
           jne("for_i");
           pop(r15);
           pop(r14);
           pop(r10);
           ret();
        }

        void dot_product_float(const Xbyak::Reg64 &reg_L, const Xbyak::Reg64 &reg_R, const Xbyak::Reg64 &reg_size)
        {
            vxorps(xmm0, xmm0, xmm0);
            test(reg_size, reg_size);
            je("end", T_NEAR);
            xor_(rax, rax);
            cmp(reg_size, 16);
            jl("remainder");
            vxorps(zmm3, zmm3, zmm3);
            vxorps(zmm2, zmm2, zmm2);
            mov(r9, reg_size);
            and_(r9, 16 - 1);
            sub(reg_size, r9);
            L("full_chunk");
            vmovups(zmm0, ptr[reg_L + rax * 4]);
            vmovups(zmm1, ptr[reg_R + rax * 4]);
            vmulps(zmm2, zmm1, zmm0);
            vaddps(zmm3, zmm2, zmm3);
            add(rax, 16);
            cmp(reg_size, rax);
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
            cmp(r9, 0);
            je("end");
            L("set_remainder");
            add(reg_size, r9);
            L("remainder");
            vmovss(xmm1, ptr[reg_L + rax * 4]);
            vfmadd231ss(xmm0, xmm1, ptr[reg_R + rax * 4]);
            inc(rax);
            cmp(reg_size, rax);
            jne("remainder");
            L("end");

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
            // out=RDI, L=RSI, R=RDX, size_out=RCX, size_reduce=R8
            // push(r10);
            // xor_(r10, r10);
            // L("for_i");
            // dot_product_double(rsi, rdx, r8); // L, R, reduce_size
            // vmovsd(ptr[rdi + r10 * 8], xmm0);
            // inc(r10);
            // add(rsi, 8);
            // add(rdx, 8);
            // cmp(r10, rcx);
            // jne("for_i");
            // pop(r10);
            // ret();

            push(r10);
            push(r14);
            push(r15);
            xor_(r10, r10);
            xor_(r11, r11);
            mov(r14, rsi);
            mov(r15, rdx);
            L("for_i");
            dot_product_double(rsi, rdx, r8); // L, R, reduce_size
            vmovsd(ptr[rdi + r10 * 8], xmm0);
            add(r11, r8);
            inc(r10);
            lea(rsi, ptr[r14 + r11 * 8]);
            lea(rdx, ptr[r15 + r11 * 8]);
            cmp(r10, rcx);
            jne("for_i");
            pop(r15);
            pop(r14);
            pop(r10);
            ret();
        }

        void dot_product_double(const Xbyak::Reg64 &reg_L, const Xbyak::Reg64 &reg_R, const Xbyak::Reg64 &reg_size)
        {
            vxorpd(xmm0, xmm0, xmm0);
            test(reg_size, reg_size);
            je("end", T_NEAR);
            xor_(rax, rax);
            cmp(reg_size, 8);
            jl("remainder");
            vxorpd(zmm3, zmm3, zmm3);
            vxorpd(zmm2, zmm2, zmm2);
            mov(r9, reg_size);
            and_(r9, 8 - 1);
            sub(reg_size, r9);
            L("full_chunk");
            vmovupd(zmm0, ptr[reg_L + rax * 8]);
            vmovupd(zmm1, ptr[reg_R + rax * 8]);
            vmulpd(zmm2, zmm1, zmm0);
            vaddpd(zmm3, zmm2, zmm3);
            add(rax, 8);
            cmp(reg_size, rax);
            jne("full_chunk");
            vextractf64x4(ymm0, zmm3, 0x0);
            vextractf64x4(ymm1, zmm3, 0x1);
            vaddpd(ymm3, ymm1, ymm0);
            vextractf128(xmm1, ymm3, 0x0);
            vextractf128(xmm2, ymm3, 0x1);
            vaddpd(xmm0, xmm1, xmm2);
            movapd(xmm1, xmm0);
            shufpd(xmm1, xmm1, 0x1);
            vaddpd(xmm0, xmm1, xmm0);
            cmp(r9, 0);
            je("end");
            L("set_remainder");
            add(reg_size, r9);
            L("remainder");
            vmovsd(xmm1, ptr[reg_L + rax * 8]);
            vfmadd231sd(xmm0, xmm1, ptr[reg_R + rax * 8]);
            inc(rax);
            cmp(reg_size, rax);
            jne("remainder");
            L("end");
        }
            void run(self_type * out, const self_type *l, const self_type *r, size_t size_out, size_t reduce_size){
                return ((void (*)(const self_type *, const self_type *, const self_type *, size_t, size_t))(this)->getCode())(out, l, r, size_out, reduce_size);
        }
    };




}