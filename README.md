# pico-303

A TB-303 style synthesizer firmware for the Raspberry Pi Pico 2, featuring a hybrid emulation engine.

## Acknowledgments

This project stands on the shoulders of giants. Huge thanks to:

*   **[Open303](https://github.com/open303/open303)**: For the core DSP and envelope modeling.
*   **[jc303](https://github.com/jc303/jc303)**: For additional filter and synthesis insights.

## Hardware
*   **Microcontroller**: [Raspberry Pi Pico 2 (RP2350)](https://www.raspberrypi.com/products/raspberry-pi-pico-2/)
*   **Audio**: [Pimoroni Pico Audio Pack](https://shop.pimoroni.com/products/pico-audio-pack?variant=32369490853971) (PCM5100A DAC)
*   **Display**: SSD1306 OLED (128x32)
*   **Input**: Rotary Encoder with Push Button

<div style="display: flex; justify-content: space-around; flex-wrap: wrap;">
  <img src="images/setup01.jpg" alt="Setup 1" style="width: 49%; max-width: 400px; margin-bottom: 10px;">
  <img src="images/setup02.jpg" alt="Setup 2" style="width: 49%; max-width: 400px; margin-bottom: 10px;">
</div>

### Pinout Connections

| Component | Pin | Pico GPIO | Description |
| :--- | :--- | :--- | :--- |
| **OLED** | SDA | GP2 | I2C Data |
| | SCL | GP3 | I2C Clock |
| **Encoder** | A | GP6 | Encoder Pin A |
| | B | GP7 | Encoder Pin B |
| | SW | GP8 | Encoder Push Button |
| **Audio Pack** | DATA | GP9 | I2S Data Out (DOUT) |
| | BCLK | GP10 | I2S Bit Clock |
| | LRCLK | GP11 | I2S Word Clock (L/R) |

## Build Instructions

1.  **Install Arduino IDE**.
2.  **Install RP2040/RP2350 Core**: Add `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json` to your Additional Boards Manager URLs and install **Raspberry Pi Pico/RP2040** by Earle F. Philhower, III.
3.  **Select Board**: Tools > Board > Raspberry Pi Pico/RP2040 > **Raspberry Pi Pico 2**.
4.  **CPU Speed**: Tools > CPU Speed > **250 MHz (Overclock)**.
5.  **USB Stack**: Tools > USB Stack > **Adafruit TinyUSB**.
6.  **Install Libraries** (via Library Manager):
    *   `Adafruit TinyUSB Library`
    *   `MIDI Library` (by Forty Seven Effects)
    *   `Adafruit SSD1306`
    *   `Adafruit GFX Library`
7.  **Compile & Upload**: Connect your Pico 2 while holding BOOTSEL, then upload.

## Web Controller

[https://akashic-trance-machines.github.io/pico-303](https://akashic-trance-machines.github.io/pico-303/) a MIDI controller/sequencer for the pico-303. Use Google Chrome for the MIDI connection.

### Features
*   **Full Parameter Control**: Sliders for every CC parameter supported by the firmware.
*   **16-Step Sequencer**: A built-in TB-303 style sequencer with slide and accent support.
*   **Distortion & Delay Control**: Dedicated sections for tweaking the effects chain.

### Usage
1.  Connect the Pico to your computer via USB.
2.  Open `webController/index.html` or [https://akashic-trance-machines.github.io/pico-303](https://akashic-trance-machines.github.io/pico-303) in a Web MIDI compatible browser (e.g., Chrome, Edge).
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
| 77 | Dist Mode | Distortion algorithm selection |
| 78 | Dist Amount | Distortion drive amount |
| 79 | Dist Mix | Dry/Wet mix for distortion |
| 80 | Dist Enable | Toggle distortion on/off |
| 81 | Delay Time | Delay time in milliseconds |
| 82 | Feedback | Delay feedback amount |
| 83 | Delay Mix | Dry/Wet mix for delay |
| 86 | Sync Div | Global delay sync division |
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

## TODO / Future Improvements

### Hardware Enhancements
- [ ] Add TRS MIDI IN and MIDI THRU connectors
- [ ] Add physical buttons and LEDs for synth control
- [ ] Design and manufacture custom PCB
- [ ] Add PSRAM and flash memory for expanded capabilities
- [ ] Find better rotary encoder with more steps and no detents
- [ ] Create a nice enclosure/case

### Firmware Features
- [ ] Implement chorus effect on second core
- [ ] Improve current delay implementation
- [ ] Add reverb effect on second core
- [ ] Create preset system for saving/loading patches
- [ ] Enhance UI/UX on OLED display
