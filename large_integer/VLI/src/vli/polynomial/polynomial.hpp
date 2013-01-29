/***********************************************************************************
 * Copyright (C) 2012 by Andreas Hehn <hehn@phys.ethz.ch>                          *
 *                       ETH Zurich                                                *
 ***********************************************************************************/
#ifndef VLI_POLYNOMIAL_HPP
#define VLI_POLYNOMIAL_HPP
#include <vli/polynomial/variable.hpp>
#include <vli/polynomial/polynomial_traits.hpp>
#include <vli/polynomial/numeric.hpp>
#include <vli/polynomial/monomial.hpp>
#include <vli/polynomial/detail/polynomial_impl.hpp>
#include <boost/type_traits/is_fundamental.hpp>

#define POLYNOMIAL_CLASS polynomial<Coeff,MaxOrder,Var0,Var1,Var2,Var3>

namespace vli {

//------------------------------------------------------------------------ 
// The polynomial class
//------------------------------------------------------------------------ 

/*! \class polynomial
        \brief This class models a dense or triangular polynomial up to 4 variables with an arbitrary Order

        The class is template over the Coefficients (e.g. a VLI number), a type of polynomial max_order_each(dense) or max_order_combined(triangular), and the variables vli::var<'x'>
 
*/
template <class Coeff, class MaxOrder, class Var0, class Var1 = no_variable, class Var2 = no_variable, class Var3 = no_variable>
class polynomial : public detail::storage<Coeff,MaxOrder,num_variables<polynomial<Coeff,MaxOrder,Var0,Var1,Var2,Var3> >::value> {
  public:
    /*! \cond I do not need this part in the doc*/
    typedef detail::storage<Coeff,MaxOrder,num_variables<polynomial>::value> base_type;
    /* \endcond */
    /*! \brief Type of the coefficient  */
    typedef typename  base_type::value_type                 value_type;
    /*! \cond I do not need this part in the doc*/
    typedef detail::element_descriptor_impl<Var0, Var1, Var2, Var3> element_descriptor;
    /* \endcond */
    /*! \brief Order of the polynomial */
    typedef MaxOrder                                        max_order;
    /*! \brief iterator std compatibility  */
    typedef typename  base_type::iterator                   iterator;
    /*! \brief const_iterator std compatibility  */
    typedef typename  base_type::const_iterator             const_iterator;
    /*! \brief reverse_iterator std compatibility  */
    typedef typename  base_type::reverse_iterator           reverse_iterator;
    /*! \brief const_reverse_iterator std compatibility  */
    typedef typename  base_type::const_reverse_iterator     const_reverse_iterator;
    /*! \brief Type of the exponents (has to be the same type as Vli::size_type */
    typedef unsigned int                                    exponent_type;

    /**
    \brief Default constructor
       If the coefficients are fundamentals type (int, double, ...), the coefficients will be initialize to 0 else nothing
    */
    polynomial() {
        detail::init(*this, typename boost::is_fundamental<value_type>::type());
    }

    /**
    \brief  Constructor with funcamental type or third-party library
    */
    template <class T>
    explicit polynomial(T const& v, typename boost::enable_if_c<boost::is_same<T,int>::value || boost::is_same<T,value_type>::value>::type* dummy = 0 ) {
        detail::init(*this, typename boost::is_fundamental<value_type>::type());
        base_type::operator[](0) = static_cast<value_type>(v);
    }

    /**
    \brief copy constructor
     \param p polynomial with different coefficient type
    The polynomial is initialized by copy, it necessitate a cast between the two kind of coefficients
    */
    template <class Coeff2>
    explicit polynomial(polynomial<Coeff2,MaxOrder,Var0,Var1,Var2,Var3> const& p) {
        iterator it = this->begin();
        typename polynomial<Coeff2,MaxOrder,Var0,Var1,Var2,Var3>::const_iterator it2 = p.begin();
        while(it != this->end() )
            *it++ = static_cast<Coeff>(*it2++);
    }

