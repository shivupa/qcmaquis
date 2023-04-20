/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
 *            See LICENSE.txt for details.
 */

#define BOOST_TEST_MAIN

#include <boost/test/included/unit_test.hpp>
#include <boost/filesystem/operations.hpp>
#include "utils/fpcomparison.h"
#include "utils/io.hpp" // has to be first include because of impi
#include <iostream>
#include "maquis_dmrg.h"
#include "test_detail.h"
#include "Fixtures/LiHFixture.h"

// Test 2: LiH with d=1 Angstrom, singlet, cc-pVDZ basis set, CAS(2,4), integrals generated by MOLCAS
// Ground and excited state calculations
BOOST_FIXTURE_TEST_CASE( Test2, LiHFixture )
{
  // type for RDM measurements
  typedef maquis::DMRGInterface<double>::meas_with_results_type rdm_measurement;
  std::vector<std::string> symmetries;
  #ifdef HAVE_SU2U1PG
  symmetries.push_back("su2u1pg");
  #endif
  #ifdef HAVE_SU2U1
  symmetries.push_back("su2u1");
  #endif
  #ifdef HAVE_TwoU1PG
  symmetries.push_back("2u1pg");
  #endif
  #ifdef HAVE_TwoU1
  symmetries.push_back("2u1");
  #endif
  test_detail::TestTmpPath tmp_path;
  for (auto&& s: symmetries) {
    // su2 symmetry?
    bool su2 = (s[0] == 's');
    maquis::cout << "Running test for symmetry " << s << std::endl;
    parametersLiH.set("symmetry", s);
    // ground state calculation
    parametersLiH.set("n_ortho_states", 0); // needed only because before n_ortho_states is set to 1
    parametersLiH.set("ortho_states", "");
    // set checkpoint name for S0
    std::string checkpoint_name = "checkpoint_" + s;
    boost::filesystem::path checkpoint_path(tmp_path.path() / checkpoint_name);
    parametersLiH.set("chkpfile", checkpoint_path.c_str());
    // measure 2-RDM
    parametersLiH.set("MEASURE[2rdm]", 1);
    {
      maquis::DMRGInterface<double> interface(parametersLiH);
      interface.optimize();
      // check energy
      BOOST_CHECK_CLOSE(interface.energy(), -7.90435750473166 , 1e-7);
      // check 2-RDM
      auto&& meas_2rdm = interface.measurements().at("twoptdm");
      const rdm_measurement ref_2rdm = {
       { { 0, 0, 0, 0 }, { 0, 0, 0, 1 }, { 0, 0, 0, 2 }, { 0, 0, 0, 3 },
         { 0, 0, 1, 1 }, { 0, 0, 1, 2 }, { 0, 0, 1, 3 }, { 0, 0, 2, 2 },
         { 0, 0, 2, 3 }, { 0, 0, 3, 3 }, { 0, 1, 0, 0 }, { 0, 1, 0, 1 },
         { 0, 1, 0, 2 }, { 0, 1, 0, 3 }, { 0, 1, 1, 1 }, { 0, 1, 1, 2 },
         { 0, 1, 1, 3 }, { 0, 1, 2, 2 }, { 0, 1, 2, 3 }, { 0, 1, 3, 3 },
         { 0, 2, 0, 0 }, { 0, 2, 0, 1 }, { 0, 2, 0, 2 }, { 0, 2, 0, 3 },
         { 0, 2, 1, 1 }, { 0, 2, 1, 2 }, { 0, 2, 1, 3 }, { 0, 2, 2, 2 },
         { 0, 2, 2, 3 }, { 0, 2, 3, 3 }, { 0, 3, 0, 0 }, { 0, 3, 0, 1 },
         { 0, 3, 0, 2 }, { 0, 3, 0, 3 }, { 0, 3, 1, 1 }, { 0, 3, 1, 2 },
         { 0, 3, 1, 3 }, { 0, 3, 2, 2 }, { 0, 3, 2, 3 }, { 0, 3, 3, 3 },
         { 1, 0, 0, 0 }, { 1, 0, 0, 1 }, { 1, 0, 0, 2 }, { 1, 0, 0, 3 },
         { 1, 0, 1, 1 }, { 1, 0, 1, 2 }, { 1, 0, 1, 3 }, { 1, 0, 2, 2 },
         { 1, 0, 2, 3 }, { 1, 0, 3, 3 }, { 1, 1, 1, 1 }, { 1, 1, 1, 2 },
         { 1, 1, 1, 3 }, { 1, 1, 2, 2 }, { 1, 1, 2, 3 }, { 1, 1, 3, 3 },
         { 1, 2, 1, 1 }, { 1, 2, 1, 2 }, { 1, 2, 1, 3 }, { 1, 2, 2, 2 },
         { 1, 2, 2, 3 }, { 1, 2, 3, 3 }, { 1, 3, 1, 1 }, { 1, 3, 1, 2 },
         { 1, 3, 1, 3 }, { 1, 3, 2, 2 }, { 1, 3, 2, 3 }, { 1, 3, 3, 3 },
         { 2, 0, 0, 0 }, { 2, 0, 0, 1 }, { 2, 0, 0, 2 }, { 2, 0, 0, 3 },
         { 2, 0, 1, 1 }, { 2, 0, 1, 2 }, { 2, 0, 1, 3 }, { 2, 0, 2, 2 },
         { 2, 0, 2, 3 }, { 2, 0, 3, 3 }, { 2, 1, 1, 1 }, { 2, 1, 1, 2 },
         { 2, 1, 1, 3 }, { 2, 1, 2, 2 }, { 2, 1, 2, 3 }, { 2, 1, 3, 3 },
         { 2, 2, 2, 2 }, { 2, 2, 2, 3 }, { 2, 2, 3, 3 }, { 2, 3, 2, 2 },
         { 2, 3, 2, 3 }, { 2, 3, 3, 3 }, { 3, 0, 0, 0 }, { 3, 0, 0, 1 },
         { 3, 0, 0, 2 }, { 3, 0, 0, 3 }, { 3, 0, 1, 1 }, { 3, 0, 1, 2 },
         { 3, 0, 1, 3 }, { 3, 0, 2, 2 }, { 3, 0, 2, 3 }, { 3, 0, 3, 3 },
         { 3, 1, 1, 1 }, { 3, 1, 1, 2 }, { 3, 1, 1, 3 }, { 3, 1, 2, 2 },
         { 3, 1, 2, 3 }, { 3, 1, 3, 3 }, { 3, 2, 2, 2 }, { 3, 2, 2, 3 },
         { 3, 2, 3, 3 }, { 3, 3, 3, 3 } },
         std::vector<double> {
              1.826089938513, -3.105214935387E-01, -1.993392964110E-08, 1.730029449859E-01,
              -4.867633632766E-03, -5.108353598560E-08, 6.760359478424E-02, -1.359411899135E-01,
              -3.271186608089E-08, -1.929351875992E-01, -3.105214935387E-01, 5.280331264958E-02,
              3.389709057426E-09, -2.941866757525E-02, 8.277275033217E-04, 8.686612611429E-09,
              -1.149580246748E-02, 2.311641964347E-02, 5.562561458584E-09, 3.280808975832E-02,
              -1.993392964110E-08, 3.389709057426E-09, 2.176023987406E-16, -1.888531588899E-09,
              5.313597337558E-11, 5.576371627046E-16, -7.379731268944E-10, 1.483958734952E-09,
              3.570886751705E-16, 2.106115571738E-09, 1.730029449859E-01, -2.941866757525E-02,
              -1.888531588899E-09, 1.639022172050E-02, -4.611574357979E-04, -4.839631378178E-09,
              6.404734368578E-03, -1.287900760194E-02, -3.099107578780E-09, -1.827859348114E-02,
              -3.105214935387E-01, 5.280331264958E-02, 3.389709057426E-09, -2.941866757525E-02,
              8.277275033217E-04, 8.686612611429E-09, -1.149580246748E-02, 2.311641964347E-02,
              5.562561458584E-09, 3.280808975832E-02, 1.297518631647E-05, 1.361685055045E-10,
              -1.802044492593E-04, 3.623654531716E-04, 8.719689877683E-11, 5.142889122246E-04,
              1.361685055045E-10, 1.429024712177E-15, -1.891161324578E-09, 3.802855774195E-09,
              9.150906277159E-16, 5.397221347510E-09, -1.802044492593E-04, -1.891161324578E-09,
              2.502749690125E-03, -5.032672774533E-03, -1.211024546234E-09, -7.142645040093E-03,
              -1.993392964110E-08, 3.389709057426E-09, 2.176023987406E-16, -1.888531588899E-09,
              5.313597337558E-11, 5.576371627046E-16, -7.379731268944E-10, 1.483958734952E-09,
              3.570886751705E-16, 2.106115571738E-09, 1.361685055045E-10, 1.429024712177E-15,
              -1.891161324578E-09, 3.802855774195E-09, 9.150906277159E-16, 5.397221347510E-09,
              1.011998737048E-02, 2.435197689633E-09, 1.436284074802E-02, 2.435197689633E-09,
              5.859876668318E-16, 3.456166033190E-09, 1.730029449859E-01, -2.941866757525E-02,
              -1.888531588899E-09, 1.639022172050E-02, -4.611574357979E-04, -4.839631378178E-09,
              6.404734368578E-03, -1.287900760194E-02, -3.099107578780E-09, -1.827859348114E-02,
              -1.802044492593E-04, -1.891161324578E-09, 2.502749690125E-03, -5.032672774533E-03,
              -1.211024546234E-09, -7.142645040093E-03, 2.435197689633E-09, 5.859876668318E-16,
              3.456166033190E-09, 2.038453081028E-02
         }
      };
      test_detail::check_measurement_mat(meas_2rdm, ref_2rdm, false);
    }
    // excited state calculation
    parametersLiH.erase("MEASURE[2rdm]");
    parametersLiH.set("n_ortho_states", 1);
    parametersLiH.set("ortho_states", checkpoint_path.c_str());
    parametersLiH.erase("chkpfile");
    // measure 1-TDM
    std::string measure_1rdm = su2 ? "MEASURE[trans1rdm]" : "MEASURE[trans1rdm_aa]";
    parametersLiH.set(measure_1rdm,checkpoint_path.c_str());
    // measure 2-TDM
    std::string measure_2rdm = "MEASURE[trans2rdm]";
    parametersLiH.set(measure_2rdm,checkpoint_path.c_str());
    {
      maquis::DMRGInterface<double> interface(parametersLiH);
      interface.optimize();
      // Check excited state energy
      // Excited states are different for SU2U1 and 2U1: for SU2 it's S1, whereas for 2U1 it's most likely (spin-contaminated?) T1
      double ref_energy_excited_state = su2 ? -7.7734906999491207 : -7.78681053707699;
      BOOST_CHECK_CLOSE(interface.energy(), ref_energy_excited_state , 1e-7);
      // Check 1-TDM
      std::string measurement_name = su2 ? "transition_oneptdm" : "transition_oneptdm_aa";
      auto&& meas_trans1rdm = interface.measurements().at(measurement_name);
      // Reference values
      const rdm_measurement ref_trans1rdm = {
          { {0,0}, {0,1}, {0,2}, {0,3}, {1,0}, {1,1}, {1,2}, {1,3},
          {2,0}, {2,1}, {2,2}, {2,3}, {3,0}, {3,1}, {3,2}, {3,3} },
          su2 ?
          std::vector<double>{ 2.2327203416481e-01, 1.2768576875559e+00, -1.8566846188891e-08, 6.9241986688843e-02,
              -7.6798657154053e-02, -2.2825761188661e-01, 1.9651789449319e-09, -8.6858241793144e-03,
              -4.0887453732287e-08, -1.2504555601357e-08, 1.8910083540871e-03, 8.0969704630913e-09,
              8.5484107425002e-02, 1.4507647800345e-01, 2.4037522379741e-09, 3.0945693677105e-03 }
              :
          std::vector<double>{
              0.11910280427789303, 0.65256003404351703, 5.1574098495915676e-08, -0.085885746557882933,
              0.0042475242314700444, -0.11687030483875573, -7.760829552541157e-09, 0.011095074501902931,
              2.092430887319513e-08, -7.1289380697275129e-09, 1.5006735785872152e-10, -1.6184768329402167e-09,
              -0.031625862740455889, 0.074561720757826896, 2.9455469254912761e-09, -0.0022324995892045933
          }
      };
      test_detail::check_measurement_mat(meas_trans1rdm, ref_trans1rdm, true);
      // Check 2-TDM
      auto&& meas_trans2rdm = interface.measurements().at("transition_twoptdm");
      const rdm_measurement ref_trans2rdm = {
        { {0, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 0, 2}, {0, 0, 0, 3}, {0, 0, 1, 1}, {0, 0, 1, 2},
          {0, 0, 1, 3}, {0, 0, 2, 2}, {0, 0, 2, 3}, {0, 0, 3, 3}, {0, 1, 0, 0}, {0, 1, 0, 1},
          {0, 1, 0, 2}, {0, 1, 0, 3}, {0, 1, 1, 1}, {0, 1, 1, 2}, {0, 1, 1, 3}, {0, 1, 2, 2},
          {0, 1, 2, 3}, {0, 1, 3, 3}, {0, 2, 0, 0}, {0, 2, 0, 1}, {0, 2, 0, 2}, {0, 2, 0, 3},
          {0, 2, 1, 1}, {0, 2, 1, 2}, {0, 2, 1, 3}, {0, 2, 2, 2}, {0, 2, 2, 3}, {0, 2, 3, 3},
          {0, 3, 0, 0}, {0, 3, 0, 1}, {0, 3, 0, 2}, {0, 3, 0, 3}, {0, 3, 1, 1}, {0, 3, 1, 2},
          {0, 3, 1, 3}, {0, 3, 2, 2}, {0, 3, 2, 3}, {0, 3, 3, 3}, {1, 0, 0, 0}, {1, 0, 0, 1},
          {1, 0, 0, 2}, {1, 0, 0, 3}, {1, 0, 1, 1}, {1, 0, 1, 2}, {1, 0, 1, 3}, {1, 0, 2, 2},
          {1, 0, 2, 3}, {1, 0, 3, 3}, {1, 1, 0, 0}, {1, 1, 0, 1}, {1, 1, 0, 2}, {1, 1, 0, 3},
          {1, 1, 1, 1}, {1, 1, 1, 2}, {1, 1, 1, 3}, {1, 1, 2, 2}, {1, 1, 2, 3}, {1, 1, 3, 3},
          {1, 2, 0, 0}, {1, 2, 0, 1}, {1, 2, 0, 2}, {1, 2, 0, 3}, {1, 2, 1, 1}, {1, 2, 1, 2},
          {1, 2, 1, 3}, {1, 2, 2, 2}, {1, 2, 2, 3}, {1, 2, 3, 3}, {1, 3, 0, 0}, {1, 3, 0, 1},
          {1, 3, 0, 2}, {1, 3, 0, 3}, {1, 3, 1, 1}, {1, 3, 1, 2}, {1, 3, 1, 3}, {1, 3, 2, 2},
          {1, 3, 2, 3}, {1, 3, 3, 3}, {2, 0, 0, 0}, {2, 0, 0, 1}, {2, 0, 0, 2}, {2, 0, 0, 3},
          {2, 0, 1, 1}, {2, 0, 1, 2}, {2, 0, 1, 3}, {2, 0, 2, 2}, {2, 0, 2, 3}, {2, 0, 3, 3},
          {2, 1, 0, 0}, {2, 1, 0, 1}, {2, 1, 0, 2}, {2, 1, 0, 3}, {2, 1, 1, 1}, {2, 1, 1, 2},
          {2, 1, 1, 3}, {2, 1, 2, 2}, {2, 1, 2, 3}, {2, 1, 3, 3}, {2, 2, 0, 0}, {2, 2, 0, 1},
          {2, 2, 0, 2}, {2, 2, 0, 3}, {2, 2, 1, 1}, {2, 2, 1, 2}, {2, 2, 1, 3}, {2, 2, 2, 2},
          {2, 2, 2, 3}, {2, 2, 3, 3}, {2, 3, 0, 0}, {2, 3, 0, 1}, {2, 3, 0, 2}, {2, 3, 0, 3},
          {2, 3, 1, 1}, {2, 3, 1, 2}, {2, 3, 1, 3}, {2, 3, 2, 2}, {2, 3, 2, 3}, {2, 3, 3, 3},
          {3, 0, 0, 0}, {3, 0, 0, 1}, {3, 0, 0, 2}, {3, 0, 0, 3}, {3, 0, 1, 1}, {3, 0, 1, 2},
          {3, 0, 1, 3}, {3, 0, 2, 2}, {3, 0, 2, 3}, {3, 0, 3, 3}, {3, 1, 0, 0}, {3, 1, 0, 1},
          {3, 1, 0, 2}, {3, 1, 0, 3}, {3, 1, 1, 1}, {3, 1, 1, 2}, {3, 1, 1, 3}, {3, 1, 2, 2},
          {3, 1, 2, 3}, {3, 1, 3, 3}, {3, 2, 0, 0}, {3, 2, 0, 1}, {3, 2, 0, 2}, {3, 2, 0, 3},
          {3, 2, 1, 1}, {3, 2, 1, 2}, {3, 2, 1, 3}, {3, 2, 2, 2}, {3, 2, 2, 3}, {3, 2, 3, 3},
          {3, 3, 0, 0}, {3, 3, 0, 1}, {3, 3, 0, 2}, {3, 3, 0, 3}, {3, 3, 1, 1}, {3, 3, 1, 2},
          {3, 3, 1, 3}, {3, 3, 2, 2}, {3, 3, 2, 3}, {3, 3, 3, 3} },
        su2 ?
        std::vector<double>{
          4.39913965188E-01, 1.29601469934E+00, -1.17846422957E-08, 3.94951056362E-02, -6.08362089400E-03, 3.15012506950E-08,
          -2.13126554150E-01, -2.54018030241E-02, -1.79734317852E-08, -6.85531435086E-02, -7.48061410436E-02, -2.20383679687E-01,
          -1.41475319948E-08, 1.22783853633E-01, 1.03450274068E-03, -5.35669969362E-09, 3.62415752432E-02, 4.31950565372E-03,
          3.05633187296E-09, 1.16572705758E-02, -4.80218079363E-09, 2.00394550636E-09, 1.28643297032E-16, -1.11647174642E-09,
          6.64099113117E-11, 3.29666784803E-16, 2.32652819857E-09, 2.77290698316E-10, 1.96201247846E-16, 7.48338573338E-10,
          4.16772525344E-02, -6.71603239849E-03, -4.31135750936E-10, 3.74174866390E-03, -5.76359525696E-04, 2.98441441795E-09,
          1.46214655756E-03, -2.40655547048E-03, -7.07499986267E-10, -6.49469419052E-03, -7.48061410436E-02, -2.20383679687E-01,
          2.00394550636E-09, -6.71603239849E-03, 1.03450274068E-03, -3.62550672535E-08, 4.79797028180E-02, 4.31950565372E-03,
          3.05633187296E-09, 1.16572705758E-02, -1.17263666335E-03, -3.45466266805E-03, 3.14131960201E-11, -1.05278332940E-04,
          1.62165274821E-05, -8.39698769065E-11, 5.68111110596E-04, 6.77111614960E-05, 4.79100613880E-11, 1.82735570651E-04,
          -1.23062727620E-08, -3.62550672535E-08, -3.43873377601E-16, -1.10484681366E-09, 1.70184863465E-10, -8.81224544055E-16,
          1.16620640762E-09, 7.10596938033E-10, 5.02793663128E-16, 1.91772425853E-09, 1.62860354330E-02, 4.79797028180E-02,
          -4.36278720798E-10, -2.01915145279E-02, -2.25221459833E-04, 5.96206012026E-09, -7.89014872743E-03, -9.40399025378E-04,
          3.81786633320E-09, -2.53790289142E-03, -4.80218079363E-09, -1.41475319948E-08, 1.28643297032E-16, -4.31135750936E-10,
          6.64099113117E-11, -3.43873377601E-16, 2.32652819857E-09, 2.77290698316E-10, 2.11105506064E-16, 7.48338573338E-10,
          -1.23062727620E-08, -5.35669969362E-09, 3.29666784803E-16, -1.10484681366E-09, 1.70184863465E-10, -8.81224544055E-16,
          5.96206012026E-09, 7.10596938033E-10, -5.64301172895E-16, 1.91772425853E-09, -3.27488951261E-02, -9.64803412244E-02,
          8.77294301118E-10, -2.94016824840E-03, 4.52888242726E-04, -2.34507480323E-09, 1.58659640811E-02, 1.89100835409E-03,
          1.33801170040E-09, 5.10336085005E-03, -7.88044795212E-09, -2.32163040766E-08, 2.11105506064E-16, -1.70279489787E-09,
          1.08979622402E-10, -5.64301172895E-16, -6.65393622552E-10, 4.55038035757E-10, 3.21969077849E-16, 1.22803439337E-09,
          4.16772525344E-02, 1.22783853633E-01, -1.11647174642E-09, 3.74174866390E-03, -5.76359525696E-04, 2.98441441795E-09,
          -2.01915145279E-02, -2.40655547048E-03, -1.70279489787E-09, -6.49469419052E-03, 1.62860354330E-02, 3.62415752432E-02,
          -4.36278720798E-10, 1.46214655756E-03, -2.25221459833E-04, 1.16620640762E-09, -7.89014872743E-03, -9.40399025378E-04,
          -6.65393622552E-10, -2.53790289142E-03, -7.88044795212E-09, -2.32163040766E-08, 1.96201247846E-16, -7.07499986267E-10,
          1.08979622402E-10, 5.02793663128E-16, 3.81786633320E-09, 4.55038035757E-10, 3.21969077849E-16, 1.22803439337E-09,
          -4.64790269148E-02, -1.36930188309E-01, 1.24510415624E-09, -4.17284792740E-03, 6.42763816673E-04, -3.32825869322E-09,
          2.25178458301E-02, 2.68382270142E-03, 1.89897954101E-09, 7.24296943124E-03 }
         :
         std::vector<double>{ // for some reasons they're all close to zeros
          4.79366365297E-08, -3.76403007651E-08, -1.82675854849E-10, 4.94630043868E-09, 4.33998634131E-10, -6.22734559765E-10,
          5.72078495686E-09, -4.03170009622E-09, 4.65488633115E-10, -4.58925872380E-09, -8.15149116615E-09, 3.65715253359E-08,
          3.10657062154E-11, -3.88914335285E-09, -7.38002556780E-11, 1.05893568920E-10, 4.08351649023E-09, 6.85579341775E-10,
          -7.91554689580E-11, 7.80390629265E-10, -5.42752535505E-16, 9.29751672242E-10, 6.94715434915E-17, -9.39289112538E-11,
          -4.91385871293E-18, -1.46381652311E-17, 1.55816309019E-10, 4.56480806338E-17, -1.84257176868E-17, 5.19609215153E-17,
          4.54149556203E-09, -7.71002593714E-09, -1.73069294267E-11, 8.87261241908E-10, 4.11168370065E-11, -5.89974977429E-11,
          -1.52505612305E-10, -3.81961468721E-10, 4.41002440984E-11, -4.34784324391E-10, -8.15149116615E-09, -2.37702693084E-08,
          3.10613316774E-11, 2.20693419269E-09, -7.38002556780E-11, 1.05894976554E-10, -6.02912349559E-09, 6.85579341775E-10,
          -7.91546151644E-11, 7.80390629265E-10, -1.27780135950E-10, 1.00334171744E-10, 4.86941664188E-13, -1.31848830085E-11,
          -1.15686891042E-12, 1.65996432934E-12, -1.52493527788E-11, 1.07469197612E-11, -1.24080880781E-12, 1.22331508026E-11,
          -1.33936502527E-15, 1.47198266463E-12, 5.21065866639E-18, -1.48740167531E-13, -1.21260612686E-17, 1.73650902471E-17,
          2.46352474165E-13, 1.12646995955E-16, -1.30267145861E-17, 1.28225363137E-16, 1.77466011028E-09, 3.68873515066E-09,
          -6.76246633880E-12, -3.30318026404E-10, 1.60670443249E-11, -2.30543472739E-11, 1.06351386552E-09, -1.49257391744E-10,
          1.72327622810E-11, -1.69898745145E-10, -5.42752535505E-16, -9.29750819893E-10, -6.53349252148E-17, 9.39287992469E-11,
          -4.91385871293E-18, 2.87397287001E-17, -1.55816438563E-10, 4.56480806338E-17, 7.88492241368E-18, 5.19609215153E-17,
          -1.33936502527E-15, -1.46987930045E-12, 4.99738547928E-18, 1.48463765083E-13, -1.21260612686E-17, 1.74337170851E-17,
          -2.46672155304E-13, 1.12646995955E-16, -1.29850893279E-17, 1.28225363137E-16, -3.56858839395E-09, 2.80208937303E-09,
          1.35990962792E-11, -3.68221710935E-10, -3.23085347840E-11, 4.63587661415E-11, -4.25877331664E-10, 3.00135328902E-10,
          -3.46527719487E-11, 3.41642146888E-10, -8.17948285863E-16, -9.04492041586E-10, -6.24550710462E-17, 9.13770046183E-11,
          -7.40536809740E-18, 3.17255137882E-17, -1.51583406283E-10, 6.87933576811E-17, 4.85524379263E-18, 7.83070440119E-17,
          4.54149556203E-09, 5.77975334259E-10, -1.73063285799E-11, 4.99593552292E-11, 4.11168370065E-11, -5.89976910828E-11,
          1.23647484092E-09, -3.81961468721E-10, 4.41001268291E-11, -4.34784324391E-10, 1.77466011028E-09, -6.47569497331E-09,
          -6.76320321920E-12, 6.96551568594E-10, 1.60670443249E-11, -2.30541101613E-11, -6.39936009565E-10, -1.49257391744E-10,
          1.72329061005E-11, -1.69898745145E-10, -8.17948285863E-16, 9.04493326108E-10, 6.86891088577E-17, -9.13771734168E-11,
          -7.40536809740E-18, -1.04739324265E-17, 1.51583211054E-10, 6.87933576811E-17, -2.07406148620E-17, 7.83070440119E-17,
          -5.06473618722E-09, 3.97687877074E-09, 1.93005825932E-11, -5.22600429077E-10, -4.58540428911E-11, 6.57949011070E-11,
          -6.04428444795E-10, 4.25968504501E-10, -4.91811127261E-11, 4.84877254927E-10 }
      };
      test_detail::check_measurement_mat(meas_trans2rdm, ref_trans2rdm, true, 1.0E-5);
    }
    // clean up to avoid messing up with the upcoming measurements
    parametersLiH.erase(measure_1rdm);
    parametersLiH.erase(measure_2rdm);
    boost::filesystem::remove_all(checkpoint_path);
  }
}