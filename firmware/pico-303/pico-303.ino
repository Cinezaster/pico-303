/**
 * @file pico-303.ino
 * @brief Main firmware file for the pico-303 synthesizer.
 * 
 * This file handles the main setup, audio processing loop, MIDI event handling,
 * and integration of various synthesis components (Oscillator, Filter, Envelopes, Effects).
 */

#include <algorithm>
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include <I2S.h>
#include <AudioBufferManager.h>
#include <cmath>

#include "Oscillator.h"
#include "Filter303.h"
#include "StereoDelay.h"
#include "DecayEnvelope.h"
#include "AnalogEnvelope.h"
#include "LeakyIntegrator.h"
#include "Distortion.h"
#include "DCBlocker.h"

// =============================================================================
// Debug Configuration
// =============================================================================
#define DEBUG_SERIAL false // Set to false to disable Serial output

#if DEBUG_SERIAL
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
  #define DEBUG_BEGIN(...) Serial.begin(__VA_ARGS__)
#else
  #define DEBUG_PRINTF(...)
  #define DEBUG_PRINTLN(...)
  #define DEBUG_BEGIN(...)
#endif

// =============================================================================
// Globals & Objects
// =============================================================================

// Audio Objects
Oscillator osc;
Filter303 filter;
StereoDelay delayFx;
Distortion distFx;
DCBlocker hpfPostFilter;

// Open303 Envelopes & Voice State
DecayEnvelope envFilt;      // Filter Envelope (was mainEnv)
AnalogEnvelope envAmp;      // Amp Envelope (was ampEnv)
LeakyIntegrator accentSmoother; // rc2 in Open303
LeakyIntegrator ampDeClicker;   // Smoothes VCA signal to prevent clicks

float accentLevel = 0.5f; // 0.0 to 1.0 (controlled by CC15)
float currentAccentGain = 0.0f; // Actual gain applied to current note
float envMod = 0.5f;  // 0..1
float accent = 0.5f;  // 0..1

// Audio Buffer
const int sampleRate = 44100;
const int bufferSize = 128;
int16_t buffer[bufferSize * 2]; // Stereo buffer

// I2S pins
#define pBCLK 20
#define pDOUT 19

I2S i2sOut(OUTPUT, pBCLK, pDOUT);

// MIDI
Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

// Sequencer / Note State
int currentNote = -1;
bool slideActive = false;

// LED pin
const int LED_PIN = 25;
uint32_t ledOnUntil = 20;

volatile uint32_t clockTickCount = 0;
uint32_t lastClockMicros = 0;
float bpm = 120.0f;

// MIDI state (Modified‑Naive)
uint8_t prev_note = 0xFF;
uint8_t noteOverlap = 0;

// Placeholder for removed globals
float volume = 0.5f;
int16_t amp = 1500; // Reduced from 3000 to prevent clipping with high resonance/accent
float dryWetMix = 0.25f;  // default to fully wet
bool lastNoteWasAccented = false;
float pitchOffset = 0.0f; // in semitones
float globalEnvMod = 2000.0f;
float driveAmount = 1.0f;  // pre-filter saturation drive
float glideTimeMs = 80.0f;  // default TB-303 glide time

float userDecayTime = 1000.0f; // Store user setting for decay

// ---- Delay globals ----
const int maxDelaySamples = 44100;  // 1 second delay max
int delayTimeSamplesL = 22050;
int delayTimeSamplesR = 22050;
float delayFeedback = 0.5f;
float delayMix = 0.3f;
float delayLpfL = 0.0f;
float delayLpfR = 0.0f;
float delayLpfAmount = 0.3f;
int stereoOffsetSamples = 220;  // ~5ms at 44.1kHz

// Delay modifiers: 0 = Full, 1 = Dotted, 2 = Triplet
int delayModL = 0;
int delayModR = 0;

StereoDelay stereoDelay;

/**
 * @brief Arduino Setup function.
 * Initializes pins, Serial, I2S, MIDI, and synthesis objects.
 */
