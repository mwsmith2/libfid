#include "fid_math.h"

namespace fid 
{

cvec dsp::fft(const vec &wf)
{
  // Grab some useful constants.
  int N = wf.size();  
  int n = N / 2 + 1;  // size of rfft
  double Nroot = std::sqrt(N);

  // Instantiate the result vector.
  cvec fft_vec(n, 0.0);
  auto wf_vec = wf; // copy waveform since fftw destroys it

  // Plan and execute the fft.
  fftw_plan wf_to_fft;
  fftw_complex *fft_ptr = reinterpret_cast<fftw_complex *>(&fft_vec[0]);
  wf_to_fft = fftw_plan_dft_r2c_1d(N, &wf_vec[0], fft_ptr, FFTW_ESTIMATE);
  fftw_execute(wf_to_fft);
  fftw_destroy_plan(wf_to_fft);

  for (auto it = fft_vec.begin(); it != fft_vec.end(); ++it) {
    *it /= Nroot;
  }

  return fft_vec;
}

vec dsp::ifft(const cvec& fft)
{
  // Grab some useful constants.
  int n = fft.size();
  int N = 2 * (n - 1);
  double Nroot = std::sqrt(N);

  // Instantiate the result vector.
  vec ifft_vec(N, 0.0);
  fftw_complex *fft_ptr = new fftw_complex[n];
  memcpy(fft_ptr, &fft[0], sizeof(fftw_complex) * n);

  // Plan and execute the fft.
  fftw_plan fft_to_wf;
  fft_to_wf = fftw_plan_dft_c2r_1d(N, fft_ptr, &ifft_vec[0], FFTW_ESTIMATE);
  fftw_execute(fft_to_wf);
  fftw_destroy_plan(fft_to_wf);

  // fftw is unnormalized, so we need to fix that.
  for (auto it = ifft_vec.begin(); it != ifft_vec.end(); ++it) {
  	*it /= Nroot;
  }

  return ifft_vec;
}

vec dsp::hilbert(const vec& wf)
{
	// Return the call to the fft version.
	auto fft_vec = dsp::fft(wf);
	return dsp::hilbert(fft_vec);
}

vec dsp::hilbert(cvec fft_vec)
{
	// Multiply in the -i.
	for (auto it = fft_vec.begin(); it != fft_vec.end(); ++it) {
		*it = std::complex<double>((*it).imag(), -(*it).real());
	}

	// Reverse the fft.
	return ifft(fft_vec);
}

vec dsp::psd(const vec& wf)
{
	return dsp::psd(dsp::fft(wf));
}

vec dsp::psd(const cvec& fft_vec)
{
	// Instatiate the power vector and fill it with the magnitude of fft_vec.
	vec power(fft_vec.size(), 0.0);

	for (uint i = 0; i < fft_vec.size(); ++i) {
		power[i] = std::norm(fft_vec[i]);
	}

	return power;
}

// Helper function to get frequencies for FFT
vec dsp::fftfreq(const vec& tm) 
{
	int N = tm.size();
	double dt = (tm[N-1] - tm[0]) / (N - 1); // sampling rate

	return dsp::fftfreq(N, dt);
}

vec dsp::fftfreq(const int N, const double dt)
{
	// Instantiate return vector.
	vec freq;

	// Handle both even and odd cases properly.
	if (N % 2 == 0) {

		freq.resize(N/2 + 1);
		
		for (int i = 0; i < freq.size(); ++i) {
			freq[i] = i / (dt * N);
		}

	} else {

		freq.resize((N + 1) / 2);

		for (int i = 0; i < freq.size(); ++i){
			freq[i] = i / (dt * N);
		}
	}

	return freq;
}

// Calculates the phase by assuming the real signal is harmonic.
vec dsp::phase(const vec& wf)
{
	return dsp::phase(wf, dsp::hilbert(wf));
}

vec dsp::phase(const vec& wf_re, const vec& wf_im)
{
	vec phase(wf_re.size(), 0.0);

	// Calculate the modulo-ed phase
  	std::transform(wf_re.begin(), wf_re.end(), wf_im.begin(), phase.begin(),
    			   [](double re, double im) { return std::atan2(im, re); });

	// Now unwrap the phase
	double thresh = params::max_phase_jump;
	int k = 0; // to track the winding number
  	for (auto it = phase.begin() + 1; it != phase.end(); ++it) {

    	// Add current total
    	*it += k * kTau;

    	// Check for large jumps, both positive and negative.
    	if (*(it - 1) - *it > thresh) {
	      	k++;
    	  	*it += kTau;

	    } else if (*it - *(it - 1) > thresh) {
	        k--;
	        *it -= kTau;
	    }
    }

    return phase;
}

vec dsp::envelope(const vec& wf)
{
	return dsp::envelope(wf, dsp::hilbert(wf));
}

vec dsp::envelope(const vec& wf_re, const vec& wf_im)
{
 	// Set the envelope function
	vec env(wf_re.size(), 0.0);

	std::transform(wf_re.begin(), wf_re.end(), wf_im.begin(), env.begin(),
    			   [](double r, double i) { return std::sqrt(r*r + i*i); });

	return env;
}


arma::cx_mat dsp::wvd_cx(const vec& wf, bool upsample)
{
  int M, N;
  if (upsample) {

    M = 2 * wf.size();
    N = wf.size();

  } else {

    M = wf.size();
    N = wf.size();
  }

  // Instiate the return matrix
  arma::cx_mat res(M, N, arma::fill::zeros);

  // Artificially double the sampling rate by repeating each sample.
  vec wf_re(M, 0.0);

  auto it1 = wf_re.begin();
  for (auto it2 = wf.begin(); it2 != wf.end(); ++it2) {
    *(it1++) = *it2;
    if (upsample) {
      *(it1++) = *it2;
    }
  }

  // Make the signal harmonic
  arma::cx_vec v(M);
  arma::vec phase(M);

  auto wf_im = dsp::hilbert(wf_re);

  for (uint i = 0; i < M; ++i) {
    v[i] = arma::cx_double(wf_re[i], wf_im[i]);
    phase[i] = (1.0 * i) / M * M_PI;
  }

  // Now compute the Wigner-Ville Distribution
  for (int idx = 0; idx < N; ++idx) {
    res.col(idx) = arma::fft(dsp::rconvolve(v, idx));
    //    res.col(idx) = res.col(idx) % (arma::cos(phase * idx) + 1j * arma::sin(phase * idx));
  }

  return res;
}

arma::mat dsp::wvd(const vec& wf, bool upsample)
{
  int M, N;
  if (upsample) {

    M = 2 * wf.size();
    N = wf.size();

  } else {

    M = wf.size();
    N = wf.size();
  }

  // Instiate the return matrix
  arma::mat res(M, N, arma::fill::zeros);

  // Artificially double the sampling rate by repeating each sample.
  vec wf_re(M, 0.0);

  auto it1 = wf_re.begin();
  for (auto it2 = wf.begin(); it2 != wf.end(); ++it2) {
    *(it1++) = *it2;
    if (upsample) {
      *(it1++) = *it2;
    }
  }

  // Make the signal harmonic
  arma::cx_vec v(M);

  auto wf_im = dsp::hilbert(wf_re);

  for (int i = 0; i < M; ++i) {
    v[i] = arma::cx_double(wf_re[i], wf_im[i]);
  }

  // Now compute the Wigner-Ville Distribution
  for (int idx = 0; idx < N; ++idx) {
    res.col(idx) = arma::real(arma::fft(dsp::rconvolve(v, idx))) ;
  }

  return res;
}

vec dsp::savgol3(const vec& wf)
{
  vec res(0, wf.size());
  vec filter = {-2.0, 3.0, 6.0, 7.0, 6.0, 3.0, -2.0};
  filter = (1.0 / 21.0) * filter;

  if (dsp::convolve(wf, filter, res) == 0) {

    return res;

  } else {

    return wf;
  }
}

vec dsp::savgol5(const vec& wf)
{
  vec res(0, wf.size());
  vec filter = {15.0, -55.0, 30.0, 135.0, 179.0, 135.0, 30.0, -55.0, 15.0};
  filter = (1.0 / 429.0) * filter;

  if (dsp::convolve(wf, filter, res) == 0) {

    return res;

  } else {

    return wf;
  }
}

int dsp::convolve(const vec& wf, const vec& filter, vec& res)
{
  int k = filter.size();
  int N = wf.size();
  res.resize(N);

  // Check to make sure we can do something.
  if (N < k) {
    return -1;
  }

  // First take care of the beginning and end.
  for (int i = 0; i < k + 1; ++i) {
    res[i] = 0.0;
    res[N -1 - i] = 0.0;

    for (int j = i; j < i + k; ++j) {

      res[i] += wf[abs(j - k/2)] * filter[j - i];
      res[N - 1 - i] += wf[N - 1 - abs(k/2 - j)] * filter[j - i];
    }
  }

  // Now the rest of the elements.
  for (auto it = wf.begin(); it != wf.end() - k; ++it) {
    double val = std::inner_product(it, it + k, filter.begin(), 0.0);
    res[std::distance(wf.begin(), it + k/2)] = val;
  }

  return 0;
}

vec dsp::convolve(const vec& wf, const vec& filter)
{
  vec res(0.0, wf.size());

  if (dsp::convolve(wf, filter, res) == 0) {

    return res;

  } else {

    return wf;
  }
}
 
} // ::fid
