/***********************************************************************************
 * Copyright (C) 2012 by Andreas Hehn <hehn@phys.ethz.ch>                          *
 *                       ETH Zurich                                                *
 ***********************************************************************************/
#ifndef VLI_POLYNOMIAL_HPP
#define VLI_POLYNOMIAL_HPP
#include <vli/polynomial/variable.hpp>
#include <vli/polynomial/numeric.hpp>
#include <vli/polynomial/monomial.hpp>
#include <vli/polynomial/detail/polynomial_impl.hpp>
#include <boost/type_traits/is_fundamental.hpp>

#define POLYNOMIAL_CLASS polynomial<Coeff,OrderSpecification,Var0,Var1,Var2,Var3>

namespace vli {

//------------------------------------------------------------------------ 
// The polynomial class
//------------------------------------------------------------------------ 

template <class Coeff, class OrderSpecification, class Var0, class Var1 = vli::no_variable, class Var2 = vli::no_variable, class Var3 = vli::no_variable>
class polynomial : public detail::storage<Coeff,OrderSpecification,Var0,Var1,Var2,Var3> {
  public:
    typedef detail::storage<Coeff,OrderSpecification,Var0,Var1,Var2,Var3> base_type;
    typedef typename  base_type::value_type                 value_type;
    typedef typename  base_type::element_descriptor         element_descriptor;
    typedef OrderSpecification                              order_specification;
    typedef typename  base_type::iterator                   iterator;
    typedef typename  base_type::const_iterator             const_iterator;
    typedef typename  base_type::reverse_iterator           reverse_iterator;
    typedef typename  base_type::const_reverse_iterator     const_reverse_iterator;
    typedef unsigned int                                    exponent_type;      // Type of the exponents (has to be the same type as Vli::size_type)

    polynomial() {
        detail::init(*this, typename boost::is_fundamental<Coeff>::type());
    }
    
    explicit polynomial(int i) {
        detail::init(*this, typename boost::is_fundamental<Coeff>::type());
        base_type::operator[](0) = value_type(i);
    }
    
    template <class Coeff2>
    explicit polynomial(polynomial<Coeff2,OrderSpecification,Var0,Var1,Var2,Var3> const& p) {
        iterator it = this->begin();
        typename polynomial<Coeff2,OrderSpecification,Var0,Var1,Var2,Var3>::const_iterator it2 = p.begin();
        while(it != this->end() )
            *it++ = static_cast<Coeff>(*it2++);
    }

    friend void swap(polynomial& a, polynomial& b) {
        using boost::swap;
        swap(static_cast<base_type&>(a),static_cast<base_type&>(b));
    }

    void print(std::ostream& os) const {
        detail::print_helper<polynomial>::apply(os,*this);
    }

    template <class T>
    polynomial& operator += (T const& t) {
        using detail::additive_op_assign;
        additive_op_assign(*this, t, detail::operations::plus_assign());
        return *this;
    }

    template <class T>
    polynomial& operator -= (T const& t) {
        using detail::additive_op_assign;
        additive_op_assign(*this, t, detail::operations::minus_assign());
        return *this;
    }

    template <class T>
    polynomial& operator *= (T const& t) {
        using detail::multiplicative_op_assign;
        multiplicative_op_assign(*this, t, detail::operations::multiply_assign());
        return *this;
    }

    template <class T>
    polynomial& operator /= (T const& t) {
        using detail::multiplicative_op_assign;
        multiplicative_op_assign(*this, t, detail::operations::devide_assign());
        return *this;
    }

    template <typename Coeff2, typename MVar0, typename MVar1, typename MVar2, typename MVar3>
    polynomial& operator *= (monomial<Coeff2, MVar0, MVar1, MVar2, MVar3> const& m) {
        detail::multiply_assign_monomial_helper<polynomial>::apply(*this,m);
        return *this;
    }
    
    polynomial operator - () const {
        polynomial p(*this);
        detail::negate(p);
        return p;
    }

    bool operator == (polynomial const& p) const {
        return detail::equal_helper<polynomial>()(*this,p);
    }

    using base_type::operator();
    // for direct access, I removed the element descriptor
    inline value_type& operator()(exponent_type i, exponent_type j=0, exponent_type k=0, exponent_type l=0) {
        return base_type::operator()(i,j,k,l);
    //     return base_type::operator()(element_descriptor(i,j,k,l));
    }