void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  DEBUG_BEGIN(115200);
  DEBUG_PRINTLN("PICO-303 Synth Starting");

  // I2S setup
  i2sOut.setBitsPerSample(16);
  if (!i2sOut.begin(sampleRate)) {
    DEBUG_PRINTLN("I2S init failed");
    while (1);
  }

  // MIDI setup
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleClock(handleClock);

  // Osc
  osc.setSampleRate(sampleRate);
  osc.setWaveform(Oscillator::SQUARE);
  osc.setMode(true); // Enable JC303 mode (Square = Pulse 53%)
  envAmp.setDecay(300.0f);    // 300ms
  envAmp.setRelease(10.0f);   // 10ms
  envFilt.setDecayTime(1000.0f); // 1000ms

  filter.setCutoff(1000.0f);
  filter.setResonance(0.0f);
  filter.setEnvMod(500.0f);      // how much the envelope modulates cutoff

  stereoDelay.setSampleRate(sampleRate);
  if (!stereoDelay.begin()) {
    DEBUG_PRINTLN("ERROR: Failed to allocate delay buffer!");
  }
  stereoDelay.setTimeSamplesL(11025); // 250ms0Hz LPF (tau ~= 0.8ms)
  // Let's use 2.0ms to be safe and smooth.
  ampDeClicker.setSampleRate(sampleRate);
  ampDeClicker.setTimeConstant(2.0f); 
  
  // Post-filter HPF to remove DC offset (crucial for distortion)
  hpfPostFilter.setSampleRate(sampleRate);
  hpfPostFilter.setCutoff(30.0f); // ~25-30Hz like Open303
}

/**
 * @brief Main execution loop.
 * Handles MIDI input and audio processing.
 * Note: Audio processing happens in the loop, pushing samples to the I2S buffer.
 */
void loop() {
  MIDI.read();

  if (millis() > ledOnUntil) {
    digitalWrite(LED_PIN, LOW);
  }
  
  // Process single sample
  float envAmpOut = envAmp.process();
  float envFiltOut = envFilt.process();
  float rawSample = osc.process();
  float filtered = filter.process(rawSample, envFiltOut, lastNoteWasAccented ? 1.0f : 0.0f);
  
  // Remove DC offset caused by resonant filter *before* VCA/Distortion
  filtered = hpfPostFilter.processHPF(filtered);
  
  // VCA Mixing (Open303 Style)
  float vcaMod = envAmpOut;
  if (envAmp.isActive()) {
      vcaMod += 0.45f * envFiltOut;
      // Apply accent thump scaled by currentAccentGain
      // Reduced from 4.0f to 3.0f to prevent massive peaks
      vcaMod += currentAccentGain * 3.0f * envFiltOut;
  }
  
  // Smooth the VCA signal to remove clicks from instant envelope jumps
  vcaMod = ampDeClicker.process(vcaMod);
  
  // Apply VCA *before* Distortion
  // This allows the accent "thump" to drive the distortion harder
  float vcaOutput = filtered * vcaMod;

  // Apply Distortion (Post-VCA)
  float distorted = distFx.process(vcaOutput);
  
  float sample = distorted * volume;
  float dryL = sample;
  float dryR = sample;

  float outL = stereoDelay.processL(dryL);
  float outR = stereoDelay.processR(dryR);
  stereoDelay.tick(dryL, dryR);

  // Soft Clipper on final output (Musical limiting)
  // We attenuate the input by 0.10x (was 0.15x) to handle the high dynamic range.
  // Output is scaled to 30000 to use the full 16-bit range.
  float finalL = std::tanh(outL * 0.10f) * 30000.0f;
  float finalR = std::tanh(outR * 0.10f) * 30000.0f;

  i2sOut.write((int16_t)finalL);
  i2sOut.write((int16_t)finalR);
}

// ---- MIDI handlers ----

/**
 * @brief Handles MIDI Note On events.
 * Triggers envelopes, sets frequency, and handles accent/slide logic.
 * 
 * @param channel MIDI channel (1-16)
 * @param pitch MIDI note number (0-127)
 * @param velocity Note velocity (0-127)
 */
