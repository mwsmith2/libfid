/*===========================================================================*\

author: Matthias W. Smith
email:  mwsmith2@uw.edu
file:   integration_step_test.cxx

about: This is a new test program for my Fid libraries 

\*===========================================================================*/

//--- std includes ----------------------------------------------------------//
#include <iostream>
#include <fstream>
#include <vector>
using std::cout;
using std::endl;

//--- other includes --------------------------------------------------------//
#include "TFile.h"
#include "TTree.h"
#include "TString.h"

//--- project includes ------------------------------------------------------//
#include "fid.h"

using namespace fid;

int main(int argc, char **argv)
{
  // initialize the configurable parameters
  if (argc > 1) load_params(argv[1]);

  // some necessary parameters
  double dt;
  std::vector<double> wf(0.0, sim::num_samples);
  std::vector<double> tm = time_vector();

  // Set up the ROOT tree to hold the results
  TFile pf("dt_test_fids.root", "recreate");
  TTree pt("t", "Fid Tree");
  cout.precision(12);

  pt.Branch("dt", &dt, "time_step/D");
  pt.Branch("fid", &wf[0], TString::Format("fid[%d]/D", sim::num_samples));
  pt.Branch("tm", &tm[0], TString::Format("time[%d]/D", sim::num_samples));

  std::vector<double> stepsizes;
  for (int i = 4; i <= 6; ++i) {
    stepsizes.push_back(10.0 * pow(10, -i));
    stepsizes.push_back(5.0 * pow(10, -i));
    stepsizes.push_back(2.0 * pow(10, -i));
  }

  // begin fid sims
  for (auto &step : stepsizes) {

    dt = step;
    sim::integration_step = dt;
    cout << "sim::integration_step = " << sim::integration_step << endl;
    cout << "sim::dt_int @ " << &sim::integration_step << endl;
    // Make FidFactory
    FidFactory ff;
    ff.PrintDiagnosticInfo();
    ff.SimulateFid(wf, tm);

    pt.Fill();

  } // grad

  pf.Write();
  pf.Close();

  return 0;
}
