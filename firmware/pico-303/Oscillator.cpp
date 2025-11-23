/**
 * @file Oscillator.cpp
 * @brief Implementation of the Oscillator class.
 */

#include "Oscillator.h"
#include <cmath>
#include <algorithm>

void Oscillator::setFrequency(float f) {
  frequency = f;
  phaseIncrement = frequency / sampleRate;
  subPhaseIncrement = (frequency * 0.5f) / sampleRate;
  phase = 0.0f;
  subPhase = 0.0f;
}

void Oscillator::setWaveform(Waveform w) {
  waveform = w;
}

void Oscillator::setBlend(float b) {
  blend = std::max(0.0f, std::min(1.0f, b));
}

void Oscillator::setMode(bool jc303) {
  jc303Mode = jc303;
  // 0.53 is an approximation of the 303 square pulse width
  // derived from tanh(70*x + 4.37) shaping of a saw wave
  pulseWidth = jc303Mode ? 0.53f : 0.5f; 
}

void Oscillator::glideTo(float newFreq, float glideTimeMs) {
  targetFreq = newFreq;
  glideSamples = (glideTimeMs / 1000.0f) * sampleRate;
  if (glideSamples < 1) glideSamples = 1;
  glideStep = std::pow(targetFreq / frequency, 1.0f / glideSamples);  // exponential step
  glideCounter = (int)glideSamples;
}

void Oscillator::tick() {
  if (glideCounter > 0) {
    frequency *= glideStep;
    phaseIncrement = frequency / sampleRate;
    subPhaseIncrement = (frequency * 0.5f) / sampleRate;
    glideCounter--;
  }
}

void Oscillator::setSubBlend(float b) {
  subBlend = std::max(0.0f, std::min(1.0f, b));
}

float Oscillator::polyBLEP(float t) {
  if (t < phaseIncrement) {
    t /= phaseIncrement;
    return t + t - t * t - 1.0f;
  } else if (t > 1.0f - phaseIncrement) {
    t = (t - 1.0f) / phaseIncrement;
    return t * t + t + t + 1.0f;
  }
  return 0.0f;
}

float Oscillator::process() {
  tick();

  float shifted = fmod(phase + 0.5f, 1.0f);
  float saw = 2.0f * shifted - 1.0f - polyBLEP(shifted);
  
  // Variable Pulse Width Square (PolyBLEP)
  float square = (phase < pulseWidth ? 1.0f : -1.0f);
  square += polyBLEP(phase); // Rising edge at 0
  square -= polyBLEP(fmod(phase + 1.0f - pulseWidth, 1.0f)); // Falling edge at pulseWidth

  float value = (1.0f - blend) * square + blend * saw;

  // Sub oscillator
  float subVal = (subPhase < 0.5f) ? 1.0f : -1.0f;
  subPhase += subPhaseIncrement;
  if (subPhase >= 1.0f) subPhase -= floorf(subPhase);

  // ✅ Blend with sub
  value = (1.0f - subBlend) * value + subBlend * subVal;
  value *= 0.707f;

  phase += phaseIncrement;
  if (phase >= 1.0f) phase -= floorf(phase);  // clean wrap

  return value;
}