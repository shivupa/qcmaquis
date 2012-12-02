/*
*Very Large Integer Library, License - Version 1.0 - May 3rd, 2012
*
*Timothee Ewart - University of Geneva, 
*Andreas Hehn - Swiss Federal Institute of technology Zurich.
*
*Permission is hereby granted, free of charge, to any person or organization
*obtaining a copy of the software and accompanying documentation covered by
*this license (the "Software") to use, reproduce, display, distribute,
*execute, and transmit the Software, and to prepare derivative works of the
*Software, and to permit third-parties to whom the Software is furnished to
*do so, all subject to the following:
*
*The copyright notices in the Software and this entire statement, including
*the above license grant, this restriction and the following disclaimer,
*must be included in all copies of the Software, in whole or in part, and
*all derivative works of the Software, unless such copies or derivative
*works are solely in the form of machine-executable object code generated by
*a source language processor.
*
*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
*SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
*FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
*ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*DEALINGS IN THE SOFTWARE.
*/

#ifndef VLI_KARATSUBA_ASM_H
#define VLI_KARATSUBA_ASM_H

#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/arithmetic/mul.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/stringize.hpp>

namespace vli {
    namespace detail {

    template <std::size_t NumBits>
    void KA_add(boost::uint64_t* x, boost::uint64_t const* y);
   
    template <std::size_t NumBits>
    boost::uint64_t KA_sub(boost::uint64_t* x, boost::uint64_t const* y); //return the mask
   
    template <std::size_t NumBits>
    void KA_sign(boost::uint64_t* x, boost::uint64_t a);
    
    #define r(w, n, MAX)  BOOST_PP_COMMA_IF(n) BOOST_PP_STRINGIZE(+r)(x[n])
    #define g(w, n, MAX)  BOOST_PP_COMMA_IF(n) BOOST_PP_STRINGIZE(g)(y[n])
   
    #define addn128_n128_ka_cpu(w, n, MAX) \
        BOOST_PP_IF(n,"adcq %"BOOST_PP_STRINGIZE(BOOST_PP_ADD(MAX,n))", %"BOOST_PP_STRINGIZE(n)"; \n\t","addq  %"BOOST_PP_STRINGIZE(BOOST_PP_ADD(MAX,n))", %0; \n\t")

    #define subn128_n128_ka_cpu(w, n, MAX) \
        BOOST_PP_IF(n,"sbbq %"BOOST_PP_STRINGIZE(BOOST_PP_ADD(MAX,BOOST_PP_ADD(n,1)))", %"BOOST_PP_STRINGIZE(BOOST_PP_ADD(n,1))"; \n\t","subq  %"BOOST_PP_STRINGIZE(BOOST_PP_ADD(MAX,BOOST_PP_ADD(n,1)))", %1; \n\t") \
   
    #define addn64_bit_ka_cpu(w, n, MAX) \
        BOOST_PP_IF(n,"adcq $0, %"BOOST_PP_STRINGIZE(n)"; \n\t","addq  %"BOOST_PP_STRINGIZE(BOOST_PP_ADD(MAX,n))", %0; \n\t")  /* n=0 no CB,  n!=0, second pass needs CB */  \

   
    #define FUNCTION_add_nbits_nbits(z, n, unused) \
    template<> \
    void KA_add<(n+1)*64>(boost::uint64_t* x,boost::uint64_t const* y){ \
        asm( \
            BOOST_PP_REPEAT(BOOST_PP_ADD(n,1), addn128_n128_ka_cpu, BOOST_PP_ADD(n,1)) \
          : BOOST_PP_REPEAT(BOOST_PP_ADD(n,1), r, ~) : BOOST_PP_REPEAT(BOOST_PP_ADD(n,1), g, BOOST_PP_ADD(n,1)): "cc" \
        ); \
    }; \
    
    #define FUNCTION_add_nbits_bit(z, n, unused) \
    template<> \
    void KA_sign<(n+1)*64>(boost::uint64_t* x,boost::uint64_t mask){ \
        for(int i(0); i < (n+1); ++i) \
            x[i] ^= mask; \
        mask >>= 63; /* transform the mask to CB */ \
        asm volatile(  \
           BOOST_PP_REPEAT(BOOST_PP_ADD(n,1), addn64_bit_ka_cpu, BOOST_PP_ADD(n,1)) \
           :BOOST_PP_REPEAT(BOOST_PP_ADD(n,1), r, ~) : "g"(mask) : "cc" \
        ); \
    }; \

