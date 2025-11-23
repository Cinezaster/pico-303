/**
 * @file AnalogEnvelope.cpp
 * @brief Implementation of the AnalogEnvelope class.
 */

#include "AnalogEnvelope.h"

void AnalogEnvelope::setSampleRate(float sr) {
  sampleRate = sr;
  calculateCoeffs();
}

void AnalogEnvelope::setDecay(float ms) {
  decayTime = ms;
  calculateCoeffs();
}

void AnalogEnvelope::setRelease(float ms) {
  releaseTime = ms;
  calculateCoeffs();
}

void AnalogEnvelope::setAttack(float ms) {
  attackTime = ms;
  calculateCoeffs();
}

void AnalogEnvelope::noteOn() {
  isNoteOn = true;
  state = ATTACK;
  // Start from current level if retriggering to avoid clicks?
  // Or reset to 0? For 303, usually reset but with slight slew.
  // Let's start from 0 for now, but the attack phase will handle the ramp.
  currentLevel = 0.0f;
}

void AnalogEnvelope::noteOff() {
  isNoteOn = false;
  state = RELEASE;
}

float AnalogEnvelope::process() {
  if (state == ATTACK) {
    currentLevel += attackCoeff;
    if (currentLevel >= 1.0f) {
      currentLevel = 1.0f;
      state = DECAY;
    }
  }
  else if (state == DECAY) {
    currentLevel *= decayCoeff;
    if (currentLevel < 0.0001f) {
      currentLevel = 0.0f;
      state = IDLE;
    }
  } else if (state == RELEASE) {
    currentLevel *= releaseCoeff;
    if (currentLevel < 0.0001f) {
      currentLevel = 0.0f;
      state = IDLE;
    }
  }
  return currentLevel;
}

bool AnalogEnvelope::isActive() const {
  return isNoteOn;
}

void AnalogEnvelope::calculateCoeffs() {
  decayCoeff = std::exp(-1.0f / (0.001f * decayTime * sampleRate));
  releaseCoeff = std::exp(-1.0f / (0.001f * releaseTime * sampleRate));
  // Linear attack: increment per sample = 1.0 / (samples)
  float attackSamples = 0.001f * attackTime * sampleRate;
  if (attackSamples < 1.0f) attackSamples = 1.0f;
  attackCoeff = 1.0f / attackSamples;
}