    /* \cond */
    friend void swap(polynomial& a, polynomial& b) {
        using boost::swap;
        swap(static_cast<base_type&>(a),static_cast<base_type&>(b));
    }

    void print(std::ostream& os) const {
        detail::print_helper<polynomial>::apply(os,*this);
    }
    /* \endcond */

    /**
    \brief Perform an addition between a polynomial and an other type
    \param t a template parameter
        
    This operator performs an addition between a polynomial and one of this type : polynomial, monomial, coefficient  and int 64-bit.
    \note Usual rules of polynomial arithemtic are respected.
    */
    template <class T>
    polynomial& operator += (T const& t) {
        using detail::additive_op_assign;
        additive_op_assign(*this, t, detail::operations::plus_assign());
        return *this;
    }

    /**
    \brief Perform a substraction between a polynomial and an other type
    \param t a template parameter
        
    This operator performs an substraction between a polynomial and one of this type : polynomial, monomial, coefficient  and int 64-bit.
    \note Usual rules of polynomial arithemtic are respected.
    */
    template <class T>
    polynomial& operator -= (T const& t) {
        using detail::additive_op_assign;
        additive_op_assign(*this, t, detail::operations::minus_assign());
        return *this;
    }

    /**
    \brief Perform a multiplication between a polynomial and an other type
    \param t a template parameter
        
    This operator performs a multiplication between a polynomial and one of this type : polynomial, monomial, coefficient  and int 64-bit.
    \note Usual rules of polynomial arithemtic are respected.
    */
    template <class T>
    polynomial& operator *= (T const& t) {
        using detail::multiplicative_op_assign;
        multiplicative_op_assign(*this, t, detail::operations::multiply_assign());
        return *this;
    }

    /**
    \brief Perform a division between a polynomial and an other type
    \param t a template parameter
        
    This operator performs a division between a polynomial and one of this type : polynomial, monomial, coefficient  and int 64-bit.
    \note Usual rules of polynomial arithemtic are respected.
    */
    template <class T>
    polynomial& operator /= (T const& t) {
        using detail::multiplicative_op_assign;
        multiplicative_op_assign(*this, t, detail::operations::devide_assign());
        return *this;
    }

    /*! \cond I do not need this part in the doc*/
    template <typename Coeff2, typename MVar0, typename MVar1, typename MVar2, typename MVar3>
    polynomial& operator *= (monomial<Coeff2, MVar0, MVar1, MVar2, MVar3> const& m) {
        detail::multiply_assign_monomial_helper<polynomial>::apply(*this,m);
        return *this;
    }
    /* \endcond */
    /**
    \brief negate a polynomial
    */
    polynomial operator - () const {
        polynomial p(*this);
        detail::negate(p);
        return p;
    }

    /**
    \brief Test the equality bewten two polynomial
    \return bool
    */
    bool operator == (polynomial const& p) const {
        return detail::equal_helper<polynomial>()(*this,p);
    }

    using base_type::operator();

    /*! \cond I do not need this part in the doc*/
    inline value_type& operator()(element_descriptor const& e) {
        return base_type::operator()(exponent(e,Var0()),exponent(e,Var1()),exponent(e,Var2()),exponent(e,Var3()));
    }

#ifdef ALPS_HAVE_HDF5
    void save(alps::hdf5::archive& ar) const {
        using alps::make_pvp;
        using std::distance;
        ar << make_pvp("coefficients",std::vector<value_type>(this->begin(),this->end()));
    }
    void load(alps::hdf5::archive& ar) {
        using alps::make_pvp;
        using std::copy;
        std::vector<value_type> v;
        ar >> make_pvp("coefficients",v);
        if( v.size() != this->size() )
            throw std::runtime_error("Polynomial order mismatch while loading from hdf5.");
        copy(v.begin(),v.end(),this->begin());
    }
#endif //ALPS_HAVE_HDF5

