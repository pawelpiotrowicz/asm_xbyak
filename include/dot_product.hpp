#pragma once
#include "xbyak/xbyak.h"
#include "xbyak/xbyak_util.h"


namespace cpugraph {

    template <class T>
    struct dot_product; /* Dot product implementation */

    template <>
    struct dot_product<float> : Xbyak::CodeGenerator
    {

        const char *name() { return "paw_float"; }
        // rdi , rsi , rdx, rcx
        dot_product() /* my version */
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

       // template <class... P>
        float run(const float* l, const float* r, size_t size)
        {
            return ((float (*)(const float*,const float*, size_t))(this)->getCode())(l,r,size);
        }
    };

    template <>
    struct dot_product<double> : Xbyak::CodeGenerator
    {

        const char *name() { return "paw_double"; }
        // rdi , rsi , rdx, rcx
        dot_product() /* my version */
        {
            vxorpd(xmm0, xmm0, xmm0);
            test(rdx, rdx);
            je("end", T_NEAR);
            xor_(rax, rax);
            cmp(rdx, 8);
            jl("remainder");
            vxorpd(zmm3, zmm3, zmm3);
            vxorpd(zmm2, zmm2, zmm2);
            mov(r8, rdx);
            and_(r8, 8 - 1);
            sub(rdx, r8);
            L("full_chunk");
            vmovupd(zmm0, ptr[rdi + rax * 8]);
            vmovupd(zmm1, ptr[rsi + rax * 8]);
            vmulpd(zmm2, zmm1, zmm0);
            vaddpd(zmm3, zmm2, zmm3);
            add(rax, 8);
            cmp(rdx, rax);
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
            cmp(r8, 0);
            je("end");
            L("set_remainder");
            add(rdx, r8);
            L("remainder");
            vmovsd(xmm1, ptr[rdi + rax * 8]);
            vfmadd231sd(xmm0, xmm1, ptr[rsi + rax * 8]);
            inc(rax);
            cmp(rdx, rax);
            jne("remainder");
            L("end");
            ret();
        }

        double run(const double* l, const double* r, size_t size)
        {
            return ((double (*)(const double*,const double*,size_t))(this)->getCode())(l,r,size);
        }
    };
}
