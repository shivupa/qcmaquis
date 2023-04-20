/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
 *            See LICENSE.txt for details.
 */

#ifndef QC_CHEM_DETAIL_H
#define QC_CHEM_DETAIL_H

#include "dmrg/models/chem/parse_integrals.h"
#include "dmrg/models/chem/2u1/term_maker.h"

namespace chem {
namespace detail {

    template <typename M, class S>
    class ChemHelper
    {
    public:
        typedef typename M::value_type value_type;
        using InputType = double;
        typedef ::term_descriptor<value_type> term_descriptor;
        typedef typename TagHandler<M, S>::tag_type tag_type;
        typedef Lattice::pos_t pos_t;

        ChemHelper(BaseParameters & parms, Lattice const & lat_
                   , std::vector<tag_type> const & ident_, std::vector<tag_type> const & fill_
                   , std::shared_ptr<TagHandler<M, S> > tag_handler_)
            : lat(lat_), ident(ident_), fill(fill_), tag_handler(tag_handler_)
        {
            boost::tie(idx_, matrix_elements) = parse_integrals<InputType, S>(parms, lat);

            for (std::size_t m=0; m < matrix_elements.size(); ++m) {
                IndexTuple pos;
                std::copy(idx_.row(m).first, idx_.row(m).second, pos.begin());
                coefficients[pos] = static_cast<value_type>(matrix_elements[m]);
            }
        }

        auto& getMatrixElements() { return matrix_elements; }

        int idx(int m, int pos) const {
            return idx_(m,pos);
        }

        void commit_terms(std::vector<term_descriptor> & tagterms) {
            for (typename std::map<IndexTuple, term_descriptor>::const_iterator it = two_terms.begin();
                    it != two_terms.end(); ++it)
                tagterms.push_back(it->second);

            for (typename std::map<SixTuple, term_descriptor>::const_iterator it = three_terms.begin();
                    it != three_terms.end(); ++it)
                tagterms.push_back(it->second);
        }

        void add_term(std::vector<term_descriptor> & tagterms,
                      value_type scale, int p1, int p2, std::vector<tag_type> const & op_1, std::vector<tag_type> const & op_2)
        {
            term_descriptor
            term = TermMaker<M, S>::two_term(false, ident, scale, p1, p2, op_1, op_2, tag_handler, lat);
            IndexTuple id(p1, p2, op_1[lat.get_prop<typename S::subcharge>("type", p1)],
                                  op_2[lat.get_prop<typename S::subcharge>("type", p2)]);
            if (two_terms.count(id) == 0) {
                two_terms[id] = term;
            }
            else
                two_terms[id].coeff += term.coeff;
        }

        // two positions with four operators - multiply first and second operator pairs
        void add_term(std::vector<term_descriptor> & tagterms,
                      value_type scale, int p1, int p2, std::vector<tag_type> const & op_1, std::vector<tag_type> const & op_2,
                                                        std::vector<tag_type> const & op_3, std::vector<tag_type> const & op_4)
        {
            std::pair<tag_type, value_type> ptag1, ptag2;
            ptag1 = tag_handler->get_product_tag(op_1[lat.get_prop<typename S::subcharge>("type", p1)],
                                                 op_2[lat.get_prop<typename S::subcharge>("type", p1)]);
            ptag2 = tag_handler->get_product_tag(op_3[lat.get_prop<typename S::subcharge>("type", p2)],
                                                 op_4[lat.get_prop<typename S::subcharge>("type", p2)]);

            term_descriptor term;
            term.is_fermionic = false;
            term.coeff = scale * ptag1.second * ptag2.second;
            term.push_back(std::make_pair(p1, ptag1.first));
            term.push_back(std::make_pair(p2, ptag2.first));

            IndexTuple id(p1, p2, ptag1.first, ptag2.first);

            if (two_terms.count(id) == 0) {
                two_terms[id] = term;
            }
            else
                two_terms[id].coeff += term.coeff;
        }

        void add_term(std::vector<term_descriptor> & tagterms,
                      value_type scale, int s, int p1, int p2,
                      std::vector<tag_type> const & op_i, std::vector<tag_type> const & op_k,
                      std::vector<tag_type> const & op_l, std::vector<tag_type> const & op_j)
        {
            term_descriptor
            term = TermMaker<M, S>::three_term(ident, fill, scale, s, p1, p2, op_i, op_k, op_l, op_j, tag_handler, lat);

            SixTuple id(term.position(0), term.position(1), term.position(2),
                        term.operator_tag(0), term.operator_tag(1), term.operator_tag(2));

            if (three_terms.count(id) == 0) {
                three_terms[id] = term;
            }
            else
                three_terms[id].coeff += term.coeff;
        }

        void add_term(std::vector<term_descriptor> & tagterms,
                      int i, int k, int l, int j,
                      std::vector<tag_type> const & op_i, std::vector<tag_type> const & op_k,
                      std::vector<tag_type> const & op_l, std::vector<tag_type> const & op_j)
        {
            // Collapse terms with identical operators and different scales into one term
            if (op_i[0] == op_k[0] && op_j[0] == op_l[0]) {

                // if i>j, we switch l,j to get the related term
                // if j<i, we have to switch i,k, otherwise we get a forbidden permutation
                IndexTuple self(i,j,k,l), twin(i,l,k,j);
                if (i<j) twin = IndexTuple(k,j,i,l);

                if (self > twin) {

                    term_descriptor
                    term = TermMaker<M, S>::four_term(ident, fill, coefficients[align<S>(i,j,k,l)], i,k,l,j,
                                                      op_i, op_k, op_l, op_j, tag_handler, lat);
                    term_descriptor
                    term_twin = TermMaker<M, S>::four_term(ident, fill, coefficients[align<S>(twin)], twin[0],twin[2],twin[3],twin[1],
                                                           op_i, op_k, op_l, op_j, tag_handler, lat);

                    //term.coeff += value_type(sign(twin)) * coefficients[align<S>(twin)];
                    term.coeff += term_twin.coeff;

                    tagterms.push_back(term);
                }
                //else: we already have the term
            }
            else {
                tagterms.push_back( TermMaker<M, S>::four_term(ident, fill, coefficients[align<S>(i,j,k,l)], i,k,l,j,
                                   op_i, op_k, op_l, op_j, tag_handler, lat) );
            }
        }

    private:

        std::vector<tag_type> const & ident;
        std::vector<tag_type> const & fill;
        std::shared_ptr<TagHandler<M, S> > tag_handler;
        Lattice const & lat;

        std::vector<InputType> matrix_elements;
        alps::numeric::matrix<Lattice::pos_t> idx_;
        std::vector<Lattice::pos_t> order;
        std::vector<Lattice::pos_t> inv_order;

        std::map<IndexTuple, value_type> coefficients;

        std::map<SixTuple, term_descriptor> three_terms;
        std::map<IndexTuple, term_descriptor> two_terms;
    };
}
}

#endif
