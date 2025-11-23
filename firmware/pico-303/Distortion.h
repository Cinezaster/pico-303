#pragma once
#include <cmath>
#include <algorithm>

/**
 * @file Distortion.h
 * @brief Multi-mode Distortion effect class.
 */

/**
 * @class Distortion
 * @brief Provides various distortion algorithms including Soft Clip, Hard Clip,
 * Wavefolder, Diode Clipper, and WaveNet Tube simulation.
 */
class Distortion {
public:
  /**
   * @brief Distortion algorithm types.
   */
  enum Type {
    SOFT_CLIP,      ///< Tanh soft clipping
    HARD_CLIP,      ///< Hard clamping
    WAVEFOLDER,     ///< Sine-like folding
    DIODE_CLIPPER,  ///< Asymmetric diode simulation
    WAVENET_TUBE    ///< Polynomial tube simulation
  };

  Distortion() {}

  /**
   * @brief Sets the distortion type.
   * @param t Distortion Type enum
   */
  void setType(Type t) { type = t; }

  /**
   * @brief Sets the input drive amount.
   * @param amt Drive amount (0.0 to 1.0), internally scaled to useful range.
   */
  void setAmount(float amt) { amount = amt; }

  /**
   * @brief Sets the dry/wet mix.
   * @param m Mix factor (0.0 = Dry, 1.0 = Wet)
   */
  void setMix(float m) { mix = std::fmax(0.0f, std::fmin(1.0f, m)); }

  /**
   * @brief Enables or disables the effect.
   * @param e True to enable, false to bypass
   */
  void setEnabled(bool e) { enabled = e; }

  /**
   * @brief Processes a single sample through the distortion effect.
   * @param input Audio input sample
   * @return float Distorted output sample
   */
  float process(float input);

private:
  Type type = SOFT_CLIP;
  float amount = 0.0f;
  float mix = 1.0f;
  bool enabled = false;

  // Internal processing functions
  float processSoftClip(float x, float drive);
  float processHardClip(float x, float drive);
  float processWavefolder(float x, float drive);
  float processDiode(float x, float drive);
  float processWaveNet(float x, float drive);
};
