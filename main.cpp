#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <vector>

#include "bmp.h"
#include "jpeg_algo.h"

using namespace std;

int main(int argc, char* argv[]) {

    initJPEGStandardTables();
    BitWriter jpegBitstreamWriter;
    int prevDC_Y = 0;
    int prevDC_Cb = 0;
    int prevDC_Cr = 0;

    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <input_bmp_file>" << endl;
        return 1;
    }

    ifstream file(argv[1], ios::binary);
    if (!file) {
        cerr << "Error: Cannot open file." << endl;
        return 1;
    }

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    file.read((char*)&fileHeader, sizeof(fileHeader));
    file.read((char*)&infoHeader, sizeof(infoHeader));

    if (fileHeader.fileType != 0x4D42) {
        cerr << "Error: Not a BMP file." << endl;
        return 1;
    }

    int width = infoHeader.width;
    int height = infoHeader.height;
    int rowPadding = (4 - (width * 3) % 4) % 4;

    cout << "Reading BMP: " << width << "x" << height << endl;

    vector<uint8_t> rgbData(width * height * 3);
    vector<uint8_t> rowBuffer(width * 3);

    for (int i = 0; i < height; ++i) {
        file.read((char*)rowBuffer.data(), width * 3);
        file.seekg(rowPadding, ios::cur);

        int targetRow = height - 1 - i;
        for (int j = 0; j < width; ++j) {
            uint8_t b = rowBuffer[j * 3 + 0];
            uint8_t g = rowBuffer[j * 3 + 1];
            uint8_t r = rowBuffer[j * 3 + 2];

            int targetIndex = (targetRow * width + j) * 3;
            rgbData[targetIndex + 0] = r;
            rgbData[targetIndex + 1] = g;
            rgbData[targetIndex + 2] = b;
        }
    }
    file.close();

    for (int mcuY = 0; mcuY < height; mcuY += 16) {
        for (int mcuX = 0; mcuX < width; mcuX += 16) {

            double blockY_TL[8][8] = { 0 }; double blockY_TR[8][8] = { 0 };
            double blockY_BL[8][8] = { 0 }; double blockY_BR[8][8] = { 0 };
            double blockCb[8][8] = { 0 };   double blockCr[8][8] = { 0 };

            for (int y = 0; y < 16; y += 2) {
                for (int x = 0; x < 16; x += 2) {

                    int realY0 = std::min(mcuY + y, height - 1);
                    int realY1 = std::min(mcuY + y + 1, height - 1);
                    int realX0 = std::min(mcuX + x, width - 1);
                    int realX1 = std::min(mcuX + x + 1, width - 1);

                    int idx00 = (realY0 * width + realX0) * 3;
                    int idx01 = (realY0 * width + realX1) * 3;
                    int idx10 = (realY1 * width + realX0) * 3;
                    int idx11 = (realY1 * width + realX1) * 3;

                    Pixel p00 = { rgbData[idx00 + 2], rgbData[idx00 + 1], rgbData[idx00] };
                    Pixel p01 = { rgbData[idx01 + 2], rgbData[idx01 + 1], rgbData[idx01] };
                    Pixel p10 = { rgbData[idx10 + 2], rgbData[idx10 + 1], rgbData[idx10] };
                    Pixel p11 = { rgbData[idx11 + 2], rgbData[idx11 + 1], rgbData[idx11] };

                    YCbCr ycbcr00 = rgbToYCbCr(p00);
                    YCbCr ycbcr01 = rgbToYCbCr(p01);
                    YCbCr ycbcr10 = rgbToYCbCr(p10);
                    YCbCr ycbcr11 = rgbToYCbCr(p11);

                    if (y < 8 && x < 8) {
                        blockY_TL[y][x] = ycbcr00.y - 128.0;         blockY_TL[y][x + 1] = ycbcr01.y - 128.0;
                        blockY_TL[y + 1][x] = ycbcr10.y - 128.0;     blockY_TL[y + 1][x + 1] = ycbcr11.y - 128.0;
                    }
                    else if (y < 8 && x >= 8) {
                        blockY_TR[y][x - 8] = ycbcr00.y - 128.0;     blockY_TR[y][x - 7] = ycbcr01.y - 128.0;
                        blockY_TR[y + 1][x - 8] = ycbcr10.y - 128.0; blockY_TR[y + 1][x - 7] = ycbcr11.y - 128.0;
                    }
                    else if (y >= 8 && x < 8) {
                        blockY_BL[y - 8][x] = ycbcr00.y - 128.0;     blockY_BL[y - 8][x + 1] = ycbcr01.y - 128.0;
                        blockY_BL[y - 7][x] = ycbcr10.y - 128.0;     blockY_BL[y - 7][x + 1] = ycbcr11.y - 128.0;
                    }
                    else {
                        blockY_BR[y - 8][x - 8] = ycbcr00.y - 128.0; blockY_BR[y - 8][x - 7] = ycbcr01.y - 128.0;
                        blockY_BR[y - 7][x - 8] = ycbcr10.y - 128.0; blockY_BR[y - 7][x - 7] = ycbcr11.y - 128.0;
                    }

                    blockCb[y / 2][x / 2] = ((ycbcr00.cb + ycbcr01.cb + ycbcr10.cb + ycbcr11.cb) / 4.0) - 128.0;
                    blockCr[y / 2][x / 2] = ((ycbcr00.cr + ycbcr01.cr + ycbcr10.cr + ycbcr11.cr) / 4.0) - 128.0;
                }
            } 

            double dct_TL[8][8], dct_TR[8][8], dct_BL[8][8], dct_BR[8][8], dct_Cb[8][8], dct_Cr[8][8];
            performDCT(blockY_TL, dct_TL); performDCT(blockY_TR, dct_TR);
            performDCT(blockY_BL, dct_BL); performDCT(blockY_BR, dct_BR);
            performDCT(blockCb, dct_Cb);   performDCT(blockCr, dct_Cr);

            int q_TL[8][8], q_TR[8][8], q_BL[8][8], q_BR[8][8], q_Cb[8][8], q_Cr[8][8];
            performQuantizationY(dct_TL, q_TL); performQuantizationY(dct_TR, q_TR);
            performQuantizationY(dct_BL, q_BL); performQuantizationY(dct_BR, q_BR);
            performQuantizationC(dct_Cb, q_Cb); performQuantizationC(dct_Cr, q_Cr);

            int zz_TL[64], zz_TR[64], zz_BL[64], zz_BR[64], zz_Cb[64], zz_Cr[64];
            performZigZag(q_TL, zz_TL); performZigZag(q_TR, zz_TR);
            performZigZag(q_BL, zz_BL); performZigZag(q_BR, zz_BR);
            performZigZag(q_Cb, zz_Cb); performZigZag(q_Cr, zz_Cr);

            std::vector<RLEPair> rle_TL = performRealRLE(zz_TL);
            std::vector<RLEPair> rle_TR = performRealRLE(zz_TR);
            std::vector<RLEPair> rle_BL = performRealRLE(zz_BL);
            std::vector<RLEPair> rle_BR = performRealRLE(zz_BR);
            std::vector<RLEPair> rle_Cb = performRealRLE(zz_Cb);
            std::vector<RLEPair> rle_Cr = performRealRLE(zz_Cr);

            performStandardHuffmanEncoding(true, prevDC_Y, rle_TL, jpegBitstreamWriter);
            prevDC_Y = rle_TL[0].level;
            performStandardHuffmanEncoding(true, prevDC_Y, rle_TR, jpegBitstreamWriter);
            prevDC_Y = rle_TR[0].level;
            performStandardHuffmanEncoding(true, prevDC_Y, rle_BL, jpegBitstreamWriter);
            prevDC_Y = rle_BL[0].level;
            performStandardHuffmanEncoding(true, prevDC_Y, rle_BR, jpegBitstreamWriter);
            prevDC_Y = rle_BR[0].level;

            performStandardHuffmanEncoding(false, prevDC_Cb, rle_Cb, jpegBitstreamWriter);
            prevDC_Cb = rle_Cb[0].level;
            performStandardHuffmanEncoding(false, prevDC_Cr, rle_Cr, jpegBitstreamWriter);
            prevDC_Cr = rle_Cr[0].level;

        } 
    }

    jpegBitstreamWriter.flush();
    buildFinalJPEGFile("Result.jpg", width, height, jpegBitstreamWriter.byteStream);

    return 0; 
}