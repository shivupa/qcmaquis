/*
 * Ambient, License - Version 1.0 - May 3rd, 2012
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

#ifndef AMBIENT_NUMERIC_TILES_H
#define AMBIENT_NUMERIC_TILES_H

#include "ambient/numeric/traits.hpp"

#ifdef AMBIENT_ALPS_HDF5
#include <alps/hdf5.hpp>
#include <alps/hdf5/complex.hpp>
#endif

namespace ambient { namespace numeric {

    template <class Matrix>
    class tiles : public ambient::memory::use_fixed_new<tiles<Matrix> > {
    public:
        typedef typename Matrix::value_type  value_type;
        typedef typename Matrix::size_type   size_type;
        typedef typename Matrix::real_type   real_type;
        typedef typename Matrix::scalar_type scalar_type;
        typedef typename Matrix::difference_type difference_type;
        typedef typename Matrix::allocator_type allocator_type;

        static tiles<Matrix> identity_matrix(size_type size);

       ~tiles();
        explicit tiles();
        explicit tiles(Matrix* a);
        explicit tiles(size_type rows, size_type cols, value_type init_value = value_type());
        tiles<subset_view<Matrix> > subset(size_type i, size_type j, size_type mt, size_type nt) const;
        tiles(const tiles& a);
        tiles& operator = (const tiles& rhs);
        template <class MatrixB> tiles& operator  = (const tiles<MatrixB>& rhs);
        size_type num_rows() const;
        size_type num_cols() const;
        scalar_type trace() const;
        void transpose();
        void conj();
        bool empty() const;          
        void swap(tiles& r);
        void resize(size_type m, size_type n); 
        Matrix& tile(size_type i, size_type j);
        const Matrix& tile(size_type i, size_type j) const;
        Matrix& locate(size_type i, size_type j);
        const Matrix& locate(size_type i, size_type j) const;
        size_t addr(size_type i, size_type j) const;
        Matrix& operator[] (size_type k);
        const Matrix& operator[] (size_type k) const;
        template <class MatrixB> operator tiles<MatrixB> () const;
        template <class MatrixB> tiles& operator += (const tiles<MatrixB>& rhs);
        template <class MatrixB> tiles& operator -= (const tiles<MatrixB>& rhs);
        template <typename T2> tiles& operator *= (const T2& t);
        template <typename T2> tiles& operator /= (const T2& t);
        value_type& operator() (size_type i, size_type j);
        const value_type& operator() (size_type i, size_type j) const;

        #ifdef AMBIENT_ALPS_HDF5
        friend void load(alps::hdf5::archive& ar
                         , std::string const& path
                         , tiles<Matrix>& m
                         , std::vector<std::size_t> chunk  = std::vector<std::size_t>()
                         , std::vector<std::size_t> offset = std::vector<std::size_t>()
                         )
        {
            std::vector<std::size_t> size(ar.extent(path));
            tiles<Matrix> r(size[chunk.size()+1], size[chunk.size()]);
            m.swap(r);

            std::vector<std::size_t> first(alps::hdf5::get_extent(value_type()));
            
            std::size_t const chunk_cols_index = chunk.size();
            chunk.push_back(0); // to be filled later
            std::size_t const chunk_row_index = chunk.size();
            chunk.push_back(0); // to be filled later
            copy(first.begin(),first.end(), std::back_inserter(chunk));
            
            std::size_t const offset_col_index = offset.size();
            offset.push_back(0); // to be filled later
            std::size_t const offset_row_index = offset.size();
            offset.push_back(0); // to be filled later
            fill_n(std::back_inserter(offset), first.size(), 0);
            
            for(int j = 0; j < m.nt; ++j){
                for(int i = 0; i < m.mt; ++i){
                    chunk[chunk_cols_index] = m.tile(i,j).num_cols();
                    chunk[chunk_row_index] = m.tile(i,j).num_rows();
                    offset[offset_row_index] = i*AMBIENT_IB;
                    offset[offset_col_index] = j*AMBIENT_IB;

                    if(!ambient::exclusive(m.tile(i,j)))
                    ar.read(path, (typename traits::real_type<value_type>::type *)ambient::naked(m.tile(i,j)), chunk, offset);
                }
            }
        }

        friend void save(alps::hdf5::archive& ar
                         , std::string const& path
                         , tiles<Matrix> const& m
                         , std::vector<std::size_t> size   = std::vector<std::size_t>()
                         , std::vector<std::size_t> chunk  = std::vector<std::size_t>()
                         , std::vector<std::size_t> offset = std::vector<std::size_t>()
                         )
        {
            size.push_back(m.cols);
            size.push_back(m.rows);
            std::vector<std::size_t> first(alps::hdf5::get_extent(value_type()));
            std::copy(first.begin(), first.end(), std::back_inserter(size));
            
            std::size_t const chunk_cols_index = chunk.size();
            chunk.push_back(0); // to be filled later
            std::size_t const chunk_row_index = chunk.size();
            chunk.push_back(0); // to be filled later
            copy(first.begin(),first.end(), std::back_inserter(chunk));
            
            std::size_t const offset_col_index = offset.size();
            offset.push_back(0); // to be filled later
            std::size_t const offset_row_index = offset.size();
            offset.push_back(0); // to be filled later
            fill_n(std::back_inserter(offset), first.size(), 0);
            
            for(int j = 0; j < m.nt; ++j){
                for(int i = 0; i < m.mt; ++i){
                    chunk[chunk_cols_index] = m.tile(i,j).num_cols();
                    chunk[chunk_row_index] = m.tile(i,j).num_rows();
                    offset[offset_row_index] = i*AMBIENT_IB;
                    offset[offset_col_index] = j*AMBIENT_IB;
                    
                    using alps::hdf5::detail::get_pointer;
                    assert(ambient::naked(m.tile(i,j)).state == ambient::locality::local);
                    if(ambient::weak(m.tile(i,j))) throw std::runtime_error("Error: attempting to write uninitialised data!");
                    ar.write(path, (typename traits::real_type<value_type>::type *)ambient::naked(m.tile(i,j)), size, chunk, offset);
                }
            }
        }
        #endif
    public:
        std::vector<Matrix*> data;
        size_type rows;
        size_type cols;
        size_type mt;
        size_type nt;
    };

    template <class Matrix>
    class tiles<subset_view<Matrix> >{
    public:
        typedef typename Matrix::difference_type difference_type;
        typedef typename Matrix::size_type  size_type;
        typedef typename Matrix::value_type value_type;
        typedef typename Matrix::allocator_type allocator_type;

        Matrix& tile(size_type i, size_type j);
        const Matrix& tile(size_type i, size_type j) const;
        Matrix& operator[] (size_type k);
        const Matrix& operator[] (size_type k) const;
        tiles<subset_view<Matrix> > subset(size_type i, size_type j, size_type mt, size_type nt) const;
        template <class MatrixB> tiles& operator += (const tiles<MatrixB>& rhs);
        template <class MatrixB> tiles& operator -= (const tiles<MatrixB>& rhs);
        template <class MatrixB> tiles& operator  = (const tiles<MatrixB>& rhs);
        tiles& operator  = (const tiles& rhs);
        std::vector<subset_view<Matrix> > data;
        size_type num_rows() const;
        size_type num_cols() const;
        Matrix& locate(size_type i, size_type j);
        const Matrix& locate(size_type i, size_type j) const;
        size_t addr(size_type i, size_type j) const;
        size_type rows;
        size_type cols;
        size_type mt;
        size_type nt;
    };

    template <typename T>
    class tiles<diagonal_matrix<T> > : public ambient::memory::use_fixed_new<tiles<diagonal_matrix<T> > > {
    public:
        typedef typename diagonal_matrix<T>::value_type  value_type;
        typedef typename diagonal_matrix<T>::size_type   size_type;
        typedef typename diagonal_matrix<T>::real_type   real_type;
        typedef typename diagonal_matrix<T>::scalar_type scalar_type;
        typedef typename diagonal_matrix<T>::difference_type difference_type;

        static tiles identity_matrix(size_type size);

        explicit tiles();
        explicit tiles(size_type rows, size_type cols, value_type init_value = value_type()); 
        tiles(const tiles& a);
        tiles& operator = (const tiles& rhs); 
       ~tiles();
    public:
        std::pair<const value_type*,const value_type*> diagonal() const;
        const value_type* begin() const;
        const value_type* end() const; // actual only for merged case
        size_type num_rows() const;
        size_type num_cols() const;
        void swap(tiles& r);
        void resize(size_type m, size_type n); 
        diagonal_matrix<T>& operator[] (size_type k);
        const diagonal_matrix<T>& operator[] (size_type k) const;
        value_type& operator() (size_type i, size_type j);
        const value_type& operator() (size_type i, size_type j) const;
    public:
        std::vector<diagonal_matrix<T>*> data;
        size_type size;
        size_type nt;
    };

} }

#ifdef AMBIENT_ALPS_HDF5
namespace alps { namespace hdf5 {
    template<class Matrix>
    struct has_complex_elements<ambient::numeric::tiles<Matrix> >
    : public has_complex_elements<typename alps::detail::remove_cvr<typename Matrix::value_type>::type>
    {};
} }
#endif
#endif
