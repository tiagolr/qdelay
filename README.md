<h1 align="center">
  <!-- <img src="doc/logo.png" width="200" style="padding: 5px;" /> -->
  REEV-R
  <br>
</h1>
<div align="center">

[![Windows Support](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)](https://github.com/tiagolr/reevr/releases)
[![Ubuntu Support](https://img.shields.io/badge/Linux-E95420?style=for-the-badge&logo=linux&logoColor=white)](https://github.com/tiagolr/reevr/releases)
[![Mac Support](https://img.shields.io/badge/MACOS-adb8c5?style=for-the-badge&logo=macos&logoColor=white)](https://github.com/tiagolr/reevr/releases)

</div>
<div align="center">

[![GitHub package.json version](https://img.shields.io/github/v/release/tiagolr/reevr?color=%40&label=latest)](https://github.com/tiagolr/reevr/releases/latest)
![GitHub issues](https://img.shields.io/github/issues-raw/tiagolr/reevr)
![GitHub all releases](https://img.shields.io/github/downloads/tiagolr/reevr/total)
![Github license](https://img.shields.io/github/license/tiagolr/reevr)

</div>

**REEV-R** is a cross-platform convolution reverb with modulation for pre/send and post/volume signals.

<div align="center">

![Screenshot](./doc/reevr.png)

</div>

## Features

  * High performance Convolution Reverb thanks to KlangFalter library
  * IR manipulations like stretch, trim, reverse, attack and decay
  * Parametric EQs for IR frequency and frequency decay
  * Pre/send and post/reverb MSEG modulation
  * 12 patterns triggered by midi notes
  * Paint mode with user defined shapes
  * Point type - hold, curve, s-curve, stairs ..
  * Built-in sequencer and randomizer
  * Envelope followers for send and reverb
  * Tempo sync or lfo rate (Hz)
  * Attack and release smooth
  * Pre and post waveform display
  * MIDI trigger mode
  * Audio trigger mode

## Download

* [Download latest release](https://github.com/tiagolr/reevr/releases)
* Current builds include VST3 and LV2 for Windows, Linux and macOS plus AU for macOS.
* Clap is planned when there is official [JUCE support](https://juce.com/blog/juce-roadmap-update-q3-2024/).

## About

REEV-R is a convolution reverb with modulation capabilities, it is possible thanks to [KlangFalter](https://github.com/HiFi-LoFi/KlangFalter) FFT Convolution library, JUCE stock convolution was consuming 20% CPU usage on debug builds while this one takes less than 1%, this was the only library I found that perfectly fits this plugin.

## MacOS

Because the builds are unsigned you may have to run the following commands:

```bash
sudo xattr -dr com.apple.quarantine /path/to/your/plugins/REEV-R.component
sudo xattr -dr com.apple.quarantine /path/to/your/plugins/REEV-R.vst3
sudo xattr -dr com.apple.quarantine /path/to/your/plugins/REEV-R.lv2
```

The commands above will recursively remove the quarantine flag from the plugins.

## Build

```bash
git clone --recurse-submodules https://github.com/tiagolr/reevr.git

# windows
cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release -S . -B ./build

# linux
sudo apt update
sudo apt-get install libx11-dev libfreetype-dev libfontconfig1-dev libasound2-dev libxrandr-dev libxinerama-dev libxcursor-dev
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -S . -B ./build
cmake --build ./build --config Release

# macOS
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -S . -B ./build
cmake --build ./build --config Release
```
