#pragma once

/**
 * @file Filter303.h
 * @brief 4-pole Diode Ladder Filter emulation (TB-303 style).
 */

/**
 * @class Filter303
 * @brief Emulates the TB-303 filter response using Open303/JC303 coefficients.
 * Includes resonance, envelope modulation, and accent modulation.
 */
class Filter303 {
public:
  Filter303(float sampleRate = 44100.0f);

  /**
   * @brief Sets the base cutoff frequency.
   * @param freq Frequency in Hz
   */
  void setCutoff(float freq);

  /**
   * @brief Sets the resonance amount.
   * @param res Resonance [0.0 ... 1.0] (can go >1.0 for self-oscillation)
   */
  void setResonance(float res);

  /**
   * @brief Sets the envelope modulation amount.
   * @param amount Modulation depth
   */
  void setEnvMod(float amount);

  /**
   * @brief Sets the accent modulation amount.
   * @param amount Accent depth
   */
  void setAccentMod(float amount);

  /**
   * @brief Sets the FM amount.
   * @param amount FM depth [0.0 ... 1.0]
   */
  void setFMAmount(float amount);
  
  /**
   * @brief Processes a single sample through the filter.
   * @param input Audio input sample
   * @param env Envelope value [0.0 ... 1.0]
   * @param accentEnv Accent envelope value [0.0 ... 1.0]
   * @param fmInput FM modulator input
   * @return float Filtered output sample
   */
  float process(float input, float env, float accentEnv = 0.0f, float fmInput = 0.0f);

  float getCutoff() const;
  float getEnvMod() const;

private:
  float sampleRate;
  float cutoff;
  float resonance;
  float envMod;
  float accentMod = 0.0f;
  float fmAmount = 0.0f;

  float y1, y2, y3, y4;

  void updateCoefficients();

  // JC303 / Open303 Specifics
  float jc_b0 = 0.0f;
  float jc_k = 0.0f;
  float jc_g = 0.0f;
  // Feedback Highpass (1-pole)
  float hp_state = 0.0f;
  float hp_cutoff = 150.0f;
  float hp_coeff = 0.0f;
  
  float processFeedbackHPF(float input);
};
