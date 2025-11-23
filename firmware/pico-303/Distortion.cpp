/**
 * @file Distortion.cpp
 * @brief Implementation of the Distortion class.
 */

#include "Distortion.h"

float Distortion::process(float input) {
  if (!enabled || amount <= 0.01f) return input;

  float drive = 1.0f + amount * 9.0f; // Map 0..1 to 1..10 drive
  float wetSignal = input;

  switch (type) {
    case SOFT_CLIP:
      wetSignal = processSoftClip(input, drive);
      break;
    case HARD_CLIP:
      wetSignal = processHardClip(input, drive);
      break;
    case WAVEFOLDER:
      wetSignal = processWavefolder(input, drive);
      break;
    case DIODE_CLIPPER:
      wetSignal = processDiode(input, drive);
      break;
    case WAVENET_TUBE:
      wetSignal = processWaveNet(input, drive);
      break;
  }

  return (1.0f - mix) * input + mix * wetSignal;
}

float Distortion::processSoftClip(float x, float drive) {
  return std::tanh(x * drive);
}

float Distortion::processHardClip(float x, float drive) {
  float val = x * drive;
  return std::fmax(-1.0f, std::fmin(1.0f, val));
}

float Distortion::processWavefolder(float x, float drive) {
  float val = x * drive;
  if (val > 1.0f) val = 2.0f - val;
  else if (val < -1.0f) val = -2.0f - val;
  
  // Safety clamp to prevent runaway folding
  return std::fmax(-1.0f, std::fmin(1.0f, val));
}

float Distortion::processDiode(float x, float drive) {
  // Asymmetric clipping simulation
  float val = x * drive;
  if (val >= 0) {
    return std::tanh(val); // Positive swings clip normally
  } else {
    // Negative swings clip harder/earlier (or softer depending on diode config)
    // Let's simulate a diode that conducts differently
    return std::tanh(val * 0.5f) * 2.0f; // Softer clipping on negative
  }
}

float Distortion::processWaveNet(float x, float drive) {
  // Polynomial approximation of a tube-like saturation curve
  // y = x - a*x^2 + b*x^3 ...
  // This creates even harmonics (asymmetry)
  
  float val = x * drive;
  // Simple "Tube" polynomial: f(x) = x - 0.15*x^2
  // But we need to keep it bounded.
  
  // Let's use a soft asymmetric curve:
  if (val > 1.0f) val = 1.0f;
  if (val < -1.0f) val = -1.0f;
  
  // Add 2nd harmonic (asymmetry)
  float out = val - 0.2f * val * val; 
  
  // Soft clip the result
  return std::tanh(out) * 1.2f; // Makeup gain
}
