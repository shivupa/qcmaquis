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
*a   source language processor.
*
*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
*SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
*FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
*ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*DEALINGS IN THE SOFTWARE.
*/

#include <vector>
#include <algorithm>
#include <iostream>

#include "vli/detail/kernels_gpu.h" // signature interface with cpu + structure max_order_each, max_order_combined
#include "vli/detail/gpu/utils/variables_gpu.h" //compile time  variable
#include "vli/detail/gpu/tasklist/tasklist.h" //tasklist
#include "vli/detail/gpu/utils/gpu_mem_block.h" // memory
#include "vli/detail/gpu/kernels/kernels_gpu_neg_asm.hpp" //kernels gpu boost pp
#include "vli/detail/gpu/kernels/kernels_gpu_add_asm.hpp" //kernels gpu boost pp
#include "vli/detail/gpu/kernels/kernels_gpu_mul_asm.hpp" //kernels gpu boost pp
#include "vli/detail/gpu/vli_number_gpu_function_hooks.hpp" // wrapper
#include "vli/detail/gpu/polynomial_multiplication/booster_polynomial_multiplication_max_order_each.hpp" // booster
#include "vli/detail/gpu/polynomial_multiplication/booster_polynomial_multiplication_max_order_combined.hpp" // booster
#include "vli/detail/gpu/polynomial_reduction/polynomial_reduction.hpp" // final reduction

namespace vli {
    namespace detail {
   
    template <typename BaseInt, std::size_t Size, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
    __global__ void
    __launch_bounds__(MulBlockSize<OrderSpecification, Var0, Var1, Var2, Var3>::value , 2)
    polynomial_mul_full_kepler( // TO DO change the name
    	const unsigned int * __restrict__ in1,
    	const unsigned int * __restrict__ in2,
        const unsigned int element_count,
        unsigned int* __restrict__ out,
        unsigned int* __restrict__ workblock_count_by_warp,
        single_coefficient_task* __restrict__ execution_plan)
    {
        booster<BaseInt, Size, OrderSpecification, Var0, Var1, Var2, Var3>::polynomial_multiplication_max_order(in1, in2, element_count, out, workblock_count_by_warp, execution_plan); // TO DO change the name
    }

    template <typename BaseInt, std::size_t Size, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
    void gpu_inner_product_vector(std::size_t VectorSize, BaseInt const* A, BaseInt const* B) {

	    gpu_memblock<BaseInt>* pgm = gpu_memblock<BaseInt>::Instance(); // allocate memory for vector input, intermediate and output, singleton only one time, whatever the type of polynomial, could we change the pattern by a ref ? 
            resize_helper<BaseInt, Size, OrderSpecification, Var0, Var1, Var2, Var3>::resize(pgm, VectorSize);
            
  	    tasklist_keep_order<Size,OrderSpecification, Var0, Var1, Var2, Var3>* ghc = tasklist_keep_order<Size, OrderSpecification, Var0, Var1, Var2, Var3>::Instance(); // calculate the different packet, singleton only one time 

            memory_transfer_helper<BaseInt, Size, OrderSpecification, Var0, Var1, Var2, Var3>::transfer_up(pgm, A, B, VectorSize); //transfer data poly to gpu
             
	    {
                dim3 grid(VectorSize) ;
                dim3 threads(MulBlockSize<OrderSpecification, Var0, Var1, Var2, Var3>::value);
                polynomial_mul_full_kepler<BaseInt, Size, OrderSpecification, Var0, Var1, Var2, Var3><<<grid,threads>>>(pgm->V1Data_, pgm->V2Data_,VectorSize, pgm->VinterData_,ghc->workblock_count_by_warp_,ghc->execution_plan_);
	    }

	    {
                dim3 grid(MaxNumberCoefficientExtend<OrderSpecification, Var0, Var1, Var2, Var3>::value);
                dim3 threads(SumBlockSize::value);
                polynomial_sum_intermediate_full<BaseInt, Size, OrderSpecification::value, Var0, Var1, Var2, Var3><<<grid,threads>>>(pgm->VinterData_, VectorSize, pgm->PoutData_); //the reduction is independent of the order specification
	    }
    } 

