/**
 * @file LeakyIntegrator.cpp
 * @brief Implementation of the LeakyIntegrator class.
 */

#include "LeakyIntegrator.h"

void LeakyIntegrator::setSampleRate(float sr) {
  sampleRate = sr;
  calculateCoeff();
}

void LeakyIntegrator::setTimeConstant(float tauMs) {
  tau = tauMs;
  calculateCoeff();
}

float LeakyIntegrator::process(float in) {
  y += c * (in - y);
  return y;
}

void LeakyIntegrator::reset() {
  y = 0.0f;
}

void LeakyIntegrator::calculateCoeff() {
  if (tau <= 0.0f) {
    c = 1.0f;
  } else {
    // c = 1 - exp(-1 / (tau * fs)) ? No, standard 1-pole is exp(-2pi*fc/fs)
    // Rosic uses: c = 1 - exp( -1.0 / (0.001*tau*fs) ) for "coeff" in some contexts
    // But for LeakyIntegrator: y[n] = y[n-1] + c * (x[n] - y[n-1])
    // where c = 1 - exp(-1 / (tau_seconds * fs))
    c = 1.0f - std::exp(-1.0f / (0.001f * tau * sampleRate));
  }
}
