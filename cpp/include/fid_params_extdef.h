#include "fid_params.h"

namespace fid {

// general fid analysis parameters
namespace params {

  int fit_width = 20;  
  int zc_width = 100;   
  int edge_ignore = 10; 
  double start_thresh = 50.0; 
  double max_phase_jump = 0.7; 
  double low_pass_freq = 500.0; 
  double centroid_thresh = 0.01; 
  double hyst_thresh = 0.3;
  Method freq_method = PH;

} // ::params

// simulation parameters
namespace sim {

  int seed = 0;   
  double dt_integration = 2.0e-5;
  double snr = 90000.0;   
  double amplitude = 2000.0;
  double baseline = 21000.0;

  int num_samples = 10000;
  double start_time = -1.0; 
  double delta_time = 0.001; 

  double freq_ref = 950.0;    
  double freq_larmor = 997.0; 
  double mixdown_phi = 950.0; 
  vec spin_0 = {0.0, 0.0, 1.0};         

  double gamma_1 = 0.05;  
  double gamma_2 = 0.05;  
  double gamma_g = 1.0;  

  double omega_r = 50.0;  
  double t_pulse = 0.005;  


} // ::sim

namespace grad {

  string root_file = "~/.fid/sim_fids.root";
  string fid_branch = "fid";
  double min = -2000;
  double max = 2000;
  int poln_order = 2;
  vec poln_coefs = {0.0, 0.0, 1.0, 0.0};
}

} // ::fid

