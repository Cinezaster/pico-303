#pragma once
#include <vector>

/**
 * @file StereoDelay.h
 * @brief Stereo Delay effect with feedback and mix controls.
 */

/**
 * @class StereoDelay
 * @brief Implements a stereo delay line with independent left/right delay times.
 * Uses std::vector for buffer management to allow dynamic allocation.
 */
class StereoDelay {
public:
  /**
   * @brief Constructor.
   * @param maxDelay Maximum delay length in samples (default 44100 = 1 sec at 44.1kHz)
   */
  StereoDelay(int maxDelay = 44100);
  ~StereoDelay();

  /**
   * @brief Allocates memory for the delay buffers.
   * Must be called in setup(), not global scope.
   * @return true if allocation succeeded
   */
  bool begin(); 

  /**
   * @brief Sets the sample rate.
   * @param sr Sample rate in Hz
   */
  void setSampleRate(float sr);

  /**
   * @brief Sets the left channel delay time.
   * @param samples Delay time in samples
   */
  void setTimeSamplesL(int samples);

  /**
   * @brief Sets the right channel delay time.
   * @param samples Delay time in samples
   */
  void setTimeSamplesR(int samples);

  /**
   * @brief Sets the feedback amount.
   * @param fb Feedback gain (0.0 to 1.1, >1.0 allows self-oscillation saturation)
   */
  void setFeedback(float fb);

  /**
   * @brief Sets the dry/wet mix.
   * @param m Mix factor (0.0 = Dry, 1.0 = Wet)
   */
  void setMix(float m);

  /**
   * @brief Processes the left channel input (read-only).
   * @param input Audio input sample
   * @return float Delayed output sample mixed with dry signal
   */
  float processL(float input);

  /**
   * @brief Processes the right channel input (read-only).
   * @param input Audio input sample
   * @return float Delayed output sample mixed with dry signal
   */
  float processR(float input);

  /**
   * @brief Advances the delay buffer write index and writes new samples.
   * Should be called once per stereo frame after processL/processR.
   * @param inL Left channel input to write to buffer
   * @param inR Right channel input to write to buffer
   */
  void tick(float inL, float inR);

private:
  std::vector<float> bufferL;
  std::vector<float> bufferR;
  int maxDelaySamples;
  int writeIndex = 0;
  
  float sampleRate = 44100.0f;
  int delaySamplesL = 10000;
  int delaySamplesR = 10000;
  float feedback = 0.3f;
  float mix = 0.3f;
};
