
namespace vli {
    namespace detail {

/*
*  I do this to remove the cloud of if of the general version l 92 to 101, for polynomial of 1 and 2 variables
*  on fermi 2 variables with cloud = 0.22 s withou 0.17 s, to retest on kerpler and remove if necessarry
* just recopy the 4 variables version (compatible 1 <-> 4) into polynomial_mul_full_kepler and remove the booster    
*/
    template <typename BaseInt, std::size_t Size, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
    struct booster;

    // 4 variables
    template <typename BaseInt, std::size_t Size, unsigned int Order, class Var0, class Var1, class Var2, class Var3>
    struct booster<BaseInt, Size, max_order_each<Order>, Var0, Var1, Var2, Var3 >{
    static __device__ void polynomial_multiplication_max_order( const unsigned int * __restrict__ in1,
                                                                const unsigned int * __restrict__ in2,
                                                                const unsigned int element_count,
                                                                unsigned int* __restrict__ out,
                                                                unsigned int* __restrict__ workblock_count_by_warp,
                                                                single_coefficient_task* __restrict__ execution_plan) {
        const unsigned int local_thread_id = threadIdx.x;
        const unsigned int element_id = blockIdx.x;
        
        unsigned int c1[Size],c2[Size];
        unsigned int res[2*Size];
        unsigned int res1[2*Size];
        
        unsigned int iteration_count = workblock_count_by_warp[local_thread_id / 32];
        
        const unsigned int input_elem_offset = element_id * stride<Var0,Order>::value * stride<Var1,Order>::value *  stride<Var2,Order>::value * stride<Var3,Order>::value * Size;
        
        for(unsigned int iteration_id = 0; iteration_id < iteration_count; ++iteration_id) {
            single_coefficient_task task = execution_plan[local_thread_id + (iteration_id * MulBlockSize<max_order_each<Order>, Var0, Var1, Var2, Var3>::value)];
            const unsigned int step_count = task.step_count;
        
            if (step_count > 0) {
                #pragma unroll
                for(unsigned int i = 0; i < 2*Size; ++i)
                    res[i] = 0;
                
                const unsigned int output_degree_x = task.output_degree_x;
                const unsigned int output_degree_y = task.output_degree_y;
                const unsigned int output_degree_z = task.output_degree_z;
                const unsigned int output_degree_w = task.output_degree_w;
                
                const unsigned int start_degree_x_inclusive = output_degree_x > Order ? output_degree_x - Order : 0;
                const unsigned int end_degree_x_inclusive = output_degree_x < (Order+1) ? output_degree_x : Order;
                unsigned int current_degree_x = start_degree_x_inclusive;
                
                const unsigned int start_degree_y_inclusive = output_degree_y > Order ? output_degree_y - Order : 0;
                const unsigned int end_degree_y_inclusive = output_degree_y < (Order+1) ? output_degree_y : Order;
                unsigned int current_degree_y = start_degree_y_inclusive;
                
                const unsigned int start_degree_z_inclusive = output_degree_z > Order ? output_degree_z - Order : 0;
                const unsigned int end_degree_z_inclusive = output_degree_z < (Order+1) ? output_degree_z : Order;
                unsigned int current_degree_z = start_degree_z_inclusive;
               
                unsigned int current_degree_w = output_degree_w > Order ? output_degree_w - Order : 0;
                
                for(unsigned int step_id = 0; step_id < step_count; ++step_id) {
                
                    unsigned int in_polynomial_offset1 = (   current_degree_w  * stride_pad<Var3, Order>::value * stride_pad<Var2, Order>::value * stride_pad<Var1, Order>::value 
                                                           + current_degree_z  * stride_pad<Var2, Order>::value * stride_pad<Var1, Order>::value  
                                                           + current_degree_y  * stride_pad<Var1, Order>::value
                                                           + current_degree_x
                                                         ) * Size + input_elem_offset;
                    
                    unsigned int in_polynomial_offset2 = (   (output_degree_w - current_degree_w) * stride_pad<Var3, Order>::value * stride_pad<Var2, Order>::value * stride_pad<Var1, Order>::value
                                                           + (output_degree_z - current_degree_z) * stride_pad<Var2, Order>::value * stride_pad<Var1, Order>::value  
                                                           + (output_degree_y - current_degree_y) * stride_pad<Var1, Order>::value
                                                           + (output_degree_x - current_degree_x)
                                                         ) * Size + input_elem_offset;
                
                    #pragma unroll
                    for(unsigned int i = 0; i < Size; ++i)
                        c1[i] = in1[in_polynomial_offset1 + i];
                
                    #pragma unroll
                    for(unsigned int i = 0; i < Size; ++i)
                        c2[i] = in2[in_polynomial_offset2 + i];
                
                    #pragma unroll
                    for(unsigned int i = 0; i < 2*Size; ++i)
                        res1[i] = 0;
                 
                    multiplies<BaseInt, Size>(res, res1, c1, c2); // the multiplication using boost pp
                 
                    // Calculate the next pair of input coefficients to be multiplied and added to the result
                    // TO SPECIALIZE ..... all this dynamic if kill the perfs 
                    current_degree_x++;
                    if (current_degree_x > end_degree_x_inclusive) {
                        current_degree_x = start_degree_x_inclusive;
                        current_degree_y++;
                        if (current_degree_y > end_degree_y_inclusive) {
                            current_degree_y = start_degree_y_inclusive;
                            current_degree_z++;
                            if (current_degree_z > end_degree_z_inclusive) {
                                current_degree_z = start_degree_z_inclusive;
                                current_degree_w++;                        
                            }                    
                        }            
                    }
                }
                
                unsigned int coefficient_id =   output_degree_w * (Order*2+1)*(Order*2+1)*(Order*2+1)
                                              + output_degree_z * (Order*2+1)*(Order*2+1) 
                                              + output_degree_y * (Order*2+1) 
                                              + output_degree_x;
                
                unsigned int * out2 = out + (coefficient_id * element_count *2* Size) + element_id; // coefficient->int_degree->element_id
                #pragma unroll
                for(unsigned int i = 0; i < 2*Size; ++i) {
                        // This is a strongly compute-bound kernel,
                        // so it is fine to waste memory bandwidth by using non-coalesced writes in order to have less instructions,
                        //     less synchronization points, less shared memory used (and thus greater occupancy) and greater scalability.
                        *out2 = res[i];
                        out2 += element_count;
                }
            } // end step count
        } // end for it

        }; // end function
    }; //end struct

