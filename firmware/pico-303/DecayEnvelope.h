#pragma once
#include <cmath>
#include <algorithm>

/**
 * @file DecayEnvelope.h
 * @brief Simple Exponential Decay Envelope.
 */

/**
 * @class DecayEnvelope
 * @brief Implements a simple trigger-to-decay envelope.
 * Useful for percussive sounds or filter envelopes in 303 emulation.
 */
class DecayEnvelope {
public:
  /**
   * @brief Sets the sample rate.
   * @param sr Sample rate in Hz
   */
  void setSampleRate(float sr);
  
  /**
   * @brief Sets the decay time.
   * @param ms Decay time in milliseconds
   */
  void setDecayTime(float ms);
  
  /**
   * @brief Triggers the envelope (resets level to 1.0).
   */
  void trigger();
  
  /**
   * @brief Processes the envelope decay.
   * @return float Current envelope level
   */
  float process();
  
  /**
   * @brief Gets the current envelope value without processing.
   * @return float Current envelope level
   */
  float getCurrentValue() const;

private:
  float sampleRate = 44100.0f;
  float decayTime = 200.0f;
  float coeff = 0.99f;
  float y = 0.0f;
  
  void calculateCoeff();
};