void handleNoteOn(byte channel, byte pitch, byte velocity) {
  bool slide = (prev_note != 0xFF);
  bool accent = (velocity >= 100);

  // Modified‑Naive overlap counter
  if (prev_note == pitch) noteOverlap++;
  prev_note = pitch;

  // Blink LED
  digitalWrite(LED_PIN, HIGH);
  ledOnUntil = millis() + 20;

  float freq = 440.0f * powf(2.0f, (pitch + pitchOffset - 69) / 12.0f);
  osc.glideTo(freq, slide ? glideTimeMs : 0.0f);  // use configurable glide time

  // Accent and envelope logic
  if (!slide || accent) {
    if (!slide) {
      osc.resetPhase();
    }
    envAmp.setRelease(accent ? 50.0f : 10.0f);
    envAmp.noteOn();
    
    // Use user-set decay time for normal notes, fixed 200ms for accent (TB-303 behavior)
    envFilt.setDecayTime(accent ? 200.0f : userDecayTime);
    envFilt.trigger();
    
    // Calculate accent gain for this note
    currentAccentGain = accent ? accentLevel : 0.0f;
    
    // Filter modulation
    // Base mod + Accent mod
    // If accentLevel is 1.0, we want max boost (e.g. 2.5x total or similar)
    // Old logic: accentBoost 1.0..2.5
    // New logic: 1.0 + accentLevel * 1.5
    float boost = 1.0f + currentAccentGain * 1.5f;
    float modAmt = globalEnvMod * boost;
    
    modAmt = std::min(modAmt, 3000.0f);  // cap to prevent filter overload
    filter.setEnvMod(modAmt);
  }

  amp = 3000;
  lastNoteWasAccented = accent;

  DEBUG_PRINTF("NoteON ch%u pitch%u vel%u slide=%d accent=%d\n",
                channel, pitch, velocity, slide, accent);
}

/**
 * @brief Handles MIDI Note Off events.
 * Manages note overlap for legato playing and triggers release phase.
 * 
 * @param channel MIDI channel
 * @param pitch MIDI note number
 * @param velocity Release velocity
 */
void handleNoteOff(byte channel, byte pitch, byte velocity) {
  // Modified‑Naive: decrement overlap before noteOff
  if (prev_note == pitch) {
    if (noteOverlap > 0) {
      noteOverlap--;
      return;
    }
    // If no overlap left, stop tone
    prev_note = 0xFF;
    envAmp.noteOff();

    digitalWrite(LED_PIN, HIGH);
    ledOnUntil = millis() + 20;

    DEBUG_PRINTF("NoteOFF ch%u pitch%u vel%u\n",
                  channel, pitch, velocity);
  }
}

/**
 * @brief Handles MIDI Control Change events.
 * Updates synth parameters based on CC messages.
 * 
 * @param channel MIDI channel
 * @param cc Control Change number
 * @param value Control value (0-127)
 */
