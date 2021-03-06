#ifndef LIBFID_FID_FID_H_
#define LIBFID_FID_FID_H_

/*============================================================================*\

author: Matthias W. Smith
email: mwmsith2@uw.edu

notes:

  Defines several frequency extraction and analysis methods for FIDs 
  as a class to encapsulate all the ideas.

\*===========================================================================*/

//--- std includes ----------------------------------------------------------//
#include <string>
#include <vector>
#include <numeric>
#include <random>
#include <complex>
#include <cmath>
#include <chrono>

//--- other includes --------------------------------------------------------//
#include <boost/algorithm/string.hpp>
#include <armadillo>
#include "TGraph.h"
#include "TCanvas.h"
#include "TF1.h"

//--- project includes ------------------------------------------------------//
#include "fid/base_fid.h"
#include "fid/params.h"
#include "fid/math.h"

namespace fid {

class Fid : public BaseFid {

 public:
  
  // ctors
  Fid(const std::string& fid_file);
  Fid(const char* fid_file);
  Fid(const std::vector<double>& wf, const std::vector<double>& tm);
  Fid(const std::vector<double>& wf);

  // Simplified frequency extraction
  using BaseFid::GetFreq; // Prevent base method from being hidden.
  double GetFreq(const std::string& method_name);
  double GetFreq(const char* method_name);
  double GetFreq(const Method m);
  
  // specific frequency extraction methods
  double CalcFreq() override;
  
  // utility functions
  void WriteFreqCsv(std::ofstream& out);
  void WriteMethodCsv(std::ofstream& out);
  void WritePhaseFreqCsv(std::ofstream& out);
  void CopyStruct(fid_t& f);
  void CopyStruct(fid_freq_t& f);
  void CopyStruct(fid_t& f, fid_freq_t& freq);

  // accessors
  const std::vector<double>& res() const { return res_; };
  const std::vector<double>& psd() const { return psd_; };
  const std::vector<double>& phi() const { return phi_; };
  const std::vector<double>& env() const { return env_; };
  const std::vector<double>& fftfreq() const { return fftfreq_; };

  // More specific use functions.
  double CalcZeroCountFreq();
  double CalcCentroidFreq();
  double CalcAnalyticalFreq();
  double CalcLorentzianFreq();
  double CalcSoftLorentzianFreq();
  double CalcExponentialFreq();
  double CalcPhaseFreq(int poln=1);
  double CalcPhaseDerivFreq(int poln=1);
  double CalcSinusoidFreq();
  
 private:
  
  // bigger data arrays
  std::vector<double> psd_;
  std::vector<double> env_;
  std::vector<double> phi_;
  std::vector<double> fftfreq_;

  // Private Member Functions  
  void InitHook() override;

  // Utility functions
  void CalcPowerEnvAndPhase();
  void CalcFftFreq();
  void GuessFitParams();
  void FreqFit(TF1& func);
  Method ParseMethod(const std::string& m);
  
}; // Fid
 
} // fid

#endif
