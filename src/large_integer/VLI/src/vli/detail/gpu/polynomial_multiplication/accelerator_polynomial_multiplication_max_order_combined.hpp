/*
 *Very Large Integer Library, License - Version 1.0 - May 3rd, 2012
 *
 *Timothee Ewart - University of Geneva,
 *Andreas Hehn - Swiss Federal Institute of technology Zurich.
 *Maxim Milakov - NVIDIA
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


namespace vli {
    namespace detail {

    #define VLI_ExtendStride result_stride<0, 4, Order>::value // 2*order+1
    #define VLI_SIZE num_words<NumBits>::value 
    // c - the declation of memory_specialization is into accelerator_polynomial_multiplication_max_order_combined
    template<std::size_t NumBits, int Order, int NumVars >
    struct memory_specialization<NumBits, max_order_combined<Order>, NumVars, typename  boost::enable_if_c< (full_value<NumBits, max_order_combined<Order>, NumVars>::value*sizeof(unsigned int) < shared_min::value) >::type >{
        enum { value1 = full_value<NumBits, max_order_combined<Order>, NumVars >::value };
        enum { value2 = full_value<NumBits, max_order_combined<Order>, NumVars >::value };

        static inline void __device__ shared_copy(unsigned int* in1shared, unsigned int const* in1, unsigned int* in2shared, unsigned int const* in2, const unsigned int offset){
            for(int i=(threadIdx.x); i < full_value<NumBits, max_order_combined<Order>, NumVars >::value; i+= mul_block_size<max_order_combined<Order>, NumVars,2>::value ){ 
               in1shared[i] = in1[offset + i];
               in2shared[i] = in2[offset + i];
            }
            __syncthreads();
        } 

        static inline unsigned __device__  int offset_1(const unsigned int offset){return 0;}  
        static inline unsigned __device__  int offset_2(const unsigned int offset){return 0;}  

        static inline void __device__ local_copy(unsigned int* c1, unsigned int const* in1shared, unsigned int* c2, unsigned int const* in2shared, const unsigned int offset1, const unsigned int offset2){

            #pragma unroll
            for( int i = 0; i < VLI_SIZE; ++i)
               c1[i] = in1shared[offset1+i];        
           
            #pragma unroll
            for( int i = 0; i < VLI_SIZE; ++i)
               c2[i] = in2shared[offset2+i];        

        } 
    };
     
    template<std::size_t NumBits, int Order, int NumVars >
    struct memory_specialization<NumBits, max_order_combined<Order>, NumVars, typename boost::enable_if_c< (full_value<NumBits, max_order_combined<Order>, NumVars>::value*sizeof(unsigned int) >= shared_min::value) &&  (full_value<NumBits, max_order_combined<Order>, NumVars>::value*sizeof(unsigned int) < 2*shared_min::value) >::type >{
        enum { value1 = full_value<NumBits, max_order_combined<Order>, NumVars >::value };
        enum { value2 = 1}; // should be 0 but not allowed, so 1

        static inline void __device__ shared_copy(unsigned int* in1shared, unsigned int const* in1, unsigned int* in2shared, unsigned int const* in2, const unsigned int offset){
            for(int i=(threadIdx.x); i < full_value<NumBits, max_order_combined<Order>, NumVars >::value; i+= mul_block_size<max_order_combined<Order>, NumVars,2>::value ){ 
               in1shared[i] = in1[offset + i];
            }
             __syncthreads();
        } 

        static inline unsigned __device__ int offset_1(const unsigned int offset){return 0;}  
        static inline unsigned __device__ int offset_2(const unsigned int offset){return offset;}  

        static inline void __device__ local_copy(unsigned int* c1, unsigned int const* in1shared, unsigned int* c2, unsigned int const* in2shared, const unsigned int offset1, const unsigned int offset2){
            #pragma unroll
            for( int i = 0; i < VLI_SIZE; ++i)
               c1[i] = in1shared[offset1+i];        
           
            #pragma unroll
            for( int i = 0; i < VLI_SIZE; ++i)
               c2[i] = tex1Dfetch(tex_reference_2,offset2+i);                 
         } 

    };

    template<std::size_t NumBits, int Order, int NumVars >
    struct memory_specialization<NumBits, max_order_combined<Order>, NumVars, typename boost::enable_if_c< (full_value<NumBits, max_order_combined<Order>, NumVars>::value*sizeof(unsigned int) >= 2*shared_min::value) >::type >{
        enum { value1 = 1}; // same 1 should be 0
        enum { value2 = 1}; // same 1 should be 0 

        static inline void __device__ shared_copy(unsigned int* in1shared, unsigned int const* in1, unsigned int* in2shared, unsigned int const* in2, const unsigned int offset){
            //nothing >_<'
        } 

        static inline unsigned __device__ int offset_1(const unsigned int offset){return offset;}  
        static inline unsigned __device__ int offset_2(const unsigned int offset){return offset;}  

        static inline void __device__ local_copy(unsigned int* c1, unsigned int const* in1shared, unsigned int* c2, unsigned int const* in2shared, const unsigned int offset1, const unsigned int offset2){
            #pragma unroll
            for( int i = 0; i < VLI_SIZE; ++i)
               c1[i] = tex1Dfetch(tex_reference_1,offset1+i);                 
           
            #pragma unroll
            for( int i = 0; i < VLI_SIZE; ++i)
               c2[i] = tex1Dfetch(tex_reference_2,offset2+i);                 
         } 
    };
    // all this could be really simplified as the max_order version
    template <std::size_t NumBits, class MaxOrder, int NumVars>
    struct accelerator;

    // 4 variables
    template <std::size_t NumBits, int Order>
    struct accelerator<NumBits, max_order_combined<Order>, 4>{
    inline static __device__ void polynomial_multiplication_max_order(const unsigned int* in1, const unsigned int* in2, const unsigned int element_count, unsigned int*  out, unsigned int*  workblock_count_by_warp, single_coefficient_task* execution_plan) {
        __shared__ unsigned int in1shared[memory_specialization<NumBits,max_order_combined<Order>,4 >::value1];
        __shared__ unsigned int in2shared[memory_specialization<NumBits,max_order_combined<Order>,4 >::value2];

        const unsigned int local_thread_id = threadIdx.x;
        const unsigned int element_id = blockIdx.x;
        
        unsigned int c1[VLI_SIZE],c2[VLI_SIZE];
        unsigned int res[2*VLI_SIZE];

        unsigned int res1[2*VLI_SIZE];
        
        unsigned int iteration_count = workblock_count_by_warp[local_thread_id / 32];
        
        const unsigned int input_elem_offset = element_id *  vli::detail::max_order_combined_helpers::size<4+1, Order>::value * VLI_SIZE;

        memory_specialization<NumBits,max_order_combined<Order>,4 >::shared_copy(in1shared, in1, in2shared, in2, input_elem_offset);
        
        for( int iteration_id = 0; iteration_id < iteration_count; ++iteration_id) {
            single_coefficient_task task = execution_plan[local_thread_id + (iteration_id * mul_block_size<max_order_combined<2*Order>,4>::value)];
            const unsigned int step_count = task.step_count;
        
            if (step_count > 0) {
                #pragma unroll
                for( int i = 0; i < 2*VLI_SIZE; ++i)
                    res[i] = 0;
                
                const int output_degree_x = task.output_degree_x;
                const int output_degree_y = task.output_degree_y;
                const int output_degree_z = task.output_degree_z;
                const int output_degree_w = task.output_degree_w;

                for(int current_degree_x=max(0, output_degree_x-Order); current_degree_x <= min(output_degree_x,Order) ;++current_degree_x)
                    for(int current_degree_y=max(0, output_degree_x+output_degree_y-Order-current_degree_x); current_degree_y <= min(output_degree_y,Order-current_degree_x) ; ++current_degree_y)
                        for(int current_degree_z=max(0, output_degree_x+output_degree_y+output_degree_z-Order-current_degree_x-current_degree_y); current_degree_z <= min(output_degree_z,Order-current_degree_x - current_degree_y) ; ++current_degree_z)
                            for(int current_degree_w=max(0, output_degree_x+output_degree_y+output_degree_z+output_degree_w-Order-current_degree_x-current_degree_y-current_degree_z); current_degree_w <= min(output_degree_w,Order-current_degree_x - current_degree_y - current_degree_z) ; ++current_degree_w){
                
                             const unsigned int in_polynomial_offset1 = ( //0____0'
                                                                   (current_degree_x*(2*(Order+1)+3-current_degree_x)*(2*(Order+1)*(Order+1)+6*(Order+1)+2 +current_degree_x*current_degree_x -2*(Order+1)*current_degree_x
                                                                   - 3*current_degree_x))/24
                                                                   +(current_degree_y*(current_degree_y*current_degree_y - 3*current_degree_y*((Order+1)+1-current_degree_x) + 3*((Order+1)-current_degree_x)*((Order+1)+2-current_degree_x)+2))/6
                                                                   +((Order+1)-current_degree_x-current_degree_y)*current_degree_z - (current_degree_z*current_degree_z-current_degree_z)/2 + current_degree_w
                                                                  ) * VLI_SIZE + memory_specialization<NumBits,max_order_combined<Order>,4 >::offset_1(input_elem_offset);
                             
                             const unsigned int in_polynomial_offset2 = ( //*.* 
                                                                   ((output_degree_x-current_degree_x)*(2*(Order+1)+3-(output_degree_x-current_degree_x))*(2*(Order+1)*(Order+1)+6*(Order+1)+2
                                                                     + (output_degree_x-current_degree_x)*(output_degree_x-current_degree_x) -2*(Order+1)*(output_degree_x-current_degree_x)
                                                                     - 3*(output_degree_x-current_degree_x)))/24
                                                                   +((output_degree_y-current_degree_y)*((output_degree_y-current_degree_y)*(output_degree_y-current_degree_y) - 3*(output_degree_y-current_degree_y)*((Order+1)+1-(output_degree_x-current_degree_x))
                                                                     + 3*((Order+1)-(output_degree_x-current_degree_x))*((Order+1)+2-(output_degree_x-current_degree_x))+2))/6
                                                                   +((Order+1)-(output_degree_x-current_degree_x)-(output_degree_y-current_degree_y))*(output_degree_z-current_degree_z) 
                                                                     - ((output_degree_z-current_degree_z)*(output_degree_z-current_degree_z)-(output_degree_z-current_degree_z))/2 + (output_degree_w-current_degree_w)
                                                                  ) * VLI_SIZE + memory_specialization<NumBits,max_order_combined<Order>,4 >::offset_2(input_elem_offset);
                             
                             memory_specialization<NumBits,max_order_combined<Order>,4 >::local_copy(c1,in1shared,c2,in2shared,in_polynomial_offset1,in_polynomial_offset2);

                             #pragma unroll
                             for( int i = 0; i < 2*VLI_SIZE; ++i)
                                 res1[i] = 0;
                             
                             multiplies<NumBits>(res, res1, c1, c2); // the multiplication using boost pp
                    } //end big loop

                const unsigned int coefficient_id = (output_degree_x*(2*VLI_ExtendStride+3-output_degree_x)*(2*VLI_ExtendStride*VLI_ExtendStride+6*VLI_ExtendStride+2 +output_degree_x*output_degree_x -2*VLI_ExtendStride*output_degree_x
                                              - 3*output_degree_x))/24
                                              +(output_degree_y*(output_degree_y*output_degree_y - 3*output_degree_y*(VLI_ExtendStride+1-output_degree_x) + 3*(VLI_ExtendStride-output_degree_x)*(VLI_ExtendStride+2-output_degree_x)+2))/6
                                              +(VLI_ExtendStride-output_degree_x-output_degree_y)*output_degree_z - (output_degree_z*output_degree_z-output_degree_z)/2 + output_degree_w;
                
                unsigned int * out2 = out + (coefficient_id * element_count *2* VLI_SIZE) + element_id; // coefficient->int_degree->element_id
                #pragma unroll
                for( int i = 0; i < 2*VLI_SIZE; ++i) {
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
    template <std::size_t NumBits, int Order>
    struct accelerator<NumBits, max_order_combined<Order>, 3>{
    inline static __device__ void polynomial_multiplication_max_order(const unsigned int* in1, const unsigned int* in2, const unsigned int element_count, unsigned int*  out, unsigned int*  workblock_count_by_warp, single_coefficient_task* execution_plan) {

        __shared__ unsigned int in1shared[memory_specialization<NumBits,max_order_combined<Order>,3 >::value1];
        __shared__ unsigned int in2shared[memory_specialization<NumBits,max_order_combined<Order>,3 >::value2];

        const unsigned int local_thread_id = threadIdx.x;
        const unsigned int element_id = blockIdx.x;
        
        unsigned int c1[VLI_SIZE],c2[VLI_SIZE];
        unsigned int res[2*VLI_SIZE];
        unsigned int res1[2*VLI_SIZE];
        
        unsigned int iteration_count = workblock_count_by_warp[local_thread_id / 32];
        
        const unsigned int input_elem_offset = element_id * vli::detail::max_order_combined_helpers::size<3+1, Order>::value  * VLI_SIZE;

        memory_specialization<NumBits,max_order_combined<Order>,3 >::shared_copy(in1shared, in1, in2shared, in2, input_elem_offset);
 
        for( int iteration_id = 0; iteration_id < iteration_count; ++iteration_id) {
            single_coefficient_task task = execution_plan[local_thread_id + (iteration_id * mul_block_size<max_order_combined<2*Order>, 3>::value)];
            const unsigned int step_count = task.step_count;
        
            if (step_count > 0) {
                #pragma unroll
                for( int i = 0; i < 2*VLI_SIZE; ++i)
                    res[i] = 0;
                
                const int output_degree_x = task.output_degree_x;
                const int output_degree_y = task.output_degree_y;
                const int output_degree_z = task.output_degree_z;

                for(int current_degree_x=max(0, output_degree_x-Order); current_degree_x <= min(output_degree_x,Order) ;++current_degree_x)
                    for(int current_degree_y=max(0, output_degree_x+output_degree_y-Order-current_degree_x); current_degree_y <= min(output_degree_y,Order-current_degree_x) ; ++current_degree_y)
                        for(int current_degree_z=max(0, output_degree_x+output_degree_y+output_degree_z-Order-current_degree_x-current_degree_y); current_degree_z <= min(output_degree_z,Order-current_degree_x - current_degree_y) ; ++current_degree_z){
                
                        const unsigned int in_polynomial_offset1 = (
                                                              (current_degree_x*(current_degree_x*current_degree_x - 3*current_degree_x*((Order+1)+1)
                                                              + 3*(Order+1)*((Order+1)+2) +2))/6
                                                              + ((Order+1) - current_degree_x)*current_degree_y - (current_degree_y*current_degree_y-current_degree_y)/2 + current_degree_z
                                                             ) * VLI_SIZE + memory_specialization<NumBits,max_order_combined<Order>,3 >::offset_1(input_elem_offset)  ;
      
                        
                        const unsigned int in_polynomial_offset2 = ( 
                                                              ((output_degree_x-current_degree_x)*((output_degree_x-current_degree_x)*(output_degree_x-current_degree_x) - 3*(output_degree_x-current_degree_x)*((Order+1)+1)
                                                              + 3*(Order+1)*((Order+1)+2) +2))/6
                                                              + ((Order+1) - (output_degree_x-current_degree_x))*(output_degree_y-current_degree_y) - ((output_degree_y-current_degree_y)*(output_degree_y-current_degree_y)-(output_degree_y-current_degree_y))/2
                                                              + (output_degree_z-current_degree_z)
                                                             ) * VLI_SIZE + memory_specialization<NumBits,max_order_combined<Order>,3 >::offset_2(input_elem_offset)  ;
                          
                        memory_specialization<NumBits,max_order_combined<Order>,3 >::local_copy(c1,in1shared,c2,in2shared,in_polynomial_offset1,in_polynomial_offset2);

                        #pragma unroll
                        for( int i = 0; i < 2*VLI_SIZE; ++i)
                            res1[i] = 0;
                     
                        multiplies<NumBits>(res, res1, c1, c2); // the multiplication using boost pp
                }
                
                const unsigned int coefficient_id =  (output_degree_x*(output_degree_x*output_degree_x - 3*output_degree_x*(VLI_ExtendStride+1)
                                               + 3*VLI_ExtendStride*(VLI_ExtendStride+2) +2))/6
                                               + (VLI_ExtendStride - output_degree_x)*output_degree_y - (output_degree_y*output_degree_y-output_degree_y)/2 + output_degree_z;
                
                unsigned int * out2 = out + (coefficient_id * element_count *2* VLI_SIZE) + element_id; // coefficient->int_degree->element_id
                #pragma unroll
                for( int i = 0; i < 2*VLI_SIZE; ++i) {
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
    template <std::size_t NumBits, int Order>
    struct accelerator<NumBits, max_order_combined<Order>, 2>{
    inline static __device__ void polynomial_multiplication_max_order(const unsigned int* in1, const unsigned int* in2, const unsigned int element_count, unsigned int*  out, unsigned int*  workblock_count_by_warp, single_coefficient_task* execution_plan) {
        __shared__ unsigned int in1shared[memory_specialization<NumBits,max_order_combined<Order>,2 >::value1];
        __shared__ unsigned int in2shared[memory_specialization<NumBits,max_order_combined<Order>,2 >::value2];

        const unsigned int local_thread_id = threadIdx.x;
        const unsigned int element_id = blockIdx.x;
        
        unsigned int c1[VLI_SIZE],c2[VLI_SIZE];
        unsigned int res[2*VLI_SIZE];
        unsigned int res1[2*VLI_SIZE];
        
        unsigned int iteration_count = workblock_count_by_warp[local_thread_id / 32];
        const unsigned int input_elem_offset = element_id * vli::detail::max_order_combined_helpers::size<2+1, Order>::value  * VLI_SIZE;

        memory_specialization<NumBits,max_order_combined<Order>,2 >::shared_copy(in1shared, in1, in2shared, in2, input_elem_offset);

        for( int iteration_id = 0; iteration_id < iteration_count; ++iteration_id) {
            single_coefficient_task task = execution_plan[local_thread_id + (iteration_id * mul_block_size<max_order_combined<2*Order>, 2>::value)];
            const unsigned int step_count = task.step_count;
        
            if (step_count > 0) {
                #pragma unroll
                for( int i = 0; i < 2*VLI_SIZE; ++i)
                    res[i] = 0;
                
                const int output_degree_x = task.output_degree_x;
                const int output_degree_y = task.output_degree_y;
                //note min and max are in numeric 
                for(int current_degree_x=max(0, output_degree_x-Order); current_degree_x <= min(output_degree_x,Order) ;++current_degree_x)
                    for(int current_degree_y=max(0, output_degree_x+output_degree_y-Order-current_degree_x); current_degree_y <= min(output_degree_y,Order-current_degree_x) ; ++current_degree_y){
                        const unsigned int in_polynomial_offset1 = (  
                                                               (Order+1)*current_degree_x - (current_degree_x*current_degree_x-current_degree_x)/2 + current_degree_y
                                                             ) * VLI_SIZE + memory_specialization<NumBits,max_order_combined<Order>,3 >::offset_1(input_elem_offset)  ;
                        
                        const unsigned int in_polynomial_offset2 = ( 
                                                               (Order+1)*(output_degree_x-current_degree_x) - ((output_degree_x-current_degree_x)*(output_degree_x-current_degree_x)-(output_degree_x-current_degree_x))/2 + (output_degree_y-current_degree_y)
                                                             ) * VLI_SIZE + memory_specialization<NumBits,max_order_combined<Order>,3 >::offset_2(input_elem_offset)  ;

                        memory_specialization<NumBits,max_order_combined<Order>,2 >::local_copy(c1,in1shared,c2,in2shared,in_polynomial_offset1,in_polynomial_offset2);

                        #pragma unroll
                        for( int i = 0; i < 2*VLI_SIZE; ++i)
                            res1[i] = 0;
                     
                        multiplies<NumBits>(res, res1, c1, c2); // the multiplication using boost pp
                    }                   

                const unsigned int coefficient_id = VLI_ExtendStride*output_degree_x - (output_degree_x*output_degree_x-output_degree_x)/2 + output_degree_y;
                unsigned int * out2 = out + (coefficient_id * element_count *2* VLI_SIZE) + element_id; // coefficient->int_degree->element_id
                #pragma unroll
                for( int i = 0; i < 2*VLI_SIZE; ++i) {
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
    template <std::size_t NumBits, int Order>
    struct accelerator<NumBits, max_order_combined<Order>, 1>{
    inline static __device__ void polynomial_multiplication_max_order(const unsigned int* in1, const unsigned int* in2, const unsigned int element_count, unsigned int*  out, unsigned int*  workblock_count_by_warp, single_coefficient_task* execution_plan) {
        __shared__ unsigned int in1shared[memory_specialization<NumBits,max_order_combined<Order>,1 >::value1];
        __shared__ unsigned int in2shared[memory_specialization<NumBits,max_order_combined<Order>,1 >::value2];

        const unsigned int local_thread_id = threadIdx.x;
        const unsigned int element_id = blockIdx.x;
        
        unsigned int c1[VLI_SIZE],c2[VLI_SIZE];
        unsigned int res[2*VLI_SIZE];
        unsigned int res1[2*VLI_SIZE];
        
        unsigned int iteration_count = workblock_count_by_warp[local_thread_id / 32];
        
        const unsigned int input_elem_offset = element_id * stride<0,1,Order>::value * VLI_SIZE;

        memory_specialization<NumBits,max_order_combined<Order>,1 >::shared_copy(in1shared, in1, in2shared, in2, input_elem_offset);
        
        for( int iteration_id = 0; iteration_id < iteration_count; ++iteration_id) {
            single_coefficient_task task = execution_plan[local_thread_id + (iteration_id * mul_block_size<max_order_combined<2*Order>, 1>::value)];
            const unsigned int step_count = task.step_count;
        
            if (step_count > 0) {
                #pragma unroll
                for( int i = 0; i < 2*VLI_SIZE; ++i)
                    res[i] = 0;
                
                const unsigned int output_degree_x = task.output_degree_x;
                
                unsigned int current_degree_x = output_degree_x > Order ? output_degree_x - Order : 0;
                
                for( int step_id = 0; step_id < step_count; ++step_id) {
                
                    const unsigned int in_polynomial_offset1 = ( + current_degree_x
                                                         ) * VLI_SIZE + memory_specialization<NumBits,max_order_combined<Order>,1 >::offset_1(input_elem_offset)  ;
                    
                    const unsigned int in_polynomial_offset2 = ( + (output_degree_x - current_degree_x)
                                                         ) * VLI_SIZE + memory_specialization<NumBits,max_order_combined<Order>,1 >::offset_2(input_elem_offset)  ;
                
                    memory_specialization<NumBits,max_order_combined<Order>,1 >::local_copy(c1,in1shared,c2,in2shared,in_polynomial_offset1,in_polynomial_offset2);

                    #pragma unroll
                    for( int i = 0; i < 2*VLI_SIZE; ++i)
                        res1[i] = 0;
                 
                    multiplies<NumBits>(res, res1, c1, c2); // the multiplication using boost pp
                 
                    current_degree_x++;
                }
                
                const unsigned int coefficient_id =  output_degree_x;
                
                unsigned int * out2 = out + (coefficient_id * element_count *2* VLI_SIZE) + element_id; // coefficient->int_degree->element_id
                #pragma unroll
                for( int i = 0; i < 2*VLI_SIZE; ++i) {
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

    #undef VLI_ExtendStride
    #undef VLI_SIZE

    }//end namesoace detail
}//end namespace vli
