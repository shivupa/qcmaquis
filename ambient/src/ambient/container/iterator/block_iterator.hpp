/*
 * Copyright Institute for Theoretical Physics, ETH Zurich 2015.
 * Distributed under the Boost Software License, Version 1.0.
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef AMBIENT_CONTAINER_ITERATOR_BLOCK_ITERATOR_HPP
#define AMBIENT_CONTAINER_ITERATOR_BLOCK_ITERATOR_HPP

namespace ambient {

    template<class PartitionedContainer>
    class block_iterator {
    public:
        typedef typename PartitionedContainer::iterator base_iterator;
        typedef typename PartitionedContainer::partition_type block_type;
        static constexpr int ib = PartitionedContainer::ib;

        block_iterator(base_iterator first, base_iterator last) 
        : base(first) 
        {
            position = first-base.get_container().cbegin();
            limit = last-base.get_container().cbegin();
            measure_step();
        }
        void operator++ (){
            position += step;
            measure_step();
        }
        size_t n_blocks(){
            size_t count = 0;
            for(size_t pos = position; pos != limit; count++)
                pos += std::min(ib*__a_ceil((pos+1)/ib) - pos, limit-pos);
            return count;
        }
        size_t offset() const {
            return position - (base - base.get_container().cbegin());
        }
        bool operator != (base_iterator it){
            return (position != it-base.get_container().cbegin());
        }
        block_type& operator* (){
            return locate(position);
        }
        block_type& locate(size_t p){
            return base.get_container().locate(p);
        }
        void measure_step(){
            step = std::min(ib*__a_ceil((position+1)/ib) - position, limit-position);
            first = position % ib;
            second = first+step;
        }
        size_t first;
        size_t second;
        size_t step;
    private:
        size_t position;
        size_t limit;
        base_iterator base;
    };

}

#endif
