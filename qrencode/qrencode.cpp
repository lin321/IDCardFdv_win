// qrencode.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <io.h>
#include <direct.h>


//文件头结构体
#pragma pack(push)
#pragma pack(2)  //两字节对齐，否则bmp_fileheader会占16Byte
//bmp文件头
struct BITMAPFILEHEADER
{
	unsigned short   bfType; //文件标识
	unsigned long	 bfSize; //文件大小
	unsigned long	 bfReserved1; //保留
	unsigned long    bfOffBits;//数据偏移
};

//bmp信息头
struct BITMAPINFOHEADER
{
	unsigned long    biSize;//信息头长度
	unsigned long    biWidth;//宽度
	unsigned long    biHeight;//高度
	unsigned short   biPlanes;//柱面数=1
	unsigned short   biBitCount;//像素位数
	unsigned long    biCompression;//压缩说明
	unsigned long    biSizeImage;//位图数据的大小
	unsigned long    biXPelsPerMeter;//水平分辨率
	unsigned long    biYPelsPerMeter;//垂直分辨率
	unsigned long    biClrUsed;//使用的颜色数
	unsigned long    biClrImportant;//重要的颜色数
};

#pragma pack(pop)


void QREncode(const char* szSourceSring, const char* outputDir)
{
	//char*           szSourceSring = "D358-3C11-5F0D-4B09";
	unsigned int    unWidth, x, y, l, n, unWidthAdjusted, unDataBytes;
	unsigned char*  pRGBData, *pSourceData, *pDestData;
	QRcode*         pQRC;
	FILE*           f;

	if (pQRC = QRcode_encodeString(szSourceSring, 0, QR_ECLEVEL_H, QR_MODE_8, 1))
	{
		unWidth = pQRC->width;
		unWidthAdjusted = unWidth * 8 * 3;
		if (unWidthAdjusted % 4)
			unWidthAdjusted = (unWidthAdjusted / 4 + 1) * 4;
		unDataBytes = unWidthAdjusted * unWidth * 8;

		// Allocate pixels buffer

		if (!(pRGBData = (unsigned char*)malloc(unDataBytes)))
		{
			exit(-1);
		}

		// Preset to white

		memset(pRGBData, 0xff, unDataBytes);


		// Prepare bmp headers

		BITMAPFILEHEADER kFileHeader;
		kFileHeader.bfType = 0x4d42;  // "BM"
		kFileHeader.bfSize = sizeof(BITMAPFILEHEADER) +
			sizeof(BITMAPINFOHEADER) +
			unDataBytes;
		kFileHeader.bfReserved1 = 0;
		kFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) +
			sizeof(BITMAPINFOHEADER);

		BITMAPINFOHEADER kInfoHeader;
		kInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
		kInfoHeader.biWidth = unWidth * 8;
		kInfoHeader.biHeight = -((int)unWidth * 8);
		kInfoHeader.biPlanes = 1;
		kInfoHeader.biBitCount = 24;
		kInfoHeader.biCompression = 0;	// no compression
		kInfoHeader.biSizeImage = 0;
		kInfoHeader.biXPelsPerMeter = 0;
		kInfoHeader.biYPelsPerMeter = 0;
		kInfoHeader.biClrUsed = 0;
		kInfoHeader.biClrImportant = 0;


		// Convert QrCode bits to bmp pixels

		pSourceData = pQRC->data;
		for (y = 0; y < unWidth; y++)
		{
			pDestData = pRGBData + unWidthAdjusted * y * 8;
			for (x = 0; x < unWidth; x++)
			{
				if (*pSourceData & 1)
				{
					for (l = 0; l < 8; l++)
					{
						for (n = 0; n < 8; n++)
						{
							*(pDestData + n * 3 + unWidthAdjusted * l) = 0;
							*(pDestData + 1 + n * 3 + unWidthAdjusted * l) = 0;
							*(pDestData + 2 + n * 3 + unWidthAdjusted * l) = 0;
						}
					}
				}
				pDestData += 3 * 8;
				pSourceData++;
			}
		}


		// Output the bmp file
		std::string outfile(outputDir);
		outfile += szSourceSring;
		outfile += ".bmp";
		if (!(fopen_s(&f, outfile.c_str(), "wb")))
		{
			fwrite(&kFileHeader, sizeof(BITMAPFILEHEADER), 1, f);
			fwrite(&kInfoHeader, sizeof(BITMAPINFOHEADER), 1, f);
			fwrite(pRGBData, sizeof(unsigned char), unDataBytes, f);

			fclose(f);
		}
		else
		{
			printf("Unable to open file");
			exit(-1);
		}

		// Free data

		free(pRGBData);
		QRcode_free(pQRC);
	}
	else
	{
		printf("NULL returned");
		//exit(-1);
	}
}

int main(int argc, char *argv[])
{
	char srcFilePath[256] = { 0 };
	char outputImgDir[256] = { 0 };

	int i = 1;
	while (i < argc) {
		if (strcmp(argv[i], "-src") == 0)
			strcpy_s(srcFilePath, argv[i + 1]);

		if (strcmp(argv[i], "-out") == 0) {
			strcpy_s(outputImgDir, argv[i + 1]);
		}

		i += 2;
	}

	if (srcFilePath[0] == 0) {
		printf("Usage: qrencode [Options]\n\n");
		printf("Options:\n");
		printf("  -src, source file. [required]\n");
		printf("  -out, output folder. [default: .\\output]\n");
		return 0;
	}

	if (outputImgDir[0] == 0) {
		strcpy_s(outputImgDir, ".\\output");
	}
	int idx = 0;
	while (outputImgDir[idx] != 0) {
		if (outputImgDir[idx] == '/')
		outputImgDir[idx] = '\\';
		idx++;
	}
	if (outputImgDir[idx - 1] != '\\') {
		outputImgDir[idx] = '\\';
		outputImgDir[idx + 1] = 0;
	}

	if (_access(outputImgDir, 0) == -1) {
		try {
		_mkdir(outputImgDir);
		}
		catch (...) {
			printf("Unable to open output folder!");
			return 0;
		}
	}
	
	std::ifstream srcfile(srcFilePath);
	
	std::string line;
	while (std::getline(srcfile, line))
	{
		QREncode(line.c_str(), outputImgDir);
	}


	return 0;
}