    // 3 variables
    template <typename BaseInt, std::size_t Size, unsigned int Order, class Var0, class Var1, class Var2>
    struct booster<BaseInt, Size, max_order_each<Order>, Var0, Var1, Var2,vli::no_variable>{
    static __device__ void polynomial_multiplication_max_order( const unsigned int * __restrict__ in1,
                                                                const unsigned int * __restrict__ in2,
                                                                const unsigned int element_count,
                                                                unsigned int* __restrict__ out,
                                                                unsigned int* __restrict__ workblock_count_by_warp,
                                                                single_coefficient_task* __restrict__ execution_plan) {
        const unsigned int local_thread_id = threadIdx.x;
        const unsigned int element_id = blockIdx.x;
        
        unsigned int c1[Size],c2[Size];
        unsigned int res[2*Size];
        unsigned int res1[2*Size];
        
        unsigned int iteration_count = workblock_count_by_warp[local_thread_id / 32];
        
        const unsigned int input_elem_offset = element_id * stride<Var0,Order>::value * stride<Var1,Order>::value *  stride<Var2,Order>::value * Size;
        
        for(unsigned int iteration_id = 0; iteration_id < iteration_count; ++iteration_id) {
            single_coefficient_task task = execution_plan[local_thread_id + (iteration_id * MulBlockSize<max_order_each<Order>, Var0, Var1, Var2, vli::no_variable>::value)];
            const unsigned int step_count = task.step_count;
        
            if (step_count > 0) {
                #pragma unroll
                for(unsigned int i = 0; i < 2*Size; ++i)
                    res[i] = 0;
                
                const unsigned int output_degree_x = task.output_degree_x;
                const unsigned int output_degree_y = task.output_degree_y;
                const unsigned int output_degree_z = task.output_degree_z;
                
                const unsigned int start_degree_x_inclusive = output_degree_x > Order ? output_degree_x - Order : 0;
                const unsigned int end_degree_x_inclusive = output_degree_x < (Order+1) ? output_degree_x : Order;
                unsigned int current_degree_x = start_degree_x_inclusive;
                
                const unsigned int start_degree_y_inclusive = output_degree_y > Order ? output_degree_y - Order : 0;
                const unsigned int end_degree_y_inclusive = output_degree_y < (Order+1) ? output_degree_y : Order;
                unsigned int current_degree_y = start_degree_y_inclusive;
                
                const unsigned int start_degree_z_inclusive = output_degree_z > Order ? output_degree_z - Order : 0;
                unsigned int current_degree_z = start_degree_z_inclusive;
               
                for(unsigned int step_id = 0; step_id < step_count; ++step_id) {
                
                    unsigned int in_polynomial_offset1 = (   current_degree_z  * stride_pad<Var2, Order>::value * stride_pad<Var1, Order>::value  
                                                           + current_degree_y  * stride_pad<Var1, Order>::value
                                                           + current_degree_x
                                                         ) * Size + input_elem_offset;
                    
                    unsigned int in_polynomial_offset2 = (   (output_degree_z - current_degree_z) * stride_pad<Var2, Order>::value * stride_pad<Var1, Order>::value  
                                                           + (output_degree_y - current_degree_y) * stride_pad<Var1, Order>::value
                                                           + (output_degree_x - current_degree_x)
                                                         ) * Size + input_elem_offset;
                
                    #pragma unroll
                    for(unsigned int i = 0; i < Size; ++i)
                        c1[i] = in1[in_polynomial_offset1 + i];
                
                    #pragma unroll
                    for(unsigned int i = 0; i < Size; ++i)
                        c2[i] = in2[in_polynomial_offset2 + i];
                
                    #pragma unroll
                    for(unsigned int i = 0; i < 2*Size; ++i)
                        res1[i] = 0;
                 
                    multiplies<BaseInt, Size>(res, res1, c1, c2); // the multiplication using boost pp
                 
                    // Calculate the next pair of input coefficients to be multiplied and added to the result
                    // TO SPECIALIZE ..... all this dynamic if kill the perfs 
                    current_degree_x++;
                    if (current_degree_x > end_degree_x_inclusive) {
                        current_degree_x = start_degree_x_inclusive;
                        current_degree_y++;
                        if (current_degree_y > end_degree_y_inclusive) {
                            current_degree_y = start_degree_y_inclusive;
                            current_degree_z++;
                        }            
                    }
                }
                
                unsigned int coefficient_id =   output_degree_z * (Order*2+1)*(Order*2+1) 
                                              + output_degree_y * (Order*2+1) 
                                              + output_degree_x;
                
                unsigned int * out2 = out + (coefficient_id * element_count *2* Size) + element_id; // coefficient->int_degree->element_id
                #pragma unroll
                for(unsigned int i = 0; i < 2*Size; ++i) {
                        // This is a strongly compute-bound kernel,
                        // so it is fine to waste memory bandwidth by using non-coalesced writes in order to have less instructions,
                        //     less synchronization points, less shared memory used (and thus greater occupancy) and greater scalability.
                        *out2 = res[i];
                        out2 += element_count;
                }
            } // end step count
        } // end for it

        }; //end function
    }; //end struct

