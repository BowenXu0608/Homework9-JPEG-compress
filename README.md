# BMP to JPEG Compressor

A from-scratch JPEG encoder written in C++, paired with a PyQt5 graphical interface. Converts 24-bit BMP bitmaps into standards-compliant JPEG files without relying on any third-party image library (no libjpeg, no OpenCV).

## Overview

This project implements every step of the JPEG lossy compression pipeline by hand, from BMP parsing all the way through to writing a valid JFIF file. A lightweight Python GUI wraps the C++ executable, providing drag-and-drop input, live preview of the compressed result, and save-as functionality.

## Features

- Reads standard 24-bit BMP files (handles both top-down and bottom-up storage)
- Outputs JFIF-compliant JPEG files that open in any image viewer
- 4:2:0 chroma subsampling for higher compression ratios
- GUI with drag-and-drop, file picker, preview, and save-as
- Pure C++ core — no external image libraries

## JPEG Pipeline

The C++ backend follows the JPEG standard step by step:

1. **BMP parsing** — read file/info headers, locate the pixel data region, handle row padding
2. **Color space conversion** — RGB → YCbCr
3. **Chroma subsampling** — 4:2:0 (each 2×2 pixel block shares one Cb/Cr pair)
4. **Block DCT** — split the image into 16×16 MCUs (4 Y blocks + 1 Cb + 1 Cr) and run a 2D Discrete Cosine Transform on each 8×8 block
5. **Quantization** — apply the standard luminance and chrominance quantization tables
6. **Zig-zag scan** — flatten each 8×8 block into a 64-element 1D array
7. **Run-length encoding** — encode AC coefficients as (run-of-zeros, value) pairs
8. **Huffman coding** — entropy-code DC differences and AC pairs using the standard JPEG Huffman tables
9. **File assembly** — emit SOI / APP0 / DQT / SOF0 / DHT / SOS / compressed data / EOI segments

## Project Structure

```
├── bmp.h              # BMP header and pixel structs
├── jpeg_algo.h        # JPEG core (DCT, quantization, zig-zag, RLE, Huffman, file writer)
├── main.cpp           # BMP loader and MCU encoding loop
├── gui.py             # PyQt5 graphical interface
└── x64/Release/
    └── homework_week_9.exe   # Built executable (path used by the GUI)
```

## Build & Run

### 1. Build the C++ backend

Open the project in Visual Studio and build in **Release x64** mode. The output should be at `x64\Release\homework_week_9.exe`.

Or build from the command line:

```bash
cl /EHsc /O2 main.cpp /Fe:homework_week_9.exe
```

### 2. Command-line usage

```bash
homework_week_9.exe <input_bmp> [output_jpeg]
```

Example:

```bash
homework_week_9.exe input.bmp output.jpg
```

If the output path is omitted, the file is written to `Result.jpg`.

### 3. Launch the GUI

Install dependencies:

```bash
pip install PyQt5
```

Run:

```bash
python gui.py
```

## How to Use the GUI

1. Launch `gui.py`. Drag a BMP file into the window, or click **Select BMP File**.
2. The app calls the C++ backend, compresses the image, and shows a preview of the result.
3. Click **Save / Save As JPEG** and pick a destination to export the final file.

## Implementation Notes

- **MCU layout**: 16×16 MCUs with sampling factors 2:1:1 (4 Y blocks, 1 Cb, 1 Cr), corresponding to `0x22, 0x11, 0x11` in the SOF0 segment.
- **DC differential coding**: each component (Y/Cb/Cr) keeps its own `prevDC`, and DC coefficients are differential-coded then VLI-encoded per the JPEG spec.
- **Byte stuffing**: any `0xFF` byte in the bitstream is followed by `0x00` to avoid colliding with marker codes.
- **Edge handling**: when the image dimensions are not multiples of 16, edge pixels are clamped via `std::min` so the encoder never reads out of bounds.

## Known Limitations

- Only 24-bit uncompressed BMPs are supported. 8-bit indexed, 16-bit, 32-bit, and RLE-compressed BMPs are not handled.
- Quantization tables are fixed — there is no quality slider.
- The GUI uses Qt's built-in JPEG decoder for preview, which doubles as a sanity check that the encoder output is valid.

## License

Personal coursework project, shared for learning and reference purposes.
