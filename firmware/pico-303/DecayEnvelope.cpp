/**
 * @file DecayEnvelope.cpp
 * @brief Implementation of the DecayEnvelope class.
 */

#include "DecayEnvelope.h"

void DecayEnvelope::setSampleRate(float sr) {
  sampleRate = sr;
  calculateCoeff();
}

void DecayEnvelope::setDecayTime(float ms) {
  decayTime = ms;
  calculateCoeff();
}

void DecayEnvelope::trigger() {
  y = 1.0f;
}

float DecayEnvelope::process() {
  y *= coeff;
  return y;
}

float DecayEnvelope::getCurrentValue() const {
  return y;
}

void DecayEnvelope::calculateCoeff() {
  if (decayTime < 0.1f) decayTime = 0.1f;
  // y *= c  =>  y(t) = y(0) * c^(t*fs)
  // We want y(tau) = 1/e * y(0) ? Or just standard time constant?
  // Rosic: c = exp( -1.0 / (0.001*tau*fs) )
  coeff = std::exp(-1.0f / (0.001f * decayTime * sampleRate));
}