    // 2 variables
    template <typename BaseInt, std::size_t Size, unsigned int Order, class Var0, class Var1>
    struct booster<BaseInt, Size, max_order_each<Order>, Var0, Var1, vli::no_variable, vli::no_variable>{
    static __device__ void polynomial_multiplication_max_order( const unsigned int * __restrict__ in1,
                                                                const unsigned int * __restrict__ in2,
                                                                const unsigned int element_count,
                                                                unsigned int* __restrict__ out,
                                                                unsigned int* __restrict__ workblock_count_by_warp,
                                                                single_coefficient_task* __restrict__ execution_plan) {
        const unsigned int local_thread_id = threadIdx.x;
        const unsigned int element_id = blockIdx.x;
        
        unsigned int c1[Size],c2[Size];
        unsigned int res[2*Size];
        unsigned int res1[2*Size];
        
        unsigned int iteration_count = workblock_count_by_warp[local_thread_id / 32];
        
        const unsigned int input_elem_offset = element_id * stride<Var0,Order>::value * stride<Var1,Order>::value * Size;
        
        for(unsigned int iteration_id = 0; iteration_id < iteration_count; ++iteration_id) {
            single_coefficient_task task = execution_plan[local_thread_id + (iteration_id * MulBlockSize<max_order_each<Order>, Var0, Var1, vli::no_variable, vli::no_variable>::value)];
            const unsigned int step_count = task.step_count;
        
            if (step_count > 0) {
                #pragma unroll
                for(unsigned int i = 0; i < 2*Size; ++i)
                    res[i] = 0;
                
                const unsigned int output_degree_x = task.output_degree_x;
                const unsigned int output_degree_y = task.output_degree_y;
                
                const unsigned int start_degree_x_inclusive = output_degree_x > Order ? output_degree_x - Order : 0;
                const unsigned int end_degree_x_inclusive = output_degree_x < (Order+1) ? output_degree_x : Order;
                unsigned int current_degree_x = start_degree_x_inclusive;
                
                unsigned int current_degree_y = output_degree_y > Order ? output_degree_y - Order : 0;
                
                for(unsigned int step_id = 0; step_id < step_count; ++step_id) {
                
                    unsigned int in_polynomial_offset1 = (   current_degree_y  * stride_pad<Var1, Order>::value
                                                           + current_degree_x
                                                         ) * Size + input_elem_offset;
                    
                    unsigned int in_polynomial_offset2 = (   (output_degree_y - current_degree_y) * stride_pad<Var1, Order>::value
                                                           + (output_degree_x - current_degree_x)
                                                         ) * Size + input_elem_offset;
                
                    #pragma unroll
                    for(unsigned int i = 0; i < Size; ++i)
                        c1[i] = in1[in_polynomial_offset1 + i];
                
                    #pragma unroll
                    for(unsigned int i = 0; i < Size; ++i)
                        c2[i] = in2[in_polynomial_offset2 + i];
                
                    #pragma unroll
                    for(unsigned int i = 0; i < 2*Size; ++i)
                        res1[i] = 0;
                 
                    multiplies<BaseInt, Size>(res, res1, c1, c2); // the multiplication using boost pp
                 
                    current_degree_x++;
                    if (current_degree_x > end_degree_x_inclusive) {
                        current_degree_x = start_degree_x_inclusive;
                        current_degree_y++;
                    }
                }
                
                unsigned int coefficient_id =   output_degree_y * (Order*2+1) 
                                              + output_degree_x;
                
                unsigned int * out2 = out + (coefficient_id * element_count *2* Size) + element_id; // coefficient->int_degree->element_id
                #pragma unroll
                for(unsigned int i = 0; i < 2*Size; ++i) {
                        // This is a strongly compute-bound kernel,
                        // so it is fine to waste memory bandwidth by using non-coalesced writes in order to have less instructions,
                        //     less synchronization points, less shared memory used (and thus greater occupancy) and greater scalability.
                        *out2 = res[i];
                        out2 += element_count;
                }
            } // end step count
        } // end for it

        }; //end function
    };// end struct

