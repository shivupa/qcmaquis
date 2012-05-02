#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/arithmetic/mul.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/config/limits.hpp>
#include <boost/preprocessor/comparison/greater_equal.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/config/limits.hpp>
#include <boost/preprocessor/iteration/local.hpp>

#define NAME_MUL_NBITS_64BITS(n)                      BOOST_PP_CAT(BOOST_PP_CAT(mul        ,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64)),BOOST_PP_CAT(_,64)) /* addnx64_64*/
#define NAME_MUL_NBITS_NBITS(n) BOOST_PP_CAT(BOOST_PP_CAT(mul,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64)),BOOST_PP_CAT(_,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64))) /* addnx64_64*/
#define NAME_MUL_TWONBITS_NBITS_NBITS(n)              BOOST_PP_CAT(BOOST_PP_CAT(mul,BOOST_PP_CAT(BOOST_PP_MUL(n,2),x64)),BOOST_PP_CAT(_,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64))) /* addnx64_64*/
#define NAME_CONDITIONAL_MUL_NBITS_64BITS(n)          BOOST_PP_STRINGIZE(BOOST_PP_CAT(BOOST_PP_CAT(_IsNegative,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64)),BOOST_PP_CAT(_,64))) /* _IsNegativenx64_64, for the input sign*/
#define NAME_RES_CONDITIONAL_MUL_NBITS_64BITS(n)      BOOST_PP_CAT(BOOST_PP_CAT(_IsNegativeRes,BOOST_PP_CAT(BOOST_PP_ADD(n,2),x64)),BOOST_PP_CAT(_,64)) /* _IsNegativeResnx64_64, for the output sign*/

// boost_pp is limiter to 256 for arithmetic therefore I calculated intermediate value
#define mul2x64_64 mul128_64
#define mul3x64_64 mul192_64
#define mul4x64_64 mul256_64
#define mul5x64_64 mul320_64
#define mul6x64_64 mul384_64
#define mul7x64_64 mul448_64
#define mul8x64_64 mul512_64

#define mul2x64_2x64 mul128_128
#define mul3x64_3x64 mul192_192
#define mul4x64_4x64 mul256_256
#define mul5x64_5x64 mul320_320
#define mul6x64_6x64 mul384_384
#define mul7x64_7x64 mul448_448
#define mul8x64_8x64 mul512_512

#define AOS 1

#define R(n) BOOST_PP_STRINGIZE(BOOST_PP_CAT(%%r, BOOST_PP_ADD(8,n))) // give register start at r8  

#define PPS(m,n ) BOOST_PP_STRINGIZE( BOOST_PP_MUL(BOOST_PP_MUL(m,n),8)) // m*n*8, 8 because long int
#define PPSr(max,n) BOOST_PP_STRINGIZE( BOOST_PP_MUL(BOOST_PP_ADD(max,n),8)) // m*n*8, 8 because long int

#define LOAD_register(z, n, unused) "movq "PPSr(1,n)"(%%rdi)                 , "R(n)"    \n" /* load 0x??(%%rdi) */     

#define LOADr_register(z, n, unused) "movq "PPSr(1,n)"(%%rsi)                , "Rr(n)"    \n" /* load 0x??(%%rdi), r for reverse */     

#define XOR_register(z, n, unused) "xorq "R(BOOST_PP_ADD(n,2))"           ,"R(BOOST_PP_ADD(n,2))"       \n" /* set up register to 0 */

#define SAVE_register(z, n, unused) "movq "R(n)"                              ,"PPS(AOS,n)"(%%rdi)    \n" /* save 0x??(%%rdi) */     

#define ADC0_register(z, n, unused) "adcq $0x0             ,"R(BOOST_PP_ADD(n,1))"      \n" /* adcq 0 + rdi + CB    */     

#define ADCMUL0_register(z, n, unused) "adcq $0x0             ,"R(BOOST_PP_ADD(n,4))"      \n" /* adcq 0 + rdi + CB    */     

#define  MUL_register(z, n, unused) "mulq "PPS(1,BOOST_PP_ADD(n,1))"(%%rdi)             \n" /* mulq r??*rax */                \
                                    "addq %%rax            ,"R(BOOST_PP_ADD(n,1))"      \n" /* add hia?b? + loa?b? */         \
                                    "movq %%rdx            ,"R(BOOST_PP_ADD(n,2))"      \n" /* save the hi into rcx */        \
                                    "adcq $0               ,"R(BOOST_PP_ADD(n,2))"      \n" /* perhaps carry */               \
                                    "movq %%rbx            ,%%rax                       \n" /* reload rax(a0) from the rbx */ \


