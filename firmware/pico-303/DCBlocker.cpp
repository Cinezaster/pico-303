/**
 * @file DCBlocker.cpp
 * @brief Implementation of the DCBlocker class.
 */

#include "DCBlocker.h"

void DCBlocker::setSampleRate(float sr) {
  sampleRate = sr;
  calculateCoeff();
}

void DCBlocker::setCutoff(float hz) {
  cutoff = hz;
  calculateCoeff();
}

float DCBlocker::process(float input) {
  // y[n] = x[n] - x[n-1] + R * y[n-1]
  // Simple DC blocker algorithm
  float output = input - lastInput + R * lastOutput;
  lastInput = input;
  lastOutput = output;
  return output;
}

// Alternative 1-pole HPF: y = x - lpf(x)
// This is often more stable for simple HPF use
float DCBlocker::processHPF(float input) {
  lpfState += (input - lpfState) * alpha;
  return input - lpfState;
}

void DCBlocker::calculateCoeff() {
  // For standard DC blocker: R = 1 - (2*pi*fc/fs)
  R = 1.0f - (2.0f * M_PI * cutoff / sampleRate);
  
  // For 1-pole HPF via LPF subtraction:
  // alpha = 1 - exp(-2*pi*fc/fs)
  alpha = 1.0f - std::exp(-2.0f * M_PI * cutoff / sampleRate);
}