    // 1 variables
    template <typename BaseInt, std::size_t Size, unsigned int Order, class Var0>
    struct booster<BaseInt, Size, max_order_each<Order>, Var0, vli::no_variable,vli::no_variable,vli::no_variable>{
    static __device__ void polynomial_multiplication_max_order( const unsigned int * __restrict__ in1,
                                                                const unsigned int * __restrict__ in2,
                                                                const unsigned int element_count,
                                                                unsigned int* __restrict__ out,
                                                                unsigned int* __restrict__ workblock_count_by_warp,
                                                                single_coefficient_task* __restrict__ execution_plan) {
        const unsigned int local_thread_id = threadIdx.x;
        const unsigned int element_id = blockIdx.x;
        
        unsigned int c1[Size],c2[Size];
        unsigned int res[2*Size];
        unsigned int res1[2*Size];
        
        unsigned int iteration_count = workblock_count_by_warp[local_thread_id / 32];
        
        const unsigned int input_elem_offset = element_id * stride<Var0,Order>::value * Size;
        
        for(unsigned int iteration_id = 0; iteration_id < iteration_count; ++iteration_id) {
            single_coefficient_task task = execution_plan[local_thread_id + (iteration_id * MulBlockSize<max_order_each<Order>, Var0, vli::no_variable, vli::no_variable, vli::no_variable>::value)];
            const unsigned int step_count = task.step_count;
        
            if (step_count > 0) {
                #pragma unroll
                for(unsigned int i = 0; i < 2*Size; ++i)
                    res[i] = 0;
                
                const unsigned int output_degree_x = task.output_degree_x;
                
                unsigned int current_degree_x = output_degree_x > Order ? output_degree_x - Order : 0;
                
                for(unsigned int step_id = 0; step_id < step_count; ++step_id) {
                
                    unsigned int in_polynomial_offset1 = ( + current_degree_x
                                                         ) * Size + input_elem_offset;
                    
                    unsigned int in_polynomial_offset2 = ( + (output_degree_x - current_degree_x)
                                                         ) * Size + input_elem_offset;
                
                    #pragma unroll
                    for(unsigned int i = 0; i < Size; ++i)
                        c1[i] = in1[in_polynomial_offset1 + i];
                
                    #pragma unroll
                    for(unsigned int i = 0; i < Size; ++i)
                        c2[i] = in2[in_polynomial_offset2 + i];
                
                    #pragma unroll
                    for(unsigned int i = 0; i < 2*Size; ++i)
                        res1[i] = 0;
                 
                    multiplies<BaseInt, Size>(res, res1, c1, c2); // the multiplication using boost pp
                 
                    current_degree_x++;
                }
                
                unsigned int coefficient_id =  output_degree_x;
                
                unsigned int * out2 = out + (coefficient_id * element_count *2* Size) + element_id; // coefficient->int_degree->element_id
                #pragma unroll
                for(unsigned int i = 0; i < 2*Size; ++i) {
                        // This is a strongly compute-bound kernel,
                        // so it is fine to waste memory bandwidth by using non-coalesced writes in order to have less instructions,
                        //     less synchronization points, less shared memory used (and thus greater occupancy) and greater scalability.
                        *out2 = res[i];
                        out2 += element_count;
                }
            } // end step count
        } // end for it

        };  // end function
    }; // end struct


    }//end namesoace detail
}//end namespace vli
