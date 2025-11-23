/**
 * @file Filter303.cpp
 * @brief Implementation of the Filter303 class.
 */

#include "Filter303.h"
#include <cmath>

Filter303::Filter303(float sr)
  : sampleRate(sr), cutoff(1000.0f), resonance(0.0f), envMod(0.0f),
    y1(0), y2(0), y3(0), y4(0) {}

void Filter303::setCutoff(float freq) {
  cutoff = freq;
  updateCoefficients();
}

void Filter303::setResonance(float res) {
  // Allow resonance > 1.0 for Devilfish self-oscillation
  resonance = std::fmax(0.0f, res);
}

void Filter303::setEnvMod(float amount) {
  envMod = amount;
}

void Filter303::setFMAmount(float amount) {
  fmAmount = amount;
}

float Filter303::process(float input, float env, float accentEnv, float fmInput) {
  float modAmt = std::fmin(std::fmax(envMod * env, -0.95f * cutoff), 4.0f * cutoff);
  
  // Apply FM: modulate cutoff by audio signal
  if (fmAmount > 0.001f) {
    modAmt += fmAmount * fmInput * 0.5f * cutoff;
  }

  float modCutoff = cutoff + modAmt;
  modCutoff = std::fmin(std::fmax(modCutoff, 5.0f), 0.45f * sampleRate);

  // JC303 / Open303 Logic - recalculate coefficients with modCutoff
  float wc = 2.0f * M_PI * modCutoff / sampleRate;
  float fx = wc * 0.70710678f / (2.0f * M_PI);
  
  float b0 = (0.00045522346f + 6.1922189f * fx) / (1.0f + 12.358354f * fx + 4.4156345f * (fx * fx));
  float k  = fx*(fx*(fx*(fx*(fx*(fx+7198.6997f)-5837.7917f)-476.47308f)+614.95611f)+213.87126f)+16.998792f;
  float g  = k * 0.058823529411764705882352941176471f; // 1/17
  
  // Apply resonance
  float r_skew = (1.0f - std::exp(-3.0f * resonance)) / 0.9502129316f;
  g = (g - 1.0f) * r_skew + 1.0f;
  g = (g * (1.0f + r_skew));
  k = k * r_skew;
  
  // 1. Feedback Highpass
  float feedback = processFeedbackHPF(k * y4);
  float y0 = input - feedback;
  
  // 2. 4-stage Ladder (Open303 topology)
  y1 += 2 * b0 * (y0 - y1 + y2);
  y2 +=     b0 * (y1 - 2 * y2 + y3);
  y3 +=     b0 * (y2 - 2 * y3 + y4);
  y4 +=     b0 * (y3 - 2 * y4);
  
  return 2 * g * y4;
}

float Filter303::getCutoff() const {
  return cutoff;
}

float Filter303::getEnvMod() const {
  return envMod;
}

void Filter303::updateCoefficients() {
  // Highpass coeff (fixed 150Hz)
  float w_hp = 2.0f * M_PI * hp_cutoff;
  // Simple 1-pole coeff approximation: exp(-2pi * fc / fs)
  hp_coeff = std::exp(-w_hp / sampleRate); 
}

float Filter303::processFeedbackHPF(float input) {
  // Simple 1-pole Highpass: y = x - lpf(x)
  hp_state += (1.0f - hp_coeff) * (input - hp_state);
  return input - hp_state;
}