    #define FUNCTION_sub_nbits_nbits(z, n, unused) \
    template<> \
        boost::uint64_t KA_sub<(n+1)*64>(boost::uint64_t* x,boost::uint64_t const* y){ \
        boost::uint64_t sign(0); \
        asm( \
            BOOST_PP_REPEAT(BOOST_PP_ADD(n,1), subn128_n128_ka_cpu, BOOST_PP_ADD(n,1)) \
            "setc %0; \n\t" /* get the CB*/\
           : "+m"(sign), BOOST_PP_REPEAT(BOOST_PP_ADD(n,1), r, ~) : BOOST_PP_REPEAT(BOOST_PP_ADD(n,1), g, BOOST_PP_ADD(n,1)): "cc" \
        ); \
        return -sign; \
    }; \
   
    BOOST_PP_REPEAT(8, FUNCTION_add_nbits_nbits, ~)
    BOOST_PP_REPEAT(4, FUNCTION_sub_nbits_nbits, ~)
    BOOST_PP_REPEAT(4, FUNCTION_add_nbits_bit,   ~)
    
    template <std::size_t NumBits>
    struct KA_helper{//lsb = partie haute, msb = partie basse
        inline static vli<2*NumBits> KA_algo(vli<NumBits> const&  vli_a, vli<NumBits> const& vli_b){
            vli<NumBits/2> al(vli_a,copy_msb_tag()),ah(vli_a,copy_lsb_tag());
            vli<NumBits/2> bl(vli_b,copy_msb_tag()),bh(vli_b,copy_lsb_tag());
            vli<NumBits/2> am(al); // partie basse
            vli<NumBits/2> bm(bh); // partie haute
            vli<NumBits> low,high,medium;
            boost::uint64_t mask_s1 = KA_sub<NumBits/2>(&am[0],&ah[0]);
            boost::uint64_t mask_s2 = KA_sub<NumBits/2>(&bm[0],&bl[0]);
            KA_sign<NumBits/2>(&am[0],mask_s1);
            KA_sign<NumBits/2>(&bm[0],mask_s2);
            low = KA_helper<NumBits/2>::KA_algo(al,bl);
            high = KA_helper<NumBits/2>::KA_algo(ah,bh);
            medium = KA_helper<NumBits/2>::KA_algo(am,bm);
            mask_s1 ^= mask_s2;
            KA_sign<NumBits>(&medium[0],mask_s1);
            vli<NumBits> low_cp(low),medium_cp(medium),high_c(high);
            KA_add<NumBits>(&medium[0],&low[0]);
            KA_add<NumBits>(&medium[0],&high[0]);
            vli<2*NumBits> tmp_res0(medium,copy_right_shift_tag());
            vli<2*NumBits> tmp_res1(low,high,copy_right_shift_tag());
            KA_add<2*NumBits>(&tmp_res0[0],&tmp_res1[0]);
            return tmp_res0;
        };
    };
   
    template <>
    struct KA_helper<64>{
        static vli<128> KA_algo(vli<64> const& vli_a, vli<64> const& vli_b){//
            vli<128> vli_res;
            /* =a means rax for lower part, =d means rdx for the higher part, = for writing, %0 directly vli_a[0] ready for mul vli[0] */
            asm("mulq %3;" :"=a"(vli_res[0]), "=d"(vli_res[1]) :"%0" (vli_a[0]), "r"(vli_b[0]): "cc");
            return vli_res;
        }
    };

    }
}

#undef FUNCTION_sub_nbits_nbits
#undef FUNCTION_add_nbits_bit
#undef FUNCTION_add_nbits_nbits
#undef addn128_n128_ka_cpu
#undef sub_m128_n128_gpu

#endif

//   Karatusba postive version 
//    template <std::size_t NumBits>
//    struct KA_helper{
//        inline static vli<2*NumBits> KA_algo(vli<NumBits> const&  vli_a, vli<NumBits> const& vli_b){
//            vli<NumBits/2> x1(vli_a,copy_lsb_tag()),x2(vli_a,copy_msb_tag());
//            vli<NumBits/2> y1(vli_b,copy_lsb_tag()),y2(vli_b,copy_msb_tag());
//            vli<NumBits/2> x1x2(x1);
//            vli<NumBits/2> y1y2(y1);
//            vli<NumBits> u,v,w;
//            u = KA_helper<NumBits/2>::KA_algo(x2,y2);
//            v = KA_helper<NumBits/2>::KA_algo(x1,y1);
//            KA_add<NumBits/2>(x1x2,x2);
//            KA_add<NumBits/2>(y1y2,y2);
//            w = KA_helper<NumBits/2>::KA_algo(x1x2,y1y2);
//            KA_sub<NumBits>(w,u);
//            KA_sub<NumBits>(w,v);
//            vli<2*NumBits> tmp_res0(w,copy_right_shift_tag());
//            vli<2*NumBits> tmp_res1(u,v,copy_right_shift_tag());
//            KA_add<2*NumBits>(tmp_res0,tmp_res1);
//            return tmp_res0;
//        };
//    };
