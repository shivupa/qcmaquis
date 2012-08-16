#ifndef MATRIX_BINDINGS_H
#define MATRIX_BINDINGS_H

namespace maquis { namespace bindings {

    template <typename O, typename I> struct binding { 
        static O convert(const I& m){ return static_cast<O>(m); }
    };

    template<typename O, typename I> O matrix_cast(I const& input){
       return binding<O,I>::convert(input);
    }

    template <typename T>
    struct binding< std::vector<T>, alps::numeric::diagonal_matrix<T> > {
        static std::vector<T> convert(const alps::numeric::diagonal_matrix<T>& m){
            return m.get_values();
        }
    };

    template <typename T, typename S, template<class M, class SS> class C>
    struct binding< std::vector< std::vector<T> >, C<alps::numeric::diagonal_matrix<T>, S> > {
        static std::vector< std::vector<T> > convert(const C<alps::numeric::diagonal_matrix<T>, S>& m){
            std::vector< std::vector<T> > set;
            for(size_t k = 0; k < m.n_blocks(); ++k){
                set.push_back(m[k].get_values());
            }
            return set;
        }
    };

#ifdef AMBIENT 

    template <typename T, typename S, template<class M, class SS> class C>
    struct binding< std::vector< std::vector<T> >, C<ambient::numeric::diagonal_matrix<T>, S> > {
        static std::vector< std::vector<T> > convert(const C<ambient::numeric::diagonal_matrix<T>, S>& m){
            std::vector< std::vector<T> > set;
            for(size_t k = 0; k < m.n_blocks(); ++k) 
                set.push_back(std::vector<T>(m[k].num_rows()));
            size_t num_cols(1);
            size_t num_rows;
            for(size_t k = 0; k < m.n_blocks(); ++k){
                num_rows = m[k].num_rows();
                std::vector<T>* v_ptr = &set[k];
                ambient::numeric::kernels::cast_to_vector<T>::spawn(v_ptr, m[k], num_rows, num_cols);
            }
            ambient::sync();
            return set;
        }
    };

    template <typename T>
    struct binding< ambient::numeric::matrix<T>, alps::numeric::matrix<T> > {
        static ambient::numeric::matrix<T> convert(const alps::numeric::matrix<T>& m){
            size_t num_rows = m.num_rows();
            size_t num_cols = m.num_cols();
            size_t lda = m.stride2();
            ambient::numeric::matrix<T> pm(num_rows, num_cols);    
            const std::vector<typename alps::numeric::matrix<T>::value_type>* v_ptr = &m.get_values();
            ambient::numeric::kernels::cast_from_vector<T>::spawn(v_ptr, pm, num_rows, num_cols, lda);
            ambient::sync();
            return pm;
        }
    };

    template <typename T>
    struct binding< alps::numeric::matrix<T>, ambient::numeric::matrix<T> > {
        static alps::numeric::matrix<T> convert(const ambient::numeric::matrix<T>& pm){
            size_t num_rows = pm.num_rows();
            size_t num_cols = pm.num_cols();
            alps::numeric::matrix<T> m(num_rows, num_cols);    
            std::vector<typename alps::numeric::matrix<T>::value_type>* v_ptr = &m.get_values();
            ambient::numeric::kernels::cast_to_vector<T>::spawn(v_ptr, pm, num_rows, num_cols);
            ambient::sync();
            return m;
        }
    };

    template <typename T>
    struct binding< ambient::numeric::diagonal_matrix<T>, alps::numeric::diagonal_matrix<T> > {
        static ambient::numeric::diagonal_matrix<T> convert(const alps::numeric::diagonal_matrix<T>& m){
            size_t num_rows(m.num_rows());
            size_t num_cols(1);
            ambient::numeric::diagonal_matrix<T> pm(num_rows, num_rows);    
            const std::vector<typename alps::numeric::diagonal_matrix<T>::value_type>* v_ptr = &m.get_values();
            ambient::numeric::kernels::cast_from_vector<T>::spawn(v_ptr, pm, num_rows, num_cols, num_rows);
            ambient::sync();
            return pm;
        }
    };

    template <typename T>
    struct binding< alps::numeric::diagonal_matrix<T>, ambient::numeric::diagonal_matrix<T> > {
        static alps::numeric::diagonal_matrix<T> convert(const ambient::numeric::diagonal_matrix<T>& pm){
            size_t num_cols(1);
            size_t num_rows = pm.num_rows();
            alps::numeric::diagonal_matrix<T> m(num_rows, num_rows);    
            std::vector<typename alps::numeric::diagonal_matrix<T>::value_type>* v_ptr = &m.get_values();
            ambient::numeric::kernels::cast_to_vector<T>::spawn(v_ptr, pm, num_rows, num_cols);
            ambient::sync();
            return m;
        }
    };

    template <typename T>
    struct binding< std::vector<T>, ambient::numeric::diagonal_matrix<T> > {
        static std::vector<T> convert(const ambient::numeric::diagonal_matrix<T>& pm){
            return binding<alps::numeric::diagonal_matrix<T>, ambient::numeric::diagonal_matrix<T> >::convert(pm).get_values();
        }
    };
#endif

} }

#ifdef AMBIENT

template<typename T>
bool operator == (alps::numeric::matrix<T> const & a, ambient::numeric::matrix<T> const & b) 
{
    ambient::future<int> ret(1);
    ambient::numeric::matrix<T> pa = maquis::bindings::matrix_cast<ambient::numeric::matrix<T> >(a);
    ambient::numeric::kernels::validation<T>::spawn(pa, b, ret); 
    return ((int)ret > 0);
}

template<typename T>
bool operator == (alps::numeric::diagonal_matrix<T> const & a, ambient::numeric::diagonal_matrix<T> const & b) 
{
    ambient::future<int> ret(1);
    ambient::numeric::diagonal_matrix<T> pa = maquis::bindings::matrix_cast<ambient::numeric::diagonal_matrix<T> >(a);
    ambient::numeric::kernels::validation<T>::spawn(pa, b, ret); 
    return ((int)ret > 0);
}

template<typename T>
bool operator == (ambient::numeric::matrix<T> const & pm, alps::numeric::matrix<T> const & m){
    return (m == pm);
}

template<typename T>
bool operator == (ambient::numeric::diagonal_matrix<T> const & pm, alps::numeric::diagonal_matrix<T> const & m){
    return (m == pm);
}

#endif
#endif
