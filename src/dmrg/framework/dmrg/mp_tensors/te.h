/*****************************************************************************
 *
 * MAQUIS DMRG Project
 *
 * Copyright (C) 2011-2011 by Bela Bauer <bauerb@phys.ethz.ch>
 *
 *****************************************************************************/

#ifndef TIME_EVOLVE_H
#define TIME_EVOLVE_H

#include <boost/random.hpp>
#include <sys/time.h>

#include "ietl_lanczos_solver.h"
#include "ietl_jacobi_davidson.h"
#ifdef HAVE_ARPACK
#include "arpackpp_solver.h"
#endif

#include "dmrg/utils/BaseParameters.h"

template<class Matrix, class SymmGroup>
struct SiteProblem
{
    SiteProblem(MPSTensor<Matrix, SymmGroup> const & ket_tensor_,
                Boundary<typename storage::constrained<Matrix>::type, SymmGroup> const & left_,
                Boundary<typename storage::constrained<Matrix>::type, SymmGroup> const & right_,
                MPOTensor<Matrix, SymmGroup> const & mpo_)
    : ket_tensor(ket_tensor_)
    , left(left_)
    , right(right_)
    , mpo(mpo_) { }
    
    MPSTensor<Matrix, SymmGroup> const & ket_tensor;
    Boundary<typename storage::constrained<Matrix>::type, SymmGroup> const & left;
    Boundary<typename storage::constrained<Matrix>::type, SymmGroup> const & right;
    MPOTensor<Matrix, SymmGroup> const & mpo;
};

#define BEGIN_TIMING(name) \
gettimeofday(&now, NULL);
#define END_TIMING(name) \
gettimeofday(&then, NULL); \
maquis::cout << "Time elapsed in " << name << ": " << then.tv_sec-now.tv_sec + 1e-6 * (then.tv_usec-now.tv_usec) << std::endl;

template<class Matrix, class SymmGroup, class Storage>
class time_evolve
{
public:
    time_evolve(MPS<Matrix, SymmGroup> const & mps_,
                MPO<Matrix, SymmGroup> const & mpo_,
                BaseParameters & parms_)
    : mps(mps_)
    , mpsp(mps_)
    , mpo(mpo_)
    , parms(parms_)
    {
        mps.canonize(0);
        mpsp = mps;
    }
    
    int sweep(int sweep)
    {
        timeval sweep_now, sweep_then;
        gettimeofday(&sweep_now, NULL);
        
        
        std::size_t L = mps.length();
        
        init_left_right(mpo);
        
        for (int _site = 0; _site < 2*L; ++_site)
        {
            int site, lr;
            if (_site < L) {
                site = _site;
                lr = 1;
            } else {
                site = 2*L-_site-1;
                lr = -1;
            }
            
            SiteProblem<Matrix, SymmGroup> sp(mps[site], left_[site], right_[site+1], mpo[site]);
            
            ietl::mult(sp, mps[site], mpsp[site]);
            
            if (lr == +1) {
                if (site < L-1) {
                    block_matrix<Matrix, SymmGroup> t;
                    t = mpsp[site].normalize_left(DefaultSolver());
                    mpsp[site+1].multiply_from_left(t);
                }
                
                left_[site+1] = contraction::overlap_mpo_left_step(mpsp[site], mps[site],
                                                                   left_[site], mpo[site]);
            } else if (lr == -1) {
                if (site > 0) {
                    block_matrix<Matrix, SymmGroup> t;
                    t = mpsp[site].normalize_right(DefaultSolver());
                    mpsp[site-1].multiply_from_right(t);
                }   
                
                right_[site] = contraction::overlap_mpo_right_step(mpsp[site], mps[site],
                                                                   right_[site+1], mpo[site]);
            }
            
            
        }
        
        return -1;
    }
    
    void finalize()
    {
        mpsp[0].normalize_right(DefaultSolver());
    }
    
    MPS<Matrix, SymmGroup> get_original_mps() const { return mps; }
    MPS<Matrix, SymmGroup> get_current_mps() const { return mpsp; }
    
private:
    void init_left_right(MPO<Matrix, SymmGroup> const & mpo)
    {
        std::size_t L = mps.length();
        
        left_.resize(mpo.length()+1);
        right_.resize(mpo.length()+1);
        
        Storage::drop(left_[0]);
        left_[0] = mps.left_boundary();
        Storage::evict(left_[0]);
        
        for (int i = 0; i < L; ++i) {
            Storage::drop(left_[i+1]);
            left_[i+1] = contraction::overlap_mpo_left_step(mpsp[i], mps[i], left_[i], mpo[i]);
            Storage::evict(left_[i+1]);
        }
        
        Storage::drop(right_[L]);
        right_[L] = mps.right_boundary();
        Storage::evict(right_[L]);
        
        for(int i = L-1; i >= 0; --i) {
            Storage::drop(right_[i]);
            right_[i] = contraction::overlap_mpo_right_step(mpsp[i], mps[i], right_[i+1], mpo[i]);
            Storage::evict(right_[i]);
        }
    }
    
    MPS<Matrix, SymmGroup> mps, mpsp;
    MPO<Matrix, SymmGroup> mpo;
    
    BaseParameters & parms;
    std::vector<Boundary<typename storage::constrained<Matrix>::type, SymmGroup> > left_, right_;
};

#endif