#define  MULNN_register(z, n, unused) \
                                      "mulq "PPS(1,n)"(%%rdi)             \n" /* mulq r??*rax */                \
                                      "movq %%rax            ,"R(n)"     \n" /* add hia?b? + loa?b? */         \
                                      "adcq %%rdx            ,"R(BOOST_PP_ADD(n,1))"     \n" /* add hia?b? + loa?b? */         \
                                       BOOST_PP_REPEAT(n, ADCMUL0_register, ~)                                      \

#define NOT_register(z, n, unused)  "notq "R(n)"                                        \n" /* start C2M negate */ 


#define CLOTHER_register(z, n, unused) R(n), /* "r8","r9", ... */


 // give register start at r15, r12, .... reverse order  
#define Rax(MAX,n) BOOST_PP_STRINGIZE(BOOST_PP_CAT(%%r, BOOST_PP_SUB(15,BOOST_PP_SUB(MAX,n)))) // give register start at r15, r12, .... reverse order  
#define Rdx(MAX,n) BOOST_PP_STRINGIZE(BOOST_PP_CAT(%%r, BOOST_PP_SUB(15,BOOST_PP_SUB(BOOST_PP_SUB(MAX,1),n)))) // give register start at r15, r12, .... reverse order  
#define NUM1(MAX,n) BOOST_PP_SUB(15,BOOST_PP_SUB(BOOST_PP_SUB(MAX,2),n))
#define Radc0(MAX,n) BOOST_PP_STRINGIZE(BOOST_PP_CAT(%%r, BOOST_PP_ADD(MAX,n))) // give register start at r15, r12, .... reverse order  
#define ADC0_register_mul(z, n, nbegin) "adcq $0x0, "Radc0(nbegin,n)"      \n" /* adcq 0 + rdi + CB    */     
#define Rr(Max,n) BOOST_PP_STRINGIZE(BOOST_PP_CAT(%%r, BOOST_PP_ADD(BOOST_PP_SUB(15,Max),n))) // give register start at r8  
#define SAVEr_register(z, n, MAX) "movq "Rr(MAX,BOOST_PP_ADD(n,1))", "PPS(AOS,n)"(%%rdi)    \n" /* save 0x??(%%rdi) */     
#define PPSr1(Max,n) BOOST_PP_STRINGIZE( BOOST_PP_MUL(BOOST_PP_SUB(Max,n),8)) // m*n*8, 8 because long int

#define MULNTON1(z, n, niteration) \
                BOOST_PP_IF(n,"movq %%rbx, %%rax \n",) \
                "mulq "PPS(1,n)"(%%rdi) \n" \
                BOOST_PP_IF(n,"addq %%rax, "Rax(niteration,n)" \n","movq %%rax, "Rax(niteration,n)" \n") \
                BOOST_PP_IF(n,BOOST_PP_IF(BOOST_PP_EQUAL(niteration,n), ,"adcq %%rdx,"Rdx(niteration,n)" \n"), BOOST_PP_IF(BOOST_PP_EQUAL(niteration,n), ,"addq %%rdx,"Rdx(niteration,n)" \n")) \
                BOOST_PP_REPEAT(BOOST_PP_SUB(niteration,BOOST_PP_ADD(n,1)), ADC0_register_mul, NUM1(niteration,n)) 

#define MULNTON0(z, n, MAX) \
       "movq "PPSr1(MAX,n)"(%%rsi), %%rax \n" \
        BOOST_PP_IF(n,"movq %%rax, %%rbx \n", ) \
        BOOST_PP_REPEAT(BOOST_PP_ADD(n,1), MULNTON1, n) \


#define BOOST_PP_LOCAL_MACRO(n) \
    BOOST_PP_REPEAT(n, MULNTON0, BOOST_PP_SUB(n,1)) \
    BOOST_PP_REPEAT(n, SAVEr_register, n) // for saving

#define BOOST_PP_LOCAL_LIMITS (2, 8) // expand 128 -> 512

#include BOOST_PP_LOCAL_ITERATE()





