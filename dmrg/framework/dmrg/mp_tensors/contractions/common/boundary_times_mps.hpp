/*****************************************************************************
 *
 * ALPS MPS DMRG Project
 *
 * Copyright (C) 2014 Institute for Theoretical Physics, ETH Zurich
 *                    Laboratory for Physical Chemistry, ETH Zurich
 *               2014-2014 by Sebastian Keller <sebkelle@phys.ethz.ch>
 * 
 * This software is part of the ALPS Applications, published under the ALPS
 * Application License; you can use, redistribute it and/or modify it under
 * the terms of the license, either version 1 or (at your option) any later
 * version.
 * 
 * You should have received a copy of the ALPS Application License along with
 * the ALPS Applications; see the file LICENSE.txt. If not, the license is also
 * available from http://alps.comp-phys.org/.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT 
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE 
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef ENGINE_COMMON_MPS_TIMES_BOUNDDARY_H
#define ENGINE_COMMON_MPS_TIMES_BOUNDDARY_H

#include "dmrg/mp_tensors/mpstensor.h"
#include "dmrg/mp_tensors/mpotensor.h"
#include "dmrg/mp_tensors/reshapes.h"
#include "dmrg/block_matrix/indexing.h"


namespace contraction {
    namespace common {

        template<class Matrix, class OtherMatrix, class SymmGroup, class Gemm>
        //static std::vector<block_matrix<OtherMatrix, SymmGroup> >
        static BoundaryMPSProduct<Matrix, OtherMatrix, SymmGroup, Gemm>
        boundary_times_mps(MPSTensor<Matrix, SymmGroup> const & mps,
                           Boundary<OtherMatrix, SymmGroup> const & left,
                           MPOTensor<Matrix, SymmGroup> const & mpo)
        {
            parallel::scheduler_permute scheduler(mpo.placement_l, parallel::groups_granularity);

            //std::vector<block_matrix<OtherMatrix, SymmGroup> > ret(left.aux_dim());
            BoundaryMPSProduct<Matrix, OtherMatrix, SymmGroup, Gemm> ret(left, mpo);
            int loop_max = left.aux_dim();
            mps.make_right_paired();
            omp_for(int b1, parallel::range(0,loop_max), {
                if (mpo.herm_info.left_skip(b1))
                {
                    parallel::guard group(scheduler(b1), parallel::groups_granularity);
                    typename Gemm::gemm_trim_left()(left[mpo.herm_info.left_conj(b1)], mps.data(), ret[b1]);
                }
                else {
                    parallel::guard group(scheduler(b1), parallel::groups_granularity);
                    typename Gemm::gemm_trim_left()(transpose(left[b1]), mps.data(), ret[b1]);
                }
            });
            return ret;
        }

        template<class Matrix, class OtherMatrix, class SymmGroup, class Gemm>
        static std::vector<block_matrix<OtherMatrix, SymmGroup> >
        mps_times_boundary(MPSTensor<Matrix, SymmGroup> const & mps,
                           Boundary<OtherMatrix, SymmGroup> const & right,
                           MPOTensor<Matrix, SymmGroup> const & mpo)
        {
            parallel::scheduler_permute scheduler(mpo.placement_r, parallel::groups_granularity);

            std::vector<block_matrix<OtherMatrix, SymmGroup> > ret(right.aux_dim());
            int loop_max = right.aux_dim();
            mps.make_left_paired();
            omp_for(int b2, parallel::range(0,loop_max), {
                if (mpo.herm_info.right_skip(b2))
                {
                    parallel::guard group(scheduler(b2), parallel::groups_granularity);
                    typename Gemm::gemm_trim_right()(mps.data(), transpose(right[mpo.herm_info.right_conj(b2)]), ret[b2]);
                }
                else {
                    parallel::guard group(scheduler(b2), parallel::groups_granularity);
                    typename Gemm::gemm_trim_right()(mps.data(), right[b2], ret[b2]);
                }
            });
            return ret;
        }

    } // namespace common
} // namespace contraction

#endif
