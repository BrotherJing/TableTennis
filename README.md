# TableTennis Tracker

[![Build Status](https://travis-ci.org/BrotherJing/TableTennis.svg?branch=master)](https://travis-ci.org/BrotherJing/TableTennis)

---

## Installation

```
mkdir build
cmake ..
make
```

## Usage

Generate camera matrix
```
./TableTennis left.mp4 right.mp4
```

Generate 2D coordinates
```
./bg xxx.mp4
```

Reconstruct 3D coordinates
```
./reconstruct seqLeft.xml seqRight.xml
```