void handleControlChange(byte channel, byte cc, byte value) {
  if (cc == 7) {  // Volume
    // Rescale volume: Max (127) = 0.5 (safe level)
    volume = (value / 127.0f) * 0.5f;
    DEBUG_PRINTF("CC7 Volume: %.2f\n", volume);
  }
  else if (cc == 14) {  // Sub oscillator blend
    float subAmt = value / 127.0f;
    osc.setSubBlend(subAmt);
    DEBUG_PRINTF("CC14 Sub Blend: %.2f\n", subAmt);
  }
  else if (cc == 15) {  // Accent intensity
    accentLevel = value / 127.0f; // 0.0 to 1.0
    DEBUG_PRINTF("CC15 Accent Level: %.2f\n", accentLevel);
  }
  else if (cc == 16) {  // Pitch offset
    pitchOffset = (value - 64) / 64.0f * 12.0f; // ±12 semitones
    DEBUG_PRINTF("CC16 Pitch Offset: %.2f semitones\n", pitchOffset);
  }
  else if (cc == 17) {  // Mod envelope amount
    globalEnvMod = (value / 127.0f) * 3000.0f;  // reduced to avoid filter instability
    DEBUG_PRINTF("CC17 Env Mod: %.1f\n", globalEnvMod);
  }
  else if (cc == 18) {  // Waveform blend
    float blendVal = value / 127.0f;
    osc.setBlend(blendVal);
    DEBUG_PRINTF("CC18 Waveform Blend: %.2f\n", blendVal);
  }
  else if (cc == 71) {  // Resonance
    float res = value / 127.0f;
    float shaped = powf(res, 0.8f);  // slightly aggressive but safe shaping
    filter.setResonance(std::min(shaped, 1.0f));  // ensure cap
    DEBUG_PRINTF("CC71 Resonance: %.2f (shaped: %.2f)\n", res, shaped);
  }
  else if (cc == 74) {  // Filter Cutoff
    // Exponential mapping: 300Hz to 3000Hz
    // freq = min * (max/min)^(val/127)
    float freq = 300.0f * pow(3000.0f / 300.0f, value / 127.0f);
    filter.setCutoff(freq);
    DEBUG_PRINTF("CC74 Cutoff: %.1f Hz\n", freq);
  }
  else if (cc == 75) {  // Envelope decay time
    userDecayTime = 50.0f + (value / 127.0f) * 1950.0f; // 50ms to 2000ms
    envFilt.setDecayTime(userDecayTime); // Update immediately
    DEBUG_PRINTF("CC75 Decay Time: %.2f ms\n", userDecayTime);
  }
  else if (cc == 76) {  // Pre-filter drive
    driveAmount = 1.0f + (value / 127.0f) * 4.0f;  // 1x to 5x drive
    DEBUG_PRINTF("CC76 Drive Amount: %.2f\n", driveAmount);
  }
  else if (cc == 77) {  // Distortion mode
    distFx.setType(static_cast<Distortion::Type>(value % 5));
    DEBUG_PRINTF("CC77 Dist Mode: %d\n", value % 5);
  }
  else if (cc == 78) {  // Distortion amount
    float amt = value / 127.0f;
    distFx.setAmount(amt);
    DEBUG_PRINTF("CC78 Dist Amount: %.2f\n", amt);
  }
  else if (cc == 79) {  // Distortion Mix
    float mix = value / 127.0f;
    distFx.setMix(mix);
    DEBUG_PRINTF("CC79 Dist Mix: %.2f\n", mix);
  }
  else if (cc == 80) {  // Distortion On/Off (was Tone Amount)
    bool on = value > 63;
    distFx.setEnabled(on);
    DEBUG_PRINTF("CC80 Dist Enable: %d\n", on);
  }
  else if (cc == 81) {  // Delay Time
    delayTimeSamplesL = map(value, 0, 127, 2000, 44100);  // 2ms to 1s
    delayTimeSamplesR = delayTimeSamplesL;
    stereoDelay.setTimeSamplesL(delayTimeSamplesL);
    stereoDelay.setTimeSamplesR(delayTimeSamplesR);
    DEBUG_PRINTF("CC81 Delay Time: %d samples\n", delayTimeSamplesL);
  }
  else if (cc == 82) {  // Delay Feedback
    delayFeedback = value / 127.0f;
    stereoDelay.setFeedback(delayFeedback);
    DEBUG_PRINTF("CC82 Feedback: %.2f\n", delayFeedback);
  }
  else if (cc == 83) {  // Delay Mix
    delayMix = value / 127.0f;
    stereoDelay.setMix(delayMix);
    DEBUG_PRINTF("CC83 Mix: %.2f\n", delayMix);
  }
  else if (cc == 84) {  // Delay LPF amount
    delayLpfAmount = value / 127.0f;
    DEBUG_PRINTF("CC84 Delay LPF Amount: %.2f\n", delayLpfAmount);
  }
  else if (cc == 86) {
    int div = max(1, value / 16);  // Map 0–127 to divs
    float beats = pow(2, div - 1) / 4.0f;  // 1/16, 1/8, 1/4, etc.
    delayTimeSamplesL = beatsToSamples(beats);
    delayTimeSamplesR = delayTimeSamplesL;
    stereoDelay.setTimeSamplesL(delayTimeSamplesL);
    stereoDelay.setTimeSamplesR(delayTimeSamplesR);
    DEBUG_PRINTF("CC86 Delay Sync Division: 1/%d beat, %d samples (BPM %.1f)\n", (int)(1.0f / beats), delayTimeSamplesL, bpm);
  }
  else if (cc == 88) {  // Stereo Offset
    stereoOffsetSamples = map(value, 0, 127, 0, 1024);  // up to ~23ms at 44.1kHz
    DEBUG_PRINTF("CC88 Stereo Offset: %d samples\n", stereoOffsetSamples);
  }
  else if (cc == 91) {
    int div = max(1, value / 16);
    float beats = pow(2, div - 1) / 4.0f;
    if (delayModL == 1) beats *= 1.5f;       // Dotted
    else if (delayModL == 2) beats *= 2.0f / 3.0f; // Triplet
    int samples = beatsToSamples(beats);
    samples = std::clamp(samples, 1, maxDelaySamples - 1);
    delayTimeSamplesL = samples;
    stereoDelay.setTimeSamplesL(delayTimeSamplesL);
    DEBUG_PRINTF("CC91 Delay L Div: 1/%d beat, %d samples\n", (int)(1.0f / beats), delayTimeSamplesL);
  }
  else if (cc == 92) {
    int div = max(1, value / 16);
    float beats = pow(2, div - 1) / 4.0f;
    if (delayModR == 1) beats *= 1.5f;
    else if (delayModR == 2) beats *= 2.0f / 3.0f;
    int samples = beatsToSamples(beats);
    samples = std::clamp(samples, 1, maxDelaySamples - 1);
    delayTimeSamplesR = samples;
    stereoDelay.setTimeSamplesR(delayTimeSamplesR);
    DEBUG_PRINTF("CC92 Delay R Div: 1/%d beat, %d samples\n", (int)(1.0f / beats), delayTimeSamplesR);
  }
  else if (cc == 93) {  // Delay L Modifier
    delayModL = value % 3;
    int div = max(1, delayTimeSamplesL > 0 ? log2f((delayTimeSamplesL * 4.0f / sampleRate) / (60.0f / bpm)) + 1 : 2);
    float beats = pow(2, div - 1) / 4.0f;
    if (delayModL == 1) beats *= 1.5f;
    else if (delayModL == 2) beats *= 2.0f / 3.0f;
    int samples = beatsToSamples(beats);
    samples = std::clamp(samples, 1, maxDelaySamples - 1);
    delayTimeSamplesL = samples;
    stereoDelay.setTimeSamplesL(delayTimeSamplesL);
    DEBUG_PRINTF("CC93 Delay L Mod: %d -> %d samples\n", delayModL, delayTimeSamplesL);
  }
  else if (cc == 94) {  // Delay R Modifier
    delayModR = value % 3;
    int div = max(1, delayTimeSamplesR > 0 ? log2f((delayTimeSamplesR * 4.0f / sampleRate) / (60.0f / bpm)) + 1 : 2);
    float beats = pow(2, div - 1) / 4.0f;
    if (delayModR == 1) beats *= 1.5f;
    else if (delayModR == 2) beats *= 2.0f / 3.0f;
    int samples = beatsToSamples(beats);
    samples = std::clamp(samples, 1, maxDelaySamples - 1);
    delayTimeSamplesR = samples;
    stereoDelay.setTimeSamplesR(delayTimeSamplesR);
    DEBUG_PRINTF("CC94 Delay R Mod: %d -> %d samples\n", delayModR, delayTimeSamplesR);
  }
  else if (cc == 100) {  // Glide Time
    glideTimeMs = (value == 64) ? 80.0f : (value / 127.0f) * 500.0f;
    DEBUG_PRINTF("CC100 Glide Time: %.1f ms\n", glideTimeMs);
  }
}

/**
 * @brief Handles MIDI Clock events.
 * Calculates BPM based on clock interval.
 */
void handleClock() {
  clockTickCount++;

  if (clockTickCount % 24 == 0) {  // one quarter note
    uint32_t now = micros();
    uint32_t interval = now - lastClockMicros;
    lastClockMicros = now;

    if (interval > 0) {
      bpm = 60.0f * 1000000.0f / (float)interval;
      DEBUG_PRINTF("MIDI Clock BPM: %.2f\n", bpm);
    }
  }
}

/**
 * @brief Converts musical beats to sample count based on current BPM.
 * 
 * @param beats Number of beats (e.g., 0.25 for 1/16th note)
 * @return int Number of samples
 */
int beatsToSamples(float beats) {
  float seconds = (60.0f / bpm) * beats;
  return (int)(seconds * sampleRate);
}