    template <typename BaseInt, std::size_t Size, unsigned int Order, class Var0, class Var1, class Var2, class Var3>
    BaseInt* gpu_get_polynomial(){
	    gpu_memblock<BaseInt>* gm = gpu_memblock<BaseInt>::Instance(); // I just get the mem pointer
	    return gm->PoutData_;
    }

    //to do clean memory for gpu

#define VLI_IMPLEMENT_GPU_FUNCTIONS(TYPE, VLI_SIZE, POLY_ORDER, VAR) \
    template<std::size_t Size, class OrderSpecification, class Var0, class Var1, class Var2, class Var3 >      \
    void gpu_inner_product_vector(std::size_t vector_size, TYPE const* A, TYPE const* B); \
    \
    template<>      \
    void gpu_inner_product_vector<VLI_SIZE, max_order_each<POLY_ORDER>, EXPEND_VAR(VAR) >(std::size_t vector_size, TYPE const* A, TYPE const* B) \
    {gpu_inner_product_vector<unsigned int, 2*VLI_SIZE, max_order_each<POLY_ORDER>, EXPEND_VAR(VAR) >(vector_size, const_cast<unsigned int*>(reinterpret_cast<unsigned int const*>(A)), const_cast<unsigned int*>(reinterpret_cast<unsigned int const*>(B)));} \
    \
    template<>      \
    void gpu_inner_product_vector<VLI_SIZE, max_order_combined<POLY_ORDER>, EXPEND_VAR(VAR) >(std::size_t vector_size, TYPE const* A, TYPE const* B) \
    {gpu_inner_product_vector<unsigned int, 2*VLI_SIZE, max_order_combined<POLY_ORDER>, EXPEND_VAR(VAR) >(vector_size, const_cast<unsigned int*>(reinterpret_cast<unsigned int const*>(A)), const_cast<unsigned int*>(reinterpret_cast<unsigned int const*>(B)));} \
    \
    template<std::size_t Size, class OrderSpecification, class Var0, class Var1, class Var2, class Var3 >      \
    unsigned int* gpu_get_polynomial();/* cuda mem allocated on unsigned int (gpu_mem_block class), do not change the return type */ \
    \
    template<>      \
    unsigned int* gpu_get_polynomial<VLI_SIZE, max_order_each<POLY_ORDER>, EXPEND_VAR(VAR) >() /* cuda mem allocated on unsigned int (gpu_mem_block class), do not change the return type */ \
    {return gpu_get_polynomial<unsigned int, 2*VLI_SIZE, POLY_ORDER, EXPEND_VAR(VAR)>();}\
    \
    template<>      \
    unsigned int* gpu_get_polynomial<VLI_SIZE, max_order_combined<POLY_ORDER>, EXPEND_VAR(VAR) >() /* cuda mem allocated on unsigned int (gpu_mem_block class), do not change the return type */ \
    {return gpu_get_polynomial<unsigned int, 2*VLI_SIZE, POLY_ORDER, EXPEND_VAR(VAR)>();}\

#define VLI_IMPLEMENT_GPU_FUNCTIONS_FOR(r, data, BASEINT_SIZE_ORDER_VAR_TUPLE) \
    VLI_IMPLEMENT_GPU_FUNCTIONS( BOOST_PP_TUPLE_ELEM(4,0,BASEINT_SIZE_ORDER_VAR_TUPLE), BOOST_PP_TUPLE_ELEM(4,1,BASEINT_SIZE_ORDER_VAR_TUPLE), BOOST_PP_TUPLE_ELEM(4,2,BASEINT_SIZE_ORDER_VAR_TUPLE), BOOST_PP_TUPLE_ELEM(4,3,BASEINT_SIZE_ORDER_VAR_TUPLE) )

    BOOST_PP_SEQ_FOR_EACH(VLI_IMPLEMENT_GPU_FUNCTIONS_FOR, _, VLI_COMPILE_BASEINT_SIZE_ORDER_VAR_TUPLE_SEQ)

#undef VLI_IMPLEMENT_GPU_FUNCTIONS_FOR
#undef VLI_IMPLEMENT_GPU_FUNCTIONS

    }
}
