# pico-303

A TB-303 style synthesizer firmware for the Raspberry Pi Pico, featuring a hybrid emulation engine.

## Acknowledgments

This project stands on the shoulders of giants. Huge thanks to:

*   **[Open303](https://github.com/open303/open303)**: For the core DSP and envelope modeling.
*   **[jc303](https://github.com/jc303/jc303)**: For additional filter and synthesis insights.

## Hardware

The audio output is handled by a **PCM5100A Stereo DAC** connected to the Raspberry Pi Pico 2 via I2S. This provides high-quality, low-noise audio output far superior to PWM.

## Web Controller

Included in the `webController` directory is a `index.html` file that serves as a full-featured Web MIDI interface for the synth.

### Features
*   **Full Parameter Control**: Sliders for every CC parameter supported by the firmware.
*   **16-Step Sequencer**: A built-in TB-303 style sequencer with slide and accent support.
*   **Distortion & Delay Control**: Dedicated sections for tweaking the effects chain.

### Usage
1.  Connect the Pico to your computer via USB.
2.  Open `webController/index.html` in a Web MIDI compatible browser (e.g., Chrome, Edge).
3.  Select the Pico from the "MIDI Output" dropdown.
4.  Start tweaking parameters or program a sequence!

## MIDI Implementation

The synth responds to the following MIDI Control Change (CC) messages:

| CC | Parameter | Description |
| :--- | :--- | :--- |
| 7 | Volume | Master volume control |
| 14 | Sub Blend | Sub-oscillator mix amount |
| 15 | Accent Level | Intensity of the accent |
| 16 | Pitch Offset | Global pitch tuning |
| 17 | Env Mod | Filter envelope modulation depth |
| 18 | Wave Blend | Blend between Saw and Square waveforms |
| 71 | Resonance | Filter resonance amount |
| 74 | Cutoff | Filter cutoff frequency |
| 75 | Decay | Filter envelope decay time |
| 76 | Drive | Pre-filter drive amount |
| 77 | Dist Mode | Distortion algorithm selection |
| 78 | Dist Amount | Distortion drive amount |
| 79 | Dist Mix | Dry/Wet mix for distortion |
| 80 | Dist Enable | Toggle distortion on/off |
| 81 | Delay Time | Delay time in milliseconds |
| 82 | Feedback | Delay feedback amount |
| 83 | Delay Mix | Dry/Wet mix for delay |
| 84 | Delay LPF | Low-pass filter on delay feedback |
| 86 | Sync Div | Global delay sync division |
| 88 | Stereo Offset | Offset between L/R delay times |
| 91 | Delay L Div | Left channel delay sync division |
| 92 | Delay R Div | Right channel delay sync division |
| 93 | Delay L Mod | Left channel rhythm modifier (Straight/Dotted/Triplet) |
| 94 | Delay R Mod | Right channel rhythm modifier (Straight/Dotted/Triplet) |
| 100 | Glide Time | Portamento time |

## Detailed Parameters

### Distortion Modes (CC 77)
The distortion effect offers 5 distinct algorithms:
*   **0**: Soft Clip
*   **1**: Hard Clip
*   **2**: Wavefolder
*   **3**: Diode Clipper
*   **4**: WaveNet Tube

### Delay Timing (CC 86, 91, 92)
The delay time can be synchronized to the tempo using the following divisions:
*   **1/16**: 0-15
*   **1/8**: 16-47
*   **1/4**: 48-79
*   **1/2**: 80-111
*   **1/1**: 112-127

### Delay Modifiers (CC 93, 94)
The rhythm of the delay can be modified:
*   **0**: Straight (Standard)
*   **1**: Dotted (1.5x length)
*   **2**: Triplet (0.66x length)
