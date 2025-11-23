#pragma once
#include <cmath>
#include <algorithm>

/**
 * @file LeakyIntegrator.h
 * @brief Simple 1-pole Lowpass Filter / Leaky Integrator.
 */

/**
 * @class LeakyIntegrator
 * @brief Implements a simple 1-pole lowpass filter.
 * Used for smoothing control signals or simple audio filtering.
 */
class LeakyIntegrator {
public:
  /**
   * @brief Sets the sample rate.
   * @param sr Sample rate in Hz
   */
  void setSampleRate(float sr);

  /**
   * @brief Sets the time constant (tau).
   * @param tauMs Time constant in milliseconds
   */
  void setTimeConstant(float tauMs);
  
  /**
   * @brief Processes a sample through the integrator.
   * @param in Input value
   * @return float Filtered output value
   */
  float process(float in);
  
  /**
   * @brief Resets the integrator state to 0.
   */
  void reset();

private:
  float sampleRate = 44100.0f;
  float tau = 15.0f; // ms
  float c = 0.1f;
  float y = 0.0f;
  
  void calculateCoeff();
};