    inline value_type const& operator()(exponent_type i, exponent_type j=0, exponent_type k=0, exponent_type l=0) const {
        return base_type::operator()(i,j,k,l);
    //    return base_type::operator()(element_descriptor(i,j,k,l));
    }
};


//------------------------------------------------------------------------ 
// Operators and free functions
//------------------------------------------------------------------------ 

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
POLYNOMIAL_CLASS operator + (POLYNOMIAL_CLASS p1, POLYNOMIAL_CLASS const& p2) {
    p1 += p2;
    return p1;
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3, class Addend>
POLYNOMIAL_CLASS operator + (POLYNOMIAL_CLASS p, Addend const& a) {
    p += a;
    return p;
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3, class Addend>
POLYNOMIAL_CLASS operator + (Addend const& a, POLYNOMIAL_CLASS const& p) {
    return p + a;
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
POLYNOMIAL_CLASS operator - (POLYNOMIAL_CLASS p1, POLYNOMIAL_CLASS const& p2) {
    p1 -= p2;
    return p1;
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3, class Addend>
POLYNOMIAL_CLASS operator - (POLYNOMIAL_CLASS p, Addend const& a) {
    p -= a;
    return p;
}

// Polynomial * Polynomial  for the moment we only provide the multiplication between two identical polynomial types
template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
typename polynomial_multiply_result_type<POLYNOMIAL_CLASS >::type operator * (POLYNOMIAL_CLASS const& p1, POLYNOMIAL_CLASS const& p2) {
    return detail::polynomial_multiply_helper<POLYNOMIAL_CLASS>::apply(p1,p2);
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
typename polynomial_multiply_keep_order_result_type<POLYNOMIAL_CLASS>::type multiply_keep_order(POLYNOMIAL_CLASS const& p1, POLYNOMIAL_CLASS const& p2) {
    return detail::polynomial_multiply_keep_order_helper<POLYNOMIAL_CLASS>::apply(p1,p2);
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3, class MCoeff, class MVar0, class MVar1, class MVar2, class MVar3>
POLYNOMIAL_CLASS operator * (POLYNOMIAL_CLASS p, monomial<MCoeff,MVar0,MVar1,MVar2,MVar3> const& m) {
    p *= m;
    return p;
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3, class MCoeff, class MVar0, class MVar1, class MVar2, class MVar3>
POLYNOMIAL_CLASS operator * (monomial<MCoeff,MVar0,MVar1,MVar2,MVar3> const& m, POLYNOMIAL_CLASS const& p) {
    return p*m;
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3, class Coeff2>
POLYNOMIAL_CLASS operator * (POLYNOMIAL_CLASS p, Coeff2 const& c) {
    p*=c;
    return p; 
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3, class Coeff2>
POLYNOMIAL_CLASS operator * (Coeff2 const& c, POLYNOMIAL_CLASS const& p ) {
    return p*c; 
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
POLYNOMIAL_CLASS operator / (POLYNOMIAL_CLASS p, int c) {
    p /= c;
    return p;
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
POLYNOMIAL_CLASS operator / (POLYNOMIAL_CLASS p, Coeff const& c) {
    p /= c;
    return p;
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
std::ostream& operator << (std::ostream& os, POLYNOMIAL_CLASS const& p) {
    p.print(os);
    return os;
}


//------------------------------------------------------------------------ 
// Specializations
//------------------------------------------------------------------------ 

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
bool is_zero(POLYNOMIAL_CLASS const& p) {
    return detail::is_zero_helper(p);
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
void negate_inplace(POLYNOMIAL_CLASS& p) {
    detail::negate(p);
}

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
struct variable<POLYNOMIAL_CLASS,0> {
    typedef Var0 type;
};

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
struct variable<POLYNOMIAL_CLASS,1> {
    typedef Var1 type;
};

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
struct variable<POLYNOMIAL_CLASS,2> {
    typedef Var2 type;
};

template <class Coeff, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
struct variable<POLYNOMIAL_CLASS,3> {
    typedef Var3 type;
};


//------------------------------------------------------------------------ 
// Specializations for vli_cpu
//------------------------------------------------------------------------ 

// TODO move someplace else

template <class BaseInt, std::size_t Size>
class vli_cpu;

template <typename BaseInt, std::size_t Size, unsigned int Order, class Var0, class Var1, class Var2, class Var3>
struct polynomial_multiply_result_type<polynomial<vli_cpu<BaseInt,Size>,max_order_each<Order>,Var0,Var1,Var2,Var3> > {
    typedef polynomial<vli_cpu<BaseInt,2*Size>,max_order_each<2*Order>,Var0,Var1,Var2,Var3> type;
};

template <typename BaseInt, std::size_t Size, unsigned int Order, class Var0, class Var1, class Var2, class Var3>
struct polynomial_multiply_result_type<polynomial<vli_cpu<BaseInt,Size>,max_order_combined<Order>,Var0,Var1,Var2,Var3> > {
    typedef polynomial<vli_cpu<BaseInt,2*Size>,max_order_combined<2*Order>,Var0,Var1,Var2,Var3> type;
};

template <typename BaseInt, std::size_t Size, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
struct polynomial_multiply_keep_order_result_type<polynomial<vli_cpu<BaseInt,Size>,OrderSpecification,Var0,Var1,Var2,Var3> > {
    typedef polynomial<vli_cpu<BaseInt,2*Size>,OrderSpecification,Var0,Var1,Var2,Var3> type;
};


namespace detail {
    template <class BaseInt, std::size_t Size, class OrderSpecification, class Var0, class Var1, class Var2, class Var3>
    struct equal_helper<polynomial<vli_cpu<BaseInt,Size>,OrderSpecification,Var0,Var1,Var2,Var3> > {
        typedef polynomial<vli_cpu<BaseInt,Size>,OrderSpecification,Var0,Var1,Var2,Var3> polynomial_type;
            bool operator()(polynomial_type const& p, polynomial_type const& p2) {
            // TODO check if this is ok
            int n = memcmp((void*)p.begin(),(void*)p2.begin(),((char*)p.end()-(char*)p.begin())*sizeof(char));
            return (0 == n);
        }
    };
}

} // end namespace vli

#undef POLYNOMIAL_CLASS

#endif //VLI_POLYNOMIAL_HPP
