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

#ifndef VLI_KERNELS_GPU_H
#define VLI_KERNELS_GPU_H

#include "vli/polynomial/variable.hpp"
#include "vli/detail/gpu/kernels/kernel_macros.h"

#include "vli/vli_config.h"

namespace vli {
    namespace detail {

    #define VLI_DECLARE_GPU_FUNCTIONS(TYPE, VLI_SIZE, POLY_ORDER, VAR) \
        template<std::size_t Size, class OrderSpecification, class Var0, class Var1, class Var2, class Var3 >      \
        void gpu_inner_product_vector(std::size_t vector_size, TYPE const * A, TYPE const * B); \
        \
        template<std::size_t Size, class OrderSpecification, class Var0, class Var1, class Var2, class Var3 >      \
        unsigned int * gpu_get_polynomial(); /* cuda mem allocated on unsigned int (gpu_mem_block class), do not change the return type */\
   
    #define VLI_DECLARE_GPU_FUNCTIONS_FOR(r, data, BASEINT_SIZE_ORDER_VAR_TUPLE) \
        VLI_DECLARE_GPU_FUNCTIONS( BOOST_PP_TUPLE_ELEM(4,0,BASEINT_SIZE_ORDER_VAR_TUPLE), BOOST_PP_TUPLE_ELEM(4,1,BASEINT_SIZE_ORDER_VAR_TUPLE), BOOST_PP_TUPLE_ELEM(4,2,BASEINT_SIZE_ORDER_VAR_TUPLE), BOOST_PP_TUPLE_ELEM(4,3,BASEINT_SIZE_ORDER_VAR_TUPLE))
   
    BOOST_PP_SEQ_FOR_EACH(VLI_DECLARE_GPU_FUNCTIONS_FOR, _, VLI_COMPILE_BASEINT_SIZE_ORDER_VAR_TUPLE_SEQ)
   
    #undef VLI_DECLARE_GPU_FUNCTIONS_FOR
    #undef VLI_DECLARE_GPU_FUNCTIONS 
   
    } //namespace detail
} //namespace vli

#endif
