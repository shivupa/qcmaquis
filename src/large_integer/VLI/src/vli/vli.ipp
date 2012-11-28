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

#ifndef VLI_VLI_IPP
#define VLI_VLI_IPP

namespace vli {
namespace detail {

#if defined __GNU_MP_VERSION
    template <typename GMPClass>
    struct gmp_convert_helper {
        template <std::size_t NumBits>
        static GMPClass apply(vli<NumBits> a) {
            // vli::value_type = boost::uint64_t is an unsigned long long on some machines.
            // GMP doesn't work with unsigned long longs, but only with unsigned long int.
            // Hence we will cast to unsigned long int manually,
            // given that sizeof(unsigned long long) == sizeof(unsigned long int).
            BOOST_STATIC_ASSERT(sizeof(typename vli<NumBits>::value_type) == sizeof(unsigned long int));
            bool const neg = a.is_negative();
            if(neg)
                negate_inplace(a);
            GMPClass result(0);
            GMPClass factor(1);

            GMPClass const segment_factor( mpz_class(~static_cast<unsigned long int>(0)) + 1 );

            for(typename vli<NumBits>::size_type i=0; i< vli<NumBits>::numwords; ++i) {
                result += factor * static_cast<unsigned long int>(a[i]);
                factor *= segment_factor;
            }
            if(neg)
                result *= -1;
            return result;
        }
    };
#endif //__GNU_MP_VERSION
} // end namespace detail

template <std::size_t NumBits>
vli<NumBits>::vli(){
    memset((void*)&data_[0],0,numwords*sizeof(value_type));
}

template <std::size_t NumBits>
vli<NumBits>::vli(long int num) {
    // We rely on the implementation-defined >>= operator for negative long ints
    // to fill the most significant part with 1 (only for negative values)
    assert( -1l >> 1 == -1l );
    data_[0] = num;
    value_type a = num >> std::numeric_limits<long int>::digits;
    for(size_type i=1; i< numwords; ++i)
        data_[i] = a;
}

template <std::size_t NumBits>
vli<NumBits>::vli(vli<2*NumBits> const& vli_a, copy_lsb_tag){
    for(size_type i=0; i < numwords; ++i)
        data_[i] = vli_a[i+numwords];
}
    
template <std::size_t NumBits>
vli<NumBits>::vli(vli<2*NumBits> const& vli_a, copy_msb_tag){
    for(size_type i=0; i < numwords; ++i)
        data_[i] = vli_a[i];
}

template <std::size_t NumBits>
vli<NumBits>::vli(vli<NumBits/2> const& vli_a, copy_right_shift_tag){
    for(size_type i=0; i < numwords; ++i)
        data_[i] = 0;
    for(int  i=0 ; i < vli<NumBits/2>::numwords; i++)
        data_[i+(numwords>>2)] = vli_a[i];
}

template <std::size_t NumBits>
vli<NumBits>::vli(vli<NumBits/2> const& vli_a,  vli<NumBits/2> const& vli_b, copy_right_shift_tag){
    for(int i=0; i < vli<NumBits/2>::numwords; ++i)
        data_[i] = vli_a[i];
    for(int i=0; i < vli<NumBits/2>::numwords; i++)
        data_[i+(numwords>>1)] = vli_b[i];
}
    
#if defined __GNU_MP_VERSION
template <std::size_t NumBits>
vli<NumBits>::operator mpz_class() const{
    return detail::gmp_convert_helper<mpz_class>::apply(*this);
}

template <std::size_t NumBits>
vli<NumBits>::operator mpq_class() const{
    return detail::gmp_convert_helper<mpq_class>::apply(*this);
}
#endif //__GNU_MP_VERSION

template <std::size_t NumBits>
typename vli<NumBits>::value_type& vli<NumBits>::operator[](size_type i){
    assert( i < numwords );
    return *(data_+i);
}

template <std::size_t NumBits>
const typename vli<NumBits>::value_type& vli<NumBits>::operator[](size_type i) const{
    assert( i < numwords );
    return *(data_+i);
}

template <std::size_t NumBits>
vli<NumBits> vli<NumBits>::operator-() const{
    vli tmp(*this);
    tmp.negate();
    return tmp;
}

template <std::size_t NumBits>
bool vli<NumBits>::operator == (vli const& vli_a) const{
    int n = memcmp((void*)data_,(void*)vli_a.data_,numwords*sizeof(value_type));
    return (0 == n);
}

template <std::size_t NumBits>
bool vli<NumBits>::operator < (vli const& vli_a) const{
    vli tmp(*this);
    return ( (tmp -= vli_a).is_negative() );
}

template <std::size_t NumBits>
bool vli<NumBits>::operator < (long int i) const{
    vli tmp1(*this);
    vli tmp2(i);
    return ( (tmp1-=tmp2).is_negative() );
}

template <std::size_t NumBits>
bool vli<NumBits>::operator > (long int i) const{
    vli tmp(i);
    return ( (tmp-=*this).is_negative() );
}

template <std::size_t NumBits>
bool vli<NumBits>::is_zero() const{
    bool result = (data_[0] == 0);
    for(std::size_t i=1; i < numwords; ++i)
        result &= (data_[i] == 0);
    return result;
}
// c - negative number

template <std::size_t NumBits>
void vli<NumBits>::negate(){
    for(size_type i=0; i < numwords; ++i)
        data_[i] = (~data_[i]);
    (*this)+=1;
}

template <std::size_t NumBits>
bool vli<NumBits>::is_negative() const{
    return static_cast<bool>( data_[numwords-1] >> (std::numeric_limits<value_type>::digits-1) );
}
// c - basic operators
template <std::size_t NumBits>
vli<NumBits>& vli<NumBits>::operator >>= (long int const a){
    assert( a >= 0 );
    assert( a < 64 );
    for(size_type i = 0; i < numwords-1; ++i){
        data_[i] >>= a;
        data_[i] |= (data_[i+1] << (std::numeric_limits<value_type>::digits-a));
    }
    // We do an arithmentic shift, i.e. fill the left-most (most significant) part
    // with 0 for positive and 1 for negative values
    //
    // We rely on the implementation-defined >>= operator for negative long ints
    // to fill the most significant part with 1 (only for negative values)
    assert( -1l >> 1 == -1l );
    reinterpret_cast<boost::int64_t&>(data_[numwords-1]) >>= a;
    return *this;
}

template <std::size_t NumBits>
vli<NumBits>& vli<NumBits>::operator <<= (long int const a){
    assert( a >= 0 );
    assert(a < 64);
    for(size_type i = numwords-1; i > 0; --i){
        data_[i] <<= a;
        data_[i] |= (data_[i-1] >> (std::numeric_limits<value_type>::digits-a));
    }
    data_[0] <<= a;
    return *this;
}

template <std::size_t NumBits>
vli<NumBits>& vli<NumBits>::operator |= (vli const& vli_a){
    for(std::size_t i=0; i < numwords; ++i)
        (*this)[i] |= vli_a[i];
    return *this;
}

template <std::size_t NumBits>
vli<NumBits>& vli<NumBits>::operator ^= (vli const& vli_a){
    for(std::size_t i=0; i < numwords; ++i)
        (*this)[i] ^= vli_a[i];
    return *this;
}

template <std::size_t NumBits>
vli<NumBits>& vli<NumBits>::operator &= (vli const& vli_a){
    for(std::size_t i=0; i < numwords; ++i)
        (*this)[i] &= vli_a[i];
    return *this;
}
    
    
    
// vli_a %/= vli_b
template <std::size_t NumBits>
void quotient_helper(vli<NumBits> const& vli_b, vli<NumBits>& vli_quotient, vli<NumBits>& vli_rest){
    vli<NumBits> tmp(vli_b);
    vli<NumBits> tmp_quotient(1);

    if(vli_rest >= tmp){
        while(vli_rest >= tmp){
            tmp <<= 1;
            tmp_quotient <<=1;
        }

        tmp >>= 1;
        tmp_quotient >>= 1;
        vli_quotient |= tmp_quotient;

        vli_rest -= tmp;
    }else{
        return;
    }
    quotient_helper(vli_b, vli_quotient, vli_rest);
}

    
// Saint HPC forgives me
template <std::size_t NumBits>
vli<NumBits>& vli<NumBits>::operator %= (vli<NumBits> vli_a){
    bool const sign_this  = this->is_negative();
    bool const sign_vli_a = vli_a.is_negative();

    if(sign_this)
        this->negate();

    if(sign_vli_a)
        vli_a.negate();

    vli<NumBits> tmp(vli_a);

    while(*this >= tmp)
        tmp <<= 1;

    while(*this >= vli_a){
        tmp >>= 1;
        if(tmp <= *this)
            *this -= tmp;
    }

    if(sign_this^sign_vli_a) {
        this->negate();
        /* say gmp convention is correct
        if(sign_this)
            (*this) = vli_a - (*this);*/
    }

    return *this;
}

// Saint HPC forgives me
template <std::size_t NumBits>
vli<NumBits>& vli<NumBits>::operator /= (vli<NumBits> vli_a){
    bool const sign_this  = this->is_negative();
    bool const sign_vli_a = vli_a.is_negative();

    if(sign_this)
        this->negate();

    if(sign_vli_a)
        vli_a.negate();

    vli<NumBits> vli_rest(*this);
    *this ^= *this; //flush to 0
    quotient_helper(vli_a, *this, vli_rest);

    if(sign_this^sign_vli_a)
        this->negate();
    /* say gmp convention is correct
      if(sign_this){
      (*this).negate();
      (*this)-=1;
    }
    if(sign_vli_a)
      (*this).negate();
    }*/
    return *this;
}

template <std::size_t NumBits>
vli<NumBits>& vli<NumBits>::operator += (vli<NumBits> const& vli_a){
    plus_assign(*this,vli_a);
    return *this;
}

template <std::size_t NumBits>
vli<NumBits>& vli<NumBits>::operator += (long int const a){
    plus_assign(*this,a);
    return *this;
}

template <std::size_t NumBits>
vli<NumBits>& vli<NumBits>::operator -= (vli<NumBits> const& vli_a){
    minus_assign(*this,vli_a);
    return *this;
}

template <std::size_t NumBits>
vli<NumBits>& vli<NumBits>::operator -= (long int const a){
    minus_assign(*this,a);
    return *this;
}

template <std::size_t NumBits>
vli<NumBits>& vli<NumBits>::operator *= (long int const a){
    multiplies_assign(*this,a);
    return *this;
}

template <std::size_t NumBits>
vli<NumBits>& vli<NumBits>::operator *= (vli<NumBits> const& vli_a){
    multiplies_assign(*this,vli_a);
    return *this;
}

template <std::size_t NumBits>
void vli<NumBits>::print_raw(std::ostream& os) const{
    os << "(" ;
    for(size_type i = numwords-1; i > 0; --i)
        os << data_[i]<<" ";
    os << data_[0];
    os << ")";
}

template <std::size_t NumBits>
void vli<NumBits>::print(std::ostream& os) const{
    os<<get_str();
}

/**
 * Returns a string with a base10 represenation of the VLI
 */
template <std::size_t NumBits>
std::string vli<NumBits>::get_str() const {
    vli<NumBits> tmp;

    if((*this).is_negative()){
        const_cast<vli<NumBits> & >(*this).negate();

        for(size_type i=0; i<numwords; ++i)
            tmp[i] = (*this)[i];

        tmp.negate();
        const_cast<vli<NumBits> & >(*this).negate();
    }else{
        for(size_type i=0; i<numwords; ++i)
            tmp[i] = (*this)[i];
    }

    if(tmp.is_negative()){
        tmp.negate();
        size_type ten_exp = order_of_magnitude_base10(tmp);
        return std::string("-")+get_str_helper_inplace(tmp,ten_exp);
    }else{
        size_type ten_exp = order_of_magnitude_base10(tmp);
        return get_str_helper_inplace(tmp,ten_exp);
    }
}

/**
 * Returns the order of magnitude of 'value' in base10
 */
template <std::size_t NumBits>
typename vli<NumBits>::size_type vli<NumBits>::order_of_magnitude_base10(vli<NumBits> const& value) const {
    assert(!value.is_negative());

    vli<NumBits> value_cpy(value);
    vli<NumBits> decimal(1);
    size_type exp = 0;

    // Find correct order (10^exp) 
    while(!value_cpy.is_negative()){
        value_cpy=value; // reset
        vli<NumBits> previous_decimal(decimal);
        decimal *= 10;
        ++exp;
        if(decimal < previous_decimal) // Overflow! (we can handle it.)
        {
            break;
        }
        value_cpy-=decimal;
    }
    --exp;
    return exp;
}

/**
 * A helper function to generate the base10 representation for get_str().
 */
template <std::size_t NumBits>
std::string vli<NumBits>::get_str_helper_inplace(vli<NumBits>& value, size_type ten_exp) const {
    assert(!value.is_negative());

    // Create a number 10^(exponent-1) sin
    vli<NumBits> dec(1);
    for(size_type e=0; e < ten_exp; ++e)
        dec *= 10;

    // Find the right digit for 10^ten_exp
    vli<NumBits> value_cpy(value);
    int digit=0;
    while((!value_cpy.is_negative()) && digit<=11){
        value_cpy = value; // reset
        ++digit;
        if(digit*dec < (digit-1)*dec){ // Overflow (we can handle it.)
            break;
        }
        value_cpy-= digit*dec;
    }
    --digit; // we went to far

    assert(digit >=0);
    assert(digit < 10);

    value-= digit*dec;

    if(ten_exp <= 0)
        return boost::lexical_cast<std::string>(digit);
    else
        return boost::lexical_cast<std::string>(digit)+get_str_helper_inplace(value,ten_exp-1);
}

// free function algebra 
template <std::size_t NumBits>
void multiply_extend(vli<2*NumBits>& vli_res, vli<NumBits> const&  vli_a, vli<NumBits> const& vli_b) {
    multiplies<NumBits>(vli_res, vli_a, vli_b);
}

template <std::size_t NumBits>
void multiply_add(vli<2*NumBits>& vli_res, vli<NumBits> const&  vli_a, vli<NumBits> const& vli_b) {
    multiply_add_assign<NumBits>(vli_res, vli_a, vli_b);
}

template <std::size_t NumBits>
bool is_zero(vli<NumBits> const& v) {
    return v.is_zero();
}

template <std::size_t NumBits>
void negate_inplace(vli<NumBits>& v) {
    v.negate();
}

template <std::size_t NumBits>
const vli<NumBits+64> plus_extend (vli<NumBits> const &vli_a, vli<NumBits> const& vli_b){
      vli<NumBits+64> vli_res;
      plus_extend_assign<NumBits>(vli_res,vli_a,vli_b);
      return vli_res;
}

template <std::size_t NumBits>
const vli<NumBits> operator + (vli<NumBits> vli_a, long int b){
    vli_a += b;
    return vli_a;
}

template <std::size_t NumBits>
const vli<NumBits> operator + (long int b, vli<NumBits> const& vli_a){
    return vli_a+b;
}

template <std::size_t NumBits>
const vli<NumBits> operator - (vli<NumBits> vli_a, long int b){
    vli_a -= b;
    return vli_a;
}

template <std::size_t NumBits>
const vli<NumBits> operator * (vli<NumBits> vli_a, long int b){
    vli_a *= b;
    return vli_a;
}

template <std::size_t NumBits>
const vli<NumBits> operator * (long int b, vli<NumBits> const& a){
    return a*b;
}

//stream
template <std::size_t NumBits>
std::ostream& operator<< (std::ostream& os,  vli<NumBits> const& vli){
    if(os.flags() & std::ios_base::hex)
        vli.print_raw(os);
    else
        vli.print(os);
    return os;
}

} // end namespace vli

#endif //VLI_VLI_IPP
