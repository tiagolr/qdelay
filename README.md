<h1 align="center">
  <!-- <img src="doc/logo.png" width="200" style="padding: 5px;" /> -->
  QDelay
  <br>
</h1>
<div align="center">

[![Windows Support](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)](https://github.com/tiagolr/qdelay/releases)
[![Ubuntu Support](https://img.shields.io/badge/Linux-E95420?style=for-the-badge&logo=linux&logoColor=white)](https://github.com/tiagolr/qdelay/releases)
[![Mac Support](https://img.shields.io/badge/MACOS-adb8c5?style=for-the-badge&logo=macos&logoColor=white)](https://github.com/tiagolr/qdelay/releases)

</div>
<div align="center">

[![GitHub package.json version](https://img.shields.io/github/v/release/tiagolr/qdelay?color=%40&label=latest)](https://github.com/tiagolr/qdelay/releases/latest)
![GitHub issues](https://img.shields.io/github/issues-raw/tiagolr/qdelay)
![GitHub all releases](https://img.shields.io/github/downloads/tiagolr/qdelay/total)
![Github license](https://img.shields.io/github/license/tiagolr/qdelay)

</div>

**QDelay** (short for quick-delay) is a dual-delay with more features than it should to be considered quick. While it offers nothing groundbreaking it is based on many popular units like *ReplikaXT* and *EchoBoy*. The main goal is to create a free and open plug-in for my own productions, an alternative to the popular [Deelay](https://sixthsample.com/deelay/) by *SixthSample* without premium versions or trimmed features or on-line activation.

<div align="center">

![Screenshot](./doc/qdelay.png)

</div>

## Features

  *

## Download

* [Download latest release](https://github.com/tiagolr/qdelay/releases)
* Current builds include VST3 for Windows, VST3 and LV2 for Linux and AU and VST3 for macOS.

## About

**QDelay** [started as a JSFX](https://github.com/tiagolr/tilr_jsfx?tab=readme-ov-file#qdelay) a few years ago to have a quick delay with basic features like Ping-Pong, cutoff and pitch shift. Like other plug-ins I've rebuilt, the goal is to make an improved version using better tools with better visuals, performance etc.

I decided to build a plug-in that could come somewhat close to EchoBoy in terms of capabilities, the result is good enough but probably nowhere near the quality of Saturations or Tape emulations that SoundToys and Native Instruments engineers can achieve. I'm not that savvy to build my own models so instead the distortion unit is based of [JClones TapeHead](https://github.com/JClones/JSFXClones/blob/master/JClones_TapeHead.md) - an open JSFX tape like distortion.

The pitch shift uses xxx library, it offers real-time pitching without a big performance hit or artifacts for moderate increments.

Delay swing is implemented using serial delay lines with different times and Feel/Offset is implemented in a hacky way by shifting the write position of the input in relation to the feedback with some extra care to pick the leading write to overrided the circular buffer while the trailing write overdubs the existing contents.

Coding jargon aside, feel free to explore the repository for snippets on parametric EQs envelope followers etc.. or most likely just download the plug-in and have fun with it.

## MacOS

Because the builds are unsigned you may have to run the following commands:

```bash
sudo xattr -dr com.apple.quarantine /path/to/your/plugins/QDelay.component
sudo xattr -dr com.apple.quarantine /path/to/your/plugins/Qdelay.vst3
sudo xattr -dr com.apple.quarantine /path/to/your/plugins/Qdelay.lv2
```

The commands above will recursively remove the quarantine flag from the plug-ins.

## Build

```bash
git clone --recurse-submodules https://github.com/tiagolr/qdelay.git

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
