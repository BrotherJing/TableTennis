# TableTennis Tracker

[![Build Status](https://travis-ci.org/BrotherJing/TableTennis.svg?branch=master)](https://travis-ci.org/BrotherJing/TableTennis)

## Installation

```bash
cd ./TableTennis/
mkdir build
cmake ..
make
```

## Usage

Generate camera matrix
```bash
./TableTennis left.mp4 right.mp4
```

Generate 2D coordinates
```bash
./bg xxx.mp4
```

Get Z scale. Find the actual scale for projecting coordinates with z>0
```bash
./GetZScale left.mp4 right.mp4
```

Reconstruct 3D coordinates
```bash
./reconstruct seqLeft.xml seqRight.xml
```

Remap to 2D for visual checking
```bash
./remaptest infoLeft.xml infoRight.xml left.mp4 right.mp4
```

## Screenshots

![3D Reconstruction](http://7xrcar.com1.z0.glb.clouddn.com/Screenshot%20from%202017-03-03%2023-03-23.png)