    inline value_type const& operator()(element_descriptor const& e) const {
        return base_type::operator()(exponent(e,Var0()),exponent(e,Var1()),exponent(e,Var2()),exponent(e,Var3()));
    }
    /* \endcond */

};


//------------------------------------------------------------------------ 
// Operators and free functions
//------------------------------------------------------------------------ 

/**
\brief addition between two polynomial of the same type
\return polynomial
\param p1 polynomial
\param p2 polynomial

Basic addition bewteen two polynomial of the same type.
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
POLYNOMIAL_CLASS operator + (POLYNOMIAL_CLASS p1, POLYNOMIAL_CLASS const& p2) {
    p1 += p2;
    return p1;
}

/**
\brief addition between a polynomial and template parameter
\return polynomial
\param p polynomial
\param a template parameter
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3, class Addend>
POLYNOMIAL_CLASS operator + (POLYNOMIAL_CLASS p, Addend const& a) {
    p += a;
    return p;
}

/**
\brief addition between a polynomial and template parameter
\return polynomial
\param a template parameter
\param p polynomial
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3, class Addend>
POLYNOMIAL_CLASS operator + (Addend const& a, POLYNOMIAL_CLASS const& p) {
    return p + a;
}

/**
\brief addition between two polynomial
\return polynomial
\param p1 polynomial
\param p2 polynomial
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
POLYNOMIAL_CLASS operator - (POLYNOMIAL_CLASS p1, POLYNOMIAL_CLASS const& p2) {
    p1 -= p2;
    return p1;
}
/**
\brief substraction between a polynomial and template parameter
\return polynomial
\param p polynomial
\param a template parameter
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3, class Addend>
POLYNOMIAL_CLASS operator - (POLYNOMIAL_CLASS p, Addend const& a) {
    p -= a;
    return p;
}

/**
\brief multiplication between two polynomial of the same type, the initial order is not conserved
\return polynomial
\param p1 polynomial
\param p2 polynomial

Basic multiplication bewteen two polynomial of the same type. The order of the return polynomial is twice larger
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
typename polynomial_multiply_result_type<POLYNOMIAL_CLASS >::type operator * (POLYNOMIAL_CLASS const& p1, POLYNOMIAL_CLASS const& p2) {
    return detail::polynomial_multiply_helper<POLYNOMIAL_CLASS>::apply(p1,p2);
}

/**
\brief multiplication between two polynomial of the same type, the initial order is not conserved
\return polynomial
\param p1 polynomial
\param p2 polynomial

Basic multiplication bewteen two polynomial of the same type. The order of polynomial is conserved
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
typename polynomial_multiply_keep_order_result_type<POLYNOMIAL_CLASS>::type multiply_keep_order(POLYNOMIAL_CLASS const& p1, POLYNOMIAL_CLASS const& p2) {
    return detail::polynomial_multiply_keep_order_helper<POLYNOMIAL_CLASS>::apply(p1,p2);
}

/**
\brief multiplication between a polynomial and a monomial
\return polynomial
\param p polynomial
\param m monomial

Basic multiplication bewteen a polynomial and a monomial
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3, class MCoeff, class MVar0, class MVar1, class MVar2, class MVar3>
POLYNOMIAL_CLASS operator * (POLYNOMIAL_CLASS p, monomial<MCoeff,MVar0,MVar1,MVar2,MVar3> const& m) {
    p *= m;
    return p;
}

/**
\brief multiplication between a monomial and a polynomial
\return polynomial
\param m monomial
\param p polynomial

Basic multiplication bewteen a monomial and a polynomial 
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3, class MCoeff, class MVar0, class MVar1, class MVar2, class MVar3>
POLYNOMIAL_CLASS operator * (monomial<MCoeff,MVar0,MVar1,MVar2,MVar3> const& m, POLYNOMIAL_CLASS const& p) {
    return p*m;
}

/**
\brief multiplication between a coefficient and a polynomial
\return polynomial
\param p polynomial
\param c coefficient

Basic multiplication bewteen a polynomial and a coefficient
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3, class Coeff2>
POLYNOMIAL_CLASS operator * (POLYNOMIAL_CLASS p, Coeff2 const& c) {
    p *= c;
    return p;
}

/**
\brief multiplication between a polynomial and a coefficient
\return polynomial
\param c coefficient
\param p polynomial

Basic multiplication bewteen a coefficient and a polynomial
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3, class Coeff2>
POLYNOMIAL_CLASS operator * (Coeff2 const& c, POLYNOMIAL_CLASS const& p ) {
    return p*c;
}

/**
\brief division between a polynomial and an int
\return polynomial
\param p polynomial
\param c int

Basic division bewteen a polynomial and an int
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
POLYNOMIAL_CLASS operator / (POLYNOMIAL_CLASS p, int c) {
    p /= c;
    return p;
}

/**
\brief division between a polynomial and a coefficient
\return polynomial
\param p polynomial
\param c coefficient

Basic division bewteen a polynomial and a coefficient
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
POLYNOMIAL_CLASS operator / (POLYNOMIAL_CLASS p, Coeff const& c) {
    p /= c;
    return p;
}

/**
    \brief  stream operator
*/
template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
std::ostream& operator << (std::ostream& os, POLYNOMIAL_CLASS const& p) {
    p.print(os);
    return os;
}

/* \cond I do not need this part in the doc*/

template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
void truncate_inplace(POLYNOMIAL_CLASS & p, unsigned int order) {
    detail::loop_helper<POLYNOMIAL_CLASS>::apply(p,detail::truncate_helper(order));
}

//------------------------------------------------------------------------ 
// Specializations
//------------------------------------------------------------------------ 

template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
bool is_zero(POLYNOMIAL_CLASS const& p) {
    return detail::is_zero_helper(p);
}

template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
void negate_inplace(POLYNOMIAL_CLASS& p) {
    detail::negate(p);
}

template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
struct variable<POLYNOMIAL_CLASS,0> {
    typedef Var0 type;
};

template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
struct variable<POLYNOMIAL_CLASS,1> {
    typedef Var1 type;
};

template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
struct variable<POLYNOMIAL_CLASS,2> {
    typedef Var2 type;
};

template <class Coeff, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
struct variable<POLYNOMIAL_CLASS,3> {
    typedef Var3 type;
};

//------------------------------------------------------------------------ 
// Specializations for integer
//------------------------------------------------------------------------ 

// TODO move someplace else


template <std::size_t NumBits>
class integer;

template <std::size_t NumBits, int Order, class Var0, class Var1, class Var2, class Var3>
struct polynomial_multiply_result_type<polynomial<integer<NumBits>,max_order_each<Order>,Var0,Var1,Var2,Var3> > {
    typedef polynomial<integer<2*NumBits>,max_order_each<2*Order>,Var0,Var1,Var2,Var3> type;
};

template <std::size_t NumBits, int Order, class Var0, class Var1, class Var2, class Var3>
struct polynomial_multiply_result_type<polynomial<integer<NumBits>,max_order_combined<Order>,Var0,Var1,Var2,Var3> > {
    typedef polynomial<integer<2*NumBits>,max_order_combined<2*Order>,Var0,Var1,Var2,Var3> type;
};

template <std::size_t NumBits, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
struct polynomial_multiply_keep_order_result_type<polynomial<integer<NumBits>,MaxOrder,Var0,Var1,Var2,Var3> > {
    typedef polynomial<integer<2*NumBits>,MaxOrder,Var0,Var1,Var2,Var3> type;
};


namespace detail {
    template <std::size_t NumBits, class MaxOrder, class Var0, class Var1, class Var2, class Var3>
    struct equal_helper<polynomial<integer<NumBits>,MaxOrder,Var0,Var1,Var2,Var3> > {
        typedef polynomial<integer<NumBits>,MaxOrder,Var0,Var1,Var2,Var3> polynomial_type;
            bool operator()(polynomial_type const& p, polynomial_type const& p2) {
            // TODO check if this is ok
            int n = memcmp((void*)p.begin(),(void*)p2.begin(),((char*)p.end()-(char*)p.begin())*sizeof(char));
            return (0 == n);
        }
    };
}

/* \endcond I do not need this part in the doc*/


} // end namespace vli

#undef POLYNOMIAL_CLASS

#endif //VLI_POLYNOMIAL_HPP
