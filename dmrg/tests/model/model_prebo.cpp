/*****************************************************************************
 *
 * ALPS MPS DMRG Project
 *
 * Copyright (C) 2021 Institute for Theoretical Physics, ETH Zurich
 *               2021 by Robin Feldmann <robin.feldmann@phys.chem.ethz.ch>
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

#define BOOST_TEST_MODULE model_prebo

#include <boost/test/included/unit_test.hpp>
#include <boost/mpl/assert.hpp>
#include "utils/fpcomparison.h"
#include "utils/io.hpp"
#include "dmrg/models/coded/lattice.hpp"
#include "dmrg/models/coded/models_nu1.hpp"
#include "dmrg/sim/matrix_types.h"
#include <iostream>

#include "maquis_dmrg.h"

/** 
 * @brief Fixture class that contains all data that are globally used by the tests 
 * 
 * So far, our reference data are obtained for H2, at a bond distance of d=3 Angstrom,
 * for the singlet state, 6-31G* basis set.
 * The integrals are generated by Blueberry.
 */
struct PreBOModelUnitTestFixture {
  // Types 
  using IntegralMapType = maquis::integral_map<double, chem::Hamiltonian::PreBO>;
  using ModelType = PreBO<tmatrix<double>, 2>;

  /** @brief Class constructor */
  PreBOModelUnitTestFixture() {
     // Generates the integral file
     integralsForH2 = IntegralMapType{{{-1,-1,-1,-1,-1,-1,-1,-1},0.1763923530387111}};
     // Sets the parameters 
     parametersForH2.set("integrals_binary", maquis::serialize(integralsForH2));
     parametersForH2.set("L", 4);
     parametersForH2.set("LATTICE", "preBO lattice");
     parametersForH2.set("MODEL", "PreBO");
     parametersForH2.set("max_bond_dimension", 1000);
     parametersForH2.set("PreBO_NumParticles", 2);
     parametersForH2.set("PreBO_NumParticleTypes", 1);
     parametersForH2.set("PreBO_ParticleTypeVector", "2");
     parametersForH2.set("PreBO_FermionOrBosonVector", "1");
     parametersForH2.set("PreBO_OrbitalVector", "4");
     parametersForH2.set("PreBO_InitialStateVector", "1 1" );
     parametersForH2.set("symmetry", "nu1");
  }
  
  /** @brief Default destructor */
  ~PreBOModelUnitTestFixture() {
      BOOST_TEST_MESSAGE( "PreBO model test completed" );
  }

  static DmrgParameters parametersForH2;
  static IntegralMapType integralsForH2;
};

// Definition of static variables
typename PreBOModelUnitTestFixture::IntegralMapType PreBOModelUnitTestFixture::integralsForH2;
DmrgParameters PreBOModelUnitTestFixture::parametersForH2;

// Defines the fixture
BOOST_TEST_GLOBAL_FIXTURE( PreBOModelUnitTestFixture );

/** Checks consistency for the physical dimensions */
BOOST_AUTO_TEST_CASE( PreBO_Test_Phys )
{
    auto preBOModel = PreBO<tmatrix<double>, 2>(lattice_factory(PreBOModelUnitTestFixture::parametersForH2),
                                                                PreBOModelUnitTestFixture::parametersForH2, false);
    preBOModel.create_terms();
    const auto& physicalDimensions0 = preBOModel.phys_dim(0);
    BOOST_CHECK_EQUAL(physicalDimensions0.sum_of_sizes(), 4);
}

/** Checks consistency for the total QN vector */
BOOST_AUTO_TEST_CASE( PreBO_Test_QN )
{
    auto preBOModel = PreBO<tmatrix<double>, 2>(lattice_factory(PreBOModelUnitTestFixture::parametersForH2),
                                                                PreBOModelUnitTestFixture::parametersForH2, false);
    auto qn = preBOModel.total_quantum_numbers(PreBOModelUnitTestFixture::parametersForH2);
    BOOST_CHECK_EQUAL(qn[0], 1);
    BOOST_CHECK_EQUAL(qn[1], 1);
}

/** Simple check on tags */
BOOST_AUTO_TEST_CASE( PreBO_Test_Tags )
{
    auto preBOModel = PreBO<tmatrix<double>, 2>(lattice_factory(PreBOModelUnitTestFixture::parametersForH2),
                                                                PreBOModelUnitTestFixture::parametersForH2, false);
    auto identityTag = preBOModel.identity_matrix_tag(0);
    auto fillingTag = preBOModel.filling_matrix_tag(0);
    BOOST_CHECK(identityTag != fillingTag);
}

/** Checks that the number of terms that are created are consistent with the input data */
BOOST_AUTO_TEST_CASE( PreBO_Test_Terms )
{
    auto preBOModel = PreBO<tmatrix<double>, 2>(lattice_factory(PreBOModelUnitTestFixture::parametersForH2),
                                                                PreBOModelUnitTestFixture::parametersForH2, false);
    BOOST_CHECK_EQUAL(preBOModel.hamiltonian_terms().size(), 0);
    preBOModel.create_terms();
    BOOST_CHECK_EQUAL(preBOModel.hamiltonian_terms().size(), 1);
}