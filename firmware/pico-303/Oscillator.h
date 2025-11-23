#pragma once

/**
 * @file Oscillator.h
 * @brief Band-limited Oscillator class with Saw/Square waveforms and sub-oscillator.
 */

/**
 * @class Oscillator
 * @brief Generates band-limited waveforms using PolyBLEP technique.
 * Supports Sawtooth, Square (with variable pulse width), and Sub-oscillator.
 */
class Oscillator {
public:
  enum Waveform { SAW,
                  SQUARE };

  /**
   * @brief Sets the sample rate for the oscillator.
   * @param rate Sample rate in Hz
   */
  void setSampleRate(float rate) {
    sampleRate = rate;
  }

  /**
   * @brief Sets the oscillator frequency.
   * @param f Frequency in Hz
   */
  void setFrequency(float f);

  /**
   * @brief Sets the waveform type.
   * @param w Waveform enum (SAW or SQUARE)
   */
  void setWaveform(Waveform w);

  /**
   * @brief Sets the blend between Square and Saw waveforms.
   * @param b Blend factor (0.0 = Square, 1.0 = Saw)
   */
  void setBlend(float b);

  /**
   * @brief Sets the sub-oscillator mix level.
   * @param b Mix level (0.0 = No sub, 1.0 = Full sub)
   */
  void setSubBlend(float b);

  /**
   * @brief Glides to a new frequency over a specified time.
   * @param newFreq Target frequency in Hz
   * @param glideTimeMs Glide duration in milliseconds
   */
  void glideTo(float newFreq, float glideTimeMs);

  /**
   * @brief Advances the oscillator state. Should be called once per sample.
   */
  void tick();

  /**
   * @brief Generates the next audio sample.
   * @return float Audio sample in range [-1.0, 1.0]
   */
  float process();

  /**
   * @brief Sets the oscillator mode (Standard vs JC303).
   * @param jc303 If true, enables JC303 mode with 53% pulse width.
   */
  void setMode(bool jc303);

  /**
   * @brief Resets the oscillator phase to 0.
   */
  void resetPhase() { phase = 0.0f; subPhase = 0.0f; }
  
  /**
   * @brief PolyBLEP (Polynomial Band-Limited Step) function.
   * Used to reduce aliasing on sharp transitions.
   * @param t Phase difference from transition
   * @return float Correction value
   */
  float polyBLEP(float t);

private:
  float sampleRate = 44100.0f;
  float frequency = 440.0f;
  float phase = 0.0f;
  float phaseIncrement = 0.01f;
  float blend = 0.0f;
  float targetFreq = 440.0f;
  float glideSamples = 0;
  float glideStep = 0;
  int glideCounter = 0;
  float subBlend = 0.0f;
  float subPhase = 0.0f;
  float subPhaseIncrement = 0.005f;
  Waveform waveform = SAW;
  
  bool jc303Mode = true;
  float pulseWidth = 0.5f; // 0.5 = square, 0.53 = 303-ish
};