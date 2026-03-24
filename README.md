# BMP to JPEG Encoder

A C++ program that converts a BMP image to a JPEG image from scratch.

## How to Run

### 1. Prepare your image
Place your target BMP image (for example, `test.bmp`) into the same directory as the source code.

### 2. Compile the code
Open your terminal and compile the project using `g++`:

```bash
g++ main.cpp -o jpeg_encoder -O3 -std=c++11
```

### 3. Execute the program
Run the compiled executable and pass your BMP file name as an argument:

```bash
./jpeg_encoder test.bmp
```

The program will process the image and generate a `Result.jpg` file in the same directory.

---

## Example
*Here is the input test image:*
![Input Image](test.bmp)
