/**
 * @file StereoDelay.cpp
 * @brief Implementation of the StereoDelay class.
 */

#include "StereoDelay.h"
#include <cmath>
#include <algorithm>

StereoDelay::StereoDelay(int maxDelay) 
  : maxDelaySamples(maxDelay) {
  // Do NOT allocate here. Global constructors run before heap is ready.
}

StereoDelay::~StereoDelay() {}

bool StereoDelay::begin() {
  // Allocate memory now
  bufferL.resize(maxDelaySamples, 0.0f);
  bufferR.resize(maxDelaySamples, 0.0f);
  
  // Check if allocation succeeded (though std::vector usually throws)
  return (bufferL.size() == maxDelaySamples && bufferR.size() == maxDelaySamples);
}

void StereoDelay::setSampleRate(float sr) {
  sampleRate = sr;
}

void StereoDelay::setTimeSamplesL(int samples) {
  delaySamplesL = std::max(1, std::min(samples, maxDelaySamples - 1));
}

void StereoDelay::setTimeSamplesR(int samples) {
  delaySamplesR = std::max(1, std::min(samples, maxDelaySamples - 1));
}

void StereoDelay::setFeedback(float fb) {
  feedback = std::max(0.0f, std::min(fb, 1.1f));
}

void StereoDelay::setMix(float m) {
  mix = std::max(0.0f, std::min(m, 1.0f));
}

float StereoDelay::processL(float input) {
  if (bufferL.empty()) return input; // Safety check

  int readIndex = writeIndex - delaySamplesL;
  if (readIndex < 0) readIndex += maxDelaySamples;
  
  float delayed = bufferL[readIndex];
  return (1.0f - mix) * input + mix * delayed;
}

float StereoDelay::processR(float input) {
  if (bufferR.empty()) return input; // Safety check

  int readIndex = writeIndex - delaySamplesR;
  if (readIndex < 0) readIndex += maxDelaySamples;
  
  float delayed = bufferR[readIndex];
  return (1.0f - mix) * input + mix * delayed;
}

void StereoDelay::tick(float inL, float inR) {
  if (bufferL.empty() || bufferR.empty()) return;

  int readIndexL = writeIndex - delaySamplesL;
  if (readIndexL < 0) readIndexL += maxDelaySamples;
  
  int readIndexR = writeIndex - delaySamplesR;
  if (readIndexR < 0) readIndexR += maxDelaySamples;

  float delayedL = bufferL[readIndexL];
  float delayedR = bufferR[readIndexR];

  // Feedback with saturation
  float nextL = inL + delayedL * feedback;
  float nextR = inR + delayedR * feedback;
  
  // Soft clip feedback to prevent explosion
  nextL = std::tanh(nextL);
  nextR = std::tanh(nextR);

  bufferL[writeIndex] = nextL;
  bufferR[writeIndex] = nextR;

  writeIndex++;
  if (writeIndex >= maxDelaySamples) writeIndex = 0;
}