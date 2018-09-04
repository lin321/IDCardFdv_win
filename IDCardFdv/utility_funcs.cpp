// utility_funcs.cpp : 实现文件
//
#include "stdafx.h"
#include "utility_funcs.h"
//文件头结构体
#pragma pack(push)
#pragma pack(2)  //两字节对齐，否则bmp_fileheader会占16Byte
//bmp文件头
struct bmp_fileheader
{
	unsigned short   bfType; //文件标识
	unsigned long	 bfSize; //文件大小
	unsigned long	 bfReserved1; //保留
	unsigned long    bfOffBits;//数据偏移
};

//bmp信息头
struct bmp_infoheader
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

using namespace cv;
IplImage *BMP2Ipl(unsigned char *src, int FileSize)
{
	bmp_fileheader *Fileheader = (bmp_fileheader *)src;
	//检查错误
	assert(FileSize == Fileheader->bfSize);
	//判断是否是位图
	if (Fileheader->bfType != 0x4d42)
		return NULL;
	//开始处理信息头
	bmp_infoheader *bmpheader = (bmp_infoheader *)(src + sizeof(bmp_fileheader));
	unsigned int width = bmpheader->biWidth;//宽度
	unsigned int height = bmpheader->biHeight;//高度
	unsigned int hSize = bmpheader->biSize;//信息头长度

	if (bmpheader->biBitCount < 24)
		return NULL;//支持真色彩32位

	if (bmpheader->biCompression != 0)
		return NULL;//不支持压缩算法

					//数据大小
	unsigned int dataByteSize = bmpheader->biSizeImage;//实际位图数据大小
	unsigned int rowByteSize = dataByteSize / height;//对齐的每行数据

	IplImage *img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, bmpheader->biBitCount / 8);
	//拷贝实际数据无颜色表
	img->origin = IPL_ORIGIN_BL;
	memcpy(img->imageData, src + Fileheader->bfOffBits, FileSize - Fileheader->bfOffBits);

	/*
	unsigned char *gc = src + FileSize - rowByteSize;
	char *p = img->imageData;
	for (int i = 0; i<height; ++i)
	{
		memcpy(p, gc, rowByteSize);
		p += rowByteSize;
		gc -= rowByteSize;
	}/**/

	
	return img;

}

