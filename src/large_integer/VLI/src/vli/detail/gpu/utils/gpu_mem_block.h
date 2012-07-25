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

#include "vli/detail/gpu/utils/singleton.h"

#ifndef GPU_MEM_BLOCK_H
#define GPU_MEM_BLOCK_H

namespace vli {
namespace detail {

    // we allocate the mem only one time so pattern of this class singleton
    template <typename BaseInt>
    class gpu_memblock : public Singleton<gpu_memblock<BaseInt> >  {
        friend class Singleton<gpu_memblock>; // to have access to the Instance, Destroy functions into the singleton class
        public:
        typedef BaseInt value_type;
        private:
        gpu_memblock();
        gpu_memblock(gpu_memblock const& );
        gpu_memblock& operator=(gpu_memblock const& );

        std::size_t block_size_;

        public:
        ~gpu_memblock();
        std::size_t  GetBlockSize() const;

        BaseInt* V1Data_; // input vector 1
        BaseInt* V2Data_; // input vector 2
        BaseInt* VinterData_; // inter value before the final reduction
        BaseInt* PoutData_; // final output
    };
    
    } //end namespace detail
} //end namespce vli  

#endif //INNER_PRODUCT_GPU_BOOSTER_HPP
