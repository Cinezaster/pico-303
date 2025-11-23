#pragma once
#include <cmath>

/**
 * @file DCBlocker.h
 * @brief DC Blocking filter and Highpass Filter.
 */

/**
 * @class DCBlocker
 * @brief Removes DC offset from the signal.
 * Provides both a standard DC blocker algorithm and a 1-pole HPF implementation.
 */
class DCBlocker {
public:
  /**
   * @brief Sets the sample rate.
   * @param sr Sample rate in Hz
   */
  void setSampleRate(float sr);

  /**
   * @brief Sets the cutoff frequency.
   * @param hz Cutoff frequency in Hz
   */
  void setCutoff(float hz);

  /**
   * @brief Processes a sample using the standard DC blocker algorithm.
   * @param input Audio input sample
   * @return float Output sample
   */
  float process(float input);
  
  /**
   * @brief Processes a sample using a 1-pole Highpass Filter (HPF).
   * Implemented as y = x - lpf(x). Often more stable for simple HPF use.
   * @param input Audio input sample
   * @return float Output sample
   */
  float processHPF(float input);

private:
  float sampleRate = 44100.0f;
  float cutoff = 25.0f;
  float R = 0.995f;
  float lastInput = 0.0f;
  float lastOutput = 0.0f;
  
  float lpfState = 0.0f;
  float alpha = 0.1f;

  void calculateCoeff();
};
