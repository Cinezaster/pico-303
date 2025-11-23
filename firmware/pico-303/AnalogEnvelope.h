#pragma once
#include <cmath>
#include <algorithm>

/**
 * @file AnalogEnvelope.h
 * @brief ADSR Envelope Generator simplified for TB-303 emulation.
 */

/**
 * @class AnalogEnvelope
 * @brief Implements an Attack-Decay-Sustain-Release (ADSR) envelope.
 * Optimized for the specific envelope characteristics of the TB-303.
 */
class AnalogEnvelope {
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
  void setDecay(float ms);

  /**
   * @brief Sets the release time.
   * @param ms Release time in milliseconds
   */
  void setRelease(float ms);

  /**
   * @brief Sets the attack time.
   * @param ms Attack time in milliseconds
   */
  void setAttack(float ms);
  
  /**
   * @brief Triggers the Note On event (starts Attack phase).
   */
  void noteOn();
  
  /**
   * @brief Triggers the Note Off event (starts Release phase).
   */
  void noteOff();
  
  /**
   * @brief Processes the envelope and returns the current level.
   * @return float Envelope level [0.0 ... 1.0]
   */
  float process();
  
  /**
   * @brief Checks if the envelope is currently active (Note On).
   * @return true if Note On, false if Note Off
   */
  bool isActive() const;

private:
  enum State { IDLE, ATTACK, DECAY, RELEASE };
  State state = IDLE;
  bool isNoteOn = false;
  
  float sampleRate = 44100.0f;
  float decayTime = 1000.0f;
  float releaseTime = 10.0f;
  float attackTime = 3.0f; // Default 3ms attack
  
  float decayCoeff = 0.99f;
  float releaseCoeff = 0.9f;
  float attackCoeff = 0.1f;
  
  float currentLevel = 0.0f;
  
  void calculateCoeffs();
};
